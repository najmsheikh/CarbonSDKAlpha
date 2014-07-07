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
#include <World/cgWorldComponent.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgNavigationTile;
class cgLandscape;
class dtNavMesh;
class dtCrowd;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {94037098-22B7-4A7C-B369-62DEB113FBBD}
const cgUID RTID_NavigationMesh = { 0x94037098, 0x22b7, 0x4a7c, { 0xb3, 0x69, 0x62, 0xde, 0xb1, 0x13, 0xfb, 0xbd } };

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
class CGE_API cgNavigationMesh : public cgWorldComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNavigationMesh, cgWorldComponent, "NavigationMesh" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationMesh( );
             cgNavigationMesh( cgUInt32 referenceId, cgWorld * world );
    virtual ~cgNavigationMesh( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        build                   ( cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[], cgLandscape * landscape );
    bool                        build                   ( const cgNavigationMeshCreateParams & params, cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[], cgLandscape * landscape );
    void                        setParameters           ( const cgNavigationMeshCreateParams & params );
    void                        debugDraw               ( cgRenderDriver * driver );
    const cgNavigationMeshCreateParams& getParameters   ( ) const;
    cgUInt32                    getTileCount            ( ) const;

    // Internal utilities
    dtNavMesh                 * getInternalMesh         ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual cgString            getDatabaseTable        ( ) const;
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual void                onComponentDeleted      ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_NavigationMesh; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_ARRAY_DECLARE( cgNavigationTile*, TileArray )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    dtNavMesh                     * mMesh;      // Internal detour navigation mesh.
    cgNavigationMeshCreateParams    mParams;    // Navigation mesh creation parameters.
    TileArray                       mTiles;     // Generated navigation tiles.
    cgInt32                         mTilesX;
    cgInt32                         mTilesY;
    cgInt32                         mTilesZ;
    cgBoundingBox                   mGeomBounds;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertMesh;
    static cgWorldQuery mDeleteTiles;
    static cgWorldQuery mUpdateParameters;
    static cgWorldQuery mLoadMesh;
    static cgWorldQuery mLoadTiles;
};

#endif // !_CGE_CGNAVIGATIONMESH_H_