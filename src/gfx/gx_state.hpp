#pragma once

#include "math.hpp"
#include "texture.hpp"

#include <array>
#include <bitset>
#include <dolphin/gx.h>
#include <optional>
#include <unordered_map>
#include <variant>

namespace GX {
constexpr u8 MaxLights = 8;
using LightMask = std::bitset<MaxLights>;
} // namespace GX

namespace porpoise::gfx::gx {

using ::porpoise::gfx::ClipRect;
using ::porpoise::gfx::TextureBind;
using ::porpoise::gfx::TextureHandle;

inline constexpr float GX_LARGE_NUMBER = -1048576.0f;
inline constexpr float M_PIF = 3.14159265358979323846f;

using TexMtxVariant = std::variant<std::monostate, porpoise::Mat2x4<float>, porpoise::Mat3x4<float>>;

struct GXTexObj_ {
  const void* data = nullptr;
  u16 width = 0;
  u16 height = 0;
  u32 fmt = 0;
  GXTexWrapMode wrapS = GX_CLAMP;
  GXTexWrapMode wrapT = GX_CLAMP;
  GXBool hasMips = GX_FALSE;
  GXTexFilter minFilter = GX_LINEAR;
  GXTexFilter magFilter = GX_LINEAR;
  float minLod = 0.f;
  float maxLod = 0.f;
  float lodBias = 0.f;
  GXBool biasClamp = GX_FALSE;
  GXBool doEdgeLod = GX_FALSE;
  GXAnisotropy maxAniso = GX_ANISO_1;
  GXTlut tlut = GX_TLUT0;
  TextureHandle* ref = nullptr;
  bool dataInvalidated = true;
};

struct GXTlutObj_ {
  const void* data = nullptr;
  u16 numEntries = 0;
  GXTlutFmt fmt = GX_TL_IA8;
  TextureHandle* ref = nullptr;
};

struct GXLightObj_ {
  GXColor color{0, 0, 0, 0};
  float a0 = 1.f;
  float a1 = 0.f;
  float a2 = 0.f;
  float k0 = 1.f;
  float k1 = 0.f;
  float k2 = 0.f;
  float px = 0.f;
  float py = 0.f;
  float pz = 0.f;
  float nx = 0.f;
  float ny = 0.f;
  float nz = 0.f;
};

struct AttrArray {
  const void* data = nullptr;
  u32 size = 0;
  u8 stride = 0;
  porpoise::Range cachedRange{};
};

struct TevPassColor {
  GXTevColorArg a = GX_CC_ZERO;
  GXTevColorArg b = GX_CC_ZERO;
  GXTevColorArg c = GX_CC_ZERO;
  GXTevColorArg d = GX_CC_ZERO;
};

struct TevPassAlpha {
  GXTevAlphaArg a = GX_CA_ZERO;
  GXTevAlphaArg b = GX_CA_ZERO;
  GXTevAlphaArg c = GX_CA_ZERO;
  GXTevAlphaArg d = GX_CA_ZERO;
};

struct TevOp {
  GXTevOp op = GX_TEV_ADD;
  GXTevBias bias = GX_TB_ZERO;
  GXTevScale scale = GX_CS_SCALE_1;
  GXTevRegID outReg = GX_TEVPREV;
  bool clamp = true;
};

struct TevSwap {
  GXTevColorChan red = GX_CH_RED;
  GXTevColorChan green = GX_CH_GREEN;
  GXTevColorChan blue = GX_CH_BLUE;
  GXTevColorChan alpha = GX_CH_ALPHA;
};

struct AlphaCompare {
  GXCompare comp0 = GX_ALWAYS;
  u32 ref0 = 0;
  GXAlphaOp op = GX_AOP_AND;
  GXCompare comp1 = GX_ALWAYS;
  u32 ref1 = 0;
};

struct TcgConfig {
  GXTexGenType type = GX_TG_MTX2x4;
  GXTexGenSrc src = GX_TG_POS;
  GXTexMtx mtx = GX_IDENTITY;
  GXPTTexMtx postMtx = GX_PTIDENTITY;
  GXBool normalize = GX_FALSE;
};

struct IndStage {
  GXTexCoordID texCoordId = GX_TEXCOORD_NULL;
  GXTexMapID texMapId = GX_TEXMAP_NULL;
  GXIndTexScale scaleS = GX_ITS_1;
  GXIndTexScale scaleT = GX_ITS_1;
};

struct IndTexMtxInfo {
  porpoise::Mat3x2<float> mtx{};
  s8 scaleExp = 0;
};

struct VtxAttrFmt {
  GXCompCnt cnt = GX_POS_XYZ;
  GXCompType type = GX_RGB565;
  u8 frac = 0;
};

struct VtxFmt {
  std::array<VtxAttrFmt, GX_VA_MAX_ATTR> attrs{};
};

struct PnMtx {
  porpoise::Mat3x4<float> pos;
  porpoise::Mat3x4<float> nrm;
};

struct Light {
  porpoise::Vec4<float> pos{0.f, 0.f, 0.f, 1.f};
  porpoise::Vec4<float> dir{0.f, 0.f, 0.f, 0.f};
  porpoise::Vec4<float> color{0.f, 0.f, 0.f, 0.f};
  porpoise::Vec4<float> cosAtt{0.f, 0.f, 0.f, 0.f};
  porpoise::Vec4<float> distAtt{0.f, 0.f, 0.f, 0.f};
};

struct ColorChannelConfig {
  GXColorSrc matSrc = GX_SRC_REG;
  GXColorSrc ambSrc = GX_SRC_REG;
  GXDiffuseFn diffFn = GX_DF_NONE;
  GXAttnFn attnFn = GX_AF_NONE;
  bool lightingEnabled = false;
};

struct ColorChannelState {
  porpoise::Vec4<float> matColor{0.f, 0.f, 0.f, 1.f};
  porpoise::Vec4<float> ambColor{0.f, 0.f, 0.f, 1.f};
  GX::LightMask lightMask{};
};

struct FogState {
  GXFogType type = GX_FOG_NONE;
  float startZ = 0.f;
  float endZ = 0.f;
  float nearZ = 0.f;
  float farZ = 0.f;
  porpoise::Vec4<float> color{0.f, 0.f, 0.f, 1.f};
  bool rangeAdjustEnabled = false;
  u16 rangeAdjustCenter = 0;
  std::array<u16, 10> rangeAdjustTable{};
};

struct TevStage {
  TevPassColor colorPass{};
  TevPassAlpha alphaPass{};
  TevOp colorOp{};
  TevOp alphaOp{};
  GXTevKColorSel kcSel = GX_TEV_KCSEL_1;
  GXTevKAlphaSel kaSel = GX_TEV_KASEL_1;
  GXTexCoordID texCoordId = GX_TEXCOORD_NULL;
  GXTexMapID texMapId = GX_TEXMAP_NULL;
  GXChannelID channelId = GX_COLOR_NULL;
  GXTevSwapSel tevSwapRas = GX_TEV_SWAP0;
  GXTevSwapSel tevSwapTex = GX_TEV_SWAP0;
  GXIndTexStageID indTexStage = GX_INDTEXSTAGE0;
  GXIndTexFormat indTexFormat = GX_ITF_8;
  GXIndTexBiasSel indTexBiasSel = GX_ITB_NONE;
  GXIndTexAlphaSel indTexAlphaSel = GX_ITBA_OFF;
  GXIndTexMtxID indTexMtxId = GX_ITM_OFF;
  GXIndTexWrap indTexWrapS = GX_ITW_OFF;
  GXIndTexWrap indTexWrapT = GX_ITW_OFF;
  bool indTexUseOrigLOD = false;
  bool indTexAddPrev = false;
};

struct GXState {
  std::array<PnMtx, 10> pnMtx{};
  u32 currentPnMtx = 0;
  porpoise::Mat4x4<float> proj{};
  GXProjectionType projType = GX_PERSPECTIVE;
  FogState fog{};
  GXCullMode cullMode = GX_CULL_BACK;
  GXBlendMode blendMode = GX_BM_NONE;
  GXBlendFactor blendFacSrc = GX_BL_SRCALPHA;
  GXBlendFactor blendFacDst = GX_BL_INVSRCALPHA;
  GXLogicOp blendOp = GX_LO_CLEAR;
  GXCompare depthFunc = GX_LEQUAL;
  porpoise::Vec4<float> clearColor{0.f, 0.f, 0.f, 1.f};
  u32 dstAlpha = UINT32_MAX;
  AlphaCompare alphaCompare{};
  std::array<porpoise::Vec4<float>, 4> colorRegs{};
  std::array<porpoise::Vec4<float>, GX_MAX_KCOLOR> kcolors{};
  std::array<ColorChannelConfig, 4> colorChannelConfig{};
  std::array<ColorChannelState, 4> colorChannelState{};
  std::array<Light, GX::MaxLights> lights{};
  std::array<TevStage, GX_MAX_TEVSTAGE> tevStages{};
  std::array<TextureBind, GX_MAX_TEXMAP> textures{};
  std::array<GXTlutObj_, 20> tluts{};
  std::array<TexMtxVariant, 10> texMtxs{};
  std::array<porpoise::Mat3x4<float>, 20> ptTexMtxs{};
  std::array<TcgConfig, GX_MAX_TEXCOORD> tcgs{};
  std::array<GXAttrType, GX_VA_MAX_ATTR> vtxDesc{};
  std::array<VtxFmt, GX_MAX_VTXFMT> vtxFmts{};
  std::array<TevSwap, GX_MAX_TEVSWAP> tevSwapTable{};
  std::array<IndStage, GX_MAX_INDTEXSTAGE> indStages{};
  std::array<IndTexMtxInfo, 3> indTexMtxs{};
  std::array<AttrArray, GX_VA_MAX_ATTR> arrays{};
  ClipRect texCopySrc{};
  GXTexFmt texCopyFmt = GX_TF_I4;
  std::unordered_map<const void*, TextureHandle*> copyTextures;
  std::optional<porpoise::ByteBuffer> dynamicDlBuf;
  u32 scissorLeft = 0;
  u32 scissorTop = 0;
  u32 scissorWd = 0;
  u32 scissorHt = 0;
  float viewportLeft = 0.f;
  float viewportTop = 0.f;
  float viewportWidth = 0.f;
  float viewportHeight = 0.f;
  float viewportNear = 0.f;
  float viewportFar = 1.f;
  bool depthCompare = true;
  bool depthUpdate = true;
  bool colorUpdate = true;
  bool alphaUpdate = true;
  u8 numChans = 0;
  u8 numIndStages = 0;
  u8 numTevStages = 0;
  u8 numTexGens = 0;
  bool stateDirty = true;
};

extern GXState g_gxState;

} // namespace porpoise::gfx::gx

namespace porpoise::gfx::gx {
using porpoise::gfx::gx::GXState;
using porpoise::gfx::gx::g_gxState;
using porpoise::gfx::gx::GXTexObj_;
using porpoise::gfx::gx::GXTlutObj_;
using porpoise::gfx::gx::GXLightObj_;
using porpoise::gfx::gx::VtxAttrFmt;
using porpoise::gfx::gx::Light;
}

