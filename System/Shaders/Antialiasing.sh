//---------------------------------------------------------------------------//
//              ____           _                         _                   //
//             / ___|__ _ _ __| |__   ___  _ __   __   _/ | __  __           //
//            | |   / _` | '__| '_ \ / _ \| '_ \  \ \ / / | \ \/ /           //
//            | |__| (_| | |  | |_) | (_) | | | |  \ V /| |_ >  <            //
//             \____\__,_|_|  |_.__/ \___/|_| |_|   \_/ |_(_)_/\_\           //
//                    Game Institute - Carbon Game Development Toolkit       //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : Antialiasing.sh                                                    //
//                                                                           //
// Desc : Internal surface shader which provides support for the various     //
//        antliasing operations supported by the antialias image processor.  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "Utilities.shh"

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENTSCRIPT
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return AntialiasingShader( owner, driver, resources );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : AntialiasingShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for antliasing
//        processes.
//-----------------------------------------------------------------------------
class AntialiasingShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;         // The parent application object.
    private RenderDriver@           mDriver;        // Render driver to which this shader is linked.
    private ResourceManager@        mResources;     // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////////////////
    // Custom Constant Buffer Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?cbuffer cbAntialiasing : register(b12), globaloffset(c180)
        float4 textureSize; // xy = dimensions, zw = recip dimensions 
        matrix previousViewProjMatrix; 
   		float  resolveMaxSpeed;
		float  reprojectionWeight;
    ?>
    
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D sImage       : register(s0);
        Sampler2D sDepth       : register(s1);
        Sampler2D sVelocity    : register(s1);
        Sampler2D sImagePrev   : register(s2);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : AntialiasingShader() (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	AntialiasingShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner     = owner;
        @mDriver    = driver;
        @mResources = resources;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Vertex Shaders
    ///////////////////////////////////////////////////////////////////////////
	//-------------------------------------------------------------------------
	// Name : transform()
	// Desc : Used when drawing full screen clip quad with eye ray interpolator 
	//-------------------------------------------------------------------------
	bool transform( bool outputEyeRay )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 sourcePosition   : POSITION;
        ?>

        // Define shader outputs.
		<?out
			float4 clipPosition     : SV_POSITION;
			float2 texCoords        : TEXCOORD0;
		?>
		if ( outputEyeRay )
		{
			<?out	
				float3 eyeRay           : TEXCOORD1;
			?>
		}
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		clipPosition.xyz = sourcePosition.xyz;
		clipPosition.w   = 1.0;

		// Get the screen coords into range [0,1]
		texCoords.xy = float2( clipPosition.x, -clipPosition.y ) * 0.5 + 0.5;

		// Adjust screen coords to take into account the currently set viewport and half pixel offset. 
		texCoords.xy = texCoords.xy * _screenUVAdjustScale + _screenUVAdjustBias;
		?>

		// This ray goes from the camera position to the pixel in the screen
		if ( outputEyeRay )
		{
			<?
			float4 viewPosition = mul( float4( clipPosition.x * _cameraFar, clipPosition.y * _cameraFar, _cameraFar, _cameraFar ), _inverseProjectionMatrix );
			eyeRay = mul( viewPosition.xyz, _inverseViewMatrix );
			?>
		}
		
		// Valid shader
		return true;
	}

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
	<?common

		#define   FXAA_PC      1
		#define   FXAA_HLSL_3  1
		#define	  FXAA_QUALITY__PRESET 39

		/*============================================================================

									 INTEGRATION KNOBS

		============================================================================*/
		//
		// FXAA_PS3 and FXAA_360 choose the console algorithm (FXAA3 CONSOLE).
		// FXAA_360_OPT is a prototype for the new optimized 360 version.
		//
		// 1 = Use API.
		// 0 = Don't use API.
		//
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_PS3
			#define FXAA_PS3 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_360
			#define FXAA_360 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_360_OPT
			#define FXAA_360_OPT 0
		#endif
		/*==========================================================================*/
		#ifndef FXAA_PC
			//
			// FXAA Quality
			// The high quality PC algorithm.
			//
			#define FXAA_PC 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_PC_CONSOLE
			//
			// The console algorithm for PC is included
			// for developers targeting really low spec machines.
			// Likely better to just run FXAA_PC, and use a really low preset.
			//
			#define FXAA_PC_CONSOLE 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_GLSL_120
			#define FXAA_GLSL_120 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_GLSL_130
			#define FXAA_GLSL_130 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_HLSL_3
			#define FXAA_HLSL_3 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_HLSL_4
			#define FXAA_HLSL_4 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_HLSL_5
			#define FXAA_HLSL_5 0
		#endif
		/*==========================================================================*/
		#ifndef FXAA_GREEN_AS_LUMA
			//
			// For those using non-linear color,
			// and either not able to get luma in alpha, or not wanting to,
			// this enables FXAA to run using green as a proxy for luma.
			// So with this enabled, no need to pack luma in alpha.
			//
			// This will turn off AA on anything which lacks some amount of green.
			// Pure red and blue or combination of only R and B, will get no AA.
			//
			// Might want to lower the settings for both,
			//    fxaaConsoleEdgeThresholdMin
			//    fxaaQualityEdgeThresholdMin
			// In order to insure AA does not get turned off on colors 
			// which contain a minor amount of green.
			//
			// 1 = On.
			// 0 = Off.
			//
			#define FXAA_GREEN_AS_LUMA 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_EARLY_EXIT
			//
			// Controls algorithm's early exit path.
			// On PS3 turning this ON adds 2 cycles to the shader.
			// On 360 turning this OFF adds 10ths of a millisecond to the shader.
			// Turning this off on console will result in a more blurry image.
			// So this defaults to on.
			//
			// 1 = On.
			// 0 = Off.
			//
			#define FXAA_EARLY_EXIT 1
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_DISCARD
			//
			// Only valid for PC OpenGL currently.
			// Probably will not work when FXAA_GREEN_AS_LUMA = 1.
			//
			// 1 = Use discard on pixels which don't need AA.
			//     For APIs which enable concurrent TEX+ROP from same surface.
			// 0 = Return unchanged color on pixels which don't need AA.
			//
			#define FXAA_DISCARD 0
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_FAST_PIXEL_OFFSET
			//
			// Used for GLSL 120 only.
			//
			// 1 = GL API supports fast pixel offsets
			// 0 = do not use fast pixel offsets
			//
			#ifdef GL_EXT_gpu_shader4
				#define FXAA_FAST_PIXEL_OFFSET 1
			#endif
			#ifdef GL_NV_gpu_shader5
				#define FXAA_FAST_PIXEL_OFFSET 1
			#endif
			#ifdef GL_ARB_gpu_shader5
				#define FXAA_FAST_PIXEL_OFFSET 1
			#endif
			#ifndef FXAA_FAST_PIXEL_OFFSET
				#define FXAA_FAST_PIXEL_OFFSET 0
			#endif
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_GATHER4_ALPHA
			//
			// 1 = API supports gather4 on alpha channel.
			// 0 = API does not support gather4 on alpha channel.
			//
			#if (FXAA_HLSL_5 == 1)
				#define FXAA_GATHER4_ALPHA 1
			#endif
			#ifdef GL_ARB_gpu_shader5
				#define FXAA_GATHER4_ALPHA 1
			#endif
			#ifdef GL_NV_gpu_shader5
				#define FXAA_GATHER4_ALPHA 1
			#endif
			#ifndef FXAA_GATHER4_ALPHA
				#define FXAA_GATHER4_ALPHA 0
			#endif
		#endif

		/*============================================================================
							  FXAA CONSOLE PS3 - TUNING KNOBS
		============================================================================*/
		#ifndef FXAA_CONSOLE__PS3_EDGE_SHARPNESS
			//
			// Consoles the sharpness of edges on PS3 only.
			// Non-PS3 tuning is done with shader input.
			//
			// Due to the PS3 being ALU bound,
			// there are only two safe values here: 4 and 8.
			// These options use the shaders ability to a free *|/ by 2|4|8.
			//
			// 8.0 is sharper
			// 4.0 is softer
			// 2.0 is really soft (good for vector graphics inputs)
			//
			#if 1
				#define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 8.0
			#endif
			#if 0
				#define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 4.0
			#endif
			#if 0
				#define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 2.0
			#endif
		#endif
		/*--------------------------------------------------------------------------*/
		#ifndef FXAA_CONSOLE__PS3_EDGE_THRESHOLD
			//
			// Only effects PS3.
			// Non-PS3 tuning is done with shader input.
			//
			// The minimum amount of local contrast required to apply algorithm.
			// The console setting has a different mapping than the quality setting.
			//
			// This only applies when FXAA_EARLY_EXIT is 1.
			//
			// Due to the PS3 being ALU bound,
			// there are only two safe values here: 0.25 and 0.125.
			// These options use the shaders ability to a free *|/ by 2|4|8.
			//
			// 0.125 leaves less aliasing, but is softer
			// 0.25 leaves more aliasing, and is sharper
			//
			#if 1
				#define FXAA_CONSOLE__PS3_EDGE_THRESHOLD 0.125
			#else
				#define FXAA_CONSOLE__PS3_EDGE_THRESHOLD 0.25
			#endif
		#endif

		/*============================================================================
								FXAA QUALITY - TUNING KNOBS
		------------------------------------------------------------------------------
		NOTE the other tuning knobs are now in the shader function inputs!
		============================================================================*/
		#ifndef FXAA_QUALITY__PRESET
			//
			// Choose the quality preset.
			// This needs to be compiled into the shader as it effects code.
			// Best option to include multiple presets is to 
			// in each shader define the preset, then include this file.
			// 
			// OPTIONS
			// -----------------------------------------------------------------------
			// 10 to 15 - default medium dither (10=fastest, 15=highest quality)
			// 20 to 29 - less dither, more expensive (20=fastest, 29=highest quality)
			// 39       - no dither, very expensive 
			//
			// NOTES
			// -----------------------------------------------------------------------
			// 12 = slightly faster then FXAA 3.9 and higher edge quality (default)
			// 13 = about same speed as FXAA 3.9 and better than 12
			// 23 = closest to FXAA 3.9 visually and performance wise
			//  _ = the lowest digit is directly related to performance
			// _  = the highest digit is directly related to style
			// 
			#define FXAA_QUALITY__PRESET 12
		#endif


		/*============================================================================

								   FXAA QUALITY - PRESETS

		============================================================================*/

		/*============================================================================
							 FXAA QUALITY - MEDIUM DITHER PRESETS
		============================================================================*/
		#if (FXAA_QUALITY__PRESET == 10)
			#define FXAA_QUALITY__PS 3
			#define FXAA_QUALITY__P0 1.5
			#define FXAA_QUALITY__P1 3.0
			#define FXAA_QUALITY__P2 12.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 11)
			#define FXAA_QUALITY__PS 4
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 3.0
			#define FXAA_QUALITY__P3 12.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 12)
			#define FXAA_QUALITY__PS 5
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 4.0
			#define FXAA_QUALITY__P4 12.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 13)
			#define FXAA_QUALITY__PS 6
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 4.0
			#define FXAA_QUALITY__P5 12.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 14)
			#define FXAA_QUALITY__PS 7
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 4.0
			#define FXAA_QUALITY__P6 12.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 15)
			#define FXAA_QUALITY__PS 8
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 2.0
			#define FXAA_QUALITY__P6 4.0
			#define FXAA_QUALITY__P7 12.0
		#endif

		/*============================================================================
							 FXAA QUALITY - LOW DITHER PRESETS
		============================================================================*/
		#if (FXAA_QUALITY__PRESET == 20)
			#define FXAA_QUALITY__PS 3
			#define FXAA_QUALITY__P0 1.5
			#define FXAA_QUALITY__P1 2.0
			#define FXAA_QUALITY__P2 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 21)
			#define FXAA_QUALITY__PS 4
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 22)
			#define FXAA_QUALITY__PS 5
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 23)
			#define FXAA_QUALITY__PS 6
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 24)
			#define FXAA_QUALITY__PS 7
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 3.0
			#define FXAA_QUALITY__P6 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 25)
			#define FXAA_QUALITY__PS 8
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 2.0
			#define FXAA_QUALITY__P6 4.0
			#define FXAA_QUALITY__P7 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 26)
			#define FXAA_QUALITY__PS 9
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 2.0
			#define FXAA_QUALITY__P6 2.0
			#define FXAA_QUALITY__P7 4.0
			#define FXAA_QUALITY__P8 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 27)
			#define FXAA_QUALITY__PS 10
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 2.0
			#define FXAA_QUALITY__P6 2.0
			#define FXAA_QUALITY__P7 2.0
			#define FXAA_QUALITY__P8 4.0
			#define FXAA_QUALITY__P9 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 28)
			#define FXAA_QUALITY__PS 11
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 2.0
			#define FXAA_QUALITY__P6 2.0
			#define FXAA_QUALITY__P7 2.0
			#define FXAA_QUALITY__P8 2.0
			#define FXAA_QUALITY__P9 4.0
			#define FXAA_QUALITY__P10 8.0
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PRESET == 29)
			#define FXAA_QUALITY__PS 12
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.5
			#define FXAA_QUALITY__P2 2.0
			#define FXAA_QUALITY__P3 2.0
			#define FXAA_QUALITY__P4 2.0
			#define FXAA_QUALITY__P5 2.0
			#define FXAA_QUALITY__P6 2.0
			#define FXAA_QUALITY__P7 2.0
			#define FXAA_QUALITY__P8 2.0
			#define FXAA_QUALITY__P9 2.0
			#define FXAA_QUALITY__P10 4.0
			#define FXAA_QUALITY__P11 8.0
		#endif

		/*============================================================================
							 FXAA QUALITY - EXTREME QUALITY
		============================================================================*/
		#if (FXAA_QUALITY__PRESET == 39)
			#define FXAA_QUALITY__PS 12
			#define FXAA_QUALITY__P0 1.0
			#define FXAA_QUALITY__P1 1.0
			#define FXAA_QUALITY__P2 1.0
			#define FXAA_QUALITY__P3 1.0
			#define FXAA_QUALITY__P4 1.0
			#define FXAA_QUALITY__P5 1.5
			#define FXAA_QUALITY__P6 2.0
			#define FXAA_QUALITY__P7 2.0
			#define FXAA_QUALITY__P8 2.0
			#define FXAA_QUALITY__P9 2.0
			#define FXAA_QUALITY__P10 4.0
			#define FXAA_QUALITY__P11 8.0
		#endif

		/*============================================================================

										API PORTING

		============================================================================*/
		#if (FXAA_GLSL_120 == 1) || (FXAA_GLSL_130 == 1)
			#define FxaaBool bool
			#define FxaaDiscard discard
			#define FxaaFloat float
			#define FxaaFloat2 vec2
			#define FxaaFloat3 vec3
			#define FxaaFloat4 vec4
			#define FxaaHalf float
			#define FxaaHalf2 vec2
			#define FxaaHalf3 vec3
			#define FxaaHalf4 vec4
			#define FxaaInt2 ivec2
			#define FxaaSat(x) clamp(x, 0.0, 1.0)
			#define FxaaTex sampler2D
		#else
			#define FxaaBool bool
			#define FxaaDiscard clip(-1)
			#define FxaaFloat float
			#define FxaaFloat2 float2
			#define FxaaFloat3 float3
			#define FxaaFloat4 float4
			#define FxaaHalf half
			#define FxaaHalf2 half2
			#define FxaaHalf3 half3
			#define FxaaHalf4 half4
			#define FxaaSat(x) saturate(x)
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_GLSL_120 == 1)
			// Requires,
			//  #version 120
			// And at least,
			//  #extension GL_EXT_gpu_shader4 : enable
			//  (or set FXAA_FAST_PIXEL_OFFSET 1 to work like DX9)
			#define FxaaTexTop(t, p) texture2DLod(t, p, 0.0)
			#if (FXAA_FAST_PIXEL_OFFSET == 1)
				#define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)
			#else
				#define FxaaTexOff(t, p, o, r) texture2DLod(t, p + (o * r), 0.0)
			#endif
			#if (FXAA_GATHER4_ALPHA == 1)
				// use #extension GL_ARB_gpu_shader5 : enable
				#define FxaaTexAlpha4(t, p) textureGather(t, p, 3)
				#define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)
				#define FxaaTexGreen4(t, p) textureGather(t, p, 1)
				#define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)
			#endif
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_GLSL_130 == 1)
			// Requires "#version 130" or better
			#define FxaaTexTop(t, p) textureLod(t, p, 0.0)
			#define FxaaTexOff(t, p, o, r) textureLodOffset(t, p, 0.0, o)
			#if (FXAA_GATHER4_ALPHA == 1)
				// use #extension GL_ARB_gpu_shader5 : enable
				#define FxaaTexAlpha4(t, p) textureGather(t, p, 3)
				#define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)
				#define FxaaTexGreen4(t, p) textureGather(t, p, 1)
				#define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)
			#endif
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_HLSL_3 == 1) || (FXAA_360 == 1) || (FXAA_PS3 == 1)
			#define FxaaInt2 float2
			#define FxaaTex sampler2D
			#define FxaaTexTop(t, p) tex2Dlod(t, float4(p, 0.0, 0.0))
			#define FxaaTexOff(t, p, o, r) tex2Dlod(t, float4(p + (o * r), 0, 0))
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_HLSL_4 == 1)
			#define FxaaInt2 int2
			struct FxaaTex { SamplerState smpl; Texture2D tex; };
			#define FxaaTexTop(t, p) t.tex.SampleLevel(t.smpl, p, 0.0)
			#define FxaaTexOff(t, p, o, r) t.tex.SampleLevel(t.smpl, p, 0.0, o)
		#endif
		/*--------------------------------------------------------------------------*/
		#if (FXAA_HLSL_5 == 1)
			#define FxaaInt2 int2
			struct FxaaTex { SamplerState smpl; Texture2D tex; };
			#define FxaaTexTop(t, p) t.tex.SampleLevel(t.smpl, p, 0.0)
			#define FxaaTexOff(t, p, o, r) t.tex.SampleLevel(t.smpl, p, 0.0, o)
			#define FxaaTexAlpha4(t, p) t.tex.GatherAlpha(t.smpl, p)
			#define FxaaTexOffAlpha4(t, p, o) t.tex.GatherAlpha(t.smpl, p, o)
			#define FxaaTexGreen4(t, p) t.tex.GatherGreen(t.smpl, p)
			#define FxaaTexOffGreen4(t, p, o) t.tex.GatherGreen(t.smpl, p, o)
		#endif


		/*============================================================================
						   GREEN AS LUMA OPTION SUPPORT FUNCTION
		============================================================================*/
		#if (FXAA_GREEN_AS_LUMA == 0)
			FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.w; }
		#else
			FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.y; }
		#endif    

	?>


	/*============================================================================
								 FXAA3 QUALITY - PC
	============================================================================*/
	__shadercall float4 fxaaQuality( float2 pos, float4 fxaaConsolePosPos, FxaaTex tex,	FxaaTex fxaaConsole360TexExpBiasNegOne,	FxaaTex fxaaConsole360TexExpBiasNegTwo,	float2 fxaaQualityRcpFrame,	float4 fxaaConsoleRcpFrameOpt, float4 fxaaConsoleRcpFrameOpt2, float4 fxaaConsole360RcpFrameOpt2, float fxaaQualitySubpix, float fxaaQualityEdgeThreshold, float fxaaQualityEdgeThresholdMin, float fxaaConsoleEdgeSharpness, float fxaaConsoleEdgeThreshold, float fxaaConsoleEdgeThresholdMin, float4 fxaaConsole360ConstDir ) 
	{
	<?
	/*--------------------------------------------------------------------------*/
		FxaaFloat2 posM;
		posM.x = pos.x;
		posM.y = pos.y;
		#if (FXAA_GATHER4_ALPHA == 1)
			#if (FXAA_DISCARD == 0)
				FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);
				#if (FXAA_GREEN_AS_LUMA == 0)
					#define lumaM rgbyM.w
				#else
					#define lumaM rgbyM.y
				#endif
			#endif
			#if (FXAA_GREEN_AS_LUMA == 0)
				FxaaFloat4 luma4A = FxaaTexAlpha4(tex, posM);
				FxaaFloat4 luma4B = FxaaTexOffAlpha4(tex, posM, FxaaInt2(-1, -1));
			#else
				FxaaFloat4 luma4A = FxaaTexGreen4(tex, posM);
				FxaaFloat4 luma4B = FxaaTexOffGreen4(tex, posM, FxaaInt2(-1, -1));
			#endif
			#if (FXAA_DISCARD == 1)
				#define lumaM luma4A.w
			#endif
			#define lumaE luma4A.z
			#define lumaS luma4A.x
			#define lumaSE luma4A.y
			#define lumaNW luma4B.w
			#define lumaN luma4B.z
			#define lumaW luma4B.x
		#else
			FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);
			#if (FXAA_GREEN_AS_LUMA == 0)
				#define lumaM rgbyM.w
			#else
				#define lumaM rgbyM.y
			#endif
			FxaaFloat lumaS = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0, 1), fxaaQualityRcpFrame.xy));
			FxaaFloat lumaE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 0), fxaaQualityRcpFrame.xy));
			FxaaFloat lumaN = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0,-1), fxaaQualityRcpFrame.xy));
			FxaaFloat lumaW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 0), fxaaQualityRcpFrame.xy));
		#endif
	/*--------------------------------------------------------------------------*/
		FxaaFloat maxSM = max(lumaS, lumaM);
		FxaaFloat minSM = min(lumaS, lumaM);
		FxaaFloat maxESM = max(lumaE, maxSM);
		FxaaFloat minESM = min(lumaE, minSM);
		FxaaFloat maxWN = max(lumaN, lumaW);
		FxaaFloat minWN = min(lumaN, lumaW);
		FxaaFloat rangeMax = max(maxWN, maxESM);
		FxaaFloat rangeMin = min(minWN, minESM);
		FxaaFloat rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;
		FxaaFloat range = rangeMax - rangeMin;
		FxaaFloat rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);
		FxaaBool earlyExit = range < rangeMaxClamped;
	/*--------------------------------------------------------------------------*/
		if(earlyExit)
			#if (FXAA_DISCARD == 1)
				FxaaDiscard;
			#else
				return rgbyM;
			#endif
	/*--------------------------------------------------------------------------*/
		#if (FXAA_GATHER4_ALPHA == 0)
			FxaaFloat lumaNW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1,-1), fxaaQualityRcpFrame.xy));
			FxaaFloat lumaSE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 1), fxaaQualityRcpFrame.xy));
			FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1,-1), fxaaQualityRcpFrame.xy));
			FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));
		#else
			FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(1, -1), fxaaQualityRcpFrame.xy));
			FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));
		#endif
	/*--------------------------------------------------------------------------*/
		FxaaFloat lumaNS = lumaN + lumaS;
		FxaaFloat lumaWE = lumaW + lumaE;
		FxaaFloat subpixRcpRange = 1.0/range;
		FxaaFloat subpixNSWE = lumaNS + lumaWE;
		FxaaFloat edgeHorz1 = (-2.0 * lumaM) + lumaNS;
		FxaaFloat edgeVert1 = (-2.0 * lumaM) + lumaWE;
	/*--------------------------------------------------------------------------*/
		FxaaFloat lumaNESE = lumaNE + lumaSE;
		FxaaFloat lumaNWNE = lumaNW + lumaNE;
		FxaaFloat edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
		FxaaFloat edgeVert2 = (-2.0 * lumaN) + lumaNWNE;
	/*--------------------------------------------------------------------------*/
		FxaaFloat lumaNWSW = lumaNW + lumaSW;
		FxaaFloat lumaSWSE = lumaSW + lumaSE;
		FxaaFloat edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
		FxaaFloat edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
		FxaaFloat edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
		FxaaFloat edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
		FxaaFloat edgeHorz = abs(edgeHorz3) + edgeHorz4;
		FxaaFloat edgeVert = abs(edgeVert3) + edgeVert4;
	/*--------------------------------------------------------------------------*/
		FxaaFloat subpixNWSWNESE = lumaNWSW + lumaNESE;
		FxaaFloat lengthSign = fxaaQualityRcpFrame.x;
		FxaaBool horzSpan = edgeHorz >= edgeVert;
		FxaaFloat subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
	/*--------------------------------------------------------------------------*/
		if(!horzSpan) lumaN = lumaW;
		if(!horzSpan) lumaS = lumaE;
		if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;
		FxaaFloat subpixB = (subpixA * (1.0/12.0)) - lumaM;
	/*--------------------------------------------------------------------------*/
		FxaaFloat gradientN = lumaN - lumaM;
		FxaaFloat gradientS = lumaS - lumaM;
		FxaaFloat lumaNN = lumaN + lumaM;
		FxaaFloat lumaSS = lumaS + lumaM;
		FxaaBool pairN = abs(gradientN) >= abs(gradientS);
		FxaaFloat gradient = max(abs(gradientN), abs(gradientS));
		if(pairN) lengthSign = -lengthSign;
		FxaaFloat subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);
	/*--------------------------------------------------------------------------*/
		FxaaFloat2 posB;
		posB.x = posM.x;
		posB.y = posM.y;
		FxaaFloat2 offNP;
		offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;
		offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;
		if(!horzSpan) posB.x += lengthSign * 0.5;
		if( horzSpan) posB.y += lengthSign * 0.5;
	/*--------------------------------------------------------------------------*/
		FxaaFloat2 posN;
		posN.x = posB.x - offNP.x * FXAA_QUALITY__P0;
		posN.y = posB.y - offNP.y * FXAA_QUALITY__P0;
		FxaaFloat2 posP;
		posP.x = posB.x + offNP.x * FXAA_QUALITY__P0;
		posP.y = posB.y + offNP.y * FXAA_QUALITY__P0;
		FxaaFloat subpixD = ((-2.0)*subpixC) + 3.0;
		FxaaFloat lumaEndN = FxaaLuma(FxaaTexTop(tex, posN));
		FxaaFloat subpixE = subpixC * subpixC;
		FxaaFloat lumaEndP = FxaaLuma(FxaaTexTop(tex, posP));
	/*--------------------------------------------------------------------------*/
		if(!pairN) lumaNN = lumaSS;
		FxaaFloat gradientScaled = gradient * 1.0/4.0;
		FxaaFloat lumaMM = lumaM - lumaNN * 0.5;
		FxaaFloat subpixF = subpixD * subpixE;
		FxaaBool lumaMLTZero = lumaMM < 0.0;
	/*--------------------------------------------------------------------------*/
		lumaEndN -= lumaNN * 0.5;
		lumaEndP -= lumaNN * 0.5;
		FxaaBool doneN = abs(lumaEndN) >= gradientScaled;
		FxaaBool doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P1;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P1;
		FxaaBool doneNP = (!doneN) || (!doneP);
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P1;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P1;
	/*--------------------------------------------------------------------------*/
		if(doneNP) {
			if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
			if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
			if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
			if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
			doneN = abs(lumaEndN) >= gradientScaled;
			doneP = abs(lumaEndP) >= gradientScaled;
			if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P2;
			if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P2;
			doneNP = (!doneN) || (!doneP);
			if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P2;
			if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P2;
	/*--------------------------------------------------------------------------*/
			#if (FXAA_QUALITY__PS > 3)
			if(doneNP) {
				if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
				if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
				if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
				if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
				doneN = abs(lumaEndN) >= gradientScaled;
				doneP = abs(lumaEndP) >= gradientScaled;
				if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P3;
				if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P3;
				doneNP = (!doneN) || (!doneP);
				if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P3;
				if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P3;
	/*--------------------------------------------------------------------------*/
				#if (FXAA_QUALITY__PS > 4)
				if(doneNP) {
					if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
					if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
					if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
					if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
					doneN = abs(lumaEndN) >= gradientScaled;
					doneP = abs(lumaEndP) >= gradientScaled;
					if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P4;
					if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P4;
					doneNP = (!doneN) || (!doneP);
					if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P4;
					if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P4;
	/*--------------------------------------------------------------------------*/
					#if (FXAA_QUALITY__PS > 5)
					if(doneNP) {
						if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
						if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
						if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
						if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
						doneN = abs(lumaEndN) >= gradientScaled;
						doneP = abs(lumaEndP) >= gradientScaled;
						if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P5;
						if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P5;
						doneNP = (!doneN) || (!doneP);
						if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P5;
						if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P5;
	/*--------------------------------------------------------------------------*/
						#if (FXAA_QUALITY__PS > 6)
						if(doneNP) {
							if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
							if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
							if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
							if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
							doneN = abs(lumaEndN) >= gradientScaled;
							doneP = abs(lumaEndP) >= gradientScaled;
							if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P6;
							if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P6;
							doneNP = (!doneN) || (!doneP);
							if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P6;
							if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P6;
	/*--------------------------------------------------------------------------*/
							#if (FXAA_QUALITY__PS > 7)
							if(doneNP) {
								if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
								if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
								if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
								if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
								doneN = abs(lumaEndN) >= gradientScaled;
								doneP = abs(lumaEndP) >= gradientScaled;
								if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P7;
								if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P7;
								doneNP = (!doneN) || (!doneP);
								if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P7;
								if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P7;
	/*--------------------------------------------------------------------------*/
		#if (FXAA_QUALITY__PS > 8)
		if(doneNP) {
			if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
			if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
			if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
			if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
			doneN = abs(lumaEndN) >= gradientScaled;
			doneP = abs(lumaEndP) >= gradientScaled;
			if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P8;
			if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P8;
			doneNP = (!doneN) || (!doneP);
			if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P8;
			if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P8;
	/*--------------------------------------------------------------------------*/
			#if (FXAA_QUALITY__PS > 9)
			if(doneNP) {
				if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
				if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
				if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
				if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
				doneN = abs(lumaEndN) >= gradientScaled;
				doneP = abs(lumaEndP) >= gradientScaled;
				if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P9;
				if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P9;
				doneNP = (!doneN) || (!doneP);
				if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P9;
				if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P9;
	/*--------------------------------------------------------------------------*/
				#if (FXAA_QUALITY__PS > 10)
				if(doneNP) {
					if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
					if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
					if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
					if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
					doneN = abs(lumaEndN) >= gradientScaled;
					doneP = abs(lumaEndP) >= gradientScaled;
					if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P10;
					if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P10;
					doneNP = (!doneN) || (!doneP);
					if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P10;
					if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P10;
	/*--------------------------------------------------------------------------*/
					#if (FXAA_QUALITY__PS > 11)
					if(doneNP) {
						if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
						if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
						if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
						if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
						doneN = abs(lumaEndN) >= gradientScaled;
						doneP = abs(lumaEndP) >= gradientScaled;
						if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P11;
						if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P11;
						doneNP = (!doneN) || (!doneP);
						if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P11;
						if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P11;
	/*--------------------------------------------------------------------------*/
						#if (FXAA_QUALITY__PS > 12)
						if(doneNP) {
							if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
							if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
							if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
							if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
							doneN = abs(lumaEndN) >= gradientScaled;
							doneP = abs(lumaEndP) >= gradientScaled;
							if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P12;
							if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P12;
							doneNP = (!doneN) || (!doneP);
							if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P12;
							if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P12;
	/*--------------------------------------------------------------------------*/
						}
						#endif
	/*--------------------------------------------------------------------------*/
					}
					#endif
	/*--------------------------------------------------------------------------*/
				}
				#endif
	/*--------------------------------------------------------------------------*/
			}
			#endif
	/*--------------------------------------------------------------------------*/
		}
		#endif
	/*--------------------------------------------------------------------------*/
							}
							#endif
	/*--------------------------------------------------------------------------*/
						}
						#endif
	/*--------------------------------------------------------------------------*/
					}
					#endif
	/*--------------------------------------------------------------------------*/
				}
				#endif
	/*--------------------------------------------------------------------------*/
			}
			#endif
	/*--------------------------------------------------------------------------*/
		}
	/*--------------------------------------------------------------------------*/
		FxaaFloat dstN = posM.x - posN.x;
		FxaaFloat dstP = posP.x - posM.x;
		if(!horzSpan) dstN = posM.y - posN.y;
		if(!horzSpan) dstP = posP.y - posM.y;
	/*--------------------------------------------------------------------------*/
		FxaaBool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
		FxaaFloat spanLength = (dstP + dstN);
		FxaaBool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
		FxaaFloat spanLengthRcp = 1.0/spanLength;
	/*--------------------------------------------------------------------------*/
		FxaaBool directionN = dstN < dstP;
		FxaaFloat dst = min(dstN, dstP);
		FxaaBool goodSpan = directionN ? goodSpanN : goodSpanP;
		FxaaFloat subpixG = subpixF * subpixF;
		FxaaFloat pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
		FxaaFloat subpixH = subpixG * fxaaQualitySubpix;
	/*--------------------------------------------------------------------------*/
		FxaaFloat pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
		FxaaFloat pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
		if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
		if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
		#if (FXAA_DISCARD == 1)
			return FxaaTexTop(tex, posM);
		#else
			return FxaaFloat4(FxaaTexTop(tex, posM).xyz, lumaM);
		#endif
		?>
	}

	//-----------------------------------------------------------------------------
	// Name : psFXAA (Pixel Shader)
	// Desc : Runs the FXAA pixel shader by Timothy Lotte (version 3.11)
	//-----------------------------------------------------------------------------
    bool fxaa( bool ReverseToneMap )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4  color          : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            cbAntialiasing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		
		// Prepare the required data
		float2 rcpFrame     = textureSize.zw; 
		float4 rcpFrameOpt  = float4( 2.0f, 2.0f, 0.5f, 0.5f ) * textureSize.zwzw; 
		float4 posPos       = texCoords.xyxy + float4( -rcpFrameOpt.zw, rcpFrameOpt.zw );
		float4 rcpFrameCon  = textureSize.zwzw * float4( -0.5, -0.5, 0.5, 0.5 );
		float4 rcpFrameCon2 = textureSize.zwzw * float4( -2.0, -2.0, 2.0, 2.0 );
		float4 rcpFrame3602 = textureSize.zwzw * float4( 8.0, 8.0, -4.0, -4.0 );

		// Run the shader		
		float4 fxaaResult   = fxaaQuality( texCoords, posPos, sImage, sImage, sImage, rcpFrame, rcpFrameCon, rcpFrameCon2, rcpFrame3602, 0.50, 0.063, 0.0625, 8.0, 0.125, 0.05, float4( 0, 0, 0, 0 ) );

		// Store a low res version of the pixel speed (used during temporal AA resolve).
		float2 velocity = sample2D( sVelocityTex, sVelocity, texCoords );
		fxaaResult.a = sqrt( 5.0 * length( velocity ) );

		// Return final result
		color = fxaaResult;
		?>
		
		// If requested, call a reverse tonemapping operation (exponential). We use this to undo a 
		// prior temporary tonemapping that allows the fxaa to better process color data. That tonemapper
		// is always expontial with an exposure of 0.0001, and squared to approximate sRGB (i.e., y = ( 1.0f - exp( -color * 0.0001 ) ) ^ 2)
		if ( ReverseToneMap )
			<?color.rgb = -log( 1.0f - sqrt( color.rgb ) ) / 0.01;?> 
		
		// Success
		return true;
	}
	
	//-----------------------------------------------------------------------------
	// Name : pixelVelocity()
	// Desc : Computes per pixel velocities (based on prev/curr camera matrices).
	//-----------------------------------------------------------------------------
    bool pixelVelocity( int depthType )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4  color          : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			_cbCamera;
            cbAntialiasing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Compute previous and current clip positions
		float  eyeZ             = decompressF32( sample2D( sDepthTex, sDepth, texCoords ).rgb ) * _cameraInverseRangeScale + _cameraInverseRangeBias;
 		float3 viewPosition     = float3( texCoords * _cameraScreenToViewScale + _cameraScreenToViewBias, 1.0f ) * eyeZ;
		float3 worldPosition    = mul( float4( viewPosition, 1 ), _inverseViewMatrix );

		float2 clipPositionCurr = texCoords - float2( 0.5, 0.5 );
		float4 clipPositionPrev = mul( float4( worldPosition, 1 ), previousViewProjMatrix );
		clipPositionPrev.xy    /= clipPositionPrev.w;
		clipPositionPrev.xy    *= float2( 0.5, -0.5 );

		// Compute relative velocity
		color = float4( clipPositionCurr.xy - clipPositionPrev.xy, 0, 0 );
		?>
		
		// Success
		return true;
	}
	
	//-----------------------------------------------------------------------------
	// Name : temporalResolve()
	// Desc : Blends between current and previous buffers based on a velocity based weight.
	//-----------------------------------------------------------------------------
	bool temporalResolve()
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4  color          : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			_cbCamera;
            cbAntialiasing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Sample pixel velocity
		float2 velocity = -sample2D( sVelocityTex, sVelocity, texCoords ).rg;

		// Fetch current and previous pixels
		float4 colorCurr = sample2D( sImageTex, sImage, texCoords ); 
		float4 colorPrev = sample2D( sImagePrevTex, sImagePrev, texCoords + velocity ); 

		// Attenuate the previous pixel if the velocity is different:
		float delta  = abs( colorCurr.a * colorCurr.a - colorPrev.a * colorPrev.a ) / 5.0;
		float weight = 0.5f * saturate( 1.0f - ( sqrt( delta ) * reprojectionWeight ) );

		// Disable blending above a certain pixel velocity (reduces motion blur artifacts)
		weight *= saturate( (resolveMaxSpeed - length(velocity) ) * 10000.0 );
	 
		// Blend
		color = lerp( colorCurr, colorPrev, weight );
		?>
		
		// Success
		return true;
	}



   
} // End Class : AntialiasingShader