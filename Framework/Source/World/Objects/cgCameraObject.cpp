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
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
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
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgSurfaceShader.h>
#include <System/cgMessageTypes.h>
#include <Math/cgCollision.h>

#define INTERLACING_OFF  -1
#define TOP_FIELD         0
#define BOTTOM_FIELD      1

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgCameraObject::mInsertCamera;
cgWorldQuery cgCameraObject::mUpdateProjection;
cgWorldQuery cgCameraObject::mLoadCamera;

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
    mProjectionMode      = cgProjectionMode::Perspective;
    mFOV                 = 60.0f;
    mNearClip            = 0.2f;
    mFarClip             = 500.0f;
    mProjectionWindow    = cgVector4( 0, 0, 0, 0 );
    mZoomFactor          = 1.0f;
    mForegroundExtents   = cgRangeF(0,0);
    mBackgroundExtents   = cgRangeF(0,0);
    mDepthOfFieldEnabled = false;
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
    mProjectionMode      = pObject->mProjectionMode;
    mFOV                 = pObject->mFOV;
    mNearClip            = pObject->mNearClip;
    mProjectionWindow    = pObject->mProjectionWindow;
    mZoomFactor          = pObject->mZoomFactor;
    mForegroundExtents   = pObject->mForegroundExtents;
    mBackgroundExtents   = pObject->mBackgroundExtents;
    mBackgroundHighBlur  = pObject->mBackgroundHighBlur;
    mBackgroundLowBlur   = pObject->mBackgroundLowBlur;
    mForegroundHighBlur  = pObject->mForegroundHighBlur;
    mForegroundLowBlur   = pObject->mForegroundLowBlur;
    mDepthOfFieldEnabled = pObject->mDepthOfFieldEnabled;
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
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("CameraObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertCamera.bindParameter( 1, mReferenceId );
        mInsertCamera.bindParameter( 2, (cgUInt32)mProjectionMode );
        mInsertCamera.bindParameter( 3, mFOV );
        mInsertCamera.bindParameter( 4, mNearClip );
        mInsertCamera.bindParameter( 5, mFarClip );
        mInsertCamera.bindParameter( 6, mZoomFactor );
        mInsertCamera.bindParameter( 7, mDepthOfFieldEnabled );
        mInsertCamera.bindParameter( 8, mForegroundExtents.min );
        mInsertCamera.bindParameter( 9, mForegroundExtents.max );
        mInsertCamera.bindParameter( 10, mForegroundHighBlur.passCount );
        mInsertCamera.bindParameter( 11, mForegroundHighBlur.pixelRadiusH );
        mInsertCamera.bindParameter( 12, mForegroundHighBlur.distanceFactorH );
        mInsertCamera.bindParameter( 13, mForegroundLowBlur.passCount );
        mInsertCamera.bindParameter( 14, mForegroundLowBlur.pixelRadiusH );
        mInsertCamera.bindParameter( 15, mForegroundLowBlur.distanceFactorH );
        mInsertCamera.bindParameter( 16, mBackgroundExtents.min );
        mInsertCamera.bindParameter( 17, mBackgroundExtents.max );
        mInsertCamera.bindParameter( 18, mBackgroundHighBlur.passCount );
        mInsertCamera.bindParameter( 19, mBackgroundHighBlur.pixelRadiusH );
        mInsertCamera.bindParameter( 20, mBackgroundHighBlur.distanceFactorH );
        mInsertCamera.bindParameter( 21, mBackgroundLowBlur.passCount );
        mInsertCamera.bindParameter( 22, mBackgroundLowBlur.pixelRadiusH );
        mInsertCamera.bindParameter( 23, mBackgroundLowBlur.distanceFactorH );
        mInsertCamera.bindParameter( 24, mSoftRefCount );

        // Execute
        if ( !mInsertCamera.step( true ) )
        {
            cgString strError;
            mInsertCamera.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for camera object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("CameraObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("CameraObject::insertComponentData") );

    } // End if !internal

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentLoading() (Virtual)
/// <summary>
/// Virtual method called when the component is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the camera data.
    prepareQueries();
    mLoadCamera.bindParameter( 1, e->sourceRefId );
    if ( !mLoadCamera.step( ) || !mLoadCamera.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( mLoadCamera.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for camera object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for camera object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadCamera.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadCamera;

    // Update our local members
    cgFloat distanceFactor;
    cgUInt32 mode, passes, pixelRadius;
    mLoadCamera.getColumn( _T("ProjectionMode"), mode );
    mLoadCamera.getColumn( _T("NearClip"), mNearClip );
    mLoadCamera.getColumn( _T("FarClip"), mFarClip );
    mLoadCamera.getColumn( _T("ZoomFactor"), mZoomFactor );
    mLoadCamera.getColumn( _T("DoFEnabled"), mDepthOfFieldEnabled );
    mLoadCamera.getColumn( _T("DoFFGMin"), mForegroundExtents.min );
    mLoadCamera.getColumn( _T("DoFFGMax"), mForegroundExtents.max );
    mLoadCamera.getColumn( _T("DoFFGPassesHigh"), passes );
    mLoadCamera.getColumn( _T("DoFFGPixelRadiusHigh"), pixelRadius );
    mLoadCamera.getColumn( _T("DoFFGDistanceFactorHigh"), distanceFactor );
    mForegroundHighBlur = cgBlurOpDesc( passes, pixelRadius, distanceFactor );
    mLoadCamera.getColumn( _T("DoFFGPassesLow"), passes );
    mLoadCamera.getColumn( _T("DoFFGPixelRadiusLow"), pixelRadius );
    mLoadCamera.getColumn( _T("DoFFGDistanceFactorLow"), distanceFactor );
    mForegroundLowBlur = cgBlurOpDesc( passes, pixelRadius, distanceFactor );
    mLoadCamera.getColumn( _T("DoFBGMin"), mBackgroundExtents.min );
    mLoadCamera.getColumn( _T("DoFBGMax"), mBackgroundExtents.max );
    mLoadCamera.getColumn( _T("DoFBGPassesHigh"), passes );
    mLoadCamera.getColumn( _T("DoFBGPixelRadiusHigh"), pixelRadius );
    mLoadCamera.getColumn( _T("DoFBGDistanceFactorHigh"), distanceFactor );
    mBackgroundHighBlur = cgBlurOpDesc( passes, pixelRadius, distanceFactor );
    mLoadCamera.getColumn( _T("DoFBGPassesLow"), passes );
    mLoadCamera.getColumn( _T("DoFBGPixelRadiusLow"), pixelRadius );
    mLoadCamera.getColumn( _T("DoFBGDistanceFactorLow"), distanceFactor );
    mBackgroundLowBlur = cgBlurOpDesc( passes, pixelRadius, distanceFactor );
    mProjectionMode = (cgProjectionMode::Base)mode;

    // Call base class implementation to read remaining data.
    if ( !cgWorldObject::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        if ( !insertComponentData() )
            return false;

    } // End if cloned

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertCamera.isPrepared( mWorld ) )
            mInsertCamera.prepare( mWorld, _T("INSERT INTO 'Objects::Camera' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21,?22,?23,?24)"), true );
        if ( !mUpdateProjection.isPrepared( mWorld ) )
            mUpdateProjection.prepare( mWorld, _T("UPDATE 'Objects::Camera' SET ProjectionMode=?1, FOV=?2, NearClip=?3, FarClip=?4, ZoomFactor=?5 WHERE RefId=?6"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadCamera.isPrepared( mWorld ) )
        mLoadCamera.prepare( mWorld, _T("SELECT * FROM 'Objects::Camera' WHERE RefId=?1"), true );
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

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProjection.bindParameter( 1, (cgUInt32)mProjectionMode );
        mUpdateProjection.bindParameter( 2, mFOV );
        mUpdateProjection.bindParameter( 3, mNearClip );
        mUpdateProjection.bindParameter( 4, mFarClip );
        mUpdateProjection.bindParameter( 5, fZoom );
        mUpdateProjection.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( mUpdateProjection.step( true ) == false )
        {
            cgString strError;
            mUpdateProjection.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update projection properties for camera object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

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

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProjection.bindParameter( 1, (cgUInt32)mProjectionMode );
        mUpdateProjection.bindParameter( 2, fFOVY );
        mUpdateProjection.bindParameter( 3, mNearClip );
        mUpdateProjection.bindParameter( 4, mFarClip );
        mUpdateProjection.bindParameter( 5, mZoomFactor );
        mUpdateProjection.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( mUpdateProjection.step( true ) == false )
        {
            cgString strError;
            mUpdateProjection.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update projection properties for camera object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

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

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProjection.bindParameter( 1, (cgUInt32)Mode );
        mUpdateProjection.bindParameter( 2, mFOV );
        mUpdateProjection.bindParameter( 3, mNearClip );
        mUpdateProjection.bindParameter( 4, mFarClip );
        mUpdateProjection.bindParameter( 5, mZoomFactor );
        mUpdateProjection.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( mUpdateProjection.step( true ) == false )
        {
            cgString strError;
            mUpdateProjection.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update projection properties for camera object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

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

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProjection.bindParameter( 1, (cgUInt32)mProjectionMode );
        mUpdateProjection.bindParameter( 2, mFOV );
        mUpdateProjection.bindParameter( 3, fDistance );
        mUpdateProjection.bindParameter( 4, mFarClip );
        mUpdateProjection.bindParameter( 5, mZoomFactor );
        mUpdateProjection.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( mUpdateProjection.step( true ) == false )
        {
            cgString strError;
            mUpdateProjection.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update projection properties for camera object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store value
    mNearClip = fDistance;

    // Notify listeners that property was altered
    static const cgString strContext = _T("NearClip");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure near clip is less than the far clip
    if ( mNearClip > mFarClip )
        setFarClip( mNearClip );
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

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProjection.bindParameter( 1, (cgUInt32)mProjectionMode );
        mUpdateProjection.bindParameter( 2, mFOV );
        mUpdateProjection.bindParameter( 3, mNearClip );
        mUpdateProjection.bindParameter( 4, fDistance );
        mUpdateProjection.bindParameter( 5, mZoomFactor );
        mUpdateProjection.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( mUpdateProjection.step( true ) == false )
        {
            cgString strError;
            mUpdateProjection.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update projection properties for camera object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store value
    mFarClip = fDistance;

    // Notify listeners that property was altered
    static const cgString strContext = _T("FarClip");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure near clip is less than the far clip
    if ( mNearClip > mFarClip )
        setNearClip( mFarClip );
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
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgCameraObject::getLocalBoundingBox( )
{
    cgFloat fNearSize = tanf(CGEToRadian(mFOV*0.5f)) * mNearClip;
    cgFloat fFarSize  = tanf(CGEToRadian(mFOV*0.5f)) * mFarClip;
    return cgBoundingBox( -fFarSize, -fFarSize, mNearClip, fFarSize, fFarSize, mFarClip ); 
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, cgUInt32 nFlags, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Retrieve useful values
    cgFloat fZoomFactor  = pCamera->estimateZoomFactor( ViewportSize, pIssuer->getPosition(), 2.5f );
    fZoomFactor *= 0.5f;
    cgFloat fSize = fZoomFactor * 17.5f;
    
    // Targets are picked as if they were solid boxes.
    cgBoundingBox Bounds( -fSize, -fSize * 1.5f, -88.0f * fZoomFactor, fSize, fSize * 1.5f, 0.0f );
    Bounds.inflate( pCamera->estimatePickTolerance( ViewportSize, fWireTolerance, Bounds.getCenter(), pIssuer->getWorldTransform(false) ) );
    return Bounds.intersect( vOrigin, vDir, fDistance, false );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // ToDo: Camera should not show additional frustra
    // if they are selected only as part of a selected closed group.
    
    // Get access to required systems.
    cgRenderDriver  * pDriver = cgRenderDriver::getInstance();
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);

    // Set the object's transformation matrix to the device (light
    // representation will be constructed in object space)
    cgTransform InverseObjectTransform;
    const cgTransform & ObjectTransform = pIssuer->getWorldTransform( false );
    cgTransform::inverse( InverseObjectTransform, ObjectTransform );
    pDriver->setWorldTransform( ObjectTransform );

    // Retrieve useful values
    const cgViewport & Viewport = pDriver->getViewport();
    bool       bOrtho      = (pCamera->getProjectionMode() == cgProjectionMode::Orthographic);
    bool       bSelected   = pIssuer->isSelected();
    cgFloat    fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, pIssuer->getPosition( false ), 2.5f );
    cgUInt32   nColor      = (bSelected) ? 0xFFFFFFFF : 0xFF4BA1DB;

    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    cgShadedVertex Points[32];
    for ( cgInt i = 0; i < 24; ++i )
        Points[i].color = nColor;
    for ( cgInt i = 24; i < 32; ++i )
        Points[i].color = 0xFF99CCE5;

    // Begin rendering
    bool bWireframe = (flags & cgSandboxRenderFlags::Wireframe);
    pDriver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    pShader->setBool( _T("wireViewport"), bWireframe );
    pDriver->setWorldTransform( ObjectTransform );
    if ( pShader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( pShader->executeTechniquePass() != cgTechniqueResult::Abort )
        {
            cgUInt32 Indices[168], nCount = 0;
            bool bCull;
            cgPlane Plane;

            // Now we'll render the base of the arrow, the "stalk" and the back plate.
            // So that we can construct in object space, transform camera culling details
            // into object space too.
            fZoomFactor *= 0.5f;
            cgVector3 vCameraLook, vCameraPos;
            InverseObjectTransform.transformNormal( vCameraLook, pCamera->getZAxis(false) );
            InverseObjectTransform.transformCoord( vCameraPos, pCamera->getPosition(false) );
            cgVector3::normalize( vCameraLook, vCameraLook );

            // Compute vertices for the camera lens coweling
            cgFloat fSize = fZoomFactor * 8.5f;
            Points[0].position = cgVector3( -fSize,  fSize, fZoomFactor * -8.0f );
            Points[1].position = cgVector3( -fSize, -fSize, fZoomFactor * -8.0f );
            Points[2].position = cgVector3(  fSize, -fSize, fZoomFactor * -8.0f );
            Points[3].position = cgVector3(  fSize,  fSize, fZoomFactor * -8.0f );
            fSize = fZoomFactor * 17.5f;
            Points[4].position = cgVector3( -fSize,  fSize, fZoomFactor * 0.0f );
            Points[5].position = cgVector3( -fSize, -fSize, fZoomFactor * 0.0f );
            Points[6].position = cgVector3(  fSize, -fSize, fZoomFactor * 0.0f );
            Points[7].position = cgVector3(  fSize,  fSize, fZoomFactor * 0.0f );
            
            // Cowel +X Face
            cgPlane::fromPoints( Plane, Points[2].position, Points[3].position, Points[7].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 2; Indices[nCount++] = 3;
                Indices[nCount++] = 3; Indices[nCount++] = 7;
                Indices[nCount++] = 7; Indices[nCount++] = 6;
                Indices[nCount++] = 6; Indices[nCount++] = 2;

            } // End if !cull

            // Cowel -X Face
            cgPlane::fromPoints( Plane, Points[1].position, Points[5].position, Points[4].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 1; Indices[nCount++] = 5;
                Indices[nCount++] = 5; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 0;
                Indices[nCount++] = 0; Indices[nCount++] = 1;

            } // End if !cull

            // Cowel +Y Face
            cgPlane::fromPoints( Plane, Points[0].position, Points[4].position, Points[7].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 0; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 7;
                Indices[nCount++] = 7; Indices[nCount++] = 3;
                Indices[nCount++] = 3; Indices[nCount++] = 0;

            } // End if !cull

            // Cowel -Y Face
            cgPlane::fromPoints( Plane, Points[2].position, Points[6].position, Points[5].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 2; Indices[nCount++] = 6;
                Indices[nCount++] = 6; Indices[nCount++] = 5;
                Indices[nCount++] = 5; Indices[nCount++] = 1;
                Indices[nCount++] = 1; Indices[nCount++] = 2;

            } // End if !cull

            // Cowel +Z Face
            cgPlane::fromPoints( Plane, Points[5].position, Points[6].position, Points[7].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 5; Indices[nCount++] = 6;
                Indices[nCount++] = 6; Indices[nCount++] = 7;
                Indices[nCount++] = 7; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 5;

            } // End if !cull

            // Cowel -Z Face
            cgPlane::fromPoints( Plane, Points[1].position, Points[0].position, Points[3].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 1; Indices[nCount++] = 0;
                Indices[nCount++] = 0; Indices[nCount++] = 3;
                Indices[nCount++] = 3; Indices[nCount++] = 2;
                Indices[nCount++] = 2; Indices[nCount++] = 1;

            } // End if !cull

            // Compute vertices for the lens coweling
            fSize = fZoomFactor * 5.5f;
            Points[8].position = cgVector3( -fSize,  fSize, fZoomFactor * -18.0f );
            Points[9].position = cgVector3( -fSize, -fSize, fZoomFactor * -18.0f );
            Points[10].position = cgVector3(  fSize, -fSize, fZoomFactor * -18.0f );
            Points[11].position = cgVector3(  fSize,  fSize, fZoomFactor * -18.0f );
            Points[12].position = cgVector3( -fSize,  fSize, fZoomFactor * -8.0f );
            Points[13].position = cgVector3( -fSize, -fSize, fZoomFactor * -8.0f );
            Points[14].position = cgVector3(  fSize, -fSize, fZoomFactor * -8.0f );
            Points[15].position = cgVector3(  fSize,  fSize, fZoomFactor * -8.0f );
            
            // Lens +X Face
            cgPlane::fromPoints( Plane, Points[10].position, Points[11].position, Points[15].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 10; Indices[nCount++] = 11;
                Indices[nCount++] = 11; Indices[nCount++] = 15;
                Indices[nCount++] = 15; Indices[nCount++] = 14;
                Indices[nCount++] = 14; Indices[nCount++] = 10;

            } // End if !cull

            // Lens -X Face
            cgPlane::fromPoints( Plane, Points[9].position, Points[13].position, Points[12].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 9; Indices[nCount++] = 13;
                Indices[nCount++] = 13; Indices[nCount++] = 12;
                Indices[nCount++] = 12; Indices[nCount++] = 8;
                Indices[nCount++] = 8; Indices[nCount++] = 9;

            } // End if !cull

            // Lens +Y Face
            cgPlane::fromPoints( Plane, Points[8].position, Points[12].position, Points[15].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 8; Indices[nCount++] = 12;
                Indices[nCount++] = 12; Indices[nCount++] = 15;
                Indices[nCount++] = 15; Indices[nCount++] = 11;
                Indices[nCount++] = 11; Indices[nCount++] = 8;

            } // End if !cull

            // Lens -Y Face
            cgPlane::fromPoints( Plane, Points[10].position, Points[14].position, Points[13].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 10; Indices[nCount++] = 14;
                Indices[nCount++] = 14; Indices[nCount++] = 13;
                Indices[nCount++] = 13; Indices[nCount++] = 9;
                Indices[nCount++] = 9; Indices[nCount++] = 10;

            } // End if !cull

            // Lens +Z Face
            cgPlane::fromPoints( Plane, Points[13].position, Points[14].position, Points[15].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 13; Indices[nCount++] = 14;
                Indices[nCount++] = 14; Indices[nCount++] = 15;
                Indices[nCount++] = 15; Indices[nCount++] = 12;
                Indices[nCount++] = 12; Indices[nCount++] = 13;

            } // End if !cull

            // Lens -Z Face
            cgPlane::fromPoints( Plane, Points[9].position, Points[8].position, Points[11].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 9; Indices[nCount++] = 8;
                Indices[nCount++] = 8; Indices[nCount++] = 11;
                Indices[nCount++] = 11; Indices[nCount++] = 10;
                Indices[nCount++] = 10; Indices[nCount++] = 9;

            } // End if !cull

            // Compute vertices for the main body
            fSize = fZoomFactor * 17.5f;
            Points[16].position = cgVector3( -fSize,  fSize * 1.f - fSize * 0.5f, fZoomFactor * -88.0f );
            Points[17].position = cgVector3( -fSize, -fSize * 1.f - fSize * 0.5f, fZoomFactor * -88.0f );
            Points[18].position = cgVector3(  fSize, -fSize * 1.f - fSize * 0.5f, fZoomFactor * -88.0f );
            Points[19].position = cgVector3(  fSize,  fSize * 1.f - fSize * 0.5f, fZoomFactor * -88.0f );
            Points[20].position = cgVector3( -fSize,  fSize * 1.5f, fZoomFactor * -18.0f );
            Points[21].position = cgVector3( -fSize, -fSize * 1.5f, fZoomFactor * -18.0f );
            Points[22].position = cgVector3(  fSize, -fSize * 1.5f, fZoomFactor * -18.0f );
            Points[23].position = cgVector3(  fSize,  fSize * 1.5f, fZoomFactor * -18.0f );
            
            // Body +X Face
            cgPlane::fromPoints( Plane, Points[18].position, Points[19].position, Points[23].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 18; Indices[nCount++] = 19;
                Indices[nCount++] = 19; Indices[nCount++] = 23;
                Indices[nCount++] = 23; Indices[nCount++] = 22;
                Indices[nCount++] = 22; Indices[nCount++] = 18;

            } // End if !cull

            // Body -X Face
            cgPlane::fromPoints( Plane, Points[17].position, Points[21].position, Points[20].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 17; Indices[nCount++] = 21;
                Indices[nCount++] = 21; Indices[nCount++] = 20;
                Indices[nCount++] = 20; Indices[nCount++] = 16;
                Indices[nCount++] = 16; Indices[nCount++] = 17;

            } // End if !cull

            // Body +Y Face
            cgPlane::fromPoints( Plane, Points[16].position, Points[20].position, Points[23].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 16; Indices[nCount++] = 20;
                Indices[nCount++] = 20; Indices[nCount++] = 23;
                Indices[nCount++] = 23; Indices[nCount++] = 19;
                Indices[nCount++] = 19; Indices[nCount++] = 16;

            } // End if !cull

            // Body -Y Face
            cgPlane::fromPoints( Plane, Points[18].position, Points[22].position, Points[21].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 18; Indices[nCount++] = 22;
                Indices[nCount++] = 22; Indices[nCount++] = 21;
                Indices[nCount++] = 21; Indices[nCount++] = 17;
                Indices[nCount++] = 17; Indices[nCount++] = 18;

            } // End if !cull

            // Body +Z Face
            cgPlane::fromPoints( Plane, Points[21].position, Points[22].position, Points[23].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 21; Indices[nCount++] = 22;
                Indices[nCount++] = 22; Indices[nCount++] = 23;
                Indices[nCount++] = 23; Indices[nCount++] = 20;
                Indices[nCount++] = 20; Indices[nCount++] = 21;

            } // End if !cull

            // Body -Z Face
            cgPlane::fromPoints( Plane, Points[17].position, Points[16].position, Points[19].position );
            if ( bOrtho )
                bCull = (cgVector3::dot( (cgVector3&)Plane, vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, (cgVector3&)Plane, Plane.d ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 17; Indices[nCount++] = 16;
                Indices[nCount++] = 16; Indices[nCount++] = 19;
                Indices[nCount++] = 19; Indices[nCount++] = 18;
                Indices[nCount++] = 18; Indices[nCount++] = 17;

            } // End if !cull


            // Draw the frustum if the camera is selected.
            if ( pIssuer->isSelected() )
            {
                fSize = tanf(CGEToRadian(mFOV*0.5f)) * mNearClip;
                cgSizeF NearSize( fSize*2, fSize*2 );
                Points[24].position = cgVector3( NearSize.width * -0.5f, NearSize.height * -0.5f, mNearClip );
                Points[25].position = cgVector3( NearSize.width * -0.5f, NearSize.height *  0.5f, mNearClip );
                Points[26].position = cgVector3( NearSize.width *  0.5f, NearSize.height *  0.5f, mNearClip );
                Points[27].position = cgVector3( NearSize.width *  0.5f, NearSize.height * -0.5f, mNearClip );
                
                Indices[nCount++] = 24; Indices[nCount++] = 25;
                Indices[nCount++] = 25; Indices[nCount++] = 26;
                Indices[nCount++] = 26; Indices[nCount++] = 27;
                Indices[nCount++] = 27; Indices[nCount++] = 24;

                fSize = tanf(CGEToRadian(mFOV * 0.5f)) * mFarClip;
                cgSizeF FarSize( fSize*2, fSize*2 );
                Points[28].position = cgVector3( FarSize.width * -0.5f, FarSize.height * -0.5f, mFarClip );
                Points[29].position = cgVector3( FarSize.width * -0.5f, FarSize.height *  0.5f, mFarClip );
                Points[30].position = cgVector3( FarSize.width *  0.5f, FarSize.height *  0.5f, mFarClip );
                Points[31].position = cgVector3( FarSize.width *  0.5f, FarSize.height * -0.5f, mFarClip );
                
                Indices[nCount++] = 28; Indices[nCount++] = 29;
                Indices[nCount++] = 29; Indices[nCount++] = 30;
                Indices[nCount++] = 30; Indices[nCount++] = 31;
                Indices[nCount++] = 31; Indices[nCount++] = 28;

                // Draw lines between near and far plates
                Indices[nCount++] = 24; Indices[nCount++] = 28;
                Indices[nCount++] = 25; Indices[nCount++] = 29;
                Indices[nCount++] = 26; Indices[nCount++] = 30;
                Indices[nCount++] = 27; Indices[nCount++] = 31;

            } // End if selected

            // Draw all geometry
            if ( nCount )
                pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineList, 0, 32, nCount / 2, Indices, cgBufferFormat::Index32, Points );
        
        } // End if begun pass

        // We have finished rendering
        pShader->endTechnique();
    
    } // End if begun technique

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
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

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProjection.bindParameter( 1, (cgUInt32)mProjectionMode );
        mUpdateProjection.bindParameter( 2, mFOV );
        mUpdateProjection.bindParameter( 3, mNearClip );
        mUpdateProjection.bindParameter( 4, mFarClip );
        mUpdateProjection.bindParameter( 5, mZoomFactor );
        mUpdateProjection.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( mUpdateProjection.step( true ) == false )
        {
            cgString strError;
            mUpdateProjection.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update projection properties for camera object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
//  Name : enableDepthOfField()
/// <summary>
/// Enable or disable depth of field processing for this camera.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::enableDepthOfField( bool enable )
{
    mDepthOfFieldEnabled = enable;
}

//-----------------------------------------------------------------------------
//  Name : setForegroundExtents ()
/// <summary>
/// Set the distance range for the part of the scene that is to be considered
/// part of the foreground.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setForegroundExtents( cgFloat minimum, cgFloat maximum )
{
    setForegroundExtents( cgRangeF(minimum,maximum) );
}

//-----------------------------------------------------------------------------
//  Name : setForegroundExtents ()
/// <summary>
/// Set the distance range for the part of the scene that is to be considered
/// part of the foreground.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setForegroundExtents( const cgRangeF & range )
{
    mForegroundExtents = range;
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundExtents ()
/// <summary>
/// Set the distance range for the part of the scene that is to be considered
/// part of the background.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setBackgroundExtents( cgFloat minimum, cgFloat maximum )
{
    setBackgroundExtents( cgRangeF(minimum,maximum) );
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundExtents ()
/// <summary>
/// Set the distance range for the part of the scene that is to be considered
/// part of the background.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setBackgroundExtents( const cgRangeF & range )
{
    mBackgroundExtents = range;
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundBlur ()
/// <summary>
/// Configure the level of blur that is applied to the two background levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setBackgroundBlur( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                        cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow )
{
    setBackgroundBlur( cgBlurOpDesc( passCountHigh, pixelRadiusHigh, distanceFactorHigh ),
                       cgBlurOpDesc( passCountLow, pixelRadiusLow, distanceFactorLow ) );
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundBlur ()
/// <summary>
/// Configure the level of blur that is applied to the two background levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setBackgroundBlur( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur )
{
    mBackgroundHighBlur = highBlur;
    mBackgroundLowBlur  = lowBlur;
}

//-----------------------------------------------------------------------------
//  Name : setForegroundBlur()
/// <summary>
/// Configure the level of blur that is applied to the two foreground levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setForegroundBlur( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                                 cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow )
{
    setForegroundBlur( cgBlurOpDesc( passCountHigh, pixelRadiusHigh, distanceFactorHigh ),
                       cgBlurOpDesc( passCountLow, pixelRadiusLow, distanceFactorLow ) );
}

//-----------------------------------------------------------------------------
//  Name : setForegroundBlur()
/// <summary>
/// Configure the level of blur that is applied to the two foreground levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::setForegroundBlur( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur )
{
    mForegroundHighBlur = highBlur;
    mForegroundLowBlur  = lowBlur;
}


//-----------------------------------------------------------------------------
//  Name : isDepthOfFieldEnabled()
/// <summary>
/// Determine if depth of field processing is enabled for this camera.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCameraObject::isDepthOfFieldEnabled( ) const
{
    return mDepthOfFieldEnabled;
}

//-----------------------------------------------------------------------------
//  Name : getForegroundExtents ()
/// <summary>
/// Get the distance range for the part of the scene that is to be considered
/// part of the foreground.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgCameraObject::getForegroundExtents( ) const
{
    return mForegroundExtents;
}

//-----------------------------------------------------------------------------
//  Name : getBackgroundExtents ()
/// <summary>
/// Get the distance range for the part of the scene that is to be considered
/// part of the background.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgCameraObject::getBackgroundExtents( ) const
{
    return mBackgroundExtents;
}

//-----------------------------------------------------------------------------
//  Name : getBackgroundBlur ()
/// <summary>
/// Retrieve the level of blur that is applied to the two background levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::getBackgroundBlur( cgBlurOpDesc & highBlur, cgBlurOpDesc & lowBlur ) const
{
    highBlur = mBackgroundHighBlur;
    lowBlur = mBackgroundLowBlur;
}

//-----------------------------------------------------------------------------
//  Name : getForegroundBlur()
/// <summary>
/// Configure the level of blur that is applied to the two foreground levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgCameraObject::getForegroundBlur( cgBlurOpDesc & highBlur, cgBlurOpDesc & lowBlur ) const
{
    highBlur = mForegroundHighBlur;
    lowBlur = mForegroundLowBlur;
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
    mVisibility = new cgVisibilitySet( pScene );
    mVisibility->setSearchFlags( cgVisibilitySearchFlags::MustRender | cgVisibilitySearchFlags::CollectMaterials );

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
    mVisibility = new cgVisibilitySet( pScene );
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
    // Make sure that any pending world transforms are up to date.
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );

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
cgVolumeQuery::Class cgCameraNode::boundsInFrustum( const cgBoundingBox & AABB )
{
    // Recompute the frustum as necessary.
    const cgFrustum & Frustum = getFrustum();

    // Request that frustum classifies
    return Frustum.classifyAABB( AABB );
}

//-----------------------------------------------------------------------------
//  Name : boundsInFrustum ()
/// <summary>
/// Determine whether or not the OOBB specified is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgCameraNode::boundsInFrustum( const cgBoundingBox & AABB, const cgTransform & Transform )
{
    // Recompute the frustum as necessary.
    const cgFrustum & Frustum = getFrustum();

    // Request that frustum classifies
    return Frustum.classifyAABB( AABB, Transform );
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
// Name : estimatePickTolerance ()
/// <summary>
/// Estimate the distance (along each axis) from the specified object space 
/// point to use as a tolerance for picking.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgCameraNode::estimatePickTolerance( const cgSize & ViewportSize, cgFloat WireTolerance, const cgVector3 & Pos, const cgTransform & ObjectTransform )
{
    // Scale tolerance based on estimated world space zoom factor.
    cgVector3 v;
    ObjectTransform.transformCoord( v, Pos );
    WireTolerance *= estimateZoomFactor( ViewportSize, v );

    // Convert into object space tolerance.
    cgVector3 ObjectWireTolerance;
    cgVector3 vAxisScale = ObjectTransform.localScale();
    ObjectWireTolerance.x = WireTolerance / vAxisScale.x;
    ObjectWireTolerance.y = WireTolerance / vAxisScale.y;
    ObjectWireTolerance.z = WireTolerance / vAxisScale.z;
    return ObjectWireTolerance;
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
void cgCameraNode::computeVisibility( )
{
    mVisibility->compute( getFrustum() );
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
void cgCameraNode::onComponentModified( cgReference * sender, cgComponentModifiedEventArgs * e )
{
    // All modifications require projection matrix and
    // frustum to be updated.
    mProjectionDirty    = true;
    mFrustumDirty = true;

    // Call base class implementation last
    cgObjectNode::onComponentModified( sender, e );
}