//---------------------------------------------------------------------------//
//          ____                   _                    _                    //
//         / ___| _ __   ___  __ _| |_ _ __ ___  __   _/ | __  __            //
//         \___ \| '_ \ / _ \/ _` | __| '__/ _ \ \ \ / / | \ \/ /            //
//          ___) | |_) |  __/ (_| | |_| | |  __/  \ V /| |_ >  <             //
//         |____/| .__/ \___|\__, |\__|_|  \___|   \_/ |_(_)_/\_\            //
//               |_|       Game |_| Institute - Speqtre Game Engine          //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : SandboxElements.sh                                                 //
//                                                                           //
// Desc : Internal surface shader which defines a number of vertex and pixel //
//        shaders for use by the engine's sandbox mode rendering functions.  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2010 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "Utilities.shh"

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return SandboxElements( owner, driver, resources );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : SandboxElements (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for the various
//        sandbox rendering features (grid, gizmos, wireframes, etc.)
//-----------------------------------------------------------------------------
class SandboxElements : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
    // Public Member Variables
    ///////////////////////////////////////////////////////////////////////////
    // Shader parameters
    bool                            wireViewport;

    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;           // The parent application object.
    private RenderDriver@           mDriver;          // Render driver to which this shader is linked.
    private ResourceManager@        mResources;       // Resource manager that owns this shader.

    // Depth stencil states
    private DepthStencilStateHandle mDepthStateDisabled;
    private DepthStencilStateHandle mDepthStateWriteDisabled;

    // Rasterizer states
    private RasterizerStateHandle   mRastStateWireNoCull;
    private RasterizerStateHandle   mRastStateAAWireNoCull;
    private RasterizerStateHandle   mRastStateAALines;

    // Blend states
    private BlendStateHandle        mBlendStateAlphaEnabled;

    ///////////////////////////////////////////////////////////////////////////
    // Custom Constant Buffer Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?cbuffer cbSandboxData : register(b11), globaloffset(c150)
        float4  diffuseReflectance = float4(1,1,1,1);
        float4  ambientReflectance = float4(0,0,0,0);
        float4  shapeInteriorColor = float4(0,0,0,0);
        float4  shapeWireColor     = float4(0,0,0,0);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : SandboxElements () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	SandboxElements( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner = owner;
        @mDriver = driver;
        @mResources = resources;

        // Initialize variables to sensible defaults
        wireViewport = false;

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
        // Rasterizer States
        ///////////////////////////////////////////////////////////////
        RasterizerStateDesc rStates;

        // Wireframe with no culling.
        rStates.fillMode            = FillMode::Wireframe;
        rStates.cullMode            = CullMode::None;
        resources.createRasterizerState( mRastStateWireNoCull, rStates, 0, DebugSource() );

        // Antialiased wireframe with no culling.
        rStates.antialiasedLineEnable = true;
        rStates.multisampleEnable = false;
        resources.createRasterizerState( mRastStateAAWireNoCull, rStates, 0, DebugSource() );

        // Standard antialiased lines
        rStates = RasterizerStateDesc();
        rStates.antialiasedLineEnable = true;
        rStates.multisampleEnable = false;
        resources.createRasterizerState( mRastStateAALines, rStates, 0, DebugSource() );

        ///////////////////////////////////////////////////////////////
        // Blend State
        ///////////////////////////////////////////////////////////////
        BlendStateDesc bStates;

        // Wireframe with no culling.
        bStates.renderTarget0.blendEnable      = true;
        bStates.renderTarget0.sourceBlend      = BlendMode::SrcAlpha;
        bStates.renderTarget0.destinationBlend = BlendMode::InvSrcAlpha;
        resources.createBlendState( mBlendStateAlphaEnabled, bStates, 0, DebugSource() );
    }

    ///////////////////////////////////////////////////////////////////////////
    // Techniques
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : drawViewportGrid (Technique)
    // Desc : Draws the grid that appears in each editor viewport.
    //-------------------------------------------------------------------------
    TechniqueResult drawViewportGrid( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthStateWriteDisabled );
        mDriver.setBlendState( null );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformDefault", -1, false, true, false ) ||
             !mOwner.selectPixelShader( "drawDefault" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : drawViewportGridAA (Technique)
    // Desc : Draws the grid that appears in each editor viewport using
    //        line antialiasing.
    //-------------------------------------------------------------------------
    TechniqueResult drawViewportGridAA( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthStateWriteDisabled );
        mDriver.setBlendState( mBlendStateAlphaEnabled );
        mDriver.setRasterizerState( mRastStateAALines );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformDefault", -1, false, true, false ) ||
             !mOwner.selectPixelShader( "drawDefault" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : drawViewportGuides (Technique)
    // Desc : Draws the three small axis guides that appear in the bottom
    //        of each editor viewport.
    //-------------------------------------------------------------------------
    TechniqueResult drawViewportGuides( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( mDepthStateDisabled );
        mDriver.setBlendState( mBlendStateAlphaEnabled );
        mDriver.setRasterizerState( mRastStateAALines );

        // Select shaders (pixel shader only, this is a screen space process)
        if ( !mOwner.selectVertexShader( null ) ||
             !mOwner.selectPixelShader( "drawDefault" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : drawWireframeGizmo (Technique)
    // Desc : Draws a representation of a wireframe gizmo. Color is provided at
    //        the vertices.
    //-------------------------------------------------------------------------
    TechniqueResult drawWireframeGizmo( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( null );
        mDriver.setBlendState( null );
        mDriver.setRasterizerState( mRastStateWireNoCull );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformDefault", -1, false, true, false ) ||
             !mOwner.selectPixelShader( "drawDefault" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : drawGizmoArrowTips (Technique)
    // Desc : Draws the cone / arrow meshes at the end of the axis lines.
    //-------------------------------------------------------------------------
    TechniqueResult drawGizmoArrowTips( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( null );
        mDriver.setBlendState( null );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformGizmoMoveTips" ) ||
             !mOwner.selectPixelShader( "drawDefault" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : drawGizmoSpline (Technique)
    // Desc : Draws a 3D representation of a spline.
    //-------------------------------------------------------------------------
    TechniqueResult drawGizmoSpline( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( null );
        mDriver.setBlendState( null );
        mDriver.setRasterizerState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformDefault", -1, false, true, false ) ||
             !mOwner.selectPixelShader( "drawDefault" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }
    
    //-------------------------------------------------------------------------
    // Name : drawWireframeMesh (Technique)
    // Desc : Draws a wireframe representation of the mesh in its default state 
    //        using the specified DiffuseReflectance as the line color.
    //-------------------------------------------------------------------------
    TechniqueResult drawWireframeMesh( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setDepthStencilState( null );
        mDriver.setBlendState( null );
        mDriver.setRasterizerState( mRastStateWireNoCull );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformDefault", System.maxBlendIndex, System.useVTFBlending, false, true ) ||
             !mOwner.selectPixelShader( "drawDefault" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }


    //-------------------------------------------------------------------------
    // Name : drawWireframeNode (Technique)
    // Desc : Draws a wireframe representation of a node in its default state 
    //        using the color specified at the vertices.
    //-------------------------------------------------------------------------
    TechniqueResult drawWireframeNode( int pass, bool commitChanges )
    {
        // Set necessary states
        mDriver.setBlendState( null );
        mDriver.setRasterizerState( mRastStateWireNoCull );

        // Z-Buffer should be disabled when viewport is in wireframe mode
        if ( wireViewport )
            mDriver.setDepthStencilState( mDepthStateDisabled );
        else
            mDriver.setDepthStencilState( null );

        // Select shaders
        if ( !mOwner.selectVertexShader( "transformDefault", -1, false, true, false ) ||
             !mOwner.selectPixelShader( "drawDefault" ) )
            return TechniqueResult::Abort;

        // Single-pass. Process is complete.
        return TechniqueResult::Complete;
    }

    //-------------------------------------------------------------------------
    // Name : drawGhostedShapeMesh (Technique)
    // Desc : Draws a solid representation of a mesh shape with an alpha
    //        blended interior. Color is taken from the constants 
    //        'shapeInteriorColor' and 'shapeWireColor'.
    //-------------------------------------------------------------------------
    TechniqueResult drawGhostedShapeMesh( int pass, bool commitChanges )
    {
        // Two passes (solid and wire)
        if ( pass == 0 )
        {
            // Set necessary states
            mDriver.setBlendState( mBlendStateAlphaEnabled );
            mDriver.setRasterizerState( null );
            mDriver.setDepthStencilState( null );

            // Select vertex shader
            if ( !mOwner.selectVertexShader( "transformConvexShapeBiasedInterior" ) ||
                 !mOwner.selectPixelShader( "drawDefaultBiased" ) )
                return TechniqueResult::Abort;

            // First pass, needs second pass
            return TechniqueResult::Continue;

        } // End if pass 0
        else
        {
            // Set necessary states
            mDriver.setBlendState( mBlendStateAlphaEnabled );
            mDriver.setRasterizerState( mRastStateWireNoCull );
            mDriver.setDepthStencilState( null );

            // Select vertex shader
            if ( !mOwner.selectVertexShader( "transformConvexShapeBiasedWire" ) ||
                 !mOwner.selectPixelShader( "drawDefaultBiased" ) )
                return TechniqueResult::Abort;

            // Finished
            return TechniqueResult::Complete;

        } // End if pass 1
    }

    ///////////////////////////////////////////////////////////////////////////
    // Vertex Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : transformGizmoMoveTips () (Vertex Shader)
    // Desc : Custom tranformation shader for 'Move' tool gizmo arrow tips.
    //-------------------------------------------------------------------------
    bool transformGizmoMoveTips( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition   : POSITION;
            float3  sourceNormal     : NORMAL;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
            float4  diffuseColor    : COLOR0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbWorld;
            _cbCamera;
            cbSandboxData;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        clipPosition = mul( float4( sourcePosition, 1 ), _worldMatrix );
        clipPosition = mul( clipPosition, _viewProjectionMatrix );
        diffuseColor = ambientReflectance + (diffuseReflectance * saturate(dot(sourceNormal, float3(0,-1,0))));
        ?>

        // Valid shader!
        return true;
    }

    //-----------------------------------------------------------------------------
    // Name : transformDefault () (Vertex Shader)
    // Desc : Outputs simple transformed vertex data. Color is generated by
    //        optionally combing vertex and reflectance (constant) colors.
    //-----------------------------------------------------------------------------
    bool transformDefault( int maxBlendIndex, bool useVTFBlending, bool includeVertexColor, bool modulateReflectance )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition      : POSITION;
        ?>

        // Retrieve blend weights?
        if ( maxBlendIndex >= 0 )
        {
            <?in
                float4  blendWeights        : BLENDWEIGHT;
		        float4  blendIndices        : BLENDINDICES;
            ?>

        } // End if skinned

        // Retrieve color from vertex?
        if ( includeVertexColor )
        {
            <?in
                float4  sourceColor : COLOR0;
            ?>
        
        } // End if VertexColor

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
            float4  color           : COLOR0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            cbSandboxData;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Retrieve world matrix (skinned and unskinned cases)
        matrix worldTransform;
		?>
        if ( maxBlendIndex >= 0 )
		    <?getWorldMatrix( worldTransform, blendWeights, blendIndices, $maxBlendIndex, $useVTFBlending );?>
        else
            <?getWorldMatrix( worldTransform );?>

        <?
        // Compute world space position of vertex
        float3 worldPosition = mul( float4(sourcePosition,1), worldTransform );

        // Compute standard output values.
        clipPosition = mul( float4(worldPosition,1), _viewProjectionMatrix );
        ?>

        // Generate the color as appropriate
        if ( includeVertexColor )
        {
            if ( modulateReflectance )
                <?color = sourceColor * diffuseReflectance;?>
            else
                <?color = sourceColor;?>

        } // End if vertexColor
        else
        {
            if ( modulateReflectance )
                <?color = diffuseReflectance;?>
            else
                <?color = float4(1,1,1,1);?>
        
        } // End if !vertexColor

        // Valid shader!
        return true;
    }

    //-----------------------------------------------------------------------------
    // Name : transformConvexShapeBiasedWire () (Vertex Shader)
    // Desc : 
    //-----------------------------------------------------------------------------
    bool transformConvexShapeBiasedWire( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition      : POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
            float4  color           : COLOR0;
            float3  depthComponents : TEXCOORD0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbWorld;
            cbSandboxData;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Compute world space position of vertex
        float3 worldPosition = mul( float4(sourcePosition,1), _worldMatrix );

        // Compute output values.
        clipPosition    = mul( float4(worldPosition,1), _viewProjectionMatrix );
        depthComponents = float3( clipPosition.zw, 0.008f);
        color           = shapeWireColor;
        ?>

        // Valid shader!
        return true;
    }

    //-----------------------------------------------------------------------------
    // Name : transformConvexShapeBiasedInterior () (Vertex Shader)
    // Desc : 
    //-----------------------------------------------------------------------------
    bool transformConvexShapeBiasedInterior( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition      : POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
            float4  color           : COLOR0;
            float3  depthComponents : TEXCOORD0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbWorld;
            cbSandboxData;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Compute world space position of vertex
        float3 worldPosition = mul( float4(sourcePosition,1), _worldMatrix );

        // Compute output values.
        clipPosition    = mul( float4(worldPosition,1), _viewProjectionMatrix );
        depthComponents = float3( clipPosition.zw, 0.004f);
        color           = shapeInteriorColor;
        ?>

        // Valid shader!
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-----------------------------------------------------------------------------
    // Name : drawDefault () (Pixel Shader)
    // Desc : Default pixel shader, simply designed to output the color coming
    //        through from the vertex shader.
    //-----------------------------------------------------------------------------
    bool drawDefault( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4  screenPosition  : SV_POSITION;
            float4  sourceColor     : COLOR0;
        ?>

        // Define shader outputs.
        <?out
            float4  data0       : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        data0 = sourceColor;
		?>

        // Valid shader!
        return true;
    }

    //-----------------------------------------------------------------------------
    // Name : drawDefaultBiased () (Pixel Shader)
    // Desc : Default pixel shader, simply designed to output the color coming
    //        through from the vertex shader with a configurable depth bias.
    //        x = Non-linear Z*W, y = W, z = Bias amount in view space units
    //-----------------------------------------------------------------------------
    bool drawDefaultBiased( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4  screenPosition  : SV_POSITION;
            float4  sourceColor     : COLOR0;
            float3  depthComponents : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4  data0       : SV_TARGET0;
            float   depth       : SV_DEPTH;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Directly output source color
        data0  = sourceColor;

        // Generate biased depth  ((w - (bias/w)) * 33 + 43) / w
        depth  = ((depthComponents.y - (depthComponents.z / depthComponents.y)) * _projectionMatrix._33 + _projectionMatrix._43) / depthComponents.y;
		?>

        // Valid shader!
        return true;
    }

} // End Class : SandboxElements