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
// Name : cgNavigationMesh.h                                                 //
//                                                                           //
// Desc : The navigation mesh is used to construct and manage navigation     //
//        data for scene geometry that can be used by AI agents or other     //
//        systems in order to navigate and find paths through a scene.       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNAVIGATIONMESH_H_ )
#define _CGE_CGNAVIGATIONMESH_H_

//-----------------------------------------------------------------------------
// cgNavigationMesh Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Navigation/cgNavigationTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgNavigationTile;
class dtNavMesh;
class dtCrowd;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgNavigationMesh (Class)
/// <summary>
/// The navigation mesh is used to construct and manage navigation data for 
/// scene geometry that can be used by AI agents or other systems in order to 
/// navigate and find paths through a scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationMesh : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgNavigationMesh, "NavigationMesh" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationMesh( );
    virtual ~cgNavigationMesh( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                build           ( const cgNavigationMeshCreateParams & params, cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[] );
    void                debugDraw       ( cgRenderDriver * driver );
    const cgNavigationMeshCreateParams& getParameters( ) const;

    // Internal utilities
    dtNavMesh         * getInternalMesh ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose         ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_ARRAY_DECLARE( cgNavigationTile*, TileArray )

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    dtNavMesh                     * mMesh;      // Internal detour navigation mesh.
    cgNavigationMeshCreateParams    mParams;    // Navigation mesh creation parameters.
    TileArray                       mTiles;     // Generated navigation tiles.
};

#endif // !_CGE_CGNAVIGATIONMESH_H_