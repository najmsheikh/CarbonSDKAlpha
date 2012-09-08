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
// Name : cgWorldObject.h                                                    //
//                                                                           //
// Desc : Base class for all objects that make up a scene. Objects are       //
//        basically the foundation of all scene components such as meshes,   //
//        cameras, light sources, etc. Objects don't specifically maintain   //
//        scene information such as transformation matrices since they       //
//        technically belong to the world and can exist in multiple scenes.  //
//        Objects can however be referenced by object node types to provide  //
//        their final connection to the scene(s) as required (i.e.           //
//        cgObjectNode and its derived types to give them position,          //
//        orientation, etc.)                                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWORLDOBJECT_H_ )
#define _CGE_CGWORLDOBJECT_H_

//-----------------------------------------------------------------------------
// cgWorldObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldComponent.h>
#include <World/cgWorldTypes.h>
#include <Resources/cgResourceHandles.h>
#include <Math/cgBoundingBox.h>
#include <Physics/cgPhysicsTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgWorld;
class cgScene;
class cgCameraNode;
class cgObjectNode;
class cgVisibilitySet;
class cgObjectSubElement;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {FCC02596-AD72-4EDD-A7ED-32330F759CD1}
const cgUID RTID_WorldObject = {0xFCC02596, 0xAD72, 0x4EDD, {0xA7, 0xED, 0x32, 0x33, 0xF, 0x75, 0x9C, 0xD1}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgWorldObject (Class)
/// <summary>
/// Base class for the objects that exist in the world.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWorldObject : public cgWorldComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgWorldObject, cgWorldComponent, "WorldObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWorldObject( cgUInt32 referenceId, cgWorld * world );
             cgWorldObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgWorldObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static const cgWorldObjectTypeDesc::Map   & getRegisteredTypes  ( );
    static void                                 registerType        ( const cgUID & globalIdentifier, const cgString & name, 
                                                                      cgWorldObjectTypeDesc::ObjectAllocNewFunc objectAllocNew, 
                                                                      cgWorldObjectTypeDesc::ObjectAllocCloneFunc objectAllocClone,
                                                                      cgWorldObjectTypeDesc::NodeAllocNewFunc nodeAllocNew, 
                                                                      cgWorldObjectTypeDesc::NodeAllocCloneFunc nodeAllocClone );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                            clone                   ( cgCloneMethod::Base method, cgWorld * world, bool internalObject, cgWorldObject *& objectOut );
    void                            setBaseMass             ( cgFloat mass );
    void                            setMassTransformAmount  ( cgFloat amount );
    cgFloat                         getBaseMass             ( ) const;
    cgFloat                         getMassTransformAmount  ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    // Properties
    virtual bool                    isRenderable            ( ) const;
    virtual bool                    isShadowCaster          ( ) const;
    virtual cgBoundingBox           getLocalBoundingBox     ( );
    
    // Sub-elements
    virtual bool                    getSubElementCategories ( cgObjectSubElementCategory::Map & categories ) const;
    virtual cgObjectSubElement    * createSubElement        ( const cgUID & category, const cgUID & identifier );
    virtual cgObjectSubElement    * cloneSubElement         ( cgObjectSubElement * element );
    virtual const cgObjectSubElementArray & getSubElements  ( const cgUID & category ) const;
    virtual void                    deleteSubElement        ( cgObjectSubElement * element );
    virtual void                    deleteSubElements       ( const cgObjectSubElementArray & elements );
    
    // Rendering
    virtual void                    sandboxRender           ( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                    render                  ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer );
    virtual bool                    renderSubset            ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer, const cgMaterialHandle & material );

    // Data operations
    virtual void                    applyObjectRescale      ( cgFloat scale );

    // Sandbox Integration
    virtual bool                    pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distanceOut );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                    onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                    onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual void                    onComponentDeleted      ( );
    virtual bool                    createTypeTables        ( const cgUID & typeIdentifier );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_WorldObject; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE( cgUID, cgObjectSubElementArray, ElementCategoryMap )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                            prepareQueries          ( );
    bool                            insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    ElementCategoryMap      mSubElementCategories;      // Array containing all object sub-elements exposed to the system.
    cgFloat                 mMass;                      // Mass of this object as it is represented in the physics world.
    cgFloat                 mMassTransformAmount;       // Scalar that describes how much the 'scale' component of the node's transform can affect the mass.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertBaseObject;
    static cgWorldQuery     mInsertSubElement;
    static cgWorldQuery     mDeleteSubElement;
    static cgWorldQuery     mUpdateMassProperties;
    static cgWorldQuery     mLoadBaseObject;
    static cgWorldQuery     mLoadSubElements;

    // Registered object types
    static cgWorldObjectTypeDesc::Map   mRegisteredObjectTypes; // All of the object types registered with the system
};

#endif // !_CGE_CGWORLDOBJECT_H_