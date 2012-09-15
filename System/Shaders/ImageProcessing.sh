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
// Name : ImageProcessing.sh                                                 //
//                                                                           //
// Desc : Internal surface shader which provides support for the various     //
//        image processing and output control operations that may need to be //
//        applied to generated render targets (blur, downsample, etc.).      //
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
    return ImageProcessingShader( owner, driver, resources );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : ImageProcessingShader (Class)
// Desc : Surface shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for image 
//        processing.
//-----------------------------------------------------------------------------
class ImageProcessingShader : ISurfaceShader
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
    <?cbuffer cbImageProcessing : register(b11), globaloffset(c150)
        float4 userColor    = { 0.0, 0.0, 0.0, 0.0 };
        float4 textureSize;     //xy = dimensions, zw = recip dimensions 
    ?>

    <?cbuffer cbColorControls : register(b12), globaloffset(c180)
        float2 blackLevel       = { 0.0, 0.0 };
        float2 whiteLevel       = { 1.0, 1.0 };
        float  inRange          = 1.0;
        float  outRange         = 1.0;
        float  minAdjust        = 1.0;
        float  gamma            = 1.0;
        float3 tint             = { 1.0, 1.0, 1.0 };
        float  brightness       = 0.0;
        float  exposure         = 1.0;
        float  exposureAmount   = 0.0;
        float  saturation       = 1.0;
        float  grainAmount      = 0.0;
        float3 vignette         = { 1.0, 0.0, 1.0 };
    ?>

    <?cbuffer cbBilateral : register(b13), globaloffset(c160)
        float2  depthScaleBias;   
        float2  depthExtents;
        float2  normalScaleBias;  
        float4  normalTestParams; //x = AngleCos MinDist, y = AngleCos MaxDist, zw = lerp scale/bias
    ?>
    
    <?cbuffer cbReprojection : register(b13), globaloffset(c160)
        matrix  reprojectionMatrix;   
        float   depthTolerance;
        float   blendAmount;  
    ?>

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
		// Generic image samplers
        Sampler2D sImage       : register(s0);
        Sampler2D sImageLinear : register(s1);
        Sampler2D sImagePoint  : register(s2);
        Sampler2D sVertical    : register(s1);
        Sampler2D sHorizontal  : register(s2);
        Sampler2D sImageCurr   : register(s0);
        Sampler2D sImagePrev   : register(s1);
        Sampler2D sImage0      : register(s0);
        Sampler2D sImage1      : register(s1);
        Sampler2D sImage2      : register(s2);
        Sampler2D sImage3      : register(s3);
        
		// Post-processing samplers
        Sampler2D sVignette    : register(s3);
        Sampler3D sGrain       : register(s4);
        Sampler1D sRemap       : register(s5);

		// For bilateral filtering/resampling
        Sampler2D sDepthSrc    : register(s6);
        Sampler2D sNormalSrc   : register(s7);
        Sampler2D sDepthDst    : register(s8);
        Sampler2D sNormalDst   : register(s9);
        Sampler2D sEdge        : register(s10);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : ImageProcessingShader() (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	ImageProcessingShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
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
	// Name : transform()
	// Desc : Used when drawing full screen clip quad with eye ray interpolator 
	//-------------------------------------------------------------------------
	bool transform( bool outputEyeRay )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 sourcePosition   : POSITION;
        ?>

        // Define shader outputs.
		<?out
			float4 clipPosition     : SV_POSITION;
			float2 texCoords        : TEXCOORD0;
		?>
		if ( outputEyeRay )
		{
			<?out	
				float3 eyeRay           : TEXCOORD1;
			?>
		}
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		clipPosition.xyz = sourcePosition.xyz;
		clipPosition.w   = 1.0;

		// Get the screen coords into range [0,1]
		texCoords.xy = float2( clipPosition.x, -clipPosition.y ) * 0.5 + 0.5;

		// Adjust screen coords to take into account the optional half pixel offset. 
		texCoords.xy = texCoords.xy + _screenUVAdjustBias;
		?>

		// This ray goes from the camera position to the pixel in the screen
		if ( outputEyeRay )
		{
			<?
			float4 viewPosition = mul( float4( clipPosition.x * _cameraFar, clipPosition.y * _cameraFar, _cameraFar, _cameraFar ), _inverseProjectionMatrix );
			eyeRay = mul( viewPosition.xyz, _inverseViewMatrix );
			?>
		}
		
		// Valid shader
		return true;
	}

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
	//-----------------------------------------------------------------------------
	// Name : drawImage()
	// Desc : Various image processing operations pixel shader
	//-----------------------------------------------------------------------------
    bool drawImage( int imageOperation )
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
            cbImageProcessing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        // Initialize to 0
        color = 0;
        ?>
        if ( imageOperation == ImageOperation::SetColorRGB || imageOperation == ImageOperation::SetColorRGBA || imageOperation == ImageOperation::SetColorA ||
             imageOperation == ImageOperation::ScaleUserColorRGBA || imageOperation == ImageOperation::ScaleUserColorRGB || imageOperation == ImageOperation::ScaleUserColorA )
        {
			<?color = userColor;?>        
        }
        else
        {
			// Sample the image
			<?float4 sample = sample2D( sImageTex, sImage, texCoords );?>

			// Run the operation
			runOperation( "color", "sample", imageOperation );
		}
		
        // Valid shader
        return true;
    }

	//-----------------------------------------------------------------------------
	// Name : drawImageMultiOp()
	// Desc : Various image processing operations pixel shader
	//-----------------------------------------------------------------------------
    bool drawImageMultiOp( const int[] & operations )
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
            cbImageProcessing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

		// Sample the image 
		<?float4 sample = sample2D( sImageTex, sImage, texCoords );?>

		// Run the list of operations
	    runOperations( "color", "sample", operations );
		
        // Valid shader
        return true;
    }

	//-----------------------------------------------------------------------------
	// Name : drawDepthImage()
	// Desc : Various depth texture conversion operations pixel shader
	//-----------------------------------------------------------------------------
    bool drawDepthImage( int inputType, int outputType, bool outputDepth )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
			float3  eyeRay         : TEXCOORD1;
        ?>

        // Define shader outputs.
        <?out
            float4 color        : SV_TARGET0;
        ?>
		if ( outputDepth )
		{
			<?out
				float depth     : SV_DEPTH;
			?>
		}

        // Constant buffer usage.
        <?cbufferrefs
			_cbCamera;
            cbImageProcessing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		float depthOut = 0, alphaOut = 0;

		// Renormalize the eye ray
		eyeRay = normalize( eyeRay );

		// Sample the depth image
		float4 sample = sample2D( sImageTex, sImage, texCoords );
		
		color.rgb = 0;
		color.a   = sample.a;
		?>
	
		// If we are outputting to a depth buffer, don't compress the output until after
		// we've done the non-linear z conversion
		if ( outputDepth )
		{
			// Find out the uncompressed output type
			int outputTypePure = getPureDepthType( outputType );
		
			// First convert the depth format as needed
            <?
			depthOut = convertDepthType( sample.rgb, eyeRay, $inputType, $outputTypePure, $true, $true );
			
			// Next convert to a non-linear Z depth (for the depth buffer output)
			depth = convertDepthType( depthOut, eyeRay, $outputTypePure, ${DepthType::NonLinearZ}, $true, $true );

			// Finally call the conversion function one last time to pack the output into our color
			color.rgb = convertDepthType( depthOut, eyeRay, $outputTypePure, $outputType, $true, $true );
            ?>
		}
		else
		{
			// Convert/compress the depth format as needed
			<?color.rgb = convertDepthType( sample.rgb, eyeRay, $inputType, $outputType, $true, $true );?>
		}
						
        // Valid shader
        return true;
    }

	//-----------------------------------------------------------------------------
	// Name : downSampleDepth()
	// Desc : Depth down sampling operations pixel shader (does avg, min, or max).
	//        Optionally allows for simultaneous second buffer downsampling based on 
	//        results of depth operation as well as possible depth output to a hardware
	//        z-buffer. Also allows the user to keep track of which texel from the
	//        higher resolution texture was selected in a min or max operation to make
	//        position reconstruction more accurate. 
	//-----------------------------------------------------------------------------
    bool downSampleDepth( int inputType, int outputType, int kernelSize, int operation, bool recordSampleOffset, bool outputData )
    {
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
			float3  eyeRay         : TEXCOORD1;
        ?>

        // Define shader outputs.
        <?out
            float4  color       : SV_TARGET0;
        ?>
		if ( outputData )
		{
			<?out
				float4 data     : SV_TARGET1;
			?>
		}

        // Constant buffer usage.
        <?cbufferrefs
			_cbCamera;
            cbImageProcessing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
        float4 sample, sampleOut, subTexelDist, bilinearWeights, depths, depthSamples[ 4 ];
        float  sampleDepth, depthOut, alphaOut = 0;
        float2 srcCoords[ 4 ], sampleCoords, dataCoords = texCoords;

        // Offset Mask           0,0   1,0  0,1  1,1
        float offsetID[ 4 ] = { -1.0, -0.5, 0.5, 1.0 };
        float currentMask = 0;
		?>

		// Determine the compression status of the i/o  
		bool inputCompressed  = isCompressedDepthType( inputType );
		bool outputCompressed = isCompressedDepthType( outputType );

		// Should we try to preserve as much precision as possible by not decompressing more than we need?
		bool preservePrecision = (inputCompressed && outputCompressed && (inputType == outputType) && (operation != ImageOperation::DownSampleAverage));
						
		<?
		// Compute the four closest sampling locations in the lower resolution texture
		srcCoords[0] = texCoords + float2( -0.5, -0.5 ) * textureSize.zw;
		srcCoords[1] = texCoords + float2(  0.5, -0.5 ) * textureSize.zw;
		srcCoords[2] = texCoords + float2( -0.5,  0.5 ) * textureSize.zw;
		srcCoords[3] = texCoords + float2(  0.5,  0.5 ) * textureSize.zw;
		?>
		
		// Get the depth samples, decompressing as needed
		for ( int i = 0; i < 4; i++ )
		{
			<?sample = sample2D( sDepthSrcTex, sDepthSrc, srcCoords[ $i ] );?>
			if ( inputCompressed )
				<?sampleDepth = decompressF32( sample.rgb );?>
			else
				<?sampleDepth = sample.r;?>
			
            <?
            depthSamples[ $i ] = sample;
            depths[ $i ] = sampleDepth;
			?>
		}		
		
		// If we are averaging, use bilinear weighting...
		if ( operation == ImageOperation::DownSampleAverage )
		{
			<?
			subTexelDist.xy = frac( srcCoords[0] * textureSize.xy );
			subTexelDist.zw = 1.0f - subTexelDist.xy;
			bilinearWeights = subTexelDist.zxzx * subTexelDist.wwyy;
			depthOut        = dot( depths, bilinearWeights );
			?>
		}
		else // Extents computations
		{
			for ( int i = 0; i < 4; i++ )
			{
				// No need to do any comparisons on the first sample
				if ( i == 0 )
				{
					<?
					depthOut    = depths[ $i ];
					sampleOut   = depthSamples[ $i ];
					dataCoords  = srcCoords[ $i ];
					alphaOut    = offsetID[ $i ];
					?>
					continue;
				}
				
				// If we need to do a proper conditional test					
				if ( preservePrecision || outputData || recordSampleOffset )
				{
					if ( operation == ImageOperation::DownSampleMinimum )
						<?if ( depths[ $i ] < depthOut )?>
					else	
						<?if ( depths[ $i ] > depthOut )?>
					<?
					{ 
					?>
						// Record the current depth
						<?depthOut = depths[ $i ];?>	

						if ( preservePrecision )
							<?sampleOut = depthSamples[ $i ];?>
					 
						if ( outputData )	
							<?dataCoords = srcCoords[ $i ];?>
						
						if ( recordSampleOffset )	
							<?alphaOut = offsetID[ $i ];?>
					<?
					} // End conditional test
					?>
				
				} // End if using conditional
				else
				{
					// Standard comparisons will be faster
					if ( operation == ImageOperation::DownSampleMinimum )
						<?depthOut = min( depths[ $i ], depthOut );?>	
					else	
						<?depthOut = max( depths[ $i ], depthOut );?>	
				
				} // End if using comparisons
				
			} // Next sample

		} // End if doing extents testing

		// If the source and dest are both packed, we minimize precision loss by 
		// returning the selected value rather than the unpacked one (for min/max cases)
		if ( preservePrecision )
			<?color = sampleOut;?>
		else
			<?color.rgb = convertDepthType( depthOut, eyeRay, ${getPureDepthType( inputType )}, $outputType, $true, $true );?>

		// Sample the data buffer based on the best coordinate found (ensures match at pixel or hw bilinear blend if averaging)
		if ( outputData )
			<?data = sample2D( sImageTex, sImage, dataCoords );?>
		
		// Copy the sample offset mask
		if ( recordSampleOffset )
			<?color.a = alphaOut * 0.5f + 0.5f;?>
		else
			<?color.a = alphaOut;?>

        // Valid shader
        return true;
    }

	//-----------------------------------------------------------------------------
	// Name : downSampleOperation()
	// Desc : Runs the specified downsampling operation on the list of samples gathered by the caller.
   	//-----------------------------------------------------------------------------
	void downSampleOperation( int numSamples, int operation, bool bilinear, bool alphaWeighted, bool alphaWeightedBinary, bool binaryAlphaOutput )
	{
		// This call assumes the "samples" array of numSamples size has already been created and filled by the caller shader.

		bool computeMin = ( operation == ImageOperation::DownSampleMinimum || operation == ImageOperation::DownSampleMinMax );
		bool computeMax = ( operation == ImageOperation::DownSampleMaximum || operation == ImageOperation::DownSampleMinMax );

		<?
		float4 sample, currentColor;
		float  weight, totalWeight = 0;
		?>
				
		// Fast option for bilinear average downsampling with no alpha weighting
		if ( operation == ImageOperation::DownSampleAverage && bilinear && !alphaWeighted && !alphaWeightedBinary )
		{
			<?color = samples[0];?>

        } // End if simple downsample
		else
		{
			// Initialize our starting value
			if ( operation == ImageOperation::DownSampleAverage )			
				<?currentColor = 0;?>
			else
				<?currentColor = float4( 99999999.0, -99999999.0, 0, 0 );?>
		
			// Iterate our samples
			for ( int i = 0; i < numSamples; i++ )
			{
				// Retrieve the sample from the array
				<?sample = samples[ $i ];?>
				
				// If we are weighting the sample's color by its alpha, we can optionally force a binary weight
				if ( alphaWeightedBinary )
				{
					<?
					weight       = sample.a > 0.0f;
					sample      *= weight;
					totalWeight += weight;
					?>
				}	
				else if ( alphaWeighted )
				{
					// Weight the sample's color by its current alpha
					<?sample.rgb *= sample.a;?>
				} 
				
				// If we are downsampling the minimum value...
				if ( computeMin )
					<?currentColor.r = min( sample.r, currentColor.r );?>

				// If we are downsampling the maximum value...
				if ( computeMax )
					<?currentColor.g = max( sample.g, currentColor.g );?>

				// For averaging, just add the sample to the current color
				if ( operation == ImageOperation::DownSampleAverage )
					<?currentColor += sample;?>

            } // Next sample
			
			// Normalize the final results if we are averaging
			if ( operation == ImageOperation::DownSampleAverage )
			{
				if ( alphaWeightedBinary )
				{
					<?color = currentColor * ( 1.0 / (totalWeight + 1e-6f) );?>
				}	
				else if ( alphaWeighted )
				{
					<?
					color.rgb = currentColor.rgb * ( 1.0 / (currentColor.a + 1e-6f) );
					color.a   = currentColor.a   * ( 1.0 / $numSamples );
					?>
				}
				else
				{
					<?color = currentColor * (1.0 / $numSamples);?>
				}	
			
            } // End if DownSampleAverage
			else
			{
				<?color = currentColor;?>

			} // End if !DownSampleAverage

		} // End if generic downsample
		
		// If a binary output alpha is desired, set it		
		if ( binaryAlphaOutput )
			<?color.a = totalWeight > 0.0f;?>
	}


	//-----------------------------------------------------------------------------
	// Name : downSample()
	// Desc : NxN down sampling using manual averaging or hw bilinear filtering
	// Note : N (i.e., nDownSampleKernelSize) must be a power of 2
    // Note : If alpha weighting is required, point sampling is preferable although
    //        bilinear sampling is still available as a faster but less accurate method.
    //        For extents downsampling, bilinear sampling should be turned off.
   	//-----------------------------------------------------------------------------
	bool downSample( int kernelSize, int operation, bool bilinear, bool oddU, bool oddV, bool alphaWeighted, bool alphaWeightedBinary, bool binaryAlphaOutput )
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
			cbImageProcessing;
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////

		// Compute texture coords
        <?float2 texCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;?>
	
		// If needed, adjust the tex coords for odd size downsampling (place in upper left texel center)
		if ( oddU || oddV )
			<?texCoords = ( floor(texCoords * textureSize.xy) - float2(0.5, 0.5) ) * textureSize.zw;?> 
	
		// Initialize the kernel (uv offsets and weights)
		int numSamples = initializeDownSampleKernel( kernelSize, bilinear, oddU, oddV );

		// Declare our samples array
		<?float4 samples[ $numSamples ];?>
		
		// Iterate our samples
		for ( int i = 0; i < numSamples; i++ )
		{
			<?samples[$i] = sample2D( sImageTex, sImage, texCoords + downSampleKernel[$i] * textureSize.zw );?>

		} // End if generic downsample
		
		// Run the required operation(s) on the samples
		downSampleOperation( numSamples, operation, bilinear, alphaWeighted, alphaWeightedBinary, binaryAlphaOutput );
		
		// Valid shader
		return true;
	}

	__shadercall float computeAlphaWeight( float sampleAlpha, float centerAlpha, script int weightType )
	{
		switch( weightType )
		{
			case AlphaWeightMethod::SetOne:
				<?return 1.0f;?> 
			break;
			case AlphaWeightMethod::Sample:
			case AlphaWeightMethod::AvgSample:
				<?return sampleAlpha;?> 
			break;
			case AlphaWeightMethod::SamplePow2:
				<?return sampleAlpha * sampleAlpha;?> 
			break;
			case AlphaWeightMethod::SamplePow3:
				<?return sampleAlpha * sampleAlpha * sampleAlpha;?> 
			break;
			case AlphaWeightMethod::SamplePow4:
				<?return sampleAlpha * sampleAlpha * sampleAlpha * sampleAlpha;?> 
			break;
			case AlphaWeightMethod::Center:
				<?return centerAlpha;?> 
			break;
			case AlphaWeightMethod::SampleCenter:
			case AlphaWeightMethod::AvgSampleCenter:
				<?return sampleAlpha * centerAlpha;?> 
			break;
			case AlphaWeightMethod::BinarySample:
				<?return sampleAlpha > 0.0f;?> 
			break;
			case AlphaWeightMethod::BinaryCenter:
				<?return centerAlpha > 0.0f;?> 
			break;
			case AlphaWeightMethod::BinarySampleCenter:
				<?return (sampleAlpha * centerAlpha) > 0.0f;?> 
			break;
			case AlphaWeightMethod::CmpGreater:
				<?return sampleAlpha > centerAlpha;?> 
			break;
			case AlphaWeightMethod::CmpGreaterEqual:
				<?return sampleAlpha >= centerAlpha;?> 
			break;
			case AlphaWeightMethod::CmpLess:
				<?return sampleAlpha < centerAlpha;?> 
			break;
			case AlphaWeightMethod::CmpLessEqual:
				<?return sampleAlpha <= centerAlpha;?> 
			break;
			case AlphaWeightMethod::CmpMin:
				<?return min( sampleAlpha, centerAlpha );?> 
			break;
			case AlphaWeightMethod::CmpMax:
				<?return max( sampleAlpha, centerAlpha );?> 
			break;
			case AlphaWeightMethod::Bilateral:
				<?return 1.0f - abs( sampleAlpha - centerAlpha );?> 
			break;
			case AlphaWeightMethod::Bilateral_Pow2:
				<?
				float t = 1.0f - abs( sampleAlpha - centerAlpha );
				return t * t;
				?> 
			break;
			case AlphaWeightMethod::Bilateral_Pow3:
				<?
				float t = 1.0f - abs( sampleAlpha - centerAlpha );
				return t * t * t;
				?> 
			break;
			default:
				<?return 0;?> 
			break;
		} 
	}

	//-----------------------------------------------------------------------------
	// Name: blur()
	// Desc: NxN box blur filter. Accomodates both U and V passes with optional
	//       alpha channel weighting and optional bilinear sampling for performance. 
	// Note: if distanceFactor > 0 -> sigma value for gaussian blurring.
	//       if distanceFactor = 0 -> all samples equally weighted
	//       if distanceFactor < 0 -> samples are weighted as pow( 1 / (pixel index + 1), abs(distanceFactor) )
	// Note: Recommended that bilinear sampling be disabled when alpha weighting is on.
	// ToDo: More comprehensize input/output alpha weighting logic.
	//-----------------------------------------------------------------------------
	bool blur( int pixelRadius, float distanceFactor, bool vertical, bool linearSampling, int inputAlphaWeighting, int outputAlphaWeighting )
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
			cbImageProcessing;
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float4 sample, currentColor = 0;
		float  centerAlpha, alphaWeight;
		?>
		// Initialize kernel and get number of sampling iterations
		int numSamples = initializeKernel( pixelRadius, distanceFactor, linearSampling, vertical );

		<?float2 stepSize = textureSize.zw;?>
				
		// If we are using alpha channel weighting...
		if ( inputAlphaWeighting != AlphaWeightMethod::None )
		{		
			<?float weightAlphaSum = 0;?>
		
			// Retrieve our samples and weights, keeping track of total weight (Note: separate weights needed for alpha and color)		
			for ( int i = 0; i < numSamples; i++ )
			{
				if ( vertical )
					<?sample = sample2D( sVerticalTex, sVertical, texCoords + filterKernel[ $i ].xy * stepSize );?>
				else
					<?sample = sample2D( sHorizontalTex, sHorizontal, texCoords + filterKernel[ $i ].xy * stepSize );?>
				
				// If this is the center sample
				if ( i == 0 )
				{
					// Keep track of the center pixel alpha
					<?centerAlpha = sample.a;?>
					
					// If we are using center weighting, only non-center samples should be affected by it
					if ( inputAlphaWeighting == AlphaWeightMethod::Center || inputAlphaWeighting == AlphaWeightMethod::SampleCenter )
						<?alphaWeight = 1.0f;?>
					else
						<?alphaWeight = computeAlphaWeight( sample.a, centerAlpha, $inputAlphaWeighting );?>
				}
				else
				{
					// Compute the alpha weight for this sample
					<?alphaWeight = computeAlphaWeight( sample.a, centerAlpha, $inputAlphaWeighting );?>
				}
				
				<?
				// Adjust by distance weight
				sample.a = alphaWeight * filterKernel[ $i ].z;
				
				// Weight the color
				sample.rgb *= sample.a;
				
				// Accumulate
				currentColor += sample;
				?>

				if ( outputAlphaWeighting != AlphaWeightMethod::Center )
					<?weightAlphaSum += filterKernel[ $i ].z;?>

            } // Next sample
					
			// Normalize the results
			<?currentColor.rgb /= (currentColor.a + 1e-6f);?>
			
			if ( outputAlphaWeighting == AlphaWeightMethod::Center )
				<?currentColor.a = centerAlpha;?>
			else
				<?currentColor.a /= (weightAlphaSum + 1e-6f);?>

        } // End if alphaWeighted
		else
		{
			for ( int i = 0; i < numSamples; i++ )
			{
				if ( vertical )
					<?currentColor += sample2D( sVerticalTex, sVertical, texCoords + filterKernel[ $i ].xy * stepSize ) * filterKernel[ $i ].z;?>
				else
					<?currentColor += sample2D( sHorizontalTex, sHorizontal, texCoords + filterKernel[ $i ].xy * stepSize ) * filterKernel[ $i ].z;?>

				if ( i == 0 )
					<?centerAlpha = currentColor.a;?>

            } // Next sample

			// Set final alpha if needed
			if ( outputAlphaWeighting != AlphaWeightMethod::None )
				<?currentColor.a = computeAlphaWeight( currentColor.a, centerAlpha, $outputAlphaWeighting );?>

		} // End if !alphaWeighted
		
		// Return results
		<?color = currentColor;?>

		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: bilateralBlur()
	// Desc: NxN Box or Gaussian blur filter with bilateral support.
	//-----------------------------------------------------------------------------
	bool bilateralBlur( int pixelRadius, float distanceFactor, float kernelAdjust, bool vertical, bool cmpDepth, bool cmpNormal, int depthType, bool depthAlphaAsColor, bool alphaWeighted, float worldRadius, float anisotropy )
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
			cbImageProcessing;
			cbBilateral;
			_cbCamera
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float4 origDepth, sample, currentColor = 0;
		float3 position;
		float2 depthData, sampleCoords, sampleStep;
		float  deltaD, weight, totalWeightColor = 0, totalWeightAlpha = 0;
		half3  normalRef, normalSmp;
		float  depthRef, depthSmp;

		const float stepSizeMax = 0.5f;
		?>

		bool sampledDepth, sampledNormal, sampledColor;

		// Initialize kernel and get number of sampling iterations
		int numSamples = initializeKernel( pixelRadius, distanceFactor, false, vertical );

		// Retrieve reference pixel data
		if ( depthType == DepthType::LinearZ_Normal_Packed )
		{
			<?sampleDepthNormalBufferFast( depthRef, normalRef, sDepthSrcTex, sDepthSrc, texCoords );?>
		}
		else
		{
			// If we are using depth alpha as color, preserve the original depth (in RGB) for better precision
			if ( depthAlphaAsColor )
			{
				if ( vertical )
					<?origDepth = sample2D( sVerticalTex, sVertical, texCoords );?>
				else
					<?origDepth = sample2D( sHorizontalTex, sHorizontal, texCoords );?>
				
				<?
                // Unpack the depth
				depthData.x = convertDepthType( origDepth.rgb, float3(0,0,0), ${DepthType::LinearZ_Packed}, ${DepthType::LinearZ}, $true, $false );
				
				// Copy the alpha value over as color data
				depthData.y = origDepth.a;
                ?>
			}
			else
			{
				sampleDepthBuffer( "depthData", "sDepthSrcTex", "sDepthSrc", "texCoords", true, depthType );
			}
			
            <?
            depthRef  = depthData.x;
			normalRef = normalize( sample2D( sNormalSrcTex, sNormalSrc, texCoords ).rgb * 2.0 - 1.0 );
            ?>
		}
		
		// Assume an initial sampling step size of 1 texel 
		<?sampleStep = textureSize.zw;?>

		// If a world radius was provided
		if ( worldRadius > 0.0f )
		{
			// Compute focal length
			<?float2 focalLength = float2( _projectionMatrix._11, _projectionMatrix._22 );?>
			
			// Compute screen space radius
			<?float2 radius = $worldRadius * focalLength * (1.0f / depthRef) * float2( 0.5f, 0.5f );?>

			// Since our kernel offsets assume pixel sized steps (e.g., -2, -1, 0, 1, 2), include a normalization factor
			float recipPixelRadius = 1.0f / float(pixelRadius);
			<?sampleStep = radius * $recipPixelRadius;?>
		}
		else if ( kernelAdjust > 0.0f )
		{
			// Do a distance based adjustment to the sample step size
			//<?sampleStep *= ( 1.0f / ( (depthRef) * $kernelAdjust ) );?>
			<?sampleStep = lerp( sampleStep * 0.1, sampleStep, saturate( 1.0f / ( depthRef * $kernelAdjust ) ) );?>
		}

		// Optionally simulate anisotropy by adjusting the step size according to viewing angle
		if ( anisotropy > 0.0f )
		{
            <?
			// Approximate anisotropy by scaling the radius according to view and surface angle
			position = getViewPosition( texCoords, depthRef, $false ); //TODO: Add orthographic camera support.
			
			// Get the camera space direction vector (surface to camera)
			half3 V = -normalize( position );
			
			// Get the dot product with the surface normal
			float anisotropy = pow( saturate( dot( V, normalRef ) ), $anisotropy );
			
			// Adjust the screen radius by viewing angle according to surface angle
			sampleStep = lerp( sampleStep, sampleStep * anisotropy, abs( normalRef.xy ) ); 			
			?>
		}

		// Clamp to ensure we don't sample beyond x% of the screen when using a world radius
		if ( worldRadius > 0.0f )
			<?sampleStep = min( stepSizeMax, sampleStep );?>

		// Retrieve our samples and weights, keeping track of total weight		
		for ( int i = 0; i < numSamples; i++ )
		{
			// Reset sampling status
			sampledDepth  = false;
			sampledNormal = false;
			sampledColor  = false;

			// Compute the texture coordinates for this sample
			<?sampleCoords = texCoords + filterKernel[ $i ].xy * sampleStep;?>

			// Did we pre-pack depth and normal into a single texture?
			if ( depthType == DepthType::LinearZ_Normal_Packed )
			{
				// We already sampled the depth buffer above, so only sample here after the first iteration (i.e., offset = 0)
				if ( i > 0 )
				{
					// Unpack depth and normal
					<?sampleDepthNormalBufferFast( depthSmp, normalSmp, sDepthSrcTex, sDepthSrc, sampleCoords );?>
				}
				else
				{
					<?
					depthSmp  = depthRef;
					normalSmp = normalRef;
					?>
				}
				
				// We now have access to the depth and normal
				sampledDepth  = true;
				sampledNormal = true;
			}
			// If the depth buffer (alpha) is also the color buffer
			else if ( depthAlphaAsColor )
			{
				// We already sampled the depth buffer above, so only sample here after the first iteration (i.e., offset = 0)
				if ( i > 0 )
				{
					if ( vertical )
						sampleDepthBuffer( "depthData", "sVerticalTex",   "sVertical",   "sampleCoords", true, depthType );
					else
						sampleDepthBuffer( "depthData", "sHorizontalTex", "sHorizontal", "sampleCoords", true, depthType );
				}
				<?
				depthSmp = depthData.x;
				sample   = depthData.y;
				?>

				// We now have access to the depth and color
				sampledDepth = true;
				sampledColor = true;
			}
						
			// Retrieve the low-res color sample if needed
			if ( !sampledColor )
			{
				// Get the current color sample
				if ( vertical )
					<?sample = sample2D( sVerticalTex,   sVertical,   sampleCoords );?>
				else
					<?sample = sample2D( sHorizontalTex, sHorizontal, sampleCoords );?>
			}	

			// Get the precomputed filter weight
			<?weight = filterKernel[ $i ].z;?>

			// Skip comparisons on index 0 (i.e., the central sample) as geometric weight will always be 1.
			if ( i > 0 )
			{
				// Adjust weight based on relative distance
				if ( cmpDepth )
				{
					if ( !sampledDepth )
						sampleDepthBuffer( "depthSmp", "sDepthSrcTex", "sDepthSrc", "sampleCoords", false, depthType );

					//<?weight *= 1.0f - saturate( abs( depthRef - depthSmp ) * depthScaleBias.x + depthScaleBias.y );?>
					<?weight *= abs( depthRef - depthSmp ) < depthExtents.y;?>
				}
				
				// Adjust weight based on relative orientation
				if ( cmpNormal )
				{
					if ( !sampledNormal )
						<?normalSmp = normalize( sample2D( sNormalSrcTex, sNormalSrc, sampleCoords ).rgb * 2.0 - 1.0 );?>

					// Square the cosines to better approximate a linear falloff
					<?
					deltaD  = saturate( dot( normalRef, normalSmp ) );
					deltaD *= deltaD;
					weight *= saturate( deltaD * normalScaleBias.x + normalScaleBias.y );
					?>
				}	
			}

			// If we are using additional alpha channel weighting...
			if ( alphaWeighted )
			{
				<?
				// Accumulate non-adjusted weight for later alpha normalization
				totalWeightAlpha += weight;
				
				// Scale the weight by the current alpha weight 
				weight *= sample.a;

				// Adjust color by total weight (alpha & standard)
				sample.rgb *= weight;
				
				// Alpha is now just the combined weight
				sample.a = weight;

				// Accumulate the weighted sample
				currentColor += sample;
				?>
			}
			else
			{
				// Accumulate the weighted sample
				<?currentColor += sample * weight;?>
			}

			// Accumulate the combined weight for color normalization
			<?totalWeightColor += weight;?>

		} // Next sample

		// Normalize the results as required
		if ( alphaWeighted )
		{
			<?
			color.rgb = currentColor.rgb / (totalWeightColor + 1e-6f);
			color.a   = currentColor.a   / (totalWeightAlpha + 1e-6f);
			?>
		}
		else
		{
			<?color = currentColor / (totalWeightColor + 1e-6f);?>
		}	
		
		// If depth alpha is color, we want to return the original reference depth in rgb
		if ( depthAlphaAsColor )
			<?color.rgb = origDepth.rgb;?>

		// Valid shader
		return true;
	}

    //-----------------------------------------------------------------------------
	// Name: bilateralResample() (Function)
	// Desc: Bilateral resampling filter. Can use depth and/or normal for weight generation
	//       in addition to the bilinear weight.
	//-----------------------------------------------------------------------------
	__shadercall float4 bilateralResample( float depthDst, half3 normalDst, float2 texCoords, script bool cmpDepth, script bool cmpNormal, script int depthTypeSrc, script bool depthAlphaAsColor, script bool clipFailed, script bool useLOD  )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Constant buffer usage.
		<?cbufferrefs
			cbImageProcessing;
			cbBilateral;
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float2  depthData, sampleCoords, srcCoords[4];
		float4  subTexelDist, srcDepths, depthDeltas, bilinearWeights, normalWeights, depthSample, colorSample[4], finalColor = 0;
		half3   normalSrc;
		?>
		
		// Are we working with a packed depth & normal texture?
		bool packedDepthNormal = (depthTypeSrc == DepthType::LinearZ_Normal_Packed) ? true : false;

		<?
		// Compute the four closest sampling locations in the lower resolution texture
		srcCoords[0] = texCoords + float2( -0.5, -0.5 ) * textureSize.zw;
		srcCoords[1] = texCoords + float2(  0.5, -0.5 ) * textureSize.zw;
		srcCoords[2] = texCoords + float2( -0.5,  0.5 ) * textureSize.zw;
		srcCoords[3] = texCoords + float2(  0.5,  0.5 ) * textureSize.zw;

		// Compute the bilinear weights for these locations
		subTexelDist.xy = frac( srcCoords[0] * textureSize.xy );
		subTexelDist.zw = 1.0f - subTexelDist.xy;
		bilinearWeights = subTexelDist.zxzx * subTexelDist.wwyy;
		?>

		// Resample from the 2x2 block of neighbors
		for ( int i = 0; i < 4; i++ )
		{
			// Select source texture coordinates
			<?sampleCoords = srcCoords[ $i ];?>

			// Get geometric data
			if ( packedDepthNormal )
			{
				// Unpack depth and normal
				if ( useLOD )
					<?sampleDepthNormalBufferLevel( srcDepths[ $i ], normalSrc, sDepthSrcTex, sDepthSrc, sampleCoords, 0 );?>
				else	
					<?sampleDepthNormalBuffer( srcDepths[ $i ], normalSrc, sDepthSrcTex, sDepthSrc, sampleCoords );?>
			}
			else if ( depthAlphaAsColor )
			{
				// Is our color packed into the depth alpha channel?
				if ( useLOD )
					sampleDepthBufferLevel( "depthData", "sImageTex", "sImage", "sampleCoords", "0", true, depthTypeSrc );
				else
					sampleDepthBuffer( "depthData", "sImageTex", "sImage", "sampleCoords", true, depthTypeSrc );
					
				<?
				colorSample[ $i ] = depthData.y;
				srcDepths[ $i ]   = depthData.x;
				?>
			}
			else if ( cmpDepth )
			{
				if ( useLOD )
					sampleDepthBufferLevel( "srcDepths[ $i ]", "sDepthSrcTex", "sDepthSrc", "sampleCoords", "0", false, depthTypeSrc );
				else
					sampleDepthBuffer( "srcDepths[ $i ]", "sDepthSrcTex", "sDepthSrc", "sampleCoords", false, depthTypeSrc );
			}	

			// Sample normals and compute angle cosines
			if ( cmpNormal )
			{
				if ( !packedDepthNormal )
				{
					if ( useLOD )
						<?normalSrc = normalize( sample2DLevel( sNormalSrcTex, sNormalSrc, sampleCoords, 0 ).rgb * 2.0 - 1.0 );?>
					else					
						<?normalSrc = normalize( sample2D( sNormalSrcTex, sNormalSrc, sampleCoords ).rgb * 2.0 - 1.0 );?>
				}	
					
				<?normalWeights[ $i ] = saturate( dot( normalDst, normalSrc ) );?>
			}

			// Retrieve the low-res color sample if needed
			if ( !depthAlphaAsColor )
			{
				if ( useLOD )
					<?colorSample[ $i ] = sample2DLevel( sImageTex, sImage, sampleCoords, 0 );?>
				else	
					<?colorSample[ $i ] = sample2D( sImageTex, sImage, sampleCoords );?>
			}	
			
		} // Next sample

		// If comparing normals, complete the weighting computation and factor it in
		if ( cmpNormal )
		{
			<?
				// Square the cosines to better approximate a linear falloff
				normalWeights   *= normalWeights;
				bilinearWeights *= saturate( normalWeights * normalScaleBias.x + normalScaleBias.y );
			?>
		}
		
		// If comparing depths, complete the weighting computation and factor it in
		if ( cmpDepth )
		{
			<?
				depthDeltas      = abs( depthDst - srcDepths );
				bilinearWeights *= 1.0f - saturate( depthDeltas * depthScaleBias.x + depthScaleBias.y ); 
			?>		
		}

		<?
		// Sum the weights
		float weightSum = dot( bilinearWeights, float4( 1, 1, 1, 1 ) );
		
		// If we failed to any find good values...
		if ( weightSum == 0.0f )
		{
		?>
			// One option is to throw away the results (a bilateral blur can fix these cases up later)
			if ( clipFailed )
			{
				<?clip( -1 );?>
			}
			else
			{
				<?
				// If distances are not available, choose the first color
				finalColor = colorSample[ 0 ];
				?>
				// If distances are available, choose the closest color
				if ( cmpDepth )
				{
					<?
					float minDelta = depthDeltas.x;
					if ( depthDeltas.y < minDelta )
					{
						minDelta   = depthDeltas.y;
						finalColor = colorSample[ 1 ];
					}
					if ( depthDeltas.z < minDelta )
					{
						minDelta   = depthDeltas.z;
						finalColor = colorSample[ 2 ];
					}
					if ( depthDeltas.w < minDelta )
					{
						finalColor = colorSample[ 3 ];
					}
					?>
					
				} // End if depth available
			
			} // End if not clipping
		<?
		}
		else // Normalize weights and compute final weighted average
		{
			bilinearWeights *= (1.0f / weightSum);
			finalColor = colorSample[ 0 ] * bilinearWeights.x + 
						 colorSample[ 1 ] * bilinearWeights.y + 
						 colorSample[ 2 ] * bilinearWeights.z + 
						 colorSample[ 3 ] * bilinearWeights.w; 
		}
		
		// Return result	
		return finalColor;
		?>
	}

	//-----------------------------------------------------------------------------
	// Name: resample()
	// Desc: Resampling filter. Can use depth and/or normal for weight generation
	//       in addition to the bilinear weight.
	// Note: When using depthAlphaAsColor, make sure that only alpha color writes are
	//       enabled by the caller or results may not be what you'd want to see.
	//-----------------------------------------------------------------------------
	bool resample( bool cmpDepth, bool cmpNormal, int depthTypeDst, int depthTypeSrc, bool depthAlphaAsColor, bool clipFailed, bool edgeTest )
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
			cbImageProcessing;
			cbBilateral;
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float  depthDst;
		float2 depthData;
		half3  normalDst;
		?>

		// If we are not doing a bilateral resample, just use standard hardware bilinear resampling
		if ( !cmpDepth && !cmpNormal )
		{
			<?color = sample2D( sImageTex, sImage, texCoords );?>
		}
		else
		{
			// If we stored a discontinuity value in normal.a, use it
			if ( edgeTest )
			{
				<?
				// Sample the normal map (dilated edge is in alpha)
				float4 normalMap = sample2D( sNormalDstTex, sNormalDst, texCoords );
				
				// If this is not an edge, just do a bilinear resample. Otherwise do full bilateral test.
				if ( normalMap.a < 1e-5f )
				{
					color = sample2DLevel( sImageLinearTex, sImageLinear, texCoords, 0 );
				}
				else
				{
					// Compute the reference normal
					normalDst = normalize( normalMap.xyz * 2.0 - 1.0 );
				    ?>
					
                    // Compute the reference depth
					sampleDepthBufferLevel( "depthData", "sDepthDstTex", "sDepthDst", "texCoords", "0", true, depthTypeDst );
					
					<?
					depthDst = depthData.x;
					
					// Call the bilateral resampler
					color = bilateralResample( depthDst, normalDst, texCoords, $cmpDepth, $cmpNormal, $depthTypeSrc, $depthAlphaAsColor, $clipFailed, $true );
				}
				?>
			}
			else
			{
				// Retrieve high-res surface data
				if ( depthTypeDst == DepthType::LinearZ_Normal_Packed )
				{
					// Unpack high res depth/normal if needed
					<?sampleDepthNormalBuffer( depthDst, normalDst, sDepthDstTex, sDepthDst, texCoords );?>
				}
				else
				{
					// If the low res buffer is using packed depth/normal, we need to ensure that we use
					// the same inaccurate values for comparison purposes!
					if ( depthTypeSrc == DepthType::LinearZ_Normal_Packed )
					{
						//TODO					
					}
					else
					{
						sampleDepthBuffer( "depthData", "sDepthDstTex", "sDepthDst", "texCoords", true, depthTypeDst );
						<?
						depthDst  = depthData.x;
						?>
						if ( cmpNormal )
							<?normalDst = normalize( sample2D( sNormalDstTex, sNormalDst, texCoords ).rgb * 2.0 - 1.0 );?>
						else	
							<?normalDst = 0;?>
					}
				}
				
				// Call the bilateral resampler
				<?color = bilateralResample( depthDst, normalDst, texCoords, $cmpDepth, $cmpNormal, $depthTypeSrc, $depthAlphaAsColor, $clipFailed, $false );?>
			}
		}

		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: discontinuityDetect()
	// Desc: Discontinuity (edge) filter. Can use depth and/or normal differences to 
	//       find geometric discontinuities in the image.
	//-----------------------------------------------------------------------------
	bool discontinuityDetect( bool cmpDepth, bool cmpNormal, bool preserveDirection, int depthType )
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
			cbImageProcessing;
			cbBilateral;
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float   depthRef,  depthSmp, dp;
		half3   normalRef, normalSmp, normalSample;
		float4  depths, depthWeights = 0, normalWeights = 0;
		float2  depthData, sampleCoords[4];
		float4  depthThreshold = depthExtents.yyyy;
		sampleCoords[0] = texCoords + float2( -1.0,  0.0 ) * textureSize.zw;
		sampleCoords[1] = texCoords + float2(  1.0,  0.0 ) * textureSize.zw;
		sampleCoords[2] = texCoords + float2(  0.0, -1.0 ) * textureSize.zw;
		sampleCoords[3] = texCoords + float2(  0.0,  1.0 ) * textureSize.zw;
		?>
				
		// Retrieve center sample data
		if ( depthType == DepthType::LinearZ_Normal_Packed )
		{
			<?sampleDepthNormalBufferFast( depthRef, normalRef, sNormalSrcTex, sNormalSrc, texCoords );?>
		}
		else
		{
			sampleDepthBuffer( "depthData", "sDepthSrcTex", "sDepthSrc", "texCoords", true, depthType );
			<?
			depthRef  = depthData.x;
			normalRef = normalize( sample2D( sNormalSrcTex, sNormalSrc, texCoords ).rgb * 2.0 - 1.0 );
			?>
		}
				
		// Compute a tolerance adjustment for the depth test based on viewing angle (reduces undersampling problems)
		bool useAnisotropy = true;
		if ( useAnisotropy )
		{
			<?
            float2 corner = -0.5 * textureSize.zw + texCoords;
			half3 v = normalize( -getViewPosition( corner, depthRef, $false ) ); //TODO: Add orthographic camera support
			float  invCosine  = 1.0f - saturate( dot( v, normalRef ) );
			float2 anisotropy = abs( normalRef.xy ) * invCosine;
						
			// Interpolate between min and max thresholds based on anisotropy 
			float2 anisoThreshold = lerp( depthExtents.xx, depthExtents.yy, anisotropy );
			depthThreshold.xy = anisoThreshold.xx; // left and right samples
			depthThreshold.zw = anisoThreshold.yy; // up and down samples
			?>
		}
		
		// Compute a tolerance adjustment for the normal test based on distance
		if ( cmpNormal )
			<?float cosineThreshold = lerp( normalTestParams.x, normalTestParams.y, saturate( depthRef * normalTestParams.z + normalTestParams.w ) );?>
		
		// Sample neighbors
		for ( int i = 0; i < 4; i++ )
		{
			// Did we pre-pack low res depth and normal into a single texture?
			if ( depthType == DepthType::LinearZ_Normal_Packed )
			{
				<?sampleDepthNormalBufferFast( depths[$i], normalSmp, sNormalSrcTex, sNormalSrc, sampleCoords[$i] );?>
			}
			else
			{						
				if ( cmpDepth )
					sampleDepthBuffer( "depths[$i]", "sDepthSrcTex", "sDepthSrc", "sampleCoords[$i]", false, depthType );

				if ( cmpNormal )
					<?normalSmp = normalize( sample2D( sNormalSrcTex, sNormalSrc, sampleCoords[$i] ).rgb * 2.0 - 1.0 );?>
			}	

			if ( cmpNormal )
				<?normalWeights[ $i ] = saturate( dot( normalRef, normalSmp ) );?>
			
		} // Next sample

		// Do our comparisons
		if ( cmpDepth )
			<?depthWeights = abs( depthRef - depths ) > depthThreshold;?> 
		
		if ( cmpNormal )
			<?normalWeights = normalWeights < cosineThreshold;?>

		// Combine our weights
		<?float4 weights = depthWeights + normalWeights;?>

		// Optionally maintain directional discontinuity information
		if ( preserveDirection )
			<?color = weights > 0 ? 1 : 0;?>
		else
			<?color = dot( weights, float4( 1, 1, 1, 1 ) ) > 0 ? 1 : 0;?>
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: discontinuityDetectFast()
	// Desc: Discontinuity (edge) filter. Can use depth and/or normal differences to 
	//       find geometric discontinuities in the image. This method uses a simpler
	//       averaging technique that does not account for direction of the edge.
	// Note: This method cannot preserve edge directions
	//-----------------------------------------------------------------------------
	bool discontinuityDetectFast( bool cmpDepth, bool cmpNormal, bool preserveDirection, int depthType )
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
			cbImageProcessing;
			cbBilateral;
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float4  depths, normalWeights, packedData;
		float   depthRef, dp;
		half3   normalRef, normalSmp, normals[4], normalAvg = 0;
		float   isEdge = 0;
		float2  depthUnpack = float2( _cameraFar, _cameraFar / 255.0f );
		float2  sampleCoords[4];
		sampleCoords[0] = texCoords + float2( -1.0,  0.0 ) * textureSize.zw;
		sampleCoords[1] = texCoords + float2(  1.0,  0.0 ) * textureSize.zw;
		sampleCoords[2] = texCoords + float2(  0.0, -1.0 ) * textureSize.zw;
		sampleCoords[3] = texCoords + float2(  0.0,  1.0 ) * textureSize.zw;
		?>

		// Retrieve samples
		if ( depthType == DepthType::LinearZ_Normal_Packed )
		{
			// Reference
			<?
			packedData = sample2D( sNormalSrcTex, sNormalSrc, texCoords );
			normalRef  = normalize( packedData.xyy * half3( 2, 2, 0 ) - half3( 1, 1, -0.01 ) );	
			depthRef   = dot( packedData.zw, depthUnpack );
			?>			
			
			// Neighbors
			for ( int i = 0; i < 4; i++ )
			{
				<?
				packedData    = sample2D( sNormalSrcTex, sNormalSrc, sampleCoords[$i] );
				depths[$i]    = dot( packedData.zw, depthUnpack );
				normalSmp     = normalize( packedData.xyy * half3( 2, 2, 0 ) - half3( 1, 1, -0.01 ) );
				normalWeights[$i] = saturate( dot( normalSmp, normalRef ) );
				?>
			}
			
			// Compare normal
			<?isEdge += dot( normalWeights, 1.0 ) < normalTestParams.x;?>
		}
		else
		{
			// Reference 
			if ( cmpDepth )
				sampleDepthBuffer( "depthRef", "sDepthSrcTex", "sDepthSrc", "texCoords", false, depthType );

			if ( cmpNormal )
				<?normalRef = normalize( sample2D( sNormalSrcTex, sNormalSrc, texCoords ).rgb * 2.0 - 1.0 );?>

			// Neighbors
			for ( int i = 0; i < 4; i++ )
			{
				if ( cmpDepth )
					sampleDepthBuffer( "depths[$i]", "sDepthSrcTex", "sDepthSrc", "sampleCoords[$i]", false, depthType );

				if ( cmpNormal )
					<?normalAvg += sample2D( sNormalSrcTex, sNormalSrc, sampleCoords[$i] ).rgb * 0.25;?>
			}

			// Compare normal
			if ( cmpNormal )
			{
				<?
				normalAvg = normalize( 2.0f * normalAvg - 1.0 );
				dp        = saturate( dot( normalRef, normalAvg ) );
				isEdge   += dp < normalTestParams.x;
				?>
			}
		}

		// Compare depth
		if ( cmpDepth )
		{	
			<?
			float depthAvg = dot( depths, float4( 0.25, 0.25, 0.25, 0.25 ) );
			isEdge += saturate( abs( depthAvg - depthRef ) * 0.5 * depthScaleBias.x + depthScaleBias.y );
			?> 
		}
		
		// If non-zero, this is an edge
		<?color = isEdge > 1e-5f;?>
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: discontinuityDetect()
	// Desc: Discontinuity (edge) filter. Can use depth and/or normal differences to 
	//       find geometric discontinuities in the image.
	//-----------------------------------------------------------------------------
	bool discontinuityDetectPlane( bool cmpDepth, bool cmpNormal, bool preserveDirection, int depthType )
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
			cbImageProcessing;
			cbBilateral;
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float   depthRef,  depthSmp, dp;
		half3   normalRef, normalSmp, normalSample;
		float3  positionRef;
		float4  depths, depthWeights = 0, normalWeights = 0;
		float2  depthData, sampleCoords[4];
		float4  depthThreshold = depthExtents.xxxx;
		sampleCoords[0] = texCoords + float2( -1.0,  0.0 ) * textureSize.zw;
		sampleCoords[1] = texCoords + float2(  1.0,  0.0 ) * textureSize.zw;
		sampleCoords[2] = texCoords + float2(  0.0, -1.0 ) * textureSize.zw;
		sampleCoords[3] = texCoords + float2(  0.0,  1.0 ) * textureSize.zw;
		?>
				
		// Retrieve center sample data
		sampleDepthBuffer( "depthData", "sDepthSrcTex", "sDepthSrc", "texCoords", true, depthType );
		<?
		depthRef  = depthData.x;
		normalRef = normalize( sample2D( sNormalSrcTex, sNormalSrc, texCoords ).rgb * 2.0 - 1.0 );
		
		float3 positionSmp;
		float4 ray = float4( 1, 1, 1, 1 );
		
        // Compute position
		float2 uvOffset = float2(-0.5, -0.5) * textureSize.zw;
		positionRef = getViewPosition( texCoords + uvOffset, depthRef, $false );
		
		// Compute a plane
		float4 planeRef = float4( normalRef, -dot( normalRef, positionRef ) );
		?>

		// Sample neighbors
		for ( int i = 0; i < 4; i++ )
		{
			sampleDepthBuffer( "depths[$i]", "sDepthSrcTex", "sDepthSrc", "sampleCoords[$i]", false, depthType );
			
            <?
			// Compute the view space position at this location
			positionSmp = getViewPosition( sampleCoords[$i] + uvOffset, depths[$i], $false );
			
			// Is this point on the main plane?
			depthWeights[ $i ] = dot( positionSmp, normalRef ) + planeRef.w;
            ?>
			
			// Should we also compare normals?
			if( cmpNormal )
			{
				<?
				normalSmp = normalize( sample2D( sNormalSrcTex, sNormalSrc, sampleCoords[$i] ).rgb * 2.0 - 1.0 );
				normalWeights[ $i ] = dot( normalRef, normalSmp );
				?>
			}
			
		} // Next sample
		
		// Compare depths
		<?float4 weights = abs( depthWeights ) > depthThreshold;?>
		
		// Compare normals
		if ( cmpNormal )
		{
			<?
			// Compute a tolerance adjustment for the normal test based on distance
			float cosineThreshold = lerp( normalTestParams.x, normalTestParams.y, saturate( depthRef * normalTestParams.z + normalTestParams.w ) );
			weights += normalWeights < cosineThreshold;
			?>		
		}
		
		// Optionally maintain directional discontinuity information
		if ( preserveDirection )
			<?color = weights > 0 ? 1 : 0;?>
		else
			<?color = dot( weights, float4( 1, 1, 1, 1 ) ) > 0 ? 1 : 0;?>
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : computeScreenMask()
	// Desc : Screen mask computation
   	//-----------------------------------------------------------------------------
	bool computeScreenMask( )
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
			cbImageProcessing;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		
		<?
		// Sample the four texels
		float4 sample = sample2D( sImageTex, sImage, texCoords );

		// Snap all values less than 1/255 (our range min) to 0
		sample = (sample < 0.004f) ? 0.0f : sample;
		
		// Snap all values equal to 1 to 0
		sample = (sample > 0.999998f) ? 0.0f : sample;

		// Convert all remaining values above 0 to 1.0
		color = (sample > 0.0f) ? 1.0f : 0.0f;
		?>
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : downSampleScreenMask()
	// Desc : Screen mask down sampling 
   	//-----------------------------------------------------------------------------
	bool downSampleScreenMask( )
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
			cbImageProcessing;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		// Sample the 2x2 texel block using linear filtering
		float4 sample = sample2D( sImageTex, sImage, texCoords );
		
		// If the value is > 0, snap to 1
		color = sample > 0.0f ? 1.0f : 0.0f;
		?>
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : resolveScreenMask()
	// Desc : Resolves the screen mask (if any channel is an edge, return 1, else 0)
	// Note : Can optionally clip pixels that are zero in all channels
	//-----------------------------------------------------------------------------
    bool resolveScreenMask( bool clipZero )
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
            cbImageProcessing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Sample the mask
		color = sample2D( sImageTex, sImage, texCoords );
		?>

		// If we are clipping
		if ( clipZero )
		{
			<?
			// Clip pixel if all values are 0
			if ( dot( color, float4( 1.0, 1.0, 1.0, 1.0 ) ) < 1e-4f )
				clip( -1 );
			?>	
		}
		else
		{
			// Return 1 if any values are > 0
			<?color = dot( color, float4( 1.0, 1.0, 1.0, 1.0 ) ) > 1e-4f ? 1 : 0;?>
		}	
						
        // Valid shader
        return true;
    }

	//-----------------------------------------------------------------------------
	// Name : packDepthNormal()
	// Desc : Packs depth and normal into the same texture
	//-----------------------------------------------------------------------------
    bool packDepthNormal( bool depthToNormalTarget )
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
            cbImageProcessing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        if ( depthToNormalTarget )
        {
 			<?
			color.rg = 0;
			color.ba = sample2D( sDepthSrcTex,  sDepthSrc,  texCoords ).rg;
			?>
        }
        else
        {
 			<?
			float4 depthSample  = sample2D( sDepthSrcTex,  sDepthSrc,  texCoords );
			float4 normalSample = sample2D( sNormalSrcTex, sNormalSrc, texCoords );
			color.rg = normalSample.rg;
			color.ba = depthSample.rg;
			?>
		}
						
        // Valid shader
        return true;
    }

	//-----------------------------------------------------------------------------
	// Name : drawCartoon()
	// Desc : Simple cartoon shader
	//-----------------------------------------------------------------------------
    bool drawCartoon( )
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
            cbImageProcessing;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		float4 sample = sample2D( sImageTex,  sImage,  texCoords );
		float  luma   = dot( sample.rgb, float3( 0.299, 0.587, 0.114 ) );
		sample.rgb   -= ( sample.rgb % 0.02f );
		
		float4 edge   = sample2D( sEdgeTex,  sEdge,  texCoords );
		float edgeVal = dot( edge, float4( 1.0, 1.0, 1.0, 1.0 ) ) > 1e-4f ? 0.0f : 1.0f;
		sample.rgb   *= edgeVal;
		
		/*		
		if ( luma > 0.95 )
			sample.rgb -= 0.0f;
		else if ( luma > 0.5 )
			sample.rgb -= 0.2f;
		else if ( luma > 0.25 )
			sample.rgb -= 0.4f;
		else
			sample.rgb -= 0.6f;
		*/
		
		color = sample;
		?>
						
        // Valid shader
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Member Functions
	///////////////////////////////////////////////////////////////////////////
	//-----------------------------------------------------------------------------
	// Name: runOperation() 
	// Desc: Runs the indicated color operation
	//-----------------------------------------------------------------------------
	void runOperation( const String & outputVariable, const String & inputVariable, int operation )
	{
        // Constant buffer usage.
        <?cbufferrefs
            cbImageProcessing;
            cbColorControls;
        ?>
		
		// Execute the requested computation
		switch( operation )
		{
			case ImageOperation::SetColorRGBA:
			case ImageOperation::SetColorRGB:
			case ImageOperation::SetColorA:
			case ImageOperation::ScaleUserColorRGBA:
			case ImageOperation::ScaleUserColorRGB:
			case ImageOperation::ScaleUserColorA:
				<?$outputVariable = userColor;?>
			    break;
			case ImageOperation::CopyAlpha:
			case ImageOperation::AlphaToRGBA:
				<?$outputVariable = $inputVariable.aaaa;?>
			    break;
			case ImageOperation::CopyRGBA:
			case ImageOperation::AddRGBA:
			case ImageOperation::TextureScaleRGBA:
			case ImageOperation::TextureScaleRGB:
			case ImageOperation::TextureScaleA:
				<?$outputVariable = $inputVariable;?>
			    break;
			case ImageOperation::CopyRGB:
			case ImageOperation::CopyRGBSetAlpha:
			case ImageOperation::AddRGB:
			case ImageOperation::AddRGBSetAlpha:
				<?$outputVariable = float4( $inputVariable.rgb, userColor.a );?>
			    break;
			case ImageOperation::ColorScaleRGBA:
			case ImageOperation::ColorScaleAddRGBA:
				<?$outputVariable = $inputVariable * userColor;?>
			    break;
			case ImageOperation::ColorScaleAddRGBSetAlpha:
				<?$outputVariable = float4( $inputVariable.rgb * userColor.rgb, userColor.a );?>
			    break;
			case ImageOperation::ColorScaleRGB:
			case ImageOperation::ColorScaleAddRGB:
				<?$outputVariable = float4( $inputVariable.rgb * userColor.rgb, userColor.a );?>
			    break;
			case ImageOperation::InverseRGBA:
				<?$outputVariable = 1.0 - $inputVariable;?>
			    break;
			case ImageOperation::InverseRGB:
				<?$outputVariable = float4( 1.0 - $inputVariable.rgb, $inputVariable.a );?>
			    break;
			case ImageOperation::UnsignedToSignedRGBA:
				<?$outputVariable = $inputVariable * 2.0 - 1.0;?>
			    break;
			case ImageOperation::UnsignedToSignedRGB:
				<?$outputVariable = $inputVariable * float4( 2.0, 2.0, 2.0, 1.0 ) - float4( 1.0, 1.0, 1.0, 0 );?>
			    break;
			case ImageOperation::SignedToUnsignedRGBA:
				<?$outputVariable = $inputVariable * 0.5 + 0.5;?>
			    break;
			case ImageOperation::SignedToUnsignedRGB:
				<?$outputVariable = $inputVariable * float4( 0.5, 0.5, 0.5, 1.0 ) + float4( 0.5, 0.5, 0.5, 0 );?>
			    break;
			case ImageOperation::Grayscale:
				<?
				$outputVariable.rgb = dot( $inputVariable, float3( 0.3, 0.59, 0.11 ) ).xxx;
				$outputVariable.a   = $inputVariable.a;
				?>
			    break;
			case ImageOperation::Luminance:
				<?
				$outputVariable.rgb = dot( $inputVariable.rgb, float3( 0.2125, 0.7154, 0.0721 ) ).xxx;
				$outputVariable.a   = $inputVariable.a;
				?>
			    break;
			case ImageOperation::Sepia:
				<?
				$outputVariable.rgb = float3( 0.191, -0.054, -0.221 ) + dot( $inputVariable.rgb, float3( 0.299, 0.587, 0.114 ) ).xxx;
				$outputVariable.a   = $inputVariable.a;
				?>
			    break;
			case ImageOperation::RGBEtoRGB:
				<?$outputVariable = float4( $inputVariable.rgb * exp2( $inputVariable.a * 255.0 - 128.0 ), userColor.a );?>
			    break;
			case ImageOperation::RGBAtoRGBL:
				
				//<?
                //float4 tcol = $inputVariable;
				//tcol.a = computeIntensity( tcol.rgb );
				//$outputVariable = tcol;
                //?>
			
				<?$outputVariable = float4( $inputVariable.rgb, sqrt( dot( $inputVariable.rgb, float3( 0.299, 0.587, 0.114 ) ) ) );?>
				//<?$outputVariable = float4( $inputVariable.rgb, saturate( ($inputVariable.y * (0.587/0.299) + $inputVariable.x) / 2.963210702 ) );?>
			    break;
			case ImageOperation::TestAlpha:
				<?
				clip( $inputVariable.a - userColor.a );
				$outputVariable = $inputVariable;
				?>
			    break;
			case ImageOperation::TestAlphaDS:
				<?
				clip( $inputVariable.a - userColor.a );
				$outputVariable = userColor;
				?>
			    break;
			case ImageOperation::Exposure:
				<?$outputVariable.rgb = lerp( $inputVariable.rgb, 1.0 - exp( -exposure * $inputVariable.rgb ), exposureAmount );?>
			    break;			
			case ImageOperation::Saturation:
				<?$outputVariable.rgb = lerp( dot( $inputVariable.rgb, float3( 0.2125, 0.7154, 0.0721 ) ), $inputVariable.rgb, saturation );?>
			    break;			
			case ImageOperation::LevelsIn:
				<?$outputVariable.rgb = $inputVariable.rgb * inRange + minAdjust;?>
			    break;			
			case ImageOperation::LevelsOut:
				<?$outputVariable.rgb = $inputVariable.rgb * outRange + blackLevel.y;?>
			    break;			
			case ImageOperation::Gamma:
				<?$outputVariable.rgb  = pow( $inputVariable.rgb, gamma );?>
			    break;			
			case ImageOperation::ColorRemap:
				<?
				$outputVariable.r = sample1D( sRemapTex, sRemap, $inputVariable.r );	
				$outputVariable.g = sample1D( sRemapTex, sRemap, $inputVariable.g );	
				$outputVariable.b = sample1D( sRemapTex, sRemap, $inputVariable.b );	
				?>
			    break;			
			case ImageOperation::TintAndBrightness:
				<?$outputVariable.rgb = $inputVariable.rgb * tint + brightness;?>
			    break;
			case ImageOperation::BlueShift:
				<?
                { // Begin ImageOperation::BlueShift
				    float __ld = dot( $inputVariable.rgb, float3( 0.2125, 0.7154, 0.0721 ) );
				    float __la = $inputVariable.a; // Caller passes adaptation luminance in alpha
				    $outputVariable.rgb = lerp( $inputVariable.rgb, float3(1.05, 0.97, 1.27) * ld, 0.04 / (0.04 + la) );
                } // End ImageOperation::BlueShift
				?>
			    break;
			case ImageOperation::Vignette:
				<?
                { // Begin ImageOperation::Vignette
				    float3 __vignetteColor = saturate( Sample2D( sVignetteTex, sVignette, texCoords ).rgb * vignette.xxx + vignette.yyy );
				    $outputVariable.rgb = lerp( $inputVariable.rgb, $inputVariable.rgb * __vignetteColor, vignette.z );
                } // End ImageOperation::Vignette
				?>
			    break;			
		};			
	}

	//-----------------------------------------------------------------------------
	// Name : runOperations()
	// Desc : Iterates and performs the list of input operations
	//-----------------------------------------------------------------------------
    void runOperations( const String & outputVariable, const String & inputVariable, int[] operations )
    {
        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
        { // Begin runOperations()
        ?>
        
        // Any operations to run?
        if ( operations.length() == 0 )
        {
            // No operations to run. Just duplicate input to output.
			<?$outputVariable = $inputVariable;?>
        
        } // End if no operations
        else
        {
			<?
			// Sample the image for use as the starting "input"
			float4 __c0 = $inputVariable;
			
			// For now just assume start and end colors are the same
			float4 __c1 = __c0;
			?>
			
            // Run the operations list auto-ping-ponging the in/out color
			bool firstAsInput = true;
			for ( uint i = 0; i < operations.length(); i++ )
			{
				if ( firstAsInput )
					runOperation( "__c1", "__c0", operations[ i ] );
				else	
					runOperation( "__c0", "__c1", operations[ i ] );
				
				// Ping-pong input and output for next pass
				firstAsInput = !firstAsInput;
			
            } // Next operation

			// Retrieve the final color (note: the final loop iteration flips the bool!)
			if ( firstAsInput )
				<?$outputVariable = __c0;?>
			else	
				<?$outputVariable = __c1;?>
		
        } // End if has operations

        <?
        } // End runOperations()
        ?>
    }

	//-----------------------------------------------------------------------------
	// Name: initializeDownSampleKernel() 
	// Desc: NxN downsample tex coord constructor.
	// Note: N must be a power of 2: i.e., nKernel = 2,4,8,16,32,64,etc.
	//-----------------------------------------------------------------------------
	int initializeDownSampleKernel( int kernel, bool bilinear, bool oddU, bool oddV )
	{
		float   u, v;
		int     i, j, index = 0;
		int     halfKernel  = kernel / 2;				
		float   offset      = bilinear ? 1.0 : 0.5;
		int     step        = bilinear ? 2 : 1;
		bool    oddKernel   = ( kernel % 2 != 0 ) ? true : false;
		int     numSamples  = bilinear ? (halfKernel * halfKernel) : (kernel * kernel);

		// For odd kernels, we'll need to take extra samples at the edges (Note: We only support point sampling at the moment for odd kernels.)
		if ( oddU || oddV )
		{
			numSamples = 4;
			if ( oddU )         numSamples += 2;
			if ( oddV )         numSamples += 2;
			if ( oddU && oddV ) numSamples += 1;
		}

        // Open constant array definition
        <?
        const float2 downSampleKernel[ $numSamples ] = {
        ?>

		// For a 1x1, just output the sample
		if ( kernel == 1 )
		{
			// Output computed sample
			<?float2( 0.0, 0.0 )?>
		}
		// For odd kernels, create the appropriate offsets
		else if ( oddU || oddV )
		{
			// Assume we are starting in the upper left texel of a 2x2 group
			<?float2( 0.0, 0.0 ), float2( 1.0, 0.0 ), float2( 0.0, 1.0 ), float2( 1.0, 1.0 )?>
			
			if ( oddU )
				<?, float2( 2.0, 0.0 ), float2( 2.0, 1.0 )?>
				
			if ( oddV )
				<?, float2( 0.0, 2.0 ), float2( 1.0, 2.0 )?>
			
			if ( oddU && oddV )
				<?, float2( 2.0, 2.0 )?>
		}
		else
		{
			// Generate elements
			for ( i = -halfKernel; i < halfKernel; i += step )
			{
				v = i + offset;
				for ( j = -halfKernel; j < halfKernel; j += step )
				{
					u = j + offset;

					// Output computed sample
					<?float2( $u, $v )?>

					// Comma separate constant array values
					if ( index < (numSamples - 1) )
						<?,?>

					index++;
				}
			}
		}

         // Close constant array definition
        <?
        };
        ?>

		// Return number of samples to take
		return numSamples;
	}

	//-----------------------------------------------------------------------------
	// Name : nullPixelShader()
	// Desc : "null" pixel shader. Does nothing, but exists for compatibility in 
	//         situations where null shader assignment is not supported
	//-----------------------------------------------------------------------------
    bool nullPixelShader()
    {
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
        ?>
        <?out
            float4  color     : SV_TARGET0;
        ?>
        <?color = 0;?>
		
        return true;
    }
   
	//-----------------------------------------------------------------------------
	// Name : computeEdgeExtents()
	// Desc : Computes depth-based edge extents (e.g., makes finding penumbras easier)
	//-----------------------------------------------------------------------------
	bool computeEdgeExtents( bool edgeOnly, int depthType )
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
			cbImageProcessing;
			cbBilateral;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		// Clear the output color
		color = 0;
				
		// Sample center
		float depthRef = getNormalizedDistance( sImageTex, sImage, texCoords, $depthType );
		
		// Sample neighbors
        float4  neighborDepths;
		neighborDepths[0] = getNormalizedDistance( sImageTex, sImage, texCoords + float2( -1.0,  0.0 ) * textureSize.zw, $depthType );
		neighborDepths[1] = getNormalizedDistance( sImageTex, sImage, texCoords + float2(  1.0,  0.0 ) * textureSize.zw, $depthType );
		neighborDepths[2] = getNormalizedDistance( sImageTex, sImage, texCoords + float2(  0.0, -1.0 ) * textureSize.zw, $depthType );
		neighborDepths[3] = getNormalizedDistance( sImageTex, sImage, texCoords + float2(  0.0,  1.0 ) * textureSize.zw, $depthType );
        ?>

		// We prefer to work with linear depths, so make that adjustment here
		if ( depthType != DepthType::LinearZ )
		{
			<?
            float3 zeroVec = 0;
			depthRef = convertDepthType( depthRef, zeroVec, $depthType, ${DepthType::LinearZ}, $true, $true );
			neighborDepths[0] = convertDepthType( neighborDepths[0], zeroVec, $depthType, ${DepthType::LinearZ}, $true, $true );
			neighborDepths[1] = convertDepthType( neighborDepths[1], zeroVec, $depthType, ${DepthType::LinearZ}, $true, $true );
			neighborDepths[2] = convertDepthType( neighborDepths[2], zeroVec, $depthType, ${DepthType::LinearZ}, $true, $true );
			neighborDepths[3] = convertDepthType( neighborDepths[3], zeroVec, $depthType, ${DepthType::LinearZ}, $true, $true );
            ?>
		}

		// If we are only interested in knowing the general edge status...
		if ( edgeOnly )
		{
			<?
			// Compute difference between center depth and average neighbor depth (more tolerant of small depth deltas)
			float depthDelta = abs( dot( neighborDepths, float4( 0.25, 0.25, 0.25, 0.25 ) ) - depthRef );
			
			// If larger than our threshold, this is an edge
			color.r = depthDelta > depthExtents.y ? 1.0f : 0.0f;
			?>
		}
		else
		{
			// Compute extents
			<?
			color.rg = float2( 1.0f, 0.0f ); 

			// Compute difference between center depth and average neighbor depth (more tolerant of small depth deltas)
			float depthDelta = abs( dot( neighborDepths, float4( 0.25, 0.25, 0.25, 0.25 ) ) - depthRef );
			
			// If larger than our threshold, this is an edge
			if ( depthDelta > depthExtents.y )
			{
				color.r = min( depthRef, min( neighborDepths.x, min( neighborDepths.y, min( neighborDepths.z, neighborDepths.w ) ) ) );
				color.g = max( depthRef, max( neighborDepths.x, max( neighborDepths.y, max( neighborDepths.z, neighborDepths.w ) ) ) );
			}
			?>
		}
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : downsampleExtents2x2()
	// Desc : Downsamples the depth extents map using a 2x2 mip filter
	//-----------------------------------------------------------------------------
	bool downsampleExtents2x2( bool edgeOnly, int edgeChannels )
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
			cbImageProcessing;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?color = 0;?>
	
		// For a standard edge, we can use a single linear tap, followed by a compare
		if ( edgeOnly )
		{
			<?color.r = sample2D( sImageTex, sImage, texCoords ).r > 0.0f;?>
		}
		else
		{
			// For depth extents, get the four samples and compute min/max		
			<?
			float2  extentsSmp, neighborCoords[4];
			float2  extents = float2( 1.0f, 0.0f ); 
			neighborCoords[0] = texCoords + float2( -0.5, -0.5 ) * textureSize.zw;
			neighborCoords[1] = texCoords + float2(  0.5, -0.5 ) * textureSize.zw;
			neighborCoords[2] = texCoords + float2( -0.5,  0.5 ) * textureSize.zw;
			neighborCoords[3] = texCoords + float2(  0.5,  0.5 ) * textureSize.zw;
			color = 0;
			?>

			// Sample neighbor extents and update min/max
			for ( int i = 0; i < 4; i++ )
			{
				<?
				extentsSmp = sample2D( sImageTex, sImage, neighborCoords[$i] ).rg;
				extents.r  = min( extents.r, extentsSmp.r );
				extents.g  = max( extents.g, extentsSmp.g );
				?>
			}

			// Return results via the appropriate channels
		    <?color.rg = extents;?>
		}

		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : filterExtents3x3()
	// Desc : Does a final filter pass on a 3x3 area in the edge texture to ensure
	//        neighbors can contribute
	//-----------------------------------------------------------------------------
	bool filterExtents3x3( bool edgeOnly, int edgeChannels )
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
			cbImageProcessing;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?color = 0;?>

		// For edge only cases, we can use linear filtering to reduce tap count
		if ( edgeOnly )
		{
			<?
			float2 neighborCoords[4];
			neighborCoords[0] = texCoords + float2( -0.5, -0.5 ) * textureSize.zw;
			neighborCoords[1] = texCoords + float2(  0.5, -0.5 ) * textureSize.zw;
			neighborCoords[2] = texCoords + float2( -0.5,  0.5 ) * textureSize.zw;
			neighborCoords[3] = texCoords + float2(  0.5,  0.5 ) * textureSize.zw;
			?>
			for ( int i = 0; i < 4; i++ )
			{
				<?color.r += sample2D( sImageTex, sImage, neighborCoords[$i] ).r;?>
			}
								
			<?float isEdge = color.r > 0.0f;?>
			//<?color.r = 0;?> //REMOVE ME
		    channelCopy( "color", "isEdge", edgeChannels, false );
		}
		else
		{
			<?
			float2  extentsSmp, neighborCoords[9];
			float2  extents = float2( 1.0, 0.0 );
			neighborCoords[0] = texCoords + float2( -1.0, -1.0 ) * textureSize.zw;
			neighborCoords[1] = texCoords + float2(  0.0, -1.0 ) * textureSize.zw;
			neighborCoords[2] = texCoords + float2(  1.0, -1.0 ) * textureSize.zw;
			neighborCoords[3] = texCoords + float2( -1.0,  0.0 ) * textureSize.zw;
			neighborCoords[4] = texCoords;
			neighborCoords[5] = texCoords + float2(  1.0,  0.0 ) * textureSize.zw;
			neighborCoords[6] = texCoords + float2( -1.0,  1.0 ) * textureSize.zw;
			neighborCoords[7] = texCoords + float2(  0.0,  1.0 ) * textureSize.zw;
			neighborCoords[8] = texCoords + float2(  1.0,  1.0 ) * textureSize.zw;
			?>
			// Sample neighbor extents and update min/max
			for ( int i = 0; i < 9; i++ )
			{
				<?
				extentsSmp = sample2D( sImageTex, sImage, neighborCoords[$i] ).rg;
				extents.r  = min( extents.r, extentsSmp.r );
				extents.g  = max( extents.g, extentsSmp.g );
				?>
			}
		    channelCopy( "color", "extents", edgeChannels, false );
		}
		
		// Valid shader
		return true;
	}
	
	//-----------------------------------------------------------------------------
	// Name: blur()
	// Desc: NxN box blur filter. Accomodates both U and V passes with optional
	//       alpha channel weighting and optional bilinear sampling for performance. 
	// Note: if distanceFactor > 0 -> sigma value for gaussian blurring.
	//       if distanceFactor = 0 -> all samples equally weighted
	//       if distanceFactor < 0 -> samples are weighted as pow( 1 / (pixel index + 1), abs(distanceFactor) )
	// Note: The convertDepth values are 0 = no convert, 1 = convert linear, 2 = convert non-linear (not recommended)
	//-----------------------------------------------------------------------------
	bool shadowBlur( int shadowType, int pixelRadius, float distanceFactor, bool linearSampling, int convertDepth, bool vertical )
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
			cbImageProcessing;
			_cbCamera;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?float4 sample, currentColor = 0;?>

		bool vsm  = (shadowType & ShadowMethod::Variance) > 0;
		bool esm  = (shadowType & ShadowMethod::Exponential) > 0;
		bool evsm = vsm && esm;

		// Initialize kernel and get number of sampling iterations
		int numSamples = initializeKernel( pixelRadius, distanceFactor, linearSampling, vertical );
				
		for ( int i = 0; i < numSamples; i++ )
		{
			// Get the current sample
			if ( vertical )
				<?sample = sample2D( sVerticalTex, sVertical, texCoords + filterKernel[ $i ].xy * textureSize.zw );?>
			else
				<?sample = sample2D( sHorizontalTex, sHorizontal, texCoords + filterKernel[ $i ].xy * textureSize.zw );?>

			// Convert non-linear to linear depth
			if ( convertDepth == 2 )
			{
				// If this is a rawz depth, unpack it
				if ( (shadowType & ShadowMethod::RAWZ) > 0 )
					<?sample.x = dot( sample.arg, float3(0.996093809371817670572857294849, 0.0038909914428586627756752238080039, 1.5199185323666651467481343000015e-5) );?>		

				// Convert to linear 
                <?
				sample.x = convertDepthType( sample.x, float3(0,0,0), ${DepthType::NonLinearZ}, ${DepthType::LinearZ}, $true, $true );
                ?>

			} // End convert non-linear depth
			
			// Convert linear depth to statistical moment(s)
			if ( convertDepth > 0 )
			{
				if ( evsm )
				{
					<?sample.y = sample.x * sample.x;?>     // Compute squared depth
					<?sample.z = exp( sample.x * 80.0f );?> // Compute exponential depth

					// Keep the center linear depth sample intact (unblurred)
					if ( i == 0 )
						<?currentColor.w = sample.x;?> 
				}
				else if ( vsm )
				{
					<?sample.y = sample.x * sample.x;?> // Compute squared depth
				}
				else if ( esm )
				{
					<?sample.x = exp( sample.x * 80.0f );?> // Compute exponential depth
				}
			}
			
			// Weight and add
			<?currentColor += sample * filterKernel[ $i ].z;?>

        } // Next sample
		
		// Return results
		<?color = currentColor;?>

		// Valid shader
		return true;
	}
   
	//-----------------------------------------------------------------------------
	// Name: blur()
	// Desc: NxN box blur filter. Accomodates both U and V passes with optional
	//       alpha channel weighting and optional bilinear sampling for performance. 
	// Note: if distanceFactor > 0 -> sigma value for gaussian blurring.
	//       if distanceFactor = 0 -> all samples equally weighted
	//       if distanceFactor < 0 -> samples are weighted as pow( 1 / (pixel index + 1), abs(distanceFactor) )
	// Note: The convertDepth values are 0 = no convert, 1 = convert linear, 2 = convert non-linear (not recommended)
	//-----------------------------------------------------------------------------
	bool blendFrames( )
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
			cbImageProcessing;
			cbReprojection;
			_cbCamera;
			_cbScene;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float  currDepth, prevDepth;
		float3 viewPosition;
		
		// Compute the texture coordinate
		float2 snappedCoord = screenPosition.xy * textureSize.zw;
		float2 uv = snappedCoord + float2(0.5, 0.5) * textureSize.zw;
		?>

		// Lookup the current frame's depth
        sampleDepthBuffer( "currDepth", "sDepthDstTex", "sDepthDst", "uv", false, DepthType::LinearZ_Packed );

		<?
        // Compute a view space position
		viewPosition = getViewPosition( uv, currDepth, $false );	
				
		// Reverse reproject position to prior frame (reprojection matrix = inverseViewMatrix * previousViewProjectionMatrix)
		float4 clipPosPrev = mul( float4( viewPosition, 1 ), reprojectionMatrix );

		// Project to screen space and compute texture coordinates
		clipPosPrev.xy   /= clipPosPrev.w;		
		float2 prevCoords = clipPosPrev.xy * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );
		?>
		
		// Lookup the previous frame's depth
        sampleDepthBuffer( "prevDepth", "sDepthSrcTex", "sDepthSrc", "prevCoords", false, DepthType::LinearZ_Packed );

		<?
		// If the depths are similar to within a tolerance, blend into the prior frame data, else replace it
		//float w = 1.0f - saturate( abs( prevDepth - currDepth ) / depthTolerance );

		float w = abs( prevDepth - currDepth ) < depthTolerance;
		if ( clipPosPrev.x < -1.0f || clipPosPrev.x > 1.0f || clipPosPrev.y < -1.0f || clipPosPrev.y > 1.0f )
			w = 0;
		
		// Time based component
		float t = blendAmount; //1.0 - exp( -_elapsedTime * blendAmount );

		float4 colorCurr = sample2D( sImageCurrTex, sImageCurr, uv );
		float4 colorPrev = sample2D( sImagePrevTex, sImagePrev, prevCoords );
		
		color = lerp( colorCurr, colorPrev, t * w );

		//color = lerp( float4( 1, 0, 0, 1 ), float4( 0, 0, 1, 1 ), w * t );
		//color = colorCurr + (colorPrev - colorCurr) * saturate(w * blendAmount);
		//color = colorPrev + (colorCurr - colorPrev) * (1.0f - saturate(w * blendAmount));

		/*		
		if ( abs( prevDepth - currDepth ) < depthTolerance )
			color = float4( 0, 0, 1, 1 );
		else
			color = float4( 1, 0, 0, 1 );
		if ( clipPosPrev.x < -1.0f || clipPosPrev.x > 1.0f || clipPosPrev.y < -1.0f || clipPosPrev.y > 1.0f )
			color = float4( 1, 0, 0, 1 );
		*/	
		?>

		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : downsampleRSM()
	// Desc : Downsamples an RSM using a 2x2 mip style filter
	//-----------------------------------------------------------------------------
	bool downsampleRSM( int operation )
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
			cbImageProcessing;
			_cbRSM;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		int i;
		float  weightSum = 0;

		float3 posAvg = 0;
		float3 nrmAvg = 0;
		float4 colAvg = 0;

		float3 Position[ 4 ];
		float3 Normal[ 4 ];
		float4 Color[ 4 ];
		
		float2  sampleCoords[4];
		sampleCoords[0] = texCoords + float2( -0.5, -0.5 ) * textureSize.zw;
		sampleCoords[1] = texCoords + float2(  0.5, -0.5 ) * textureSize.zw;
		sampleCoords[2] = texCoords + float2( -0.5,  0.5 ) * textureSize.zw;
		sampleCoords[3] = texCoords + float2(  0.5,  0.5 ) * textureSize.zw;

		// Sample first texel
		Color[ 0 ]       = sample2D( sImage2Tex, sImage2, sampleCoords[ 0 ] );
		Normal[ 0 ]      = sample2D( sImage1Tex, sImage1, sampleCoords[ 0 ] ).rgb * 2.0f - 1.0f;
		Position[ 0 ].z  = sample2D( sImage0Tex, sImage0, sampleCoords[ 0 ] ).r;
		Position[ 0 ].xy = (sampleCoords[ 0 ] * _rsmScreenToViewScaleBias.xy + _rsmScreenToViewScaleBias.zw) * Position[ 0 ].z;
		
		// Initially assume first texel is the maximum
		float3 maxPosition = Position[ 0 ];
		float3 maxNormal   = Normal[ 0 ];
		float4 maxColor    = Color[ 0 ];  // Note: Color.a contains intensity
		
		// Find the maximum intensity texel (sampling remaining texels along the way)
		for ( i = 1; i < 4; i++ )
		{
			Color[ i ]       = sample2D( sImage2Tex, sImage2, sampleCoords[ i ] );
			Normal[ i ]      = sample2D( sImage1Tex, sImage1, sampleCoords[ i ] ).rgb * 2.0f - 1.0f;
			Position[ i ].z  = sample2D( sImage0Tex, sImage0, sampleCoords[ i ] ).r;
			Position[ i ].xy = (sampleCoords[ i ] * _rsmScreenToViewScaleBias.xy + _rsmScreenToViewScaleBias.zw) * Position[ i ].z;

			if ( Color[ i ].a > maxColor.a )
			{
				maxPosition  = Position[ i ];
				maxNormal    = Normal[ i ];
				maxColor     = Color[ i ];				
			}
		}
		
		// Now do a weighted avg against the maximum		
		for ( i = 0 ; i < 4; i++ )
		{
			float deltaP = length( Position[ i ] - maxPosition );
			float deltaN = dot( Normal[ i ], maxNormal );
			float deltaC = abs( Color[ i ].a - maxColor.a );
			
			float weight = (deltaN * 0.7) + (0.3 * 1.0f / (1.0f + deltaC));
			
			posAvg  += Position[ i ] * weight;
			nrmAvg  += Normal[ i ] * weight;
			colAvg  += Color[ i ] * weight;

			weightSum += weight;
		}
		
		// Normalize		
		float recipSum = 1.0f / weightSum;
		posAvg *= recipSum;
		nrmAvg *= recipSum;
		colAvg *= recipSum;
		
		data0.r   = posAvg.z;
		data0.gba = 0;
		data1.rgb = normalize(nrmAvg) * 0.5 + 0.5;
		data1.a   = 0;
		data2     = colAvg;
	    ?>

		// Valid shader
		return true;
	}


   
} // End Class : ImageProcessing