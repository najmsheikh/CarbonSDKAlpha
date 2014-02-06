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
// Name : RenderDriver.sh                                                    //
//                                                                           //
// Desc : Internal surface shader which defines a number of vertex and pixel //
//        shaders specific to the internal implementation of the engine      //
//        render driver.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENTSCRIPT
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return RenderDriverShader( owner, driver, resources );
}
#endif // !_PARENTSCRIPT

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : RenderDriverShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for specific
//        internal render driver features.
//-----------------------------------------------------------------------------
class RenderDriverShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@      mOwner;           // The parent application object.
    private RenderDriver@       mDriver;          // Render driver to which this shader is linked.
    private ResourceManager@    mResources;       // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
		// Generic image samplers
        Sampler1D   sImage1D    : register(s0);
        Sampler2D   sImage2D0   : register(s0);
        Sampler2D   sImage2D1   : register(s1);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : RenderDriverShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
    RenderDriverShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
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
    //-------------------------------------------------------------------------
    // Name : transformSimpleDefault() (Vertex Shader)
    // Desc : Basic default transformation shader. Simply transforms an
    //        incoming 'sqShadedVertex' into clip space ready for
    //        rasterization.
    //-------------------------------------------------------------------------
    bool transformSimpleDefault( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float3 sourcePosition   : POSITION;
            float4 sourceColor      : COLOR0;
            float2 sourceTexCoords  : TEXCOORD0;
		?>

        // Define shader outputs.
        <?out
            float4 clipPosition : SV_POSITION;
            float4 color        : COLOR0;
            float2 texCoords    : TEXCOORD0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbWorld;
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            // Transform position into clip space.
            float4 worldPosition = mul( float4( sourcePosition, 1 ), _worldMatrix );
            clipPosition = mul( worldPosition, _viewProjectionMatrix );

            // Pass remaining values straight through.
            color       = sourceColor;
            texCoords   = sourceTexCoords;
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : transformScreenElement() (Vertex Shader)
    // Desc : Standard pass-through shader to convert screen space data into
    //        clip space data as required by DX11.
    //-----------------------------------------------------------------------------
    bool transformScreenElement( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 sourcePosition   : POSITION;
            float4 sourceColor      : COLOR;
		?>

        // Define shader outputs.
        <?out
            float4 screenPosition  : SV_POSITION;
            float4 color           : COLOR;
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
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : transformClipElement() (Vertex Shader)
    // Desc : Transform a clip space screen element by the required
    //        matrix (we use the world transform matrix for this).
    //-------------------------------------------------------------------------
    bool transformClipElement( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float3 sourcePosition   : POSITION;
            float4 sourceColor      : COLOR0;
		?>

        // Define shader outputs.
        <?out
            float4 elementPosition : SV_POSITION;
            float4 color           : COLOR0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbWorld;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
            // Transform position.
            elementPosition = mul( float4( sourcePosition, 1 ), _worldMatrix );

            // Pass remaining values straight through.
            color = sourceColor;
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : transformStretchRect() (Vertex Shader)
    // Desc : Standard 2D stretch rect vertex shader (D3D11 passthrough).
    //-----------------------------------------------------------------------------
    bool transformStretchRect( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 sourcePosition   : POSITION;
            float2 sourceTexCoords  : TEXCOORD0;
		?>

        // Define shader outputs.
        <?out
            float4 screenPosition  : SV_POSITION;
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
            // Screen space to clip space
            screenPosition.x = ((sourcePosition.x - _viewportOffset.x) * (_viewportSize.z * 2.0f)) - 1.0f;
            screenPosition.y = -((sourcePosition.y - _viewportOffset.y) * (_viewportSize.w * 2.0f)) + 1.0f;
            screenPosition.z = sourcePosition.z;
            screenPosition.w = 1;
            texCoords        = sourceTexCoords;

        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : defaultVertexShader() (Vertex Shader)
    // Desc : System level default vertex shader (there must always be one).
    //-------------------------------------------------------------------------
    bool defaultVertexShader( )
    {
        <?out
            float4 pos : SV_POSITION;
        ?>
        <?
            pos = float4(0,0,0,0);
        ?>
        
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : drawScreenElement() (Pixel Shader)
    // Desc : Basic pixel shader used for the rendering of screen elements
    //        such as lines and rectangles.
    //-------------------------------------------------------------------------
    bool drawScreenElement( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 pos : SV_POSITION;
            float4 color : COLOR0;
		?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?data0 = color;?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : drawStretchRect() (Pixel Shader)
    // Desc : 2D stretchRect pixel shader.
    //-------------------------------------------------------------------------
    bool drawStretchRect( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 screenPosition : SV_POSITION;
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
            // Always forcibly sample from the top mip level to ensure that the hardware does
            // not mis-interpret our having scaled the geometry, in turn modifying the pixel aspect (DX11).
            data0 = sample2DLevel( sImage2D0Tex, sImage2D0, texCoords, 0 );
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : testLinearFiltering() (Pixel Shader)
    // Desc : Pixel shader used to determine whether linear filtering is 
    //        supported for a given texture format.
    // Note : The input is a 1D texture consisting of a 0.0 and a 1.0 texel.
    //-------------------------------------------------------------------------
    bool testLinearFiltering( bool bTestAlpha )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
		<?in
            float4 color : COLOR0;
		?>

        // Define shader outputs.
        <?out
            float4 data0 : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?float sample;?>
        
        // If this is an alpha only format, test that, otherwise the red channel will do
        if ( bTestAlpha )
			<?sample = sample1D( sImage1DTex, sImage1D, 0.5f ).a;?>
        else
			<?sample = sample1D( sImage1DTex, sImage1D, 0.5f ).r;?>
		
		<?data0 = (sample > 0.01 && sample < 0.99f) ? float4( 1, 1, 1, 1 ) : float4( 0, 0, 0, 0 );?>
        
        // Valid shader
        return true;
    }

    // ToDo: 6767 - What are these doing in here?
	//-----------------------------------------------------------------------------
	// Name : mergeDepthAndNormalBuffers()
	// Desc : Combines depth and normal buffers into a single buffer
	//-----------------------------------------------------------------------------
	bool mergeDepthAndNormalBuffers( )
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
		?>
		
		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		data0.w   = sample2D( sImage2D0Tex, sImage2D0, texCoords ).r;
		data0.xyz = sample2D( sImage2D1Tex, sImage2D1, texCoords ).rgb * 2.0f - 1.0f;
		data0.xyz = normalize( data0.xyz );
		?>

		// Valid shader
		return true;
	}
 
} // End Class : RenderDriverShader
