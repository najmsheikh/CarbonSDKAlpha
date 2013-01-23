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
// Name : cgRagdollController.h                                              //
//                                                                           //
// Desc : Contains classes which provide ragdoll physics behaviors for scene //
//        objects (usually bone hierarchies). This controller should be      //
//        attached to the root of the hierarchy to ragdoll.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRAGDOLLCONTROLLER_H_ )
#define _CGE_CGRAGDOLLCONTROLLER_H_

//-----------------------------------------------------------------------------
// cgRagdollController Header Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsController.h>
#include <Scripting/cgScriptInterop.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsWorld;
class cgRigidBody;
class cgBallJoint;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgRagdollController (Class)
/// <summary>
/// Contains classes which provide ragdoll physics behaviors for scene objects 
/// (usually bone hierarchies). This controller should be attached to the root 
/// of the hierarchy to ragdoll.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgRagdollController : public cgPhysicsController, public cgPhysicsBodyEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgRagdollController, cgPhysicsController, "RagdollController" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgRagdollController( cgPhysicsWorld * world );
    virtual ~cgRagdollController( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgPhysicsController * allocate( const cgString & typeName, cgPhysicsWorld * world );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void            restoreHierarchy            ( bool restoreTransforms );
    void            applyImpulseTo              ( cgObjectNode * boneNode, const cgVector3 & impulse, const cgVector3 & at );
    void            applyImpulseTo              ( cgObjectNode * boneNode, const cgVector3 & impulse );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsController)
    //-------------------------------------------------------------------------
    virtual bool    initialize                  ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsBodyEventListener)
    //-------------------------------------------------------------------------
    virtual void    onPhysicsBodyTransformed    ( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                     ( bool disposeBase );
	
protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    struct Bone
    {
        cgRigidBody   * body;
        cgBallJoint   * joint;  // Connection to parent.
        cgObjectNode  * node;   // Scene node being represented
        cgVector3       scale;  // Original scale of the bone.
        cgInt           parentBoneIndex;
        bool            collidable;
        cgTransform     originalTransform;
    };
    CGE_VECTOR_DECLARE( Bone, BoneArray )
    CGE_MAP_DECLARE( cgRigidBody*, size_t, BoneLUT );

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            constructHierarchy      ( cgObjectNode * currentNode, cgInt parentBoneIndex );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    BoneArray mBones;
    BoneLUT   mBoneLUT;
};

#endif // !_CGE_CGRAGDOLLCONTROLLER_H_