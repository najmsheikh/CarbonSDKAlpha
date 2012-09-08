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
// Name : Atmospheric.sh                                                     //
//                                                                           //
// Desc : Internal surface shader which provides support for skyboxes,       //
//        global fog, and related effects.                                   //
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
    return AtmosphericShader( owner, driver, resources );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : AtmosphericShader (Class)
// Desc : Material shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for atmospheric
//        effects.
//-----------------------------------------------------------------------------
class AtmosphericShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;         // The parent application object.
    private RenderDriver@           mDriver;        // Render driver to which this shader is linked.
    private ResourceManager@        mResources;     // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        SamplerCube sSkyBox     : register(s0);
        Sampler2D   sDepth      : register(s0);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : AtmosphericShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	AtmosphericShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
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

	//-----------------------------------------------------------------------------
	// Name : transform()
	// Desc : Basic transform vertex shader
	//-----------------------------------------------------------------------------
	bool transform( bool passTexCoords, bool passEyeDir )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 inClipPosition   : POSITION;
        ?>

        // Define shader outputs.
		<?out
			float4 clipPosition     : SV_POSITION;
		?>
		if ( passTexCoords && passEyeDir )
		{
			<?out
				float2 texCoords    : TEXCOORD0;
				float3 eyeDir       : TEXCOORD1;
			?>
		}
		else if ( passTexCoords )
		{
			<?out
				float2 texCoords    : TEXCOORD0;
			?>
		}
		else if ( passEyeDir )
		{
			<?out
				float3 eyeDir       : TEXCOORD0;
			?>	
		}

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Compute clip position
		clipPosition = float4( sign( inClipPosition.xy ), 1.0, 1.0 );
		?>			

		// Compute texture coordinates
		if ( passTexCoords )
		{
			<?
			// Get the screen coords into range [0,1]
			texCoords = float2( clipPosition.x, -clipPosition.y ) * 0.5 + 0.5;

			// Adjust screen coords to take into account the currently set viewport and half pixel offset.
			texCoords = texCoords.xy * _screenUVAdjustScale + _screenUVAdjustBias;
			?>
		}

		// Compute eye direction
		if ( passEyeDir )
		{
			<?
			// Reverse the projection process. First multiply by 'W' (we want it to
			// sit at the far plane) and the the inverse projection matrix.
			float3 viewPosition = mul( float4( clipPosition.x * _cameraFar, clipPosition.y * _cameraFar, _cameraFar, _cameraFar ), _inverseProjectionMatrix );
		    
			// We are expected to output a world space direction vector so re-orientate
			// based on the inverse of the view matrix (only the upper 3x3)
			eyeDir = mul( viewPosition, _inverseViewMatrix );
			?>
		}

		// Valid shader
		return true;
	}

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------------
	// Name : drawSkyBox()
	// Desc : Simple cubemap skybox shader
	// ToDo : Temporary ILR alpha output. Should come from texture. HDR should
	//        also be provided via texture (perhaps just lookup a different sampler?). 
	//-----------------------------------------------------------------------------
    bool drawSkyBox( bool decodeSRGB )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4 screenPosition : SV_POSITION;
			float3 eyeDir         : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4 skyColor : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?skyColor = sampleCube( sSkyBoxTex, sSkyBox, eyeDir );?>	

		if ( decodeSRGB )
			<?skyColor.rgb = pow( skyColor.rgb, 2.2 );?>  // ToDo: 6767 - Get intensity from CF!

		<?	
			//skyColor  *= _skyIntensity; ToDo: 6767 - Get intensity from CF
			skyColor.a = dot( skyColor.rgb, float3( 0.2125, 0.7154, 0.0721 ) );
		?>

        // Valid shader
        return true;
    }

	//-----------------------------------------------------------------------------
	// Name : drawFog()
	// Desc : Fog pixel shader
	//-----------------------------------------------------------------------------
    bool drawFog( int fogModel, int depthType )
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
            float4 color : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?float distance;?>
		getDistance( "distance", "sDepthTex", "sDepth", "texCoords", depthType );
		<?
        float factor = computeFogFactor( distance, $fogModel );
		color = float4( _fogColor.rgb, factor );
        ?>

        // Valid shader
        return true;
    }

} // End Class : AtmosphericShader
