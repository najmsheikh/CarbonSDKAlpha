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
// Name : ParticleScreenBlend.sh                                             //
//                                                                           //
// Desc : Default surface shader for particles that require screen blending. //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENTSCRIPT
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return ParticleScreenBlendShader( owner, driver, resources );
}
#endif // !_PARENTSCRIPT

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : ParticleScreenBlendShader (Class)
// Desc : Default surface shader for particles that require screen blending. //
//-----------------------------------------------------------------------------
class ParticleScreenBlendShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;                 // The parent application object.
    private RenderDriver@           mDriver;                // Render driver to which this shader is linked.
    private ResourceManager@        mResources;             // Resource manager that owns this shader.

    // Depth stencil states
    private DepthStencilStateHandle mDepthStateWriteDisabled;

    // Blend states
    private BlendStateHandle        mBlendStateScreen;          // Screen blending (InvDestColor -> One).
    
    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D     sBillboardInput   : register(s0);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : ParticleScreenBlendShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
    ParticleScreenBlendShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner = owner;
        @mDriver = driver;
        @mResources = resources;

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
        // Screen blending (InvDestColor -> One).
        BlendStateDesc blStates;
        blStates.renderTarget0.blendEnable      = true;
        blStates.renderTarget0.sourceBlend      = BlendMode::InvDestColor;
        blStates.renderTarget0.destinationBlend = BlendMode::One;
        resources.createBlendState( mBlendStateScreen, blStates, 0, DebugSource() );
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
        mDriver.setBlendState( mBlendStateScreen );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformBillboard" ) ||
             !mOwner.selectPixelShader( "drawBillboard" ) )
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
        mDriver.setBlendState( mBlendStateScreen );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformBillboardYLocked" ) ||
             !mOwner.selectPixelShader( "drawBillboard" ) )
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
        mDriver.setBlendState( mBlendStateScreen );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformBillboardAxisLocked" ) ||
             !mOwner.selectPixelShader( "drawBillboard" ) )
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
            float3 sourceProperties : TEXCOORD2;	// angle, scaleX, scaleY
            float3 sourceDirection  : TEXCOORD3;
		?>

        // Define shader outputs.
        <?out
            float4 clipPosition     : SV_POSITION;
            float4 color            : COLOR;
            float2 texCoords        : TEXCOORD0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            float3 worldPosition;

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
                float3 look     = float3( sourcePosition.x - _cameraPosition.x, 0.0f, sourcePosition.z - _cameraPosition.z );
                float  distance = length( look );
                look           /= distance + 1e-5f; // Div by 0 protection
            #else
                // Always point toward near plane
                float3 look     = normalize( float3( _cameraDirection.x, 0.0f, _cameraDirection.z ) );
            #endif

            float3 up    = float3( 0.0f, 1.0f, 0.0f );
            float3 right = cross( up, look );

            // Multiply by this new matrix
            worldPosition.x = dot( offset, float3( right.x, up.x, look.x) ) + sourcePosition.x;
            worldPosition.y = dot( offset, float3( right.y, up.y, look.y) ) + sourcePosition.y;
            worldPosition.z = dot( offset, float3( right.z, up.z, look.z) ) + sourcePosition.z;

            // Compute the clip space position
            clipPosition = mul( float4( worldPosition, 1 ), _viewProjectionMatrix );

            // Copy over color
            color = sourceColor;

            // Copy over our texture coordinates
            texCoords = sourceTexCoords;
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
            float3 sourcePosition          : POSITION;
            float4 sourceColor        : COLOR;
            float2 sourceTexCoords    : TEXCOORD0;
            float2 sourceOffsetUV     : TEXCOORD1;	// Offset for vertex positions (billboard expansion)
            float3 sourceProperties   : TEXCOORD2;	// Angle, ScaleX, ScaleY
            float3 sourceDirection    : TEXCOORD3;
		?>

        // Define shader outputs.
        <?out
            float4 clipPosition    : SV_POSITION;
            float4 color           : COLOR;
            float2 texCoords       : TEXCOORD0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            float3 worldPosition;

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
                float3 look  = sourcePosition - _cameraPosition;
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
            worldPosition.x = dot( Offset, float3( right.x, up.x, look.x) ) + sourcePosition.x;
            worldPosition.y = dot( Offset, float3( right.y, up.y, look.y) ) + sourcePosition.y;
            worldPosition.z = dot( Offset, float3( right.z, up.z, look.z) ) + sourcePosition.z;

            // Compute the clip space position
            clipPosition = mul( float4( worldPosition, 1 ), _viewProjectionMatrix );

            // Copy over color
            color = sourceColor;

            // Copy over our texture coordinates
            texCoords = sourceTexCoords;
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
            float3 sourceProperties : TEXCOORD2;	// angle, scaleX, scaleY
            float3 sourceDirection  : TEXCOORD3;
		?>

        // Define shader outputs.
        <?out
            float4 clipPosition    : SV_POSITION;
            float4 color           : COLOR;
            float2 texCoords       : TEXCOORD0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            float3 worldPosition;

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
                worldPosition.x = dot( offset, float3( _viewMatrix._11, _viewMatrix._12, _viewMatrix._13 ) ) + sourcePosition.x;
                worldPosition.y = dot( offset, float3( _viewMatrix._21, _viewMatrix._22, _viewMatrix._23 ) ) + sourcePosition.y;
                worldPosition.z = dot( offset, float3( _viewMatrix._31, _viewMatrix._32, _viewMatrix._33 ) ) + sourcePosition.z;

            #else

                float  distance;
                float3 up, look, right;
                float3 worldUp = {0,1,0};

                // Always point toward camera position (camera aligned).
                look     = sourcePosition - _cameraPosition;
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
                worldPosition.x = dot( offset, float3(right.x,up.x,look.x) ) + sourcePosition.x;
                worldPosition.y = dot( offset, float3(right.y,up.y,look.y) ) + sourcePosition.y;
                worldPosition.z = dot( offset, float3(right.z,up.z,look.z) ) + sourcePosition.z;

            #endif

            // Compute the clip space position
            clipPosition = mul( float4( worldPosition, 1 ), _viewProjectionMatrix );

            // Copy over color
            color = sourceColor;

            // Copy over our texture coordinates
            texCoords = sourceTexCoords;
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
    bool drawBillboard( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 screenPosition   : SV_POSITION;
            float4 color            : COLOR;
            float2 texCoords        : TEXCOORD0;
		?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            // Modulate the texture and interpolated color
            data0 = sample2D( sBillboardInputTex, sBillboardInput, texCoords ) * color;

            // Pre-multiply color by alpha.
            data0.rgb = data0.rgb * data0.a;
            data0.a   = 1.0f;
        ?>
        
        // Valid shader
        return true;
    }
 
} // End Class : ParticleScreenBlendShader