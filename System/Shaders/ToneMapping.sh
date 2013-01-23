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
		float whiteBias          = 0.0;
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

	<?cbuffer cbDownsample : register(b11), globaloffset(c210)
		float4 lumTextureSize  = { 0.0, 0.0, 0.0, 0.0 };
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
		
        // Compute luminance and log of luminance
		lumData.x = max( minLuminance, computeLuminance( sample ) );
		lumData.y = log( lumData.x + 1e-7f );
		lumData.z = lumData.x;
		lumData.w = lumData.x;
		color = lumData;
		?>

        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : luminanceDownsample()
	// Desc : Downsamples the luminance data
	//-----------------------------------------------------------------------------
    bool luminanceDownsample( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4  screenPosition : SV_POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4  color       : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            cbDownsample;
			_cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Compute texture coords
        float2 texCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;
		
		// Sample the 4 texels
		float4 s0 = sample2D( sLightingPointTex, sLightingPoint, texCoords + float2( -0.5, -0.5 ) * lumTextureSize.zw );
		float4 s1 = sample2D( sLightingPointTex, sLightingPoint, texCoords + float2(  0.5, -0.5 ) * lumTextureSize.zw );
		float4 s2 = sample2D( sLightingPointTex, sLightingPoint, texCoords + float2( -0.5,  0.5 ) * lumTextureSize.zw );
		float4 s3 = sample2D( sLightingPointTex, sLightingPoint, texCoords + float2(  0.5,  0.5 ) * lumTextureSize.zw );

		// xy are averages
		color.xy = (s0.xy + s1.xy + s2.xy + s3.xy) * 0.25f;
		
		// zw are min & max
		color.z  = min( s0.z, min( s1.z, min( s2.z, s3.z ) ) );
		color.w  = max( s0.w, max( s1.w, max( s2.w, s3.w ) ) );
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
    
		// Complete the log average for luminance
		cacheEntry.y = exp( cacheEntry.y );
		?>
			
		// Reinhard automatic parameter generation 
		if ( toneMapper == ToneMapMethod::PhotographicWhitePoint )
		{
			<?
			float eps  = 1e-6f;
			float La   = cacheEntry.y + eps;
			float Lmin = cacheEntry.z + eps;	
			float Lmax = cacheEntry.w + eps;
			cacheEntry.z = 0.18f * pow( 4.0f, max( 0.00001f, (2.0 * log2( La ) - log2( Lmin ) - log2( Lmax ) ) / (log2( Lmax ) - log2( Lmin ) + 1e-6f) ) );
			cacheEntry.w = max( 0.95f, 1.50f * exp2( log2(Lmax) - log2(Lmin) - 5.0f ) );
			?>
		}
		else
		{
			<?
			cacheEntry.z = 1.03f - ( 2.0f / ( 2.0f + log10( cacheEntry.y + 1.0f ) ) );
			cacheEntry.w = maxLuminance;  
			?>		
		}
		<?		
		// Apply user scale and bias to adjust/override values    
		cacheEntry.z = cacheEntry.z * keyScale + keyBias;
		cacheEntry.w = cacheEntry.w * whiteScale + whiteBias;

		// Final exposure			
		cacheEntry.z /= (cacheEntry.y + 1e-7f);
		
		// Return results
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
			<?color = sample2D( sLuminanceCacheTex, sLuminanceCache, texCoords + float2( 0.5, 0.5 ) * cacheTextureSize.zw );?>
		}
		else
		{
			<?
			// Get last frame's data 
			float4 previousValues = sample2D( sLuminancePrevTex, sLuminancePrev, float2( 0.5, 0.5 ) ); 

			// Get current frame's data
			float4 currentValues = sample2D( sLuminanceCacheTex, sLuminanceCache, texCoords + float2( 0.5, 0.5 ) * cacheTextureSize.zw );
			
			// Simulate eye adaptation by interpolating based on elapsed time and current lighting conditions
			float adaptationRate = lerp( coneTime, rodTime, saturate( rodSensitivity / (rodSensitivity + currentValues.x) ) );
			color = lerp( previousValues, currentValues, 1.0 - exp( -adaptationRate * _elapsedTime ) );
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
		float exposure = lumData.z; 
		float white    = lumData.w;

		// Get our current pixel's HDR color
		float4 hdrColor = sample2D( sLightingPointTex, sLightingPoint, texCoords ); 
		
		// Tonemap
		ldrColor = toneMapping( hdrColor.rgb, exposure, white, $toneMapper );	
        ?>
		
		// Non-filmic tonemappers require gamma correction. Note: This is temporary and will be removed once tonemapping becomes a color op.
		if ( toneMapper != ToneMapMethod::Filmic )		
		{
			<?
			ldrColor.rgb = pow( ldrColor.rgb, 1.0f / 2.2f );		
			
			// Temporary contrast adjustment
			float luminance    = dot( ldrColor.rgb, float3( 0.2125, 0.7154, 0.0721 ) ) + 1e-7f; 
			float luminanceAdj = saturate( (luminance - 0.5) * 1.1 + 0.5 );
			ldrColor.rgb       = ldrColor.rgb * (luminanceAdj / luminance);
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