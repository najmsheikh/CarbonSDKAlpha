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
// Name : Billboards.sh                                                      //
//                                                                           //
// Desc : Rendering extension module. Provides additional support required   //
//        by internal sqBillboardBuffer class functionality.                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "Utilities.shh" // computeMeshScreenTextureCoords

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENTSCRIPT
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return BillboardShader( owner, driver, resources );
}
#endif // !_PARENTSCRIPT

///////////////////////////////////////////////////////////////////////////////
// Module Local Defines
///////////////////////////////////////////////////////////////////////////////
const int BILLBOARD_STANDARD  = 0;
const int BILLBOARD_SOFT      = 1;

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : BillboardShader (Class)
// Desc : Rendering extension module. Provides additional support required
//        by internal sqBillboardBuffer class functionality.
//-----------------------------------------------------------------------------
class BillboardShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;                 // The parent application object.
    private RenderDriver@           mDriver;                // Render driver to which this shader is linked.
    private ResourceManager@        mResources;             // Resource manager that owns this shader.

    // Depth stencil states
    private DepthStencilStateHandle mDepthStateDisabled;
    private DepthStencilStateHandle mDepthStateWriteDisabled;

    // Blend states
    private BlendStateHandle        mBlendStateSrcInvSrc;       // Standard blending (SrcAlpha -> InvSrcAlpha).
    private BlendStateHandle        mBlendStateMaskedAdditive;  // Masked additive blending (SrcAlpha -> One).
    private BlendStateHandle        mBlendStateScreen;          // Screen blending (InvDestColor -> One).

    // Sampler states
    private SamplerStateHandle      mSamplerStatePoint;     // Point filtered sampler states
    private SamplerStateHandle      mSamplerStateLinear;    // Linearly filtered sampler states
    
    // Rasterizer states
    private RasterizerStateHandle	mRastStateScissorTest;

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
	// Name : BillboardShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
    BillboardShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner = owner;
        @mDriver = driver;
        @mResources = resources;

        ///////////////////////////////////////////////////////////////
        // Depth Stencil States
        ///////////////////////////////////////////////////////////////
        DepthStencilStateDesc dsStates;

        // Z-Buffer disabled.
        dsStates.depthEnable        = false;
        dsStates.depthWriteEnable   = false;
        resources.createDepthStencilState( mDepthStateDisabled, dsStates, 0, DebugSource() );

        // Z-Writes disabled.
        dsStates.depthEnable        = true;
        dsStates.depthWriteEnable   = false;
        resources.createDepthStencilState( mDepthStateWriteDisabled, dsStates, 0, DebugSource() );

        ///////////////////////////////////////////////////////////////
        // Blend States
        ///////////////////////////////////////////////////////////////
        BlendStateDesc blStates;

        // Standard blending (SrcAlpha -> InvSrcAlpha).
        blStates.renderTarget0.blendEnable      = true;
        blStates.renderTarget0.sourceBlend      = BlendMode::SrcAlpha;
        blStates.renderTarget0.destinationBlend = BlendMode::InvSrcAlpha;
        resources.createBlendState( mBlendStateSrcInvSrc, blStates, 0, DebugSource() );

        // Masked additive blending (SrcAlpha -> One).
        blStates.renderTarget0.blendEnable      = true;
        blStates.renderTarget0.sourceBlend      = BlendMode::SrcAlpha;
        blStates.renderTarget0.destinationBlend = BlendMode::One;
        resources.createBlendState( mBlendStateMaskedAdditive, blStates, 0, DebugSource() );

        // Screen blending (InvDestColor -> One).
        blStates.renderTarget0.blendEnable      = true;
        blStates.renderTarget0.sourceBlend      = BlendMode::InvDestColor;
        blStates.renderTarget0.destinationBlend = BlendMode::One;
        resources.createBlendState( mBlendStateScreen, blStates, 0, DebugSource() );

        ///////////////////////////////////////////////////////////////
        // Sampler States
        ///////////////////////////////////////////////////////////////
        SamplerStateDesc sStates;

        // Point filtered.
        sStates.minificationFilter  = FilterMethod::Point;
        sStates.magnificationFilter = FilterMethod::Point;
        sStates.mipmapFilter        = FilterMethod::None;
        sStates.addressU            = AddressingMode::Clamp;
        sStates.addressV            = AddressingMode::Clamp;
        resources.createSamplerState( mSamplerStatePoint, sStates, 0, DebugSource() );

        // Linearly filtered.
        sStates.minificationFilter  = FilterMethod::Linear;
        sStates.magnificationFilter = FilterMethod::Linear;
        resources.createSamplerState( mSamplerStateLinear, sStates, 0, DebugSource() );
        
        ///////////////////////////////////////////////////////////////
        // Rasterizer States
        ///////////////////////////////////////////////////////////////
        RasterizerStateDesc rsStates;
        
        // Enable scissor test.
        rsStates.scissorTestEnable = true;
        resources.createRasterizerState( mRastStateScissorTest, rsStates, 0, DebugSource() );
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
             !mOwner.selectPixelShader( "drawBillboard", BILLBOARD_STANDARD ) )
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
        mDriver.setBlendState( mBlendStateSrcInvSrc );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformBillboardYLocked" ) ||
             !mOwner.selectPixelShader( "drawBillboard", BILLBOARD_STANDARD ) )
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
        mDriver.setBlendState( mBlendStateSrcInvSrc );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformBillboardAxisLocked" ) ||
             !mOwner.selectPixelShader( "drawBillboard", BILLBOARD_STANDARD ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : billboard2D (Technique)
    // Desc : Simple unfiltered 2D billboard.
    //-------------------------------------------------------------------------
    TechniqueResult billboard2D( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthStateDisabled );
        mDriver.setBlendState( mBlendStateSrcInvSrc );
        mDriver.setRasterizerState( mRastStateScissorTest ); // ToDo: Do not always force to true.

        // Force point sampling for input texture.
        mDriver.setSamplerState( 0, mSamplerStatePoint );

        // Select shaders
        #ifndef DX9C
            if ( !mOwner.selectVertexShader( "transformBillboard2D" ) )
                return TechniqueResult::Abort;
        #else
            if ( !mOwner.selectVertexShader( null ) )
                return TechniqueResult::Abort;
        #endif
        if ( !mOwner.selectPixelShader( "drawBillboard2D" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : billboard2DFiltered (Technique)
    // Desc : Simple linear filtered 2D billboard.
    //-------------------------------------------------------------------------
    TechniqueResult billboard2DFiltered( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthStateDisabled );
        mDriver.setBlendState( mBlendStateSrcInvSrc );
        mDriver.setRasterizerState( null );

        // Force linear sampling for input texture.
        mDriver.setSamplerState( 0, mSamplerStateLinear );

        // Select shaders
        if ( !mOwner.selectVertexShader( null ) ||
             !mOwner.selectPixelShader( "drawBillboard2D" ) )
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
            float4 screenTexCoords  : TEXCOORD1;
            float3 worldPosition    : TEXCOORD2;
            float  fogFactor        : TEXCOORD3;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
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

            // Calculate texture coordiates for looking up into a screen texture.
            computeMeshScreenTextureCoords( clipPosition, screenTexCoords );

            // Output depth as world position (at center of billboard)
            worldPosition = sourcePosition;
            fogFactor     = 0;
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
            float4 screenTexCoords : TEXCOORD1;
            float3 worldPosition   : TEXCOORD2;
            float  fogFactor       : TEXCOORD3;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
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

            // Calculate texture coordiates for looking up into a screen texture.
            computeMeshScreenTextureCoords( clipPosition, screenTexCoords );

            // Output depth as world position (at center of billboard)
            worldPosition = sourcePosition;
            fogFactor     = 0;
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : TransformBillboard() (Vertex Shader)
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
            float4 screenTexCoords : TEXCOORD1;
            float3 worldPosition   : TEXCOORD2;
            float  fogFactor       : TEXCOORD3;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
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

            // Calculate texture coordiates for looking up into a screen texture.
            computeMeshScreenTextureCoords( clipPosition, screenTexCoords );

            // Compute camera relative position (i.e. world space eye dir)
	        worldPosition = worldPosition - _cameraPosition;
            
            // Compute fog factor (adjust alpha by its inverse)
	        //fogFactor = computeFogFactor( length( worldPosition ), ${FogModel::Exponential} );
	        //color.a  *= 1.0f - fogFactor;
            fogFactor = 0;
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : transformBillboard2D() (Vertex Shader)
    // Desc : Standard 2D billboard vertex shader (D3D11 passthrough).
    //-----------------------------------------------------------------------------
    bool transformBillboard2D( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 sourcePosition   : POSITION;
            float4 sourceColor      : COLOR;
            float2 sourceTexCoords  : TEXCOORD0;
		?>

        // Define shader outputs.
        <?out
            float4 screenPosition  : SV_POSITION;
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
            // sqBillboard2D always subtracts a half texel such that it is compatible with the
            // DX9 texture coordinate offsets when working with screen-space rendering.
            // For the DX11 only vertex shader, we need to remove the influence that this had.
            sourcePosition.xy += 0.5f;

            // Screen space to clip space
            screenPosition.x = ((sourcePosition.x - _viewportOffset.x) * (_viewportSize.z * 2.0f)) - 1.0f;
            screenPosition.y = -((sourcePosition.y - _viewportOffset.y) * (_viewportSize.w * 2.0f)) + 1.0f;
            screenPosition.z = sourcePosition.z;
            screenPosition.w = 1;
            color            = sourceColor;
            texCoords        = sourceTexCoords;

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
    bool drawBillboard( int type )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 screenPosition   : SV_POSITION;
            float4 color            : COLOR;
            float2 texCoords        : TEXCOORD0;
            float4 screenTexCoords  : TEXCOORD1;
            float3 position         : TEXCOORD2;
            float  fogFactor        : TEXCOORD3;
		?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            // Modulate the texture and interpolated color
            data0 = sample2D( sBillboardInputTex, sBillboardInput, texCoords ) * color;

            // Optionally soften the billboard based on proximity to scene
            ?>
            if ( type == BILLBOARD_SOFT )
            {
                // ToDo: Reimplement this.

                /*<?
                float ooRange   = 1.0f / (BillboardFade.y - BillboardFade.x);
                float MinAdjust = BillboardFade.x * -ooRange;

                // Get the distance to the particle pixel and the scene	behind it
                float d0 = GetDistance( DepthSampler, In.ScreenTexCoords.xy / In.ScreenTexCoords.w );
                float d1 = length( In.Position );

                // Adjust the particle transparency based on depth difference
                BillboardColor.a *= saturate( (d0 - d1) * ooRange + MinAdjust );
                ?>*/
            
            } // End if BILLBOARD_SOFT
            <?

            // Include fog
            data0.rgb = lerp( data0.rgb * data0.a, _fogColor.rgb, fogFactor );
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : drawBillboard2D() (Pixel Shader)
    // Desc : 2D billboard pixel shader.
    //-------------------------------------------------------------------------
    bool drawBillboard2D( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 screenPosition : SV_POSITION;
            float4 color          : COLOR;
            float2 texCoords      : TEXCOORD0;
		?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            // Modulate the texture and interpolated color. Always forcably
            // sample from the top mip level to ensure that the hardware does
            // not mis-interpret our having scaled the billboard geometry, in
            // turn modifying the pixel aspect (DX11).
            data0 = sample2DLevel( sBillboardInputTex, sBillboardInput, texCoords, 0 ) * color;
        ?>
        
        // Valid shader
        return true;
    }
 
} // End Class : BillboardShader