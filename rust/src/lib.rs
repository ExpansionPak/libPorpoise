use std::collections::HashMap;
use std::num::NonZeroIsize;
use std::path::PathBuf;
use std::io::Write;
use std::sync::{Mutex, OnceLock};

use bytemuck::{Pod, Zeroable};
use image::{ColorType, ImageFormat};
use raw_window_handle::{
    RawDisplayHandle, RawWindowHandle, Win32WindowHandle, WindowsDisplayHandle,
};

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct RustRange {
    pub offset: u32,
    pub size: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct RustDrawCommand {
    pub vert_range: RustRange,
    pub idx_range: RustRange,
    pub index_count: u32,
    pub primitive: u32,
    pub vtx_fmt: u32,
    pub dst_alpha: u32,
    pub indexed_stride: u32,
    pub pos_index_offset: u32,
    pub color_index_offset: u32,
    pub pos_index_size: u32,
    pub color_index_size: u32,
    pub pos_array_stride: u32,
    pub color_array_stride: u32,
    pub model_view: [f32; 16],
    pub array_ranges: [RustRange; 26],
}

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct RustFrameState {
    pub fb_width: u16,
    pub fb_height: u16,
    pub clear_color: [f32; 4],
    pub proj: [f32; 16],
    pub model_view: [f32; 16],
    pub depth_compare: u32,
    pub depth_update: u32,
    pub color_update: u32,
    pub alpha_update: u32,
    pub cull_mode: u32,
    pub blend_mode: u32,
    pub blend_src: u32,
    pub blend_dst: u32,
    pub blend_op: u32,
    pub depth_func: u32,
    pub num_tev_stages: u32,
    pub num_tex_gens: u32,
    pub num_chans: u32,
}

#[derive(Default)]
struct RendererState {
    init: bool,
    window_handle: u64,
    display_handle: u64,
    frame_state: RustFrameState,
    draw_commands: Vec<RustDrawCommand>,
    action_counters: [u64; 8],
    last_status: &'static str,
    gpu: Option<GpuState>,
    pipeline_cache: HashMap<u64, wgpu::RenderPipeline>,
    cached_rgba: Vec<u8>,
    cached_width: u32,
    cached_height: u32,
    capture_enabled: bool,
    capture_remaining: u32,
    capture_index: u32,
    capture_dir: String,
    debug_log_enabled: bool,
    logged_draw_debug: bool,
}

fn global_state() -> &'static Mutex<RendererState> {
    static STATE: OnceLock<Mutex<RendererState>> = OnceLock::new();
    STATE.get_or_init(|| Mutex::new(RendererState::default()))
}

fn capture_config_from_env() -> (bool, u32) {
    let enabled = std::env::var("PORPOISE_CAPTURE_PNG")
        .ok()
        .map(|v| v == "1" || v.eq_ignore_ascii_case("true"))
        .unwrap_or(false);
    let count = std::env::var("PORPOISE_CAPTURE_PNG_COUNT")
        .ok()
        .and_then(|v| v.parse::<u32>().ok())
        .unwrap_or(4);
    (enabled, count.max(1))
}

fn debug_log_enabled_from_env() -> bool {
    std::env::var("PORPOISE_BRIDGE_DEBUG")
        .ok()
        .map(|v| v == "1" || v.eq_ignore_ascii_case("true"))
        .unwrap_or(false)
}

fn dump_capture_png(path: &str, rgba: &[u8], width: u32, height: u32) -> Result<(), &'static str> {
    image::save_buffer_with_format(path, rgba, width, height, ColorType::Rgba8, ImageFormat::Png)
        .map_err(|_| "capture_png_write_failed")
}

fn dump_debug_error_png(path: &str) {
    let rgba = [
        255u8, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255,
    ];
    let _ = dump_capture_png(path, &rgba, 2, 2);
}

fn dump_reason_png(state: &mut RendererState, reason: &str, rgba: [u8; 4]) {
    if !state.capture_enabled || state.capture_remaining == 0 {
        return;
    }
    let px = [
        rgba[0], rgba[1], rgba[2], rgba[3], rgba[0], rgba[1], rgba[2], rgba[3], rgba[0], rgba[1], rgba[2], rgba[3],
        rgba[0], rgba[1], rgba[2], rgba[3],
    ];
    let path = format!(
        "{}/porpoise_capture_reason_{}_{}.png",
        state.capture_dir, reason, state.capture_index
    );
    let _ = dump_capture_png(&path, &px, 2, 2);
    state.capture_index = state.capture_index.saturating_add(1);
    state.capture_remaining = state.capture_remaining.saturating_sub(1);
}

fn default_capture_dir() -> String {
    let exe_dir: Option<PathBuf> = std::env::current_exe()
        .ok()
        .and_then(|p| p.parent().map(|d| d.to_path_buf()));
    exe_dir
        .map(|d| d.to_string_lossy().to_string())
        .unwrap_or_else(|| ".".to_string())
}

fn write_capture_probe(path: &str) {
    let _ = std::fs::write(path, b"porpoise_rust_capture_probe\n");
}

fn append_debug_log(dir: &str, line: &str) {
    let path = format!("{}/porpoise_bridge_debug.log", dir);
    if let Ok(mut f) = std::fs::OpenOptions::new().create(true).append(true).open(path) {
        let _ = f.write_all(line.as_bytes());
        let _ = f.write_all(b"\n");
    }
}

fn mul_mat4_vec4_col_major(m: &[f32; 16], v: [f32; 4]) -> [f32; 4] {
    [
        m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * v[3],
        m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * v[3],
        m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3],
        m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3],
    ]
}

/// Returns projection matrix with Y row negated (flip view right-side up without shader Y flip).
fn proj_flip_y(proj: &[f32; 16]) -> [f32; 16] {
    let mut p = *proj;
    p[1] = -p[1];
    p[5] = -p[5];
    p[9] = -p[9];
    p[13] = -p[13];
    p
}

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct Vertex {
    pos: [f32; 3],
    color: [f32; 4],
}

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct TransformData {
    proj: [f32; 16],
    model_view: [f32; 16],
}

struct GpuState {
    instance: wgpu::Instance,
    surface: wgpu::Surface<'static>,
    device: wgpu::Device,
    queue: wgpu::Queue,
    config: wgpu::SurfaceConfiguration,
    depth_tex: wgpu::Texture,
    depth_view: wgpu::TextureView,
    width: u32,
    height: u32,
    surface_format: wgpu::TextureFormat,
    shader: wgpu::ShaderModule,
    transform_layout: wgpu::BindGroupLayout,
    vertex_layout: wgpu::VertexBufferLayout<'static>,
}

const GX_QUADS: u32 = 0x80;
const GX_TRIANGLES: u32 = 0x90;
const GX_TRIANGLESTRIP: u32 = 0x98;
const GX_TRIANGLEFAN: u32 = 0xA0;
const GX_LINES: u32 = 0xA8;
const GX_LINESTRIP: u32 = 0xB0;
const GX_POINTS: u32 = 0xB8;

const GX_CULL_NONE: u32 = 0;
const GX_CULL_FRONT: u32 = 1;
const GX_CULL_BACK: u32 = 2;
const GX_CULL_ALL: u32 = 3;

const GX_BM_NONE: u32 = 0;
const GX_BM_BLEND: u32 = 1;
const GX_BM_LOGIC: u32 = 2;
const GX_BM_SUBTRACT: u32 = 3;

const GX_BL_ZERO: u32 = 0;
const GX_BL_ONE: u32 = 1;
const GX_BL_SRCCLR: u32 = 2;
const GX_BL_INVSRCCLR: u32 = 3;
const GX_BL_SRCALPHA: u32 = 4;
const GX_BL_INVSRCALPHA: u32 = 5;
const GX_BL_DSTALPHA: u32 = 6;
const GX_BL_INVDSTALPHA: u32 = 7;

fn choose_backends() -> wgpu::Backends {
    #[cfg(target_os = "windows")]
    {
        wgpu::Backends::DX12 | wgpu::Backends::VULKAN
    }
    #[cfg(not(target_os = "windows"))]
    {
        wgpu::Backends::all()
    }
}

fn make_surface(
    instance: &wgpu::Instance,
    window_handle: u64,
    display_handle: u64,
) -> Result<wgpu::Surface<'static>, &'static str> {
    if window_handle == 0 {
        return Err("window_handle_missing");
    }
    let hwnd = NonZeroIsize::new(window_handle as isize).ok_or("window_handle_invalid")?;
    let mut win = Win32WindowHandle::new(hwnd);
    win.hinstance = NonZeroIsize::new(display_handle as isize);
    let raw_window = RawWindowHandle::Win32(win);
    let raw_display = RawDisplayHandle::Windows(WindowsDisplayHandle::new());
    // SAFETY: native handles are provided by SDL and remain valid for the app lifetime.
    unsafe {
        instance
            .create_surface_unsafe(wgpu::SurfaceTargetUnsafe::RawHandle {
                raw_display_handle: raw_display,
                raw_window_handle: raw_window,
            })
            .map_err(|_| "create_surface_failed")
    }
}

fn create_gpu_state(
    window_handle: u64,
    display_handle: u64,
    width: u32,
    height: u32,
) -> Result<GpuState, &'static str> {
    let instance = wgpu::Instance::new(&wgpu::InstanceDescriptor {
        backends: choose_backends(),
        ..Default::default()
    });
    let surface = make_surface(&instance, window_handle, display_handle)?;
    let adapter = pollster::block_on(instance.request_adapter(&wgpu::RequestAdapterOptions {
        power_preference: wgpu::PowerPreference::HighPerformance,
        compatible_surface: Some(&surface),
        force_fallback_adapter: false,
    }))
    .map_err(|_| "request_adapter_failed")?;

    let (device, queue) = pollster::block_on(adapter.request_device(&wgpu::DeviceDescriptor {
        label: Some("porpoise-rust-renderer-device"),
        required_features: wgpu::Features::empty(),
        required_limits: wgpu::Limits::downlevel_defaults(),
        memory_hints: wgpu::MemoryHints::Performance,
        trace: wgpu::Trace::Off,
    }))
    .map_err(|_| "request_device_failed")?;

    let caps = surface.get_capabilities(&adapter);
    let surface_format = caps
        .formats
        .iter()
        .copied()
        .find(|f| matches!(f, wgpu::TextureFormat::Bgra8Unorm | wgpu::TextureFormat::Rgba8Unorm))
        .or_else(|| caps.formats.first().copied())
        .ok_or("surface_no_format")?;
    let config = wgpu::SurfaceConfiguration {
        usage: wgpu::TextureUsages::RENDER_ATTACHMENT | wgpu::TextureUsages::COPY_SRC,
        format: surface_format,
        width: width.max(1),
        height: height.max(1),
        present_mode: wgpu::PresentMode::Fifo,
        desired_maximum_frame_latency: 2,
        alpha_mode: caps.alpha_modes.first().copied().unwrap_or(wgpu::CompositeAlphaMode::Auto),
        view_formats: vec![],
    };
    surface.configure(&device, &config);

    let depth_tex = device.create_texture(&wgpu::TextureDescriptor {
        label: Some("porpoise-rust-depth"),
        size: wgpu::Extent3d {
            width: config.width,
            height: config.height,
            depth_or_array_layers: 1,
        },
        mip_level_count: 1,
        sample_count: 1,
        dimension: wgpu::TextureDimension::D2,
        format: wgpu::TextureFormat::Depth32Float,
        usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
        view_formats: &[],
    });
    let depth_view = depth_tex.create_view(&wgpu::TextureViewDescriptor::default());

    let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: Some("porpoise-rust-shader"),
        source: wgpu::ShaderSource::Wgsl(
            "
struct TransformData {
    proj: mat4x4<f32>,
    model_view: mat4x4<f32>,
}

@group(0) @binding(0)
var<uniform> transforms: TransformData;

struct VsIn {
    @location(0) pos: vec3<f32>,
    @location(1) color: vec4<f32>,
}

struct VsOut {
    @builtin(position) pos: vec4<f32>,
    @location(0) color: vec4<f32>,
}

@vertex
fn vs_main(in: VsIn) -> VsOut {
    var out: VsOut;
    let world = transforms.model_view * vec4<f32>(in.pos, 1.0);
    out.pos = transforms.proj * world;
    // TEMPORARY: Mirror Z for depth ordering. GX clip z in [-w, 0]; we use z += w so NDC in [0,1]
    // (0=near, 1=far) with standard depth clear 1.0 and Less. This may not match true GX depth
    // semantics; proper mapping TBD.
    out.pos.z = out.pos.z + out.pos.w;
    // Y flip only in projection (proj_flip_y); no shader flip or screen is black.
    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: VsOut) -> @location(0) vec4<f32> {
    return in.color;
}
"
            .into(),
        ),
    });

    let vertex_layout = wgpu::VertexBufferLayout {
        array_stride: std::mem::size_of::<Vertex>() as u64,
        step_mode: wgpu::VertexStepMode::Vertex,
        attributes: &[
            wgpu::VertexAttribute {
                offset: 0,
                shader_location: 0,
                format: wgpu::VertexFormat::Float32x3,
            },
            wgpu::VertexAttribute {
                offset: 12,
                shader_location: 1,
                format: wgpu::VertexFormat::Float32x4,
            },
        ],
    };
    let transform_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("porpoise-rust-transform-layout"),
        entries: &[wgpu::BindGroupLayoutEntry {
            binding: 0,
            visibility: wgpu::ShaderStages::VERTEX,
            ty: wgpu::BindingType::Buffer {
                ty: wgpu::BufferBindingType::Uniform,
                has_dynamic_offset: false,
                min_binding_size: None,
            },
            count: None,
        }],
    });

    let cfg_w = config.width;
    let cfg_h = config.height;
    Ok(GpuState {
        instance,
        surface,
        device,
        queue,
        config,
        depth_tex,
        depth_view,
        width: cfg_w,
        height: cfg_h,
        surface_format,
        shader,
        transform_layout,
        vertex_layout,
    })
}

fn ensure_gpu_size(state: &mut RendererState, width: u32, height: u32) -> Result<(), &'static str> {
    let want_w = width.max(1);
    let want_h = height.max(1);
    if state.gpu.is_none() {
        state.gpu = Some(create_gpu_state(
            state.window_handle,
            state.display_handle,
            want_w,
            want_h,
        )?);
        state.pipeline_cache.clear();
        return Ok(());
    }
    let gpu = state.gpu.as_mut().ok_or("no_gpu")?;
    if gpu.width != want_w || gpu.height != want_h {
        gpu.width = want_w;
        gpu.height = want_h;
        gpu.config.width = want_w;
        gpu.config.height = want_h;
        gpu.surface.configure(&gpu.device, &gpu.config);
        gpu.depth_tex = gpu.device.create_texture(&wgpu::TextureDescriptor {
            label: Some("porpoise-rust-depth"),
            size: wgpu::Extent3d {
                width: want_w,
                height: want_h,
                depth_or_array_layers: 1,
            },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Depth32Float,
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            view_formats: &[],
        });
        gpu.depth_view = gpu
            .depth_tex
            .create_view(&wgpu::TextureViewDescriptor::default());
    }
    Ok(())
}

fn map_blend_factor(f: u32) -> wgpu::BlendFactor {
    match f {
        GX_BL_ZERO => wgpu::BlendFactor::Zero,
        GX_BL_ONE => wgpu::BlendFactor::One,
        GX_BL_SRCCLR => wgpu::BlendFactor::Src,
        GX_BL_INVSRCCLR => wgpu::BlendFactor::OneMinusSrc,
        GX_BL_SRCALPHA => wgpu::BlendFactor::SrcAlpha,
        GX_BL_INVSRCALPHA => wgpu::BlendFactor::OneMinusSrcAlpha,
        GX_BL_DSTALPHA => wgpu::BlendFactor::DstAlpha,
        GX_BL_INVDSTALPHA => wgpu::BlendFactor::OneMinusDstAlpha,
        _ => wgpu::BlendFactor::One,
    }
}

fn map_compare_mode(v: u32) -> wgpu::CompareFunction {
    match v {
        0 => wgpu::CompareFunction::Never,
        1 => wgpu::CompareFunction::Less,
        2 => wgpu::CompareFunction::Equal,
        3 => wgpu::CompareFunction::LessEqual,
        4 => wgpu::CompareFunction::Greater,
        5 => wgpu::CompareFunction::NotEqual,
        6 => wgpu::CompareFunction::GreaterEqual,
        _ => wgpu::CompareFunction::Always,
    }
}

/// Inverted for our depth output: we output 1=near, 0=far; so "closer wins" = Greater.
fn map_compare_mode_inverted(v: u32) -> wgpu::CompareFunction {
    match v {
        0 => wgpu::CompareFunction::Never,
        1 => wgpu::CompareFunction::Greater,      // Less -> Greater
        2 => wgpu::CompareFunction::Equal,
        3 => wgpu::CompareFunction::GreaterEqual, // LessEqual -> GreaterEqual
        4 => wgpu::CompareFunction::Less,         // Greater -> Less
        5 => wgpu::CompareFunction::NotEqual,
        6 => wgpu::CompareFunction::LessEqual,    // GreaterEqual -> LessEqual
        _ => wgpu::CompareFunction::Always,
    }
}

fn map_primitive_topology(p: u32) -> Option<wgpu::PrimitiveTopology> {
    match p {
        GX_TRIANGLES | GX_QUADS => Some(wgpu::PrimitiveTopology::TriangleList),
        GX_TRIANGLESTRIP => Some(wgpu::PrimitiveTopology::TriangleStrip),
        GX_TRIANGLEFAN => Some(wgpu::PrimitiveTopology::TriangleList),
        GX_LINES => Some(wgpu::PrimitiveTopology::LineList),
        GX_LINESTRIP => Some(wgpu::PrimitiveTopology::LineStrip),
        GX_POINTS => Some(wgpu::PrimitiveTopology::PointList),
        _ => None,
    }
}

fn encode_pipeline_key(frame_state: &RustFrameState, topology: wgpu::PrimitiveTopology) -> u64 {
    let mut key = 0u64;
    key |= (topology as u64) & 0x7;
    key |= (frame_state.depth_compare as u64) << 8;
    key |= (frame_state.depth_update as u64) << 16;
    key |= (frame_state.depth_func as u64) << 20;
    key |= (frame_state.cull_mode as u64) << 28;
    key |= (frame_state.blend_mode as u64) << 36;
    key |= (frame_state.blend_src as u64) << 40;
    key |= (frame_state.blend_dst as u64) << 44;
    key |= (frame_state.color_update as u64) << 52;
    key |= (frame_state.alpha_update as u64) << 56;
    key
}

fn encode_pipeline_key_with_front_face(
    frame_state: &RustFrameState,
    topology: wgpu::PrimitiveTopology,
    front_face: wgpu::FrontFace,
) -> u64 {
    let mut key = encode_pipeline_key(frame_state, topology);
    key |= (match front_face {
        wgpu::FrontFace::Ccw => 0u64,
        wgpu::FrontFace::Cw => 1u64,
    }) << 60;
    key
}

fn build_pipeline(
    device: &wgpu::Device,
    shader: &wgpu::ShaderModule,
    transform_layout: &wgpu::BindGroupLayout,
    vertex_layout: &wgpu::VertexBufferLayout<'static>,
    color_format: wgpu::TextureFormat,
    frame_state: &RustFrameState,
    topology: wgpu::PrimitiveTopology,
    front_face: wgpu::FrontFace,
) -> wgpu::RenderPipeline {
    let blend = if frame_state.blend_mode == GX_BM_BLEND || frame_state.blend_mode == GX_BM_SUBTRACT {
        Some(wgpu::BlendState {
            color: wgpu::BlendComponent {
                src_factor: map_blend_factor(frame_state.blend_src),
                dst_factor: map_blend_factor(frame_state.blend_dst),
                operation: if frame_state.blend_mode == GX_BM_SUBTRACT {
                    wgpu::BlendOperation::Subtract
                } else {
                    wgpu::BlendOperation::Add
                },
            },
            alpha: wgpu::BlendComponent::OVER,
        })
    } else {
        None
    };

    let mut write_mask = wgpu::ColorWrites::empty();
    if frame_state.color_update != 0 {
        write_mask |= wgpu::ColorWrites::RED | wgpu::ColorWrites::GREEN | wgpu::ColorWrites::BLUE;
    }
    if frame_state.alpha_update != 0 {
        write_mask |= wgpu::ColorWrites::ALPHA;
    }
    if write_mask.is_empty() {
        write_mask = wgpu::ColorWrites::ALL;
    }

    // GX front face is CCW; cull back faces for correct solid geometry.
    let cull_mode = match frame_state.cull_mode {
        GX_CULL_NONE => None,
        GX_CULL_FRONT => Some(wgpu::Face::Front),
        GX_CULL_BACK => Some(wgpu::Face::Back),
        GX_CULL_ALL => Some(wgpu::Face::Front),
        _ => Some(wgpu::Face::Back),
    };

    let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("porpoise-rust-layout"),
            bind_group_layouts: &[transform_layout],
            push_constant_ranges: &[],
        });

    device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("porpoise-rust-pipeline"),
            layout: Some(&pipeline_layout),
            vertex: wgpu::VertexState {
                module: shader,
                entry_point: Some("vs_main"),
                buffers: &[vertex_layout.clone()],
                compilation_options: Default::default(),
            },
            primitive: wgpu::PrimitiveState {
                topology,
                strip_index_format: if topology == wgpu::PrimitiveTopology::TriangleStrip
                    || topology == wgpu::PrimitiveTopology::LineStrip
                {
                    Some(wgpu::IndexFormat::Uint32)
                } else {
                    None
                },
                front_face,
                cull_mode,
                unclipped_depth: false,
                polygon_mode: wgpu::PolygonMode::Fill,
                conservative: false,
            },
            // Note: Depth uses a temporary "mirror Z" (z += w, clear 1.0, Less). May not match GX exactly.
            depth_stencil: Some(wgpu::DepthStencilState {
                format: wgpu::TextureFormat::Depth32Float,
                depth_write_enabled: frame_state.depth_update != 0,
                depth_compare: if frame_state.depth_compare != 0 {
                    map_compare_mode(frame_state.depth_func)
                } else {
                    wgpu::CompareFunction::Always
                },
                stencil: Default::default(),
                bias: Default::default(),
            }),
            multisample: Default::default(),
            fragment: Some(wgpu::FragmentState {
                module: shader,
                entry_point: Some("fs_main"),
                targets: &[Some(wgpu::ColorTargetState {
                    format: color_format,
                    blend,
                    write_mask,
                })],
                compilation_options: Default::default(),
            }),
            multiview: None,
            cache: None,
        })
}

fn detect_front_face(
    verts: &[Vertex],
    inds: &[u32],
    model_view: &[f32; 16],
    proj: &[f32; 16],
) -> wgpu::FrontFace {
    if inds.len() < 3 || verts.is_empty() {
        return wgpu::FrontFace::Ccw;
    }
    let mut i = 0usize;
    while i + 2 < inds.len() {
        let ia = inds[i] as usize;
        let ib = inds[i + 1] as usize;
        let ic = inds[i + 2] as usize;
        if ia >= verts.len() || ib >= verts.len() || ic >= verts.len() {
            i += 3;
            continue;
        }
        let ta = mul_mat4_vec4_col_major(
            model_view,
            [verts[ia].pos[0], verts[ia].pos[1], verts[ia].pos[2], 1.0],
        );
        let tb = mul_mat4_vec4_col_major(
            model_view,
            [verts[ib].pos[0], verts[ib].pos[1], verts[ib].pos[2], 1.0],
        );
        let tc = mul_mat4_vec4_col_major(
            model_view,
            [verts[ic].pos[0], verts[ic].pos[1], verts[ic].pos[2], 1.0],
        );
        let mut ca = mul_mat4_vec4_col_major(proj, ta);
        let mut cb = mul_mat4_vec4_col_major(proj, tb);
        let mut cc = mul_mat4_vec4_col_major(proj, tc);
        ca[2] = ca[2] + ca[3];
        cb[2] = cb[2] + cb[3];
        cc[2] = cc[2] + cc[3];
        if ca[3].abs() <= f32::EPSILON || cb[3].abs() <= f32::EPSILON || cc[3].abs() <= f32::EPSILON {
            i += 3;
            continue;
        }
        let ax = ca[0] / ca[3];
        let ay = ca[1] / ca[3];
        let bx = cb[0] / cb[3];
        let by = cb[1] / cb[3];
        let cx = cc[0] / cc[3];
        let cy = cc[1] / cc[3];
        let area2 = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
        if area2.abs() > 1.0e-7 {
            return if area2 > 0.0 {
                wgpu::FrontFace::Ccw
            } else {
                wgpu::FrontFace::Cw
            };
        }
        i += 3;
    }
    wgpu::FrontFace::Ccw
}

fn read_index(data: &[u8], offset: usize, size: u32) -> Option<u32> {
    match size {
        1 => data.get(offset).copied().map(u32::from),
        2 => {
            let a = *data.get(offset)?;
            let b = *data.get(offset + 1)?;
            Some(u16::from_le_bytes([a, b]) as u32)
        }
        _ => None,
    }
}

fn expand_draw_vertices(draw: &RustDrawCommand, vertex_bytes: &[u8], index_bytes: &[u8]) -> Option<(Vec<Vertex>, Vec<u32>)> {
    if draw.indexed_stride == 0 || draw.pos_index_size == 0 || draw.color_index_size == 0 {
        return None;
    }
    if draw.pos_index_offset >= draw.indexed_stride || draw.color_index_offset >= draw.indexed_stride {
        return None;
    }
    let pos_arr = draw.array_ranges[9];
    let clr_arr = draw.array_ranges[11];
    if pos_arr.size == 0 || clr_arr.size == 0 {
        return None;
    }
    let vert_start = draw.vert_range.offset as usize;
    let vert_len = draw.vert_range.size as usize;
    if vert_start.checked_add(vert_len)? > vertex_bytes.len() {
        return None;
    }

    let stride = draw.indexed_stride as usize;
    if stride == 0 {
        return None;
    }
    let attr_sets = vert_len / stride;
    if attr_sets == 0 {
        return None;
    }

    let mut output_vertices = Vec::new();
    let mut output_indices = Vec::new();
    let source_indices: Vec<u32> = if draw.index_count > 0 && draw.idx_range.size > 0 {
        let idx_start = draw.idx_range.offset as usize;
        let idx_len = draw.idx_range.size as usize;
        if idx_len % 2 != 0 {
            return None;
        }
        if idx_start.checked_add(idx_len)? > index_bytes.len() {
            return None;
        }
        let expected = draw.index_count as usize;
        let available = idx_len / 2;
        if expected > available {
            return None;
        }
        let mut out = Vec::with_capacity(expected.max(available));
        let mut cur = idx_start;
        let end = idx_start + expected * 2;
        while cur + 1 < end {
            out.push(u16::from_le_bytes([index_bytes[cur], index_bytes[cur + 1]]) as u32);
            cur += 2;
        }
        if out.is_empty() {
            (0..attr_sets as u32).collect()
        } else {
            out
        }
    } else {
        (0..attr_sets as u32).collect()
    };

    let pos_base = pos_arr.offset as usize;
    let clr_base = clr_arr.offset as usize;
    let pos_stride = draw.pos_array_stride as usize;
    let clr_stride = draw.color_array_stride as usize;
    if pos_stride == 0 || clr_stride == 0 {
        return None;
    }

    for src_idx in source_indices {
        let attr_idx = src_idx as usize;
        if attr_idx >= attr_sets {
            continue;
        }
        let attr_base = vert_start + attr_idx * stride;
        let pos_idx =
            read_index(vertex_bytes, attr_base + draw.pos_index_offset as usize, draw.pos_index_size)?;
        let clr_idx = read_index(
            vertex_bytes,
            attr_base + draw.color_index_offset as usize,
            draw.color_index_size,
        )?;

        let pos_off = pos_base + (pos_idx as usize) * pos_stride;
        let clr_off = clr_base + (clr_idx as usize) * clr_stride;
        if pos_off + 6 > vertex_bytes.len() || clr_off + 4 > vertex_bytes.len() {
            continue;
        }
        let px = i16::from_le_bytes([vertex_bytes[pos_off], vertex_bytes[pos_off + 1]]) as f32;
        let py = i16::from_le_bytes([vertex_bytes[pos_off + 2], vertex_bytes[pos_off + 3]]) as f32;
        let pz = i16::from_le_bytes([vertex_bytes[pos_off + 4], vertex_bytes[pos_off + 5]]) as f32;
        let r = vertex_bytes[clr_off] as f32 / 255.0;
        let g = vertex_bytes[clr_off + 1] as f32 / 255.0;
        let b = vertex_bytes[clr_off + 2] as f32 / 255.0;
        let a = vertex_bytes[clr_off + 3] as f32 / 255.0;
        output_indices.push(output_vertices.len() as u32);
        output_vertices.push(Vertex {
            pos: [px, py, pz],
            color: [r, g, b, a],
        });
    }

    if output_vertices.is_empty() {
        return None;
    }
    Some((output_vertices, output_indices))
}

fn read_color_rgba8(
    cache: &[u8],
    cache_w: u32,
    cache_h: u32,
    left: u32,
    top: u32,
    width: u32,
    height: u32,
) -> Option<Vec<u8>> {
    if width == 0 || height == 0 {
        return Some(Vec::new());
    }
    if cache_w == 0 || cache_h == 0 || cache.is_empty() {
        return None;
    }
    let out_w = width.min(cache_w.saturating_sub(left));
    let out_h = height.min(cache_h.saturating_sub(top));
    let mut out = vec![0u8; (out_w * out_h * 4) as usize];
    for y in 0..out_h {
        let src_row = (top + y) * cache_w;
        let dst_row = y * out_w;
        let src_start = ((src_row + left) * 4) as usize;
        let src_end = src_start + (out_w * 4) as usize;
        let dst_start = (dst_row * 4) as usize;
        let dst_end = dst_start + (out_w * 4) as usize;
        if src_end > cache.len() || dst_end > out.len() {
            return None;
        }
        out[dst_start..dst_end].copy_from_slice(&cache[src_start..src_end]);
    }
    Some(out)
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_init() -> i32 {
    let Ok(mut state) = global_state().lock() else {
        return 0;
    };
    state.init = true;
    state.draw_commands.clear();
    state.cached_rgba.clear();
    state.cached_width = 0;
    state.cached_height = 0;
    let (capture_enabled, capture_count) = capture_config_from_env();
    state.capture_enabled = capture_enabled;
    state.capture_remaining = if capture_enabled { capture_count } else { 0 };
    state.capture_index = 0;
    state.capture_dir = default_capture_dir();
    state.debug_log_enabled = capture_enabled || debug_log_enabled_from_env();
    state.logged_draw_debug = false;
    if state.capture_enabled {
        let probe = format!("{}/porpoise_capture_probe.txt", state.capture_dir);
        write_capture_probe(&probe);
    }
    state.last_status = "ok_initialized";
    let width = state.frame_state.fb_width as u32;
    let height = state.frame_state.fb_height as u32;
    match create_gpu_state(
        state.window_handle,
        state.display_handle,
        width.max(1),
        height.max(1),
    ) {
        Ok(gpu) => state.gpu = Some(gpu),
        Err(reason) => {
            state.gpu = None;
            state.last_status = reason;
        }
    }
    1
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_shutdown() {
    if let Ok(mut state) = global_state().lock() {
        state.init = false;
        state.draw_commands.clear();
        state.pipeline_cache.clear();
        state.gpu = None;
        state.last_status = "shutdown";
    }
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_notify_state(action: u32) {
    if let Ok(mut state) = global_state().lock() {
        let idx = action as usize;
        if idx < state.action_counters.len() {
            state.action_counters[idx] = state.action_counters[idx].saturating_add(1);
        }
    }
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_set_window_info(
    native_window: u64,
    native_display: u64,
    width: u32,
    height: u32,
) {
    if let Ok(mut state) = global_state().lock() {
        let handle_changed = state.window_handle != native_window || state.display_handle != native_display;
        state.window_handle = native_window;
        state.display_handle = native_display;
        state.frame_state.fb_width = width as u16;
        state.frame_state.fb_height = height as u16;
        if handle_changed {
            state.gpu = None;
            state.pipeline_cache.clear();
        }
        state.last_status = if native_window != 0 {
            "window_info_received"
        } else {
            "window_info_missing"
        };
    }
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_last_status() -> *const core::ffi::c_char {
    static mut STATUS_BUF: [u8; 64] = [0; 64];
    let status = if let Ok(state) = global_state().lock() {
        state.last_status
    } else {
        "status_lock_failed"
    };
    let bytes = status.as_bytes();
    // SAFETY: static buffer access is serialized by this function usage from C++ render thread.
    unsafe {
        STATUS_BUF.fill(0);
        let copy_len = bytes.len().min(STATUS_BUF.len() - 1);
        STATUS_BUF[..copy_len].copy_from_slice(&bytes[..copy_len]);
        STATUS_BUF.as_ptr() as *const core::ffi::c_char
    }
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_begin_frame(frame_state: *const RustFrameState) -> i32 {
    let Ok(mut state) = global_state().lock() else {
        return 0;
    };
    if !state.init {
        return 0;
    }
    if !frame_state.is_null() {
        // SAFETY: caller owns pointer validity for this call.
        state.frame_state = unsafe { *frame_state };
    }
    state.draw_commands.clear();
    let width = state.frame_state.fb_width as u32;
    let height = state.frame_state.fb_height as u32;
    match ensure_gpu_size(&mut state, width.max(1), height.max(1)) {
        Ok(()) => {
            state.last_status = "ok_begin_frame";
            1
        }
        Err(reason) => {
            state.last_status = reason;
            0
        }
    }
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_push_draw(draw: *const RustDrawCommand) {
    let Ok(mut state) = global_state().lock() else {
        return;
    };
    if !state.init || draw.is_null() {
        return;
    }
    // SAFETY: caller owns pointer validity for this call.
    let draw = unsafe { *draw };
    state.draw_commands.push(draw);
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_render(
    vertex_buffer: *const u8,
    vertex_size: u32,
    index_buffer: *const u8,
    index_size: u32,
    frame_state: *const RustFrameState,
) -> i32 {
    let Ok(mut state) = global_state().lock() else {
        return 0;
    };
    if !state.init {
        return 0;
    }
    if !frame_state.is_null() {
        // SAFETY: caller owns pointer validity for this call.
        state.frame_state = unsafe { *frame_state };
    }
    let width = state.frame_state.fb_width as u32;
    let height = state.frame_state.fb_height as u32;
    if ensure_gpu_size(&mut state, width.max(1), height.max(1)).is_err() {
        state.last_status = "no_gpu";
        return 0;
    }
    if vertex_buffer.is_null() || vertex_size == 0 {
        if state.debug_log_enabled {
            append_debug_log(
                &state.capture_dir,
                &format!(
                    "render decline: no_vertex_data draw_cmds={} vb={} ib={}",
                    state.draw_commands.len(),
                    vertex_size,
                    index_size
                ),
            );
        }
        dump_reason_png(&mut state, "no_vertex_data", [255, 0, 0, 255]);
        state.last_status = "no_vertex_data";
        return 0;
    }
    // SAFETY: C++ passes valid pointers for this call duration.
    let vertex_bytes = unsafe { std::slice::from_raw_parts(vertex_buffer, vertex_size as usize) };
    let index_bytes = if index_buffer.is_null() || index_size == 0 {
        &[]
    } else {
        // SAFETY: C++ passes valid pointers for this call duration.
        unsafe { std::slice::from_raw_parts(index_buffer, index_size as usize) }
    };

    let Some(gpu) = state.gpu.as_ref() else {
        if state.debug_log_enabled {
            append_debug_log(&state.capture_dir, "render decline: no_gpu_after_size");
        }
        state.last_status = "no_gpu";
        return 0;
    };
    let gpu_width = gpu.width;
    let gpu_height = gpu.height;
    let device = gpu.device.clone();
    let queue = gpu.queue.clone();
    let shader = gpu.shader.clone();
    let vertex_layout = gpu.vertex_layout.clone();
    let transform_layout = gpu.transform_layout.clone();
    let surface_format = gpu.surface_format;
    let depth_view = gpu.depth_view.clone();
    let output = match gpu.surface.get_current_texture() {
        Ok(frame) => frame,
        Err(_) => {
            gpu.surface.configure(&gpu.device, &gpu.config);
            match gpu.surface.get_current_texture() {
                Ok(frame) => frame,
                Err(_) => {
                    if state.debug_log_enabled {
                        append_debug_log(&state.capture_dir, "render decline: surface_acquire_failed");
                    }
                    state.last_status = "surface_acquire_failed";
                    return 0;
                }
            }
        }
    };
    let color_view = output
        .texture
        .create_view(&wgpu::TextureViewDescriptor::default());
    let bytes_per_row = gpu_width * 4;
    let aligned = bytes_per_row.next_multiple_of(256);
    let staging_size = aligned as u64 * gpu_height as u64;
    let staging = device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("porpoise-rust-present-readback"),
        size: staging_size,
        usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
        mapped_at_creation: false,
    });
    let mut encoder = device
        .create_command_encoder(&wgpu::CommandEncoderDescriptor::default());
    let mut drew_any = false;
    let mut pipeline_cache = std::mem::take(&mut state.pipeline_cache);
    {
        let mut pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
            label: Some("porpoise-rust-pass"),
            color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                view: &color_view,
                depth_slice: None,
                resolve_target: None,
                ops: wgpu::Operations {
                    load: wgpu::LoadOp::Clear(wgpu::Color {
                        r: state.frame_state.clear_color[0] as f64,
                        g: state.frame_state.clear_color[1] as f64,
                        b: state.frame_state.clear_color[2] as f64,
                        a: state.frame_state.clear_color[3] as f64,
                    }),
                    store: wgpu::StoreOp::Store,
                },
            })],
            depth_stencil_attachment: Some(wgpu::RenderPassDepthStencilAttachment {
                view: &depth_view,
                depth_ops: Some(wgpu::Operations {
                    load: wgpu::LoadOp::Clear(1.0), // mirrored Z: 0=near, 1=far; clear to far
                    store: wgpu::StoreOp::Store,
                }),
                stencil_ops: None,
            }),
            timestamp_writes: None,
            occlusion_query_set: None,
        });

        pass.set_viewport(0.0, 0.0, gpu_width as f32, gpu_height as f32, 0.0, 1.0);

        // #region agent log
        {
            use std::io::Write;
            let ts = std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH).map(|d| d.as_millis()).unwrap_or(0);
            let line = format!(
                "{{\"sessionId\":\"496880\",\"runId\":\"post-fix\",\"hypothesisId\":\"H4-H5\",\"location\":\"lib.rs:render_pass\",\"timestamp\":{},\"message\":\"viewport_and_draws\",\"data\":{{\"viewport_w\":{},\"viewport_h\":{},\"draw_commands\":{}}}}}\n",
                ts, gpu_width, gpu_height, state.draw_commands.len()
            );
            if let Ok(mut f) = std::fs::OpenOptions::new().create(true).append(true).open("debug-496880.log") {
                let _ = f.write_all(line.as_bytes());
            }
        }
        // #endregion

        let draw_commands = state.draw_commands.clone();
        for draw in &draw_commands {
            if state.frame_state.cull_mode == GX_CULL_ALL {
                continue;
            }
            let Some(topology) = map_primitive_topology(draw.primitive) else {
                continue;
            };
            let Some((verts, inds)) = expand_draw_vertices(draw, vertex_bytes, index_bytes) else {
                continue;
            };
            if verts.is_empty() || inds.is_empty() {
                continue;
            }
            let proj = proj_flip_y(&state.frame_state.proj);
            // GX quads/triangles use CCW front face; only detect for strips.
            let front_face = match topology {
                wgpu::PrimitiveTopology::TriangleStrip | wgpu::PrimitiveTopology::LineStrip => {
                    detect_front_face(&verts, &inds, &draw.model_view, &proj)
                }
                _ => wgpu::FrontFace::Ccw,
            };
            if !state.logged_draw_debug {
                let sample = verts[0];
                let world = mul_mat4_vec4_col_major(
                    &draw.model_view,
                    [sample.pos[0], sample.pos[1], sample.pos[2], 1.0],
                );
                let mut clip = mul_mat4_vec4_col_major(&proj, world);
                clip[2] = clip[2] + clip[3]; // mirrored Z: 0=near, 1=far (matches shader)
                let ndc = if clip[3].abs() > f32::EPSILON {
                    [clip[0] / clip[3], clip[1] / clip[3], clip[2] / clip[3]] // Y flip in proj
                } else {
                    [0.0, 0.0, 0.0]
                };
                let ndc_gx_depth = ndc[2];
                let mut in_ndc = 0usize;
                let sample_count = verts.len().min(128);
                for vtx in verts.iter().take(sample_count) {
                    let w = mul_mat4_vec4_col_major(
                        &draw.model_view,
                        [vtx.pos[0], vtx.pos[1], vtx.pos[2], 1.0],
                    );
                    let mut c = mul_mat4_vec4_col_major(&proj, w);
                    c[2] = c[2] + c[3]; // mirrored Z (matches shader)
                    if c[3].abs() > f32::EPSILON {
                        let x = c[0] / c[3];
                        let y = c[1] / c[3]; // Y flip in proj
                        let z = c[2] / c[3];
                        if (-1.0..=1.0).contains(&x) && (-1.0..=1.0).contains(&y) && (0.0..=1.0).contains(&z) {
                            in_ndc += 1;
                        }
                    }
                }
                if state.debug_log_enabled {
                    append_debug_log(
                        &state.capture_dir,
                        &format!(
                            "draw_debug verts={} inds={} pos0=({:.2},{:.2},{:.2}) clr0=({:.2},{:.2},{:.2},{:.2}) ndc0=({:.2},{:.2},{:.2}) in_ndc_sample={}/{}",
                            verts.len(),
                            inds.len(),
                            sample.pos[0],
                            sample.pos[1],
                            sample.pos[2],
                            sample.color[0],
                            sample.color[1],
                            sample.color[2],
                            sample.color[3],
                            ndc[0],
                            ndc[1],
                            ndc[2],
                            in_ndc,
                            sample_count
                        ),
                    );
                }
                // #region agent log
                {
                    use std::io::Write;
                    let ts = std::time::SystemTime::now()
                        .duration_since(std::time::UNIX_EPOCH).map(|d| d.as_millis()).unwrap_or(0);
                    let entry = format!(
                        concat!(
                            "{{\"sessionId\":\"496880\",\"runId\":\"post-fix\",\"hypothesisId\":\"H1-H2-H3\",",
                            "\"location\":\"lib.rs:draw_debug\",\"timestamp\":{},",
                            "\"message\":\"matrices_and_ndc\",\"data\":{{",
                            "\"y_flip_in_shader\":true,",
                            "\"pos0\":[{:.3},{:.3},{:.3}],",
                            "\"view_pos\":[{:.3},{:.3},{:.3},{:.3}],",
                            "\"ndc_current\":[{:.3},{:.3},{:.3}],",
                            "\"ndc_gx_depth_z\":{:.3},",
                            "\"in_ndc_sample\":{}/{},",
                            "\"cull_mode\":{},\"front_face_cw\":false}}}}\n"
                        ),
                        ts,
                        sample.pos[0], sample.pos[1], sample.pos[2],
                        world[0], world[1], world[2], world[3],
                        ndc[0], ndc[1], ndc[2],
                        ndc_gx_depth,
                        in_ndc, sample_count,
                        state.frame_state.cull_mode
                    );
                    if let Ok(mut f) = std::fs::OpenOptions::new().create(true).append(true)
                        .open("E:/GPT5/Porpoise_SDK/debug-496880.log") {
                        let _ = f.write_all(entry.as_bytes());
                    }
                }
                // #endregion
                state.logged_draw_debug = true;
            }
            let key = encode_pipeline_key_with_front_face(&state.frame_state, topology, front_face);
            let pipeline = if let Some(p) = pipeline_cache.get(&key) {
                p
            } else {
                let p = build_pipeline(
                    &device,
                    &shader,
                    &transform_layout,
                    &vertex_layout,
                    surface_format,
                    &state.frame_state,
                    topology,
                    front_face,
                );
                pipeline_cache.insert(key, p);
                pipeline_cache.get(&key).unwrap()
            };

            let vb = device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("porpoise-rust-vb"),
                size: (verts.len() * std::mem::size_of::<Vertex>()) as u64,
                usage: wgpu::BufferUsages::VERTEX | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            });
            queue.write_buffer(&vb, 0, bytemuck::cast_slice(&verts));
            let ib = device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("porpoise-rust-ib"),
                size: (inds.len() * std::mem::size_of::<u32>()) as u64,
                usage: wgpu::BufferUsages::INDEX | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            });
            queue.write_buffer(&ib, 0, bytemuck::cast_slice(&inds));

            let transform_data = TransformData {
                proj,
                model_view: draw.model_view,
            };
            let transform_buffer = device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("porpoise-rust-transform-buffer"),
                size: std::mem::size_of::<TransformData>() as u64,
                usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            });
            queue.write_buffer(&transform_buffer, 0, bytemuck::bytes_of(&transform_data));
            let transform_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
                label: Some("porpoise-rust-transform-bind-group"),
                layout: &transform_layout,
                entries: &[wgpu::BindGroupEntry {
                    binding: 0,
                    resource: transform_buffer.as_entire_binding(),
                }],
            });

            pass.set_pipeline(pipeline);
            pass.set_bind_group(0, &transform_bind_group, &[]);
            pass.set_vertex_buffer(0, vb.slice(..));
            pass.set_index_buffer(ib.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..inds.len() as u32, 0, 0..1);
            drew_any = true;
        }
    }
    state.pipeline_cache = pipeline_cache;
    encoder.copy_texture_to_buffer(
        wgpu::TexelCopyTextureInfo {
            texture: &output.texture,
            mip_level: 0,
            origin: wgpu::Origin3d::ZERO,
            aspect: wgpu::TextureAspect::All,
        },
        wgpu::TexelCopyBufferInfo {
            buffer: &staging,
            layout: wgpu::TexelCopyBufferLayout {
                offset: 0,
                bytes_per_row: Some(aligned),
                rows_per_image: Some(gpu_height),
            },
        },
        wgpu::Extent3d {
            width: gpu_width,
            height: gpu_height,
            depth_or_array_layers: 1,
        },
    );
    queue.submit([encoder.finish()]);
    let slice = staging.slice(..);
    let (tx, rx) = std::sync::mpsc::channel();
    slice.map_async(wgpu::MapMode::Read, move |res| {
        let _ = tx.send(res.is_ok());
    });
    let _ = device.poll(wgpu::PollType::Wait);
    if !rx.recv().ok().unwrap_or(false) {
        if state.capture_enabled && state.capture_remaining > 0 {
            let path = format!(
                "{}/porpoise_capture_error_{:03}.png",
                state.capture_dir, state.capture_index
            );
            dump_debug_error_png(&path);
            state.capture_index = state.capture_index.saturating_add(1);
            state.capture_remaining = state.capture_remaining.saturating_sub(1);
        }
        state.last_status = "readback_map_failed";
        if state.debug_log_enabled {
            append_debug_log(&state.capture_dir, "render decline: readback_map_failed");
        }
        return 0;
    }
    let mapped = slice.get_mapped_range();
    state.cached_rgba.resize((gpu_width * gpu_height * 4) as usize, 0);
    for row in 0..gpu_height as usize {
        let src_start = row * aligned as usize;
        let src_end = src_start + bytes_per_row as usize;
        let dst_start = row * bytes_per_row as usize;
        let dst_end = dst_start + bytes_per_row as usize;
        if src_end <= mapped.len() && dst_end <= state.cached_rgba.len() {
            state.cached_rgba[dst_start..dst_end].copy_from_slice(&mapped[src_start..src_end]);
        }
    }
    drop(mapped);
    staging.unmap();
    if matches!(surface_format, wgpu::TextureFormat::Bgra8Unorm | wgpu::TextureFormat::Bgra8UnormSrgb) {
        for px in state.cached_rgba.chunks_exact_mut(4) {
            px.swap(0, 2);
        }
    }
    state.cached_width = gpu_width;
    state.cached_height = gpu_height;
    if state.capture_enabled && state.capture_remaining > 0 && !state.cached_rgba.is_empty() {
        let path = format!(
            "{}/porpoise_capture_{:03}.png",
            state.capture_dir, state.capture_index
        );
        if dump_capture_png(&path, &state.cached_rgba, gpu_width, gpu_height).is_ok() {
            state.capture_index = state.capture_index.saturating_add(1);
            state.capture_remaining = state.capture_remaining.saturating_sub(1);
            state.last_status = "ok_render_present_capture";
        } else {
            state.last_status = "capture_png_write_failed";
        }
    }
    output.present();
    if state.last_status != "ok_render_present_capture" && state.last_status != "capture_png_write_failed" {
        state.last_status = if drew_any {
            "ok_render_present"
        } else {
            "ok_render_clear_only"
        };
    }
    if state.debug_log_enabled {
        append_debug_log(
            &state.capture_dir,
            &format!(
                "render handled: status={} draw_cmds={} drew_any={} vb={} ib={} cached={}x{}",
                state.last_status,
                state.draw_commands.len(),
                drew_any,
                vertex_size,
                index_size,
                state.cached_width,
                state.cached_height
            ),
        );
    }
    1
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_copy_disp(
    dest: *mut core::ffi::c_void,
    width: u16,
    height: u16,
    _clear: u8,
    _frame_state: *const RustFrameState,
) -> i32 {
    let Ok(mut state) = global_state().lock() else {
        return 0;
    };
    if dest.is_null() {
        state.last_status = "copy_disp_no_dest";
        return 0;
    }
    let w = width as u32;
    let h = height as u32;
    let Some(rgba) = read_color_rgba8(
        &state.cached_rgba,
        state.cached_width,
        state.cached_height,
        0,
        0,
        w,
        h,
    ) else {
        state.last_status = "copy_disp_readback_failed";
        return 0;
    };
    // SAFETY: destination is owned by caller and expected to fit width*height u16 pixels.
    let out = unsafe { std::slice::from_raw_parts_mut(dest as *mut u16, (width as usize) * (height as usize)) };
    let rw = w.min(state.cached_width) as usize;
    let rh = h.min(state.cached_height) as usize;
    for y in 0..rh {
        let src_y = rh.saturating_sub(1).saturating_sub(y);
        for x in 0..rw {
            let src_idx = (src_y * rw + x) * 4;
            if src_idx + 2 >= rgba.len() {
                continue;
            }
            let r = rgba[src_idx];
            let g = rgba[src_idx + 1];
            let b = rgba[src_idx + 2];
            out[y * width as usize + x] =
                (((r as u16) >> 3) << 11) | (((g as u16) >> 2) << 5) | ((b as u16) >> 3);
        }
    }
    state.last_status = "ok_copy_disp";
    1
}

#[no_mangle]
pub extern "C" fn porpoise_rust_renderer_copy_tex(
    dest: *mut core::ffi::c_void,
    left: u16,
    top: u16,
    width: u16,
    height: u16,
    _fmt: u32,
    _clear: u8,
    _frame_state: *const RustFrameState,
) -> i32 {
    let Ok(mut state) = global_state().lock() else {
        return 0;
    };
    if dest.is_null() {
        state.last_status = "copy_tex_no_dest";
        return 0;
    }
    let w = width as u32;
    let h = height as u32;
    let Some(rgba) = read_color_rgba8(
        &state.cached_rgba,
        state.cached_width,
        state.cached_height,
        left as u32,
        top as u32,
        w,
        h,
    ) else {
        state.last_status = "copy_tex_readback_failed";
        return 0;
    };
    let out_len = (width as usize) * (height as usize) * 4;
    // SAFETY: destination points to writable memory used by GX texture copy flow.
    let out = unsafe { std::slice::from_raw_parts_mut(dest as *mut u8, out_len) };
    let copy_len = out.len().min(rgba.len());
    out[..copy_len].copy_from_slice(&rgba[..copy_len]);
    state.last_status = "ok_copy_tex";
    1
}
