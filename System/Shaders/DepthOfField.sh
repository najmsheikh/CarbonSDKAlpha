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
// Name : DepthOfField.sh                                                    //
//                                                                           //
// Desc : Internal surface shader which provides support for DoF.            //
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
    return DepthOfFieldShader( owner, driver, resources );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : DepthOfFieldShader (Class)
// Desc : Material shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for depth of field.
//-----------------------------------------------------------------------------
class DepthOfFieldShader : ISurfaceShader
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
	<?cbuffer cbDepthOfField : register(b13), globaloffset(c180)
		float4 regionScaleBias    = { 0, 0, 0, 0 };       // x = 1/(FGMax-FGMin), y = 1/(BGMax-BGMin), z = -FGMin*x, w = -BGMin*y
		float4 blurScale;
		float4 blurBias;
		float4 textureSize;
		float4 textureSizeMedium;
		float4 textureSizeLarge;
	?>

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
		Sampler2D sColor                : register(s0);
		Sampler2D sDepth                : register(s1);
		Sampler2D sDepthLowRes          : register(s2);
		Sampler2D sBlur0                : register(s3);
		Sampler2D sBlur1                : register(s4);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : DepthOfFieldShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	DepthOfFieldShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
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
	// Name  : computeBlurriness ( Pixel Shader )
	// Desc  : Computes a blurriness factor for depth of field according to linear
	//         distance to near and far sets of planes (foreground and background).
	//-----------------------------------------------------------------------------
	bool computeBlurriness( int depthType, bool foreground, bool background )
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
            float4  color     : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			cbDepthOfField;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        
		<?float distance;?>
		getDistance( "distance", "sDepthTex", "sDepth", "texCoords", depthType );

		// If the depth type is not what we need (i.e., distance from camera along eye ray), convert it
		if ( getPureDepthType( depthType ) != DepthType::LinearZ )
		{
			<?
            eyeRay = normalize( eyeRay );
			distance = convertDepthType( distance, eyeRay, $(getPureDepthType( depthType )), $(DepthType::LinearZ), $false, $false );
            ?>
		}	
		
		<?
		// Compute blurriness 
		float2 blurriness = saturate( distance.xx * regionScaleBias.xy + regionScaleBias.zw );
		?>
		
		if ( !foreground )
			<?blurriness.x = 0;?>
		else
			<?blurriness.x = 1.0 - blurriness.x;?>

		if ( !background )
			<?blurriness.y = 0;?>
		
		<?color = max( blurriness.x, blurriness.y );?>
		
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name  : maxBlurriness ( Pixel Shader )
	// Desc  : Sets the maximum blurriness factor
	//-----------------------------------------------------------------------------
	bool maxBlurriness( )
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
            float4 color : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			cbDepthOfField;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?color = 1.0f;?> // Make this configurable?
		
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name  : getSmallBlur ( Function )
	// Desc  : Computes a small blur to help smooth the transition between no blur
	//         and lower blur radii.
	//-----------------------------------------------------------------------------
	__shadercall float4 getSmallBlur( float2 texCoords, float4 origColor )
	{
		/////////////////////////////////////////////
		// Definitions
		/////////////////////////////////////////////
		// Constant buffer usage.
		<?cbufferrefs
			cbDepthOfField;
		?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		const float centerWeight   = 1.0f / 17.0f;
		const float neighborWeight = 4.0f / 17.0f;
		float4 c0 = sample2DLevel( sColorTex, sColor, texCoords + float2(  0.5f, -1.5f ) * textureSize.zw, 0 );
		float4 c1 = sample2DLevel( sColorTex, sColor, texCoords + float2( -1.5f, -0.5f ) * textureSize.zw, 0 );
		float4 c2 = sample2DLevel( sColorTex, sColor, texCoords + float2( -0.5f,  1.5f ) * textureSize.zw, 0 );
		float4 c3 = sample2DLevel( sColorTex, sColor, texCoords + float2(  1.5f,  0.5f ) * textureSize.zw, 0 );
		float4 cOut = origColor * centerWeight   + 
		                     c0 * neighborWeight + 
		                     c1 * neighborWeight + 
		                     c2 * neighborWeight + 
		                     c3 * neighborWeight;
		return cOut;		
		?>
	}

	//-----------------------------------------------------------------------------
	// Name  : composite
	// Desc  : Computes depth of field
	//-----------------------------------------------------------------------------
	bool compositeNEW( bool foreground, bool background )
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
            float4  color     : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			cbDepthOfField;
			_cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////        
        if ( foreground )
        {
			<?
			float blurriness;
			float4 blurNone, blurSmall, blurMedium, blurLarge;
			
			// Sample the widest blur
			blurLarge = sample2D( sBlur1Tex, sBlur1, texCoords );
			        
			// If this is a non-blurred pixel, exit
			if( blurLarge.a < 1e-6f )
			{
				color = sample2DLevel( sColorTex, sColor, texCoords, 0 );
			}
			else
			{
				// Sample our low, medium, and high blurs
				blurNone   = sample2DLevel( sColorTex, sColor, texCoords, 0 );
				blurMedium = sample2DLevel( sBlur0Tex, sBlur0, texCoords, 0 );
				blurSmall  = getSmallBlur( texCoords, blurNone );

				// Get the blurriness 
				blurriness = blurMedium.a;
			
				// Compute interpolated weights for the blur levels
				float4 weights    = saturate( blurriness * blurScale + blurBias );
					   weights.yz = min( weights.yz, 1.0f - weights.xy );        
		
				// Compute a final color based on weighted average of the blur levels
				color = blurNone * weights.x + blurSmall * weights.y + blurMedium * weights.z + blurLarge * weights.w;
			}
			?>
        }
		else
		{
			<?
			// Sample our low, medium, and high blurs
			blurNone   = sample2D( sColorTex, sColor, texCoords );
			blurMedium = sample2D( sBlur0Tex, sBlur0, texCoords );
			blurSmall  = getSmallBlur( texCoords, blurNone );
			
			// Compute interpolated weights for the blur levels
			blurriness        = blurNone.a;
			float4 weights    = saturate( blurriness * blurScale + blurBias );
				   weights.yz = min( weights.yz, 1.0f - weights.xy );        
			
			// Compute a final color based on weighted average of the blur levels
			color = blurNone * weights.x + blurSmall * weights.y + blurMedium * weights.z + blurLarge * weights.w;
			?>
		}        
		
        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name  : composite
	// Desc  : Computes depth of field
	//-----------------------------------------------------------------------------
	bool composite( bool foreground, bool background )
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
            float4  color     : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			cbDepthOfField;
			_cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
        <?
		float blurriness;

		// Sample original color (CoC in alpha)
		float4 origColor  = sample2D( sColorTex, sColor, texCoords );

		// Sample our low, medium, and high blurs
		float4 blurNone   = origColor;
		float4 blurMedium = sample2D( sBlur0Tex, sBlur0, texCoords );
		float4 blurLarge  = sample2D( sBlur1Tex, sBlur1, texCoords );
		float4 blurSmall  = blurMedium;
		?>
		
		// Get the small blur
		if ( !background )
			<?blurSmall = getSmallBlur( texCoords, origColor );?>
		
		// If we are doing a foreground pass only, select the low res blurriness
		if ( foreground && !background )
		{
			<?blurriness = blurMedium.a;?>
		}
		// If we are doing a background pass only, select the high res blurriness
		else if ( background && !foreground )
		{
			<?blurriness = blurNone.a;?>
		}
		else
		{
			<?blurriness = max( blurLarge.a, blurNone.a );?>
		}
		
		<?	
		// Compute interpolated weights for the blur levels
        float4 weights    = saturate( blurriness * blurScale + blurBias );
		       weights.yz = min( weights.yz, 1.0f - weights.xy );        
		?>
		
		// Compute a final color based on weighted average of the blur levels
		<?color = blurNone * weights.x + blurSmall * weights.y + blurMedium * weights.z + blurLarge * weights.w;?>

        // Valid shader
        return true;
	}


	//-----------------------------------------------------------------------------
	// Name : finalizeForegroundCoC()
	// Desc : Compute a final CoC for foreground pixels after initial blurring
   	//-----------------------------------------------------------------------------
	bool finalizeForegroundCoC( )
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

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		float4 medium = sample2D( sBlur0Tex, sBlur0, texCoords ); // CoC is unblurred here
		float4 large  = sample2D( sBlur1Tex, sBlur1, texCoords ); // CoC is blurred here
		
		// Compute a final color and CoC
		color.rgb = medium.rgb;
		color.a   = 2.0f * max( medium.a, large.a ) - medium.a;
		?>	

		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : blurForegroundCoC()
	// Desc : Compute a small blur to smooth the foreground CoC
   	//-----------------------------------------------------------------------------
	bool blurForegroundCoC( )
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
			cbDepthOfField;
        ?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?
		color    = 0;
		color.a += sample2D( sBlur0Tex, sBlur0, texCoords + float2( -0.5, -0.5 ) * textureSize.zw ).a; 
		color.a += sample2D( sBlur0Tex, sBlur0, texCoords + float2(  0.5, -0.5 ) * textureSize.zw ).a; 
		color.a += sample2D( sBlur0Tex, sBlur0, texCoords + float2( -0.5,  0.5 ) * textureSize.zw ).a; 
		color.a += sample2D( sBlur0Tex, sBlur0, texCoords + float2(  0.5,  0.5 ) * textureSize.zw ).a; 
		color.a *= 0.25f;
		?>	

		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : upsampleBackgroundBlur()
	// Desc : Upsamples the background blur texture and composites with next level up
	//        using destination alpha blending.
   	//-----------------------------------------------------------------------------
	bool upsampleBackgroundBlur( )
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

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		<?color = sample2D( sBlur0Tex, sBlur0, texCoords );?>	

		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : downSampleBackground()
	// Desc : Down samples the background and attempts to eliminate halos as well as
	//        provide a smoother transition between non-blurred and blurred pixels.
   	//-----------------------------------------------------------------------------
	bool downSampleBackground( int mipLevel )
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
			cbDepthOfField;
        ?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		// Select the appropriate texel size
		<?float2 recipTexelSize;?>
		if ( mipLevel == 0 )
			<?recipTexelSize = textureSize.zw;?>
		else
			<?recipTexelSize = textureSizeMedium.zw;?>

		<?
		float4 weights, finalWeights, samples[4];
		float  minimumWeight, maximumWeight;

		// Get the samples
		samples[0] = sample2D( sColorTex, sColor, texCoords + float2( -0.5, -0.5 ) * recipTexelSize );
		samples[1] = sample2D( sColorTex, sColor, texCoords + float2(  0.5, -0.5 ) * recipTexelSize );
		samples[2] = sample2D( sColorTex, sColor, texCoords + float2( -0.5,  0.5 ) * recipTexelSize );
		samples[3] = sample2D( sColorTex, sColor, texCoords + float2(  0.5,  0.5 ) * recipTexelSize );

		weights[0] = samples[0].a;
		weights[1] = samples[1].a;
		weights[2] = samples[2].a;
		weights[3] = samples[3].a;
		
		// Find the maximum weight
		maximumWeight = max( weights.x, max( weights.y, max( weights.z, weights.w ) ) );
				
		// Compute difference between the maximum and each sample		
		float4 deltas = abs( maximumWeight.xxxx - weights );

		// Compute weights for each pixel based on their deltas
		float recipMinDelta = 1.0f / 0.7f; //TODO: Make user controllable.
		finalWeights = 1.0f - saturate( deltas * recipMinDelta );
		
		// Exclude all foreground pixels
		if ( maximumWeight < 1e-8f )
			finalWeights = 0.0f;
				
		// Weight and add the colors
		color.rgb = samples[0].rgb * finalWeights[0];
		?>
		for ( int i = 1; i < 4; i++ )
		{
			<?color.rgb += samples[$i].rgb * finalWeights[$i];?>
		} 

		<?
		// Normalize the color
		float weightSum = dot( finalWeights, float4( 1, 1, 1, 1 ) );
		if ( weightSum > 0.0f )
			color.rgb /= weightSum;

		// Store the maximum as the new weight		
		color.a = maximumWeight;
		
		// Find the minimum non-zero weight
		weights = weights > 0.0f ? weights : 100.0f;
		minimumWeight = min( weights.x, min( weights.y, min( weights.z, weights.w ) ) );
		if ( minimumWeight > 2.0f )
			minimumWeight = 0.0f;
				
		// Store the maximum as the new weight		
		color.a = minimumWeight;
		?>
		
		// Valid shader
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : downSampleBackground()
	// Desc : Down samples the background and attempts to eliminate halos as well as
	//        provide a smoother transition between non-blurred and blurred pixels.
   	//-----------------------------------------------------------------------------
	bool downSampleBackgroundTEST( int mipLevel )
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
			cbDepthOfField;
        ?>

		/////////////////////////////////////////////
		// Shader Code
		/////////////////////////////////////////////
		// Select the appropriate texel size
		<?float2 recipTexelSize;?>
		if ( mipLevel == 0 )
			<?recipTexelSize = textureSize.zw;?>
		else
			<?recipTexelSize = textureSizeMedium.zw;?>

		<?
		float4 weights, samples[4];
		float  maximumWeight;

		// Get the samples
		samples[0] = sample2D( sColorTex, sColor, texCoords + float2( -0.5, -0.5 ) * recipTexelSize );
		samples[1] = sample2D( sColorTex, sColor, texCoords + float2(  0.5, -0.5 ) * recipTexelSize );
		samples[2] = sample2D( sColorTex, sColor, texCoords + float2( -0.5,  0.5 ) * recipTexelSize );
		samples[3] = sample2D( sColorTex, sColor, texCoords + float2(  0.5,  0.5 ) * recipTexelSize );

		weights[0] = samples[0].a;
		weights[1] = samples[1].a;
		weights[2] = samples[2].a;
		weights[3] = samples[3].a;
		
		// Find the maximum weight
		maximumWeight = max( weights.x, max( weights.y, max( weights.z, weights.w ) ) );
		
		float tolerance = 0.05f;
		float4 zeroTest = weights > 0.0f;
		
		weights  = saturate( (weights + tolerance) - maximumWeight );
		weights  = weights > 0.0f;
		weights *= zeroTest;
		
		// Weight and add the colors
		color = samples[0] * weights[0];
		?>
		for ( int i = 1; i < 4; i++ )
		{
			<?color += samples[$i] * weights[$i];?>
		} 

		<?
		// Normalize the color
		float weightSum = dot( weights, float4( 1, 1, 1, 1 ) );
		if ( weightSum > 0.0f )
			color /= weightSum;
		?>
		
		// Valid shader
		return true;
	}


} // End Class : DepthOfField


