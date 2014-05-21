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
// Name : cgCameraObject.h                                                   //
//                                                                           //
// Desc : Our camera class implemented as a world object that can be         //
//        managed and controlled in the same way as any other object via     //
//        an object node.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCAMERAOBJECT_H_ )
#define _CGE_CGCAMERAOBJECT_H_

//-----------------------------------------------------------------------------
// cgCameraObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgObjectNode.h>
#include <World/cgVisibilitySet.h>
#include <Rendering/cgRenderingTypes.h>
#include <Math/cgFrustum.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {729ED2C9-F4CE-46a1-86C4-5A14C21D468C}
const cgUID RTID_CameraNode   = { 0x729ed2c9, 0xf4ce, 0x46a1, { 0x86, 0xc4, 0x5a, 0x14, 0xc2, 0x1d, 0x46, 0x8c } };
// {EB868E58-89C2-4F05-B5AC-557EB16933F4}
const cgUID RTID_CameraObject = { 0xEB868E58, 0x89C2, 0x4F05, { 0xB5, 0xAC, 0x55, 0x7E, 0xB1, 0x69, 0x33, 0xF4 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCameraObject (Class)
/// <summary>
/// An individual camera object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCameraObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgCameraObject, cgWorldObject, "CameraObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCameraObject( cgUInt32 referenceId, cgWorld * world );
             cgCameraObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgCameraObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setProjectionMode       ( cgProjectionMode::Base mode );
    void                        setFOV                  ( cgFloat degrees );
    // ToDo: 6767 -- Move to a rectangle, and/or LTRB ordering
    void                        setProjectionWindow     ( cgFloat left, cgFloat right, cgFloat bottom, cgFloat top );
    void                        setNearClip             ( cgFloat distance );
    void                        setFarClip              ( cgFloat distance );
    void                        setZoomFactor           ( cgFloat zoom );

    void                        enableDepthOfField      ( bool enable );
    void                        setForegroundExtents    ( cgFloat minimum, cgFloat maximum );
    void                        setForegroundExtents    ( const cgRangeF & range );
    void                        setBackgroundExtents    ( cgFloat minimum, cgFloat maximum );
    void                        setBackgroundExtents    ( const cgRangeF & range );
    void                        setBackgroundBlur       ( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                                          cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow );
    void                        setBackgroundBlur       ( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur );
    void                        setForegroundBlur       ( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                                          cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow );
    void                        setForegroundBlur       ( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur );

    cgProjectionMode::Base      getProjectionMode       ( ) const;
    cgFloat                     getFOV                  ( ) const;
    // ToDo: 6767 -- Move to a rectangle, and/or LTRB ordering
    void                        getProjectionWindow     ( cgFloat & left, cgFloat & right, cgFloat & bottom, cgFloat & top ) const;
    cgFloat                     getNearClip             ( ) const;
    cgFloat                     getFarClip              ( ) const;
    cgFloat                     getZoomFactor           ( ) const;

    bool                        isDepthOfFieldEnabled   ( ) const;
    const cgRangeF            & getForegroundExtents    ( ) const;
    const cgRangeF            & getBackgroundExtents    ( ) const;
    void                        getBackgroundBlur       ( cgBlurOpDesc & highBlur, cgBlurOpDesc & lowBlur ) const;
    void                        getForegroundBlur       ( cgBlurOpDesc & highBlur, cgBlurOpDesc & lowBlur ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_CameraObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgProjectionMode::Base  mProjectionMode;        /// The type of projection currently selected for this camera.
    cgFloat                 mFOV;                   /// Vertical degrees angle (perspective only).
    cgFloat                 mNearClip;              /// Near clip plane Distance
    cgFloat                 mFarClip;               /// Far clip plane Distance
    cgVector4               mProjectionWindow;      /// Projection window (orthographic only)
    cgFloat                 mZoomFactor;            /// The zoom factor (scale) currently applied to any orthographic view.
    cgRangeF                mForegroundExtents;     /// Minimum and maximum distances (from the camera origin) that are to be considered 'foreground'.
    cgRangeF                mBackgroundExtents;     /// Minimum and maximum distances (from the camera origin) that are to be considered 'background'.
    cgBlurOpDesc            mBackgroundHighBlur;    /// Settings to use when processing the high resolution blur pass for the elements within the background extents.
    cgBlurOpDesc            mBackgroundLowBlur;     /// Settings to use when processing the low resolution blur pass for the elements within the background extents.
    cgBlurOpDesc            mForegroundHighBlur;    /// Settings to use when processing the high resolution blur pass for the elements within the foreground extents.
    cgBlurOpDesc            mForegroundLowBlur;     /// Settings to use when processing the low resolution blur pass for the elements within the background extents.
    bool                    mDepthOfFieldEnabled;   /// Render control script should process depth of field for this camera.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertCamera;
    static cgWorldQuery     mUpdateProjection;
    static cgWorldQuery     mLoadCamera;

};

//-----------------------------------------------------------------------------
//  Name : cgCameraNode (Class)
/// <summary>
/// Custom node type for the camera object. Manages additional processes
/// necessary when changes to the camera object take place and to
/// correctly intepret node transformations.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCameraNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgCameraNode, cgObjectNode, "CameraNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCameraNode( cgUInt32 referenceId, cgScene * scene );
             cgCameraNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgCameraNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew                 ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone               ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setAspectRatio              ( cgFloat aspect, bool locked = false );
    cgFloat                     getAspectRatio              ( ) const;
    bool                        isAspectLocked              ( ) const;
    bool                        isFrustumLocked             ( ) const;
    void                        lockFrustum                 ( bool locked );

    const cgFrustum           & getFrustum                  ( );
    const cgFrustum           & getClippingVolume           ( );
    const cgMatrix            & getProjectionMatrix         ( );
    const cgMatrix            & getViewMatrix               ( );
    const cgMatrix            & getPreviousViewMatrix       ( ) const;
    const cgMatrix            & getPreviousProjectionMatrix ( ) const;
    void                        recordCurrentMatrices       ( );

	void						setInterlaceField           ( cgUInt32 field );
	void                        projectionInterlace         ( );

	void                        setJitterAA                 ( const cgVector2 & jitter );
	cgVector2                   getJitterAA                 ( );

    void                        computeVisibility           ( );
    cgVisibilitySet           * getVisibilitySet            ( );

    cgVolumeQuery::Class        boundsInFrustum             ( const cgBoundingBox & bounds );
    cgVolumeQuery::Class        boundsInFrustum             ( const cgBoundingBox & bounds, const cgTransform & transform );
    bool                        viewportToRay               ( const cgSize & viewportSize, const cgVector2 & point, cgVector3 & rayOriginOut, cgVector3 & rayDirectionOut );
    bool                        viewportToWorld             ( const cgSize & viewportSize, const cgVector2 & point, const cgPlane & plane, cgVector3 & positionOut );
    bool                        viewportToMajorAxis         ( const cgSize & viewportSize, const cgVector2 & point, const cgVector3 & axisOrigin, cgVector3 & positionOut, cgVector3 & majorAxisOut );
    bool                        viewportToMajorAxis         ( const cgSize & viewportSize, const cgVector2 & point, const cgVector3 & axisOrigin, const cgVector3 & alignNormal, cgVector3 & positionOut, cgVector3 & majorAxisOut );
    bool                        viewportToCamera            ( const cgSize & viewportSize, const cgVector3 & point, cgVector3 & positionOut );
    bool                        worldToViewport             ( const cgSize & viewportSize, const cgVector3 & position, cgVector3 & pointOut, bool clipX = true, bool clipY = true, bool clipZ = true );
    cgFloat                     estimateZoomFactor          ( const cgSize & viewportSize, const cgPlane & plane );
    cgFloat                     estimateZoomFactor          ( const cgSize & viewportSize, const cgVector3 & position );
    cgFloat                     estimateZoomFactor          ( const cgSize & viewportSize, const cgPlane & plane, cgFloat maximumValue );
    cgFloat                     estimateZoomFactor          ( const cgSize & viewportSize, const cgVector3 & position, cgFloat maximumValue );
    cgVector3                   estimatePickTolerance       ( const cgSize & viewportSize, cgFloat pixelTolerance, const cgVector3 & referencePosition, const cgTransform & objectTransform );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                canScale                    ( ) const;
    virtual void                update                      ( cgFloat timeDelta );
    virtual bool                setCellTransform            ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_CameraNode; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                onComponentModified         ( cgReference * sender, cgComponentModifiedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setProjectionMode( cgProjectionMode::Base mode )
    {
        ((cgCameraObject*)mReferencedObject)->setProjectionMode( mode );
    }
    inline void setFOV( cgFloat degrees )
    {
        ((cgCameraObject*)mReferencedObject)->setFOV( degrees );
    }
    // ToDo: 6767 -- Move to a rectangle, and/or LTRB ordering
    inline void setProjectionWindow( cgFloat left, cgFloat right, cgFloat bottom, cgFloat top )
    {
        ((cgCameraObject*)mReferencedObject)->setProjectionWindow( left, right, bottom, top );
    }
    inline void setNearClip( cgFloat distance )
    {
        ((cgCameraObject*)mReferencedObject)->setNearClip( distance );
    }
    inline void setFarClip( cgFloat distance )
    {
        ((cgCameraObject*)mReferencedObject)->setFarClip( distance );
    }
    inline void setZoomFactor( cgFloat zoom )
    {
        ((cgCameraObject*)mReferencedObject)->setZoomFactor( zoom );
    }
    inline void enableDepthOfField( bool enable )
    {
        ((cgCameraObject*)mReferencedObject)->enableDepthOfField( enable );
    }
    inline void setForegroundExtents( cgFloat minimum, cgFloat maximum )
    {
        ((cgCameraObject*)mReferencedObject)->setForegroundExtents( minimum, maximum );
    }
    inline void setForegroundExtents( const cgRangeF & range )
    {
        ((cgCameraObject*)mReferencedObject)->setForegroundExtents( range );
    }
    inline void setBackgroundExtents( cgFloat minimum, cgFloat maximum )
    {
        ((cgCameraObject*)mReferencedObject)->setBackgroundExtents( minimum, maximum );
    }
    inline void setBackgroundExtents( const cgRangeF & range )
    {
        ((cgCameraObject*)mReferencedObject)->setBackgroundExtents( range );
    }
    inline void setBackgroundBlur( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                   cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow )
    {
        ((cgCameraObject*)mReferencedObject)->setBackgroundBlur( passCountHigh, pixelRadiusHigh, distanceFactorHigh,
                                                                 passCountLow, pixelRadiusLow, distanceFactorLow );
    }
    inline void setBackgroundBlur( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur )
    {
        ((cgCameraObject*)mReferencedObject)->setBackgroundBlur( highBlur, lowBlur );
    }
    inline void setForegroundBlur( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                   cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow )
    {
        ((cgCameraObject*)mReferencedObject)->setForegroundBlur( passCountHigh, pixelRadiusHigh, distanceFactorHigh,
                                                                 passCountLow, pixelRadiusLow, distanceFactorLow );
    }
    inline void setForegroundBlur( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur )
    {
        ((cgCameraObject*)mReferencedObject)->setForegroundBlur( highBlur, lowBlur );
    }

    // Object Property 'Get' Routing
    inline cgProjectionMode::Base getProjectionMode( ) const
    {
        return ((cgCameraObject*)mReferencedObject)->getProjectionMode( );
    }
    inline cgFloat getFOV( ) const
    {
        return ((cgCameraObject*)mReferencedObject)->getFOV( );
    }
    // ToDo: 6767 -- Move to a rectangle, and/or LTRB ordering
    inline void getProjectionWindow( cgFloat & left, cgFloat & right, cgFloat & bottom, cgFloat & top ) const
    {
        ((cgCameraObject*)mReferencedObject)->getProjectionWindow( left, right, bottom, top );
    }
    inline cgFloat getNearClip( ) const
    {
        return ((cgCameraObject*)mReferencedObject)->getNearClip( );
    }
    inline cgFloat getFarClip( ) const
    {
        return ((cgCameraObject*)mReferencedObject)->getFarClip( );
    }
    inline cgFloat getZoomFactor( ) const
    {
        return ((cgCameraObject*)mReferencedObject)->getZoomFactor( );
    }
    inline bool isDepthOfFieldEnabled( ) const
    {
        return ((cgCameraObject*)mReferencedObject)->isDepthOfFieldEnabled( );
    }
    inline const cgRangeF & getForegroundExtents( ) const
    {
        return ((cgCameraObject*)mReferencedObject)->getForegroundExtents( );
    }
    inline const cgRangeF & getBackgroundExtents( ) const
    {
        return ((cgCameraObject*)mReferencedObject)->getBackgroundExtents( );
    }
    inline void getBackgroundBlur( cgBlurOpDesc & highBlur, cgBlurOpDesc & lowBlur ) const
    {
        ((cgCameraObject*)mReferencedObject)->getBackgroundBlur( highBlur, lowBlur );
    }
    inline void getForegroundBlur( cgBlurOpDesc & highBlur, cgBlurOpDesc & lowBlur ) const
    {
        ((cgCameraObject*)mReferencedObject)->getForegroundBlur( highBlur, lowBlur );
    }

private:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgMatrix            mViewMatrix;                /// Cached view matrix
    cgMatrix            mProjectionMatrix;          /// Cached projection matrix.
    cgMatrix            mPreviousViewMatrix;        /// Cached "previous" view matrix.
    cgMatrix            mPreviousProjectionMatrix;  /// Cached "previous" projection matrix.
    cgFrustum           mFrustum;                   /// Details regarding the camera frustum.
    cgFrustum           mClippingVolume;            /// The near clipping volume (area of space between the camera position and the near plane).
    cgVisibilitySet   * mVisibility;                /// Visibility details from the point of view of this camera.
    cgFloat             mAspectRatio;               /// The aspect ratio used to generate the correct horizontal degrees (perspective only)
	cgUInt32            mInterlaceField;            /// Current field (interlaced rendering)
	cgVector2           mJitterAA;                  /// Current jitter value for temporal super sample antialiasing

    // Update States
    bool                mViewDirty;                 /// View matrix dirty ?
    bool                mProjectionDirty;           /// Projection matrix dirty ?
    bool                mAspectDirty;               /// Has the aspect ratio changed?
    bool                mAspectLocked;              /// Should the aspect ratio be automatically updated by the render driver?
    bool                mFrustumDirty;              /// Are the frustum planes dirty ?
    bool                mFrustumLocked;             /// Is the frustum locked?

};

#endif // !_CGE_CGCAMERAOBJECT_H_