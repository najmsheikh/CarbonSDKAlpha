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
// Name : ToneMapping.sh                                                     //
//                                                                           //
// Desc : Internal surface shader which provides support for HDR.            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "ImageProcessing.sh"

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENTSCRIPT
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return ToneMappingShader( owner, driver, resources );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : ToneMappingShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for HDR to LDR.
//-----------------------------------------------------------------------------
class ToneMappingShader : ImageProcessingShader
{
    ///////////////////////////////////////////////////////////////////////////
    // Custom Constant Buffer Declarations
    ///////////////////////////////////////////////////////////////////////////
	<?cbuffer cbToneMapping : register(b9), globaloffset(c200)
		float keyScale           = 1.0;
		float keyBias            = 0.0;
		float whiteScale         = 1.0;
		float whiteBias          = 1.0;
		float prF                = 1.0;
		float prM                = 1.0;
		float prC                = 1.0;
		float prA                = 1.0;
	?>    
    
	<?cbuffer cbLuminance  : register(b10), globaloffset(c210)
		float  coneTime          = 0.0;
		float  rodTime           = 0.0;
		float  rodSensitivity    = 0.0;
		float  minLuminance      = 0.0;
		float  maxLuminance      = 0.0;
		float4 lumTextureSize    = { 0.0, 0.0, 0.0, 0.0 };
		float4 cacheTextureSize  = { 0.0, 0.0, 0.0, 0.0 };
	?>
    
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
		Sampler2D sLighting             : register(s0);
		Sampler2D sLightingPoint        : register(s1);
        Sampler2D sLuminance            : register(s2);
        Sampler2D sLuminanceAverage     : register(s3);
        Sampler2D sLuminanceCache       : register(s4);
        Sampler2D sLuminancePrev        : register(s5);
        Sampler2D sLuminanceCurr        : register(s6);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : ToneMappingShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	ToneMappingShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
		// Call the base class constructor
		super( owner, driver, resources );
    }

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------------
	// Name : luminanceCompute()
	// Desc : Computes the luminance and log luminance values for a pixel
	//-----------------------------------------------------------------------------
    bool luminanceCompute( int toneMapper )
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
            float4  color       : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            cbLuminance;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		float4 lumData = 0;
		float3 sample = sample2D( sLightingTex, sLighting, texCoords ).rgb;
		
		// Clamp to maximum allowed HDR values
		sample = min( sample, $HDR_FLOAT_MAX );

        // Compute luminance and log of luminance
		lumData.x = computeLuminance( sample );
		lumData.y = log( lumData.x + 1e-6f );

		color = lumData;
		?>

        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : luminanceUpdateCache (Pixel Shader)
	// Desc : Updates the luminance cache depending on selected tonemapper. Also does
	//        last minute safety tests to ensure no out of range numbers are used. 
	//-----------------------------------------------------------------------------
	bool luminanceUpdateCache( int toneMapper )
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
            float4  color       : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            cbToneMapping;
            cbLuminance;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
 		// Read the downsampled data
		float4 cacheEntry = sample2D( sLuminanceAverageTex, sLuminanceAverage, texCoords );

		// Make sure that we have legitimate values to work with
		float2 maxLuminanceVals = float2( $HDR_FLOAT_MAX, log( $HDR_FLOAT_MAX ) );
		cacheEntry.xy = clamp( cacheEntry.xy, 0.0, maxLuminanceVals );
	    
		// Complete the log average for luminance
		cacheEntry.y = exp( cacheEntry.y );
	
		// Compute key and whitepoint 
		cacheEntry.z = 1.03 - ( 2.0 / ( 2.0 + log10( cacheEntry.y + 1.0 ) ) );
		cacheEntry.w = maxLuminance;  
		
		// Apply user scale and bias to adjust/override values    
		cacheEntry.z = cacheEntry.z * keyScale + keyBias;
		cacheEntry.w = cacheEntry.w * whiteScale + whiteBias;

		color = cacheEntry;
		?>

        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : luminanceAdapt (Pixel Shader)
	// Desc : Adaptation/smoothing for luminance between previous and current frames
	//-----------------------------------------------------------------------------
	bool luminanceAdapt( bool adaptation, bool enableCache )
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
            float4  color       : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            cbLuminance;
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		// If we are not using adaptation (e.g., on first frame or for testing), sample the luminance cache for current data and return immediately.
		if ( !adaptation )
		{
			<?color = sample2D( sLuminanceCacheTex, sLuminanceCache, texCoords + float2( -0.5, -0.5 ) * cacheTextureSize.zw );?>
		}
		else
		{
			<?
			// Get last frame's data 
			float4 previousValues = sample2D( sLuminancePrevTex, sLuminancePrev, float2( 0.5, 0.5 ) ); 

			// Get current frame's data
			float4 currentValues = sample2D( sLuminanceCacheTex, sLuminanceCache, texCoords + float2( -0.5, -0.5 ) * cacheTextureSize.zw );
			
			// Simulate eye adaptation by interpolating based on elapsed time and current lighting conditions
			float adaptationRate = lerp( coneTime, rodTime, rodSensitivity / (rodSensitivity + currentValues.y ) );
			color = lerp( previousValues, currentValues, 1.0 - exp( -_elapsedTime / adaptationRate ) );
			?>
		}

        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : ToneMap
	// Desc : Tone mapping operation (HDR->LDR).
	// Note : Image operations can be run post-tonemapping. 
	// Note : The use of multi-image operation support means that this shader should
	//        be created once and cached locally by the caller if post-ops are
	//        used. If no operations are used beyond the basic tonemapper, then this 
	//        is unnecessary and the standard shader cache system can be employed.  
	//-----------------------------------------------------------------------------
	bool toneMap( int toneMapper )
	{
		int[] postOperations; 
		return toneMap( toneMapper, postOperations );
	}
	bool toneMap( int toneMapper, const int[] & postOperations )
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
            float4  color       : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            cbToneMapping;
            cbLuminance;
            cbImageProcessing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		float4 ldrColor = 0;

		// Get our scene luminance data
		float4 lumData = sample2D( sLuminanceCurrTex, sLuminanceCurr, float2( 0.5, 0.5 ) );
		float la       = lumData.y;
		float key      = lumData.z;
		float white    = lumData.w;

		// Compute auto-exposure scalar
		float exposure = key / la;

		// Get our current pixel's HDR color
		float4 hdrColor = sample2D( sLightingPointTex, sLightingPoint, texCoords ); 
		
		// Ensure hdr color is within acceptable range
		hdrColor.rgb = min( hdrColor.rgb, $HDR_FLOAT_MAX );
	
		// Compute the world luminance
		float lw = computeLuminance( hdrColor );
        ?>

		// Select tonemapper
		if ( toneMapper == ToneMapMethod::Photographic )
		{
			<?
			// Apply auto-exposure 
			float lm = lw * exposure;
			
			// Compute white point luminance
			float lmax        = white * exposure;
			float recipLmaxSq = 1.0 / (lmax * lmax);
			float lb = saturate( lm * lm * recipLmaxSq );

			// Compute display luminance
			float ld = (lm + lb) / (lm + 1.0);
		
			// Convert HDR color to LDR color
			ldrColor.rgb = hdrColor.rgb * (ld / lw);
			?>
		}
		else if ( toneMapper == ToneMapMethod::Filmic )
		{
			<?
			// Place the white point in alpha
			hdrColor.a = (white + 1e-7f);

			// Apply automatic exposure
			hdrColor *= exposure; 
			
			// Run the tonemap operation
			hdrColor = (hdrColor * (6.2 * hdrColor + 0.5)) / (hdrColor * (6.2 * hdrColor + 1.7) + 0.06);

			// Apply a white point (not originally a part of the algorithm, but seems to work ok)
			//ldrColor.rgb = hdrColor.rgb / hdrColor.a;
			ldrColor.rgb = hdrColor.rgb;
			?>
		}
		else if ( toneMapper == ToneMapMethod::ExponentialTM )
		{
			<?
			// Apply exposure and compute display luminance 
			float ld = 1.0f - exp( -lw * exposure );
			
			// Convert HDR color to LDR color
			ldrColor.rgb = hdrColor.rgb * ( ld / max( lw, 0.0001f ) );
			?>
		}
		
		// Non-filmic tonemappers require gamma correction. Note: This is temporary and will be removed once tonemapping becomes a color op.
		if ( toneMapper != ToneMapMethod::Filmic )		
		{
			<?
			ldrColor.rgb = pow( ldrColor.rgb, 1.0f / 2.2f );
			?>		
		}
	
		// Prior to running any final color corrections, place the adaptation luminance in the alpha channel for potential use by utilities
		if ( postOperations.length() > 0 )
		{
			<?ldrColor.a = la;?>
			runOperations( "color", "ldrColor", postOperations );
		}
		else
		{
			<?
			color   = ldrColor;
			color.a = dot( color.rgb, float3( 0.299, 0.587, 0.114 ) );
			?>
		}
		
        // Valid shader
        return true;
	}

} // End Class : ToneMappingShader