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
// Name : cgJointObject.cpp                                                  //
//                                                                           //
// Desc : Our physics joint class implemented as a scene object that can be  //
//        managed and controlled in the same way as any other object.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgJointObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgJointObject.h>
#include <World/Objects/cgCameraObject.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgJointObject::mInsertBaseJoint;
cgWorldQuery cgJointObject::mUpdateBaseJointProperties;
cgWorldQuery cgJointObject::mLoadBaseJoint;

///////////////////////////////////////////////////////////////////////////////
// cgJointObject Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgJointObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointObject::cgJointObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
    mBodyCollision = false;
}

//-----------------------------------------------------------------------------
//  Name : cgJointObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointObject::cgJointObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgJointObject * pObject = (cgJointObject*)pInit;
    mBodyCollision = pObject->mBodyCollision;
}

//-----------------------------------------------------------------------------
//  Name : ~cgJointObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointObject::~cgJointObject()
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
void cgJointObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources

    // Dispose base.
    if ( bDisposeBase == true )
        cgWorldObject::dispose( true );
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
bool cgJointObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_JointObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData( ) )
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
bool cgJointObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("JointObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertBaseJoint.bindParameter( 1, mReferenceId );
        mInsertBaseJoint.bindParameter( 2, mBodyCollision );

        // Execute
        if ( !mInsertBaseJoint.step( true ) )
        {
            cgString strError;
            mInsertBaseJoint.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert joint object '0x%x' base data into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("JointObject::insertComponentData") );
            return false;

        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("JointObject::insertComponentData") );

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
bool cgJointObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the base light data.
    prepareQueries();
    mLoadBaseJoint.bindParameter( 1, e->sourceRefId );
    if ( !mLoadBaseJoint.step( ) || !mLoadBaseJoint.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadBaseJoint.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for joint object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadBaseJoint.reset();
        return false;

    } // End if failed

    // Update our local members
    mLoadBaseJoint.getColumn( _T("BodyCollision"), mBodyCollision );
    mLoadBaseJoint.reset();

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
//  Name : createTypeTables() (Virtual)
/// <summary>
/// When this type of component is being inserted into the database for the
/// first time, this method will be called in order to allow it to create any 
/// necessary tables.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointObject::createTypeTables( const cgUID & TypeIdentifier )
{
    // Ensure this base class table is created first.
    if ( !cgWorldObject::createTypeTables( RTID_JointObject ) )
        return false;

    // Then allow derived type to create as normal.
    return cgWorldObject::createTypeTables( TypeIdentifier );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgJointObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertBaseJoint.isPrepared() )
            mInsertBaseJoint.prepare( mWorld, _T("INSERT INTO 'Objects::Base::Joint' VALUES(?1,?2)"), true );
        if ( !mUpdateBaseJointProperties.isPrepared() )
            mUpdateBaseJointProperties.prepare( mWorld, _T("UPDATE 'Objects::Base::Joint' SET BodyCollision=?1 WHERE RefId=?2"), true );

    } // End if sandbox

    // Read queries
    if ( !mLoadBaseJoint.isPrepared() )
        mLoadBaseJoint.prepare( mWorld, _T("SELECT * FROM 'Objects::Base::Joint' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointObject::getSubElementCategories( cgObjectSubElementCategory::Map & Categories ) const
{
    // Joints do not have support for sub-elements. They cannot
    // have collision shapes associated with them because they
    // cannot be rigid bodies.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgJointObject::applyObjectRescale( cgFloat fScale )
{
    // Nothing to re-scale in the base class.

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
//  Name : isBodyCollisionEnabled ()
/// <summary>
/// Get the state that describes whether collision detection should be 
/// performed between the two child bodies attached to this joint. In most 
/// cases this should be disabled for performance reasons.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointObject::isBodyCollisionEnabled( ) const
{
    return mBodyCollision;
}

//-----------------------------------------------------------------------------
//  Name : enableBodyCollision ()
/// <summary>
/// Enable or disable the state that describes whether collision detection
/// should be performed between the two child bodies attached to this joint.
/// In most cases this should be disabled for performance reasons.
/// </summary>
//-----------------------------------------------------------------------------
void cgJointObject::enableBodyCollision( bool enable )
{
    // Is this a no-op?
    if ( mBodyCollision == enable )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateBaseJointProperties.bindParameter( 1, enable );
        mUpdateBaseJointProperties.bindParameter( 6, mReferenceId );

        // Execute
        if ( !mUpdateBaseJointProperties.step( true ) )
        {
            cgString strError;
            mUpdateBaseJointProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update base properties for joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update internal value.
    mBodyCollision = enable;

    // Notify any listeners of this change.
    static const cgString strContext = _T("BodyCollision");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

///////////////////////////////////////////////////////////////////////////////
// cgJointNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointNode::cgJointNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Joint%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointNode::cgJointNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables to sensible defaults
    cgJointNode * pNode = (cgJointNode*)pInit;
}

//-----------------------------------------------------------------------------
//  Name : ~cgJointNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointNode::~cgJointNode()
{
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_JointNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgJointNode::canScale( ) const
{
    // Joints cannot be scaled by default.
    return false;
}

//-----------------------------------------------------------------------------
// Name : validateAttachment ( ) (Virtual)
/// <summary>
/// Allows nodes to describe whether or not a particular attachment of
/// another node as either a child or parent of this node is valid.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointNode::validateAttachment( cgObjectNode * pNode, bool bNodeAsChild )
{
    // Joints cannot be attached to other joints in any fashion.
    if ( pNode->queryObjectType( RTID_JointObject ) )
        return false;

    // Valid
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSandboxIconInfo ( ) (Virtual)
/// <summary>
/// Retrieve information about the iconic representation of this object as it
/// is to be displayed in the sandbox rendering viewports.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointNode::getSandboxIconInfo( cgCameraNode * pCamera, const cgSize & ViewportSize, cgString & strAtlas, cgString & strFrame, cgVector3 & vIconOrigin )
{
    strAtlas  = _T("sys://Textures/ObjectBillboardSheet.xml");
    strFrame  = _T("Joint");
    
    // Position icon
    vIconOrigin = getPosition(false);
    cgFloat fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, vIconOrigin, 2.5f );
    vIconOrigin += (pCamera->getXAxis() * 0.0f * fZoomFactor) + (pCamera->getYAxis() * 17.0f * fZoomFactor);
    return true;
}
