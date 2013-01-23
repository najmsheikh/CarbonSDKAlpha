///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "sys://Shaders/Mesh.sh"

///////////////////////////////////////////////////////////////////////////////
// Prebuilt String Table
///////////////////////////////////////////////////////////////////////////////
const String drawZeroMaskShader                    = "drawZeroMask";

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return FirstPersonWeaponShader( owner, driver, resources );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : FirstPersonWeaponShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for first person
//        weapon / character models.
//-----------------------------------------------------------------------------
class FirstPersonWeaponShader : MeshShader
{
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    // Note: Currently, derived shaders must re-declare samplers and other
    // class level items such as local constant buffers, etc.
    <?samplers
        Sampler2D     sDiffuse              : register(s0);
        Sampler2D     sNormal               : register(s1);
        Sampler2D     sSpecular             : register(s2);
        Sampler2D     sReflection           : register(s3);
        SamplerCube   sReflectionCube       : register(s3);
        Sampler2D     sEmissive             : register(s4);
        Sampler2D     sOpacity              : register(s5); 
        Sampler2D     sLight                : register(s6);

        Sampler2D     sLighting             : register(s9);
        Sampler2D     sDepth                : register(s10);
        Sampler2D     sSurfaceNormal        : register(s11);

        // Material internal samplers.
        Sampler2D     sTransmissionCurve    : register(s7);
        Sampler2D     sNormalsFit           : register(s8);
    ?>    

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : FirstPersonWeaponShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	FirstPersonWeaponShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Call the base class constructor
        super( owner, driver, resources );
    }

    ///////////////////////////////////////////////////////////////////////////
    // Techniques
    ///////////////////////////////////////////////////////////////////////////

    TechniqueResult motionBlurMask( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setRasterizerState( null );
        mDriver.setDepthStencilState( mDepthLessEqualState );
		mDriver.setBlendState( mAlphaOnlyBlendState );

        // Select shaders (Note: We force a world space normal computation so that a worldIT matrix gets generated and our world matrix becomes identical to the geometry pass. This fixes skinning z fighting.)
        if ( !mOwner.selectVertexShader( transformDepthShader, System.maxBlendIndex, System.useVTFBlending, System.depthType, int(NormalType::NormalWorld), int(LightType::NoLight), System.materialFlags, System.renderFlags ) ||
             !mOwner.selectPixelShader( drawZeroMaskShader ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    ///////////////////////////////////////////////////////////////////////////
    // Shader Functions
    ///////////////////////////////////////////////////////////////////////////

    //-------------------------------------------------------------------------
    // Name : transformDefault() (Vertex Shader)
    // Desc : Outputs vertex data needed for lighting and g-buffer filling.
    //-------------------------------------------------------------------------
	bool transformDefault( int maxBlendIndex, bool useVTFBlending, int normalType, int lightmapType, bool useViewSpace, bool orthographicCamera )
	{
        // Call base class implementation
        if ( !MeshShader::transformDefault( maxBlendIndex, useVTFBlending, normalType, lightmapType, useViewSpace, orthographicCamera ) )
            return false;

		// Do not use temporal jittering 
		<?clipPosition.xy += _cameraJitterAA * clipPosition.w;?>

        // Tweak the final computed clip space Z value to adjust the depth range for the 
        // first person geometry. It should then no longer clip through walls or be
        // clipped (as much) by the camera near plane.
        <?
            clipPosition.z += 0.1f;
            clipPosition.z *= 0.2f;
        ?>

        // Adjust output camera space linear depth too in case readable
        // depth stencil buffer is not available.
		if ( orthographicCamera )
			<?depth = clipPosition.z;?>  
		else
			<?depth = (clipPosition.w + 0.1f) * _cameraRangeScale + _cameraRangeBias;?>  

        // Valid shader
        return true;
	}
	
    //-------------------------------------------------------------------------
    // Name : transformDepth() (Vertex Shader)
    // Desc : Outputs vertex data needed for depth pass.
    //-------------------------------------------------------------------------
	bool transformDepth( int maxBlendIndex, bool useVTFBlending, int depthOutputType, int normalOutputType, int lightType, int materialFlags, int renderFlags )
	{
        // Call base class implementation
        if ( !MeshShader::transformDepth( maxBlendIndex, useVTFBlending, depthOutputType, normalOutputType, lightType, materialFlags, renderFlags ) )
            return false;

		// Do not use temporal jittering 
		if ( lightType == LightType::NoLight )
		{
			<?clipPosition.xy += _cameraJitterAA * clipPosition.w;?>
		
		} // End if not filling a shadow map

        // Tweak the final computed clip space Z value to adjust the depth range for the 
        // first person geometry. It should then no longer clip through walls or be
        // clipped (as much) by the camera near plane.
        <?
            clipPosition.z += 0.1f;
            clipPosition.z *= 0.2f;
        ?>

        // Adjust output camera space linear depth too in case readable
        // depth stencil buffer is not available.
		bool orthographicCamera = testFlagAny( renderFlags, RenderFlags::OrthographicCamera );
		if ( orthographicCamera )
			<?depth = clipPosition.z;?>  
		else
			<?depth = (clipPosition.w + 0.1f) * _cameraRangeScale + _cameraRangeBias;?>  

        // Valid shader
        return true;
	}
	
    //-------------------------------------------------------------------------
    // Name : drawZeroMask() (Pixel Shader)
    // Desc : Outputs a mask of 0.
    //-------------------------------------------------------------------------
	bool drawZeroMask( )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 screenPosition : SV_POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4 color : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?color = float4( 0, 0, 0, 0 );?>

        // Valid shader
        return true;
	}
   
} // End Class : FirstPersonWeaponShader
