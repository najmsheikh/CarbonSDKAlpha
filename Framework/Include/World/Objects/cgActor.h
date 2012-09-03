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
// Name : cgActor.h                                                          //
//                                                                           //
// Desc : An actor is a specialized object group that can be used to collect //
//        together objects (as children) that can be managed and animated    //
//        as one entity. The actor can then be used to manage animation data //
//        and behaviors for the collection as a whole.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGACTOR_H_ )
#define _CGE_CGACTOR_H_

//-----------------------------------------------------------------------------
// cgActor Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgGroupObject.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgAnimationController;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {F94939EE-4328-408C-AF02-9604B38F32E4}
const cgUID RTID_ActorNode   = {0xF94939EE, 0x4328, 0x408C, {0xAF, 0x2, 0x96, 0x4, 0xB3, 0x8F, 0x32, 0xE4}};
// {9039210A-CBD9-4D7E-AAB8-7B959BE0EA1C}
const cgUID RTID_ActorObject = {0x9039210A, 0xCBD9, 0x4D7E, {0xAA, 0xB8, 0x7B, 0x95, 0x9B, 0xE0, 0xEA, 0x1C}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgActorObject (Class)
/// <summary>
/// Maintains a collection of scene nodes as a single parent level group.
/// Almost all of the functionality is supplied by the custom cgGroupNode
/// type, with this object type largely managing database access.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgActorObject : public cgGroupObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgActorObject, cgGroupObject, "ActorObject" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgActorNode;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgActorObject( cgUInt32 referenceId, cgWorld * world );
             cgActorObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgActorObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        addAnimationSet         ( const cgAnimationSetHandle & animationSet );
    const cgAnimationSetHandle& getAnimationSetByName   ( const cgString & name ) const;
    cgUInt32                    getAnimationSetCount    ( ) const;
    const cgAnimationSetHandle& getAnimationSet         ( cgUInt32 index ) const;
    
    //---------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgGroupObject)
    //---------------------------------------------------------------------
    virtual void                setOpen                 ( bool open );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual void                onComponentDeleted      ( );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ActorObject; }
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
    cgAnimationSetHandleArray   mAnimationSets;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertActor;
    static cgWorldQuery     mUpdateOpen;
    static cgWorldQuery     mInsertSetReference;
    static cgWorldQuery     mLoadActor;
    static cgWorldQuery     mLoadSetReferences;
};

//-----------------------------------------------------------------------------
//  Name : cgActorNode (Class)
/// <summary>
/// Custom node type for the actor object. Manages additional processes
/// necessary when changes to the "group" object take place and to correctly
/// intepret child node arrangements.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgActorNode : public cgGroupNode, public cgObjectNodeEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgActorNode, cgGroupNode, "ActorNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgActorNode( cgUInt32 referenceId, cgScene * scene );
             cgActorNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgActorNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew                 ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone               ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgAnimationController     * getAnimationController      ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgGroupNode)
    //-------------------------------------------------------------------------
    virtual cgUInt32            onGroupRefAdded             ( cgObjectNode * node );
    virtual cgUInt32            onGroupRefRemoved           ( cgObjectNode * node );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                onNodeCreated               ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeLoading               ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual void                update                      ( cgFloat timeDelta );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNodeEventListener)
    //-------------------------------------------------------------------------
    virtual void                onInstanceIdentifierChange  ( cgObjectNodeNameChangeEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                onComponentModified         ( cgComponentModifiedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );
    
    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline bool addAnimationSet( const cgAnimationSetHandle & animationSet )
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgActorObject*)mReferencedObject)->addAnimationSet( animationSet );
    }
    
    // Object Property 'Get' Routing
    inline const cgAnimationSetHandle & getAnimationSetByName( const cgString & name ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgActorObject*)mReferencedObject)->getAnimationSetByName( name );
    }
    inline cgUInt32 getAnimationSetCount( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgActorObject*)mReferencedObject)->getAnimationSetCount( );
    }
    inline const cgAnimationSetHandle & getAnimationSet( cgUInt32 index ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgActorObject*)mReferencedObject)->getAnimationSet( index );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_ActorNode; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE( cgString, cgAnimationTarget*, TargetMap )

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    TargetMap               mTargets;   // Preconstructed list of all animation targets attached to this actor.
    cgAnimationController * mController;
};

#endif // !_CGE_CGACTOR_H_