/*---------------------------------------------------------------------------*
  Project:  Dolphin GD demo
  File:     gd-tev-create.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-tev-create.c $
    
    2     10/13/01 2:29a Hirose
    Fixes due to GDSetTexCoordGen API change.
    
    1     10/04/01 2:48p Hirose
    Initial check in.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   gd-tev
     Displaylist demo with multitexture shader commands
     [Displaylist creation function for both off-line/runtime]
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include "gd-tev.h"

/*---------------------------------------------------------------------------*
    Name:           CreateModelDL
    
    Description:    Creates a display list for drawing model
                    
    Arguments:      dlPtr  : pointer to display list buffer memory
                    dlSize : actual display list size will be set
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static GXVtxDescList VcdSetting[] =
{
    { GX_VA_POS,  GX_INDEX16 },
    { GX_VA_NBT,  GX_INDEX16 },
    { GX_VA_TEX0, GX_INDEX16 },
    { GX_VA_NULL, GX_NONE    }  // NULL Terminator
};

void CreateModelDL( void* dlPtr, u32* dlSize )
{
    GDLObj  dlObj;
    u32     i, j, k;
    u16     idx;

    GDInitGDLObj(&dlObj, dlPtr, MDL_SIZE_MAX);
    GDSetCurrent(&dlObj);

    // set up vertex descriptor
    GDSetVtxDescv(VcdSetting);
    
    // send primitives
    for ( i = 0 ; i < MODEL_MESHY - 1 ; i++ )
    {
        GDBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)(MODEL_MESHX*2));
            for ( j = 0 ; j < MODEL_MESHX ; j++ )
            {
                for ( k = 0 ; k <= 1 ; k++ )
                {
                    idx = (u16)((i+k)*MODEL_MESHX + j);
                    GDPosition1x16(idx);
                    GDNormal1x16(idx);
                    GDTexCoord1x16(idx);
                }
            }
        GDEnd();
    }

    // close the DL creation (padding + flushing)
    GDPadCurr32();
    GDFlushCurrToMem();
    
    // get actual DL size
    *dlSize = GDGetCurrOffset();
    
    // release GD object
    GDSetCurrent(NULL);
}

/*---------------------------------------------------------------------------*
    Name:           CreateShader?DL
        
    Description:    Creates display list which contains shader settings
                    such as TEV and TexGen.
                    
    Arguments:      dlPtr  : pointer to display list buffer memory
                    dlSize : actual display list size will be set
                    plPtr  : pointer to patch list buffer memory
                    plSize : actual patch list size will be set

    Returns:        none
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    These shader display lists expect the environment as following:
    
    These texture map are obtained from a tpl file and should be loaded
    appropriately before you call these shader display lists.

        GX_TEXMAP0 : Emboss bump map texture
        GX_TEXMAP1 : Base texture
        GX_TEXMAP2 : Reflection map texture
        GX_TEXMAP3 : Gloss map texture

    Color channels are reserved for each of diffuse/specular lighting.
    
        GX_COLOR0A0 : Diffuse lighting channel
        GX_COLOR1A1 : Specular lighting channel

    Also, these texgen matrices will be composed in run-time.

        GX_TEXMTX0   : Texgen matrix for bump map
        GX_TEXMTX1   : Texgen matrix for base texture
        GX_TEXMTX2   : Texgen matrix for spherical reflection map (1st.)
        GX_TEXMTX3   : Texgen matrix for gloss map
        GX_PTTEXMTX0 : Texgen matrix for spherical reflection map (2nd.)


 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Shader0DL: (1 TEV stage)
        Base texture * Diffuse light
 *---------------------------------------------------------------------------*/
void CreateShader0DL( void* dlPtr, u32* dlSize, u32* plPtr, u32* plSize )
{
    GDLObj  dlObj;
    u32     plIndex = 0;
    
    GDInitGDLObj(&dlObj, dlPtr, SDL_SIZE_MAX);
    GDSetCurrent(&dlObj);

    // ------------------------ TEV Order -------------------------
    GDSetTevOrder(
        GX_TEVSTAGE0,
        GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0,              // stage 0
        GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);   // stage 1


    // ---------------------- TEXCOORD Scale ----------------------
    // We don't know the actual size of textures at this time
    // because textures are provided from a TPL file in the
    // application. Therefore, the code stuffs dummy data for now
    // and patch them later.
    // 
    // For patching, keep connection infomation between each
    // TEXCOORDs and TEXMAPs in the patch list.

    // GX_TEXCOORD0 - scaled by GX_TEXMAP1
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD0;                // target info (TEXCOORD0)
    plPtr[plIndex++] = 1;                           // source info (TEXMAP1)
    GDSetTexCoordScale2(GX_TEXCOORD0,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);


    // ------------------------ TEVSTAGE 0 ------------------------
    // REGPREV(C) = diffuse lit color * base texture
    
    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE0,
        GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE0,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    
    // ------------- Texture coordinate generation ---------------
    GDSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                     GX_FALSE, GX_PTIDENTITY);

    // All texgen matrix should be set simultaneously (also with PN current matrix.)
    GDSetCurrentMtx(
        GX_PNMTX0,      // Position/Normal current matrix index
        GX_TEXMTX1,     // TEXCOORD0
        GX_IDENTITY,    // TEXCOORD1 (Not used)
        GX_IDENTITY,    // TEXCOORD2 (Not used)
        GX_IDENTITY,    // TEXCOORD3 (Not used)
        GX_IDENTITY,    // TEXCOORD4 (Not used)
        GX_IDENTITY,    // TEXCOORD5 (Not used)
        GX_IDENTITY,    // TEXCOORD6 (Not used)
        GX_IDENTITY);   // TEXCOORD7 (Not used)


    // ------- General graphics pipe configuration setting -------
    GDSetGenMode(1, 1, 1); // NumTexGens, NumChans, NumTevStages

    
    // close the DL creation (padding + flushing)
    GDPadCurr32();
    GDFlushCurrToMem();
    
    // get actual DL size and PL size
    *dlSize = GDGetCurrOffset();
    *plSize = plIndex * sizeof(u32);
    
    // release GD object
    GDSetCurrent(NULL);
}

/*---------------------------------------------------------------------------*
    Shader1DL: (2 TEV stages)
          Base texture * Diffuse light
        + Reflection map * REFLEX_SCALE
        + Specular light
 *---------------------------------------------------------------------------*/
void CreateShader1DL( void* dlPtr, u32* dlSize, u32* plPtr, u32* plSize )
{
    GDLObj      dlObj;
    GXColor     reg0Col = { 0, 0, 0, REFLEX_SCALE };
    u32         plIndex = 0;

    GDInitGDLObj(&dlObj, dlPtr, SDL_SIZE_MAX);
    GDSetCurrent(&dlObj);

    // ------------------- TEV Color Registers --------------------
    GDSetTevColor(GX_TEVREG0, reg0Col);

    // ------------------------ TEV Order -------------------------
    GDSetTevOrder(
        GX_TEVSTAGE0,
        GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR1A1,      // stage 0
        GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);     // stage 1


    // ---------------------- TEXCOORD Scale ----------------------
    // We don't know the actual size of textures at this time
    // because textures are provided from a TPL file in the
    // application. Therefore, the code stuffs dummy data for now
    // and patch them later.
    // 
    // For patching, keep connection infomation between each
    // TEXCOORDs and TEXMAPs in the patch list.

    // GX_TEXCOORD0 - scaled by GX_TEXMAP2
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD0;                // target info (TEXCOORD0)
    plPtr[plIndex++] = 2;                           // source info (TEXMAP2)
    GDSetTexCoordScale2(GX_TEXCOORD0,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);
    // GX_TEXCOORD1 - scaled by GX_TEXMAP1
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD1;                // target info (TEXCOORD1)
    plPtr[plIndex++] = 1;                           // source info (TEXMAP1)
    GDSetTexCoordScale2(GX_TEXCOORD1,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);


    // ------------------------ TEVSTAGE 0 ------------------------
    // REGPREV(C) = specular lit color + reflection map * REG0(A)
    
    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE0,
        GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_RASC,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE0,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    // ------------------------ TEVSTAGE 1 ------------------------
    // REGPREV(C) = REGPREV(C) + diffuse lit color * base texture
    
    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE1,
        GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_CPREV,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE1,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    
    // ------------- Texture coordinate generation ---------------
    GDSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM,
                     GX_TRUE, GX_PTTEXMTX0);
    GDSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0,
                     GX_FALSE, GX_PTIDENTITY);

    // All texgen matrix should be set simultaneously (also with PN current matrix.)
    GDSetCurrentMtx(
        GX_PNMTX0,      // Position/Normal current matrix index
        GX_TEXMTX2,     // TEXCOORD0
        GX_TEXMTX1,     // TEXCOORD1
        GX_IDENTITY,    // TEXCOORD2 (Not used)
        GX_IDENTITY,    // TEXCOORD3 (Not used)
        GX_IDENTITY,    // TEXCOORD4 (Not used)
        GX_IDENTITY,    // TEXCOORD5 (Not used)
        GX_IDENTITY,    // TEXCOORD6 (Not used)
        GX_IDENTITY);   // TEXCOORD7 (Not used)


    // ------- General graphics pipe configuration setting -------
    GDSetGenMode(2, 2, 2); // NumTexGens, NumChans, NumTevStages

    
    // close the DL creation (padding + flushing)
    GDPadCurr32();
    GDFlushCurrToMem();
    
    // get actual DL size and PL size
    *dlSize = GDGetCurrOffset();
    *plSize = plIndex * sizeof(u32);
    
    // release GD object
    GDSetCurrent(NULL);
}

/*---------------------------------------------------------------------------*
    Shader2DL: (3 TEV stages)
        (Diffuse light + Emboss bump map * BUMP_SCALE) * constant color
 *---------------------------------------------------------------------------*/
void CreateShader2DL( void* dlPtr, u32* dlSize, u32* plPtr, u32* plSize )
{
    GDLObj      dlObj;
    GXColor     reg0Col = { 0xFF, 0xE0, 0xC0, BUMP_SCALE };
    u32         plIndex = 0;    

    GDInitGDLObj(&dlObj, dlPtr, SDL_SIZE_MAX);
    GDSetCurrent(&dlObj);

    // ------------------- TEV Color Registers --------------------
    GDSetTevColor(GX_TEVREG0, reg0Col);

    // ------------------------ TEV Order -------------------------
    GDSetTevOrder(
        GX_TEVSTAGE0,
        GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0,              // stage 0
        GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);           // stage 1
    GDSetTevOrder(
        GX_TEVSTAGE2,
        GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL,    // stage 2
        GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);   // stage 3


    // ---------------------- TEXCOORD Scale ----------------------
    // We don't know the actual size of textures at this time
    // because textures are provided from a TPL file in the
    // application. Therefore, the code stuffs dummy data for now
    // and patch them later.
    // 
    // For patching, keep connection infomation between each
    // TEXCOORDs and TEXMAPs in the patch list.

    // GX_TEXCOORD0 - scaled by GX_TEXMAP0
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD0;                // target info (TEXCOORD0)
    plPtr[plIndex++] = 0;                           // source info (TEXMAP0)
    GDSetTexCoordScale2(GX_TEXCOORD0,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);
    // GX_TEXCOORD1 - scaled by GX_TEXMAP0
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD1;                // target info (TEXCOORD1)
    plPtr[plIndex++] = 0;                           // source info (TEXMAP0)
    GDSetTexCoordScale2(GX_TEXCOORD1,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);


    // ------------------------ TEVSTAGE 0 ------------------------
    // REGPREV(C) = diffuse lit color + bump texture * bump scale(REG0(A))
    
    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE0,
        GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_RASC,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE0,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    // ------------------------ TEVSTAGE 1 ------------------------
    // REGPREV(C) = REGPREV(C) - bump texture * bump scale(REG0(A))

    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE1,
        GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_CPREV,
        GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE1,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    // ------------------------ TEVSTAGE 2 ------------------------
    // REGPREV(C) = REGPREV(C) * constant color (REG0(C))

    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE2,
        GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0, GX_CC_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE2,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    
    // ------------- Texture coordinate generation ---------------
    GDSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD1, GX_TG_BUMP0, GX_TG_TEXCOORD0,
                     GX_FALSE, GX_PTIDENTITY);

    // All texgen matrix should be set simultaneously (also with PN current matrix.)
    GDSetCurrentMtx(
        GX_PNMTX0,      // Position/Normal current matrix index
        GX_TEXMTX0,     // TEXCOORD0
        GX_IDENTITY,    // TEXCOORD1
        GX_IDENTITY,    // TEXCOORD2 (Not used)
        GX_IDENTITY,    // TEXCOORD3 (Not used)
        GX_IDENTITY,    // TEXCOORD4 (Not used)
        GX_IDENTITY,    // TEXCOORD5 (Not used)
        GX_IDENTITY,    // TEXCOORD6 (Not used)
        GX_IDENTITY);   // TEXCOORD7 (Not used)


    // ------- General graphics pipe configuration setting -------
    GDSetGenMode(2, 1, 3); // NumTexGens, NumChans, NumTevStages

    
    // close the DL creation (padding + flushing)
    GDPadCurr32();
    GDFlushCurrToMem();
    
    // get actual DL size and PL size
    *dlSize = GDGetCurrOffset();
    *plSize = plIndex * sizeof(u32);
    
    // release GD object
    GDSetCurrent(NULL);
}

/*---------------------------------------------------------------------------*
    Shader3DL: (5 TEV stages)
          (Diffuse light + Emboss bump map * BUMP_SCALE)
          * Base texture * gloss map
        + (Specular light + Reflection map * REFLEX_SCALE)
          * (1 - gloss map)
 *---------------------------------------------------------------------------*/
void CreateShader3DL( void* dlPtr, u32* dlSize, u32* plPtr, u32* plSize )
{
    GDLObj      dlObj;
    GXColor     reg0Col = { 0, 0, 0, BUMP_SCALE };
    GXColor     reg1Col = { 0, 0, 0, REFLEX_SCALE };
    u32         plIndex = 0;    

    GDInitGDLObj(&dlObj, dlPtr, SDL_SIZE_MAX);
    GDSetCurrent(&dlObj);

    // ------------------- TEV Color Registers --------------------
    GDSetTevColor(GX_TEVREG0, reg0Col);
    GDSetTevColor(GX_TEVREG1, reg1Col);

    // ------------------------ TEV Order -------------------------
    GDSetTevOrder(
        GX_TEVSTAGE0,
        GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0,              // stage 0
        GX_TEXCOORD4, GX_TEXMAP0, GX_COLOR_NULL);           // stage 1
    GDSetTevOrder(
        GX_TEVSTAGE2,
        GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL,            // stage 2
        GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1);             // stage 3
    GDSetTevOrder(
        GX_TEVSTAGE4,
        GX_TEXCOORD3, GX_TEXMAP3, GX_COLOR_NULL,            // stage 4
        GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);   // stage 5


    // ---------------------- TEXCOORD Scale ----------------------
    // We don't know the actual size of textures at this time
    // because textures are provided from a TPL file in the
    // application. Therefore, the code stuffs dummy data for now
    // and patch them later.
    // 
    // For patching, keep connection infomation between each
    // TEXCOORDs and TEXMAPs in the patch list.

    // GX_TEXCOORD0 - scaled by GX_TEXMAP0
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD0;                // target info (TEXCOORD0)
    plPtr[plIndex++] = 0;                           // source info (TEXMAP0)
    GDSetTexCoordScale2(GX_TEXCOORD0,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);
    // GX_TEXCOORD1 - scaled by GX_TEXMAP1
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD1;                // target info (TEXCOORD1)
    plPtr[plIndex++] = 1;                           // source info (TEXMAP1)
    GDSetTexCoordScale2(GX_TEXCOORD1,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);
    // GX_TEXCOORD2 - scaled by GX_TEXMAP2
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD2;                // target info (TEXCOORD2)
    plPtr[plIndex++] = 2;                           // source info (TEXMAP2)
    GDSetTexCoordScale2(GX_TEXCOORD2,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);
    // GX_TEXCOORD3 - scaled by GX_TEXMAP3
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD3;                // target info (TEXCOORD3)
    plPtr[plIndex++] = 3;                           // source info (TEXMAP3)
    GDSetTexCoordScale2(GX_TEXCOORD0,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);
    // GX_TEXCOORD4 - scaled by GX_TEXMAP0
    plPtr[plIndex++] = GDGetCurrOffset();           // patch pointer
    plPtr[plIndex++] = GX_TEXCOORD4;                // target info (TEXCOORD4)
    plPtr[plIndex++] = 0;                           // source info (TEXMAP0)
    GDSetTexCoordScale2(GX_TEXCOORD4,               // dummy
                        1, GX_DISABLE, GX_DISABLE,
                        1, GX_DISABLE, GX_DISABLE);


    // ------------------------ TEVSTAGE 0 ------------------------
    // REGPREV(C) = diffuse lit color + bump texture * bump scale(REG0(A))
    
    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE0,
        GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_RASC,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE0,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    // ------------------------ TEVSTAGE 1 ------------------------
    // REGPREV(C) = REGPREV(C) - bump texture * bump scale(REG0(A))

    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE1,
        GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_CPREV,
        GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE1,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    // ------------------------ TEVSTAGE 2 ------------------------
    // REG2(C) = REGPREV(C) * base texture

    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE2,
        GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVREG2);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE2,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVREG2,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    // ------------------------ TEVSTAGE 3 ------------------------
    // REGPREV(C) = specular lit color + reflection map * reflection scale(REG1(A))

    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE3,
        GX_CC_ZERO, GX_CC_A1, GX_CC_TEXC, GX_CC_RASC,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE3,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    // ------------------------ TEVSTAGE 4 ------------------------
    // REGPREV(C) = gloss map * REG2(C) + (1 - gloss map) * REGPREV(C)

    // Color operation
    GDSetTevColorCalc(
        GX_TEVSTAGE4,
        GX_CC_CPREV, GX_CC_C2, GX_CC_TEXC, GX_CC_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
    // Alpha operation + swap mode
    GDSetTevAlphaCalcAndSwap(
        GX_TEVSTAGE4,
        GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_DISABLE, GX_TEVPREV,
        GX_TEV_SWAP0, GX_TEV_SWAP0);

    
    // ------------- Texture coordinate generation ---------------
    GDSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_NRM,
                     GX_TRUE, GX_PTTEXMTX0);
    GDSetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0,
                     GX_FALSE, GX_PTIDENTITY);
    GDSetTexCoordGen(GX_TEXCOORD4, GX_TG_BUMP0, GX_TG_TEXCOORD0,
                     GX_FALSE, GX_PTIDENTITY);

    // All texgen matrix should be set simultaneously (also with PN current matrix.)
    GDSetCurrentMtx(
        GX_PNMTX0,      // Position/Normal current matrix index
        GX_TEXMTX0,     // TEXCOORD0
        GX_TEXMTX1,     // TEXCOORD1
        GX_TEXMTX2,     // TEXCOORD2
        GX_TEXMTX3,     // TEXCOORD3
        GX_IDENTITY,    // TEXCOORD4
        GX_IDENTITY,    // TEXCOORD5 (Not used)
        GX_IDENTITY,    // TEXCOORD6 (Not used)
        GX_IDENTITY);   // TEXCOORD7 (Not used)


    // ------- General graphics pipe configuration setting -------
    GDSetGenMode(5, 2, 5); // NumTexGens, NumChans, NumTevStages

    
    // close the DL creation (padding + flushing)
    GDPadCurr32();
    GDFlushCurrToMem();
    
    // get actual DL size and PL size
    *dlSize = GDGetCurrOffset();
    *plSize = plIndex * sizeof(u32);
    
    // release GD object
    GDSetCurrent(NULL);
}

/*============================================================================*/
