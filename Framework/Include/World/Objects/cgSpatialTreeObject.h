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
// Name : cgSpatialTreeObject.h                                              //
//                                                                           //
// Desc : Base world object and scene node types designed for storage and    //
//        rendering of data organized using a spatial tree structure.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSPATIALTREEOBJECT_H_ )
#define _CGE_CGSPATIALTREEOBJECT_H_

//-----------------------------------------------------------------------------
// cgSpatialTreeObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgObjectNode.h>
#include <World/cgSpatialTree.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5EC62995-FB55-4427-BC0F-6C6F82C2D3F3}
const cgUID RTID_SpatialTreeObject = {0x5EC62995, 0xFB55, 0x4427, {0xBC, 0x0F, 0x6C, 0x6F, 0x82, 0xC2, 0xD3, 0xF3}};
// {F089680B-3092-47CC-B5A3-7270373DC60C}
const cgUID RTID_SpatialTreeNode   = {0xF089680B, 0x3092, 0x47CC, {0xB5, 0xA3, 0x72, 0x70, 0x37, 0x3D, 0xC6, 0x0C}};

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeObject (Base Class)
/// <summary>
/// Base spatial tree object type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSpatialTreeObject : public cgWorldObject, public cgSpatialTree
{
    // ToDo: 9999 - Will eventually derive2 from cgSpatialTree as well as cgWorldObject
    DECLARE_DERIVED_SCRIPTOBJECT( cgSpatialTreeObject, cgWorldObject, "SpatialTreeObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSpatialTreeObject( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
             cgSpatialTreeObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgSpatialTreeObject( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual void                    sandboxRender       ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                    render              ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer );
    virtual bool                    renderSubset        ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer, const cgMaterialHandle & hMaterial );
    virtual cgBoundingBox           getLocalBoundingBox ( );
    virtual void                    applyObjectRescale  ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                    onComponentCreated  ( cgComponentCreatedEventArgs * e );
    virtual bool                    onComponentLoading  ( cgComponentLoadingEventArgs * e );
    virtual bool                    createTypeTables    ( const cgUID & typeIdentifier );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType    ( ) const { return RTID_SpatialTreeObject; }
    virtual bool                    queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeNode (Class)
/// <summary>
/// Custom node type for the spatial tree object. Manages additional properties
/// that may need to be overriden by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSpatialTreeNode : public cgObjectNode, public cgSpatialTreeInstance
{
    // ToDo: 9999 - Will eventually derive from cgSpatialTreeInstance
    DECLARE_DERIVED_SCRIPTOBJECT( cgSpatialTreeNode, cgObjectNode, "SpatialTreeNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSpatialTreeNode( cgUInt32 referenceId, cgScene * scene );
             cgSpatialTreeNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgSpatialTreeNode( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    // Object Property 'Get' Routing

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                onNodeCreated           ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeLoading           ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual bool                registerVisibility      ( cgVisibilitySet * pSet, cgUInt32 nFlags );
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SpatialTreeNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

};

#endif // !_CGE_CGSPATIALTREEOBJECT_H_