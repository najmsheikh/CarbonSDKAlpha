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
// Name : ParticleShader.sh                                                  //
//                                                                           //
// Desc : Default surface shader for particles.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "sys://Shaders/Utilities.shh"

///////////////////////////////////////////////////////////////////////////////
// Global Enumerations
///////////////////////////////////////////////////////////////////////////////
enum ParticleBlendMethod
{
    Additive        = 0,
    Linear          = 1,
    Screen          = 2,
    SoftAdditive    = 3,
    SoftLinear      = 4
};

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENTSCRIPT
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return ParticleShader( owner, driver, resources );
}
#endif // !_PARENTSCRIPT

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : ParticleShader (Class)
// Desc : Default surface shader for particles.
//-----------------------------------------------------------------------------
class ParticleShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Public Member Variables
	///////////////////////////////////////////////////////////////////////////
    int blendMethod;    // The method used for blending the particles with the current target.

    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;                 // The parent application object.
    private RenderDriver@           mDriver;                // Render driver to which this shader is linked.
    private ResourceManager@        mResources;             // Resource manager that owns this shader.

    // Depth stencil states
    private DepthStencilStateHandle mDepthStateWriteDisabled;

    // Blend states
    private array<BlendStateHandle> mBlendStates;
    
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D     sBillboardInput   : register(s0);
        Sampler2D     sDepth            : register(s10);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : ParticleShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
    ParticleShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner = owner;
        @mDriver = driver;
        @mResources = resources;

        // Default variables
        blendMethod = ParticleBlendMethod::Additive;

        ///////////////////////////////////////////////////////////////
        // Depth Stencil States
        ///////////////////////////////////////////////////////////////
        // Z-Writes disabled.
        DepthStencilStateDesc dsStates;
        dsStates.depthEnable        = true;
        dsStates.depthWriteEnable   = false;
        resources.createDepthStencilState( mDepthStateWriteDisabled, dsStates, 0, DebugSource() );

        ///////////////////////////////////////////////////////////////
        // Blend States
        ///////////////////////////////////////////////////////////////
        mBlendStates.resize(5);

        // Additive blending (One -> One).
        BlendStateDesc blStates;
        blStates.renderTarget0.blendEnable      = true;
        blStates.renderTarget0.sourceBlend      = BlendMode::One;
        blStates.renderTarget0.destinationBlend = BlendMode::One;
        resources.createBlendState( mBlendStates[ParticleBlendMethod::Additive], blStates, 0, DebugSource() );

        // Alpha blending (SrcAlpha -> InvSrcAlpha).
        blStates.renderTarget0.blendEnable      = true;
        blStates.renderTarget0.sourceBlend      = BlendMode::SrcAlpha;
        blStates.renderTarget0.destinationBlend = BlendMode::InvSrcAlpha;
        resources.createBlendState( mBlendStates[ParticleBlendMethod::Linear], blStates, 0, DebugSource() );

        // Screen blending (InvDestColor -> One).
        blStates.renderTarget0.blendEnable      = true;
        blStates.renderTarget0.sourceBlend      = BlendMode::InvDestColor;
        blStates.renderTarget0.destinationBlend = BlendMode::One;
        resources.createBlendState( mBlendStates[ParticleBlendMethod::Screen], blStates, 0, DebugSource() );

        // Soft additive
        mBlendStates[ParticleBlendMethod::SoftAdditive] = mBlendStates[ParticleBlendMethod::Additive];

        // Soft linear
        mBlendStates[ParticleBlendMethod::SoftLinear] = mBlendStates[ParticleBlendMethod::Linear];
    }

    ///////////////////////////////////////////////////////////////////////////
    // Techniques
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : billboard3D (Technique)
    // Desc : Standard 3D billboard, no alignment. Always points toward the
    //        camera.
    //-------------------------------------------------------------------------
    TechniqueResult billboard3D( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthStateWriteDisabled );
        mDriver.setBlendState( mBlendStates[blendMethod] );
        mDriver.setRasterizerState( null );

        // Depth type control only required if using soft blending.
        int depthType = 0;
        if ( blendMethod == ParticleBlendMethod::SoftAdditive || blendMethod == ParticleBlendMethod::SoftLinear )
            depthType = System.depthType;

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformBillboard" ) ||
             !mOwner.selectPixelShader( "drawBillboard", blendMethod, System.hdrLighting, depthType ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : billboardYLocked3D (Technique)
    // Desc : 3D billboard, aligned to the world Y axis. Always points toward 
    //        the camera.
    //-------------------------------------------------------------------------
    TechniqueResult billboardYLocked3D( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthStateWriteDisabled );
        mDriver.setBlendState( mBlendStates[blendMethod] );
        mDriver.setRasterizerState( null );

        // Depth type control only required if using soft blending.
        int depthType = 0;
        if ( blendMethod == ParticleBlendMethod::SoftAdditive || blendMethod == ParticleBlendMethod::SoftLinear )
            depthType = System.depthType;

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformBillboardYLocked" ) ||
             !mOwner.selectPixelShader( "drawBillboard", blendMethod, System.hdrLighting, depthType ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : billboardYLocked3D (Technique)
    // Desc : 3D billboard, aligned to an arbitrary axis supplied as a vertex
    //        component. Always points toward  the camera.
    //-------------------------------------------------------------------------
    TechniqueResult billboardAxisLocked3D( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthStateWriteDisabled );
        mDriver.setBlendState( mBlendStates[blendMethod] );
        mDriver.setRasterizerState( null );

        // Depth type control only required if using soft blending.
        int depthType = 0;
        if ( blendMethod == ParticleBlendMethod::SoftAdditive || blendMethod == ParticleBlendMethod::SoftLinear )
            depthType = System.depthType;

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformBillboardAxisLocked" ) ||
             !mOwner.selectPixelShader( "drawBillboard", blendMethod, System.hdrLighting, depthType ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Vertex Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : transformBillboardYLocked() (Vertex Shader)
    // Desc : Billboard vertex shader which constructs the four points of the
    //        billboard, based on the various input properties such as angle,
    //        scale and colour which is then orientated toward the camera
    //        rotating about the world Y axis.
    //-------------------------------------------------------------------------
    bool transformBillboardYLocked( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float3 sourcePosition   : POSITION;
            float4 sourceColor      : COLOR;
            float2 sourceTexCoords  : TEXCOORD0;
            float2 sourceOffsetUV   : TEXCOORD1;	// Offset for vertex positions (billboard expansion)
            float4 sourceProperties : TEXCOORD2;	// angle, scaleX, scaleY, hdrScale
            float3 sourceDirection  : TEXCOORD3;
		?>

        // Define shader outputs.
        <?out
            float4 clipPosition     : SV_POSITION;
            float4 color            : COLOR;
            float3 texCoords        : TEXCOORD0;
            float  hdrScale         : TEXCOORD1;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbWorld;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            // Transform source position into world space.
            float3 worldPosition = mul( float4(sourcePosition,1), _worldMatrix );

            // Scale offset.
            float2 offsetUV = sourceOffsetUV * sourceProperties.yz;

            // Compute rotated offset values
            float3 offset;
            offset.x = (offsetUV.x * cos( sourceProperties.x ) + offsetUV.y * sin( sourceProperties.x ));
            offset.y = (offsetUV.y * cos( sourceProperties.x ) - offsetUV.x * sin( sourceProperties.x ));
            offset.z = 0.0f;

            // Calculate axis vectors
            #if ( 1 )
                // Always point toward camera position
                float3 look     = float3( worldPosition.x - _cameraPosition.x, 0.0f, worldPosition.z - _cameraPosition.z );
                float  distance = length( look );
                look           /= distance + 1e-5f; // Div by 0 protection
            #else
                // Always point toward near plane
                float3 look     = normalize( float3( _cameraDirection.x, 0.0f, _cameraDirection.z ) );
            #endif

            float3 up    = float3( 0.0f, 1.0f, 0.0f );
            float3 right = cross( up, look );

            // Multiply by this new matrix
            worldPosition.x += dot( offset, float3( right.x, up.x, look.x) );
            worldPosition.y += dot( offset, float3( right.y, up.y, look.y) );
            worldPosition.z += dot( offset, float3( right.z, up.z, look.z) );

            // Compute the clip space position
            clipPosition = mul( float4( worldPosition, 1 ), _viewProjectionMatrix );

            // Copy over color
            color = sourceColor;
            hdrScale = sourceProperties.w;

            // Copy over our texture coordinates
            texCoords.xy = sourceTexCoords;
            texCoords.z = clipPosition.w;
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : transformBillboardAxisLocked() (Vertex Shader)
    // Desc : Billboard vertex shader which constructs the four points of the 
    //        billboard, based on the various input properties such as angle,
    //        scale and colour which is then orientated toward the camera 
    //        rotatingabout a custom axis.
    //-------------------------------------------------------------------------
    bool transformBillboardAxisLocked( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float3 sourcePosition     : POSITION;
            float4 sourceColor        : COLOR;
            float2 sourceTexCoords    : TEXCOORD0;
            float2 sourceOffsetUV     : TEXCOORD1;	// Offset for vertex positions (billboard expansion)
            float4 sourceProperties   : TEXCOORD2;	// Angle, ScaleX, ScaleY, HDRScale
            float3 sourceDirection    : TEXCOORD3;
		?>

        // Define shader outputs.
        <?out
            float4 clipPosition    : SV_POSITION;
            float4 color           : COLOR;
            float3 texCoords       : TEXCOORD0;
            float  hdrScale        : TEXCOORD1;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbWorld;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            // Transform source position into world space.
            float3 worldPosition = mul( float4(sourcePosition,1), _worldMatrix );

            // Scale offset.
            float2 offsetUV = sourceOffsetUV * sourceProperties.yz;

            // Compute rotated offset values
            float3 offset;
            offset.x = (offsetUV.x * cos( sourceProperties.x ) + offsetUV.y * sin( sourceProperties.x ));
            offset.y = (offsetUV.y * cos( sourceProperties.x ) - offsetUV.x * sin( sourceProperties.x ));
            offset.z = 0.0f;

            // Calculate axis vectors
            #if ( 1 )
                // Always point toward camera position
                float3 look  = worldPosition - _cameraPosition;
                float  distance  = length( look );
                look        /= distance + 1e-5f; // Div by 0 protection
            #else
                // Always point toward near plane
                float3 look  = _cameraDirection;
            #endif

            float3 up    = sourceDirection;
            float3 right = normalize(cross( up, look ));
            look = normalize( cross( right, up ) );

            // Multiply by this new matrix
            worldPosition.x += dot( offset, float3( right.x, up.x, look.x) );
            worldPosition.y += dot( offset, float3( right.y, up.y, look.y) );
            worldPosition.z += dot( offset, float3( right.z, up.z, look.z) );

            // Compute the clip space position
            clipPosition = mul( float4( worldPosition, 1 ), _viewProjectionMatrix );

            // Copy over color
            color = sourceColor;
            hdrScale = sourceProperties.w;

            // Copy over our texture coordinates
            texCoords.xy = sourceTexCoords;
            texCoords.z = clipPosition.w;
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : transformBillboard() (Vertex Shader)
    // Desc : Standard billboard vertex shader which constructs the four points
    //        of the billboard, based on the various input properties such as
    //        angle, scale and colour which is then fully orientated toward the
    //        camera.
    //-----------------------------------------------------------------------------
    bool transformBillboard( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float3 sourcePosition   : POSITION;
            float4 sourceColor      : COLOR;
            float2 sourceTexCoords  : TEXCOORD0;
            float2 sourceOffsetUV   : TEXCOORD1;	// Offset for vertex positions (billboard expansion)
            float4 sourceProperties : TEXCOORD2;	// angle, scaleX, scaleY, HDRScale
            float3 sourceDirection  : TEXCOORD3;
		?>

        // Define shader outputs.
        <?out
            float4 clipPosition    : SV_POSITION;
            float4 color           : COLOR;
            float3 texCoords       : TEXCOORD0;
            float  hdrScale        : TEXCOORD1;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbWorld;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            // Transform source position into world space.
            float3 worldPosition = mul( float4(sourcePosition,1), _worldMatrix );

            // Scale offset.
            float2 offsetUV = sourceOffsetUV * sourceProperties.yz;

            // Compute rotated offset values
            float3 offset;
            offset.x = (offsetUV.x * cos( sourceProperties.x ) + offsetUV.y * sin( sourceProperties.x ));
            offset.y = (offsetUV.y * cos( sourceProperties.x ) - offsetUV.x * sin( sourceProperties.x ));
            offset.z = 0.0f;

            // Calculate axis vectors
            #if ( 1 )
                
                // Multiply by inverse view matrix (screen-aligned)
                worldPosition.x += dot( offset, float3( _viewMatrix._11, _viewMatrix._12, _viewMatrix._13 ) );
                worldPosition.y += dot( offset, float3( _viewMatrix._21, _viewMatrix._22, _viewMatrix._23 ) );
                worldPosition.z += dot( offset, float3( _viewMatrix._31, _viewMatrix._32, _viewMatrix._33 ) );

            #else

                float  distance;
                float3 up, look, right;
                float3 worldUp = {0,1,0};

                // Always point toward camera position (camera aligned).
                look     = worldPosition - _cameraPosition;
                distance = length( look );
                look    /= distance + 1e-5f; // Div by 0 protection

                // Potentially select the world +Y or world +X as the
                // selected billboard up vector as necessary.
                distance = dot( worldUp, look );
                if ( abs( distance ) >= 0.99999f )
                {
                    worldUp  = float3(1,0,0);
                    distance = dot( worldUp, look );

                } // End if look == up

                up    = normalize( worldUp - (look * distance) );
                right = cross( up, look );

                // Multiply by inverse look-at matrix
                worldPosition.x += dot( offset, float3(right.x,up.x,look.x) );
                worldPosition.y += dot( offset, float3(right.y,up.y,look.y) );
                worldPosition.z += dot( offset, float3(right.z,up.z,look.z) );

            #endif

            // Compute the clip space position
            clipPosition = mul( float4( worldPosition, 1 ), _viewProjectionMatrix );

            // Copy over color
            color = sourceColor;
            hdrScale = sourceProperties.w;

            // Copy over our texture coordinates
            texCoords.xy = sourceTexCoords;
            texCoords.z = clipPosition.w;
        ?>
        
        // Valid shader
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : drawBillboard() (Pixel Shader)
    // Desc : Billboard pixel shader with optional softness support.
    //-------------------------------------------------------------------------
    bool drawBillboard( int blendMethod, bool hdrLighting, int depthType )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 screenPosition   : SV_POSITION;
            float4 color            : COLOR;
            float3 texCoords        : TEXCOORD0; // Camera space Z is packed into Z
            float  hdrScale         : TEXCOORD1;
		?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
        ?>

        // Constant buffer usage.
        bool softBlending = (blendMethod == ParticleBlendMethod::SoftAdditive || blendMethod == ParticleBlendMethod::SoftLinear);
        if ( softBlending )
        {
            <?cbufferrefs
                _cbCamera;
            ?>
        
        } // End if softBlending

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        
        // Modulate the texture and interpolated color
        <?
        data0 = sample2D( sBillboardInputTex, sBillboardInput, texCoords.xy ) * color;
        ?>

        // Adjust alpha based on provided depth information when soft particle blending is enabled.
        if ( softBlending )
        {
            <?
            //float2 depthBufferData;
		    float2 screenTexCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;
            float sceneZ = decompressF32( tex2D( sDepth, screenTexCoords ).rgb ) * _cameraFar; 
            ?>

            // ToDo: Support the different depth types properly -- the above currently assumes LinearZ_Packed
            // Sample current depth value from scene
            /*bool isCompressedDepth = isCompressedDepthType( depthType );
            int pureType = sampleDepthBuffer( "depthBufferData", "sDepthTex", "sDepth", "screenTexCoords", isCompressedDepth, depthType );*/
             
            // Adjust the particle transparency based on depth difference
            <?
            const float SoftFadeDistanceMin = 0.3f;
            const float SoftFadeDistanceMax = 0.8f;
            data0.a *= saturate( ( (sceneZ - texCoords.z) - SoftFadeDistanceMin) / (SoftFadeDistanceMax - SoftFadeDistanceMin) );

            // Fade as we approach the near clip plane
            //float clipFadeScale = 1.0f / (NearClipFadeEnd - NearClipFadeStart);
            //float clipFadeBias  = -NearClipFadeStart * ClipFadeScale;
            //data0.a *= saturate( In.ClipCoords.w * clipFadeScale + clipFadeBias );
            ?>

        } // End if soft particles

        // Pre-multiply color by alpha for screen and additive blending.
        if ( blendMethod == ParticleBlendMethod::Screen || 
             blendMethod == ParticleBlendMethod::Additive ||
             blendMethod == ParticleBlendMethod::SoftAdditive )
        {
            <?
            data0.rgb *= data0.a;
            data0.a = 1.0f;
            ?>
        
        } // End if Screen || Additive

        <?
        // ToDo: Conditional if manual SRGB is enabled
        data0.rgb  = SRGBToLinear( data0.rgb );
        ?>

        // Apply HDR scale if necessary.
        if ( hdrLighting )
        {
            <?
            data0.rgb *= hdrScale;
            ?>
        
        } // End if HDR
        
        // Valid shader
        return true;
    }
 
} // End Class : ParticleShader

/*
float4 psBillboardSoftLit( PS_BBINPUT_SOFT In ) : COLOR0
{
 // Combine texture and vertex color (opacities as well)
 float4 BillboardColor = tex2D( BillboardSampler, In.TexCoords ) * In.Color;
 BillboardColor.rgb *= BillboardColor.rgb; // sRGB fast remap

 // Compute screen coords
 float2 ScreenCoords = (In.ClipCoords.xy / In.ClipCoords.w) * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );
 
 // Sample current depth value from scene
 float SceneZ = DecompressF32( tex2D( OpaqueDepthSampler, ScreenCoords ).rgb ) * CameraFarClip;
 
    // Adjust the particle transparency based on depth difference
    BillboardColor.a *= saturate( ( abs(In.ClipCoords.w - SceneZ) - SoftFadeDistanceMin) / (SoftFadeDistanceMax - SoftFadeDistanceMin) );

 // Fade as we approach the near clip plane
 float ClipFadeScale = 1.0f / (NearClipFadeEnd - NearClipFadeStart);
 float ClipFadeBias  = -NearClipFadeStart * ClipFadeScale;
 BillboardColor.a   *= saturate( In.ClipCoords.w * ClipFadeScale + ClipFadeBias );

 // Compute the position and normal using a ray-sphere intersection approach
 float3 Position, N = 0;
 Impostor( In.WorldPos, In.Normal.xy, In.Offset * 0.5, Position, N );
 Position = Position + N * ((BillboardColor.r * 2.0f - 1.0f) * 25.0f);

 // Compute the eye ray
 float3 E = normalize( -Position );

 // Sample the dual depth map
 float4 DDM = tex2D( DualDepthSampler, ScreenCoords );

 // Compute distance across the system
 float AxisMin  = DDM.x;
 float AxisMax  = 1.0f - DDM.y;
 float Distance = ma(angry) 1.0f, (AxisMax - AxisMin) * CameraFarClip );

 // Compute average density along the ray (Avg = Density / Thickness)
 // Note: For the moment, we are simply using exponential density and ignoring distance
 float Density = DDM.a;// / Distance;
 float Transmission = exp( -Density );

 /////////////////////////////////////////////////////////////////////////
 
 // Compute lighting
 float3 L;
 float2 LightData = TestPointLighting( Position, N, LP, LR, LightWrapping, L );

 // Compute attenuation
 float Ks = 0.20f;
 float Ka = LightData.y * LightData.y;

 // Compute E.L
 float EdotL = saturate( dot( E, -L ) );

 // Compute diffuse term
 float Id = lerp( LightData.x, LightData.x * Transmission, saturate( 2.0f * EdotL ) );

 // Compute scattering term
 float Is = EdotL * Ks * Transmission;

 // Combine reflection and scattering and then attenuate
 float3 FinalLighting = ((Is + Id) * Ka) * LightColor;

 // Apply albedo
 BillboardColor.rgb *= FinalLighting;

 // Return result
 return BillboardColor;
};*/