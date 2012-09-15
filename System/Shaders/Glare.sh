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
   
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D sSource    : register(s0);
        Sampler2D sLuminance : register(s1);
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

    //-------------------------------------------------------------------------
    // Name : verticalBrightPass() (Pixel Shader)
    // Desc : Runs a bright pass based on vertical samples (for anamorphic flares)
    // Note : This is all still a work in progress. Not nearly close to complete.
    //-------------------------------------------------------------------------
    bool verticalBrightPass( int radius )
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
            cbBrightPass;
        ?>
    
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		// Take the center sample
		<?
		float4 sample;
		float4 c = sample2D( sSourceTex, sSource, texCoords );
		float4 center = c;
		float  lum, centerLum = computeLuminance( center.rgb );
		centerLum *= 0.8;
		?>
		for ( int i = -radius; i <= radius; i++ )
		{
			if ( i != 0 )
			{
				<?
				sample = sample2D( sSourceTex, sSource, texCoords + float2( 0, $i ) * _targetSize.zw );
				lum    = computeLuminance( sample.rgb );
				if ( lum < centerLum )
					c += sample;
				?>
			}
		}
		<?
		// Complete the average
		float r = $radius;
		c *= 1.0 / (r * 2.0 + 1.0);

		// Bright pass the result
		float intensity   = max( c.r, max( c.g, c.b ) );
		float colorAdjust = saturate( intensity * brightRangeScale + brightRangeBias );
		
        // Use intensity range to adjust the final output color
		color = c * colorAdjust;
        ?>

		// Return valid shader
		return true;
	}



} // End Class : Glare
