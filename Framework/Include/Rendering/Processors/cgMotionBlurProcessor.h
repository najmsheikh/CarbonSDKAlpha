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
// Name : cgMotionBlurProcessor.h                                            //
//                                                                           //
// Desc : Image processing class designed to apply a blur to a rendered      //
//        based on the amount and direction of motion of the camera and / or //
//        objects in the scene.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGMOTIONBLURPROCESSOR_H_)
#define _CGE_CGMOTIONBLURPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgMotionBlurProcessor Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgImageProcessor.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgMotionBlurProcessor (Class)
/// <summary>
/// Image processing class designed to apply a blur to a rendered based on the 
/// amount and direction of motion of the camera and / or objects in the scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgMotionBlurProcessor : public cgImageProcessor
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgMotionBlurProcessor, cgImageProcessor, "MotionBlurProcessor" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgMotionBlurProcessor( );
    virtual ~cgMotionBlurProcessor( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void            setTargetRate           ( cgFloat rate );
    void            setAttenuationRates     ( cgFloat minimum, cgFloat maximum );
    void            setAttenuationRates     ( const cgRangeF & range );
	void			setBlurAmount			( cgFloat amt );
	void			setRotationBlurAmount   ( cgFloat amt );
	void			setTranslationBlurAmount( cgFloat amt );
	void			setMaxSpeed				( cgFloat speed );
	void			setCompositeSpeedScale	( cgFloat minimumAngularVelocity, cgFloat maximumAngularVelocity, cgFloat power );

    bool			computePixelVelocity    ( cgCameraNode * activeCamera, cgFloat timeDelta, const cgTextureHandle & sourceDepth, cgDepthType::Base depthType, const cgRenderTargetHandle & velocity );
	bool			execute                 ( cgInt32 nPasses, const cgTextureHandle & sourceColor, const cgTextureHandle & sourceVelocity, const cgRenderTargetHandle & sourceColorLow, const cgRenderTargetHandle & sourceColorLowScratch, const cgRenderTargetHandle & destination );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgImageProcessor)
    //-------------------------------------------------------------------------
    virtual bool    initialize              ( cgRenderDriver * driver );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct CameraData
    {
        cgMatrix    viewKeyMatrix1;
        cgMatrix    viewKeyMatrix2;
        cgMatrix    prevCameraMatrix;
        cgFloat     accumulatedTime;
        cgFloat     currentRenderRate;

        // Constructor
        CameraData() : 
            viewKeyMatrix1(cgMatrix::Identity), viewKeyMatrix2(cgMatrix::Identity),
            prevCameraMatrix(cgMatrix::Identity), accumulatedTime(0), currentRenderRate(0) {}
    };
    CGE_UNORDEREDMAP_DECLARE( void*, CameraData, CameraDataMap )

    // Constant buffers
    struct _cbMotionBlur
    {
        cgMatrix interpolatedCameraMatrix;
		cgFloat  blurAmount;
		cgFloat  maxSpeed;
		cgFloat  compositeBlend;
    };
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool            update                  ( cgCameraNode * activeCamera, cgFloat timeDelta );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle   mMotionBlurShader;
    cgConstantBufferHandle  mMotionBlurConstants;
    cgSampler             * mColorSampler;
    cgSampler             * mDepthSampler;
	cgSampler             * mVelocitySampler;
	cgSampler             * mColorLowSampler;

    CameraDataMap           mCameraData;
    _cbMotionBlur           mOperationData;
    
    // Configuration
    cgFloat                 mTargetRate;            // Target frame rate at which motion blur will be sampled
    cgRangeF                mAttenuationRates;      // Frame rates between which motion blur will begin to be attenuated out.
	cgFloat                 mBlurAmount;            // General blurriness control.
	cgFloat                 mRotationalBlurAmt;     // How much should rotational blur would we like (0 to 1)?
	cgFloat                 mTranslationBlurAmt;    // How much should translation blur would we like (0 to 1)?

	cgFloat					mMinimumAngularVelocity;
	cgFloat					mMaximumAngularVelocity;
	cgFloat					mBlendPower;
};

#endif // !_CGE_CGMOTIONBLURPROCESSOR_H_