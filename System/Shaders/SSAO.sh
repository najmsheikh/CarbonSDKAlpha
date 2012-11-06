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
// Name : SSAO.sh                                                            //
//                                                                           //
// Desc : Internal surface shader which provides support for SSAO.           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "Utilities.shh"

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENTSCRIPT
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return SSAOShader( owner, driver, resources );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : SSAOShader (Class)
// Desc : Material shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for screen 
//        space ambient occlusion
//-----------------------------------------------------------------------------
class SSAOShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;         // The parent application object.
    private RenderDriver@           mDriver;        // Render driver to which this shader is linked.
    private ResourceManager@        mResources;     // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////////////////
    // Custom Constant Buffer Declarations
    ///////////////////////////////////////////////////////////////////////////
	<?cbuffer cbSSAO : register(b12), globaloffset(c180)
		float4 textureSize;
		float4 sampleTextureSize;
		float4 planeBias; 
		float2 sampleScreenToViewScale;
		float2 sampleScreenToViewBias;
		float2 sampleFocalLength;
		float  worldRadius;
		float  intensity;
		float  maxMipLevel;
		float  mipBias;
	?>

	<?cbuffer cbTangentRays : register(b13), globaloffset(c196)
		float3 tangentRays[ 32 ];
	?>

	<?cbuffer cbUpsampleSSAO : register(b13), globaloffset(c196)
		matrix previousViewProjectionMatrix;
		float  reprojectionThrehold;
		float  upsampleDepthThreshold;
	?>

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D sDepth        : register(s0);
        Sampler2D sNormal       : register(s1);
        Sampler2D sRotation     : register(s2);
        Sampler2D sDepthLow     : register(s3);
        Sampler2D sReprojection : register(s4);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : SSAOShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	SSAOShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner     = owner;
        @mDriver    = driver;
        @mResources = resources;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------------
	// Name : drawHemisphereAO 
	// Desc : Screen space hemispheric ambient occlusion. 
	// ToDo : Make sure we dont run AO on sky pixels via a proper clip quad draw
	//-----------------------------------------------------------------------------
	bool drawHemisphereAO( int numSamples, bool centerPackedDepthNormal, bool samplePackedDepthNormal )
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
            float4  color       : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			_cbCamera;
            cbSSAO;
            cbTangentRays;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		float3x3 tangentToViewMatrix;
		float3 Pc, Nc;
		float4 sampleCoord = 0;
		float  centerDepth, aoSum = 0;
		float  radiusSq = worldRadius * worldRadius; 

		// Compute the screen texture coordinates and center on pixel
        float2 texCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;
		?>

		// Get the position and normal for the current pixel
		if ( centerPackedDepthNormal )
		{
			<?
			float4 centerData = sample2D( sDepthTex, sDepth, texCoords );
			centerDepth = decompressF16( centerData.xy );
			Nc = decodeNormal_Lambert( centerData.zw );
			?>
		}
		else
		{
			<?
			centerDepth = decompressF32( sample2D( sDepthTex, sDepth, texCoords ).rgb );
			Nc = sample2D( sNormalTex, sNormal, texCoords ).rgb * 2.0f - 1.0f;
			Nc = normalize( mul( Nc, _viewMatrix ) );
			?>
		}
		<?
		Pc.z  = centerDepth * _cameraRangeScale + _cameraRangeBias;
		Pc.xy = (texCoords  * _cameraScreenToViewScale + _cameraScreenToViewBias) * Pc.z;

		// Compute the texel coordinates in the sample texture for the center
		float2 centerPixelCoords = (Pc.xy * (1.0f / Pc.z) * sampleFocalLength ) * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );

		// Apply a position bias to reduce co-planar shadowing.
		Pc += Nc * lerp( planeBias.x, planeBias.y, saturate( (Pc.z - planeBias.z) / (planeBias.w - planeBias.z) ) );

		// Sample texture for sin/cos of random angles
		float2 randomAngle = sample2D( sRotationTex, sRotation, texCoords * _targetSize.xy * 0.25 ).rg;

		// Compute a randomly rotated tangent to view space matrix
		getTangentFrame( Nc, randomAngle.y, randomAngle.x, tangentToViewMatrix, $true );

		// For each sample
		for ( int i = 0; i < $numSamples; i++ )
		{
			// Transform the tangent space ray to view space
			float3 ray = mul( tangentRays[ i ], tangentToViewMatrix );

			// Compute the offset position
			float3 Ps = Pc + ray * (worldRadius * ( (i + 0.5f) / $numSamples ) );

			// Project to screen space
			sampleCoord.xy = (Ps.xy * (1.0f / Ps.z) * sampleFocalLength ) * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );
			?>
			
			// Compute the z coordinate at this location
			if ( samplePackedDepthNormal )
				<?Ps.z = decompressF16( sample2D( sDepthTex, sDepth, sampleCoord.xy ).rg )  * _cameraRangeScale + _cameraRangeBias;?>
			else
				<?Ps.z = decompressF32( sample2D( sDepthTex, sDepth, sampleCoord.xy ).rgb ) * _cameraRangeScale + _cameraRangeBias;?>

			<?
			// Compute the xy coordinates
			Ps.xy = (sampleCoord.xy * sampleScreenToViewScale + sampleScreenToViewBias) * Ps.z;

			// Get the "shadow" vector
			float3 v = Ps - Pc;

			// Compute a final occlusion value
			float vv = dot( v, v ); 

			// Compute N dot L (unnormalized L)
			float vn = dot( v, Nc );

			// Compute occlusion value
			float f = max( radiusSq - vv, 0 );
			float occ = f * f * f * max( (vn - 0.01) / (0.01 + vv), 0 );

			// Accumulate
			aoSum += occ;

		} // Next sample

		// Normalize
		float temp = radiusSq * worldRadius;
		aoSum /= (temp * temp);

		// Compute final AO value
		float ao = max( 0.0, 1.0 - aoSum * intensity );

		// Return results along with depth
		color = float4( compressF32( centerDepth ), ao );
		?>
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: downsampleSSAO() 
	// Desc: SSAO and depth downsampling filter.
	//-----------------------------------------------------------------------------
	bool downsampleSSAO( bool averageDownsample )
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
			float4  color       : SV_TARGET0;
		?>

		// Constant buffer usage.
		<?cbufferrefs
			_cbCamera;
		?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Compute the screen texture coordinates and center on pixel
		float2 texCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;

		// Sample the four texels from the higher resolution texture
		float4 data0 = sample2D( sDepthTex, sDepth, texCoords + float2( -0.25, -0.25 ) * _targetSize.zw );
		float4 data1 = sample2D( sDepthTex, sDepth, texCoords + float2(  0.25, -0.25 ) * _targetSize.zw );
		float4 data2 = sample2D( sDepthTex, sDepth, texCoords + float2( -0.25,  0.25 ) * _targetSize.zw );
		float4 data3 = sample2D( sDepthTex, sDepth, texCoords + float2(  0.25,  0.25 ) * _targetSize.zw );

		// Decompress the depths
		float2 sample0 = float2( decompressF32( data0.rgb ), data0.a );
		float2 sample1 = float2( decompressF32( data1.rgb ), data1.a );
		float2 sample2 = float2( decompressF32( data2.rgb ), data2.a );
		float2 sample3 = float2( decompressF32( data3.rgb ), data3.a );
		?>
		
		// Use an average or minimum depth downsample
		if ( averageDownsample )
		{
			<?float2 result = (sample0 + sample1 + sample2 + sample3) * 0.25f;?>
		}
		else
		{
			<?
			float2 result = sample0;
			if ( sample1.x < result.x )
				result = sample1;
			if ( sample2.x < result.x )
				result = sample2;
			if ( sample3.x < result.x )
				result = sample3;

			// Based on the selected minimum depth, weight ao averaging based on the relative depths
			float4 depthDeltas = abs( result.xxxx - float4( sample0.x, sample1.x, sample2.x, sample3.x ) );
			float maxDepth = 2.0f / _cameraFar;
			float4 weights = 1.0f - saturate( depthDeltas / maxDepth ); 
			weights /= dot( weights, float4( 1, 1, 1, 1 ) );
			result.y = dot( float4(sample0.y, sample1.y, sample2.y, sample3.y), weights );
			?>
		}
		
		// Recompress the final result and return 
		<?color = float4( compressF32( result.x ), result.y );?>

		// Valid shader
		return true;
	}
	
	//-----------------------------------------------------------------------------
	// Name: upsampleSSAO()
	// Desc: Bilateral resampling filter for SSAO. Uses depths for weight generation
	//       in addition to the bilinear weight.
	//-----------------------------------------------------------------------------
	bool upsampleSSAO( bool useReprojection )
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
			float4  color       : SV_TARGET0;
		?>

		// Constant buffer usage.
		<?cbufferrefs
			_cbCamera;
		?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		float4 subTexelDist, srcDepths, srcAO, sample;

		// Compute the screen texture coordinates and center on pixel
		float2 texCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;

		// Compute the four closest sampling locations in the lower resolution texture (RECHECK THIS!)
		float2 srcCoords[4];
		srcCoords[0] = texCoords + float2( -0.25, -0.25 ) * _targetSize.zw;
		srcCoords[1] = texCoords + float2(  0.25, -0.25 ) * _targetSize.zw;
		srcCoords[2] = texCoords + float2( -0.25,  0.25 ) * _targetSize.zw;
		srcCoords[3] = texCoords + float2(  0.25,  0.25 ) * _targetSize.zw;

		// Compute the bilinear weights for these locations
		subTexelDist.xy = frac( srcCoords[0] * _targetSize.xy * 0.5f );
		subTexelDist.zw = 1.0f - subTexelDist.xy;
		float4 weights  = subTexelDist.zxzx * subTexelDist.wwyy;

		// Sample the destination depth 
		float4 currData = sample2D( sDepthTex, sDepth, texCoords );
		float  dstDepth = decompressF32( currData.rgb );

		// Sample the 2x2 block of neighbors (depth in RGB, SSAO in a)
		for ( int i = 0; i < 4; i++ )
		{
			sample         = sample2D( sDepthLowTex, sDepthLow, srcCoords[ i ] );
			srcDepths[ i ] = decompressF32( sample.rgb );
			srcAO[ i ]     = sample.a;
			
		} // Next sample

		// Complete the weighting computation and factor it in
		float4 depthDeltas = abs( dstDepth.xxxx - srcDepths );

		float  maxDepth       = upsampleDepthThreshold / _cameraFar;
		float4 depthWeights   = 1.0f - saturate( depthDeltas / maxDepth );
		float  depthWeightSum = dot( depthWeights, float4( 1, 1, 1, 1 ) );

		// Normalize weights and compute final weighted average
		if ( depthWeightSum > 0.0f )
		{
			weights *= depthWeights;
			weights *= (1.0f / dot( weights, float4( 1, 1, 1, 1 ) ) );
		}

		// Compute current AO	
		float currentAO = dot( srcAO, weights );
		?>
		
		// If we are doing reprojection, incorporate it now
		if ( useReprojection )
		{
			<?
			// Compute the view space position
			float3 viewPosition;
			viewPosition.z  = dstDepth *  _cameraRangeScale + _cameraRangeBias;
			viewPosition.xy = (texCoords * _cameraScreenToViewScale + _cameraScreenToViewBias) * ViewPosition.z;

			// Project to last frame
			float3 worldPosition     = mul( float4( viewPosition, 1 ), _inverseViewMatrix );
			float4 clipPositionPrev  = mul( float4( worldPosition, 1 ), previousViewProjectionMatrix );
				   clipPositionPrev /= clipPositionPrev.w;

			// Convert to screen coords
			float2 screenCoordsReproj = clipPositionPrev.xy * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );

			// Sample the last frame's results (RGB = depth, A = SSAO)
			float4 lastFrameData  = tex2D( sReprojectionTex, sReprojection, screenCoordsReproj );
			float  lastFrameDepth = decompressF32( lastFrameData.rgb );
			float  lastFrameAO    = lastFrameData.a;
		
			// Compute a weight based on depth differences
			float blendWeight = abs( 1.0f - lastFrameDepth / dstDepth ) < 0.1f; //reprojectionThrehold; 

			// Include a user adjustment
			blendWeight *= 0.85f;

			// Ensure we are not sampling out of bounds
			// if ( clipPositionPrev.x < -1.0f || clipPositionPrev.x > 1.0f || clipPositionPrev.y < -1.0f || clipPositionPrev.y > 1.0f )
			//	blendWeight = 0;

			// Blend the last frame's results into the current 
			currentAO = lerp( currentAO, lastFrameAO, blendWeight );
			?>
		}

		// Return the results (RGB = depth, A = SSAO)
		<?color = float4( currData.rgb, currentAO );?>
		
		// Valid shader
		return true;
	}

} // End Class : SSAO

