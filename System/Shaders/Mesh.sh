///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "Utilities.shh"
#include "Types.shh"
#include "Config.shh"
#include "Lighting.shh"

///////////////////////////////////////////////////////////////////////////////
// Prebuilt String Table
///////////////////////////////////////////////////////////////////////////////
const String transformDepthShader               = "transformDepth";
const String transformDefaultShader             = "transformDefault";
const String transformReflectiveShadowMapShader = "transformReflectiveShadowMap";

const String drawDepthShader                    = "drawDepth";
const String drawGBufferShader                  = "drawGBuffer";
const String drawOpacityShader                  = "drawOpacity";
const String drawDirectLightingShader           = "drawDirectLighting";
const String drawPrecomputedLightingShader      = "drawPrecomputedLighting";
const String drawFogShader                      = "drawFog";
const String drawReflectiveShadowMapShader      = "drawReflectiveShadowMap";
const String drawGBufferDSShader                = "drawGBufferDS";

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return MeshShader( owner, driver, resources );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : MeshShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for scene meshes.
//-----------------------------------------------------------------------------
class MeshShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;             // The parent application object.
    private RenderDriver@           mDriver;            // Render driver to which this shader is linked.
    private ResourceManager@        mResources;         // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////
    // Depth Stencil States
    ///////////////////////////////////////////////////////////////
    private DepthStencilStateHandle mDepthFillState;
    private DepthStencilStateHandle mDepthEqualState;
    private DepthStencilStateHandle mDepthLessEqualState;

    ///////////////////////////////////////////////////////////////
    // Blend States
    ///////////////////////////////////////////////////////////////
    private BlendStateHandle        mNoColorWritingBlendState;
    private BlendStateHandle        mAdditiveBlendState;
    private BlendStateHandle        mAlphaBlendState;
    private BlendStateHandle        mOpacityBlendState;

    ///////////////////////////////////////////////////////////////
    // Rasterizer States
    ///////////////////////////////////////////////////////////////
    private RasterizerStateHandle   mBackFaceRasterizerState;
    private RasterizerStateHandle   mFrontFaceRasterizerState;

    ///////////////////////////////////////////////////////////////
    // Sampler States
    ///////////////////////////////////////////////////////////////
    private SamplerStateHandle      mGBufferSamplerState;

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
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
	// Name : MeshShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	MeshShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner     = owner;
        @mDriver    = driver;
        @mResources = resources;
        
        ///////////////////////////////////////////////////////////////
        // Depth Stencil States
        ///////////////////////////////////////////////////////////////
        DepthStencilStateDesc dsStates;

        // Depth buffer filling passes (depth, shadowmap, etc.)
        dsStates.depthEnable                = true;
        dsStates.depthWriteEnable           = true;
        dsStates.depthFunction              = ComparisonFunction::LessEqual;
        resources.createDepthStencilState( mDepthFillState, dsStates, 0, DebugSource() );

        // Depth equality state (g-buffer pass, forward lighting, etc.)
        dsStates.depthWriteEnable           = false;
        dsStates.depthFunction              = ComparisonFunction::Equal;
        resources.createDepthStencilState( mDepthEqualState, dsStates, 0, DebugSource() );

        ///////////////////////////////////////////////////////////////
        // Blend States
        ///////////////////////////////////////////////////////////////
        BlendStateDesc blStates;
        
        // Additive blending
        blStates.renderTarget0.blendEnable      = true;
        blStates.renderTarget0.sourceBlend      = BlendMode::One;
        blStates.renderTarget0.destinationBlend = BlendMode::One;
        resources.createBlendState( mAdditiveBlendState, blStates, 0, DebugSource() );

        // Standard alpha blend state
        blStates.renderTarget0.sourceBlend      = BlendMode::SrcAlpha;
        blStates.renderTarget0.destinationBlend = BlendMode::InvSrcAlpha;
        resources.createBlendState( mAlphaBlendState, blStates, 0, DebugSource() );

        // Opacity blend state
        blStates.renderTarget0.sourceBlend      = BlendMode::Zero;
        blStates.renderTarget0.destinationBlend = BlendMode::InvSrcAlpha;
        resources.createBlendState( mOpacityBlendState, blStates, 0, DebugSource() );

        // Disable color output
        blStates.renderTarget0.renderTargetWriteMask = 0;
        blStates.renderTarget0.blendEnable      = false;
        blStates.renderTarget0.sourceBlend      = BlendMode::Zero;
        blStates.renderTarget0.destinationBlend = BlendMode::Zero;
        resources.createBlendState( mNoColorWritingBlendState, blStates, 0, DebugSource() );

        ///////////////////////////////////////////////////////////////
        // Rasterizer States
        ///////////////////////////////////////////////////////////////
        RasterizerStateDesc rsStates;

        // Back face rendering (cull front faces)
        rsStates.cullMode = CullMode::Front;
        resources.createRasterizerState( mBackFaceRasterizerState, rsStates, 0, DebugSource() );

        // Front face rendering (cull back faces)
        rsStates.cullMode = CullMode::Back;
        resources.createRasterizerState( mFrontFaceRasterizerState, rsStates, 0, DebugSource() );
        
        ///////////////////////////////////////////////////////////////
        // Sampler States
        ///////////////////////////////////////////////////////////////
        SamplerStateDesc smpStates;

        // G-buffer texture reading
        smpStates.addressU            = AddressingMode::Clamp;
        smpStates.addressV            = AddressingMode::Clamp;
        smpStates.minificationFilter  = FilterMethod::Point;
        smpStates.magnificationFilter = FilterMethod::Point;
        smpStates.mipmapFilter        = FilterMethod::None;
        resources.createSamplerState( mGBufferSamplerState, smpStates, 0, DebugSource() );
    }

    ///////////////////////////////////////////////////////////////////////////
    // Techniques
    ///////////////////////////////////////////////////////////////////////////

    TechniqueResult depth( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setRasterizerState( null );
        mDriver.setDepthStencilState( mDepthFillState );
		if ( System.depthType == DepthType::None )
			mDriver.setBlendState( mNoColorWritingBlendState );
		else
	        mDriver.setBlendState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( transformDepthShader, System.maxBlendIndex, System.useVTFBlending, System.depthType, System.surfaceNormalType, int(LightType::NoLight), System.materialFlags, System.renderFlags ) ||
             !mOwner.selectPixelShader( drawDepthShader, System.depthType, System.surfaceNormalType, System.materialFlags, System.renderFlags ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    TechniqueResult renderToShadowMap( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthFillState );
        if ( System.colorWrites == ColorChannel::None )
            mDriver.setBlendState( mNoColorWritingBlendState );
        else
            mDriver.setBlendState( null );

		if ( System.cullMode == CullMode::Front )
			mDriver.setRasterizerState( mBackFaceRasterizerState );
        else
			mDriver.setRasterizerState( mFrontFaceRasterizerState );
		
		// Turn off group ID compute flag 
        int renderFlagsNoGroup = System.renderFlags & ~int( RenderFlags::ComputeGroupID );
        
        // Select shaders
        if ( !mOwner.selectVertexShader( transformDepthShader, System.maxBlendIndex, System.useVTFBlending, int(DepthType::LinearZ), int(NormalType::NoNormal), System.lightType, System.materialFlags, renderFlagsNoGroup ) ||
             !mOwner.selectPixelShader( drawDepthShader, int(DepthType::LinearZ), int(NormalType::NoNormal), System.materialFlags, renderFlagsNoGroup ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    TechniqueResult renderToReflectiveShadowMap( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthFillState );
        mDriver.setBlendState( null );
        mDriver.setRasterizerState( null );
		
		// Turn off group ID compute flag 
        int renderFlagsNoGroup = System.renderFlags & ~int( RenderFlags::ComputeGroupID );
        
        // Select shaders
        if ( !mOwner.selectVertexShader( transformReflectiveShadowMapShader, System.maxBlendIndex, System.useVTFBlending, System.lightType, System.materialFlags, renderFlagsNoGroup ) ||
             !mOwner.selectPixelShader( drawReflectiveShadowMapShader, System.lightType, System.materialFlags, renderFlagsNoGroup ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    TechniqueResult geometry( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthEqualState );
        mDriver.setBlendState( null );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( transformDefaultShader, System.maxBlendIndex, System.useVTFBlending, System.normalSource, System.reflectionMode, System.lightTextureType, false, true, System.viewSpaceLighting, System.orthographicCamera ) ||
             !mOwner.selectPixelShader( drawGBufferDSShader, System.normalSource, System.reflectionMode, System.lightTextureType, System.materialFlags, System.renderFlags, System.outputEncodingType, System.shadingQuality ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    TechniqueResult transparent_Opacity( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthLessEqualState );
        mDriver.setBlendState( mOpacityBlendState );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( transformDepthShader, System.maxBlendIndex, System.useVTFBlending, int(DepthType::LinearDistance), int(NormalType::NormalWorld), int(LightType::NoLight), System.materialFlags, System.renderFlags ) ||
             !mOwner.selectPixelShader( drawOpacityShader, System.sampleOpacityTexture, System.shadingQuality ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    TechniqueResult transparent_DirectLighting( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthLessEqualState );
        mDriver.setBlendState( mAdditiveBlendState );
        mDriver.setRasterizerState( null );
        
        // Will we be projecting a light texture?
        bool projective = ( System.primaryTaps > 0 || System.lightType == LightType::Projector || testFlagAny(System.lightFlags, LightFlags::AttenuationTexture) );

        // Select shaders
        if ( !mOwner.selectVertexShader( transformDefaultShader, System.maxBlendIndex, System.useVTFBlending, System.normalSource, System.reflectionMode, System.lightTextureType, false, true, System.viewSpaceLighting, System.orthographicCamera ) ||
             !mOwner.selectPixelShader( drawDirectLightingShader, System.normalSource, System.lightType, System.shadowMethod, System.primaryTaps, System.secondaryTaps, System.materialFlags, System.lightFlags, System.renderFlags, System.shadingQuality ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    TechniqueResult transparent_Precomputed( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthLessEqualState );
        mDriver.setBlendState( mAdditiveBlendState );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( transformDefaultShader, System.maxBlendIndex, System.useVTFBlending, System.normalSource, System.reflectionMode, System.lightTextureType, false, true, System.viewSpaceLighting, System.orthographicCamera ) ||
             !mOwner.selectPixelShader( drawPrecomputedLightingShader, System.normalSource, System.reflectionMode, System.lightTextureType, System.materialFlags, System.renderFlags, System.shadingQuality ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    TechniqueResult transparent_Fog( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthLessEqualState );
        mDriver.setBlendState( mAlphaBlendState );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( transformDepthShader, System.maxBlendIndex, System.useVTFBlending, int(DepthType::LinearDistance), int(NormalType::NoNormal), int(LightType::NoLight), System.materialFlags, System.renderFlags ) ||
             !mOwner.selectPixelShader( drawFogShader, System.sampleOpacityTexture, System.fogModel ) )
            return TechniqueResult::Abort;

        // We're done 
        return TechniqueResult::Complete; // Continue, Abort
    }

    ///////////////////////////////////////////////////////////////////////////
    // Shader Functions
    ///////////////////////////////////////////////////////////////////////////
    //-----------------------------------------------------------------------------
	// Name : getSurfaceData()
	// Desc : Assembles the surface data needed for lighting
	//-----------------------------------------------------------------------------
	__shadercall void getSurfaceData( inout SurfaceData surface, float3 position, float3 normal, inout float4 texCoords, float2 parallaxDirection, float3x3 tangentToWorldSpace, 
	                                  script int normalType, script int materialFlags, script int renderFlags, script int shadingQuality )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Constant buffer usage.
		<?cbufferrefs
			_cbCamera;
			_cbMaterial;
			_cbObject;
		?>
		
		// Extract the current settings from the relevant flags
		bool useViewSpace        = testFlagAny( renderFlags,   RenderFlags::ViewSpaceLighting );
		bool orthographicCamera  = testFlagAny( renderFlags,   RenderFlags::OrthographicCamera );
		bool computeToksvig      = testFlagAny( materialFlags, MaterialFlags::ComputeToksvig );
		bool hasOpacityTexture   = testFlagAny( materialFlags, MaterialFlags::SampleOpacityTexture );
		bool opacityInDiffuse    = testFlagAny( materialFlags, MaterialFlags::OpacityInDiffuse );
		bool isTranslucent       = testFlagAny( materialFlags, MaterialFlags::Translucent );
		bool manualSRGBLinearize = testFlagAny( materialFlags, MaterialFlags::DecodeSRGB );
		bool surfaceFresnel      = testFlagAny( materialFlags, MaterialFlags::SurfaceFresnel );
		bool isMetal             = testFlagAny( materialFlags, MaterialFlags::Metal );
		bool computeRoughness    = (shadingQuality >= ShadingQuality::HighQuality);
		
		<?float rlen;?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		// Copy the position
		<?surface.position = position;?>
		
		// Determine the view direction (surface to camera)
		if ( useViewSpace )
		{
			if ( orthographicCamera )
				<?surface.viewDirection = float3( 0, 0, -1 );?>
			else
				<?surface.viewDirection = -position;?>
		}
		else
		{
			if ( orthographicCamera )
				<?surface.viewDirection = -_cameraDirection;?>
			else
				<?surface.viewDirection = _cameraPosition - position;?>
		}
		<?
		// Normalize the view ray and compute the distance to the camera
		surface.distanceToCamera  = length( surface.viewDirection );
		surface.viewDirection    /= surface.distanceToCamera;
		?>

		// Compute normal
		if ( normalType == NormalSource::Vertex )
		{
			<?
		    surface.tangentSpaceNormal = half3( 0, 0, 1 );
			surface.surfaceNormal      = normalize( normal.xyz );
			surface.normal             = surface.surfaceNormal;
		    ?>
		}
		else
		{
			// Store the surface bumpiness
			// <?surface.bumpiness = _materialBumpiness;?>
			<?surface.bumpiness = 1.0f;?> // ToDo: 6767 -- Reset for now. Will come in as strength for the normal sampler, _materialBumpiness can be removed from the CB
		
			// Compute texture coord adjustments for parallax mapping (if needed)
			if ( normalType == NormalSource::ParallaxOffset ) 
			{
                <?
				float2 uvAdjust = computeParallaxOffset( sNormalTex, sNormal, texCoords.zw, parallaxDirection );
				texCoords += uvAdjust.xyxy;
                ?>
			
            } // End if parallax offset

			// Retrieve the tangent space normal
			<?surface.tangentSpaceNormal = getTangentSpaceNormal( sNormalTex, sNormal, texCoords.zw, surface.bumpiness );?>
			
			// Compute the inverse bump length for the Toksvig factor
			if ( computeToksvig )
				<?rlen = 1.0 / length( surface.tangentSpaceNormal.xyz );?>
			
			<?
			// Normalize the tangent space normal
			surface.tangentSpaceNormal = normalize( surface.tangentSpaceNormal );
			
			// Compute the world space normal
			surface.normal = normalize( mul( surface.tangentSpaceNormal, tangentToWorldSpace ).xyz );

			// Normalize the surface normal (interpolated vertex)
			surface.surfaceNormal = normalize( tangentToWorldSpace[2].xyz );
			?>
		}

		// Compute diffuse reflectance
		<?
		float4 kD = sample2D( sDiffuseTex, sDiffuse, texCoords.xy );
		       kD.rgb *= _materialDiffuse.rgb;
		?>

		// Compute specular reflectance and gloss
		<?float4 kS = float4( _materialSpecular.rgb, _materialGloss );?>

		// Apply metallic shading before specular masking
		if ( isMetal )
		{
			<?
			kS.rgb = lerp( kS.rgb, kD.rgb * _materialMetalnessSpecular, _materialMetalnessAmount );
			kD.rgb *= _materialMetalnessDiffuse;
			?>
		}
		
		// Do we have a full specular color texture?
		if ( testFlagAny( materialFlags, MaterialFlags::SampleSpecularColor ) )
		{
			// Do we have a gloss map?
			if ( testFlagAny( materialFlags, MaterialFlags::SampleGlossTexture ) )
				<?kS.rgba *= sample2D( sSpecularTex, sSpecular, texCoords.xy );?>
			else
				<?kS.rgb  *= sample2D( sSpecularTex, sSpecular, texCoords.xy ).rgb;?>
		}
		// Do we have a specular mask?
		else if ( testFlagAny( materialFlags, MaterialFlags::SampleSpecularMask ) )
		{
			// Do we have a gloss map?
			if ( testFlagAny( materialFlags, MaterialFlags::SampleGlossTexture ) )
				<?kS.rgba *= sample2D( sSpecularTex, sSpecular, texCoords.xy ).rrrg;?>
			else
				<?kS.rgb  *= sample2D( sSpecularTex, sSpecular, texCoords.xy ).rrr;?>
		}
		// Do we only have a gloss map?
		else if ( testFlagAny( materialFlags, MaterialFlags::SampleGlossTexture ) )
		{
			<?kS.a *= sample2D( sSpecularTex, sSpecular, texCoords.xy ).r;?>
		}
		
		// Get the gloss
		<?surface.gloss = kS.a;?>
		
		// Compute specular power
		<?surface.specularPower = exp2( surface.gloss * $MAX_GLOSS_LEVELS );?>
	
		// Adjust specular power based on Toksvig factor
		if ( computeToksvig )
		{
			<?
			// Adjust power
			surface.specularPower *= ( 1.0 / ( 1.0 + surface.specularPower * (rlen - 1.0) ) );
			
			// Recompute the new gloss
			surface.gloss = log2( surface.specularPower ) * (1.0f / $MAX_GLOSS_LEVELS);
			?>
		}

		// Compute surface roughness 
		if ( computeRoughness )
			<?surface.roughness = sqrt( 2.0f / (surface.specularPower + 2.0f) );?>

        // If the opacity value was not in the diffuse texture, sample it separately.
        if ( hasOpacityTexture && !opacityInDiffuse )
            <?kD.a = sample2D( sOpacityTex, sOpacity, texCoords.xy ).a;?>
        else if ( !opacityInDiffuse )
            <?kD.a = 1.0;?>
		
		// Compute opacity for diffuse and specular reflectance
		if ( isTranslucent )
		{
			<?
			float2 globalOpacity = float2( _materialDiffuse.a, _materialSpecular.a ); // * _objectBlendFactor; ToDo: 6767 - Object blend factor broken.
			float2 localOpacity  = globalOpacity * kD.a;
			float2 opacity       = lerp( globalOpacity, localOpacity, _materialOpacityMapStrength );        
			?>
		
        } // End if isTranslucent
		
		// Apply a Fresnel term based on surface orientation with respect to the camera.
		if ( surfaceFresnel )
		{ 
			<?
			float f = pow( 1.0 - saturate( dot( surface.surfaceNormal, surface.viewDirection ) ), _materialFresnelExponent );
    		kS.rgb  = lerp( kS.rgb, float3(1.0, 1.0, 1.0), f * _materialFresnelSpecular );
			kD.rgb *= 1.0 - (f * _materialFresnelDiffuse);
			?>
			if ( isTranslucent )
				<?opacity = lerp( opacity, float2(1.0, 1.0), f * _materialFresnelTransparent );?>
		
        } // End if high quality shading

		// Premultiply translucency for diffuse and specular (Note: specular separately describes how much translucency it allows)
		if ( isTranslucent )
		{
			<?
			surface.diffuse      = kD.rgb * opacity.x;
			surface.specular     = kS.rgb * opacity.y;
			surface.transmission = (1.0 - opacity.x);
			?>

            // ToDo: Sample from 1D transmission curve texture optionally based on a permutation.
		
        } // End if isTranslucent
		else
		{
			<?
			surface.diffuse      = kD.rgb;
			surface.specular     = kS.rgb;
			surface.transmission = 0;
			?>
		
        } // End if !isTranslucent
		<?	
		// Get ambient occlusion (not supported in forward mode -- yet)
		surface.ambientOcclusion = 1; 
		?>

		// If we need to convert from sRGB to linear, do so now
		if ( manualSRGBLinearize )
		{
			<?
			surface.diffuse.rgb  = SRGBToLinear( surface.diffuse.rgb );
			surface.specular.rgb = SRGBToLinear( surface.specular.rgb );
			?>
		}
	
	}

	//-----------------------------------------------------------------------------
	// Name  : getPrecomputedLighting()
	// Desc  : Retrieves precomputed lighting from lightmaps, reflections, emissives, etc.
	//-----------------------------------------------------------------------------
	__shadercall void getPrecomputedLighting( inout LightingData lighting, SurfaceData surface, float4 reflectionCoords, float4 texCoords, float2 texCoordsLight,
                                              script int reflectionType, script int lightmapType, script int materialFlags, script int renderFlags )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Constant buffer usage.
		<?cbufferrefs
			_cbMaterial;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		// Get lightmap colors
		if ( lightmapType != LightTextureType::None )
			<?getLightmapData( lighting, surface, texCoordsLight, $lightmapType );?>

		// Add scene reflections to our specular lighting
		if ( reflectionType != ReflectionMode::None )
		{
			<?float4 reflectionColor;?>
			if ( reflectionType == ReflectionMode::Planar )
			{
				<?reflectionColor = sample2D( sReflectionTex, sReflection, reflectionCoords.xy / reflectionCoords.w );?>
			
            }	// End if Planar
			else
			{
				<?
				// Compute the reflection vector
				half3 N = lerp( surface.surfaceNormal, surface.normal, _materialReflectionBumpiness.xxx );
				half3 R = reflect( -surface.viewDirection, N );
				?>
				// Environment map lookups must use world space 
				if ( testFlagAny( renderFlags, RenderFlags::ViewSpaceLighting ) )
					<?R = mul( R, (float3x3)_inverseViewMatrix );?>
				
				// Sample the cube texture
				<?reflectionColor = sampleCubeLevel( sReflectionCubeTex, sReflectionCube, R, _materialReflectionMipLevel - surface.gloss * _materialReflectionMipLevel );?>
				
			} // End if EnvironmentMap
			
			// Convert from RGBM as needed (hdr only)
			//if ( testFlagAny( materialFlags, MaterialFlags::DecodeRGBM ) )
			//	<?lighting.reflection = RGBMToRGB( reflectionColor, $maxEnvMapRange );
			//else
			<?lighting.reflection = reflectionColor.rgb;?>
			
			// Linearize reflection color
			//if ( testFlagAny( materialFlags, MaterialFlags::DecodeSRGB ) ) // ToDo: 6767 - Fix this flag.
			if ( testFlagAny( renderFlags, RenderFlags::HDRLighting ) )
				<?lighting.reflection = pow( lighting.reflection, 2.2 );?>
				
            if ( testFlagAny( renderFlags, RenderFlags::HDRLighting ) )
    			<?lighting.specular += lighting.reflection * _materialReflectionIntensity;?> // ToDo: 6767 - Get intensity from CF!
            else
                <?lighting.specular += lighting.reflection * _materialReflectionIntensity;?>
		
        } // End if reflections

		// Add emissive lighting
		<?lighting.emissive = _materialEmissive.rgb;?>
		if ( testFlagAny( materialFlags, MaterialFlags::SampleEmissiveTexture ) )
		{
			<?float3 emissiveTex = sample2D( sEmissiveTex, sEmissive, texCoords.xy ).rgb;?>
			//if ( testFlagAny( materialFlags, MaterialFlags::DecodeSRGB ) ) // ToDo: 6767 - Fix this flag.
			if ( testFlagAny( renderFlags, RenderFlags::HDRLighting ) )
				<?emissiveTex = pow( emissiveTex, 2.2 );?>
				
			<?lighting.emissive += emissiveTex * _materialEmissiveTint.rgb;?>
		}
	}

	//-----------------------------------------------------------------------------
	// Name  : getLightmapData()
	// Desc  : Reconstructs precomputed lighting from lightmaps
	//-----------------------------------------------------------------------------
	__shadercall void getLightmapData( inout LightingData lighting, SurfaceData surface, float2 texCoordsLight, script int lightmapType )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Constant buffer usage.
		<?cbufferrefs
			_cbMaterial;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////        
        // If using radiosity normal mapping...
        if ( lightmapType == LightTextureType::RNM )
        {
            <?
            // Sample directional lightmaps
            float4 l0 = sample2D( sLightTex, sLight, texCoordsLight + float2( 0.5, 0.0 ) );
            float4 l1 = sample2D( sLightTex, sLight, texCoordsLight + float2( 0.0, 0.5 ) );
            float4 l2 = sample2D( sLightTex, sLight, texCoordsLight + float2( 0.5, 0.5 ) );

            // Dot normal with basis vectors (tangent space)
            float3 weights;
            weights.x = saturate( dot( surface.tangentSpaceNormal, float3( -0.40824825,  0.70710671, 0.57735026 ) ) );
            weights.y = saturate( dot( surface.tangentSpaceNormal, float3( -0.40824825, -0.70710671, 0.57735026 ) ) );
            weights.z = saturate( dot( surface.tangentSpaceNormal, float3(  0.81649655,  0.00000000, 0.57735026 ) ) );

            // Square the weights (for a linear falloff) and normalize the results
            weights  = weights * weights;
            weights /= dot( weights, float3( 1.0, 1.0, 1.0 ) );

            // Blend colors based on weights and add to current diffuse lighting
            lighting.diffuse = ( l0.rgb * weights.x ) + ( l1.rgb * weights.y ) + ( l2.rgb * weights.z );
            lighting.shadow  = l0.a;
            ?>

        } // End if RNM
        else if ( lightmapType == LightTextureType::Standard )
        {
            <?
            // Sample the lightmap
            float4 data = sample2D( sLightTex, sLight, texCoordsLight );
            lighting.diffuse = data.rgb;
            
            // Copy the shadow term
            lighting.shadow = data.a;
            ?>
            
        } // End if Standard
	}

	//-----------------------------------------------------------------------------
	// Name : computeProjectiveCoords()
	// Desc : Computes projective texture coordinates for lighting
	//-----------------------------------------------------------------------------
	__shadercall void computeProjectiveCoords( inout SurfaceData surface, float4 texCoordsProj, script int lightType  )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Constant buffer usage.
		<?cbufferrefs
			_cbLight;
		?>

        if ( lightType == LightType::Projector )
        {
            <?cbufferrefs
                _cbProjectorLight;
            ?>
        
        } // End if projector light

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
        surface.projectiveTexCoords = TexCoordsProj;
		?>
        // For projector lights, we have to interpolate our projection window size based on distance along light's z axis
        if ( lightType == LightType::Projector )
        {
            <?
            float2 projDirection = lerp( _lightConeAdjust.xz, _lightConeAdjust.yw, abs( surface.projectiveTexCoords.z ) );
            surface.projectiveTexCoords.xy /= projDirection.xy;
            surface.projectiveTexCoords.xy += 0.5;
            surface.projectiveTexCoords.w   = 1.0;
            ?>
        
        } // End if projector light
        else 
        {
           // Compute the final 2D coordinates (and normalize w)
           <?surface.projectiveTexCoords.xyzw /= surface.projectiveTexCoords.w;?>	
        
        } // End if other light type
	}

    ///////////////////////////////////////////////////////////////////////////
    // Vertex Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : TransformDepth() (Vertex Shader)
    // Desc : Outputs vertex data needed for depth and related passes.
    //-------------------------------------------------------------------------
	bool transformDepth( int maxBlendIndex, bool useVTFBlending, int depthOutputType, int normalOutputType, int lightType, int materialFlags, int renderFlags )
	{
        /////////////////////////////////////////////
        // Setup
        /////////////////////////////////////////////
		bool computeTexCoords   = testFlagAny( renderFlags, RenderFlags::ComputeGroupID ) || testFlagAny( materialFlags, MaterialFlags::SampleOpacityTexture ) || testFlagAny( materialFlags, MaterialFlags::SampleDiffuseTexture );
		bool orthographicCamera = testFlagAny( renderFlags, RenderFlags::OrthographicCamera );

        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition  : POSITION;
            float2  sourceTexCoords : TEXCOORD0;
            float3  sourceNormal    : NORMAL;
        ?>

        // Requires blending inputs?
        if ( maxBlendIndex >= 0 )
        {
            <?in
                float4  blendWeights    : BLENDWEIGHT;
                float4  blendIndices    : BLENDINDICES;
            ?>
            
        } // End if vertex blending

        // Define shader outputs.
		<?out
			float4  clipPosition : SV_POSITION;
			${mapDepthTexCoordRegisters( depthOutputType, normalOutputType, computeTexCoords )}
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?	
		// Get the world matrix 
		matrix mtxToWorld;
        float3x3 mtxToWorldIT;
		?>

        // Vertex blending?
        if ( maxBlendIndex >= 0 )
        {
            if ( normalOutputType == NormalType::NoNormal )
                <?getWorldMatrix( mtxToWorld, blendWeights, blendIndices, $maxBlendIndex, $useVTFBlending );?>
            else	
                <?getWorldMatrixEx( mtxToWorld, mtxToWorldIT, blendWeights, blendIndices, $maxBlendIndex, $useVTFBlending );?>
        
        } // End if blending
        else
        {
            if ( normalOutputType == NormalType::NoNormal )
                <?getWorldMatrix( mtxToWorld );?>
            else	
                <?getWorldMatrixEx( mtxToWorld, mtxToWorldIT );?>
        
        } // End if !blending

		<?
		// Transform to clip space (Note: projector lights compute custom clip positions for shadow map fills).
		float4 worldPosition = mul( float4( sourcePosition, 1 ), mtxToWorld );
        ?>
        if ( lightType == LightType::Projector )
			<?clipPosition = computeProjShadowClipPosition( worldPosition );?>
		else
    		<?clipPosition = mul( worldPosition, _viewProjectionMatrix );?>
			
		// If we are filling a shadow map
		if ( lightType != LightType::NoLight )
		{
			// For directional/projetor lights, the camera is always orthographic
			if ( lightType == LightType::Directional || lightType == LightType::Projector )
				orthographicCamera = true;

			// For point lights, we do a clip position adjustment for possible border repairs
			if ( lightType == LightType::Omni )
			{
				<?
				//clipPosition.x -= _targetSize.z * clipPosition.w;
				//clipPosition.y += _targetSize.w * clipPosition.w;
				?>

			} // End point light

		} // End if filling a shadow map
		else
		{
			// If we are not filling a shadow map, apply any temporal jittering
			<?clipPosition.xy -= _cameraJitterAA * clipPosition.w;?>
		
		} // End if not filling a shadow map

		// Compute depth
		switch( getPureDepthType( depthOutputType ) )
		{
	        case DepthType::LinearZ:
				if ( orthographicCamera )
					<?depth = clipPosition.z;?>  
				else
					<?depth = clipPosition.w * _cameraRangeScale + _cameraRangeBias;?>  
	        break;
	        case DepthType::LinearDistance:
				<?eyeRay = worldPosition - _cameraPosition;?>
	        break;
	        case DepthType::NonLinearZ:
				if ( orthographicCamera )
					<?depth = clipPosition.z;?>  
				else
					<?depth = clipPosition.w;?>  
            break;
		};
	
		// Compute normal
		switch ( normalOutputType )
		{
			case NormalType::NormalWorld:
				<?normal = mul( sourceNormal, mtxToWorldIT );?>
			break; 
			case NormalType::NormalView:
				<?
				normal = mul( sourceNormal, mtxToWorldIT );
				normal = mul( normal, _viewMatrix );
				?>
			break; 
		};

        // Compute texture coordinates for texture map lookups
		if ( computeTexCoords )
			<?texCoords = sourceTexCoords;?>
	
        // Valid shader
        return true;
	}
	
    //-------------------------------------------------------------------------
    // Name : transformDefault() (Vertex Shader)
    // Desc : Outputs vertex data needed for lighting and g-buffer filling.
    //-------------------------------------------------------------------------
	bool transformDefault( int maxBlendIndex, bool useVTFBlending, int normalType, int reflectionType, int lightmapType, bool projectiveCoords, bool screenCoords, bool useViewSpace, bool orthographicCamera )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float3 sourcePosition       : POSITION;
			float3 sourceNormal         : NORMAL;
			float2 sourceTexCoords[3]   : TEXCOORD0;
        ?>

        // Requires tangent frame inputs?
        if ( normalType != NormalSource::Vertex )
        {
            <?in
                float3 sourceTangent        : TANGENT;
			    float3 sourceBinormal       : BINORMAL;
            ?>

        } // End if tangent frame       

        // Requires blending inputs?
        if ( maxBlendIndex >= 0 )
        {
            <?in
                float4  blendWeights    : BLENDWEIGHT;
                float4  blendIndices    : BLENDINDICES;
            ?>
            
        } // End if vertex blending 

        // Define shader outputs.
		<?out
			float4 clipPosition : SV_POSITION;   
			${mapTexCoordRegisters(normalType, reflectionType, lightmapType, projectiveCoords, screenCoords)}
		?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>
        if ( projectiveCoords )
        {
			<?cbufferrefs
				_cbLightingSystem;
			?>
        } // End if projectiveCoords

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?	
		// Start by computing the world matrix (and world IT matrix for normals)
		matrix mtxToWorld;
        float3x3 mtxToWorldIT;
		?>

        if ( maxBlendIndex >= 0 )
		    <?getWorldMatrixEx( mtxToWorld, mtxToWorldIT, blendWeights, blendIndices, $maxBlendIndex, $useVTFBlending );?>
        else
            <?getWorldMatrixEx( mtxToWorld, mtxToWorldIT );?>

		<?		
		// Transform to world and clip space
		float4 worldPosition = mul( float4( sourcePosition, 1 ), mtxToWorld );
		clipPosition         = mul( worldPosition, _viewProjectionMatrix );
		
		// Apply temporal jittering for antialiasing
		clipPosition.xy -= _cameraJitterAA * clipPosition.w;
		
		// Copy out world position for interpolation
		worldPos = worldPosition.xyz;

		// Compute standard texture coordinates
		texCoords.xy = sourceTexCoords[ 0 ];
        texCoords.zw = 0;
		?>

		// Compute eye direction
		if ( orthographicCamera )
			<?float3 eyeRay = -_cameraDirection;?>
		else
			<?float3 eyeRay = _cameraPosition - worldPos;?>

		if ( normalType == NormalSource::Vertex )
		{
			<?
			// Compute normal
			normal = normalize( mul( sourceNormal, mtxToWorldIT ) );
			?>
		}
		else
		{
			<?
			// Compute normal map texture coords
			texCoords.zw = sourceTexCoords[ 0 ];

			// Compute tangent space matrix		
			tangentToWorldSpace[0].xyz = normalize( mul( sourceTangent,  mtxToWorldIT ) );
			tangentToWorldSpace[1].xyz = normalize( mul( sourceBinormal, mtxToWorldIT ) );
			tangentToWorldSpace[2].xyz = normalize( mul( sourceNormal,   mtxToWorldIT ) );
			?>
			if ( normalType == NormalSource::ParallaxOffset )
			{
				<?
				// Compute parallax direction
				parallaxDirection = mul( tangentToWorldSpace, (float3x3)eyeRay ).xy;
				?>
			} // End if ParallaxOffset
		}

		if ( projectiveCoords )
		{
			<?
			// Compute projective texture coords for the current light source
			texCoordsProj = mul( worldPosition, _lightTexProjMatrix );
			?>
		}
		else if ( lightmapType != LightTextureType::None )
		{
			<?
			// Compute lightmap coords
			texCoordsLight = sourceTexCoords[ 1 ];
			?>
		
        } // End if has lightmap

		if ( reflectionType == ReflectionMode::Planar )
		{
			<?
			// Compute planar reflection coords
            computeMeshScreenTextureCoords( clipPosition, reflection );
			?>
			
		}
	
		if ( screenCoords )
		{
			<?
			// Compute screen texture lookup coords
            computeMeshScreenTextureCoords( clipPosition, texCoordsScreen );
			?>
		}
		
		// Transform required items to view space as needed
		if ( useViewSpace )
		{
			<?worldPos = mul( worldPosition, _viewMatrix );?>
			if ( normalType == NormalSource::Vertex )
			{
				<?normal = mul( normal, (float3x3)_viewMatrix );?>
			}
			else
			{
				<?
				tangentToWorldSpace[0] = mul( tangentToWorldSpace[0], (float3x3)_viewMatrix );
				tangentToWorldSpace[1] = mul( tangentToWorldSpace[1], (float3x3)_viewMatrix );
				tangentToWorldSpace[2] = mul( tangentToWorldSpace[2], (float3x3)_viewMatrix );
				?>
			}
		}
	
        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : transformReflectiveShadowMap() (Vertex Shader)
    // Desc : Outputs vertex data needed for RSM passes.
    //-------------------------------------------------------------------------
	bool transformReflectiveShadowMap( int maxBlendIndex, bool useVTFBlending, int lightType, int materialFlags, int renderFlags )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition  : POSITION;
            float2  sourceTexCoords : TEXCOORD0;
            float3  sourceNormal    : NORMAL;
        ?>

        // Requires blending inputs?
        if ( maxBlendIndex >= 0 )
        {
            <?in
                float4  blendWeights    : BLENDWEIGHT;
                float4  blendIndices    : BLENDINDICES;
            ?>
            
        } // End if vertex blending
	
        // Define shader outputs.
		<?out
			float4  clipPosition  : SV_POSITION;
			float   depth         : TEXCOORD0;
			float3  normal        : TEXCOORD1;
			float3  lightDir      : TEXCOORD2;
			float2  texCoords     : TEXCOORD3;
		?>
			
        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbLight;
            _cbLightingSystem;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?	
		// Get the world matrix 
		matrix mtxToWorld;
        float3x3 mtxToWorldIT;
		?>

        // Vertex blending?
        if ( maxBlendIndex >= 0 )
            <?getWorldMatrixEx( mtxToWorld, mtxToWorldIT, blendWeights, blendIndices, $maxBlendIndex, $useVTFBlending );?>
        else
            <?getWorldMatrixEx( mtxToWorld, mtxToWorldIT );?>

		<?
		// Transform to clip space (Note: projector lights compute custom clip positions for shadow map fills).
		float4 worldPosition = mul( float4( sourcePosition, 1 ), mtxToWorld );
        ?>
        if ( lightType == LightType::Projector )
			<?clipPosition = computeProjShadowClipPosition( worldPosition );?>
		else
    		<?clipPosition = mul( worldPosition, _viewProjectionMatrix );?>
			
		// For directional/projetor lights, the camera is orthographic
		if ( lightType == LightType::Directional || lightType == LightType::Projector )
			<?depth = clipPosition.z;?>  
		else
			<?depth = clipPosition.w;?>  
	
		// Compute normal
		<?normal = mul( sourceNormal, mtxToWorldIT );?>

		// Compute light direction
		if ( lightType == LightType::Directional || lightType == LightType::Projector )
			<?lightDir = -_lightDirectionWorld.xyz;?>
		else
			<?lightDir = _lightPositionWorld.xyz - worldPosition.xyz;?>		

        // Compute texture coordinates for texture map lookups
		<?texCoords = sourceTexCoords;?>
	
        // Valid shader
        return true;
	}


    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name  : drawDepth() (Pixel Shader)
    // Desc  : Depth render pixel shader
    //-------------------------------------------------------------------------
    bool drawDepth( int depthOutputType, int normalOutputType, int materialFlags, int renderFlags )
    {
        /////////////////////////////////////////////
        // Setup
        /////////////////////////////////////////////
	    bool orthographicCamera = testFlagAny( renderFlags, RenderFlags::OrthographicCamera );
	    bool computeGroup       = testFlagAny( renderFlags, RenderFlags::ComputeGroupID );
	    bool hasOpacityTexture  = testFlagAny( materialFlags, MaterialFlags::SampleOpacityTexture );
	    bool hasEmissiveTexture = testFlagAny( materialFlags, MaterialFlags::SampleEmissiveTexture );
   		bool computeTexCoords   = hasOpacityTexture || computeGroup || testFlagAny( materialFlags, MaterialFlags::SampleDiffuseTexture );
    
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
			float4 screenPosition : SV_POSITION;
			${mapDepthTexCoordRegisters(depthOutputType, normalOutputType, hasOpacityTexture || computeGroup)}
		?>

        // Define shader outputs.
		<?out
			float4 depthOut : SV_TARGET0;
		?>

        // Define shader outputs.
	    if ( normalOutputType != NormalType::NoNormal )
	    {
			<?out
				float4 normalOut : SV_TARGET1;
			?>
	    }

        // Constant buffer usage.
        <?cbufferrefs
            _cbMaterial;
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		// If this is the "null" shader, just return 0 
        if ( depthOutputType == DepthType::None )
        {
			<?depthOut = 0;?>
			if ( normalOutputType != NormalType::NoNormal )
				<?normalOut = 0;?>
			
			// Done	
			return true;		
        }
        
        // Do we have an opactity map?
        if ( hasOpacityTexture )
        {
            // Note: There is no need to pay attention to the 'opacityInDiffuse' state here
            // since we do not actually sample the diffuse texture anyway, and thus no
            // optimization is possible. Just read from whatever opacity texture was assigned.
			<?clip( (sample2D( sOpacityTex, sOpacity, texCoords ).a * _materialDiffuse.a) - _materialAlphaTestValue );?> 

        } // End if hasOpacityTexture

		// Output depth        
		switch( getPureDepthType( depthOutputType ) )
		{
	        case DepthType::LinearZ:
		        <?depthOut = setNormalizedDistance( depth, $depthOutputType );?>
	            break;
	        case DepthType::LinearDistance:
				<?depthOut = setDistance( length( eyeRay ), $depthOutputType );?>
	            break;
	        case DepthType::NonLinearZ:
				if ( !orthographicCamera )
		        	<?depth = (1.0f / depth) * _projectionMatrix._43 + _projectionMatrix._33;?>
		        <?depthOut = setNormalizedDistance( depth, $depthOutputType );?>
	            break;
	        default:
				<?depthOut = 0;?>
	            break;
		
        } // End switch depthOutputType

		// Optional material data group computation  // TODO: PRE-COMPUTE OFFLINE?
		if ( computeGroup )
		{
			<?float3 diffuseColor  = _materialDiffuse.rgb * sample2DBias( sDiffuseTex, sDiffuse, texCoords, 12 ).rgb;?>
			<?float3 emissiveColor = _materialEmissive.rgb;?>
			if ( hasEmissiveTexture ) 
				<?emissiveColor *= sample2D( sEmissiveTex, sEmissive, texCoords ).rgb;?>
			<?			
			float avgDiffuse  = dot( diffuseColor, float3( 0.33333, 0.33333, 0.33333 ) );
			float maxEmissive = max( emissiveColor.r, max( emissiveColor.g, emissiveColor.b ) ); 
			depthOut.a = lerp( avgDiffuse, maxEmissive, maxEmissive );
			?>
		}
		
		// Output normal 
		if ( normalOutputType != NormalType::NoNormal )
			<?normalOut = float4( normal * 0.5f + 0.5f, 0 );?> 

        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name  : drawGBufferDS() (Pixel Shader)
    // Desc  : G-Buffer render pixel shader for deferred shading
    //-------------------------------------------------------------------------
    bool drawGBufferDS( int normalType, int reflectionType, int lightmapType, int materialFlags, int renderFlags, int outputType, int shadingQuality )
    {
        /////////////////////////////////////////////
        // Setup
        /////////////////////////////////////////////
		// Is there pre-computed lighting we need to account for?
		bool precomputedLighting = false;
		if ( (reflectionType != ReflectionMode::None) || (lightmapType != LightTextureType::None) ||
		     testFlagAny( materialFlags, uint(MaterialFlags::Emissive) | uint(MaterialFlags::SampleEmissiveTexture) ) )
			precomputedLighting = true;

		// If there is no precomputed diffuse or specular lighting
		bool encodeSRGB = false;
		if ( (reflectionType == ReflectionMode::None) && (lightmapType == LightTextureType::None) )
		{
			// There is no need to decode sRGB (Note: No need to encode output either as it is already sRGB.)
			materialFlags &= ~uint(MaterialFlags::DecodeSRGB);
			
		} // End no precomputed diffuse/specular
		else
		{
			// We linearized diffuse/specular, so we'll need to re-encode
			if ( testFlagAny( renderFlags, RenderFlags::GBufferSRGB ) )
				encodeSRGB = true;
				
		} // End precomputed diffuse/specular
		
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 screenPosition : SV_POSITION;
			${mapTexCoordRegisters( normalType, reflectionType, lightmapType, false, true )}
        ?>

        // Define shader outputs.
        <?out
            float4  data0     : SV_TARGET0;
            float4  data1     : SV_TARGET1;
            float4  data2     : SV_TARGET2;
            float4  data3     : SV_TARGET3;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?float4 sceneLighting = 0;?>

		// Declare our lighting structures
		initLightingStructures( false, true );
        
        // Retrieve surface data
        <?
        LightingData lighting = (LightingData)0;
        SurfaceData  surface  = (SurfaceData)0;
        getSurfaceData( surface, worldPos, normal, texCoords, parallaxDirection, tangentToWorldSpace, 
                        $normalType, $materialFlags, $renderFlags, $shadingQuality );
        ?>
			
		// If we are using precomputed lighting 
		if ( precomputedLighting )
		{		
			<?
            // Sample all precomputed lighting 
			getPrecomputedLighting( lighting, surface, reflection, texCoords, texCoordsLight, 
			                        $reflectionType, $lightmapType, $materialFlags, $renderFlags );

			// Combine lighting results with reflectance as needed 
			sceneLighting.rgb = lighting.diffuse.rgb * surface.diffuse.rgb + lighting.specular.rgb * surface.specular.rgb + lighting.emissive.rgb;

            // Compress HDR lighting results as required
			compressColor( sceneLighting, sceneLighting, $outputType );
            ?>
				
		} // End if precomputed lighting 
		
		/////////////////////////////////////////		
		// Fill the g-buffer
		/////////////////////////////////////////		

		// Simulate fitting the normal to a tesslated cube to reduce quantization artifacts
		if ( testFlagAny( materialFlags, MaterialFlags::CorrectNormals ) && ( normalType != NormalSource::Vertex ) )
			<?compressUnsignedNormal( surface.normal, sNormalsFitTex, sNormalsFit );?>
		else
			<?surface.normal = surface.normal * 0.5f + 0.5f;?>
		<?				
		data0.xyz = surface.normal;
		data0.w   = surface.gloss;
		?>
		if ( testFlagAny( renderFlags, RenderFlags::SpecularColorOutput ) )
		{
			<?		
			data1.rgb = surface.diffuse;
			data1.a   = surface.transmission;
			data2.rgb = surface.specular;
			data2.a   = 0;
			data3     = sceneLighting;		
			?>

        } // End if full specular color
		else
		{	
			<?
			data1     = float4( surface.diffuse, dot( surface.specular, float3( 0.2125, 0.7154, 0.0721 ) ) );
			data2     = sceneLighting;
			?>

        } // End if specular intensity only
		
		// If we need to manually non-linearize the color data, do so now
		if ( encodeSRGB )
		{
			<?data1 = LinearToSRGB4( data1 );?>
			if ( testFlagAny( renderFlags, RenderFlags::SpecularColorOutput ) )
				<?data2.rgb = LinearToSRGB( data2.rgb );?>
		}

        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name  : drawDirectLighting() (Pixel Shader)
    // Desc  : Forward lighting pixel shader
    //-------------------------------------------------------------------------
    bool drawDirectLighting( int normalType, int lightType, int shadowMethod, int primaryTaps, int secondaryTaps, int materialFlags, int lightFlags, int renderFlags, int shadingQuality )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.

		// Will we be projecting a light texture?
		bool projective = ( primaryTaps > 0 || lightType == LightType::Projector || testFlagAny( lightFlags, LightFlags::AttenuationTexture ) );

        <?in
			float4 screenPosition : SV_POSITION;
			${mapTexCoordRegisters( normalType, ReflectionMode::None, LightTextureType::None, projective, false )}
        ?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		// Declare our lighting structures
		initLightingStructures( true, false );

        // Fill out the lighting parameter data structure 
		LightingParameters params = initializeLightingSystem( lightType, lightFlags, shadowMethod, primaryTaps, secondaryTaps, renderFlags, shadingQuality );
        
        // Retrieve surface data
        <?
        LightingData lighting = (LightingData)0;
        SurfaceData  surface  = (SurfaceData)0;
        getSurfaceData( surface, worldPos, normal, texCoords, parallaxDirection, tangentToWorldSpace, 
                        $normalType, $materialFlags, $renderFlags, $shadingQuality );
        ?>

		// Compute projective texture coordinates
		if ( projective )
			<?computeProjectiveCoords( surface, texCoordsProj, $lightType );?>

        // Compute lighting
        <?
		computeLighting( lighting, surface, $params );
        data0 = float4( lighting.diffuse + lighting.specular, 0 );
        ?>
		
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name  : drawReflectiveShadowMap() (Pixel Shader)
    // Desc  : Reflective shadow map pixel shader
    // Note  : Should force a simplistic model (basically just N.L and linear atten)
    //-------------------------------------------------------------------------
    bool drawReflectiveShadowMap( int lightType, int materialFlags, int renderFlags )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
			float4 screenPosition : SV_POSITION;
			float  depth          : TEXCOORD0;
			float3 normal         : TEXCOORD1;
			float3 lightDir       : TEXCOORD2;
			float2 texCoords      : TEXCOORD3;
		?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
            float4 data1 : SV_TARGET1;
            float4 data2 : SV_TARGET2;
        ?>
        
        // Constant buffer usage.
        <?cbufferrefs
            _cbMaterial;
            _cbCamera;
            _cbLight;
            _cbLightingSystem;
        ?>
		if ( lightType == LightType::Spot )
		{
			<?cbufferrefs
				_cbSpotLight;
			?>

		} // End if Spot
		else if ( lightType == LightType::Projector )
		{
			<?cbufferrefs
				_cbProjectorLight;
			?>

		} // End if Projector        
        
	    /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

        // Do we have an opactity map?
        if ( testFlagAny( materialFlags, MaterialFlags::SampleOpacityTexture ) )
			<?clip( sample2D( sOpacityTex, sOpacity, texCoords ).a - _materialAlphaTestValue );?> 

		// Normalize normal
		<?normal = normalize( normal );?>

		// Compute distance attenuation and lambertian cosine
		<?float attenuation = 1.0f;?>
		if ( lightType == LightType::Directional )		
		{
			<?attenuation = saturate( dot( normal, _lightDirectionWorld ) );?>
		}
		if ( lightType == LightType::Projector )		
		{
			<?attenuation = saturate( dot( normal, _lightDirectionWorld ) ) * (1.0f - depth);?>
		}
		else if ( lightType == LightType::Omni || lightType == LightType::Spot )		
		{
			<?
			float d = length( lightDir );
			half3 l = lightDir / d;
			attenuation = saturate( dot( normal, l ) ) * (1.0 - saturate( d * _lightAttenuation.z + _lightAttenuation.w ) );
			?>
			// Cone falloff for spotlight (linear)
			if ( lightType == LightType::Spot )
				<?attenuation *= saturate( dot( -_lightDirectionWorld, l ) * _lightSpotTerms.x + _lightSpotTerms.y );?>
		}
		
        <?
		// Output depth        
        data0 = depth;

		// Output normal 
		data1 = float4( normal * 0.5f + 0.5f, 0 );
		?> 

		// Output diffuse reflectance modulated by light color (ldr), distance attenuation, and lambertian term
		//if ( hasDiffuseTexture )
			<?data2 = float4( sample2DBias( sDiffuseTex, sDiffuse, texCoords, 15 ).rgb * _materialDiffuse.rgb * attenuation, attenuation );?>	
		//else
			//<?data2 = float4( _materialDiffuse.rgb * attenuation, 0 );?>	

        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name  : drawPrecomputedLighting() (Pixel Shader)
    // Desc  : Precomputed lighting pixel shader
    //-------------------------------------------------------------------------
    bool drawPrecomputedLighting( int normalType, int reflectionType, int lightmapType, int materialFlags, int renderFlags, int shadingQuality )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 screenPosition : SV_POSITION;
			${mapTexCoordRegisters( normalType, reflectionType, lightmapType, false, false )}
        ?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		// Declare our lighting structures
		initLightingStructures( false, true );
        
        <?
        LightingData lighting = (LightingData)0;
        SurfaceData  surface  = (SurfaceData)0;

        // Retrieve surface data
        getSurfaceData( surface, worldPos, normal, texCoords, parallaxDirection, tangentToWorldSpace, 
                        $normalType, $materialFlags, $renderFlags, $shadingQuality );

        // Sample precomputed lighting
        getPrecomputedLighting( lighting, surface, reflection, texCoords, texCoordsLight,
                                $reflectionType, $lightmapType, $materialFlags, $renderFlags );

		// Compute final lighting results
		data0 = float4( (lighting.diffuse * surface.diffuse) + (lighting.specular * surface.specular) + lighting.emissive, 0 );

        ?>

		
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : drawOpacity() (Pixel Shader)
	// Desc : Returns opacity (with optional Fresnel effect)
    //-------------------------------------------------------------------------
    bool drawOpacity( bool hasOpacityTexture, int shadingQuality )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 screenPosition : SV_POSITION;
			${mapDepthTexCoordRegisters(DepthType::LinearDistance, NormalType::NormalWorld, hasOpacityTexture)}
        ?>

        // Define shader outputs.
        <?out
            float4 color : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbMaterial;
            _cbObject;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		//<?float opacity = _materialDiffuse.a * _objectBlendFactor;?> //ToDo: objectBlendFactor is currently broken (0).
		<?float opacity = _materialDiffuse.a;?>  
		
        if ( hasOpacityTexture )
        {
            // Note: There is no need to pay attention to the 'opacityInDiffuse' state here
            // since we do not actually sample the diffuse texture anyway, and thus no
            // optimization is possible. Just read from whatever opacity texture was assigned.
		    <?opacity = lerp( opacity, opacity * sample2D( sOpacityTex, sOpacity, texCoords ).a, _materialOpacityMapStrength.x );?>
        
        } // End if has opacity

		// Apply a Fresnel term based on surface orientation with respect to the camera.
		if ( shadingQuality > ShadingQuality::LowQuality )
		{
			<?
			half3 n = normalize( normal );
			half3 v = normalize( eyeRay );
			float f = pow( 1.0 - saturate( dot( n, v ) ), _materialFresnelExponent );
			opacity = lerp( opacity, 1.0, f * _materialFresnelTransparent ); 
			?>
        } 
		
		<?color = opacity;?>;
  
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : drawFog() (Pixel Shader)
	// Desc : Pixel shader used during fog pass.
	// Note : For optimization, we don't include the Fresnel adjustment on opacity here.
    //-------------------------------------------------------------------------
    bool drawFog( bool hasOpacityTexture, int fogModel )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 screenPosition : SV_POSITION;
			${mapDepthTexCoordRegisters(DepthType::LinearDistance, NormalType::NoNormal, hasOpacityTexture)}
        ?>

        // Define shader outputs.
        <?out
            float4 color : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbMaterial;
            _cbObject;
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		//<?float opacity = _materialDiffuse.a * _objectBlendFactor;?> //ToDo: objectBlendFactor is currently broken (0).
		<?float opacity = _materialDiffuse.a;?> 
        if ( hasOpacityTexture )
        {
            // Note: There is no need to pay attention to the 'opacityInDiffuse' state here
            // since we do not actually sample the diffuse texture anyway, and thus no
            // optimization is possible. Just read from whatever opacity texture was assigned.
		    <?
			opacity *= sample2D( sOpacityTex, sOpacity, texCoords ).a;
			clip( opacity - _materialAlphaTestValue ); 
		    ?>

        } // End if has opacity

		<?
		float distance = length( eyeRay );
        float factor = computeFogFactor( distance, $fogModel );
        color = float4( _fogColor.rgb, factor );
        ?>

        // Valid shader
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Member Functions
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : mapTexCoordRegisters() 
    // Desc : Sets up the minimal texture coordinate registers for shader I/O
    //        based on what features are currently active for the permutation
    //-------------------------------------------------------------------------
	String mapTexCoordRegisters( int normalType, int reflectionType, int lightmapType, bool projectiveCoords, bool screenCoords )
	{	
		String str;
		int coordIndex = 0;
		
        str += "float4 texCoords : TEXCOORD" + coordIndex++ + ";";
		str += "float3 worldPos : TEXCOORD" + coordIndex++ + ";";

		if ( projectiveCoords )
			str += "float4 texCoordsProj : TEXCOORD" + coordIndex++ + ";";
        else
            <?float4 texCoordsProj;?>
		
		if ( lightmapType != LightTextureType::None )
            str += "float2 texCoordsLight : TEXCOORD" + coordIndex++ + ";";
        else
            <?float2 texCoordsLight;?>

		switch( normalType )
		{
			case NormalSource::Vertex:
				str += "float3 normal : TEXCOORD" + coordIndex++ + ";";
                // Dummy variables
                <?
                float2 parallaxDirection;
                float3x3 tangentToWorldSpace;
                ?>
			    break;
			case NormalSource::NormalMap:
				str += "float3x3 tangentToWorldSpace : TEXCOORD" + coordIndex + ";";        
				coordIndex += 3;
                <?
                float3 normal;
                float2 parallaxDirection;
                ?>
			    break;
			case NormalSource::ParallaxOffset:
				str += "float2 parallaxDirection : TEXCOORD" + coordIndex++ + ";";
				str += "float3x3 tangentToWorldSpace : TEXCOORD" + coordIndex + ";";
				coordIndex += 3;
                // Dummy variables
                <?
                float3 normal;
                ?>
			    break;
		
        } // End switch normalType
 
        switch( reflectionType )
        {
            case ReflectionMode::Planar:
				str += "float4 reflection : TEXCOORD" + coordIndex++ + ";";
			    break;
            default:
                // Dummy variables
                <?float4 reflection;?>
                break;
        
        } // End switch reflectionType
       
		if ( screenCoords )
			str += "float4 texCoordsScreen : TEXCOORD" + coordIndex++ + ";";
        else
            <?float4 texCoordsScreen;?>
       
		return str;
	}	

    //-------------------------------------------------------------------------
    // Name : mapDepthTexCoordRegisters()
    // Desc : Sets up the minimal texture coordinate registers for shader I/O
    //        based on what features are currently active for the permutation
    // Note : Used for depth and shadowmap passes
    //-------------------------------------------------------------------------
	String mapDepthTexCoordRegisters( int depthType, int normalType, bool hasOpacityTexture )
	{	
		String str;
		int coordIndex = 0;
		
		if ( hasOpacityTexture )
			str += "float2 texCoords : TEXCOORD" + coordIndex++ + ";";

		switch( getPureDepthType( depthType ) )
		{
	        case DepthType::LinearZ:
	        case DepthType::NonLinearZ:
				str += "float depth : TEXCOORD" + coordIndex++ + ";";
	            break;
	        case DepthType::LinearDistance:
				str += "float3 eyeRay : TEXCOORD" + coordIndex++ + ";";
	            break;
		
        } // End switch output type

		if ( normalType != NormalType::NoNormal )
			str += "float3 normal : TEXCOORD" + coordIndex++ + ";";
				
		return str;
	}

} // End Class : MeshShader
