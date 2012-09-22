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
// Name : cgTargetObject.h                                                   //
//                                                                           //
// Desc : Contains simple classes responsible for providing a 'target' point //
//        with which objects can be reorientated. This is essentially a      //
//        'look at' point that will, for instance, manipulate a parent spot  //
//        light or camera object's matrices to face it.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGTARGETOBJECT_H_ )
#define _CGE_CGTARGETOBJECT_H_

//-----------------------------------------------------------------------------
// cgTargetObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {A59569A7-F869-444C-8C6D-13C8101872B3}
const cgUID RTID_TargetNode   = { 0xA59569A7, 0xF869, 0x444C, { 0x8C, 0x6D, 0x13, 0xC8, 0x10, 0x18, 0x72, 0xB3 } };
// {2FC4959B-366C-45F1-94A4-4C98F9CF1369}
const cgUID RTID_TargetObject = { 0x2FC4959B, 0x366C, 0x45F1, { 0x94, 0xA4, 0x4C, 0x98, 0xF9, 0xCF, 0x13, 0x69 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgTargetObject (Class)
/// <summary>
/// A simple target object. Used to provide positional information for any node
/// type in order to orient that node toward the target.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTargetObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgTargetObject, cgWorldObject, "TargetObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgTargetObject( cgUInt32 referenceId, cgWorld * world );
             cgTargetObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgTargetObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setMode                 ( cgNodeTargetMethod::Base mode );
    cgNodeTargetMethod::Base    getMode                 ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distanceOut );
    virtual void                applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_TargetObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgNodeTargetMethod::Base    mTargetingMode;            //< Method used to reorientate the targeting object (if any).

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertTarget;
    static cgWorldQuery     mUpdateAlignment;
    static cgWorldQuery     mLoadTarget;
};

//-----------------------------------------------------------------------------
//  Name : cgTargetNode (Class)
/// <summary>
/// Custom node type for the target object. Manages additional properties
/// that may need to be override by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTargetNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgTargetNode, cgObjectNode, "TargetNode" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgObjectNode;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgTargetNode( cgUInt32 referenceId, cgScene * scene );
             cgTargetNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgTargetNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        setTargetingNode        ( cgObjectNode * node );
    cgObjectNode              * getTargetingNode        ( ) const;
    void                        adjustTargetingTransform( cgTransform & targetingTransform );
    bool                        isUpdating              ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                canScale                ( ) const;
    virtual bool                canAdjustPivot          ( ) const;
    virtual bool                canSetName              ( ) const;
    virtual bool                canDelete               ( ) const;
    virtual bool                validateAttachment      ( cgObjectNode * node, bool nodeAsChild );
    virtual void                setTargetMethod         ( cgNodeTargetMethod::Base mode );
    virtual bool                setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_TargetNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setMode( cgNodeTargetMethod::Base mode )
    {
        ((cgTargetObject*)mReferencedObject)->setMode( mode );
    }
    
    // Object Property 'Get' Routing
    inline cgNodeTargetMethod::Base getMode( ) const
    {
        return ((cgTargetObject*)mReferencedObject)->getMode();
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        adjustTargetTransform   ( cgTransform & targetTransform, const cgTransform & targetingTransform );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgObjectNode  * mTargetingNode;             // The node that is targeting us.
    bool            mTargetUpdated;             // Has the target transform been updated in some way? (this is a key piece of the update logic that prevents endlessly recursive loops).
    bool            mUpdating;                  // Was this target node responsible for triggering the update of the targeting node's transform?
    cgTransform     mNewTargetTransform;        // Record of the new target transform on which to base the update.
};

#endif // !_CGE_CGTARGETOBJECT_H_