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
// Name : SSAO.sh                                                            //
//                                                                           //
// Desc : Internal surface shader which provides support for SSAO.           //
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
	<?cbuffer cbSSAO : register(b12), globaloffset(c190)
		float4 textureSize;
		float4 radii;
		float4 maxDistance;
		float4 power        = 1.0f;
		float  radBias      = 1.0f;
		float  depthFalloff = 1.0f;
	?>

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
        Sampler2D sDepth      : register(s0);
        Sampler2D sNormal     : register(s1);
        Sampler2D sBump       : register(s2);
        Sampler2D sRotation   : register(s3);
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
	//-----------------------------------------------------------------------------
	bool drawHemisphereAO( int numSamples, int numRadii, bool bumpMap, bool orthographicCamera )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
            float3  eyeScreenRay   : TEXCOORD1;
        ?>

        // Define shader outputs.
        <?out
            float4  color       : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			_cbCamera;
            cbSSAO;
        ?>
        
		<?
		float3 tangentRays[ 31 ] =
		{
		   {-0.16412561,-0.47851399,0.00106239},
		   {-0.56289822,0.61292064,0.23963156},
		   {0.12981524,0.21929535,0.30340615},
		   {0.05311453,0.00891391,0.00791528},
		   {-0.51085919,-0.36309385,0.00856270},
		   {0.34534886,0.50795805,0.12735191},
		   {0.31136537,-0.48069260,0.46914089},
		   {0.10276060,0.17637284,0.18212342},
		   {-0.65985847,-0.17739491,0.72975218},
		   {0.02496089,-0.03679606,0.05733593},
		   {0.80118972,0.10958137,0.44468015},
		   {-0.61773467,-0.28674123,0.28988382},
		   {0.77669978,0.21446092,0.50643247},
		   {0.39152741,-0.07076974,0.48655197},
		   {0.17756817,0.42336029,0.67065418},
		   {0.37651864,0.49597532,0.25906488},
		   {-0.77677196,0.07053248,0.54603386},
		   {0.07049119,0.09208190,0.01440313},
		   {-0.07740894,-0.34512579,0.48830381},
		   {0.50361282,0.10962988,0.11792406},
		   {-0.24336979,-0.35306019,0.63720858},
		   {0.01174363,-0.04537220,0.06891482},
		   {0.41840059,-0.65081871,0.50752723},
		   {0.20630561,-0.15131569,0.24034059},
		   {0.51814777,-0.14331950,0.24042094},
		   {0.65374023,-0.59289628,0.15263288},
		   {0.11689397,-0.20305358,0.00560732},
		   {0.08869601,-0.08453453,0.11370607},
		   {-0.14252484,0.33723447,0.04988577},
		   {-0.56640387,-0.49749848,0.09518766},
		   {0.00477835,-0.42680627,0.74659544}
		};

		float    occ, refZ, occluderZ, deltaD, depthValue;
		float2   clipCoord, screenCoord, depthData, tcTest;
		float3   ray, position, adjPosition, samplePosition, occluderPosition;
		float3x3 tmpTS, tangentToViewSpace;
		half3    toSample, N, T, B, up = half3( 0, 1, 0 );
		float4   sample, sampleSum[4];

		sampleSum[0] = float4( 0, 0, 0, 0 );
		sampleSum[1] = float4( 0, 0, 0, 0 );
		sampleSum[2] = float4( 0, 0, 0, 0 );
		sampleSum[3] = float4( 0, 0, 0, 0 );

		color.rgb = 0;
		color.a   = 1;

		// Compute adjusted focal length
		float2 focalLength = float2( _projectionMatrix._11, _projectionMatrix._22 ); 
		?>

		int i, j, sampleIndex = 0;			

		// Compute total number of samples (reciprocal)		
		float recipTotalSamples = 1.0f / numSamples;

		// Compute the reference normal (frame Z axis)
		<?
		// Create a matrix to transform from tangent to view space
		N = sample2D( sNormalTex, sNormal, texCoords ).xyz * 2.0f - 1.0f;
		N = normalize( N.xyz );
		?>
		// If desired, switch over to a bump normal for dot product calcs
		if ( bumpMap )
			<?N = normalize( sample2D( sBumpTex, sBump, texCoords ).xyz * 2.0f - 1.0f );?>
		
		<?
		if ( abs( N.y ) < (1.0f - 1e-5f) )
			up = half3( 0, 1, 0 ) - (N * N.y);
		else
			up = half3( 1, 0, 0 ) - (N * N.x);

		B = normalize( up );
		T = cross( B, N );		
					
		tmpTS[0] = T;
		tmpTS[1] = B;
		tmpTS[2] = N;
			
		// Randomly rotate the tangent and bitangent about the normal
		float2 cstex  = sample2D( sRotationTex, sRotation, texCoords * _viewportSize.xy * 0.25 ).rg;
		float4 cosSin = float4( cstex.x, cstex.y, -cstex.y, 0 ); 
		tangentToViewSpace[0] = mul( cosSin.xyw, (float3x3)tmpTS );
		tangentToViewSpace[1] = mul( cosSin.zxw, (float3x3)tmpTS );
		tangentToViewSpace[2] = N;
		?>
		
		// Sample the current pixel depth
        sampleDepthBuffer( "refZ", "sDepthTex", "sDepth", "texCoords", false, DepthType::LinearZ_Packed );

        <?
		// Compute view space position
		position = getViewPosition( texCoords, refZ, $orthographicCamera );
		
		// Apply an adaptive bias to reduce shadowing from co-planar occluders.
		adjPosition = position + N * (radBias * refZ);
        ?>
		
		// For every radius scale
		for ( i = 0; i < numSamples; i++ )
		{
			// Transform the random tangent ray
			<?ray = mul( tangentRays[ $i ], tangentToViewSpace );?>
		
			// For every sample...
			for ( j = 0; j < numRadii; j++ )
			{
				<?			
				// Compute the ray position in view space
				samplePosition = adjPosition + ray * radii[ $j ];

				// Project to screen space
				screenCoord = samplePosition.xy * focalLength * (1.0f / samplePosition.z); 
				screenCoord = screenCoord * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );
				?>
				
				// Lookup the potential occluder depth
		        sampleDepthBuffer( "occluderZ", "sDepthTex", "sDepth", "screenCoord", false, DepthType::LinearZ_Packed );
				
				<?
                // Convert to a view space coordinate
				occluderPosition = getViewPosition( screenCoord, occluderZ, $orthographicCamera );
				
				// Get the ray to the occluder
				toSample = occluderPosition - adjPosition;
				
				// Get the distance between the two points
				deltaD = length( toSample );

				// Normalize the occlusion direction
				toSample /= deltaD;
				
				// Compute depth falloff				
				// depthValue = 1.0 / ( 1.0 + deltaD * depthFalloff );
				// depthValue = 1.0f - saturate( deltaD * 1.0f / depthFalloff );
				// Optionally discard samples outside radius
				// if ( deltaD > radii[ $j ] ) depthValue = 0.0f;
				// Scale by clamped cosine term
				//depthValue *= saturate( dot( N, toSample ) );

				depthValue = max( 0, dot( N, toSample ) ) / (0.0001f + (deltaD * deltaD) * depthFalloff); 

				if ( deltaD > radii[ $j ] * 2.0f ) 
					depthValue = 0.0f;
				
				// Add to sum
				sampleSum[ $j ] += depthValue;
				?>
					
			} // Next radius
						
		} // Next sample
			
		for ( j = 0; j < numRadii; j++ )
		{
			<?
			// Normalize, adjust, and accumulate
			sampleSum[ $j ] *= $recipTotalSamples;
			sampleSum[ $j ]  = saturate( 1.0f - sampleSum[ $j ].a * power[ $j ] );
			color.a         *= sampleSum[ $j ].a;
			?>
		}
						
		// Return valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : drawVolumetricAO 
	// Desc : Screen space volumetric ambient occlusion. 
	// ToDo : Opportunitis for pre-shading...
	//-----------------------------------------------------------------------------
	bool drawVolumetricAO( int numSamples, int numRadii, bool bumpMap, bool orthographicCamera )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4  screenPosition : SV_POSITION;
            float2  texCoords      : TEXCOORD0;
            float3  eyeScreenRay   : TEXCOORD1;
        ?>

        // Define shader outputs.
        <?out
            float4  color       : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			_cbCamera;
            cbSSAO;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        
        // Initialize the poisson sampling distribution
        initializePoissonKernel( numSamples, false );
        
		<?
		float4 centerTC, centerWS = float4(0,0,0,1);
		float3 position, adjPosition, smpPosition, L;
		float2 offset, screenRad;
		float  cosine, halfR, zRef, zEntry, zCenter, zSample, deltaD, distanceMod, occlusionContributions, ooRange;
		float  ao[2] = { 1, 1 };

		float2 focalLength = float2( _projectionMatrix._11, _projectionMatrix._22 ); 

		// Get the rotation data (r=cos,g=sin) Note: rotation is tiled every 4x4 screen pixels
		float2 cosSin = sample2D( sRotationTex, sRotation, texCoords * _viewportSize.xy * 0.25 ).rg;

		// Get the normal (view space)
		half3 N = sample2D( sNormalTex, sNormal, texCoords ).xyz * 2.0 - 1.0;
		?>

		// Compute the position of this pixel in view space
		getDistance( "zRef", "sDepthTex", "sDepth", "texCoords", DepthType::LinearZ_Packed );

		<?
        // Compute view space position
		position = getViewPosition( texCoords, zRef, $orthographicCamera );
		
		// Apply a bias to reduce shadowing from co-planar occluders
		adjPosition = position + N * radBias;
        ?>
		
		// If desired, switch over to a bump normal for dot product calcs
		if ( bumpMap )
			<?float3 bN = normalize( sample2D( sBumpTex, sBump, texCoords ).xyz * 2.0f - 1.0f );?>
		
		// For each radii (max of 2 supported)
		for( int i = 0; i < numRadii; i++ )
		{	
			<?
			ao[$i] = 0.0f;
			
			// Compute the half radius
			halfR = radii[$i] * 0.5;

			// Compute tangent sphere center
			centerWS.xyz = adjPosition + N * halfR;
			
			// Compute a texture coordinate at the center of the camera-oriented disk
			centerTC     = mul( centerWS, _projectionMatrix );
			centerTC.xy /= centerTC.w;
			centerTC.xy  = centerTC.xy * float2( 0.5, -0.5 ) + float2( 0.5, 0.5 );

			// Compute camera space Z at the sphere center
			zCenter = centerWS.z - radBias;
			
			// Apply perspective, FOV, and aspect ratio correct to the radius
			screenRad = ( halfR * 0.5 * focalLength ) / zCenter;
			?>

			// Iterate our samples and compute AO
			for( int j = 0; j < numSamples; j++ )
			{
				<?
				// Rotate the sample offset about Z axis (only do this the first time through)
				offset.x = cosSin.x * poissonSamples[$j].x - cosSin.y * poissonSamples[$j].y;
				offset.y = cosSin.y * poissonSamples[$j].x + cosSin.x * poissonSamples[$j].y;
				
				// Scale by radius
				offset *= screenRad;
				?>

				// Get the samples' camera space Z
				getDistance( "zSample", "sDepthTex", "sDepth", "centerTC + offset", DepthType::LinearZ_Packed );

				<?
				ooRange = 1.0 / (radii[$i] * poissonSamples[$j].z);
				zEntry  = zCenter - halfR * poissonSamples[$j].z;
				occlusionContributions = saturate( (zSample - zEntry) * ooRange );
				?>
				
				// If using a bump map
				if ( bumpMap )
				{
					<?
					smpPosition = getViewPosition( centerTC + offset, zSample, $orthographicCamera );
					occlusionContributions *= 1.0f - saturate( dot( bN, normalize( smpPosition - position ) ) );
                    ?>
				}
				
				<?				
				// Reduce occlusion from far away objects
				deltaD = abs( zRef - zSample );
				distanceMod = 1.0f - saturate( (deltaD - (radii[$i] * depthFalloff)) / (radii[$i] * depthFalloff) );

				occlusionContributions = lerp( 1.0, occlusionContributions, distanceMod );
				?>
				
				<?
				ao[$i] += occlusionContributions * poissonSamples[$j].w;
				?>	
			}
			<?
			// Apply a power based correction
			ao[$i] = pow( ao[$i], power[$i] );
			?>
		}

		// Return results
		<?color = (ao[0] * ao[1]);?>
		
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : drawObscuranceAO
	// Desc : Screen space volumetric obscurance. 
	// ToDo : Opportunities for pre-shading...
	//-----------------------------------------------------------------------------
	bool drawObscuranceAO( int numPairedSamples, int numRadii, bool bumpMap, bool orthographicCamera )
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
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////

        // Initialize the poisson sampling distribution (paired samples) 
        initializePairedPoissonKernel( numPairedSamples, true );

		<?
		float2 offset, zSample, radius, deltaD, occlusionContributions, distanceMod, modifiedContributions;
		float3 position, smpPosition;
		float  zCenter, ooMaxDistance, ooHeight;
		float  ao[2] = { 1, 1 };

		// Get the rotation data (r=cos,g=sin) Note: rotation is tiled for every 4 screen pixels
		float2 cosSin = sample2D( sRotationTex, sRotation, texCoords * _viewportSize.xy * 0.25 ).rg;
		?>

		// Compute camera space Z for reference sample
		getDistance( "zCenter", "sDepthTex", "sDepth", "texCoords", DepthType::LinearZ_Packed );

bumpMap = false;

		if ( bumpMap )
			<?position = getViewPosition( texCoords, zCenter, $orthographicCamera );?>

		<?
		// Compute adjusted focal length (scaled by inverse z)
		float2 focalLength = float2( 0.5 * _projectionMatrix._11, 0.5 * _projectionMatrix._22 ) * (1.0 / zCenter);

		// The center is always half occluded by definition
		float occlusionAmount = 0.5 * poissonSamples[0].w;
		?>
				
		// If we are using 2 radii...
		if ( numRadii == 2 )
			<?float2 occlusionAmounts = { occlusionAmount, occlusionAmount };?>
		else
			<?float2 occlusionAmounts = { occlusionAmount, 0 };?>
		
		// If desired, switch over to a bump normal for dot product calcs
		if ( bumpMap )
			<?float3 bN = normalize( sample2D( sBumpTex, sBump, texCoords ).xyz * 2.0f - 1.0f );?>
	
		// Iterate over n radii
		for ( int i = 0; i < numRadii; i++ )
		{
			// Apply perspective, FOV, and aspect ratio correct to the radius
			<?radius = radii[$i] * focalLength;?>
			
			for( int j = 1; j <= numPairedSamples; j++ )
			{
				<?
				// Rotate the sample offset about Z axis
				offset.x = cosSin.x * poissonSamples[$j].x - cosSin.y * poissonSamples[$j].y;
				offset.y = cosSin.y * poissonSamples[$j].x + cosSin.x * poissonSamples[$j].y;

				// Scale by radius
				offset *= radius;
				?>
								
				// Get the samples' camera space Z values
				getDistance( "zSample.x", "sDepthTex", "sDepth", "texCoords + offset.xy", DepthType::LinearZ_Packed );
				getDistance( "zSample.y", "sDepthTex", "sDepth", "texCoords - offset.xy", DepthType::LinearZ_Packed );
				
				<?
				// Compute occlusion
				deltaD  = zCenter.xx - zSample;
				ooHeight = 1.0 / (radii[$i] * poissonSamples[$j].z);
				occlusionContributions = saturate( deltaD * ooHeight + 0.5 ); 
				?>
				
				// If using a bump map
				if ( bumpMap )
				{
                    <?
					smpPosition = getViewPosition( texCoords + offset.xy, zSample.x, $orthographicCamera );
					occlusionContributions.x += (saturate( dot( bN, normalize( smpPosition - position ) ) ));

					smpPosition = getViewPosition( texCoords - offset.xy, zSample.y, $orthographicCamera );
					<?occlusionContributions.y += (saturate( dot( bN, normalize( smpPosition - position ) ) ));
                    ?>
				}
				
				<?
				// Minimize occlusion from far away objects
				distanceMod = 1.0f - saturate( deltaD / maxDistance[$i] );
				
				modifiedContributions = lerp( lerp( 0.5f, 1.0f - occlusionContributions.yx, distanceMod.yx ), occlusionContributions.xy, distanceMod.xy );

				// Apply weights
				modifiedContributions *= poissonSamples[$j].ww;
				
				// Add both samples' contributions to total
				occlusionAmounts[$i] += modifiedContributions.x;		
				occlusionAmounts[$i] += modifiedContributions.y;		
				?>
				
			} // Next sample

			<?
			// Clip values below 0.5 and normalize
			occlusionAmounts[$i] = saturate( ( occlusionAmounts[$i] - 0.5 ) * 2.0 );

			// Return results
			ao[$i] = occlusionAmounts[$i]; 

			// Reset occlusion amounts for next pass
			occlusionAmounts = 0.5 * poissonSamples[0].w;

			// Compute final occlusion amount
			ao[$i] = pow( 1.0 - saturate(ao[$i]), power[$i] );
			?>
			
		} // Next radius
		
		<?
		// Compute final occlusion amount
		color = ao[0] * ao[1];
		?>

        // Valid shader
        return true;
	}

} // End Class : SSAO

