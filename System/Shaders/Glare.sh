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
// Name : Glare.sh                                                           //
//                                                                           //
// Desc : Internal surface shader which provides support for glare.          //
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
    return GlareShader( owner, driver, resources );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : GlareShader (Class)
// Desc : Material shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for the glare
//        image processing effect.
//-----------------------------------------------------------------------------
class GlareShader : ISurfaceShader
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
    <?cbuffer cbAddLayer : register(b11), globaloffset(c190)
		float  layerWeight;
    ?>

    <?cbuffer cbUpdateCache : register(b12), globaloffset(c180)
		float  blendAmount;
		float  blendRate;
    ?>

    <?cbuffer cbBrightPass : register(b13), globaloffset(c182)
		float  brightRangeScale  = 0.0;
		float  brightRangeBias   = 0.0;
		float  alphaMaskAmt      = 0.0;
		float2 recipTextureSize  = { 0.0, 0.0 };
    ?>

    <?cbuffer cbILRBrightPass : register(b12), globaloffset(c180)
		float  brightThreshold;
		float  flareContrast;
		float2 distanceAttenScaleBias;
    ?>

    <?cbuffer cbILRCompositePass : register(b13), globaloffset(c182)
		float  flareDelta;
		float  flareAlpha;
		float  flareSlice;
    ?>

    <?cbuffer cbAnamorphicFlare : register(b12), globaloffset(c180)
		float3 flareColor;
		float  flarePassScale;
		float  flareCoordScale;
		float  flareCoordBias;
    ?>
   
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D sSource    : register(s0);
        Sampler2D sLuminance : register(s1);
        Sampler2D sFlare     : register(s2);
        Sampler2D sDepth     : register(s3);
    ?>
 
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : GlareShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	GlareShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner     = owner;
        @mDriver    = driver;
        @mResources = resources;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : brightPass() (Pixel Shader)
    // Desc : Make a pass over the input render target and build an image that
    //        represents the luminance of the image with values above a certain
    //        threshold. Optionally does initial 2x2 downsample at same time.
    //-------------------------------------------------------------------------
    bool brightPass( bool linearSampling, int toneMappingMethod, bool downSample, bool glowMask )
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
            cbBrightPass;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?float4 finalColor;?>
        if ( downSample && !linearSampling )
        {
			<?
			finalColor  = sample2D( sSourceTex, sSource, texCoords + float2( -0.5, -0.5 ) * recipTextureSize );
			finalColor += sample2D( sSourceTex, sSource, texCoords + float2(  0.5, -0.5 ) * recipTextureSize );
			finalColor += sample2D( sSourceTex, sSource, texCoords + float2( -0.5,  0.5 ) * recipTextureSize );
			finalColor += sample2D( sSourceTex, sSource, texCoords + float2(  0.5,  0.5 ) * recipTextureSize );
	        finalColor *= 0.25f;
	        ?>
		}
		else
		{
			<?finalColor = sample2D( sSourceTex, sSource, texCoords );?>
		}
			
		<?float intensity;?>

		// If HDR, optionally use a tonemapper to get intensity into the [0,1] range
        if ( toneMappingMethod >= 0 )
        {
			<?
			// Get our scene luminance data
			float4 lumData = sample2D( sLuminanceTex, sLuminance, float2( 0.5, 0.5 ) );
			float la       = lumData.y;
			float exposure = lumData.z;
			float white    = lumData.w;

			// Ensure hdr color is within acceptable range
			float4 ldrColor = toneMapping( finalColor.rgb, exposure, white, $toneMappingMethod );	
            intensity = max( ldrColor.r, max( ldrColor.g, ldrColor.b ) );
            
            // Apply auto-exposure to the color
            finalColor *= exposure;
            ?>
		}
		else
		{
			<?intensity = max( finalColor.r, max( finalColor.g, finalColor.b ) );?>
		}

		// First compute an adjustment factor based on intensity.
		<?float colorAdjust = saturate( intensity * brightRangeScale + brightRangeBias );?>
		
		// Include an optional additive alpha channel mask (i.e., glow mask).
		if ( glowMask )
			<?colorAdjust = saturate( colorAdjust + finalColor.a * alphaMaskAmt );?>

        // Use intensity range to adjust the final output color
		<?color = finalColor * colorAdjust;?>
		
        // Valid shader
        return true;
    }
	
    //-------------------------------------------------------------------------
    // Name : updateCache() (Pixel Shader)
    // Desc : Combine the results of the previous and current glare passes.
    //-------------------------------------------------------------------------
    bool updateCache( )
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
            _cbScene;
            cbUpdateCache;
        ?>
    
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?color = float4( sample2D( sSourceTex, sSource, texCoords ).rgb, blendAmount * saturate( blendRate /_elapsedTime ) );?>

		// Return valid shader
		return true;
	}

    //-------------------------------------------------------------------------
    // Name : addLayer() (Pixel Shader)
    // Desc : Combine the results of the previous glare level with the current. 
    //        Returns the weight for the current level in alpha for dest blend.
    //-------------------------------------------------------------------------
    bool addLayer( )
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
            _cbScene;
            cbAddLayer;
        ?>
    
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?color = float4( sample2D( sSourceTex, sSource, texCoords ).rgb, layerWeight );?>

		// Return valid shader
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// ILR
	///////////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------------
	// Name : flareBrightPass() (Pixel Shader)
	// Desc : Make a pass over the input render target and build an image that
	//        represents the luminance of the image with values above a certain
	//        threshold.
	//-----------------------------------------------------------------------------
    bool flareBrightPass( bool applyContrast, bool materialAttenuationInAlpha, int toneMappingMethod )
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
            cbILRBrightPass;
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
			float eps = 1e-6f;
		    float4 ldrColor; 			                          // We need an LDR color for our computations		    
			float3 flareBaseColor = float3(1.0f, 0.8f, 0.5f);     // Base color that describes the color to write for a fully saturated flare (during bright pass).

			// Sample scene color
			float4 sceneColor = sample2D( sSourceTex, sSource, texCoords );
			
			// Compute luminance
			float luminance = computeLuminance( sceneColor.rgb );

    		// Compute distance attenuation
			float eyeZ  = decompressF32( sample2D( sDepthTex, sDepth, texCoords ).rgb ) * _cameraInverseRangeScale + _cameraInverseRangeBias;
			float atten = 1.0f - saturate( eyeZ * distanceAttenScaleBias.x + distanceAttenScaleBias.y );
		?>
		
	    // Apply material attenuation (Optionally computed during a prior pass and stored in the input alpha channel.)
		if ( materialAttenuationInAlpha )
		    <?atten *= sceneColor.a;?>
		
		// If HDR, optionally use a tonemapper to get intensity into the [0,1] range
        if ( toneMappingMethod >= 0 )
        {
			<?
			// Get our scene luminance data
			float4 lumData = sample2D( sLuminanceTex, sLuminance, float2( 0.5, 0.5 ) );
			float la       = lumData.y;
			float exposure = lumData.z;
			float white    = lumData.w;

			// Ensure hdr color is within acceptable range
			ldrColor = toneMapping( sceneColor.rgb, exposure, white, $toneMappingMethod );	
			
			// Apply auto-exposure to luminance
			luminance *= exposure;
            ?>
		}
		else
		{
			<?ldrColor = sceneColor;?>	
		}			
		<?
			// Compute ldr luminance
			float ldrLuminance = computeLuminance( ldrColor.rgb );
	
			// Remove pixels below luminance threshold and distance attenuate.
			if ( ldrLuminance < brightThreshold )
				atten = 0;

			// Compute color saturation
			float saturation;
			float cmax = max (ldrColor.r, max (ldrColor.g, ldrColor.b));
			float cmin = min (ldrColor.r, min (ldrColor.g, ldrColor.b));
			if ( ldrLuminance < 0.5f )
			  saturation = (cmax - cmin) / ((cmax + cmin) + eps);
			else
			  saturation = (cmax - cmin) / ((2.0 - (cmax + cmin)) + eps);

			// Compute final flare color.
			sceneColor.rgb = lerp( flareBaseColor, float3(1,1,1), saturate( saturation ) ) * luminance;
		?>    
			// Apply contrast? (Note: Only ldr.)
			if ( applyContrast )
				<?sceneColor.rgb = saturate( sceneColor.rgb * flareContrast + (0.5 - (0.5 * flareContrast)) );?>
		<?    
			// Return attenuated color. 
			color = float4( sceneColor.rgb * atten, 0.0f );
		?>
        
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : flareDownsample() (Pixel Shader)
	// Desc : Downsamples the high detail flare target.
	//-----------------------------------------------------------------------------
    bool flareDownsample( )
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

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		// Sample original flare color computed during bright pass.
		<?color = sample2D( sSourceTex, sSource, texCoords );?>
        
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : flareDraw() (Pixel Shader)
	// Desc : Sample the flare texture and tint as required based on the provided
	//        flare color map.
	//-----------------------------------------------------------------------------
    bool flareDraw( )
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
            cbILRCompositePass;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Sample original flare color computed during bright pass.
		float3 flareColor = sample2D( sSourceTex, sSource, texCoords );
		
		// Tint the flare based on the supplied 1D color map
		flareColor *= pow( sample2D( sFlareTex, sFlare, float2( flareSlice, 0.5f ) ).rgb, 2.2 );

		flareColor *= float3( 0.55, 0.55, 1.0f );
	
		// Return final color and blend using specified alpha
		color = float4( flareColor * flareAlpha, 1 );
		?>
        
        // Valid shader
        return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Anamorphic Flares
	///////////////////////////////////////////////////////////////////////////////

    //-------------------------------------------------------------------------
    // Name : anamorphicFlare() (Pixel Shader)
    // Desc : Runs a vertical sampling for anamorphic flares
    // Note : This is all still a work in progress. Not nearly close to complete.
    //-------------------------------------------------------------------------
    bool anamorphicFlare( int radius, int passDistance, int passFalloff, bool lastPass )
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
            _cbScene;
            _cbCamera;
            cbAnamorphicFlare;
        ?>
    
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		// Take the center sample
		<?
		float  vignetteBias  = 0.25f;
		float  vignetteScale = 0.75f;
		float  falloff, sqLength;
		float2 sampleCoords, clipCoords;
		float4 sample;
		
		// For each pass, we will increase our sampling distance
		float2 offsetUV = _targetSize.zw * float2( $passDistance, 0 );

		// Take the center sample
		float4 c = sample2D( sSourceTex, sSource, texCoords );
		?>
		
		// Take samples left and right of center
		for ( int i = -radius; i <= radius; i++ )
		{
			if ( i != 0 )
			{
				<?
				// Compute the sample coordinates
				sampleCoords = texCoords + float2( $i, 0 ) * offsetUV;
				
				// Take the sample
				sample = sample2D( sSourceTex, sSource, sampleCoords );

				// Compute a falloff based on pixel distance and the current pass
				falloff = pow( 0.995f, $passFalloff * abs( $i ) );
			
				// Reduce the influence of bright areas towards the edge of the screen 
				clipCoords = sampleCoords * 2.0f - 1.0f;
				sqLength   = dot( clipCoords, clipCoords );
				falloff   *= saturate( 1.0f - (sqLength - flareCoordBias) * flareCoordScale );

				// Weight and accumulate
				c += sample * falloff;
				?>
			}
		}
		
		// On the last pass, scale by the flare color		
		if ( lastPass )
			<?c.rgb *= flareColor;?>
		
		// Adjust final result by a user defined scale		
		<?color = c * flarePassScale;?>

		// Return valid shader
		return true;
	}

} // End Class : Glare
