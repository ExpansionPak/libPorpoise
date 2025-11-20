/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     spheremap.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/spheremap.c $
    
    5     11/01/00 4:26p Carl
    Removed use of clamping.  Fixed TFAN drawing.
    Changed choices for what can be displayed.
    
    4     5/23/00 2:33p Alligator
    fixed warning
    
    3     5/21/00 10:51p Alligator
    fixed clamp mode, cleaned up code
    
    2     5/17/00 9:23p Alligator
    fixed warning
    
    1     5/12/00 4:48p Alligator
    initial checkin
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

//
//  Functions to convert a cube map into a sphere map
//

/*>*******************************(*)*******************************<*/

// can be used by the app to show only certain cube faces
u8 CubeFaceStart  = 0;
u8 CubeFaceEnd    = 6;

u8 CubeTevMode    = 0; // 0 = show final res, 1 = show ras alpha, 2 = show tex color

//
//  used to rotate the projection for each face of the cube
//
static f32 angle1[6] = {90.0F, 180.0F, 270.0F, 0.0F, 90.0F, -90.0F};
static char axis1[6] = { 'y',    'y',    'y',  'y',   'x',    'x'};

//
//  used for the top and bottom faces
//
static f32 angle2[6]   = {0.0F, 0.0F, 0.0F, 0.0F, 180.0F, 180.0F};

/*>*******************************(*)*******************************<*/
void genMapSphere        ( void**    display_list, 
                           u32*      size, 
                           u16       tess,
                           GXVtxFmt  fmt );

void drawSphereMap       ( GXTexObj* cubemap, 
                           GXTexObj* spheremap,
                           void*     dl, 
                           u32       dlsz );
/*>*******************************(*)*******************************<*/


/*---------------------------------------------------------------------------*
    Name:           genMapSphere

    Description:    Initialize hemisphere geometry display list.

                    This function only generates geometry for a hemisphere
                    since it is always facing the viewpoint (viewed from the
                    north pole).  Computes a position on the hemisphere 
                    and a normal that is really the reflection vector. 
                    The reflection vector, assuming the eye point is
                    (0, 0, 1) is related to the normal by:
                    Rx = 2NxNz, Ry = 2NyNz, Rz = 2NzNz - 1

                    The reflection vectors will be projected onto
                    each face of a cube-map to generate texture coordinates.
                    See drawSphereMap for further details.

                    This function creates a display list which will be called
                    multiple times by drawSphereMap.  The Mac emulator version
                    calls the immediate mode code repeatedly since the display
                    list functionality is not fully emulated.

                    amount of buffer required? 
                        ((tess+1) + (tess-2)*(tess+1)*2) * 6 * sizeof()
                        (rounded up to 32B).

    Arguments:      tess = amount of tesselation desired
                    fmt  = vertex format to use for geometry

    Returns:        display_list = pointer to the display list
                    size         = size of the display list
 *---------------------------------------------------------------------------*/
#define M_PI  3.14159265F

void genMapSphere( void **display_list, u32 *size, u16 tess, GXVtxFmt fmt) 
{
#ifdef MAC
#pragma unused(display_list, size)
#endif // MAC
    f32 r = 1.0f, r1, r2, z1, z2;
    f32 n1x, n1y, n1z;
    f32 n2x, n2y, n2z;
    f32 theta, phi;
    u16 nlon = tess, nlat = tess;
    s32 i, j;
    u32 dl_sz = ((tess+1) + (tess-2)*(tess+1)*2) * 6 * sizeof(f32); // bytes

    
    dl_sz = OSRoundUp32B(dl_sz);

#ifndef MAC
    *display_list = (void *)NULL;
    *display_list = (void *)OSAlloc(dl_sz);
    GXBeginDisplayList(*display_list, dl_sz);
#endif

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);

    // could maybe get away with 8-bits?
    GXSetVtxAttrFmt(fmt, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(fmt, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);


    //
    //  Draw a fan for the pole
    //
#ifndef BUG_TRIANGLE_FAN
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT7, (u16)(nlon+2));
        theta = M_PI*1.0f/nlat;
        r2 = r*sinf(theta); 
        z2 = r*cosf(theta);
        n2z = 2*z2*z2-1.0F;
        n1z = 2*r*r-1.0F;
        GXPosition3f32(0.0F, 0.0F, 1.0F);
        GXNormal3f32(0.0F, 0.0F, n1z); // refl vector
        for (j = 0, phi = 0.0f; j <= nlon; j++, phi = -2*M_PI*j/nlon) {
            n2x = r2*cosf(phi);
            n2y = r2*sinf(phi);
            GXPosition3f32(n2x, n2y, z2);
            GXNormal3f32(2*n2x*z2, 2*n2y*z2, n2z); // refl vector
        }
    GXEnd();
#else
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (u16)((nlon+1)*2));
        theta = M_PI*1.0f/nlat;
        r2 = r*sinf(theta); 
        z2 = r*cosf(theta);
        n2z = 2*z2*z2-1.0F;
        n1z = 2*r*r-1.0F;
        for (j = 0, phi = 0.0f; j <= nlon; j++, phi = 2*M_PI*j/nlon) {
            n2x = r2*cosf(phi);
            n2y = r2*sinf(phi);
            GXPosition3f32(n2x, n2y, z2);
            GXNormal3f32(2*n2x*z2, 2*n2y*z2, n2z); // refl vector
            GXPosition3f32(0.0f, 0.0f, r);
            GXNormal3f32(0.0f, 0.0f, n1z); // refl vector
        }
    GXEnd();
#endif // BUG_TRIANGLE_FAN

    //
    //  Draw circular strips down to the equator
    //
    for (i = 2; i < nlat; i++) 
    {
        theta = M_PI*i/nlat;
        r1 = r*sinf(M_PI*(i-1)/nlat); 
        z1 = r*cosf(M_PI*(i-1)/nlat);
        r2 = r*sinf(theta); 
        z2 = r*cosf(theta);

        n1z = 2*z1*z1-1.0F; // reflection vector z
        n2z = 2*z2*z2-1.0F; // reflection vector z

        // only render hemisphere, quit at equator
        if (fabs(z1) < 0.01f || fabs(z2) < 0.01f)
        {
            break;
        }

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (u16)((nlon+1)*2));
            for (j = 0, phi = 0.0f; j <= nlon; j++, phi = 2*M_PI*j/nlon) 
            {
                n2x = r2*cosf(phi);
                n2y = r2*sinf(phi);
                GXPosition3f32(n2x, n2y, z2);
                GXNormal3f32(2*n2x*z2, 2*n2y*z2, n2z); // refl vector
                n1x = r1*cosf(phi); 
                n1y = r1*sinf(phi); 
                GXPosition3f32(n1x, n1y, z1);
                GXNormal3f32(2*n1x*z1, 2*n1y*z1, n1z); // refl vector
            }
        GXEnd();
    }

#ifndef MAC
    *size = GXEndDisplayList();

    if (*size > dl_sz) 
    {
        OSReport("Error allocating display list (%d, %d)\n", dl_sz, *size);
        OSHalt("Exiting");
    }

#else
    //*size = dl_sz;
#endif // MAC
}


/*---------------------------------------------------------------------------*
    Name:           drawSphereMap

    Description:    Creates a sphere map from a cube map.

                    Projects the reflection vectors of a hemisphere onto
                    each face of the cube map.  The projection is clipped
                    using vertex lighting so that only the forward projection 
                    is visible.  Each cube-face texture should have a border
                    of alpha=0 to avoid streaking outside the desired 
                    projection.

                    The hemisphere geometry is created using genMapSphere and
                    is in form of a display list that gets called once for each
                    cube face.  The display list only needs to be created once.

                    This function trashes some state, like viewport, light 0,
                    texmap 0, tev stage 0, ...

    Arguments:      cubemap - array of six GXTexObj's which describe the
                              cube map.  Each can be a different size if
                              desired.

                    spheremap - texture object that describes the spheremap.
                                This function will query the width, height,
                                and format to create the spheremap.  The 
                                calling function should allocate the texture
                                image memory before calling this function.

                    dl - display list of sphere geometry.

                    dlsz - display list size in bytes.

    Returns:        none
 *---------------------------------------------------------------------------*/
void drawSphereMap(GXTexObj *cubemap,   // array of six textures
                   GXTexObj *spheremap, // output texture
                   void*     dl,        // display list of sphere geometry
                   u32       dlsz)      // size of display list
{
#ifdef MAC
#pragma unused(dl)
#pragma unused(dlsz)
#endif // MAC

    s32        i;
    GXColor    color;
    u16        width, height;
    GXTexFmt   fmt;
    void*      data;      // sphere map texture data
    GXLightObj ClipLight; // to clip q
    Mtx44      p;
    Mtx        v;
    Mtx        tm, tc, tmp;
    Vec camLoc = {0.0F, 0.0F, 6.0F};
    Vec up     = {0.0F, 1.0F, 0.0F};
    Vec objPt  = {0.0F, 0.0F, 0.0F};


    // get sphere map size, format, and data pointer
    width  = GXGetTexObjWidth(spheremap);
    height = GXGetTexObjHeight(spheremap);
    fmt    = GXGetTexObjFmt(spheremap);
    data   = GXGetTexObjData(spheremap);
    //OSReport("w %d h %d fmt %d data %08x\n", width, height, fmt, data);
    

    // set projection
    MTXOrtho(p, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 100.0f);
    GXSetProjection(p, GX_ORTHOGRAPHIC);

    // set modelview matrix
    MTXLookAt(v, &camLoc, &up, &objPt);
    GXLoadPosMtxImm(v, GX_PNMTX0);

    // size the sphere map
    GXSetViewport(0.0F, 0.0F, 
                  (f32)width, (f32)height, 
                  0.0F, 1.0F);

    // set copy parameters
    GXSetTexCopySrc(0, 0, width, height);
    GXSetTexCopyDst(width, height, fmt, GX_FALSE);
    
    // tev equation to deal with q clipping
    // Generate alpha=1 wherever light is pointing.  This alpha is used for
    // clipping the rear projected image.  The tex alpha is used to keep the
    // texture from smearing outside the front projected image.  Multiply
    // these two together to merge.
    GXSetChanCtrl(GX_COLOR0A0, 
            GX_ENABLE, 
            GX_SRC_REG, 
            GX_SRC_REG, 
            GX_LIGHT0, 
            GX_DF_NONE, 
            GX_AF_SPEC);

    // init light direction in the cube-face loop
    color.r = color.g = color.b = color.a = 0xff;
    GXInitLightColor(&ClipLight, color); // pass angle attn through
    GXSetChanMatColor(GX_COLOR0A0, color);

    color.r = color.g = color.b = color.a = 0x0;
    GXSetChanAmbColor(GX_COLOR0A0, color);

    //
    // The specular lighting equation is used to clip the rear projection
    // where q > 0.  The angle and distance attenuation are computed as
    // datt = aatt = N*L > 0 ? N*H : 0
    // We set L = (0,0,-1) to select Nz which is the q component used
    // for texgen (we use the same matrix for normal and texgen).
    // 
    GXInitLightAttnA(&ClipLight, 0.0F, 2.0F, 0.0F);  // saturate on any value..
    GXInitLightAttnK(&ClipLight, 0.0F, 1.0F, 0.0F);  //   of Nz
    GXInitLightPos(&ClipLight, 0.0F, 0.0F,-1.0F); // direction for spec lit
    GXInitLightDir(&ClipLight, 0.0F, 0.0F,-1.0F); // half-angle for spec lit
    GXLoadLightObjImm(&ClipLight, GX_LIGHT0);

    // the texgen (projection) matrix is computed in the loop based 
    // on the cube face being rendered. The same matrix is used for
    // the normal transform.
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX0);

    //
    //  texc*1.0, texa*rasa, a one-texel border of alpha=0 is required
    //

//#ifndef MAC
#ifndef JUNK
    switch (CubeTevMode)
    {
        case 0:  // final result
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0); 
     
        // texa*rasa must be > zero or pixel gets clipped
    
        // output tex color
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        // output ras alpha * tex alpha
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

        GXSetNumTevStages(1);
        break;
    
    
        case 1:  // unclipped texture
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0); 
    
        // output tex color
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        // output ras alpha
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

        GXSetNumTevStages(1);
        break;
    
    
        case 2: // texture alpha
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL); 
    
        // output = 1.0
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        // output tex alpha
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

        GXSetNumTevStages(1);
        break;
    
    
        case 3:  // raster alpha
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0); 

        // out = 1.0
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        // output ras alpha
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

        GXSetNumTevStages(1);
        break;
  
        
        case 4: // ras alpha * tex alpha
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0); 
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL); 
     
        // texa*rasa must be > zero or pixel gets clipped
    
        // output 1.0
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        // output ras alpha * tex alpha
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

        GXSetNumTevStages(1);
        break;
   
    }
    
#else // MAC
    // can't do per-pixel clip on Mac
    GXSetNumTevStages(1);
#endif

    GXSetNumTexGens(1);
    GXSetNumChans(1);

    // compute constant part of the texture matrix 
    // scale/translate texture coordinates so range is moved 
    // from -1,1 to 0,1.  Flip frustum for reflected image.
    // The 0.02 factor is to cover the seams caused by the alpha border.
    MTXLightFrustum(tc, -1.02F, 1.02F, -1.02F, 1.02F,   // t, b, l, r
                         1.0F,                          // near
                         0.5F,  0.5F,   0.5F,   0.5F);  // ss,st,ts,tt

    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

    //
    // we don't clear the background here, but a real app might.
    //

    //
    //  Loop over each cube face texture.  Compute the projection matrix
    //  for each face orientation and direct the light to properly clip the
    //  the rear projected image.
    //
    for (i = CubeFaceStart; i < CubeFaceEnd; i++) 
    {
        // load each face of the cube map
        GXLoadTexObj(&cubemap[i], GX_TEXMAP0);

        // rotate (texture matrix) projection to a face of the cube.
        // when capturing the cube-face images, the rotations must match.
        MTXIdentity(tm);
        if (angle2[i]) 
        {
            // used for top/bottom faces
            MTXRotDeg(tmp, 'y', angle2[i]);
            MTXConcat(tm, tmp, tm);
        }
        MTXRotDeg(tmp, axis1[i], angle1[i]);
        MTXConcat(tm, tmp, tm);

        MTXConcat(tc, tm, tm);
        GXLoadTexMtxImm(tm, GX_TEXMTX0, GX_MTX3x4); 

        // Use same matrix for the normal transform for lighting
        GXLoadNrmMtxImm(tm, GX_PNMTX0);
        
#ifndef MAC
        GXCallDisplayList(dl, dlsz);
#else
        genMapSphere(NULL, NULL, 40, GX_VTXFMT7);
#endif // MAC
    }
 

    //
    //  Copy texture
    //
    GXCopyTex(data, GX_TRUE); // clear old texture
    GXPixModeSync(); // prevent data from being used until copy completes
}
