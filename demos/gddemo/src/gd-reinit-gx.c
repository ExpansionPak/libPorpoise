/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-reinit-gx.c

  Copyright 1998 - 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-reinit-gx.c $
    
    1     10/24/02 8:39a Hirose
    Initial check in.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    This is sample code for GX re-initialization for GD users.

    Display lists made by GD can directly modify state of the graphics
    processor. Sometimes it causes coherancy issues between GX library
    and actual state in the graphics processor after calling GD disaplay
    list. The most safest way (and simplest way) is just calling bunch of
    GX APIs again so that you can make all GX state stable.
    
    This code shows an example of such GX re-initialization.
    It is useful when you want to draw something by using GX after calling
    GD display lists. It may also help debugging in the eraly stage of
    development with GD library.
    
    However, these are heavy task and some optimization might be necessary
    if you want to use in the final application. You can edit this code and
    remove sections that are redundant for your code as you like. We put
    three levels of the compile option to inform about start point of
    such work.
 *---------------------------------------------------------------------------*/


#include <dolphin.h>


/*---------------------------------------------------------------------------*
    Compile option
 *---------------------------------------------------------------------------*/
// Reinitialize minimum sets at least necessary to guarantee state coherencies.
#define REINIT_MINIMUM      0
// Reinitialize and set up a typical safe graphics state.
#define REINIT_TYPICAL      1
// Reinitialize and set all state to defualts.
#define REINIT_ALLDEFAULTS  2


// You can change this definition as you prefer.
#define REINIT_LEVEL    REINIT_ALLDEFAULTS

/*---------------------------------------------------------------------------*
    Forward references
 *---------------------------------------------------------------------------*/
extern void ReInitializeGX      ( void );

static void ReInitGenMode       ( void );
static void ReInitGeometry      ( void );
static void ReInitLighting      ( void );
static void ReInitTransform     ( void );
static void ReInitTexture       ( void );
static void ReInitTevStages     ( void );
static void ReInitIndStages     ( void );
static void ReInitPixelProc     ( void );


/*---------------------------------------------------------------------------*
    Data
 *---------------------------------------------------------------------------*/
#define NULLTEX_WIDTH   4
#define NULLTEX_HEIGHT  4
#define NULLTEX_FMT     GX_TF_IA8

static u8   NullTexData[] ATTRIBUTE_ALIGN(32) =
{
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000
};

static f32  IdentityMtx[3][4] =
{
    { 1.0F, 0.0F, 0.0F, 0.0F },
    { 0.0F, 1.0F, 0.0F, 0.0F },
    { 0.0F, 0.0F, 1.0F, 0.0F }
};

static f32  IndMtx[2][3] =
{
    { 0.5F, 0.0F, 0.0F },
    { 0.0F, 0.5F, 0.0F }
};


static GXColor  ColorBlack = { 0x00, 0x00, 0x00, 0x00 };
static GXColor  ColorWhite = { 0xFF, 0xFF, 0xFF, 0xFF };


/*---------------------------------------------------------------------------*
  Name:         ReInitializeGX

  Description:  Re-initializes graphics.

  Arguments:    none

  Returns:      none
 *---------------------------------------------------------------------------*/
void ReInitializeGX( void )
{
    ReInitGenMode();
    ReInitGeometry();
    ReInitLighting();
    ReInitTransform();
    ReInitTexture();
    ReInitTevStages();
    ReInitIndStages();
    ReInitPixelProc();
}

/*---------------------------------------------------------------------------*
            "Genmode" - Rendering Pipe Control API Group
 *---------------------------------------------------------------------------*/
static void ReInitGenmode( void )
{
    // RELATED: GDSetCullMode, GDSetCoPlanar, GDSetGenMode, GDSetGenMode2
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetCullMode(GX_CULL_BACK);
    GXSetCoPlanar(GX_DISABLE);
}

/*---------------------------------------------------------------------------*
            Geometry API Group
 *---------------------------------------------------------------------------*/
static void ReInitGeometry( void )
{
#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    GXVtxAttrFmtList defAttrList[] =
    {
        { GX_VA_POS,  GX_POS_XYZ,   GX_F32,   0 },
        { GX_VA_NRM,  GX_NRM_XYZ,   GX_F32,   0 },
        { GX_VA_CLR0, GX_CLR_RGBA,  GX_RGBA8, 0 },
        { GX_VA_CLR1, GX_CLR_RGBA,  GX_RGBA8, 0 },
        { GX_VA_TEX0, GX_TEX_ST,    GX_F32,   0 },
        { GX_VA_TEX1, GX_TEX_ST,    GX_F32,   0 },
        { GX_VA_TEX2, GX_TEX_ST,    GX_F32,   0 },
        { GX_VA_TEX3, GX_TEX_ST,    GX_F32,   0 },
        { GX_VA_TEX4, GX_TEX_ST,    GX_F32,   0 },
        { GX_VA_TEX5, GX_TEX_ST,    GX_F32,   0 },
        { GX_VA_TEX6, GX_TEX_ST,    GX_F32,   0 },
        { GX_VA_TEX7, GX_TEX_ST,    GX_F32,   0 },
        { GX_VA_NULL, (GXCompCnt)0, (GXCompType)0, 0 }
    };
    u32     i;
#endif

    // RELATED: GDSetVtxDescv
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);


    // RELATED: GDSetVtxAttrv
#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    for ( i = 0; i < GX_MAX_VTXFMT; ++i )
    {
        GXSetVtxAttrFmtv((GXVtxFmt)i, defAttrList);
    }
#endif

    // RELATED: GDSetLPSize
    GXSetLineWidth(6, GX_TO_ZERO);
    GXSetPointSize(6, GX_TO_ZERO);

    // RELATED: GDSetTexCoordScaleAndTOEs
    GXEnableTexOffsets(GX_TEXCOORD0, GX_DISABLE, GX_DISABLE);

#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    GXEnableTexOffsets(GX_TEXCOORD1, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD2, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD3, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD4, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD5, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD6, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD7, GX_DISABLE, GX_DISABLE);
#endif

}

/*---------------------------------------------------------------------------*
            Lighting API Group
 *---------------------------------------------------------------------------*/
static void ReInitLighting( void )
{

#if ( REINIT_LEVEL >= REINIT_TYPICAL )
    // RELATED: GDSetChanCtrl
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
#endif

#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    // RELATED: GDSetChanAmbColor
    GXSetChanAmbColor(GX_COLOR0A0, ColorBlack);
    GXSetChanAmbColor(GX_COLOR1A1, ColorBlack);
    
    // RELATED: GDSetChanMatColor
    GXSetChanMatColor(GX_COLOR0A0, ColorWhite);
    GXSetChanMatColor(GX_COLOR1A1, ColorWhite);
#endif

}

/*---------------------------------------------------------------------------*
            Transform + TexGen API Group
 *---------------------------------------------------------------------------*/
static void ReInitTransform( void )
{

#if ( REINIT_LEVEL >= REINIT_TYPICAL )
    // RELATED: GDLoad*MtxImm, GDLoad*MtxIndx
    GXLoadPosMtxImm(IdentityMtx, GX_PNMTX0);
    GXLoadNrmMtxImm(IdentityMtx, GX_PNMTX0);
    GXLoadTexMtxImm(IdentityMtx, GX_IDENTITY, GX_MTX3x4);
    GXLoadTexMtxImm(IdentityMtx, GX_PTIDENTITY, GX_MTX3x4);
#endif
    
    // RELATED: GDSetTexCoordGen, GDSetCurrentMtx
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
    GXSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY);
    GXSetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX3, GX_IDENTITY);
    GXSetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX4, GX_IDENTITY);
    GXSetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX5, GX_IDENTITY);
    GXSetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX6, GX_IDENTITY);
    GXSetTexCoordGen(GX_TEXCOORD7, GX_TG_MTX2x4, GX_TG_TEX7, GX_IDENTITY);
#endif

    // RELATED: GDSetTexCoordScale, GDSetTexCoordScale2, GDSetTexCoordScaleAndTOEs
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_DISABLE, 0, 0);
    GXSetTexCoordCylWrap(GX_TEXCOORD0, GX_DISABLE, GX_DISABLE);

#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_DISABLE, 0, 0);
    GXSetTexCoordCylWrap(GX_TEXCOORD1, GX_DISABLE, GX_DISABLE);
    GXSetTexCoordScaleManually(GX_TEXCOORD2, GX_DISABLE, 0, 0);
    GXSetTexCoordCylWrap(GX_TEXCOORD2, GX_DISABLE, GX_DISABLE);
    GXSetTexCoordScaleManually(GX_TEXCOORD3, GX_DISABLE, 0, 0);
    GXSetTexCoordCylWrap(GX_TEXCOORD3, GX_DISABLE, GX_DISABLE);
    GXSetTexCoordScaleManually(GX_TEXCOORD4, GX_DISABLE, 0, 0);
    GXSetTexCoordCylWrap(GX_TEXCOORD4, GX_DISABLE, GX_DISABLE);
    GXSetTexCoordScaleManually(GX_TEXCOORD5, GX_DISABLE, 0, 0);
    GXSetTexCoordCylWrap(GX_TEXCOORD5, GX_DISABLE, GX_DISABLE);
    GXSetTexCoordScaleManually(GX_TEXCOORD6, GX_DISABLE, 0, 0);
    GXSetTexCoordCylWrap(GX_TEXCOORD6, GX_DISABLE, GX_DISABLE);
    GXSetTexCoordScaleManually(GX_TEXCOORD7, GX_DISABLE, 0, 0);
    GXSetTexCoordCylWrap(GX_TEXCOORD7, GX_DISABLE, GX_DISABLE);
#endif
}

/*---------------------------------------------------------------------------*
            Texture API Group
 *---------------------------------------------------------------------------*/
static void ReInitTexture( void )
{
    GXTexObj    tobj;
    
    // RELATED: GDSetTexImgAttr, GDSetTexImgPtr, GDSetTexLookupMode,
    //          GDSetTexTlut, GDSetTexCached, GDSetTexPreLoaded
    GXInitTexObj(&tobj, NullTexData, NULLTEX_WIDTH, NULLTEX_HEIGHT,
                 NULLTEX_FMT, GX_CLAMP, GX_CLAMP, GX_FALSE);

    GXLoadTexObj(&tobj, GX_TEXMAP0);

#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    GXLoadTexObj(&tobj, GX_TEXMAP1);
    GXLoadTexObj(&tobj, GX_TEXMAP2);
    GXLoadTexObj(&tobj, GX_TEXMAP3);
    GXLoadTexObj(&tobj, GX_TEXMAP4);
    GXLoadTexObj(&tobj, GX_TEXMAP5);
    GXLoadTexObj(&tobj, GX_TEXMAP6);
    GXLoadTexObj(&tobj, GX_TEXMAP7);
#endif

}

/*---------------------------------------------------------------------------*
            Texture Environment API Group
 *---------------------------------------------------------------------------*/
static void ReInitTevStages( void )
{
    u32     i, max;

    // RELATED: GDSetTevOrder
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP3, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD4, GX_TEXMAP4, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD5, GX_TEXMAP5, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE6, GX_TEXCOORD6, GX_TEXMAP6, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE7, GX_TEXCOORD7, GX_TEXMAP7, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE8, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE9, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE10,GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE11,GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE12,GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE13,GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE14,GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE15,GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
#endif

#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    // RELATED: GDSetTevColor, GDSetTevColorS10
    GXSetTevColor(GX_TEVREG0, ColorWhite);
    GXSetTevColor(GX_TEVREG1, ColorWhite);
    GXSetTevColor(GX_TEVREG2, ColorWhite);
    // RELATED: GDSetTevKColor
    GXSetTevKColor(GX_KCOLOR0, ColorWhite);
    GXSetTevKColor(GX_KCOLOR1, ColorWhite);
    GXSetTevKColor(GX_KCOLOR2, ColorWhite);
    GXSetTevKColor(GX_KCOLOR3, ColorWhite);
#endif

    // RELATED: GDSetTevOp, GDSetTevColorCalc, GDSetTevAlphaCalcAndSwap
    //           GDSetTevKonstantSel, GDSetTevSwapModeTable
#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    max = GX_MAX_TEVSTAGES;
#else
    max = GX_TEVSTAGE1;
#endif
   for (i = GX_TEVSTAGE0; i < max; i++)
    {
        GXSetTevOp((GXTevStageID)i, GX_REPLACE);
        GXSetTevKColorSel((GXTevStageID)i, GX_TEV_KCSEL_1_4);
        GXSetTevKAlphaSel((GXTevStageID)i, GX_TEV_KASEL_1);
        GXSetTevSwapMode ((GXTevStageID)i, GX_TEV_SWAP0, GX_TEV_SWAP0);
    }
#if ( REINIT_LEVEL >= REINIT_TYPICAL )
    GXSetTevSwapModeTable(GX_TEV_SWAP0,
                          GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP1,
                          GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP2,
                          GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP3,
                          GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);
#endif


#if ( REINIT_LEVEL >= REINIT_TYPICAL )
    // RELATED: GDSetAlphaCompare
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    
    // RELATED: GDSetZTexture
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
#endif

}

/*---------------------------------------------------------------------------*
            Indirect Texture API Group
 *---------------------------------------------------------------------------*/
static void ReInitIndStages( void )
{
    u32     i;

    for (i = GX_TEVSTAGE0; i < GX_MAX_TEVSTAGE; i++)
    {
        GXSetTevDirect((GXTevStageID) i);
    }

#if ( REINIT_LEVEL >= REINIT_ALLDEFAULTS )
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexOrder(GX_INDTEXSTAGE2, GX_TEXCOORD2, GX_TEXMAP2);
    GXSetIndTexOrder(GX_INDTEXSTAGE3, GX_TEXCOORD3, GX_TEXMAP3);

    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE2, GX_ITS_1, GX_ITS_1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE3, GX_ITS_1, GX_ITS_1);

    GXSetIndTexMtx(GX_ITM_0, IndMtx, 1);
    GXSetIndTexMtx(GX_ITM_1, IndMtx, 1);
    GXSetIndTexMtx(GX_ITM_2, IndMtx, 1);
#endif

}

/*---------------------------------------------------------------------------*
            Pixel Processing API Group
 *---------------------------------------------------------------------------*/
static void ReInitPixelProc( void )
{

    // RELATED: GDSetBlendMode, GDSetBlendModeEtc
    GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetColorUpdate(GX_ENABLE);
    GXSetAlphaUpdate(GX_ENABLE);
    GXSetDither(GX_ENABLE);
    
#if ( REINIT_LEVEL >= REINIT_TYPICAL )
    // RELATED: GDSetFog
    GXSetFog(GX_FOG_NONE, 0.0F, 1.0F, 0.1F, 1.0F, ColorBlack);
    GXSetFogRangeAdj(GX_DISABLE, 0, 0);

    // RELATED: GDSetZMode
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    
    // RELATED: GDSetDstAlpha
    GXSetDstAlpha(GX_DISABLE, 0);
#endif
    
}

/*===========================================================================*/
