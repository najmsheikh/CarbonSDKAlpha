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
    return LandscapeTerrainShader( owner, driver, resources );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : LandscapeTerrainShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for the terrain 
//        portion of a scene's landscape.
//-----------------------------------------------------------------------------
class LandscapeTerrainShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;             // The parent application object.
    private RenderDriver@           mDriver;            // Render driver to which this shader is linked.
    private ResourceManager@        mResources;         // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////////////////
    // Common Shader Declarations / Definitions
    ///////////////////////////////////////////////////////////////////////////
    <?ctype LayerData
        float4  textureSize;
        float2  scale;
        float2  baseScale;
        float4  rotation;
        float2  offset;
    ?>

    <?ctype ProceduralData
        float3  heightParams;     // X = Center, Y = -(1/FadeRange), Z = ((Max-Center)+FadeRange)/FadeRange
        float3  slopeAxis;        // Direction Vector
        float2  slopeParams;      // X = SlopeScale, Y = SlopeBias
        float   weight;
    ?>

    ///////////////////////////////////////////////////////////////////////////
    // Custom Constant Buffer Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?cbuffer cbTerrainBaseData : register(b11), globaloffset(c150)
        float4          terrainData_Color           = float4(1,1,1,1);
        float4          terrainData_TerrainSize;
        float3          terrainData_TerrainOffset   = float3(0,0,0);
        float           terrainData_NoiseStrength   = 0.5f;
    ?>

    <?cbuffer cbTerrainLayerData : register(b12), globaloffset(c154)
        LayerData       terrainData_Layers[4];                   // 4 * 5 registers = 20 registers
    ?>

    <?cbuffer cbTerrainPaintData : register(b13), globaloffset(c174)
        float2          terrainData_BlendMapScale   = float2(1,1);
        float2          terrainData_BlendMapOffset  = float2(0,0);
        float2          terrainData_PassCount;  // x = count, y = 1/count
    ?>

    <?cbuffer cbTerrainProcData : register(b13), globaloffset(c174)
        ProceduralData  terrainData_Procedural[3];
    ?>

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        sampler2D     sTerrainBlendMap        : register(s0);
        sampler2D     sTerrainNoise           : register(s1);
        sampler2D     sTerrainLayerColor0     : register(s2);
        sampler2D     sTerrainLayerColor1     : register(s3);
        sampler2D     sTerrainLayerColor2     : register(s4);
        sampler2D     sTerrainLayerColor3     : register(s5);     // TerrainDrawPainted Only
        sampler2D     sTerrainLayerNormal2    : register(s5);     // TerrainDrawProcedural Only
        sampler2D     sTerrainLayerNormal0    : register(s6);
        sampler2D     sTerrainLayerNormal1    : register(s7);
        sampler2D     sTerrainNormalMap       : register(s8);
    ?>    

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : LandscapeTerrainShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	LandscapeTerrainShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
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
    // Name : transformDefault () (Vertex Shader)
    // Desc : Outputs simple transformed vertex data (wireframe views, etc.)
    //-------------------------------------------------------------------------
    bool transformDefault( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition  : POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
            float4  diffuseColor    : COLOR0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            cbTerrainBaseData;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Compute standard output values.
        clipPosition = mul( float4(sourcePosition,1), _viewProjectionMatrix );
        
        // Return wireframe color even if this is overriden by the pixel shader.
        diffuseColor = terrainData_Color;
        ?>

        // Valid shader
        return true;
    }

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
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
            float4  eyeRay          : TEXCOORD0;
        ?>

        // Includes normal?
        if ( normalOutputType != NormalType::NoNormal )
        {
            <?out
                float3 normal       : TEXCOORD1;
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
    // Name : transformShadowMap() (Vertex Shader)
    // Desc : Outputs vertex data needed for shadow map population.
    //-------------------------------------------------------------------------
    bool transformShadowMap( int lightType )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition  : POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition    : SV_POSITION;
            float   depth           : TEXCOORD0;
            float2  texCoords       : TEXCOORD1;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
        ?>
        
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Unused.
        texCoords = float2(0,0);
        ?>

        // Transform from world to light clip space
        if ( lightType == LightType::Projector )
            <?clipPosition = computeProjShadowClipPosition( float4(sourcePosition,1) );?>
        else
            <?clipPosition = mul( float4( sourcePosition, 1 ), _viewProjectionMatrix );?>

        // Output depth should be linear
        if ( lightType == LightType::Directional || lightType == LightType::Projector )
        {
            <?depth = clipPosition.z;?>

        } // End if Directional || Projector
        else
        {
            <?depth = clipPosition.w * _cameraRangeScale + _cameraRangeBias;?>

        } // End if other type

        // ToDo: 9999 - Add a small depth bias?

        // Used to align shadow texel for border repair
        <?
        //clipPosition.x -= _targetSize.z * clipPosition.w;
        //clipPosition.y += _targetSize.w * clipPosition.w;
        ?>

        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : transformPainted () (Vertex Shader)
    // Desc : Outputs transformed vertex data for painted shaders.
    //-------------------------------------------------------------------------
    bool transformPainted( int layerCount )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition      : POSITION;
            float3  sourceNormal        : NORMAL;
            float4  sourceColor         : COLOR0;
        ?>
        
        // Define shader outputs.
        <?out
            float4  clipPosition        : SV_POSITION;
            float4  diffuseColor        : COLOR0;
            float4  eyeDir              : TEXCOORD0;
            float4  layerCoords01       : TEXCOORD1;
            float4  layerCoords23       : TEXCOORD2;
            float4  layerBaseCoords01   : TEXCOORD3;
            float4  layerBaseCoords23   : TEXCOORD4;
            float4  terrainCoords       : TEXCOORD5;
            float3  normal              : TEXCOORD6;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            cbTerrainBaseData;
            cbTerrainPaintData;
            cbTerrainLayerData;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Compute standard output values.
        clipPosition = mul( float4(sourcePosition,1), _viewProjectionMatrix );
        normal       = sourceNormal;
        diffuseColor = sourceColor;
        
        // Compute base coordinates from object space position.
        terrainCoords.xy = (sourcePosition.xz - terrainData_TerrainOffset.xz) * terrainData_TerrainSize.zw;
        terrainCoords.zw = terrainCoords.xy * terrainData_BlendMapScale + terrainData_BlendMapOffset;
        
        // Compute layer texture coordinates
        float2 coords = terrainCoords.xy * terrainData_Layers[0].scale + (terrainData_Layers[0].offset * terrainData_Layers[0].textureSize.zw);
        layerCoords01.x  = dot( terrainData_Layers[0].rotation.xy, coords.xy );
        layerCoords01.y  = dot( terrainData_Layers[0].rotation.zw, coords.xy );
        layerBaseCoords01.xy = layerCoords01.xy * terrainData_Layers[0].baseScale;
        ?>

        if ( layerCount >= 2 )
        {
            <?
            coords = terrainCoords.xy * terrainData_Layers[1].scale + (terrainData_Layers[1].offset * terrainData_Layers[1].textureSize.zw);
            layerCoords01.z  = dot( terrainData_Layers[1].rotation.xy, coords.xy );
            layerCoords01.w  = dot( terrainData_Layers[1].rotation.zw, coords.xy );
            layerBaseCoords01.zw = layerCoords01.zw * terrainData_Layers[1].baseScale;
            ?>
        }
        if ( layerCount >= 3 )
        {
            <?
            coords = terrainCoords.xy * terrainData_Layers[2].scale + (terrainData_Layers[2].offset * terrainData_Layers[2].textureSize.zw);
            layerCoords23.x  = dot( terrainData_Layers[2].rotation.xy, coords.xy );
            layerCoords23.y  = dot( terrainData_Layers[2].rotation.zw, coords.xy );
            layerBaseCoords23.xy = layerCoords23.xy * terrainData_Layers[2].baseScale;
            ?>
        }
        if ( layerCount >= 4 )
        {
            <?
            coords = terrainCoords.xy * terrainData_Layers[3].scale + (terrainData_Layers[3].offset * terrainData_Layers[3].textureSize.zw);
            layerCoords23.z  = dot( terrainData_Layers[3].rotation.xy, coords.xy );
            layerCoords23.w  = dot( terrainData_Layers[3].rotation.zw, coords.xy );
            layerBaseCoords23.zw = layerCoords23.zw * terrainData_Layers[3].baseScale;
            ?>
        }
        
        <?
        // Compute eye direction for specular computation
        eyeDir.xyz = sourcePosition - _cameraPosition;
        
        // Interpolated height stored in eyeDir.w
        eyeDir.w = sourcePosition.y;
        ?>

        // Valid Shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : transformProcedural () (Vertex Shader)
    // Desc : Outputs transformed vertex data for procedural shaders.
    //-------------------------------------------------------------------------
    bool transformProcedural( int layerCount )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float3  sourcePosition      : POSITION;
            float4  sourceColor         : COLOR0;
        ?>

        // Define shader outputs.
        <?out
            float4  clipPosition        : SV_POSITION;
            float4  diffuseColor        : COLOR0;
            float4  eyeDir              : TEXCOORD0;
            float4  layerCoords01       : TEXCOORD1;
            float4  layerCoords23       : TEXCOORD2;
            float4  layerBaseCoords01   : TEXCOORD3;
            float4  layerBaseCoords23   : TEXCOORD4;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            cbTerrainBaseData;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Compute standard output values.
        clipPosition      = mul( float4(sourcePosition,1), _viewProjectionMatrix );
        diffuseColor      = sourceColor;
        layerCoords23.zw  = (sourcePosition.xz - terrainData_TerrainOffset.xz) * terrainData_TerrainSize.zw;
            
        // Compute layer texture coordinates
        layerCoords01.xy     = layerCoords23.zw * terrainData_Layers[0].scale + (terrainData_Layers[0].offset * terrainData_Layers[0].textureSize.zw);
        layerBaseCoords01.xy = layerCoords01.xy * terrainData_Layers[0].baseScale;
        ?>

        if ( layerCount >= 2 )
        {
            <?
            layerCoords01.zw     = layerCoords23.zw * terrainData_Layers[1].scale + (terrainData_Layers[1].offset * terrainData_Layers[1].textureSize.zw);
            layerBaseCoords01.zw = layerCoords01.zw * terrainData_Layers[1].baseScale;
            ?>
        }
        if ( layerCount >= 3 )
        {
            <?
            layerCoords23.xy     = layerCoords23.zw * terrainData_Layers[2].scale + (terrainData_Layers[2].offset * terrainData_Layers[2].textureSize.zw);
            layerBaseCoords23.xy = layerCoords23.xy * terrainData_Layers[2].baseScale;
            ?>
        }
        
        <?
        // Compute eye direction for specular computation
        eyeDir.xyz = sourcePosition - _cameraPosition;
        
        // Interpolated height stored in eyeDir.w
        eyeDir.w = sourcePosition.y;
        ?>

        // Valid shader
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name  : terrainDrawDepth() (Pixel Shader)
    // Desc  : Terrain depth render pixel shader
    //-------------------------------------------------------------------------
    bool terrainDrawDepth( int depthOutputType, int normalOutputType, bool orthographicCamera )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4  screenPosition  : SV_POSITION;
            float4  eyeRay          : TEXCOORD0;    // Interpolated depth in W
        ?>

            // Includes normal?
        if ( normalOutputType != NormalType::NoNormal )
	    {
			<?in
				float3 normal       : TEXCOORD1;
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
    // Name  : terrainDrawShadowDepth() (Pixel Shader)
    // Desc  : Terrain shadow depth render pixel shader
    //-------------------------------------------------------------------------
    bool terrainDrawShadowDepth( )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4  screenPosition  : SV_POSITION;
            float   depth           : TEXCOORD0;
        ?>

        // Define shader outputs.
        <?out
            float4  data0           : SV_TARGET0;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?data0 = float4(depth,0,0,0);?>

        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : terrainDrawPainted () (Pixel Shader)
    // Desc : G-buffer fill based on painted blend map data.
    //-------------------------------------------------------------------------
    bool terrainDrawPainted( int layerCount, bool swizzle, bool tileReduce0, bool tileReduce1, bool tileReduce2, bool tileReduce3, bool viewSpaceNormals )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4 screenPosition       : SV_POSITION;
            float4 diffuseColor         : COLOR0;
            float4 eyeDirIn             : TEXCOORD0;
            float4 layerCoords01        : TEXCOORD1;
            float4 layerCoords23        : TEXCOORD2;
            float4 layerBaseCoords01    : TEXCOORD3;
            float4 layerBaseCoords23    : TEXCOORD4;
            float4 terrainCoords        : TEXCOORD5;
            float3 normal               : TEXCOORD6;
        ?>

        // Define shader outputs (2 g-buffer targets)
        <?out
            float4 data0 : SV_TARGET0;
            float4 data1 : SV_TARGET1;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            cbTerrainBaseData;
            cbTerrainPaintData;
            cbTerrainLayerData;
        ?>
        
        if ( viewSpaceNormals )
        {
            <?cbufferrefs
                _cbCamera;
            ?>
        
        } // End if view space normals

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        float4 one = {1,1,1,1};
        float4 weights = float4(0,0,0,0);
        float4 layerColor0, layerColor1, layerColor2, layerColor3, finalColor, color0, color1;
        float  sumWeights = 0.0f;
        ?>

        // Extract blend map data.
        if ( swizzle )
            <?weights.ar = sample2D( sTerrainBlendMapTex, sTerrainBlendMap, terrainCoords.zw ).gb;?>
        else
            <?weights = sample2D( sTerrainBlendMapTex, sTerrainBlendMap, terrainCoords.zw );?>
            
        <?
        // Sample noise texture for tiling reduction and normal detail (signed V8U8).
        float2 noise = sample2D( sTerrainNoiseTex, sTerrainNoise, terrainCoords.xy * 8.0f ).xy;
        
        // Sample normal from normal map
        //half3 normal = 2.0f * sample2D( sTerrainNormalMapTex, sTerrainNormalMap, terrainCoords.xy ).rgb - 1.0f;
        
        // Add detail to normal.
        normal.xz = noise.xy * terrainData_NoiseStrength + normal.xz;
        normal = normalize( normal );
            
        // Generate tangent frame for first two layers.
        half3 tangent0, binormal0, tangent1, binormal1;
        tangent0.xyz  = half3( terrainData_Layers[0].rotation.x, 0, terrainData_Layers[0].rotation.y );
        tangent0.xyz  = normalize( tangent0.xyz - (normal.xyz * dot( normal.xyz, tangent0.xyz )));
        binormal0.xyz = cross( normal.xyz, tangent0.xyz );
        ?>

        if ( layerCount >= 2 )
        {
            <?
            tangent1.xyz  = half3( terrainData_Layers[1].rotation.x, 0, terrainData_Layers[1].rotation.y );
            tangent1.xyz  = normalize( tangent1.xyz - (normal.xyz * dot( normal.xyz, tangent1.xyz )));
            binormal1.xyz = cross( normal.xyz, tangent1.xyz );
            ?>
        
        } // End if second bump layer
            
        <?
        // Compute t value used to lerp between high and low resolution texture representations.
        float distance = length(eyeDirIn.xyz);
        float t        = saturate( distance * 0.005f - (100.0f * 0.005f) ); // / 200.0f
        
        // Compute layer color belnding values (vectorized)
        float4 t2      = t.xxxx + 0.5f;
        float4 average = one / (one + t2);
        t2            *= average;
        ?>
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 0 Color Generation
        ///////////////////////////////////////////////////////////////////////
        if ( tileReduce0 )
        {
            <?
            // Sample layer 0 colors at differing scales
            color0 = sample2D( sTerrainLayerColor0Tex, sTerrainLayerColor0, layerCoords01.xy );
            color1 = sample2D( sTerrainLayerColor0Tex, sTerrainLayerColor0, layerBaseCoords01.xy );
            
            // Compute the final blended color
            layerColor0 = color0*average.x + color1*t2.x;
            ?>
        
        } // End if no tile reduction
        else
        {
            // Sample once.
            <?
            layerColor0 = sample2D( sTerrainLayerColor0Tex, sTerrainLayerColor0, layerCoords01.xy );
            ?>
        
        } // End if tile reduction
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 1 Color Generation
        ///////////////////////////////////////////////////////////////////////
        
        if ( layerCount >= 2 )
        {
            if ( tileReduce1 )
            {
                <?
                // Sample layer 1 colors at differing scales
                color0 = sample2D( sTerrainLayerColor1Tex, sTerrainLayerColor1, layerCoords01.zw );
                color1 = sample2D( sTerrainLayerColor1Tex, sTerrainLayerColor1, layerBaseCoords01.zw );
                
                // Compute the final blended color
                layerColor1 = color0*average.y + color1*t2.y;
                ?>
            
            } // End if no tile reduction
            else
            {
                // Sample once.
                <?
                layerColor1 = sample2D( sTerrainLayerColor1Tex, sTerrainLayerColor1, layerCoords01.zw );
                ?>
            
            } // End if tile reduction
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 2 Color Generation
        ///////////////////////////////////////////////////////////////////////
        
        if ( layerCount >= 3 )
        {
            if ( tileReduce2 )
            {
                <?
                // Sample layer 2 colors at differing scales
                color0 = sample2D( sTerrainLayerColor2Tex, sTerrainLayerColor2, layerCoords23.xy );
                color1 = sample2D( sTerrainLayerColor2Tex, sTerrainLayerColor2, layerBaseCoords23.xy );
                
                // Compute the final blended color
                layerColor2 = color0*average.z + color1*t2.z;
                ?>
            
            } // End if no tile reduction
            else
            {
                <?
                // Sample once.
                layerColor2 = sample2D( sTerrainLayerColor2Tex, sTerrainLayerColor2, layerCoords23.xy );
                ?>
            
            } // End if tile reduction
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 3 Color Generation
        ///////////////////////////////////////////////////////////////////////
        
        if ( layerCount >= 4 )
        {
            if ( tileReduce3 )
            {
                <?
                // Sample layer 3 colors at differing scales
                color0 = sample2D( sTerrainLayerColor3Tex, sTerrainLayerColor3, layerCoords23.zw );
                color1 = sample2D( sTerrainLayerColor3Tex, sTerrainLayerColor3, layerBaseCoords23.zw );
                
                // Compute the final blended color
                layerColor3 = color0*average.w + color1*t2.w;
                ?>
            
            } // End if no tile reduction
            else
            {
                // Sample once.
                <?
                layerColor3 = sample2D( sTerrainLayerColor3Tex, sTerrainLayerColor3, layerCoords23.zw );
                ?>
            
            } // End if tile reduction
        }
        
        // Construct final color based on final weights
        <?
        finalColor = layerColor0 * weights.a;
        sumWeights = weights.a;
        ?>

        if ( layerCount >= 2 )
        {
            <?
            finalColor += layerColor1 * weights.r;
            sumWeights += weights.r;
            ?>
        }
        if ( layerCount >= 3 )
        {
            <?
            finalColor += layerColor2 * weights.g;
            sumWeights += weights.g;
            ?>
        }
        if ( layerCount >= 4 )
        {
            <?
            finalColor += layerColor3 * weights.b;
            sumWeights += weights.b;
            ?>
        }
            
        ///////////////////////////////////////////////////////////////////////
        // Normal Generation
        ///////////////////////////////////////////////////////////////////////
        
        // Extract and weight layer0 normal
        <?
        float2 twoWeights    = 2.0f * weights.ar;
        float3 layerNormal   = twoWeights.x * sample2D( sTerrainLayerNormal0Tex, sTerrainLayerNormal0, layerCoords01.xy ).rgb - weights.a;
        
        // Transform tangent space normal into world space and sum the weighted normal.
        half3 terrainNormal;
        terrainNormal.x = dot( layerNormal, float3( tangent0.x, binormal0.x, normal.x ) );
        terrainNormal.y = dot( layerNormal, float3( tangent0.y, binormal0.y, normal.y ) );
        terrainNormal.z = dot( layerNormal, float3( tangent0.z, binormal0.z, normal.z ) );
        ?>
        
        // Handle second layer.
        if ( layerCount >= 2 )
        {
            <?
            // Extract and weight the normal.
            layerNormal   = twoWeights.y * sample2D( sTerrainLayerNormal1Tex, sTerrainLayerNormal1, layerCoords01.zw ).rgb - weights.r;
            
            // Transform tangent space normal into world space and sum the weighted normal.
            terrainNormal.x += dot( layerNormal, float3( tangent1.x, binormal1.x, normal.x ) );
            terrainNormal.y += dot( layerNormal, float3( tangent1.y, binormal1.y, normal.y ) );
            terrainNormal.z += dot( layerNormal, float3( tangent1.z, binormal1.z, normal.z ) );
            ?>
        
        } // End if two (or more) layers
        
        // Continue to sum remaining color only layer normals.
        if ( layerCount >= 3 )
            <?terrainNormal += normal.xyz * weights.g;?>
        if ( layerCount >= 4 )
            <?terrainNormal += normal.xyz * weights.b;?>

        // ToDo: Can we push this back to the vertex shader when using vertex normals?
        // Is it reliable enough given the way the tangent frame is computed?
        
        // Transform terrain normal into view space if required.
		if ( viewSpaceNormals )
            <?terrainNormal = mul( terrainNormal, _viewMatrix );?>
        
        ///////////////////////////////////////////////////////////////////////
        // G-Buffer Output
        ///////////////////////////////////////////////////////////////////////  
        <?
        // Store the normal and diffuse color as RGB and the sum of weights in alpha (for blending)
        data0.rgb = (terrainNormal.xyz * 0.5f + 0.5f) * terrainData_PassCount.y;
        data0.a   = sumWeights;
        
        data1.rgb = finalColor.rgb * diffuseColor.rgb;
        data1.a   = sumWeights;
        ?>

        // Valid shader
        return true;
     }

    //-------------------------------------------------------------------------
    // Name : terrainDrawProcedural () (Pixel Shader)
    // Desc : Procedural terrain texture g-buffer fill. 
    //-------------------------------------------------------------------------
    bool terrainDrawProcedural( int layerCount, bool tileReduce0, bool tileReduce1, bool tileReduce2 )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4 screenPosition       : SV_POSITION;
            float4 diffuseColor         : COLOR0;
            float4 eyeDirIn             : TEXCOORD0;
            float4 layerCoords01        : TEXCOORD1;
            float4 layerCoords23        : TEXCOORD2;
            float4 layerBaseCoords01    : TEXCOORD3;
            float4 layerBaseCoords23    : TEXCOORD4;
        ?>

        // Define shader outputs (2 g-buffer targets)
        <?out
            float4 data0 : TARGET0;
            float4 data1 : TARGET1;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
            cbTerrainBaseData;
            cbTerrainProcData;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        float3   one = {1,1,1};    
        float4   layerColor0, layerColor1, layerColor2, finalColor, color0, color1;
        float3   weights     = float3(terrainData_Procedural[0].weight, terrainData_Procedural[1].weight, terrainData_Procedural[2].weight);
        float    fSumWeights = 0.0f;
        
        // Sample noise texture for tiling reduction and normal detail (signed V8U8).
        float2 noise = sample2D( sTerrainNoiseTex, sTerrainNoise, layerCoords23.zw * 10.0f ).xy;
        
        // Sample normal from normal map
        half3 normal = 2.0f * sample2D( sTerrainNormalMapTex, sTerrainNormalMap, layerCoords23.zw ).rgb - 1.0f;

        // Add detail to normal.
        normal.xz = noise.xy * terrainData_NoiseStrength + normal.xz;
        normal = normalize( normal );
        
        // Generate tangent frame.
        float3 tangent  = normalize( normal * -normal.x + float3(1,0,0) ); // GramSchmidt orthogonalize (re-arranged to a "mad")
        float3 binormal = cross( normal, tangent );
        float3x3 TangentToWorld = { tangent, binormal, normal };
        
        // Compute t value used to lerp between high and low resolution texture representations.
        float distance = length(eyeDirIn.xyz);
        float t        = saturate( distance * 0.005f - (100.0f * 0.005f) /* / 200.0f */ );
        
        ///////////////////////////////////////////////////////////////////////
        // Compute procedural height weights (vectorized)
        ///////////////////////////////////////////////////////////////////////
        float3 heightParams0 = terrainData_Procedural[0].heightParams;
        float3 heightParams1 = terrainData_Procedural[1].heightParams;
        float3 heightParams2 = terrainData_Procedural[2].heightParams;
        float3 heightCenters = { heightParams0.x, heightParams1.x, heightParams2.x };
        float3 heightOORange = { heightParams0.y, heightParams1.y, heightParams2.y };
        float3 heightAdjust  = { heightParams0.z, heightParams1.z, heightParams2.z };
        weights.xyz *= saturate( abs( heightCenters - eyeDirIn.w ) * heightOORange + heightAdjust );
        
        ///////////////////////////////////////////////////////////////////////
        // Compute procedural slope weights (vectorized)
        ///////////////////////////////////////////////////////////////////////
        float2 slopeParams0 = terrainData_Procedural[0].slopeParams;
        float2 slopeParams1 = terrainData_Procedural[1].slopeParams;
        float2 slopeParams2 = terrainData_Procedural[2].slopeParams;
        float3 slopeAxis0   = terrainData_Procedural[0].slopeAxis;
        float3 slopeAxis1   = terrainData_Procedural[1].slopeAxis;
        float3 slopeAxis2   = terrainData_Procedural[2].slopeAxis;
        float3 slope        = { dot(slopeAxis0, Normal), dot(slopeAxis1, Normal), dot(slopeAxis2, Normal) };
        float3 slopeScale   = { slopeParams0.x, slopeParams1.x, slopeParams2.x };
        float3 slopeBias    = { slopeParams0.y, slopeParams1.y, slopeParams2.y };
        weights.xyz        *= saturate( (-slope * slope + 1.0f) * slopeScale + slopeBias );
        
        ///////////////////////////////////////////////////////////////////////
        // Compute layer color belnding values (vectorized)
        ///////////////////////////////////////////////////////////////////////
        float3 t2      = t.xxx + 0.5f;
        float3 average = one / (one + t2);
        t2            *= average;
        ?>
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 0 Color Generation
        ///////////////////////////////////////////////////////////////////////
        if ( tileReduce0 )
        {
            <?
            // Sample layer 0 colors at differing scales
            color0 = sample2D( sTerrainLayerColor0Tex, sTerrainLayerColor0, layerCoords01.xy );
            color1 = sample2D( sTerrainLayerColor0Tex, sTerrainLayerColor0, layerBaseCoords01.xy );
            
            // Compute the final blended color
            layerColor0 = color0*average.x + color1*t2.x;
            ?>
        
        } // End if no tile reduction
        else
        {
            // Sample once.
            <?
            layerColor0 = sample2D( sTerrainLayerColor0Tex, sTerrainLayerColor0, layerCoords01.xy );
            ?>
        
        } // End if tile reduction
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 1 Color Generation
        ///////////////////////////////////////////////////////////////////////
        
        if ( layerCount >= 2 )
        {
            if ( tileReduce1 )
            {
                <?
                // Sample layer 1 colors at differing scales
                color0 = sample2D( sTerrainLayerColor1Tex, sTerrainLayerColor1, layerCoords01.zw );
                color1 = sample2D( sTerrainLayerColor1Tex, sTerrainLayerColor1, layerBaseCoords01.zw );
                
                // Compute the final blended color
                layerColor1 = color0*average.y + color1*t2.y;
                ?>
            
            } // End if no tile reduction
            else
            {
                // Sample once.
                <?
                layerColor1 = sample2D( sTerrainLayerColor1Tex, sTerrainLayerColor1, layerCoords01.zw );
                ?>
            
            } // End if tile reduction
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 2 Color Generation
        ///////////////////////////////////////////////////////////////////////
        
        if ( layerCount >= 3 )
        {
            if ( tileReduce2 )
            {
                // Sample layer 2 colors at differing scales
                <?
                color0 = sample2D( sTerrainLayerColor2Tex, sTerrainLayerColor2, layerCoords23.xy );
                color1 = sample2D( sTerrainLayerColor2Tex, sTerrainLayerColor2, layerBaseCoords23.xy );
                
                // Compute the final blended color
                layerColor2 = color0*average.z + color1*t2.z;
                ?>
            
            } // End if no tile reduction
            else
            {
                // Sample once.
                <?
                layerColor2 = sample2D( sTerrainLayerColor2Tex, sTerrainLayerColor2, layerCoords23.xy );
                ?>
            
            } // End if tile reduction
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 0 Weight Generation
        ///////////////////////////////////////////////////////////////////////
        
        // Take into account layer 0 color map alpha.
        <?
        weights.x *= layerColor0.a;
        ?>
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 1 Weight Generation
        ///////////////////////////////////////////////////////////////////////
        if ( layerCount >= 2 )
        {
            <?
            // Take into account layer 1 color map alpha.
            weights.y *= layerColor1.a;
            
            // Remove whatever was added from prior layers
            weights.x *= (1.0f - weights.y);
            ?>
            
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Layer 2 Weight Generation
        ///////////////////////////////////////////////////////////////////////
        if ( layerCount >= 3 )
        {
            <?
            // Take into account layer 2 color map alpha.
            weights.z *= layerColor2.a;
            
            // Remove whatever was added from prior layers
            weights.x *= (1.0f - weights.z);
            weights.y *= (1.0f - weights.z);
            ?>
        }
        
        // Construct final color based on final weights
        <?
        finalColor = layerColor0 * weights.x;
        sumWeights = weights.x;
        ?>
        
        if ( layerCount >= 2 )
        {
            <?
            finalColor += layerColor1 * weights.y;
            sumWeights += weights.y;
            ?>
        }
        if ( layerCount >= 3 )
        {
            <?
            finalColor += layerColor2 * weights.z;
            sumWeights += weights.z;
            ?>
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Normal Generation
        ///////////////////////////////////////////////////////////////////////
        <?
        // Extract and weight normal
        float3 twoWeights  = 2.0f * weights;
        float3 terrainNormal = twoWeights.x * sample2D( sTerrainLayerNormal0Tex, sTerrainLayerNormal0, layerCoords01.xy ).rgb - weights.x;
        ?>
        if ( layerCount >= 2 )
            <?terrainNormal += twoWeights.y * sample2D( sTerrainLayerNormal1Tex, sTerrainLayerNormal1, layerCoords01.zw ).rgb - weights.y;?>
        if ( layerCount >= 3 )
            <?terrainNormal += twoWeights.z * sample2D( sTerrainLayerNormal2Tex, sTerrainLayerNormal2, layerCoords23.xy ).rgb - weights.z;?>
            
        <?
        // Transform tangent space normal into world space
        terrainNormal = mul( terrainNormal, tangentToWorld );

        ///////////////////////////////////////////////////////////////////////
        // G-Buffer Output
        ///////////////////////////////////////////////////////////////////////
        
        // Store the normal and diffuse color as RGB and the sum of weights in alpha (for blending)
        data0.rgb = terrainNormal.xyz * 0.5f + 0.5f;
        data0.a   = sumWeights;
        
        data1.rgb = finalColor.rgb * diffuseColor.rgb;
        data1.a   = sumWeights;
        ?>
        
        // Valid shader
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : terrainGBufferPostProcess() (Pixel Shader, Screen Pass)
    // Desc : Updates the g-buffer channels with correct values for lighting 
    //        terrain. This is necessary because of our destination alpha 
    //        blending mode during g-buffer fill passes. 
    //-------------------------------------------------------------------------
    bool terrainGBufferPostProcess( int shadingQuality )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader outputs (2 or 3 g-buffer targets depending on quality)
        if ( shadingQuality == ShadingQuality::Low )
        {
            // 2 g-buffer output
            <?out
                float4 data0 : SV_TARGET0;
                float4 data1 : SV_TARGET1;
            ?>

        } // End if low-quality
        else
        {
            // 3 g-buffer output
            <?out
                float4 data0 : SV_TARGET0;
                float4 data1 : SV_TARGET1;
                float4 data2 : SV_TARGET2;
            ?>

        } // End if high-quality

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Normal.a needs to be overwritten with specular power
        //data0 = float4( 0, 0, 0, MaterialPower / MaxPower );
        data0 = float4( 0, 0, 0, 0.31f );
        ?>
        
        // Low or high quality shading?
        if ( shadingQuality == ShadingQuality::Low )
        {
            // Low quality shading mode packs diffuse.rgb with a specular luminance/mask in diffuse.a
            <?
            // Diffuse.a needs to be overwritten with specular reflectance
            //data1 = float4( 0, 0, 0, LUMINANCE( MaterialSpecular.rgb ) );
            data1 = float4( 0, 0, 0, 0 );
            ?>

        } // End if low quality
        else
        {
            // High quality shading uses separate diffuse+translucence and specular+rim buffers
            <?
            // Diffuse.a needs to be overwritten with translucence/transmission
            data1 = float4( 0, 0, 0, 0 );

            // Specular.rgb needs to be overwritten with specular reflectance Specular.a with rim lighting
            //data2 = float4( MaterialSpecular.rgb, 0 );
            data2 = float4( 0, 0, 0, 0 );
            ?>

        } // End if high quality

        // Valid Shader
        return true;
    }

} // End Class : LandscapeTerrain