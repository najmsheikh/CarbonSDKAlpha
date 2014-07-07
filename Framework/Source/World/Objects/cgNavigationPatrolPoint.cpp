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
// Name : cgNavigationPatrolPoint.cpp                                        //
//                                                                           //
// Desc : Specialized navigation waypoint designed to provide points of      //
//        interest for different types of navigation agent during regular    //
//        patrol pathing.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSpotLight Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgNavigationPatrolPoint.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgNavigationPatrolPointObject::mInsertPatrolPoint;
cgWorldQuery cgNavigationPatrolPointObject::mLoadPatrolPoint;

///////////////////////////////////////////////////////////////////////////////
// cgNavigationPatrolPointObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationPatrolPointObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationPatrolPointObject::cgNavigationPatrolPointObject( cgUInt32 referenceId, cgWorld * world ) : cgNavigationWaypointObject( referenceId, world )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgNavigationPatrolPointObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationPatrolPointObject::cgNavigationPatrolPointObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgNavigationWaypointObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    cgNavigationPatrolPointObject * object = (cgNavigationPatrolPointObject*)init;
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationPatrolPointObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationPatrolPointObject::~cgNavigationPatrolPointObject()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationPatrolPointObject::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base.
    if ( disposeBase )
        cgNavigationWaypointObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgNavigationPatrolPointObject::allocateNew( const cgUID & type, cgUInt32 referenceId, cgWorld * world )
{
    return new cgNavigationPatrolPointObject( referenceId, world );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgNavigationPatrolPointObject::allocateClone( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod )
{
    return new cgNavigationPatrolPointObject( referenceId, world, init, initMethod );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgNavigationPatrolPointObject::getDatabaseTable( ) const
{
    return _T("Objects::NavigationPatrolPoint");
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationPatrolPointObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_NavigationPatrolPointObject )
        return true;

    // Supported by base?
    return cgNavigationWaypointObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationPatrolPointObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last.
    return cgNavigationWaypointObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationPatrolPointObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("NavigationPatrolPointObject::insertComponentData") );

        // Insert new spot light object
        prepareQueries();
        mInsertPatrolPoint.bindParameter( 1, mReferenceId );
        mInsertPatrolPoint.bindParameter( 2, mSoftRefCount );
        
        // Execute
        if ( !mInsertPatrolPoint.step( true ) )
        {
            cgString error;
            mInsertPatrolPoint.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for navigation patrol point object '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("NavigationPatrolPointObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("NavigationPatrolPointObject::insertComponentData") );

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
bool cgNavigationPatrolPointObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the object data.
    prepareQueries();
    mLoadPatrolPoint.bindParameter( 1, e->sourceRefId );
    if ( !mLoadPatrolPoint.step( ) || !mLoadPatrolPoint.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadPatrolPoint.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for navigation patrol point object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for navigation patrol point object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadPatrolPoint.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadPatrolPoint;

    // Update our local members
    //mLoadSpotLight.getColumn( _T("ShadowUpdateRate"), mShadowUpdateRate );

    // Call base class implementation to read remaining data.
    if ( !cgNavigationWaypointObject::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        // Insert
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
void cgNavigationPatrolPointObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertPatrolPoint.isPrepared( mWorld ) )
            mInsertPatrolPoint.prepare( mWorld, _T("INSERT INTO 'Objects::NavigationPatrolPoint' VALUES(?1,?2)"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadPatrolPoint.isPrepared( mWorld ) )
        mLoadPatrolPoint.prepare( mWorld, _T("SELECT * FROM 'Objects::NavigationPatrolPoint' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgNavigationPatrolPointNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationPatrolPointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationPatrolPointNode::cgNavigationPatrolPointNode( cgUInt32 referenceId, cgScene * scene ) : cgNavigationWaypointNode( referenceId, scene )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgNavigationPatrolPointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationPatrolPointNode::cgNavigationPatrolPointNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgNavigationWaypointNode( referenceId, scene, init, initMethod, initTransform )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationPatrolPointNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationPatrolPointNode::~cgNavigationPatrolPointNode()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationPatrolPointNode::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated memory
    
    // Clear variables
    
    // Dispose base.
    if ( disposeBase )
        cgNavigationWaypointNode::dispose( true );
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
bool cgNavigationPatrolPointNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_NavigationPatrolPointNode )
        return true;

    // Supported by base?
    return cgNavigationWaypointNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgNavigationPatrolPointNode::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgNavigationPatrolPointNode( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgNavigationPatrolPointNode::allocateClone( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform )
{
    return new cgNavigationPatrolPointNode( referenceId, scene, init, initMethod, initTransform );
}