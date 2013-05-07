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
// Name : cgNavigationTile.h                                                 //
//                                                                           //
// Desc : Maintains navigation data for a pre-defined region of the larger   //
//        navigation mesh. Allows navigation data for individual sections of //
//        the scene to be paged in and out of memory as necessary.           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNAVIGATIONTILE_H_ )
#define _CGE_CGNAVIGATIONTILE_H_

//-----------------------------------------------------------------------------
// cgNavigationTile Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class  cgNavigationMesh;
struct cgNavigationMeshCreateParams;
struct rcPolyMesh;
struct rcPolyMeshDetail;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgNavigationTile (Class)
/// <summary>
/// Maintains navigation data for a pre-defined region of the larger navigation
/// mesh. Allows navigation data for individual sections of the scene to be
/// paged in and out of memory as necessary.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationTile : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgNavigationTile, "NavigationTile" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationTile( cgInt32 tileX, cgInt32 tileY, cgInt32 tileZ, cgNavigationMesh * navMesh );
    virtual ~cgNavigationTile( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgNavigationMesh  * getNavigationMesh   ( ) const;
    const cgByteArray & getNavigationData   ( ) const;
    bool                buildTile           ( const cgNavigationMeshCreateParams & params, const cgBoundingBox & tileBounds, cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[] );
    void                debugDraw           ( cgRenderDriver * driver );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    void                buildDebugMeshes    ( cgResourceManager * resources );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgNavigationMesh  * mNavMesh;                   // Parent navigation mesh to which this tile belongs.
    cgInt32             mTileX, mTileY, mTileZ;     // Location of the navigation tile in the tile grid.
    cgUInt              mTileRef;                   // Navigation mesh reference ID for the tile.
    cgByteArray         mNavData;                   // The actual compiled tile data that is added to the navigation mesh.
    rcPolyMesh        * mPolyMesh;                  // Actual polygon navigation mesh.
    rcPolyMeshDetail  * mDetailMesh;                // Detail mesh.
    cgMeshHandle        mDebugMesh;                 // Renderable version of the mesh used for debug rendering.
};

#endif // !_CGE_CGNAVIGATIONTILE_H_