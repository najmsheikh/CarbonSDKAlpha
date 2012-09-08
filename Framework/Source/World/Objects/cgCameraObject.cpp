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
// Name : cgCameraObject.cpp                                                 //
//                                                                           //
// Desc : Our camera class implemented as a world object that can be         //
//        managed and controlled in the same way as any other object via     //
//        an object node.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgCameraObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgLightObject.h>
#include <World/cgScene.h>
#include <Rendering/cgRenderDriver.h>
#include <System/cgMessageTypes.h>

#define INTERLACING_OFF  -1
#define TOP_FIELD         0
#define BOTTOM_FIELD      1

///////////////////////////////////////////////////////////////////////////////
// cgCameraObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCameraObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraObject::cgCameraObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Reset / Clear all required values
    mProjectionMode     = cgProjectionMode::Perspective;
    mFOV                = 60.0f;
    mNearClip           = 0.2f;
    mFarClip            = 500.0f;
    mProjectionWindow   = cgVector4( 0, 0, 0, 0 );
    mZoomFactor         = 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgCameraObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraObject::cgCameraObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgWorldObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    cgCameraObject * pObject = (cgCameraObject*)init;
    mProjectionMode     = pObject->mProjectionMode;
    mFOV                = pObject->mFOV;
    mNearClip           = pObject->mNearClip;
    mProjectionWindow   = pObject->mProjectionWindow;
    mZoomFactor         = pObject->mZoomFactor;
}

//-----------------------------------------------------------------------------
//  Name : ~cgCameraObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraObject::~cgCameraObject()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgCameraObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base.
    if ( bDisposeBase == true )
        cgWorldObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgCameraObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgCameraObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgCameraObject::allocateClone( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod )
{
    // Valid clone?
    return new cgCameraObject( referenceId, world, init, initMethod );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_CameraObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgCameraObject::getDatabaseTable( ) const
{
    return _T("Objects::Camera");
}

//-----------------------------------------------------------------------------
// Name : getZoomFactor( )
/// <summary>
/// Get the zoom factor (scale) currently applied to any orthographic view.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraObject::getZoomFactor( ) const
{
    return mZoomFactor;
}

//-----------------------------------------------------------------------------
// Name : setZoomFactor( )
/// <summary>
/// Set the zoom factor (scale) currently applied to any orthographic view.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setZoomFactor( cgFloat fZoom )
{
    const cgFloat MinZoomFactor = 0.0001f;
    const cgFloat MaxZoomFactor = 20000.0f;

    // Clamp the new zoom factor
    if ( fZoom < MinZoomFactor ) fZoom = MinZoomFactor;
    if ( fZoom > MaxZoomFactor ) fZoom = MaxZoomFactor;
   
    // Skip if this is a no-op
    if ( fZoom == mZoomFactor )
        return;

    // Update
    cgFloat fOldValue = mZoomFactor;
    mZoomFactor = fZoom;

    // Notify listeners that property was altered
    static const cgString strContext = _T("ZoomFactor");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setFOV ()
/// <summary>
/// Sets the field of view angle of this camera (perspective only).
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setFOV( cgFloat fFOVY )
{
    // Skip if no-op
    if ( mFOV == fFOVY )
        return;

    // Update projection matrix and view frustum
    mFOV = fFOVY;

    // Notify listeners that property was altered
    static const cgString strContext = _T("FOV");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setProjectionWindow ()
/// <summary>
/// Sets offsets for the projection window (orthographic only).
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setProjectionWindow( cgFloat l, cgFloat r, cgFloat b, cgFloat t )
{
    mProjectionWindow = cgVector4( l, r, b, t );

    // Notify listeners that property was altered
    static const cgString strContext = _T("ProjectionWindow");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setProjectionMode ()
/// <summary>
/// Sets the current projection mode for this camera (i.e. orthographic
/// or perspective).
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setProjectionMode( cgProjectionMode::Base Mode )
{
    // Bail if this is a no op.
    if ( Mode == mProjectionMode )
        return;

    // Alter the projection mode.
    mProjectionMode = Mode;

    // Notify listeners that property was altered
    static const cgString strContext = _T("ProjectionMode");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setNearClip ()
/// <summary>
/// Set the near plane distance
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setNearClip( cgFloat fDistance )
{
    // Skip if this is a no-op
    if ( fDistance == mNearClip )
        return;

    // Store value
    mNearClip = fDistance;

    // Notify listeners that property was altered
    static const cgString strContext = _T("NearClip");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setFarClip()
/// <summary>
/// Set the far plane distance
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setFarClip( cgFloat fDistance )
{
    // Skip if this is a no-op
    if ( fDistance == mFarClip )
        return;

    // Store value
    mFarClip = fDistance;

    // Notify listeners that property was altered
    static const cgString strContext = _T("FarClip");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getFOV()
/// <summary>
/// Retrieve the current field of view angle in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraObject::getFOV( ) const
{
    return mFOV;
}

//-----------------------------------------------------------------------------
//  Name : getProjectionWindow ()
/// <summary>
/// Retrieve offsets for the projection window (orthographic only).
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::getProjectionWindow( cgFloat & l, cgFloat & r, cgFloat & b, cgFloat & t ) const
{
    l = mProjectionWindow.x;
    r = mProjectionWindow.y;
    b = mProjectionWindow.z;
    t = mProjectionWindow.w;
}

//-----------------------------------------------------------------------------
//  Name : getNearClip()
/// <summary>
/// Retrieve the distance from the camera to the near clip plane.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraObject::getNearClip( ) const
{
    return mNearClip;
}

//-----------------------------------------------------------------------------
//  Name : getFarClip()
/// <summary>
/// Retrieve the distance from the camera to the far clip plane.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraObject::getFarClip( ) const
{
    return mFarClip;
}

//-----------------------------------------------------------------------------
//  Name : getProjectionMode ()
/// <summary>
/// Retrieve the current projection mode for this camera.
/// </summary>
//-----------------------------------------------------------------------------
cgProjectionMode::Base cgCameraObject::getProjectionMode( ) const
{
    return mProjectionMode;
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::applyObjectRescale( cgFloat fScale )
{
    // Apply the scale to object-space data
    mNearClip *= fScale;
    mFarClip  *= fScale;

    // ToDo: Update world database

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

///////////////////////////////////////////////////////////////////////////////
// cgCameraNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCameraNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraNode::cgCameraNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Reset / Clear all required values
    mAspectRatio    = 1.0f;
    
    // Internal features are dirty by default
    mViewDirty      = true;
    mProjectionDirty      = true;
    mAspectDirty    = true;
    mFrustumDirty   = true;
    mFrustumLocked  = false;
    mAspectLocked   = false;
    
    // Clear matrices
    cgMatrix::identity( mViewMatrix );
    cgMatrix::identity( mProjectionMatrix );
    cgMatrix::identity( mPreviousViewMatrix );
    cgMatrix::identity( mPreviousProjectionMatrix );

    // Allocate necessary visibility sets dynamically on the heap
    // rather than statically with the object. This allows the camera
    // to be destroyed but the visibility set to remain active i.e.
    // in a script.
    mVisibility = new cgVisibilitySet();

    // Disable interlacing by default
    mInterlaceField = INTERLACING_OFF;

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Camera%X"), nReferenceId );

	// Assume no jittering initially
	mJitterAA = cgVector2( 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : cgCameraNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraNode::cgCameraNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Duplicate necessary properties.
    cgCameraNode * pCamera = (cgCameraNode*)pInit;
    mViewMatrix                 = pCamera->mViewMatrix;
    mProjectionMatrix           = pCamera->mProjectionMatrix;
    mPreviousViewMatrix         = pCamera->mPreviousViewMatrix;
    mPreviousProjectionMatrix   = pCamera->mPreviousProjectionMatrix;
    mAspectLocked               = pCamera->mAspectLocked;
    mFrustumLocked              = pCamera->mFrustumLocked;
    mFrustum                    = pCamera->mFrustum;
    mViewDirty                  = true;    // Matrix may have been adjusted
    mProjectionDirty            = pCamera->mProjectionDirty;
    mFrustumDirty               = true;    // Matrix may have been adjusted
    mAspectDirty                = pCamera->mAspectDirty;
    mAspectRatio                = pCamera->mAspectRatio;
    mInterlaceField             = pCamera->mInterlaceField;
    mClippingVolume             = pCamera->mClippingVolume;
    mJitterAA                   = pCamera->mJitterAA;

    // Allocate necessary visibility sets dynamically on the heap
    // rather than statically with the object. This allows the camera
    // to be destroyed but the visibility set to remain active i.e.
    // in a script.
    mVisibility = new cgVisibilitySet();
}

//-----------------------------------------------------------------------------
//  Name : ~cgCameraNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraNode::~cgCameraNode()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgCameraNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgCameraNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgCameraNode::allocateClone( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform )
{
    return new cgCameraNode( referenceId, scene, init, initMethod, initTransform );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgCameraNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // If this camera is currently assigned to the scene, or its render driver,
    // remove it.
    if ( mParentScene )
    {
        if ( mParentScene->getActiveCamera() == this )
            mParentScene->setActiveCamera( CG_NULL );
        cgRenderDriver * renderDriver = mParentScene->getRenderDriver();
        if ( renderDriver && renderDriver->getCamera() == this )
            renderDriver->setCamera( CG_NULL );
    
    } // End if in scene

    // Release allocated memory
    if ( mVisibility != CG_NULL )
        mVisibility->scriptSafeDispose();

    // Clear variables
    mVisibility = CG_NULL;

    // Dispose base.
    if ( bDisposeBase == true )
        cgObjectNode::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_CameraNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : lockFrustum ()
/// <summary>
/// Prevent the frustum from updating.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraNode::lockFrustum( bool Locked )
{
    mFrustumLocked = Locked;
}

//-----------------------------------------------------------------------------
//  Name : isFrustumLocked ()
/// <summary>
/// Inform the caller whether or not the frustum is currently locked
/// This is useful as a debugging tool.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::isFrustumLocked( ) const
{
    return mFrustumLocked;
}

//-----------------------------------------------------------------------------
//  Name : setAspectRatio ()
/// <summary>
/// Set the aspect ratio that should be used to generate the horizontal
/// FOV angle (perspective only).
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraNode::setAspectRatio( cgFloat fAspect, bool bLocked /* = false */ )
{
    // Is this a no-op?
    if ( fAspect == mAspectRatio )
    {
        mAspectLocked = bLocked;
        return;
    
    } // End if aspect is the same

    // Update camera properties
    mAspectRatio    = fAspect;
    mAspectLocked   = bLocked;
    mAspectDirty    = true;
    mFrustumDirty   = true;
}

//-----------------------------------------------------------------------------
//  Name : getAspectRatio()
/// <summary>
/// Retrieve the aspect ratio used to generate the horizontal FOV angle.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraNode::getAspectRatio( ) const
{
    return mAspectRatio;
}

//-----------------------------------------------------------------------------
//  Name : isAspectLocked()
/// <summary>
/// Determine if the aspect ratio is currently being updated by the
/// render driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::isAspectLocked( ) const
{
    return (getProjectionMode() == cgProjectionMode::Orthographic || mAspectLocked);
}

//-----------------------------------------------------------------------------
//  Name : getProjectionMatrix ()
/// <summary>
/// Return the current projection matrix.
/// </summary>
//-----------------------------------------------------------------------------
const cgMatrix & cgCameraNode::getProjectionMatrix()
{
    // Only update matrix if something has changed
    if ( getProjectionMode() == cgProjectionMode::Perspective || (mInterlaceField != INTERLACING_OFF && mAspectDirty) )
    {
        if ( mProjectionDirty ) 
        {     
            // Generate the updated perspective projection matrix
            float fFOVRadians  = CGEToRadian(getFOV());
            cgMatrix::perspectiveFovLH( mProjectionMatrix, fFOVRadians, mAspectRatio, getNearClip(), getFarClip() );

            // If interlaced rendering is active, make appropriate adjustments to the projection matrix
            if ( mInterlaceField != INTERLACING_OFF )
                projectionInterlace();

            // Matrix has been updated
            mProjectionDirty   = false; 
            mAspectDirty = false;

        } // End if projection matrix needs updating
        else if ( mAspectDirty )
        {
            // Just alter the aspect ratio
            mProjectionMatrix._11 = mProjectionMatrix._22 / mAspectRatio;

            // Matrix has been updated
            mAspectDirty = false;

        } // End if only aspect ratio changed

    } // End if perspective
    else if ( getProjectionMode() == cgProjectionMode::Orthographic )
    {
        if ( mProjectionDirty || mAspectDirty ) 
        {
            // Generate the updated orthographic projection matrix
            cgFloat l, r, b, t, fZoom = getZoomFactor();
            getProjectionWindow( l, r, b, t );
            cgMatrix::orthoOffCenterLH( mProjectionMatrix, l * fZoom, r * fZoom, b * fZoom, t * fZoom, getNearClip(), getFarClip() );

            // If interlaced rendering is active, make appropriate adjustments to the projection matrix
            if ( mInterlaceField != INTERLACING_OFF )
                projectionInterlace();

            // Matrix has been updated
            mProjectionDirty   = false; 
            mAspectDirty = false;

        } // End if projection matrix needs updating

    } // End if orthographic

    // Return the projection matrix.
    return mProjectionMatrix;
}

//-----------------------------------------------------------------------------
//  Name : getViewMatrix ()
/// <summary>
/// Return the current view matrix.
/// </summary>
//-----------------------------------------------------------------------------
const cgMatrix & cgCameraNode::getViewMatrix()
{
    // Only update matrix if something has changed
    if ( mViewDirty ) 
    {
        cgVector3 vecRight = getXAxis( false ), vecUp = getYAxis( false ), vecLook = getZAxis( false );
        const cgVector3 & vecPosition = getPosition( false );

        // Because many rotations will cause floating point errors, the axis will eventually become
        // non-perpendicular to one other causing all hell to break loose. Therefore, we must
        // perform base vector regeneration to ensure that all vectors remain unit length and
        // perpendicular to one another. This need not be done on EVERY call to rotate (i.e. you
        // could do this once every 50 calls for instance).
        cgVector3::normalize( vecLook, vecLook );
        cgVector3::cross( vecRight, vecUp, vecLook );
        cgVector3::normalize( vecRight, vecRight );
        cgVector3::cross( vecUp, vecLook, vecRight );
        cgVector3::normalize( vecUp, vecUp );

        // Set view matrix values
        mViewMatrix._11 = vecRight.x; mViewMatrix._12 = vecUp.x; mViewMatrix._13 = vecLook.x;
        mViewMatrix._21 = vecRight.y; mViewMatrix._22 = vecUp.y; mViewMatrix._23 = vecLook.y;
        mViewMatrix._31 = vecRight.z; mViewMatrix._32 = vecUp.z; mViewMatrix._33 = vecLook.z;
        mViewMatrix._41 = -cgVector3::dot( vecPosition, vecRight );
        mViewMatrix._42 = -cgVector3::dot( vecPosition, vecUp    );
        mViewMatrix._43 = -cgVector3::dot( vecPosition, vecLook  );

        // View Matrix has been updated
        mViewDirty = false;

    } // End If View Dirty

    // Return the view matrix.
    return mViewMatrix;
}

//-----------------------------------------------------------------------------
//  Name : getFrustum()
/// <summary>
/// Retrieve the current camera object frustum.
/// </summary>
//-----------------------------------------------------------------------------
const cgFrustum & cgCameraNode::getFrustum( )
{
    // Reclaculate frustum if necessary
    if ( mFrustumDirty == true && mFrustumLocked == false )
    {
        mFrustum.update( getViewMatrix(), getProjectionMatrix() );
        mFrustumDirty = false;

        // Also build the frustum / volume that represents the space between the
        // camera position and its near plane. This frustum represents the
        // 'volume' that can end up clipping geometry.
        using namespace cgVolumeGeometry;
        using namespace cgVolumePlane;
        mClippingVolume = mFrustum;
        mClippingVolume.planes[ Far  ].d = -mClippingVolume.planes[ Near ].d; // At near plane
        mClippingVolume.planes[ Near ].d = -cgVector3::dot( (cgVector3&)mClippingVolume.planes[Near], getPosition(false) ); // At camera

        // The corner points also need adjusting in this case such that they sit
        // precisely on the new planes.
        mClippingVolume.points[ LeftBottomFar ]   = mClippingVolume.points[ LeftBottomNear ];
        mClippingVolume.points[ LeftTopFar ]      = mClippingVolume.points[ LeftTopNear ];
        mClippingVolume.points[ RightBottomFar ]  = mClippingVolume.points[ RightBottomNear ];
        mClippingVolume.points[ RightTopFar ]     = mClippingVolume.points[ RightTopNear ];
        mClippingVolume.points[ LeftBottomNear ]  = mClippingVolume.position;
        mClippingVolume.points[ LeftTopNear ]     = mClippingVolume.position;
        mClippingVolume.points[ RightBottomNear ] = mClippingVolume.position;
        mClippingVolume.points[ RightTopNear ]    = mClippingVolume.position;
    
    } // End if recalc frustum

    // Return the frustum
    return mFrustum;
}

//-----------------------------------------------------------------------------
//  Name : getClippingVolume()
/// <summary>
/// Retrieve the frustum / volume that represents the space between the camera 
/// position and its near plane. This frustum represents the 'volume' that can 
/// end up clipping geometry.
/// </summary>
//-----------------------------------------------------------------------------
const cgFrustum & cgCameraNode::getClippingVolume( )
{
    // Reclaculate frustum if necessary
    if ( mFrustumDirty == true && mFrustumLocked == false )
        getFrustum();
    
    // Return the clipping volume
    return mClippingVolume;
}

//-----------------------------------------------------------------------------
//  Name : boundsInFrustum ()
/// <summary>
/// Determine whether or not the AABB specified falls within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgCameraNode::boundsInFrustum( const cgBoundingBox & AABB, cgUInt8 * FrustumBits /* = CG_NULL */, cgInt8 * LastOutside /* = CG_NULL */ )
{
    // Recompute the frustum as necessary.
    const cgFrustum & Frustum = getFrustum();

    // Request that frustum classifies
    return Frustum.classifyAABB( AABB, CG_NULL, FrustumBits, LastOutside );
}

//-----------------------------------------------------------------------------
//  Name : boundsInFrustum ()
/// <summary>
/// Determine whether or not the OOBB specified is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgCameraNode::boundsInFrustum( const cgBoundingBox & AABB, const cgTransform & Transform, cgUInt8 * FrustumBits /* = CG_NULL */, cgInt8 * LastOutside /* = CG_NULL */ )
{
    // Recompute the frustum as necessary.
    const cgFrustum & Frustum = getFrustum();

    // Request that frustum classifies
    return Frustum.classifyAABB( AABB, &Transform, FrustumBits, LastOutside );
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform() (Override)
/// <summary>
/// Update our internal cell matrix with that specified here.
/// Note : Hooked into base class so that we can flag our own matrices as dirty.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgObjectNode::setCellTransform( Transform, Source ) )
        return false;

    // Flag our view matrix as dirty
    mViewDirty    = true;
    mFrustumDirty = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::canScale( ) const
{
    // Camera nodes cannot be scaled.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : worldToViewport()
/// <summary>
/// Transform a point from world space, into screen space. Returns false 
/// if the point was clipped off the screen.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::worldToViewport( const cgSize & ViewportSize, const cgVector3 & WorldPos, cgVector3 & ViewportPos, bool bClipX /* = true */, bool bClipY /* = true */, bool bClipZ /* = true */ )
{
    // Ensure we have an up-to-date projection and view matrix
    cgMatrix mtxTransform = getViewMatrix() * getProjectionMatrix();

    // Transform the point into clip space
    cgVector4 vClip;
    cgVector3::transform( vClip, WorldPos, mtxTransform );

    // Was this clipped?
    if ( bClipX == true && (vClip.x < -vClip.w || vClip.x > vClip.w) ) return false;
    if ( bClipY == true && (vClip.y < -vClip.w || vClip.y > vClip.w) ) return false;
    if ( bClipZ == true && (vClip.z < 0.0f || vClip.z > vClip.w) ) return false;

    // Project!
    const cgFloat recipW = 1.0f / vClip.w;
    vClip.x *= recipW;
    vClip.y *= recipW;
    vClip.z *= recipW;

    // Transform to final screen space position
    ViewportPos.x = ((vClip.x * 0.5f) + 0.5f) * (cgFloat)ViewportSize.width;
    ViewportPos.y = ((vClip.y * -0.5f) + 0.5f) * (cgFloat)ViewportSize.height;
    ViewportPos.z = vClip.z;

    // Point on screen!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : viewportToRay()
/// <summary>
/// Convert the specified screen position into a ray origin and direction
/// vector, suitable for use during picking.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::viewportToRay( const cgSize & ViewportSize, const cgVector2 & ViewportPos, cgVector3 & vecRayStart, cgVector3 & vecRayDir )
{
    cgVector3 vCursor;

    // Ensure we have an up-to-date projection and view matrix
    cgMatrix mtxInvView;
    cgMatrix mtxProj = getProjectionMatrix();
    cgMatrix mtxView = getViewMatrix();
    cgMatrix::inverse( mtxInvView, mtxView );
     
    // Transform the pick position from viewport space into camera space
    vCursor.x =  ( ( ( 2.0f * ViewportPos.x ) / (cgFloat)ViewportSize.width ) - 1 ) / mtxProj._11;
    vCursor.y = -( ( ( 2.0f * ViewportPos.y ) / (cgFloat)ViewportSize.height ) - 1 ) / mtxProj._22;
    vCursor.z =  1.0f;

    // Transform the camera space pick ray into 3D space
    if ( getProjectionMode() == cgProjectionMode::Orthographic ) 
    {
        // Obtain the ray from the cursor pos
        cgVector3::transformCoord( vecRayStart, vCursor, mtxInvView );
        vecRayDir = (cgVector3&)mtxInvView._31;

    } // End If IsOrthohraphic
    else 
    {
        // Obtain the ray from the cursor pos
        vecRayStart.x = mtxInvView._41;
        vecRayStart.y = mtxInvView._42;
        vecRayStart.z = mtxInvView._43;
        vecRayDir.x   = vCursor.x * mtxInvView._11 + vCursor.y * mtxInvView._21 + vCursor.z * mtxInvView._31;
        vecRayDir.y   = vCursor.x * mtxInvView._12 + vCursor.y * mtxInvView._22 + vCursor.z * mtxInvView._32;
        vecRayDir.z   = vCursor.x * mtxInvView._13 + vCursor.y * mtxInvView._23 + vCursor.z * mtxInvView._33;

    } // End If !IsOrthohraphic

    // Normalize the ray direction
    cgVector3::normalize( vecRayDir, vecRayDir );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : viewportToWorld ()
/// <summary>
/// Given a view screen position (in screen space) this function will cast that 
/// ray and return the world space position on the specified plane. The value
/// is returned via the world parameter passed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::viewportToWorld( const cgSize & ViewportSize, const cgVector2 & ViewportPos, const cgPlane & Plane, cgVector3 & WorldPos )
{
    cgVector3 vPickRayDir, vPickRayOrig;
    cgFloat   fProjRayLength, fDistance;

    // Convert the screen coordinates to a ray.
    if ( viewportToRay( ViewportSize, ViewportPos, vPickRayOrig, vPickRayDir ) == false )
        return false;
    
    // Get the length of the 'adjacent' side of the virtual triangle formed
    // by the direction and normal.
    fProjRayLength = cgVector3::dot( vPickRayDir, (cgVector3&)Plane );
    if ( fabsf( fProjRayLength ) < CGE_EPSILON )
        return false;

    // Calculate distance to plane along its normal
    fDistance = cgVector3::dot( vPickRayOrig, (cgVector3&)Plane ) + Plane.d;

    // If both the "direction" and origin are on the same side of the plane
    // then we can't possibly intersect (perspective rule only)
    if ( getProjectionMode() == cgProjectionMode::Perspective )
    {
        cgInt nSign1 = (fDistance > 0) ? 1 : (fDistance < 0) ? -1 : 0;
        cgInt nSign2 = (fProjRayLength > 0) ? 1 : (fProjRayLength < 0) ? -1 : 0;
        if ( nSign1 == nSign2 )
            return false;
    
    } // End if perspective

    // Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
    fDistance /= -fProjRayLength;

    // Store the results
    WorldPos = vPickRayOrig + (vPickRayDir * fDistance);

    // Success!
	return true;
}

//-----------------------------------------------------------------------------
//  Name : viewportToMajorAxis ()
/// <summary>
/// Given a view screen position (in screen space) this function will cast that 
/// ray and return the world space intersection point on one of the major axis
/// planes selected based on the camera look vector.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::viewportToMajorAxis( const cgSize & ViewportSize, const cgVector2 & ViewportPos, const cgVector3 & Origin, cgVector3 & WorldPos, cgVector3 & MajorAxis )
{
    return viewportToMajorAxis( ViewportSize, ViewportPos, Origin, getZAxis( false ), WorldPos, MajorAxis );
}

//-----------------------------------------------------------------------------
//  Name : viewportToMajorAxis ()
/// <summary>
/// Given a view screen position (in screen space) this function will cast that 
/// ray and return the world space intersection point on one of the major axis
/// planes selected based on the specified normal.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::viewportToMajorAxis( const cgSize & ViewportSize, const cgVector2 & ViewportPos, const cgVector3 & Origin, const cgVector3 & Normal, cgVector3 & WorldPos, cgVector3 & MajorAxis )
{
    // First select the major axis plane based on the specified normal
    MajorAxis = cgVector3( 1, 0, 0 ); // YZ

    // Get absolute normal vector
    cgFloat x = fabsf( Normal.x );
    cgFloat y = fabsf( Normal.y );
    cgFloat z = fabsf( Normal.z );

    // If all the components are effectively equal, select one plane
    if ( fabsf( x - y ) < CGE_EPSILON && fabsf( x - z ) < CGE_EPSILON )
    {
        MajorAxis = cgVector3( 0, 0, 1 ); // XY
    
    } // End if components equal
    else
    {
        // Calculate which component of the normal is the major axis
        cgFloat fNorm = x;
        if (fNorm < y) { fNorm = y; MajorAxis = cgVector3( 0, 1, 0 ); } // XZ
        if (fNorm < z) { fNorm = z; MajorAxis = cgVector3( 0, 0, 1 ); } // XY

    } // End if perform compare

    // Generate the intersection plane based on this information
    // and pass through to the standard viewportToWorld method
    cgPlane Plane;
    cgPlane::fromPointNormal( Plane, Origin, MajorAxis );
    return viewportToWorld( ViewportSize, ViewportPos, Plane, WorldPos );
}

//-----------------------------------------------------------------------------
//  Name : viewportToCamera ()
/// <summary>
/// Given a view screen position (in screen space) this function will convert
/// the point into a camera space position at the near plane.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraNode::viewportToCamera( const cgSize & ViewportSize, const cgVector3 & ViewportPos, cgVector3 & CameraPos )
{
    // Ensure that we have an up-to-date projection and view matrix
    const cgMatrix & mtxProj    = getProjectionMatrix();
    const cgMatrix & mtxView    = getViewMatrix();

    // Transform the pick position from screen space into camera space
    CameraPos.x =  ( ( ( 2.0f * ViewportPos.x ) / (cgFloat)ViewportSize.width ) - 1 ) / mtxProj._11;
    CameraPos.y = -( ( ( 2.0f * ViewportPos.y ) / (cgFloat)ViewportSize.height ) - 1 ) / mtxProj._22;
    CameraPos.z =  getNearClip();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : estimateZoomFactor ()
/// <summary>
/// Given the current viewport type and projection mode, estimate the "zoom"
/// factor that can be used for scaling various operations relative to their
/// "scale" as it appears in the viewport.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraNode::estimateZoomFactor( const cgSize & ViewportSize, const cgPlane & Plane )
{
    cgVector3 vWorld;
    
    // Just return the actual zoom factor if this is orthographic
    if ( getProjectionMode() == cgProjectionMode::Orthographic )
        return getZoomFactor();

    // Otherwise, estimate is based on the distance from the grid plane.
    viewportToWorld( ViewportSize, cgVector2( (cgFloat)ViewportSize.width / 2, (cgFloat)ViewportSize.height / 2 ), Plane, vWorld );

    // Perform full position based estimation
    return estimateZoomFactor( ViewportSize, vWorld );
}

//-----------------------------------------------------------------------------
// Name : estimateZoomFactor ()
/// <summary>
/// Given the current viewport type and projection mode, estimate the "zoom"
/// factor that can be used for scaling various operations relative to the
/// "scale" of an object as it appears in the viewport at the specified position.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraNode::estimateZoomFactor( const cgSize & ViewportSize, const cgVector3 & WorldPos )
{
    return estimateZoomFactor( ViewportSize, WorldPos, FLT_MAX );
}

//-----------------------------------------------------------------------------
//  Name : estimateZoomFactor ()
/// <summary>
/// Given the current viewport type and projection mode, estimate the "zoom"
/// factor that can be used for scaling various operations relative to their
/// "scale" as it appears in the viewport.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraNode::estimateZoomFactor( const cgSize & ViewportSize, const cgPlane & Plane, cgFloat fMax )
{
    // Just return the actual zoom factor if this is orthographic
    if ( getProjectionMode() == cgProjectionMode::Orthographic )
    {
        cgFloat fFactor = getZoomFactor();
        return min( fMax, fFactor );
    
    } // End if Orthographic

    // Otherwise, estimate is based on the distance from the grid plane.
    cgVector3 vWorld;
    viewportToWorld( ViewportSize, cgVector2( (cgFloat)ViewportSize.width / 2, (cgFloat)ViewportSize.height / 2 ), Plane, vWorld );

    // Perform full position based estimation
    return estimateZoomFactor( ViewportSize, vWorld, fMax );
}

//-----------------------------------------------------------------------------
// Name : estimateZoomFactor ()
/// <summary>
/// Given the current viewport type and projection mode, estimate the "zoom"
/// factor that can be used for scaling various operations relative to the
/// "scale" of an object as it appears in the viewport at the specified position.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCameraNode::estimateZoomFactor( const cgSize & ViewportSize, const cgVector3 & WorldPos, cgFloat fMax )
{
    // Just return the actual zoom factor if this is orthographic
    if ( getProjectionMode() == cgProjectionMode::Orthographic )
    {
        cgFloat fFactor = getZoomFactor();
        return min( fMax, fFactor );
    
    } // End if Orthographic

    // New Zoom factor is based on the distance to this position 
    // along the camera's look vector.
    cgVector3 viewPos;
    cgVector3::transformCoord( viewPos, WorldPos, getViewMatrix() );
    cgFloat distance = viewPos.z / ((cgFloat)ViewportSize.height * (45.0f / getFOV()) );
    return min( fMax, distance );
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Update the scene object.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraNode::update( cgFloat fElapsedTime )
{
    // Automatically record matrices last used in the previous frame.
    recordCurrentMatrices();

    // Call base class implementation
    cgObjectNode::update( fElapsedTime );
}

//-----------------------------------------------------------------------------
//  Name : getVisibilitySet ()
/// <summary>
/// Retrieve the camera's visibility set.
/// </summary>
//-----------------------------------------------------------------------------
cgVisibilitySet * cgCameraNode::getVisibilitySet( )
{
    return mVisibility;
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility ()
/// <summary>
/// compute the visibility set from the point of view of this camera.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraNode::computeVisibility( cgUInt32 nFlags /* = MustRender | CollectMaterials */, bool bAutoApply /* = false */ )
{
    mVisibility->compute( mParentScene, getFrustum(), nFlags, bAutoApply );
}

//-----------------------------------------------------------------------------
//  Name : applyVisibility ()
/// <summary>
/// Apply the visibility set from the point of view of this camera.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraNode::applyVisibility()
{
    mVisibility->apply();
}

//-----------------------------------------------------------------------------
//  Name : recordCurrentMatrices ()
/// <summary>
/// Make a copy of the current view / projection matrices before they
/// are changed. Useful for performing effects such as motion blur.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraNode::recordCurrentMatrices( )
{
    mPreviousViewMatrix = getViewMatrix();
    mPreviousProjectionMatrix = getProjectionMatrix();
}

//-----------------------------------------------------------------------------
//  Name : getPreviousViewMatrix ()
/// <summary>
/// Retrieve a copy of the view matrix recorded with the most recent call
/// to recordCurrentMatrices().
/// </summary>
//-----------------------------------------------------------------------------
const cgMatrix & cgCameraNode::getPreviousViewMatrix() const
{
    return mPreviousViewMatrix;
}

//-----------------------------------------------------------------------------
//  Name : getPreviousProjectionMatrix ()
/// <summary>
/// Retrieve a copy of the projection matrix recorded with the most
/// recent call to recordCurrentMatrices().
/// </summary>
//-----------------------------------------------------------------------------
const cgMatrix & cgCameraNode::getPreviousProjectionMatrix() const
{
    return mPreviousProjectionMatrix;
}

//-----------------------------------------------------------------------------
// Name : setInterlaceField ()
// Desc : Sets a new value for the current interlacing field
//-----------------------------------------------------------------------------
void cgCameraNode::setInterlaceField( cgUInt32 nField )
{
	// Set the new field
	mInterlaceField = nField;

	// Flag the matrix as dirty
	mProjectionDirty    = true;
	mAspectLocked = false;
}

//-----------------------------------------------------------------------------
// Name : setJitterAA ()
// Desc : Sets the current jittering value for temporal anti-aliasing
//-----------------------------------------------------------------------------
void cgCameraNode::setJitterAA( const cgVector2 & jitter )
{
	mJitterAA = jitter;

	cgRenderDriver * pDriver = mParentScene->getRenderDriver();
	cgFloat fWidth  = (cgFloat)pDriver->getConfig().width;
	cgFloat fHeight = (cgFloat)pDriver->getConfig().height;
	mJitterAA.x *= (2.0f / fWidth); 
	mJitterAA.y *= (2.0f / fHeight);
}

//-----------------------------------------------------------------------------
// Name : getJitterAA ()
// Desc : Gets the current jittering value for temporal anti-aliasing
//-----------------------------------------------------------------------------
cgVector2 cgCameraNode::getJitterAA( )
{
	return mJitterAA;
}

//-----------------------------------------------------------------------------
// Name : viewportMatrix ()
// Desc : Basic viewport matrix construction
//-----------------------------------------------------------------------------
// ToDo: 6767 -- yeurgh
cgMatrix viewportMatrix(float width, float height, float sx, float sy, float minZ, float maxZ)
{
	return cgMatrix( width/2, 0, 0, 0,
				     0, -height/2, 0, 0,
				     0, 0,	maxZ-minZ, 0,
                     sx+width/2, height/2+sy, minZ,	1 );
};

//-----------------------------------------------------------------------------
// Name : projectionInterlace ()
// Desc : Adjusts the current projection matrix according to what interlacing field is active.
//-----------------------------------------------------------------------------
void cgCameraNode::projectionInterlace()
{
 	// Get the frame buffer dimensions
	cgRenderDriver * pDriver = mParentScene->getRenderDriver();
	DWORD nWidth  = pDriver->getConfig().width;
	DWORD nHeight = pDriver->getConfig().height;

	// Compute amount to shift for this field
	float shift = (mInterlaceField == TOP_FIELD) ? 0.0f : -0.5f;

	// Compute standard and desired viewport matrices
	cgMatrix BaseVP, ShiftedVP, mtxShifted, mtxInvBase;
	BaseVP    =	viewportMatrix( (float)nWidth, (float)nHeight/2, 0,	0,		 0, 1 );	
	ShiftedVP =	viewportMatrix( (float)nWidth, (float)nHeight/2, 0,	0+shift, 0, 1 );
	cgMatrix::multiply( mtxShifted, mProjectionMatrix, ShiftedVP );
	cgMatrix::inverse( mtxInvBase, BaseVP );
	cgMatrix::multiply( mProjectionMatrix, mtxShifted, mtxInvBase ); 
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // All modifications require projection matrix and
    // frustum to be updated.
    mProjectionDirty    = true;
    mFrustumDirty = true;

    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}