///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "sys://Shaders/Mesh.sh"

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

    //-------------------------------------------------------------------------
    // Name : transformDefault() (Vertex Shader)
    // Desc : Outputs vertex data needed for lighting and g-buffer filling.
    //-------------------------------------------------------------------------
	bool transformDefault( int maxBlendIndex, bool useVTFBlending, int normalType, int lightmapType, bool useViewSpace, bool orthographicCamera )
	{
        // Call base class implementation
        if ( !MeshShader::transformDefault( maxBlendIndex, useVTFBlending, normalType, lightmapType, useViewSpace, orthographicCamera ) )
            return false;

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

    
} // End Class : FirstPersonWeaponShader
