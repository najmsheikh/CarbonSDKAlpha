///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "Utilities.shh"
#include "Types.shh"
#include "Config.shh"

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return FoliageShader( owner, driver, resources );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : FoliageShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for foliage 
//        clutter type.
//-----------------------------------------------------------------------------
class FoliageShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;             // The parent application object.
    private RenderDriver@           mDriver;            // Render driver to which this shader is linked.
    private ResourceManager@        mResources;         // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////////////////
    // Custom Constant Buffer Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?cbuffer cbFoliageData : register(b11), globaloffset(c150)
        float4	fadeDistances;
    ?>

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        sampler2D     sDiffuse                : register(s0);
		sampler2D     sNormal                 : register(s1);
        sampler2D     sScreenDoorNoise        : register(s2);
    ?>    

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : FoliageShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	FoliageShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
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
    // Name : transformDepth() (Vertex Shader)
    // Desc : Outputs vertex data needed for depth pass.
    //-------------------------------------------------------------------------
    bool transformDepth( int depthOutputType, int normalOutputType, bool orthographicCamera )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition  : POSITION;
            float3  sourceNormal    : NORMAL;
			float4  sourceColor     : COLOR;
			float2  sourceCoords    : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
            float4  eyeRay          : TEXCOORD0;
			float2  coords          : TEXCOORD1;
        ?>
		
        // Includes normal?
        if ( normalOutputType != NormalType::NoNormal )
        {
            <?out
                float3 normal       : TEXCOORD2;
            ?>

        } // End if output normal

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Compute standard output values.
        clipPosition = mul( float4(sourcePosition,1), _viewProjectionMatrix );
		coords       = sourceCoords;
		
        // Compute vertex to eye ray for distance computation.
        eyeRay.xyz = sourcePosition - _cameraPosition;
        eyeRay.w   = 0;
        ?>

        // Compute depth to interpolate if necessary.
		switch( getPureDepthType( depthOutputType ) )
		{
	        case DepthType::LinearZ:
				if ( orthographicCamera )
					<?eyeRay.w = clipPosition.z;?>  
				else
					<?eyeRay.w = clipPosition.w * _cameraRangeScale + _cameraRangeBias;?>  
	            break;
	        case DepthType::NonLinearZ:
				if ( orthographicCamera )
					<?eyeRay.w = clipPosition.z;?>  
				else
					<?eyeRay.w = clipPosition.w;?>  
                break;
		
        } // End switch

        // Compute normal
		switch ( normalOutputType )
		{
			case NormalType::NormalWorld:
				<?normal = sourceNormal;?>
			    break; 
			case NormalType::NormalView:
				<?normal = mul( sourceNormal, _viewMatrix );?>
			    break; 

        } // End switch

        // Valid shader
        return true;
    }
	
	//-------------------------------------------------------------------------
    // Name : transformGeometry() (Vertex Shader)
    // Desc : Outputs vertex data needed for geometry fill pass.
    //-------------------------------------------------------------------------
    bool transformGeometry( bool viewSpaceLighting )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition  : POSITION;
            float3  sourceNormal    : NORMAL;
			float4  sourceColor     : COLOR;
			float2  sourceCoords    : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
			float4  diffuseColor    : COLOR;
            float4  eyeRay          : TEXCOORD0;
			float2  coords          : TEXCOORD1;
			float3  normal          : TEXCOORD2;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Compute standard output values.
        clipPosition = mul( float4(sourcePosition,1), _viewProjectionMatrix );
        diffuseColor = sourceColor;
		coords       = sourceCoords;
		
        // Compute vertex to eye ray for distance computation.
        eyeRay.xyz = sourcePosition - _cameraPosition;
        eyeRay.w   = 0;
        ?>
		
		// Compute normal
		if ( viewSpaceLighting )
			<?normal = mul( sourceNormal, _viewMatrix );?>
		else
			<?normal = sourceNormal;?>

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
    bool drawDepth( int depthOutputType, int normalOutputType, bool orthographicCamera )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4  screenPosition  : SV_POSITION;
            float4  eyeRay          : TEXCOORD0;    // Interpolated depth in W
			float3	coords  		: TEXCOORD1;
        ?>

            // Includes normal?
        if ( normalOutputType != NormalType::NoNormal )
	    {
			<?in
				float3 normal       : TEXCOORD2;
			?>
	    
        } // End if normal input

        // Define shader outputs.
        <?out
            float4  depthOut        : SV_TARGET0;
        ?>
		
		// Includes normal?
        if ( normalOutputType != NormalType::NoNormal )
	    {
			<?out
				float4 normalOut    : SV_TARGET1;
			?>
	    
        } // End if normal output
		
		// Constant buffer usage.
		<?cbufferrefs
			cbFoliageData
		?>

        // Conditional constant buffer usage.
        int pureDepthType = getPureDepthType( depthOutputType );
        if ( pureDepthType == DepthType::NonLinearZ && !orthographicCamera )
        {
            <?cbufferrefs
                _cbCamera;
            ?>
        
        } // End if requires camera

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
			float attenuation = saturate((length(eyeRay.xyz) + fadeDistances.z) * fadeDistances.w);
			
			// Get the foliage alpha value
			float noise = sample2D( sScreenDoorNoiseTex, sScreenDoorNoise, coords ).r;
			float alpha = sample2D( sDiffuseTex, sDiffuse, coords ).a * noise;

			// Alpha test.
			clip( alpha - (attenuation + 0.08) );
		?>
		
		// Output depth        
		switch( pureDepthType )
		{
	        case DepthType::LinearZ:
		        <?depthOut = setNormalizedDistance( eyeRay.w, $depthOutputType );?>
	            break;
	        case DepthType::LinearDistance:
				<?depthOut = setDistance( length( eyeRay.xyz ), $depthOutputType );?>
	            break;
	        case DepthType::NonLinearZ:
				if ( !orthographicCamera )
		        	<?eyeRay.w = (1.0f / eyeRay.w) * _projectionMatrix._43 + _projectionMatrix._33;?>
		        <?depthOut = setNormalizedDistance( eyeRay.w, $depthOutputType );?>
	            break;
	        default:
				<?depthOut = 0;?>
	            break;
		
        } // End switch depthOutputType

        // Output surface normal if requested.
		if ( normalOutputType != NormalType::NoNormal )
			<?normalOut = float4( normal * 0.5f + 0.5f, 0 );?> 

        // Valid shader
        return true;
    }
	
	//-------------------------------------------------------------------------
    // Name : drawGeometry () (Pixel Shader)
    // Desc : G-buffer fill.
    //-------------------------------------------------------------------------
    bool drawGeometry( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4 screenPosition       : SV_POSITION;
            float4 diffuseColor         : COLOR0;
            float4 eyeDirIn             : TEXCOORD0;
			float2 coords               : TEXCOORD1;
            float3 normal               : TEXCOORD2;
        ?>

        // Define shader outputs (2 g-buffer targets)
        <?out
            float4 data0 : SV_TARGET0;
            float4 data1 : SV_TARGET1;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
			// Retrieve diffuse texture color.
			float4 color = sample2D( sDiffuseTex, sDiffuse, coords );
			
			// Alpha test (prevents sparkles).
			clip( color.a - 0.08 );
		
			// Output color
			data1.rgb = color.rgb * diffuseColor.rgb;
			data1.a = 0;
			
			// Output normal
			data0 = float4( normal * 0.5f + 0.5f, 0.31f );
        ?>

        // Valid shader
        return true;
     }

} // End Class : FoliageShader