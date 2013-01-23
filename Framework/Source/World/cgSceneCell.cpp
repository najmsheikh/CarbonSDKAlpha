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
// Name : cgSceneCell.cpp                                                    //
//                                                                           //
// Desc : Scenes are separated into a number of so-called 'cells' in order   //
//        to provide localized object information and area properties, as    //
//        well as providing the ability to load / unload objects within a    //
//        given area. This file houses the classes necessary to provide      //
//        this support.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSceneTree Module Includes
//-----------------------------------------------------------------------------
#include <World/cgSceneCell.h>
#include <World/cgScene.h>
#include <World/cgObjectNode.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgSceneCell::mInsertCell;
cgWorldQuery cgSceneCell::mDeleteCell;

///////////////////////////////////////////////////////////////////////////////
// cgSceneCell Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSceneCell () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneCell::cgSceneCell( cgScene * parentScene )
{
    // Initialize variables to sensible defaults
    mCellId       = 0;
    mParentScene  = parentScene;
    mCellOffsetX  = 0;
    mCellOffsetY  = 0;
    mCellOffsetZ  = 0;
}

//-----------------------------------------------------------------------------
//  Name : cgSceneCell () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneCell::cgSceneCell( cgScene * parentScene, cgInt16 cellX, cgInt16 cellY, cgInt16 cellZ )
{
    // Initialize variables to sensible defaults
    mCellId       = 0;
    mParentScene  = parentScene;
    mCellOffsetX  = cellX;
    mCellOffsetY  = cellY;
    mCellOffsetZ  = cellZ;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSceneCell () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneCell::~cgSceneCell()
{
    // Clear variables
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgSceneCell::dispose( bool disposeBase )
{
    // Clear variables
    mParentScene = CG_NULL;
    mNodes.clear();
}

//-----------------------------------------------------------------------------
//  Name : getParentScene ()
/// <summary>
/// Retrieve the parent scene that owns this cell.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgSceneCell::getParentScene( )
{
    return mParentScene;
}

//-----------------------------------------------------------------------------
//  Name : getBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this scene cell as it exists in world space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgSceneCell::getBoundingBox( ) const
{
    const cgVector3 & cellSize = mParentScene->getCellSize();
    cgVector3 origin( (cgFloat)mCellOffsetX * cellSize.x,
                      (cgFloat)mCellOffsetY * cellSize.y,
                      (cgFloat)mCellOffsetZ * cellSize.z );
    return cgBoundingBox( (cellSize * -0.5f) + origin, (cellSize * 0.5f) + origin );
}

//-----------------------------------------------------------------------------
//  Name : getWorldOrigin ()
/// <summary>
/// Retrieve the origin of this scene cell (its center point) as it
/// exists in world space.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgSceneCell::getWorldOrigin( ) const
{
    const cgVector3 & cellSize = mParentScene->getCellSize();
    return cgVector3( (cgFloat)mCellOffsetX * cellSize.x,
                      (cgFloat)mCellOffsetY * cellSize.y,
                      (cgFloat)mCellOffsetZ * cellSize.z );
}

//-----------------------------------------------------------------------------
//  Name : getNodes ()
/// <summary>
/// Retrieve the set containing all allocated nodes associated with this
/// scene cell.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectNodeSet & cgSceneCell::getNodes( ) const
{
    return mNodes;
}

//-----------------------------------------------------------------------------
//  Name : getCellId ()
/// <summary>
/// Retrieve the unique identifier of this cell.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgSceneCell::getCellId( ) const
{
    return mCellId;
}

//-----------------------------------------------------------------------------
//  Name : getGridOffsets ()
/// <summary>
/// Retrieve the 3D grid coordinates for this cell.
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneCell::getGridOffsets( cgInt16 & x, cgInt16 & y, cgInt16 & z ) const
{
    x = mCellOffsetX;
    y = mCellOffsetY;
    z = mCellOffsetZ;
}

//-----------------------------------------------------------------------------
//  Name : isEmpty ()
/// <summary>
/// Determine if this scene cell is now obsolete because it no longer contains
/// any active nodes AND no custom properties have been defined.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSceneCell::isEmpty() const
{
    // ToDo: 9999 - Needs to return false if custom properties have been assigned too.
    return mNodes.empty();
}

//-----------------------------------------------------------------------------
//  Name : addNode ()
/// <summary>
/// Add the specified node to this cell.
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneCell::addNode( cgObjectNode * node )
{
    mNodes.insert( node );
}

//-----------------------------------------------------------------------------
//  Name : removeNode ()
/// <summary>
/// Remove the specified node to this cell.
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneCell::removeNode( cgObjectNode * node )
{
    mNodes.erase( node );
}

//-----------------------------------------------------------------------------
// Name : load()
/// <summary>
/// Load the cell data from the world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSceneCell::load( cgWorldQuery * cellQuery )
{
    // Load data.
    cellQuery->getColumn( _T("CellId"), mCellId );
    cellQuery->getColumn( _T("LocationX"), mCellOffsetX );
    cellQuery->getColumn( _T("LocationY"), mCellOffsetY );
    cellQuery->getColumn( _T("LocationZ"), mCellOffsetZ );

    // ToDo: 9999 - Editor name, friendly name, flags, etc.

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : insert()
/// <summary>
/// Insert cell data into the world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSceneCell::insert( cgWorld * world, cgUInt32 sceneId )
{
    // This only applies in sandbox mode, and for serialized scenes.
    // If either of these is not the case, this is a non-error no-op.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled || !sceneId )
        return true;

    // Run insert query
    cgInt32 flags = 0;
    prepareQueries( world );
    mInsertCell.bindParameter( 1, sceneId );
    mInsertCell.bindParameter( 2, (cgInt32)0 );       // Flags
    mInsertCell.bindParameter( 3, mCellOffsetX );
    mInsertCell.bindParameter( 4, mCellOffsetY );
    mInsertCell.bindParameter( 5, mCellOffsetZ );
    
    // Execute
    if ( mInsertCell.step( true ) == false )
    {
        cgString error;
        mInsertCell.getLastError( error );
        cgAppLog::write( cgAppLog::Error, _T("Failed to insert new cell data into database at location %i,%i,%i. Error: %s\n"), mCellOffsetX, mCellOffsetY, mCellOffsetZ, error.c_str() );
        return false;
    
    } // End if failed

    // Cell identifier can now be retrieved.
    mCellId = mInsertCell.getLastInsertId();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : remove()
/// <summary>
/// Delete cell data from the world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSceneCell::remove( cgWorld * world, cgUInt32 sceneId )
{
    // This only applies in sandbox mode, and for serialized scenes.
    // If either of these is not the case, this is a non-error no-op.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled || !sceneId || !mCellId )
        return true;

    // Run delete query
    prepareQueries( world );
    mDeleteCell.bindParameter( 1, mCellId );
    
    // Execute
    if ( mDeleteCell.step( true ) == false )
    {
        cgString error;
        mDeleteCell.getLastError( error );
        cgAppLog::write( cgAppLog::Error, _T("Failed to delete cell data from database at location %i,%i,%i. Error: %s\n"), mCellOffsetX, mCellOffsetY, mCellOffsetZ, error.c_str() );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneCell::prepareQueries( cgWorld * world )
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( mInsertCell.isPrepared() == false )
            mInsertCell.prepare( world, _T("INSERT INTO 'Cells' VALUES(NULL,?1,0,NULL,NULL,?2,?3,?4,?5)"), true );
        if ( mDeleteCell.isPrepared() == false )
            mDeleteCell.prepare( world, _T("DELETE FROM 'Cells' WHERE CellId=?1"), true );
        
    } // End if sandbox

    // Read queries
}