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
// Name : LightSource.sh                                                     //
//                                                                           //
// Desc : Internal shaders for light source processing.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "Utilities.shh"
#include "Types.shh"
#include "Config.shh"
#include "Lighting.shh"

///////////////////////////////////////////////////////////////////////////////
// Global Types
///////////////////////////////////////////////////////////////////////////////
// ToDo: These enums can go
enum CameraStatus
{
    Inside       = 0,
    Outside      = 1,
    Intersecting = 2,
};

enum LightMode
{
	Dynamic = 0,
	Shadow  = 1,
	Static  = 2
};

enum StencilMode
{
    Fill        = 0,
    FillFront   = 1,
    FillBack    = 2,
    Clear       = 3
};

enum DirectionalMode
{
    LightingOnly  = 0,
    PSSM_Near     = 1,
    PSSM_Middle   = 2,
    PSSM_Far      = 3
};

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return LightSourceShader( owner, driver, resources );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : LightSourceShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders 
//        for light sources (deferred lighting/shading).
//-----------------------------------------------------------------------------
class LightSourceShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@      mOwner;         // The parent application object.
    private RenderDriver@       mDriver;        // Render driver to which this shader is linked.
    private ResourceManager@    mResources;     // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////////////////
    // Custom Constant Buffer Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?cbuffer cbBilateral : register(b12), globaloffset(c180)
        float4  textureSize;      //xy = dimensions, zw = recip dimensions 
        float2  depthScaleBias;   
    ?>
    
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D     sGBuffer0             : register(s0);  // Diffuse
        Sampler2D     sGBuffer1             : register(s1);  // Normal (Bump)
        Sampler2D     sGBuffer2             : register(s2);  // Specular
        Sampler2D     sDepth                : register(s3);  // Depth
        Sampler2D     sNormal               : register(s4);  // Normal (Surface)
        Sampler2D     sAttenuationBuffer    : register(s5);  // Attenuation Buffer
        Sampler2D     sRandom               : register(s6);  // Random numbers (various)

        Sampler2D     sDepthLowRes          : register(s6);  // Low resolution depth
        Sampler2D     sLighting             : register(s5);  // Lighting buffer
        
        Sampler2D     sSHR                  : register(s7);  // Spherical harmonics (red)
        Sampler2D     sSHG                  : register(s8);  // Spherical harmonics (green)
        Sampler2D     sSHB                  : register(s9);  // Spherical harmonics (blue)
        Sampler2D     sSHA                  : register(s10); // Spherical harmonics (auxiliary)
        Sampler2D     sSHRPoint             : register(s7);  // Spherical harmonics (red)
        Sampler2D     sSHGPoint             : register(s8);  // Spherical harmonics (green)
        Sampler2D     sSHBPoint             : register(s9);  // Spherical harmonics (blue)
        Sampler2D     sSHAPoint             : register(s10); // Spherical harmonics (auxiliary)
        
        Sampler2D     sSHRVertical          : register(s7);  // Spherical harmonics (red)
        Sampler2D     sSHGVertical          : register(s8);  // Spherical harmonics (green)
        Sampler2D     sSHBVertical          : register(s9);  // Spherical harmonics (blue)
        Sampler2D     sSHRHorizontal        : register(s10); // Spherical harmonics (red)
        Sampler2D     sSHGHorizontal        : register(s11); // Spherical harmonics (green)
        Sampler2D     sSHBHorizontal        : register(s12); // Spherical harmonics (blue)
    ?>    

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : LightSourceShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	LightSourceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner     = owner;
        @mDriver    = driver;
        @mResources = resources;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Shader Functions
    ///////////////////////////////////////////////////////////////////////////
    //-----------------------------------------------------------------------------
	// Name : getSurfaceData() 
	// Desc : Assembles the surface data needed to run lighting
	//-----------------------------------------------------------------------------
	__shadercall void getSurfaceData( inout SurfaceData surface, float4 screenTexCoord, float3 eyeRay, script LightingParameters params )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Constant buffer usage.
		<?cbufferrefs
			_cbLight;
			_cbLightingSystem;
			_cbCamera;
			_cbShadow;
		?>

        if ( params.lightType == LightType::Projector )
        {
            <?cbufferrefs
                _cbProjectorLight;
            ?>
        
        } // End if projector light

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
        float4 texData;
        float2 depthBufferData;
        ?>
		
        // Compute screen space texture coordinates for g-buffer lookups
        if ( params.lightType == LightType::Directional )
            <?surface.screenCoords = screenTexCoord.xy;?>
        else 
            <?surface.screenCoords = screenTexCoord.xy / screenTexCoord.w;?>

		// Is this a packed depth format (i.e., with something in the texture alpha channel?)
		bool isCompressedDepth = isCompressedDepthType( params.depthType );

        // Sample the depth buffer
        int pureType = sampleDepthBuffer( "depthBufferData", "sDepthTex", "sDepth", "surface.screenCoords", isCompressedDepth, params.depthType );

		// If we are doing view space lighting...
		if ( testFlagAny( params.renderFlags, RenderFlags::ViewSpaceLighting ) )
		{
			<?
            surface.position         = getViewPosition( surface.screenCoords, depthBufferData.x, ${params.orthographicCamera} );
			surface.distanceToCamera = length( surface.position );
			surface.linearZ          = depthBufferData.x;
			?>
			if ( params.orthographicCamera )
				<?surface.viewDirection = float3( 0, 0, -1 );?>  
			else
				<?surface.viewDirection = -surface.position / surface.distanceToCamera;?>
				
		} // End if view space lighting 
		else
		{
			// We are doing world space lighting
			if ( params.orthographicCamera )
			{
				<?
				surface.position         = getViewPosition( surface.screenCoords, depthBufferData.x, $true );
				surface.position         = mul( float4( surface.position, 1 ), _inverseViewMatrix );
				surface.viewDirection    = -_cameraDirection;
				surface.distanceToCamera = depthBufferData.x;
				surface.linearZ          = depthBufferData.x;
				?>
			
			} // End if orthographic camera
			else
			{
				// Normalize the viewing ray 
				<?
                surface.viewDirection = normalize( eyeRay.xyz );

				// If the depth type is not what we need (i.e., distance from camera along eye ray), convert it
				surface.distanceToCamera = convertDepthType( depthBufferData.x, surface.viewDirection, $pureType, ${DepthType::LinearDistance}, $false, $false );
				
				// Get the linear Z distance as well
				surface.linearZ = convertDepthType( depthBufferData.x, surface.viewDirection, $pureType, ${DepthType::LinearZ}, $false, $false );
				
				// Compute position 
				surface.position = surface.viewDirection * surface.distanceToCamera + _cameraPosition;
								
				// Reverse the eye ray to points towards the camera
				surface.viewDirection = -surface.viewDirection;
                ?>
		
			} // End if perspective camera 
		
		} // End if world space lighting 
		
        <?		            
        // Get normal and specular power
        texData                = sample2D( sGBuffer0Tex, sGBuffer0, surface.screenCoords );
        surface.normal         = normalize( texData.xyz * 2.0 - 1.0 );
        surface.specularPower  = exp2( texData.w * $MAX_GLOSS_LEVELS );
        ?>

		// Compute roughness 
		if ( params.approximateRoughness )
			<?surface.roughness = sqrt( 2.0f / (surface.specularPower + 2.0f) );?>

		// Read surface normal if needed
		if ( params.normalOffset )
			<?surface.surfaceNormal = normalize( sample2D( sNormalTex, sNormal, surface.screenCoords ).rgb * 2.0f - 1.0f );?>
		else
			<?surface.surfaceNormal = surface.normal;?>			
		
		// Retrieve AO if desired
		if ( params.useSSAO && isCompressedDepth ) 
            <?surface.ambientOcclusion = depthBufferData.y;?>
		else
            <?surface.ambientOcclusion = 1;?>

        // Retrieve reflectance properties if needed
        if ( params.applyReflectance )
        {
            if ( params.shadingQuality == ShadingQuality::LowQuality )
            {
                <?
                texData                    = sample2D( sGBuffer1Tex, sGBuffer1, surface.screenCoords );
                surface.diffuse.rgb        = texData.rgb;
                surface.specular.rgb       = texData.aaa;
                surface.ambientOcclusion   = 1; 
	            surface.transmission       = 0;
                ?>
            
            } // End if low quality	
            else
            {
                <?
                texData                    = sample2D( sGBuffer1Tex, sGBuffer1, surface.screenCoords );
                surface.diffuse.rgb        = texData.rgb;
                surface.transmission       = texData.a;
                texData                    = sample2D( sGBuffer2Tex, sGBuffer2, surface.screenCoords );
                surface.specular.rgb       = texData.rgb;
                ?>
            
            } // End if high quality
        
			// If we need to manually linearize the color data, so so now
			if ( testFlagAny( params.renderFlags, RenderFlags::GBufferSRGB ) )
			{
				<?
				surface.diffuse.rgb  = pow( surface.diffuse.rgb,  $GAMMA_TO_LINEAR );
				surface.specular.rgb = pow( surface.specular.rgb, $GAMMA_TO_LINEAR );
				?>
			}
        
        } // End if getReflectance
		
		// Get the data needed for lighting
		bool computeProjTexCoords = (params.primaryTaps > 0) || (params.lightType == LightType::Projector) || (params.attenuationTexture);
		
        // Compute projective texture coordinates if requested
        if ( computeProjTexCoords )
        {
			// Compute projective coordinates
            <?surface.projectiveTexCoords = mul( float4( surface.position, 1.0 ), _lightTexProjMatrix );?>

			// Cache light space Z coordinate
			<?surface.lightSpaceZ = surface.projectiveTexCoords.w;?>

            // If this is a projector light
            if ( params.lightType == LightType::Projector )
            {
                <?
                // Interpolate our projection window size based on distance along light's z axis
                surface.projectiveTexCoords.xy /= lerp( _lightConeAdjust.xz, _lightConeAdjust.yw, surface.projectiveTexCoords.z );
                surface.projectiveTexCoords.xy += 0.5;
                surface.projectiveTexCoords.w   = 1.0;
                ?>

            } // End if projector light
			else
			{			
   				// Complete the projection for standard projective textures
                <?surface.projectiveTexCoords.xyz /= surface.projectiveTexCoords.w;?>	
            
            } // End if other light type

        } // End if computeProjTexCoords

	} // End function                       

	//-----------------------------------------------------------------------------
	// Name : getGBufferData() 
	// Desc : Retrieves g-buffer data for the surface
	//-----------------------------------------------------------------------------
	__shadercall void getGBufferData( inout SurfaceData surface, float3 eyeRay, script int renderFlags, script int shadingQuality, script bool getReflectance )
	{
		/////////////////////////////////////////////
		// Setup
		/////////////////////////////////////////////
		bool orthographicCamera = (renderFlags & uint(RenderFlags::OrthographicCamera)) > 0;
		int  depthType;		
		if ( testFlagAny( renderFlags, RenderFlags::PackedDepth ) )
		{
			if ( testFlagAny( renderFlags, RenderFlags::NormalizedDistance ) )
				depthType = DepthType::LinearDistance_Packed;
			else if ( testFlagAny( renderFlags, RenderFlags::NonLinearZ ) )          
				depthType = DepthType::NonLinearZ_Packed;
			else
				depthType = DepthType::LinearZ_Packed;
		}
		else
		{
			if ( testFlagAny( renderFlags, RenderFlags::NormalizedDistance ) )
				depthType = DepthType::LinearDistance;
			else if ( testFlagAny( renderFlags, RenderFlags::NonLinearZ ) )          
				depthType = DepthType::NonLinearZ;
			else
				depthType = DepthType::LinearZ;
		}
		
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Constant buffer usage.
		<?cbufferrefs
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
        float4 texData;
        float2 depthBufferData;
        ?>
		
		// Is this a packed depth format (i.e., with something in the texture alpha channel?)
		bool isCompressedDepth = isCompressedDepthType( depthType );

        // Sample the depth buffer
        int pureType = sampleDepthBuffer( "depthBufferData", "sDepthTex", "sDepth", "surface.screenCoords", isCompressedDepth, depthType );

		// If we are doing view space lighting...
		if ( testFlagAny( renderFlags, RenderFlags::ViewSpaceLighting ) )
		{
			<?
            surface.position         = getViewPosition( surface.screenCoords, depthBufferData.x, $orthographicCamera );
			surface.distanceToCamera = length( surface.position );
			surface.linearZ          = depthBufferData.x;
			?>
			if ( orthographicCamera )
				<?surface.viewDirection = float3( 0, 0, -1 );?>  
			else
				<?surface.viewDirection = -surface.position / surface.distanceToCamera;?>
				
		} // End if view space lighting 
		else
		{
			// We are doing world space lighting
			if ( orthographicCamera )
			{
				<?
                surface.position         = getViewPosition( surface.screenCoords, depthBufferData.x, $true );
				surface.position         = mul( float4( surface.position, 1 ), _inverseViewMatrix );
				surface.viewDirection    = -_cameraDirection;
				surface.distanceToCamera = depthBufferData.x;
				surface.linearZ          = depthBufferData.x;
				?>
			
			} // End if orthographic camera
			else
			{
				<?
                // Normalize the viewing ray 
                surface.viewDirection = normalize( eyeRay.xyz );

				// If the depth type is not what we need (i.e., distance from camera along eye ray), convert it
				surface.distanceToCamera = convertDepthType( depthBufferData.x, surface.viewDirection, $pureType, ${DepthType::LinearDistance}, $false, $false );
				
				// Get the linear Z distance as well
				surface.linearZ = convertDepthType( depthBufferData.x, surface.viewDirection, $pureType, ${DepthType::LinearZ}, $false, $false );
				
				// Compute position 
				surface.position = surface.viewDirection * surface.distanceToCamera + _cameraPosition;
								
				// Reverse the eye ray to points towards the camera
				surface.viewDirection = -surface.viewDirection;
                ?>
		
			} // End if perspective camera 
		
		} // End if world space lighting 
		
        <?		            
        // Get normal and specular power
        texData                = sample2D( sGBuffer0Tex, sGBuffer0, surface.screenCoords );
        surface.normal         = normalize( texData.xyz * 2.0 - 1.0 );
        surface.specularPower  = exp2( texData.w * $MAX_GLOSS_LEVELS );
        ?>

		// Read surface normal 
		if ( testFlagAny( renderFlags, RenderFlags::SurfaceNormals ) )
			<?surface.surfaceNormal = normalize( sample2D( sNormalTex, sNormal, surface.screenCoords ).rgb * 2.0f - 1.0f );?>
		
		// Read ambient occlusion
		if ( isCompressedDepth )
			<?surface.ambientOcclusion = depthBufferData.y;?>
		else
			<?surface.ambientOcclusion = 1;?>

        // Retrieve reflectance properties
        if ( getReflectance )
        {
			if ( shadingQuality == ShadingQuality::LowQuality )
			{
				<?
				texData                    = sample2D( sGBuffer1Tex, sGBuffer1, surface.screenCoords );
				surface.diffuse.rgb        = texData.rgb;
				surface.specular.rgb       = texData.aaa;
				surface.ambientOcclusion   = 1; 
				surface.transmission       = 0;
				?>
	        
			} // End if low quality	
			else
			{
				<?
				texData                    = sample2D( sGBuffer1Tex, sGBuffer1, surface.screenCoords );
				surface.diffuse.rgb        = texData.rgb;
				surface.transmission       = texData.a;
				texData                    = sample2D( sGBuffer2Tex, sGBuffer2, surface.screenCoords );
				surface.specular.rgb       = texData.rgb;
				?>
	        
			} // End if high quality
	        
			// If we need to manually linearize the color data, so so now
			if ( testFlagAny( renderFlags, RenderFlags::GBufferSRGB ) )
			{
				<?
				surface.diffuse.rgb  = pow( surface.diffuse.rgb,  $GAMMA_TO_LINEAR );
				surface.specular.rgb = pow( surface.specular.rgb, $GAMMA_TO_LINEAR );
				?>
			}
			
		} // End if reflectance
				
	} // End function                       

    //-------------------------------------------------------------------------
    // Name : addSample() (Pixel Shader)
	// Desc : Samples an RSM texel, converts to SH, and adds to running total
    //-------------------------------------------------------------------------
	__shadercall void addSample( float3 position, float2 sampleUV, inout float4 shR, inout float4 shG, inout float4 shB, inout float weightSum, script int numOcclusionSamples )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Constant buffer usage.
        <?cbufferrefs
            _cbRSM;
            _cbGrid;
            _cbCamera;
        ?>
        
		// Sampler buffer usage.
		<?samplerrefs
			_sbLight;
		?> 
		       
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Grab a VPL 
		float3 vplNormal = normalize( sample2D( sShadowCustomTex, sShadowCustom, sampleUV ).rgb * 2.0f - 1.0f );			
		float3 vplColor  = sample2D( sShadowColorTex,  sShadowColor,  sampleUV ).rgb;
		float  vplDepth  = sample2D( sShadowTex,  sShadow,  sampleUV ).r;

		// Compute sample position in (rsm) view space
		float4 vplPositionVS;
		vplPositionVS.xy = (sampleUV * _rsmScreenToViewScaleBias.xy + _rsmScreenToViewScaleBias.zw) * vplDepth;
		vplPositionVS.z  = vplDepth;
		vplPositionVS.w  = 1;

		// Transform sample position to world space
		float3 vplPosition = mul( vplPositionVS, _rsmInvViewMatrix ).xyz;
		
		// Compute ray between vpl and cell location
		float3 ray = position - vplPosition;
		
		// Compute the form factor for this lighting direction
		float dist    = length( ray );
		         ray /= dist;
		float cosine  = saturate( dot( ray, vplNormal ) );
		float ff      = min( _geometryBias, cosine / ( dist * dist + 1e-6f ) );
		
		// Compute approximate occlusion along ray
		if ( $numOcclusionSamples > 0 )
		{
			//Note: We'll switch to an occlusion grid approach. If start and end locations are in same cell, no occlusion.
			//    Otherwise, step through the grid trilinearly and lookup and apply SH occlusion. 
			float smpDepth, occlusion = 1.0; 
			float v = 1.0 / $numOcclusionSamples;

			// Transform to clip space
			float4 clipPos = mul( float4( position, 1 ), _viewProjectionMatrix );
			clipPos.xyz /= clipPos.w;
		
			// Sample the screen depth buffer
			float2 depthCoords = clipPos.xy * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );
			?>
			
			// Sample the depth buffer
			sampleDepthBuffer( "smpDepth", "sDepthTex", "sDepth", "depthCoords", false, DepthType::LinearZ_Packed );

			<?	
			// Is the current cell sample point visible?
			float rhVisibility = clipPos.w < (smpDepth * 1.1) ? 1.0 : -1.0; 
			
			// Get the ray between the source (cell) and destination (rsm texel)
			float3 L = vplPosition - position;
		
			// Take the remaining samples
			for ( int j = 1; j <= $numOcclusionSamples; j++ )
			{
				// Determine next point along the line of sight
				float4 Pj    = float4( position + ( j * v ) * L, 1 );
				clipPos      = mul( Pj, _viewProjectionMatrix );
				clipPos.xyz /= clipPos.w;
				depthCoords  = clipPos.xy * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );
				?>
				// Sample the depth buffer
				sampleDepthBuffer( "smpDepth", "sDepthTex", "sDepth", "depthCoords", false, DepthType::LinearZ_Packed );
				<?	

				// Modulate the visibility according to the number of hidden LoS samples
				occlusion -= (rhVisibility * clipPos.w) < (rhVisibility * smpDepth) ? 0.0 : v; 
				
			} // Next step

			// Adjust form factor 
			ff = saturate( ff * occlusion );
			
		} // End if occlusion 
		
		// Should this sample be included?
		float weight = ff > 0.0f;
		weightSum += weight;
	
		// Adjust color sample by form factor
		vplColor *= (ff * weight);
	
		// Compute radiance at sample location (projecting to SH)
		float4 shRay = float4( 0.282094792f, -0.488602512f, 0.488602512f, -0.488602512f );
		shRay.yzw *= ray.yzx;
		shR += shRay * vplColor.r;	
		shG += shRay * vplColor.g;	
		shB += shRay * vplColor.b;	
		?>	
	} 

	//----------------------------------------------------------------------------
	// Name : sampleVolumeSH2()
	// Desc : Samples spectral spherical harmonics from a volumetric grid 
	//----------------------------------------------------------------------------
	__shadercall void sampleVolumeSH2( float3 volTexCoords, float3 gridDimensions,
									   Texture2D tR, Texture2D tG, Texture2D tB, 
									   Sampler2D sR, Sampler2D sG, Sampler2D sB, 
									   out float4 shR, out float4 shG, out float4 shB, 
									   script bool bUnrolled )
	{
		// If we had to use volume texture unrolling b/c we could not update a volume slice, simulate trilinear blending
		if ( bUnrolled )
		{
			<?
			float2 uv0, uv1;
			float  recipSizeZ = (1.0f / gridDimensions.z);
			
			// What slice are we in?
			float slice = volTexCoords.z * gridDimensions.z;
			float snappedSlice = floor( slice );
			
			// Compute interpolant
			float t = saturate( slice - snappedSlice );

			// Linear filtering fix (necessary or do via borders?)
			float halfTexel = 0.5 / gridDimensions.x;
			if ( volTexCoords.x <= halfTexel )
				volTexCoords.x = halfTexel;
			if ( volTexCoords.x >= (1.0 - halfTexel) )
				volTexCoords.x = (1.0 - halfTexel);

			// Adjust the u texture coordinate to index the appropriate slice
			uv0.x = (volTexCoords.x + snappedSlice) * recipSizeZ;
			uv0.y = volTexCoords.y;
			
			// For the second sample, translate u by one sub-texture width
			uv1.x = uv0.x + recipSizeZ;
			uv1.y = volTexCoords.y;
			if ( snappedSlice >= (gridDimensions.z - 1.0f) )
			{
				uv1 = uv0;
				  t = 0;				
			}
	        
			// Interpolate between the two slices
			shR = lerp( sample2D( tR, sR, uv0 ), sample2D( tR, sR, uv1 ), t );
			shG = lerp( sample2D( tG, sG, uv0 ), sample2D( tG, sG, uv1 ), t );
			shB = lerp( sample2D( tB, sB, uv0 ), sample2D( tB, sB, uv1 ), t );
			?>
		}
		else
		{
			<?
			shR = sample3D( tR, sR, volTexCoords );
			shG = sample3D( tG, sG, volTexCoords );
			shB = sample3D( tB, sB, volTexCoords );
			?>
		}
	}

	//----------------------------------------------------------------------------
	// Name : worldToGridCS()
	// Desc : Transforms a world space coordinate to volume clip space
	//----------------------------------------------------------------------------
	__shadercall float4 worldToGridCS( float3 position, float3 gridMin, float3 gridDimensions, float3 gridSize )
	{
		<?
		// Compute the 3D coordinate
		float3 volCoords = (position - gridMin) / gridSize;
	    
		// Unroll to 2D
		float2 uv;
		uv.x = ( volCoords.x + floor( volCoords.z * gridDimensions.z ) ) / gridDimensions.z;
		uv.y = 1.0f - volCoords.y;

		// Snap to the center of the texel
		float2 texSize = { gridDimensions.x * gridDimensions.x, gridDimensions.y };
		uv  = (floor( uv * texSize ) + 0.5f) / texSize;

		// Convert final unrolled texture coordinate to [-1, 1]
		return float4( uv * float2( 2.0f, -2.0f ) - float2( 1.0f, -1.0f ), 0.0f, 1.0f );
		?>
	}

    ///////////////////////////////////////////////////////////////////////////
    // Vertex Shaders
    ///////////////////////////////////////////////////////////////////////////
	//-----------------------------------------------------------------------------
    // Name : transform() (Vertex Shader)
	// Desc : Used to transform light shapes for stencil filling or screen execution.
	//-----------------------------------------------------------------------------
	bool transform( int lightType, bool stencilOnly )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
		    float3 sourcePosition   : POSITION;
        ?>

		// Define shader outputs.
		<?out
			float4 clipPosition     : SV_POSITION;
		?>
		if ( !stencilOnly )
		{
			<?out
			   float4 texCoords     : TEXCOORD0;
			   float3 eyeRay        : TEXCOORD1;
			?>
		
        } // End if !stencilOnly

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbLight;
            _cbLightingSystem;
            _cbWorld;
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
        if ( lightType == LightType::Directional )
        {
			<?
			clipPosition.xyz = sourcePosition.xyz;
			clipPosition.w   = 1.0;
			?>

			if ( !stencilOnly )
			{
				<?
				// Get the screen coords into range [0,1]
				texCoords.xy = float2( clipPosition.x, -clipPosition.y ) * 0.5 + 0.5;
				texCoords.zw = float2( 0, 1 );

				// Adjust screen coords to take into account the currently set viewport and half pixel offset. 
				texCoords.xy = texCoords.xy * _screenUVAdjustScale + _screenUVAdjustBias;

				// This ray goes from the camera position to the pixel in the screen
				float4 viewPosition = mul( float4( clipPosition.x * _cameraFar, clipPosition.y * _cameraFar, _cameraFar, _cameraFar ), _inverseProjectionMatrix );
				eyeRay = mul( viewPosition.xyz, _inverseViewMatrix );
				?>
			
            } // End if !stencilOnly
        
        } // End if Directional
        else 
        {        
			<?float4 modelPosition = float4( sourcePosition, 1 );?>

			// Scale model position as required to construct the projector frustum shape
			if ( lightType == LightType::Projector )        
			{
				<?
				float2 windowSize = lerp( _lightConeAdjust.xz, _lightConeAdjust.yw, modelPosition.z );
				modelPosition.xy *= windowSize;
				modelPosition.z  *= _lightClipDistance.y;
				?>
			
            } // End if Projector

			<?			
			// Transform geometry to clip space
            float4 worldPosition = mul( modelPosition, _worldMatrix );
            clipPosition = mul( worldPosition, _viewProjectionMatrix );
			?>

			// Compute data necessary for g-buffer lookups			
			if ( !stencilOnly )
			{
				<?
				// This ray goes from the camera position to the vertex
                eyeRay = worldPosition.xyz - _cameraPosition;		
				
				// Compute texture coordinates for our lookup
				computeMeshScreenTextureCoords( clipPosition, texCoords );
                ?>
			
            } // End if !stencilOnly
        
        } // End if !Directional
        
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
    // Name : transformBorderRepair() (Vertex Shader)
	// Desc : Used to transform border repair quads into the clip space of the
	//        relevant shadow map frustum.
	//-----------------------------------------------------------------------------
	bool transformBorderRepair( )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
		    float3  sourcePosition      : POSITION;
		    float2  sourceTexCoords     : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition        : SV_POSITION;
		    float2  texCoords           : TEXCOORD0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbLightingSystem;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Compute clip space position (transform by upper 3x3 of frustum view matrix only)
		clipPosition = float4( mul( sourcePosition, _lightTexProjMatrix ).xyz, 1 );
		
		// Offset quad to ensure we sample from the correct texel when
		// similar shadow map edges connect (i.e. -V to -V). See 
		// ShadowMapVertexShader() for more information.
		clipPosition.x -= _targetSize.z; // Equivalent to (1.0 / TargetSize.x)
		clipPosition.y += _targetSize.w; // Equivalent to (1.0 / TargetSize.y)

		// Return texture coordinates
		texCoords = sourceTexCoords;
		?>
	
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
    // Name : transformPassThrough() (Vertex Shader)
	// Desc : Basic pass-through vertex shader for clip quad draws.
	//-----------------------------------------------------------------------------
	bool transformPassThrough( )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float3 sourcePosition   : POSITION;
		    float2 sourceTexCoords  : TEXCOORD0;
        ?>

        // Define shader outputs.
		<?out
			float4 clipPosition     : SV_POSITION;
			float2 texCoords        : TEXCOORD0;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		clipPosition = float4( sourcePosition.xyz, 1 );

		// Get the screen coords into range [0,1]
		texCoords = float2( clipPosition.x, -clipPosition.y ) * 0.5 + 0.5;

		// Adjust screen coords to take into account the currently set viewport and half pixel offset. 
		texCoords = texCoords * _screenUVAdjustScale + _screenUVAdjustBias;
		?>

		// Valid shader
		return true;
	}

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : nullOutput() (Pixel Shader)
	// Desc : Selected when no output is required (stencil fill / clear).
    //-------------------------------------------------------------------------
    bool nullOutput( )
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
			float4  data0    : SV_TARGET0;
		?>
		
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////		
		<?data0 = float4(0,0,0,0);?>
		
        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : drawDirectLighting() (Pixel Shader)
	// Desc : Used when drawing light sources. Handles diffuse, specular, and shadowing. 
    //-------------------------------------------------------------------------
    bool drawDirectLighting( int lightType, int lightFlags, int shadowMethod, int primaryTaps, int secondaryTaps, int renderFlags, int shadingQuality )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
			float4   texCoords      : TEXCOORD0;
			float3   eyeRay         : TEXCOORD1;
        ?>

        // Define shader outputs.
		<?out
			float4  data0           : SV_TARGET0;
		?>
		
		// If we are using high quality deferred lighting (separate diffuse/specular), add another output	
		bool deferredLightingHighQuality = testFlagAll( renderFlags, uint(RenderFlags::DeferredRendering) | uint(RenderFlags::DeferredLighting) | uint(RenderFlags::SpecularColorOutput) );
		if ( deferredLightingHighQuality )
		{
			<?out
				float4  data1       : SV_TARGET1;
			?>
		}
				
        // Constant buffer usage.
        <?cbufferrefs
            _cbLight;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		
		// Declare our lighting structures
		initLightingStructures( true, false );
        <?
            LightingData lighting = (LightingData)0;
            SurfaceData  surface  = (SurfaceData)0;
        ?>

		// Fill out the lighting parameter data structure 
		LightingParameters params = initializeLightingSystem( lightType, lightFlags, shadowMethod, primaryTaps, secondaryTaps, renderFlags, shadingQuality );

		<?
        // Get the data needed for lighting
		getSurfaceData( surface, texCoords, eyeRay, $params );
        
		// Compute lighting
		computeLighting( lighting, surface, $params ); 
        ?>

		// Compute final lighting results
		if ( deferredLightingHighQuality )
		{
	        <?data0 = float4( lighting.diffuse,  0 );?>
	        <?data1 = float4( lighting.specular, 0 );?>
		}
		else if ( testFlagAll( renderFlags, uint(RenderFlags::DeferredRendering) | uint(RenderFlags::DeferredLighting) ) )
		{
	        <?data0 = float4( lighting.diffuse, dot( lighting.specular.rgb, float3( 0.299, 0.587, 0.114 ) ) );?>
		}
		else
		{
	        <?data0 = float4( lighting.diffuse + lighting.specular, 0 );?>
		}
            
        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : drawShadowing() (Pixel Shader)
	// Desc : Used when drawing shadow-only sources or computing deferred shadows. 
    //-------------------------------------------------------------------------
    bool drawShadowing( int lightType, int lightFlags, int shadowMethod, int primaryTaps, int secondaryTaps, int renderFlags, int shadingQuality, bool writeAttenuationBuffer )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
			float4   texCoords      : TEXCOORD0;
			float3   eyeRay         : TEXCOORD1;
        ?>

        // Define shader outputs.
		<?out
			float4  data0       : SV_TARGET0;
		?>
		
		// If we are using high quality deferred lighting (separate diffuse/specular), add another output		
		bool deferredLightingHighQuality = testFlagAll( renderFlags, uint(RenderFlags::DeferredRendering) | uint(RenderFlags::DeferredLighting) | uint(RenderFlags::SpecularColorOutput) );
		if ( deferredLightingHighQuality )
		{
			<?out
				float4  data1       : SV_TARGET1;
			?>
		}
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbLight;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
	
		// Declare our lighting structures
		initLightingStructures( true, false );
        <?
            LightingData lighting = (LightingData)0;
            SurfaceData surface = (SurfaceData)0;
        ?>

		// Fill out the lighting parameter data structure 
		LightingParameters params = initializeLightingSystem( lightType, 0, shadowMethod, primaryTaps, secondaryTaps, renderFlags, shadingQuality );

		// Do not bother retrieving or using reflectance values
		params.applyReflectance = false;
		
        <?
		// Get the data needed for lighting
		getSurfaceData( surface, texCoords, eyeRay, $params );

		// Compute shadows and attenuation (no diffuse or specular)
		computeLighting( lighting, surface, $params );
        ?>

		// If we are filling the attenuation buffer, just write out the masked shadows
		if ( writeAttenuationBuffer )
		{
			<?
			// We remap the shadow into the range [1/255, 1] to allow 0 to be used as means to identify texels that not influenced by the light source
			float fMin       = 0.00392157;   // <- 1.0 / 255.0
			float fMax       = 1.0f;
			float ooRange    = 1.003937008;  // <- 1.0f / (fMax - fMin)
			float nearAdjust = 0.003937008;  // <-  fMin * ooRange       We want to do (v + min) / range during packing and (v - min) / range during unpacking
			//data0 = saturate( lighting.shadow * ooRange + nearAdjust ) * _lightAttenuationBufferMask;
			data0 = lighting.shadow * _lightAttenuationBufferMask;
			?>
		}
		else
		{
			// If shadows are disabled, this is just a 'negative' light source (modulative)
			if ( primaryTaps == 0 )
			{ 
				<?data0 = float4( lighting.diffuse, dot( lighting.diffuse, float3( 0.2125, 0.7154, 0.0721 ) ) );?>
			
			} // End if no shadows
			else
			{
				<?
				// Compute attenuation luminance
				float attenuationLuminance = dot( lighting.attenuation, float3( 0.2125, 0.7154, 0.0721 ) );

				// Compute ambient luminance
				float ambientLuminance = dot( lighting.ambient, float3( 0.2125, 0.7154, 0.0721 ) ); 

				// Invert shadow 
				float shadowTerm = 1.0 - lighting.shadow;

				// Attenuate the shadow 
				shadowTerm *= attenuationLuminance;
				
				// Bias by the ambient term 
				shadowTerm = saturate( shadowTerm - ambientLuminance );

				// Reverse the shadow term 
				shadowTerm = 1.0 - shadowTerm;

				// Apply an exponential falloff to darken the results (ToDo: Replace with something more configurable?)
				shadowTerm = shadowTerm * shadowTerm * shadowTerm * shadowTerm; //i.e., pow( Shadow, 4 );

				// Return shadow (for subtractive or modulative blend)
				data0 = shadowTerm.xxxx;
				?>
			
			} // End if shadows
		
		} // End if not writing to the attenuation buffer

		// High quality deferred lighting requires shadows output to two targets (when not writing)
		if ( !writeAttenuationBuffer && deferredLightingHighQuality )
	        <?data1 = data0;?>

        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : drawBorderRepair() (Pixel Shader)
	// Desc : Duplicates shadow map border texels.
    //-------------------------------------------------------------------------
    bool drawBorderRepair( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 screenPosition : SV_POSITION;
			float4 texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
		<?out
			float4 borderTexel  : SV_TARGET0;
		?>
		
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
			borderTexel = sample2D( sShadowTex, sShadow, texCoords );
		?>

        // Valid shader
        return true;
	}

    ///////////////////////////////////////////////////////////////////////////
    // Member Functions
    ///////////////////////////////////////////////////////////////////////////
	//-------------------------------------------------------------------------
    // Name : drawRSM() (Pixel Shader)
	// Desc : Used when drawing RSMs into radiance volume. 
	// Note : 
	/*
	RSM Sampling Options:
	
	0 = Random sampling in disc from RSM center
	1 = Random sampling in disc from projected cell position
	2 = Fixed NxN sampling (assumes RSM was downsampled)
	
	*/
    //-------------------------------------------------------------------------
    bool drawRSM( int samplingMode, int numRadianceSamples, int numOcclusionSamples )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
        ?>

        // Define shader outputs.
		<?out
			float4  data0           : SV_TARGET0;
			float4  data1           : SV_TARGET1;
			float4  data2           : SV_TARGET2;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbRSM;
            _cbGrid;
            _cbCamera;
        ?>
        
		// Sampler buffer usage.
		<?samplerrefs
			_sbLight;
		?> 
		       
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
        float  weightSum = 0.0f;
		float4 avgSHR = 0, avgSHG = 0, avgSHB = 0;
		
		// Compute grid data
		float3 gridSize   = _gridMax - _gridMin;
		float3 cellSize   = gridSize / _gridDimensions;
		float3 halfCell   = cellSize * 0.5f; 
		float3 gridOrigin = _gridMin + float3( 0, gridSize.y, 0 );
		
		// Compute cell coords
		float3 gridCoords;
		gridCoords.z = floor( screenPosition.x / _gridDimensions.x );
		gridCoords.y = screenPosition.y;
		gridCoords.x = screenPosition.x - (gridCoords.z * _gridDimensions.x);
		
		// Compute world space position of cell
		float3 cellCenter = gridOrigin + gridCoords * (cellSize * float3( 1, -1,  1 ) );
		?>

        // Are we sampling all values from a downsampled rsm?
		if ( samplingMode == 2 )
		{
			<?
			// For each RSM sample we will take
			for ( int i = 0; i < $numRadianceSamples; i++ )
			{
				for ( int j = 0; j < $numRadianceSamples; j++ )
				{
					// Compute rsm texture coordinates		
					float2 sampleUV  = float2( j, i ) * float2( 0.25, 0.25 ) + float2( 0.125, 0.125 ); 
				?>
					// Accumulate the new sample				
					addSample( "cellCenter", "sampleUV", "avgSHR", "avgSHG", "avgSHB", "weightSum", numOcclusionSamples );
				<?
				}        
			}        
			?>
		} // End if 4x4
        else
        {
			// Optionally project the cell position into the RSM for localizing samples
			if ( samplingMode == 1 )
			{
				<?
				float4 projTexCoords = mul( float4( cellCenter, 1.0 ), _rsmTexProjMatrix );
				float2 rsmUV = projTexCoords.xy / projTexCoords.w; 
	
				// Handle behind near plane cases
				float reverse = sign( dot( _rsmDirection, cellCenter - _rsmPosition ) ); 
				rsmUV = saturate( rsmUV * reverse * 0.5 + 0.5 );
				?>
			}
			else
			{
				// Start from the center of the map
				<?float2 rsmUV = float2( 0.5, 0.5 );?>
			}

			<?
			// For each RSM sample we will take
			for ( int i = 0; i < $numRadianceSamples; i++ )
			{
				// Get a random value
				float3 rndOffset = sample2DLevel( sRandomTex, sRandom, float2( float(i) / $numRadianceSamples, 0 ), 0 ).xyz * 2.0f - 1.0f;
				
				// Compute a sample location in the cell
				float3 samplePosition = cellCenter + rndOffset * halfCell;
				
				// Compute rsm texture coordinates		
				float2 sampleUV  = rsmUV + rndOffset.xy * _sampleRadius;
			?>
				// Accumulate the new sample				
				addSample( "samplePosition", "sampleUV", "avgSHR", "avgSHG", "avgSHB", "weightSum", numOcclusionSamples );
			<?
			} // Next sample
			?>
			
		} // End if random
				
		<?
		// Compute a final scalar to include HDR, global indirect, pi, and recip sum for average
		float f = (1.0f / 3.14159) * (1.0f / weightSum) * _gridColor.a;
		
		// Average the radiance
		data0 = avgSHR * f * _rsmColor.r;
		data1 = avgSHG * f * _rsmColor.g;
		data2 = avgSHB * f * _rsmColor.b;
		?>
            
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
    // Name : transformGrid() (Vertex Shader)
	// Desc : Used to transform grid shapes for stencil filling or screen execution.
	//-----------------------------------------------------------------------------
	bool transformGrid( bool stencilOnly )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
		    float3 sourcePosition   : POSITION;
        ?>

		// Define shader outputs.
		<?out
			float4 clipPosition     : SV_POSITION;
		?>
		if ( !stencilOnly )
		{
			<?out
			   float4 texCoords     : TEXCOORD0;
			   float3 eyeRay        : TEXCOORD1;
			?>
		
        } // End if !stencilOnly

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbWorld;
        ?>
        
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		<?			
		// Transform geometry to clip space
        float4 worldPosition = mul( float4( sourcePosition, 1 ), _worldMatrix );
        clipPosition = mul( worldPosition, _viewProjectionMatrix );
		?>

		// Compute data necessary for g-buffer lookups			
		if ( !stencilOnly )
		{
			<?
			// This ray goes from the camera position to the vertex
            eyeRay = worldPosition.xyz - _cameraPosition;		
			
			// Compute texture coordinates for our lookup
			computeMeshScreenTextureCoords( clipPosition, texCoords );
            ?>
		
        } // End if !stencilOnly
        
        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : compositeRadiance() (Pixel Shader)
	// Desc : Used when drawing RSMs into radiance volume. 
    //-------------------------------------------------------------------------
    bool compositeRadiance( int renderFlags, int shadingQuality )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
			float4   texCoords      : TEXCOORD0;
			float3   eyeRay         : TEXCOORD1;
        ?>

        // Define shader outputs.
		<?out
			float4  data0           : SV_TARGET0;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbGrid;
            _cbCamera;
        ?>
       
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
         
		// Declare our lighting structures
		initLightingStructures( true, false );
        <?
            SurfaceData surface = (SurfaceData)0;
        ?>

        // Compute screen space texture coordinates for g-buffer lookups
        <?surface.screenCoords = texCoords.xy / texCoords.w;?>

		// Get the data needed for lighting
		getGBufferData( "surface", "eyeRay", renderFlags, shadingQuality, true );
      
		// If the g-buffer data is in view space, transform it to world space
		if ( testFlagAny( renderFlags, RenderFlags::ViewSpaceLighting ) )
		{
			<?
			surface.position      = mul( float4( surface.position, 1 ), _inverseViewMatrix );
			surface.normal        = normalize( mul( surface.normal, _inverseViewMatrix ) );
			surface.surfaceNormal = mul( surface.surfaceNormal, _inverseViewMatrix );
			?>		
		}
		
		int   nNumSamples  = 4;
		float recipSamples = 1.0f / float(nNumSamples);
		
		<?
		int j;
		float4 shR, shG, shB;
		float3 indirectLighting = 0;
	    float4 shLobe = float4( 0.886226925f, -1.02332671f * -surface.normal.y, 1.02332671f * -surface.normal.z, -1.02332671f * -surface.normal.x );
		float3 gridSize   = _gridMax - _gridMin;
		float3 recipSize  = 1.0f / gridSize;

		// Always take at least one sample
		float3 offsets[ 4 ];
		offsets[ 0 ] = surface.surfaceNormal; //(surface.surfaceNormal + surface.normal) * 0.5;
		?>
	
		// Optionally take additional samples for smoothness
		if ( nNumSamples > 1 )
		{
			<?
			// Create a matrix to transform vectors from tangent to world space
			half3 N  = normalize( surface.surfaceNormal );
			half3 up = half3( 0, 1, 0 );
			if ( abs( N.y ) > 0.99995 )
				up = half3( 1, 0, 0 );

			float3x3 tmpTS;
			tmpTS[0] = normalize( cross( N, up ) );
			tmpTS[1] = cross( N, tmpTS[0] );
			tmpTS[2] = N;

			// We'll use the HL2 rnm basis directions for our extra samples
			offsets[ 1 ] = mul( float3( -0.40824825,  0.70710671, 0.57735026 ), tmpTS );
			offsets[ 2 ] = mul( float3( -0.40824825, -0.70710671, 0.57735026 ), tmpTS );
			offsets[ 3 ] = mul( float3(  0.81649655,  0.00000000, 0.57735026 ), tmpTS );
			?>
        }
        <?
        float4 avgSHR = 0, avgSHG = 0, avgSHB = 0;
		for ( j = 0; j < $nNumSamples; j++ )
		{
			float3 adjPosition = surface.position + offsets[ j ] * (1.414213 * gridSize / _gridDimensions) * 0.5f;
			float3 volCoord    = (adjPosition - _gridMin) * recipSize;
			       volCoord.y  = 1.0f - volCoord.y;
			       volCoord    = saturate( volCoord );
			?>
			sampleVolumeSH2( "volCoord", "_gridDimensions", "sSHRTex", "sSHGTex", "sSHBTex", "sSHR", "sSHG", "sSHB", "shR", "shG", "shB", true ); 
			<?
			float3 res = float3( dot( shLobe, shR ), dot( shLobe, shG ), dot( shLobe, shB ) );
		    indirectLighting += max( 0, res );

			avgSHR += shR;
			avgSHG += shG; 
			avgSHB += shB;
		}
		indirectLighting *= $recipSamples;

		bool useSSAO = false;
		if ( useSSAO )
		{
			float lum = dot( indirectLighting, float3( 0.2125, 0.7154, 0.0721 ) );
			indirectLighting = lerp( indirectLighting * surface.ambientOcclusion, indirectLighting, saturate( lum * 3 ) ); 
		}
		
		// Optional "specular" component
		float3 specular = 0;
		bool computeSpecular = false;
		if ( computeSpecular )
		{
			float3 lightDirR  = normalize( float3( avgSHR.w, avgSHR.y, -avgSHR.z ) * $recipSamples );
			float3 lightDirG  = normalize( float3( avgSHG.w, avgSHG.y, -avgSHG.z ) * $recipSamples );
			float3 lightDirB  = normalize( float3( avgSHB.w, avgSHB.y, -avgSHB.z ) * $recipSamples );
			float3 weights    = float3( avgSHR.x, avgSHG.x, avgSHB.x );
			       weights   /= dot( weights, float3( 1.0, 1.0, 1.0 ) );
			float3 lightDir   = normalize( (lightDirR * weights.x) + (lightDirG * weights.y) + (lightDirB * weights.z) );
			float3 eyeDir     = normalize( _cameraPosition - surface.position );
			float3 halfVector = normalize( lightDir + eyeDir );
				   specular   = indirectLighting * 0.2 * pow( saturate( dot( surface.normal, halfVector ) ), 9.0 );
		}
		
	    // Compute an inter-grid blend term as needed
		float3 halfSize, planeDist, fadeDist, center, delta, perAxis;
        float blend = 1.0f;
		
		// Compute the blend-to term (i.e., fade out this grid to the next grid in line)
		if ( _blendRegion > 0.0f )
		{
			halfSize  = gridSize * 0.5f;
			planeDist = (1.0f - _blendRegion) * halfSize;
			fadeDist  = _blendRegion * halfSize;
			center    = (_gridMin + _gridMax) * 0.5f; 
			delta     = abs( surface.position - center );
			perAxis   = saturate( ( delta - planeDist ) / (fadeDist) ); 
			blend    *= 1.0f - max( perAxis.x, max( perAxis.y, perAxis.z ) );
		}
		
		// Compute the blend-from term (i.e., fade in this grid from the previous grid in line)
		if ( _blendRegionPrev > 0.0f )
		{
			halfSize  = (_gridMaxPrev - _gridMinPrev) * 0.5f;
			planeDist = (1.0f - _blendRegionPrev) * halfSize;
			fadeDist  = _blendRegionPrev * halfSize;
			center    = (_gridMinPrev + _gridMaxPrev) * 0.5f; 
			delta     = abs( surface.position - center );
			perAxis   = saturate( ( delta - planeDist ) / (fadeDist) ); 
			blend    *= max( perAxis.x, max( perAxis.y, perAxis.z ) );
		}
		
		// Return final results        
        data0 = float4( (specular + indirectLighting * surface.diffuse) * blend, 0 );
		?>
            
        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : computeIrradiance() (Pixel Shader)
	// Desc : Computes irradiance
    //-------------------------------------------------------------------------
    bool computeIrradiance( int renderFlags, int shadingQuality )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
			float4   texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
		<?out
			float4  data0           : SV_TARGET0;
			float4  data1           : SV_TARGET1;
			float4  data2           : SV_TARGET2;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbGrid;
            _cbCamera;
        ?>
       
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		int   numSamples  = 4;
		float recipSamples = 1.0f / float(numSamples);
		bool  useAO = false;
		
		<?
		float2 depthData;
		float4 normalData;
		float3 position, normal;
		?>
         
        // Compute screen space texture coordinates for g-buffer lookups
        <?float2 screenCoords = texCoords.xy / texCoords.w;?>

		// Sample depth (alpha contains the texel offset from the higher res texture to ease position reconstruction)
		sampleDepthBuffer( "depthData", "sDepthTex", "sDepth", "screenCoords", true, DepthType::LinearZ_Packed );

		<?
		// Reconstruct the view space position
		float2 highResCoords;
		float  highResOffsetID = depthData.y * 2.0f - 1.0f;
		if ( highResOffsetID < 0 )
		{
			// Top row
			if ( highResOffsetID > 0.9 )
				highResCoords = float2( -0.25, -0.25 );
			else
				highResCoords = float2(  0.25, -0.25 );
		}
		else
		{
			// Bottom row
			if ( highResOffsetID > 0.9 )
				highResCoords = float2(  0.25,  0.25 );
			else
				highResCoords = float2( -0.25,  0.25 );
		}
		highResCoords = screenCoords + highResCoords * (0.5 * _targetSize.zw);
        position = getViewPosition( highResCoords, depthData.x, $false );

		// Sample surface normal
		normalData = sample2D( sNormalTex, sNormal, screenCoords );
		normal     = normalize( normalData.xyz * 2.0f - 1.0f );
		?>
     
		// If the g-buffer data is in view space, transform it to world space
		if ( testFlagAny( renderFlags, RenderFlags::ViewSpaceLighting ) )
		{
			<?
			position = mul( float4( position, 1 ), _inverseViewMatrix );
			normal   = mul( normal, _inverseViewMatrix );
			?>		
		}
	
		<?
		int j;
		float4 shR, shG, shB, avgSHR = 0, avgSHG = 0, avgSHB = 0;
		float3 gridSize  = _gridMax - _gridMin;
		float3 recipSize = 1.0f / gridSize;

		// Always take at least one sample
		float3 offsets[ 4 ];
		offsets[ 0 ] = normal;
		?>
	
		// Optionally take additional samples for smoothness
		if ( numSamples > 1 )
		{
			<?
			// Create a matrix to transform vectors from tangent to world space
			half3 N  = normalize( normal );
			half3 up = half3( 0, 1, 0 );
			if ( abs( N.y ) > 0.99995 )
				up = half3( 1, 0, 0 );

			float3x3 tmpTS;
			tmpTS[0] = normalize( cross( N, up ) );
			tmpTS[1] = cross( N, tmpTS[0] );
			tmpTS[2] = N;

			// We'll use the HL2 rnm basis directions for our extra samples
			offsets[ 1 ] = mul( float3( -0.40824825,  0.70710671, 0.57735026 ), tmpTS );
			offsets[ 2 ] = mul( float3( -0.40824825, -0.70710671, 0.57735026 ), tmpTS );
			offsets[ 3 ] = mul( float3(  0.81649655,  0.00000000, 0.57735026 ), tmpTS );
			?>
        }
        <?
		for ( j = 0; j < $numSamples; j++ )
		{
			float3 adjPosition = position + offsets[ j ] * (0.5f * 1.414213 * (gridSize / _gridDimensions));
			float3 volCoord    = (adjPosition - _gridMin) * recipSize;
			       volCoord.y  = 1.0f - volCoord.y;
			       volCoord    = saturate( volCoord );
			?>
			sampleVolumeSH2( "volCoord", "_gridDimensions", "sSHRTex", "sSHGTex", "sSHBTex", "sSHR", "sSHG", "sSHB", "shR", "shG", "shB", true ); 
			<?
			avgSHR += shR;
			avgSHG += shG; 
			avgSHB += shB;
		}

		// Compute final weight
		float s = $recipSamples;
		
		// Compute the blend-to term (i.e., fade out this grid to the next grid in line)
		float3 halfSize, planeDist, fadeDist, center, delta, perAxis;
		if ( _blendRegion > 0.0f )
		{
			halfSize  = gridSize * 0.5f;
			planeDist = (1.0f - _blendRegion) * halfSize;
			fadeDist  = _blendRegion * halfSize;
			center    = (_gridMin + _gridMax) * 0.5f; 
			delta     = abs( position - center );
			perAxis   = saturate( ( delta - planeDist ) / (fadeDist) ); 
			s        *= 1.0f - max( perAxis.x, max( perAxis.y, perAxis.z ) );
		}
		
		// Compute the blend-from term (i.e., fade in this grid from the previous grid in line)
		if ( _blendRegionPrev > 0.0f )
		{
			halfSize  = (_gridMaxPrev - _gridMinPrev) * 0.5f;
			planeDist = (1.0f - _blendRegionPrev) * halfSize;
			fadeDist  = _blendRegionPrev * halfSize;
			center    = (_gridMinPrev + _gridMaxPrev) * 0.5f; 
			delta     = abs( position - center );
			perAxis   = saturate( ( delta - planeDist ) / (fadeDist) ); 
			s        *= max( perAxis.x, max( perAxis.y, perAxis.z ) );
		}
		?>
	
		// Apply optional AO (Note: Must be available in normal texture alpha channel)
		if ( useAO )
		{
			<?
			float ssaoScale = 1; // ToDo
			float ssaoBias  = 0; // ToDo
			float maxCoeff0 = max( avgSHR.x, max( avgSHG.x, avgSHB.x );
			float t = saturate( maxCoeff0 * ssaoScale + ssaoBias ); 
			s *= lerp( 1.0f, normalData.a, t );
			?>
		}
		
		<?
		// Return final results        
        data0 = avgSHR * s;
        data1 = avgSHG * s;
        data2 = avgSHB * s;
		?>
            
        // Valid shader
        return true;
	}
	
	//-----------------------------------------------------------------------------
	// Name: blurSH()
	// Desc: NxN bilateral blur on low resolution indirect spherical harmonics
	//-----------------------------------------------------------------------------
	bool blurSH( int pixelRadius, float distanceFactor, bool vertical, int depthType )
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
			float4  data0       : SV_TARGET0;
			float4  data1       : SV_TARGET1;
			float4  data2       : SV_TARGET2;
		?>

		// Constant buffer usage.
		<?cbufferrefs
			cbBilateral;
			_cbCamera
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float4 shR = 0, shG = 0, shB = 0;
		float2 sampleCoords;
		float  weight, totalWeight = 0;
		float  depthRef, depthSmp;
		?>

		// Initialize kernel and get number of sampling iterations
		int numSamples = initializeKernel( pixelRadius, distanceFactor, false, vertical );

		// Retrieve reference pixel data
		sampleDepthBuffer( "depthRef", "sDepthTex", "sDepth", "texCoords", false, depthType );

		// Retrieve our samples and weights, keeping track of total weight		
		for ( int i = 0; i < numSamples; i++ )
		{
			// Compute the texture coordinates for this sample
			<?sampleCoords = texCoords + filterKernel[ $i ].xy * textureSize.zw;?>

			// Compute weight (Skip comparisons on index 0 (i.e., the central sample) as geometric weight will always be 1.)
			if ( i > 0 )
			{
				sampleDepthBuffer( "depthSmp", "sDepthTex", "sDepth", "sampleCoords", false, depthType );
				<?weight = filterKernel[ $i ].z / ( abs( depthRef - depthSmp ) * depthScaleBias.x + depthScaleBias.y );?>
			}
			else
			{
				<?weight = filterKernel[ $i ].z;?>
			}

			// Accumulate weighted sh coeffs
			if ( vertical )
			{
				<?
				shR += sample2D( sSHRVerticalTex,   sSHRVertical,   sampleCoords ) * weight;
				shG += sample2D( sSHGVerticalTex,   sSHGVertical,   sampleCoords ) * weight;
				shB += sample2D( sSHBVerticalTex,   sSHBVertical,   sampleCoords ) * weight;
				?>
			}
			else
			{
				<?
				shR += sample2D( sSHRHorizontalTex, sSHRHorizontal, sampleCoords ) * weight;
				shG += sample2D( sSHGHorizontalTex, sSHGHorizontal, sampleCoords ) * weight;
				shB += sample2D( sSHBHorizontalTex, sSHBHorizontal, sampleCoords ) * weight;
				?>
			}	

			// Accumulate the combined weight for color normalization
			<?totalWeight += weight;?>

		} // Next sample

		<?
		// Return normalized results
		float recipWeight = 1.0f / totalWeight;
		data0 = shR * recipWeight;
		data1 = shG * recipWeight;
		data2 = shB * recipWeight;
		?>
		
		// Valid shader
		return true;
	}
	
    //-------------------------------------------------------------------------
    // Name : compositeIrradiance() (Pixel Shader)
	// Desc : Upsamples and computes final indirect lighting 
    //-------------------------------------------------------------------------
    bool compositeIrradiance( int renderFlags, int shadingQuality, bool computeSpecular, bool bilateral )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
			float2   texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
		<?out
			float4  data0           : SV_TARGET0;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbGrid;
            _cbCamera;
            cbBilateral;
        ?>
       
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        
		// Declare our lighting structures
		initLightingStructures( true, false );
        <?
            SurfaceData surface = (SurfaceData)0;
        ?>

        // Compute screen space texture coordinates for g-buffer lookups
        <?surface.screenCoords = texCoords.xy;?>

		// Get the surface data needed for resolving lighting
		getGBufferData( "surface", "float3(0,0,0)", renderFlags, shadingQuality, true );
      
		// If the g-buffer data is in view space, transform it to world space
		if ( testFlagAny( renderFlags, RenderFlags::ViewSpaceLighting ) )
		{
			<?
			surface.normal   = mul( surface.normal, _inverseViewMatrix );
			surface.position = mul( float4( surface.position, 1 ), _inverseViewMatrix );
			?>		
		}
		
		<?float4 shR, shG, shB;?>

		// Upsample the irradiance (bilinear or bilateral)
		if ( bilateral )
		{
			<?
			float  depthSample;
			float2 srcCoords[4];
			float4 subTexelDist, srcDepths, depthDeltas, bilinearWeights;

			// Compute the four closest sampling locations in the lower resolution texture
			srcCoords[0] = texCoords + float2( -0.5, -0.5 ) * textureSize.zw;
			srcCoords[1] = texCoords + float2(  0.5, -0.5 ) * textureSize.zw;
			srcCoords[2] = texCoords + float2( -0.5,  0.5 ) * textureSize.zw;
			srcCoords[3] = texCoords + float2(  0.5,  0.5 ) * textureSize.zw;
			?>

			// Get the depths
			sampleDepthBuffer( "depthSample", "sDepthTex", "sDepth", "texCoords", false, DepthType::LinearZ_Packed );
			sampleDepthBuffer( "srcDepths[ 0 ]", "sDepthLowResTex", "sDepthLowRes", "srcCoords[ 0 ]", false, DepthType::LinearZ_Packed );
			sampleDepthBuffer( "srcDepths[ 1 ]", "sDepthLowResTex", "sDepthLowRes", "srcCoords[ 1 ]", false, DepthType::LinearZ_Packed );
			sampleDepthBuffer( "srcDepths[ 2 ]", "sDepthLowResTex", "sDepthLowRes", "srcCoords[ 2 ]", false, DepthType::LinearZ_Packed );
			sampleDepthBuffer( "srcDepths[ 3 ]", "sDepthLowResTex", "sDepthLowRes", "srcCoords[ 3 ]", false, DepthType::LinearZ_Packed );

			<?
			// Compute bilinear weights for these samples
			subTexelDist.xy = frac( srcCoords[0] * textureSize.xy );
			subTexelDist.zw = 1.0f - subTexelDist.xy;
			bilinearWeights = subTexelDist.zxzx * subTexelDist.wwyy;
			
			// Compute depth weights for these samples
			depthDeltas = abs( depthSample - srcDepths );
			bilinearWeights *= 1.0f / (depthDeltas * depthScaleBias.x + depthScaleBias.y);

			// Normalize the weights
			float weightSum  = dot( bilinearWeights, float4( 1, 1, 1, 1 ) );
			bilinearWeights *= (1.0f / weightSum);

			// Get the low res SH coeffs and blend
			shR = sample2D( sSHRPointTex, sSHRPoint, srcCoords[ 0 ] ) * bilinearWeights.x + 
				  sample2D( sSHRPointTex, sSHRPoint, srcCoords[ 1 ] ) * bilinearWeights.y + 
				  sample2D( sSHRPointTex, sSHRPoint, srcCoords[ 2 ] ) * bilinearWeights.z + 
				  sample2D( sSHRPointTex, sSHRPoint, srcCoords[ 3 ] ) * bilinearWeights.w;

			shG = sample2D( sSHGPointTex, sSHGPoint, srcCoords[ 0 ] ) * bilinearWeights.x + 
				  sample2D( sSHGPointTex, sSHGPoint, srcCoords[ 1 ] ) * bilinearWeights.y + 
				  sample2D( sSHGPointTex, sSHGPoint, srcCoords[ 2 ] ) * bilinearWeights.z + 
				  sample2D( sSHGPointTex, sSHGPoint, srcCoords[ 3 ] ) * bilinearWeights.w;

			shB = sample2D( sSHBPointTex, sSHBPoint, srcCoords[ 0 ] ) * bilinearWeights.x + 
				  sample2D( sSHBPointTex, sSHBPoint, srcCoords[ 1 ] ) * bilinearWeights.y + 
				  sample2D( sSHBPointTex, sSHBPoint, srcCoords[ 2 ] ) * bilinearWeights.z + 
				  sample2D( sSHBPointTex, sSHBPoint, srcCoords[ 3 ] ) * bilinearWeights.w;
			?>
			
		} // End if bilateral upsample
		else
		{
			<?
			shR = sample2D( sSHRTex, sSHR, texCoords );
			shG = sample2D( sSHGTex, sSHG, texCoords );
			shB = sample2D( sSHBTex, sSHB, texCoords );
			?>
			
		} // End if bilinear
		
		<?
		// Resolve the spherical harmonics
		float4 shLobe = float4( 0.886226925f, -1.02332671f * -surface.normal.y, 1.02332671f * -surface.normal.z, -1.02332671f * -surface.normal.x );
		float3 res = float3( dot( shLobe, shR ), dot( shLobe, shG ), dot( shLobe, shB ) );
	    float3 indirectLighting = max( 0, res ) * surface.diffuse;
		?>
		
		// Optional "specular" component
		if ( computeSpecular )
		{
			<?
			float3 lightDirR  = normalize( float3( shR.w, shR.y, -shR.z ) );
			float3 lightDirG  = normalize( float3( shG.w, shG.y, -shG.z ) );
			float3 lightDirB  = normalize( float3( shB.w, shB.y, -shB.z ) );
			float3 weights    = float3( shR.x, shG.x, shB.x );
			       weights   /= dot( weights, float3( 1.0, 1.0, 1.0 ) );
			float3 lightDir   = normalize( (lightDirR * weights.x) + (lightDirG * weights.y) + (lightDirB * weights.z) );
			float3 eyeDir     = normalize( _cameraPosition - surface.position );
			float3 halfVector = normalize( lightDir + eyeDir );
			float3 specular   = indirectLighting * surface.specular * pow( saturate( dot( surface.normal, halfVector ) ), surface.specularPower );
			indirectLighting += specular;
			?>
		}
		
		// Return final results        
        <?data0 = float4( indirectLighting, 0 );?>

        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : copyGrid() (Pixel Shader)
	// Desc : Used when copying from one grid to another
    //-------------------------------------------------------------------------
    bool copyGrid( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
			float4   texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
		<?out
			float4  data0           : SV_TARGET0;
			float4  data1           : SV_TARGET1;
			float4  data2           : SV_TARGET2;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbGrid;
            _cbCamera;
        ?>
       
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		<?         
		float3 coords3D;
		float2 coords2D;

		// Compute destination grid cell coords
		coords3D.z = floor( screenPosition.x / _gridDimensions.x );
		coords3D.y = screenPosition.y;
		coords3D.x = screenPosition.x - (coords3D.z * _gridDimensions.x);
		
		// Shift by delta to get source cell coords
		coords3D += _gridShift;

		// Center x and y 
		coords3D.xy += 0.5f;
		
		// Respect boundaries (necessary?)
		coords3D = clamp( coords3D, 0, _gridDimensions ); 
		
		// Convert to volume coords
		coords3D /= _gridDimensions;
		
		// Convert 3D to 2D coords
		float slice = floor( coords3D.z * _gridDimensions.z );
		coords2D.x  = (coords3D.x + slice) / _gridDimensions.z;
		coords2D.y  = coords3D.y;
		
		// Do the lookup
		data0 = sample2D( sSHRPointTex, sSHRPoint, coords2D );
		data1 = sample2D( sSHGPointTex, sSHGPoint, coords2D );
		data2 = sample2D( sSHBPointTex, sSHBPoint, coords2D );
		?>
            
        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : readGrid() (Pixel Shader)
	// Desc : Used when copying or adding grids together
    //-------------------------------------------------------------------------
    bool readGrid( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
			float2   texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
		<?out
			float4  data0           : SV_TARGET0;
			float4  data1           : SV_TARGET1;
			float4  data2           : SV_TARGET2;
		?>
		
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		<?         
		data0 = sample2D( sSHRTex, sSHR, texCoords );
		data1 = sample2D( sSHGTex, sSHG, texCoords );
		data2 = sample2D( sSHBTex, sSHB, texCoords );
		?>
            
        // Valid shader
        return true;
	}

    //-------------------------------------------------------------------------
    // Name : reprojectGrid() (Pixel Shader)
	// Desc : Used when projecting values from one grid to another
	// Note : Type: 0 = nothing, 1 = clamp copy, 2 = test box
    //-------------------------------------------------------------------------
    bool reprojectGrid( int nType )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4   screenPosition : SV_POSITION;
			float4   texCoords      : TEXCOORD0;
        ?>

        // Define shader outputs.
		<?out
			float4  data0           : SV_TARGET0;
			float4  data1           : SV_TARGET1;
			float4  data2           : SV_TARGET2;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbGrid;
            _cbCamera;
        ?>
       
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		<?         
		float3 coords3D;
		float2 coords2D;
		float  pointInGrid = 1.0f;
		float  blendAmt = 1.0f;

		// Compute destination grid cell coords
		coords3D.z = floor( screenPosition.x / _gridDimensions.x );
		coords3D.y = screenPosition.y;
		coords3D.x = screenPosition.x - (coords3D.z * _gridDimensions.x);
		
		// Shift by delta to get source cell coords
		coords3D += _gridShift;

		// Center x and y 
		coords3D.xy += 0.5f;
		
		// Convert to volume coords
		coords3D /= _gridDimensions;
		?>
		
		if ( nType == 1 )
		{
			<?coords3D = saturate( coords3D );?>
		}
		else if ( nType == 2 )
		{
			<?
			// Compute world position
			float3 extents    = _gridMax - _gridMin;
			float3 adjCoord   = float3( coords3D.x, 1.0f - coords3D.y, coords3D.z );
			float3 worldPos   = _gridMin + extents * adjCoord;
			
            // Is position inside the grid? (we use prev grid constants for this test)
			pointInGrid = isPointInAABB( worldPos, _gridMinPrev, _gridMaxPrev );

			// Compute amount to use
			blendAmt = ( pointInGrid > 0.0f ) ? _blendRegion : _blendRegionPrev;
            ?>
		}

		<?
		// Convert 3D to 2D coords
		float slice = floor( coords3D.z * _gridDimensions.z );
		coords2D.x  = (coords3D.x + slice) / _gridDimensions.z;
		coords2D.y  = coords3D.y;
		
		// Do the lookup and scale by interpolant value
		data0 = sample2D( sSHRPointTex, sSHRPoint, coords2D ) * blendAmt;
		data1 = sample2D( sSHGPointTex, sSHGPoint, coords2D ) * blendAmt;
		data2 = sample2D( sSHBPointTex, sSHBPoint, coords2D ) * blendAmt;
		?>
            
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : SSGI 
	// Desc : Screen space global illumination
	//-----------------------------------------------------------------------------
	bool SSGI( )
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
            float4  data0       : SV_TARGET0;
            float4  data1       : SV_TARGET1;
            float4  data2       : SV_TARGET2;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			_cbCamera;
			cbBilateral;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        
        int numSamples = 12;
        
        // Initialize the poisson sampling distribution
        // initializePoissonKernel( min( 16, numSamples ), false );
        initializeHaltonKernel( numSamples );
        
		<?
		float3 position, smpPosition;
		float2 offset;
		float  cosine, zRef, zSample, weight, weightSum = 0;
		float2 focalLength = float2( _projectionMatrix._11, _projectionMatrix._22 ) * 0.5; 
		float4 shR = 0, shG = 0, shB = 0, shCoeffs = float4( 0.282094792f, -0.488602512f, 0.488602512f, -0.488602512f );

		float radius = 15.0f;
		float halfR  = radius * 0.5;

		// Get the rotation data (r=cos,g=sin) Note: rotation is tiled every 4x4 screen pixels
		float2 cosSin = sample2D( sRandomTex, sRandom, texCoords * _viewportSize.xy * 0.25 ).rg;

		// Get the normal
		half3 N = sample2D( sNormalTex, sNormal, texCoords ).xyz * 2.0 - 1.0;
		?>

		// Compute the position of this pixel in view space
		getDistance( "zRef", "sDepthTex", "sDepth", "texCoords", DepthType::LinearZ_Packed );

		<?
        // Compute view space position
		position = getViewPosition( texCoords, zRef, $false );
		position += N * 0.2f;
		
		// Apply perspective, FOV, and aspect ratio correct to the radius
		// float2 screenRad = ( halfR * focalLength ) / zRef;
		float2 screenRad = 8.0f * textureSize.zw;

		// Iterate our samples and compute AO
		for( int j = 0; j < $numSamples; j++ )
		{
			// Rotate the sample offset about Z axis
			//offset.x = cosSin.x * poissonSamples[j].x - cosSin.y * poissonSamples[j].y;
			//offset.y = cosSin.y * poissonSamples[j].x + cosSin.x * poissonSamples[j].y;

			//offset.x = poissonSamples[j].x;
			//offset.y = poissonSamples[j].y;

			offset.x = haltonSamples[j].x;
			offset.y = haltonSamples[j].y;
			
			// Compute texture lookup coords 
			float2 uv = texCoords + offset * screenRad;
			//float2 uv = texCoords + offset * textureSize.zw;
			?>

			// Get the samples' camera space Z
			getDistance( "zSample", "sDepthTex", "sDepth", "uv", DepthType::LinearZ_Packed );

			<?
            // Convert to a view space coordinate
			smpPosition = getViewPosition( uv, zSample, $false );

			// Get the ray to the occluder
			float3 ray = smpPosition - position;
			
			// Get the distance between the two points
			float distance = length( ray );

			// Normalize the occlusion direction
			ray /= distance;

			// Sample the color
			float3 smpColor = sample2D( sLightingTex, sLighting, uv ) * 5.0f;

			// Sample the normal
			float3 smpNormal = normalize( sample2D( sNormalTex, sNormal, uv ).rgb * 2.0f - 1.0f );
			
			// Compute dot product and attenuation
			float atten = max( 0, dot( smpNormal, -ray ) ) / (0.0001f + (distance * distance) ); 

			// If this location has little to no lighting, don't allow it to count as much
			float weight = dot( smpColor.rgb, float3( 0.2125, 0.7154, 0.0721 ) );
			//atten     *= weight;
			//weightSum += weight;

			// Attenuate color
			smpColor *= atten;

			// Convert to SH and accumulate
			float4 shRay = shCoeffs * float4( 1, -smpNormal.yzx );
			shR += smpColor.r * shRay;
			shG += smpColor.g * shRay;
			shB += smpColor.b * shRay;
		
		} // Next sample
		
		// Normalize
		float recipSum = 0;	
		//if ( weightSum > 0.0f )
			recipSum = 1.0f / 12.0; //weightSum;	
			
		data0 = shR * recipSum;
		data1 = shG * recipSum;
		data2 = shB * recipSum;
		?>
		
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : drawGridVS 
	// Desc : Vertex shader for grid debug drawing
	//-----------------------------------------------------------------------------
	bool drawGridVS( ) 
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float3  position : POSITION;
        ?>

        // Define shader outputs.
		<?out
			float4  clipPosition : SV_POSITION;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbGrid;
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		float3 gridSize   = _gridMax - _gridMin;
		float3 gridOrigin = _gridMin + float3( 0, gridSize.y, 0 );
		float3 worldPos   = gridOrigin + position * gridSize * float3(1.0, -1.0, 1.0);
		clipPosition      = mul( float4( worldPos, 1.0f ), _viewProjectionMatrix );
		?>

	    // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : drawGridPS 
	// Desc : Pixel shader for grid debug drawing
	//-----------------------------------------------------------------------------
    bool drawGridPS( )
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
			float4  data0          : SV_TARGET0;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbGrid;
        ?>
       
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?data0 = _gridColor;?>
            
        // Valid shader
        return true;
	}

	////////////////////////////////////////////////////////////////////////////////
	//-----------------------------------------------------------------------------
	// Name  : initLPVStructures()
	// Desc  : Declares and initializes the "global" propagation volume data
	//-----------------------------------------------------------------------------
	void initLPVData( )
	{
		// Build the structures dynamically and add to the global
		// section of the shader permutation.
		<?global    

		float3 neighborOffsets[ 6 ] = 
		{
			float3( 1,  0,  0 ), float3( -1,  0,  0 ),
			float3( 0, -1,  0 ), float3(  0,  1,  0 ),
			float3( 0,  0,  1 ), float3(  0,  0, -1 ),
		};
		
		struct neighborSHData
		{
			float4 shDir;       // Precomputed spherical harmonic for direction from source center to face center
			float4 cosineLobe;  // Precomputed cosine lobe for negated face normal pre-multiplied by solid angle / pi
			float  fluxAdjust;  // Solid angle / 4pi
		};
		
		neighborSHData neighborData[ 30 ] =
		{
			{ {0.28209481,0.00000000,0.00000000,0.48860249}, {0.88622695,0.00000000,0.00000000,1.02332675}, {0.12753712} }, //<-1.000,0.000,0.000>
			{ {0.28209481,0.24923933,-0.00000000,0.42025253}, {0.88622695,1.02332675,0.00000000,0.00000000}, {0.13478556} }, //<-0.860,-0.510,-0.000>
			{ {0.28209481,-0.24923933,-0.00000000,0.42025253}, {0.88622695,-1.02332675,0.00000000,0.00000000}, {0.13478556} }, //<-0.860,0.510,-0.000>
			{ {0.28209481,0.00000000,-0.24923933,0.42025253}, {0.88622695,0.00000000,-1.02332675,0.00000000}, {0.13478556} }, //<-0.860,-0.000,-0.510>
			{ {0.28209481,0.00000000,0.24923933,0.42025253}, {0.88622695,0.00000000,1.02332675,0.00000000}, {0.13478556} }, //<-0.860,-0.000,0.510>

			{ {0.28209481,0.00000000,0.00000000,-0.48860249}, {0.88622695,0.00000000,0.00000000,-1.02332675}, {0.12753712} }, //<1.000,0.000,0.000>
			{ {0.28209481,0.24923933,0.00000000,-0.42025253}, {0.88622695,1.02332675,0.00000000,0.00000000}, {0.13478556} }, //<0.860,-0.510,0.000>
			{ {0.28209481,-0.24923933,0.00000000,-0.42025253}, {0.88622695,-1.02332675,0.00000000,0.00000000}, {0.13478556} }, //<0.860,0.510,0.000>
			{ {0.28209481,0.00000000,-0.24923933,-0.42025253}, {0.88622695,0.00000000,-1.02332675,0.00000000}, {0.13478556} }, //<0.860,0.000,-0.510>
			{ {0.28209481,0.00000000,0.24923933,-0.42025253}, {0.88622695,0.00000000,1.02332675,0.00000000}, {0.13478556} }, //<0.860,0.000,0.510>

			{ {0.28209481,0.42025253,0.00000000,0.24923933}, {0.88622695,0.00000000,0.00000000,1.02332675}, {0.13478556} }, //<-0.510,-0.860,0.000>
			{ {0.28209481,0.42025253,0.00000000,-0.24923933}, {0.88622695,0.00000000,0.00000000,-1.02332675}, {0.13478556} }, //<0.510,-0.860,0.000>
			{ {0.28209481,0.48860249,0.00000000,0.00000000}, {0.88622695,1.02332675,0.00000000,0.00000000}, {0.12753712} }, //<0.000,-1.000,0.000>
			{ {0.28209481,0.42025253,-0.24923933,0.00000000}, {0.88622695,0.00000000,-1.02332675,0.00000000}, {0.13478556} }, //<0.000,-0.860,-0.510>
			{ {0.28209481,0.42025253,0.24923933,0.00000000}, {0.88622695,0.00000000,1.02332675,0.00000000}, {0.13478556} }, //<0.000,-0.860,0.510>

			{ {0.28209481,-0.42025253,0.00000000,0.24923933}, {0.88622695,0.00000000,0.00000000,1.02332675}, {0.13478556} }, //<-0.510,0.860,0.000>
			{ {0.28209481,-0.42025253,0.00000000,-0.24923933}, {0.88622695,0.00000000,0.00000000,-1.02332675}, {0.13478556} }, //<0.510,0.860,0.000>
			{ {0.28209481,-0.48860249,0.00000000,0.00000000}, {0.88622695,-1.02332675,0.00000000,0.00000000}, {0.12753712} }, //<0.000,1.000,0.000>
			{ {0.28209481,-0.42025253,-0.24923933,0.00000000}, {0.88622695,0.00000000,-1.02332675,0.00000000}, {0.13478556} }, //<0.000,0.860,-0.510>
			{ {0.28209481,-0.42025253,0.24923933,0.00000000}, {0.88622695,0.00000000,1.02332675,0.00000000}, {0.13478556} }, //<0.000,0.860,0.510>

			{ {0.28209481,0.00000000,-0.42025253,0.24923933}, {0.88622695,0.00000000,0.00000000,1.02332675}, {0.13478556} }, //<-0.510,-0.000,-0.860>
			{ {0.28209481,0.00000000,-0.42025253,-0.24923933}, {0.88622695,0.00000000,0.00000000,-1.02332675}, {0.13478556} }, //<0.510,-0.000,-0.860>
			{ {0.28209481,0.24923933,-0.42025253,0.00000000}, {0.88622695,1.02332675,0.00000000,0.00000000}, {0.13478556} }, //<0.000,-0.510,-0.860>
			{ {0.28209481,-0.24923933,-0.42025253,0.00000000}, {0.88622695,-1.02332675,0.00000000,0.00000000}, {0.13478556} }, //<0.000,0.510,-0.860>
			{ {0.28209481,0.00000000,-0.48860249,0.00000000}, {0.88622695,0.00000000,-1.02332675,0.00000000}, {0.12753712} }, //<0.000,0.000,-1.000>

			{ {0.28209481,0.00000000,0.42025253,0.24923933}, {0.88622695,0.00000000,0.00000000,1.02332675}, {0.13478556} }, //<-0.510,0.000,0.860>
			{ {0.28209481,0.00000000,0.42025253,-0.24923933}, {0.88622695,0.00000000,0.00000000,-1.02332675}, {0.13478556} }, //<0.510,0.000,0.860>
			{ {0.28209481,0.24923933,0.42025253,0.00000000}, {0.88622695,1.02332675,0.00000000,0.00000000}, {0.13478556} }, //<0.000,-0.510,0.860>
			{ {0.28209481,-0.24923933,0.42025253,0.00000000}, {0.88622695,-1.02332675,0.00000000,0.00000000}, {0.13478556} }, //<0.000,0.510,0.860>
			{ {0.28209481,0.00000000,0.48860249,0.00000000}, {0.88622695,0.00000000,1.02332675,0.00000000}, {0.12753712} }, //<0.000,0.000,1.000>
		};
		
		?>
	}
	
    //-----------------------------------------------------------------------------
	// Name: lightInjectionPS()
	// Desc : Vertex shader used during reflective shadow map injetion into the volume texture.
	// Note : This approach uses vertex texture fetching which is best for nVidia cards.
	//        ATI SM3.0 cards do not support VTF, but instead use RTVB which requires
	//        a slightly different approach.  
	// Note : When cascading, if this VPL was contained in the previous finer grid, we throw it away (during lighting injection only).
	//        This allows objects that span grids to be rendered into both, but only the VPLs that exist inside a given grid contribute there. The
	//        propagation scheme will ensure that we cross over where needed. This even works when not propagating between grids (i.e., no occlusion) but
	//        rather using additive blending since it still splits the light flux up across the grids. This makes object management during the collection
	//        and rendering stage much simpler.
	//-----------------------------------------------------------------------------
	//bool lightInjectionVS( bool injectGeometry, bool testPreviousGrid )
	bool lightInjectionVS( )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Define shader inputs.
        <?in
			float3  position   : POSITION;
        ?>

		// Define shader outputs.
		<?out
			float4 clipPosition : SV_POSITION; 
			float2 texCoords    : TEXCOORD0;
			float3 normal       : TEXCOORD1;
			float  fluxScale    : TEXCOORD2;
		?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbRSM;
            _cbGrid;
            _cbCamera;
        ?>
        
		// Sampler usage.
		<?global
			Sampler2D sDepthNormalVTF : register(s0);
		?> 
        
		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		bool injectGeometry   = false;
		bool testPreviousGrid = false;
		
		<?
		// Compute grid information
		float3 gridSize = _gridMax - _gridMin;
		float3 cellSize = gridSize / _gridDimensions;
		float3 halfCell = cellSize * 0.5f; 

		// Compute texture coords for RSM g-buffer sampling
		texCoords = position.xy;

		// Sample the normal and depth
		float4 vplData  = sample2DLevel( sDepthNormalVTFTex, sDepthNormalVTF, texCoords * _rsmTextureScaleBias.xy, 0 );  
		float  vplDepth = vplData.w;
		normal = normalize( vplData.xyz ); 

		// Compute initial sample position in (rsm) view space
		float4 vplPositionVS;
		vplPositionVS.xy = (texCoords * _rsmScreenToViewScaleBias.xy + _rsmScreenToViewScaleBias.zw) * vplDepth;
		vplPositionVS.z  = vplDepth;
		vplPositionVS.w  = 1;

		// Compute final sample position in world space
		float3 vplPosition = mul( vplPositionVS, _rsmInvViewMatrix ).xyz;
		?>
		
		// Shift position to minimize self-lighting/occlusion
		if ( injectGeometry )
			<?float3 shiftedPosition = vplPosition + halfCell;?>
		else
			<?float3 shiftedPosition = vplPosition + normal * halfCell;?>
		
		<?
        // Compute clip space position
		clipPosition = worldToGridCS( shiftedPosition, _gridMin, _gridDimensions, gridSize );

	    // Is the point inside the grid?
	    float pointInGrid = 1.0f;
		pointInGrid = isPointInAABB( shiftedPosition, _gridMin, _gridMax );
        ?>

		// Test against prior grid for containment.
		if( testPreviousGrid ) 			
			<?pointInGrid = 1.0f - isPointInAABB( vplPosition, _gridMinPrev, _gridMaxPrev );?>

		<?
		// Clip any points that are considered invalid
		if( (pointInGrid < 1e-6f) || ( dot( normal, normal ) <= 0.0f ) ) 
			 clipPosition.xy = -20.0f; 

		// Account for surfel area relative to the grid into which it is being injected
		float projectedArea = (vplDepth * vplDepth) * (4.0f / (_rsmTextureSize.x * _rsmTextureSize.y));
		fluxScale = saturate( projectedArea / (cellSize * cellSize) );
		?>
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: lightInjectionPS()
	// Desc : Pixel shader used during rsm injection into the light volume texture(s). 
	//-----------------------------------------------------------------------------
	bool lightInjectionPS( )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
            float3  normal         : TEXCOORD1;
            float   fluxScale      : TEXCOORD2;
        ?>

		// Define shader outputs.
		<?out
			float4  data0 : SV_TARGET0;
			float4  data1 : SV_TARGET1;
			float4  data2 : SV_TARGET2;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbRSM;
            _cbGrid;
            _cbCamera;
        ?>
		
		// Sampler buffer usage.
		<?samplerrefs
			_sbLight;
		?> 

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		// Sample the reflectance data
		float4 reflectance = sample2D( sShadowColorTex, sShadowColor, texCoords );
		
		// Compute final radiance
		float3 radiance = _rsmColor.rgb * reflectance.rgb * reflectance.a * fluxScale * (1.0f / 3.14159f);
		
		// Renormalize the normal (vpl light direction)
		normal = normalize( normal );
		
		// Project radiance into SH 
		float4 shRay = float4( 0.282094792f, -0.488602512f, 0.488602512f, -0.488602512f ) * float4( 1, normal.yzx );
		data0 = shRay * radiance.r;
		data1 = shRay * radiance.g;
		data2 = shRay * radiance.b;
		?>

		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: geometryInjectionPS()
	// Desc : Pixel shader used during rsm injection into the geometry volume texture(s). 
	//-----------------------------------------------------------------------------
	bool geometryInjectionPS( )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
            float3  normal         : TEXCOORD1;
            float   fluxScale      : TEXCOORD2;
        ?>

		// Define shader outputs.
		<?out
			float4  data0 : SV_TARGET0;
		?>
		
		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		// Renormalize the normal (vpl light direction)
		normal = normalize( normal );
		
		// Project occlusion into SH 
		data0 = float4( 0.282094792f, -0.488602512f, 0.488602512f, -0.488602512f ) * float4( 1, normal.yzx ) * fluxScale;
		?>

		// Valid shader
		return true;
	}
	
	//-----------------------------------------------------------------------------
	// Name : propagateIndirectLighting()
	// Desc : Indirect lighting propagation through grid
	//-----------------------------------------------------------------------------
	bool propagateIndirectLighting( )
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
			float4  data0 : SV_TARGET0;
			float4  data1 : SV_TARGET1;
			float4  data2 : SV_TARGET2;
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbRSM;
            _cbGrid;
            _cbCamera;
        ?>
		
		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		
		// Initialize system
		initLPVData();
	
		<?
		float3 recipDimensions = 1.0f / _gridDimensions;
		
		// Initialize accumulators
		float4 finalSHR = 0;
		float4 finalSHG = 0;
		float4 finalSHB = 0;

		// Compute the 3D coordinate for this cell
		float3 cellCoord;
		cellCoord.z   = floor( screenPosition.x / _gridDimensions.x );
		cellCoord.y   = screenPosition.y;
		cellCoord.x   = screenPosition.x - (cellCoord.z * _gridDimensions.x);
		cellCoord.xy += 0.5f;
		cellCoord     = clamp( cellCoord, 0, _gridDimensions ); 
		cellCoord    *= recipDimensions;

		// Iterate our neighbors
		for ( int i = 0; i < 6; i++ )
		{
			float4 shR, shG, shB;

			// Compute the coordinate of the neighbor to sample
			float3 sampleCoord = cellCoord + neighborOffsets[ i ] * recipDimensions;
			?>
	  		
			// Sample the neighbor (must use border mode to zero out samples outside the grid)
			sampleVolumeSH2( "sampleCoord", "_gridDimensions", "sSHRTex", "sSHGTex", "sSHBTex", "sSHR", "sSHG", "sSHB", "shR", "shG", "shB", true ); 

			<?
			// Evaluate flux along the direction to each inner face
			for ( int j = 0; j < 5; j++ )
			{
				// Get the neighor data for the face
				neighborSHData neighbor = neighborData[ i * 5 + j ];

				// Evaluate flux along the direction to the face
				float3 flux;
				flux.r = max( 0.0f, dot( shR, neighbor.shDir ) * neighbor.fluxAdjust );
				flux.g = max( 0.0f, dot( shG, neighbor.shDir ) * neighbor.fluxAdjust );
				flux.b = max( 0.0f, dot( shB, neighbor.shDir ) * neighbor.fluxAdjust );

				// Reproject flux into SH and accumulate
				finalSHR += flux.r * neighbor.cosineLobe;
				finalSHG += flux.g * neighbor.cosineLobe;
				finalSHB += flux.b * neighbor.cosineLobe;
				
			} // Next face

		} // Next neighbor
		
		// Set outputs
		data0 = finalSHR;
		data1 = finalSHG;
		data2 = finalSHB;
		?>
		
		// Return results
		return true; 
	}
	

} // End Class : LightSource