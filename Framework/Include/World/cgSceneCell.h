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
// Name : cgSceneCell.h                                                      //
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

#pragma once
#if !defined( _CGE_CGSCENECELL_H_ )
#define _CGE_CGSCENECELL_H_

//-----------------------------------------------------------------------------
// cgSceneTree Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldQuery.h>
#include <Math/cgMathTypes.h>
#include <Math/cgBoundingBox.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;
class cgObjectNode;
class cgWorld;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSceneCell (Class)
/// <summary>
/// An individual cell designed to break down the scene into broad spatial 
/// sub-areas that serve as a container for local nodes and properties.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSceneCell : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgSceneCell, "SceneCell" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSceneCell( cgScene * parentScene );
             cgSceneCell( cgScene * parentScene, cgInt16 cellX, cgInt16 cellY, cgInt16 cellZ );
    virtual ~cgSceneCell( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgScene       * getParentScene      ( );
    cgVector3       getWorldOrigin      ( ) const;
    cgBoundingBox   getBoundingBox      ( ) const;
    cgUInt32        getCellId           ( ) const;
    void            getGridOffsets      ( cgInt16 & x, cgInt16 & y, cgInt16 & z ) const;
    bool            isEmpty             ( ) const;
    void            addNode             ( cgObjectNode * node );
    void            removeNode          ( cgObjectNode * node );
    bool            load                ( cgWorldQuery * cellQuery );
    bool            insert              ( cgWorld * world, cgUInt32 sceneId );
    bool            remove              ( cgWorld * world, cgUInt32 sceneId );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            prepareQueries      ( cgWorld * world );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32            mCellId;        // The identifier of the cell as it exists in the database.
    cgScene           * mParentScene;   // The parent scene that owns this cell.
    cgInt16             mCellOffsetX;   // Grid based X offset for the cell as it exists in the scene.
    cgInt16             mCellOffsetY;   // Grid based Y offset for the cell as it exists in the scene.
    cgInt16             mCellOffsetZ;   // Grid based Z offset for the cell as it exists in the scene.
    cgObjectNodeSet     mNodes;         // List of all active nodes that exist in this cell.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertCell;
    static cgWorldQuery mDeleteCell;
};

#endif // !_CGE_CGSCENECELL_H_