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
    <?cbuffer cbUpSample : register(b12), globaloffset(c180)
		float  srcIntensity;
		float  dstIntensity;
    ?>

    <?cbuffer cbBrightPass : register(b13), globaloffset(c182)
		float  brightRangeScale  = 0.0;
		float  brightRangeBias   = 0.0;
		float2 recipTextureSize  = { 0.0, 0.0 };
    ?>
    
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D sSource   : register(s0);
        Sampler2D sPrevious : register(s1);
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
    bool brightPass( bool hdr, bool downSample )
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
		int numSamples = downSample ? 4 : 1;
        if ( downSample )
        {
	        <?
			const float2 offset[ 4 ] = 
			{
			   float2( -0.5, -0.5 ),
			   float2(  0.5, -0.5 ),
			   float2( -0.5,  0.5 ),
			   float2(  0.5,  0.5 )
			};
			?>
		}
		else
		{
			<?const float2 offset[ 1 ] = { float2( 0, 0 ) };?>
		}
			
        <?float4 sample, finalColor = 0;?>

        for ( int i = 0; i < numSamples; i++ )
        {
            // Sample texel
            <?sample = sample2D( sSourceTex, sSource, texCoords + offset[ $i ] * recipTextureSize );?>
            
            // Test for overflow cases and clamp to maximum when HDR is enabled
            if ( hdr )
                <?sample.rgb = min( sample.rgb, $HDR_FLOAT_MAX );?>

            <?
            // Use normalized color for weighted averaging. This allows all channels to play an equal role (versus luminance, which heavily favors green).
            sample.a = computeIntensity( sample.rgb );
            
            // Accumulate results
            finalColor.rgb += sample.rgb * saturate( sample.a * brightRangeScale + brightRangeBias );
            ?>
        }

        // Return final color (averaged if downsampled)
        if ( downSample ) 
	        <?color = finalColor * 0.25f;?>
	    else    
	        <?color = finalColor;?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : upSample() (Pixel Shader)
    // Desc : Combine the results of the various glare passes into a final 
    //        image via upsampling.
    //-------------------------------------------------------------------------
    bool upSample( )
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
            cbUpsample;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?color = float4( sample2D( sSourceTex, sSource, texCoords ).rgb * srcIntensity, dstIntensity );?>

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
        ?>
    
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Get previous and current frames' data 
		float4 previousValues = sample2D( sPreviousTex, sPrevious, float2( 0.5, 0.5 ) ); 
		float4 currentValues  = sample2D( sSourceTex, sSource, float2( 0.5, 0.5 ) );

		float  minLuminance = 0.05f;
		float  maxLuminance = 0.95f;
		float  lumScale     = 1.0f / (maxLuminance - minLuminance);
		float  lumBias      = -minLuminance * lumScale;
		
		float  currentLuminance = dot( currentValues.rgb, float3( 0.2125, 0.7154, 0.0721 ) );
		float  weight = saturate( currentLuminance * lumScale + lumBias );
		currentValues *= weight;

		// Simulate eye adaptation by interpolating based on elapsed time
		float deltaT = min( 1.0f / 30.0f, _elapsedTime ); 
		float adaptationRate = 0.25f;
		color = lerp( previousValues, currentValues, 1.0 - exp( -deltaT / adaptationRate ) );
		?>

		// Return valid shader
		return true;
	}

} // End Class : Glare
