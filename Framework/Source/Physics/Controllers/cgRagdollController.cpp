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
// Name : cgRagdollController.cpp                                            //
//                                                                           //
// Desc : Contains classes which provide ragdoll physics behaviors for scene //
//        objects (usually bone hierarchies). This controller should be      //
//        attached to the root of the hierarchy to ragdoll.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgRagdollController Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Controllers/cgRagdollController.h>
#include <Physics/Shapes/cgBoxShape.h>
#include <Physics/Shapes/cgCylinderShape.h>
#include <Physics/Shapes/cgCapsuleShape.h>
#include <Physics/Shapes/cgSphereShape.h>
#include <Physics/Bodies/cgRigidBody.h>
#include <Physics/Joints/cgBallJoint.h>
#include <World/Objects/cgBoneObject.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgRagdollController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRagdollController () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRagdollController::cgRagdollController( cgPhysicsWorld * world ) : cgPhysicsController( world )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgRagdollController () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRagdollController::~cgRagdollController()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgRagdollController::dispose( bool disposeBase )
{
    // Release internal references
    for ( size_t i = 0; i < mBones.size(); ++i )
    {
        if ( mBones[i].body )
            mBones[i].body->removeReference( CG_NULL );
        if ( mBones[i].joint )
            mBones[i].joint->removeReference( CG_NULL );

    } // Next bone
    mBones.clear();
    mBoneLUT.clear();
    
    // Dispose base
    if ( disposeBase )
        cgPhysicsController::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : allocate() (Static)
/// <summary>
/// Allocate a physics controller of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsController * cgRagdollController::allocate( const cgString & typeName, cgPhysicsWorld * world )
{
    return new cgRagdollController( world );
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Virtual)
/// <summary>
/// Allow the controller to initialize (called after setting up).
/// </summary>
//-----------------------------------------------------------------------------
bool cgRagdollController::initialize(  )
{
    // Validate requirements
    if ( !mParentObject )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Attempting to initialize a 'Ragdoll' physics controller without first attaching it to a parent object is not allowed.\n") );
        return false;
    
    } // End if no parent

    // Release internal references
    for ( size_t i = 0; i < mBones.size(); ++i )
    {
        if ( mBones[i].body )
            mBones[i].body->removeReference( CG_NULL );
        if ( mBones[i].joint )
            mBones[i].joint->removeReference( CG_NULL );

    } // Next bone
    mBones.clear();
    mBoneLUT.clear();

    // Build the ragdoll hierarchy.
    constructHierarchy( mParentObject, -1 );

    // Attach all bones to the root bone's parent so that all bones are peers.
    for ( size_t i = 1; i < mBones.size(); ++i )
        mBones[i].node->setParent( mBones[0].node->getParent() );

    // Disable collision for the entire hierarchy.
    for ( size_t i = 0; i < mBones.size(); ++i )
        NewtonBodySetJointRecursiveCollision( mBones[i].body->getInternalBody(), mBones[i].collidable );
    
    // Call base class implementation.
    return cgPhysicsController::initialize( );
}

//-----------------------------------------------------------------------------
//  Name : restoreHierarchy()
/// <summary>
/// This method can be called in order to destroy all of the physics bodies
/// and joints that represent this ragdoll, and to restore the original
/// parent child relationships of the bones which are detached from one another
/// during initialization. Optionally, you can request that the transformations
/// of each bone are restored to the state in which they existed at the point
/// where the ragdoll controller was attached.
/// </summary>
//-----------------------------------------------------------------------------
void cgRagdollController::restoreHierarchy( bool restoreTransforms )
{
    // First destroy all the existing joints and physics bodies.
    for ( size_t i = 0; i < mBones.size(); ++i )
    {
        if ( mBones[i].body )
            mBones[i].body->removeReference( CG_NULL );
        if ( mBones[i].joint )
            mBones[i].joint->removeReference( CG_NULL );

    } // Next bone
    
    // Next restore the original transforms of all bones if requested.
    if ( restoreTransforms )
    {
        for ( size_t i = 0; i < mBones.size(); ++i )
            mBones[i].node->setWorldTransform( mBones[i].originalTransform );

    } // End if restore transforms

    // Now re-attach bones to their original parent.
    for ( size_t i = 1; i < mBones.size(); ++i )
        mBones[i].node->setParent( mBones[mBones[i].parentBoneIndex].node );

    // We're done. Clean up.
    mBones.clear();
    mBoneLUT.clear();
}

//-----------------------------------------------------------------------------
//  Name : applyImpulseTo ()
/// <summary>
/// Apply a physics impulse to the specified bone in the ragdoll at the
/// desired world space location.
/// </summary>
//-----------------------------------------------------------------------------
void cgRagdollController::applyImpulseTo( cgObjectNode * boneNode, const cgVector3 & impulse, const cgVector3 & at )
{
    // Find the associated bone.
    for ( size_t i = 0; i < mBones.size(); ++i )
    {
        if ( mBones[i].node == boneNode )
        {
            mBones[i].body->applyImpulse( impulse, at );
            return;
        
        } // End if match

    } // Next bone
}

//-----------------------------------------------------------------------------
//  Name : applyImpulseTo ()
/// <summary>
/// Apply a physics impulse to the specified bone in the ragdoll at its
/// center of mass.
/// </summary>
//-----------------------------------------------------------------------------
void cgRagdollController::applyImpulseTo( cgObjectNode * boneNode, const cgVector3 & impulse )
{
    // Find the associated bone.
    for ( size_t i = 0; i < mBones.size(); ++i )
    {
        if ( mBones[i].node == boneNode )
        {
            mBones[i].body->applyImpulse( impulse );
            return;
        
        } // End if match

    } // Next bone
}

//-----------------------------------------------------------------------------
//  Name : constructHierarchy () (Protected, Recursive)
/// <summary>
/// Build the individual ragdoll bone shapes and bodies recursively.
/// </summary>
//-----------------------------------------------------------------------------
void cgRagdollController::constructHierarchy( cgObjectNode * currentNode, cgInt parentBoneIndex )
{
    // Do nothing for now unless this is a bone.
    if ( !currentNode->queryObjectType( RTID_BoneObject ) )
        return;

    // Allows ragdoll?
    cgPropertyContainer & properties = currentNode->getCustomProperties();
    if ( !properties.getProperty( _T("ragdoll_participate"), true ) )
        return;

    // Compute the bounding box of this bone, taking into account any local scale.
    cgBoundingBox bounds = currentNode->getLocalBoundingBox();
    cgVector3 scale = currentNode->getScale();
    bounds.max.x *= scale.x;
    bounds.max.y *= scale.y;
    bounds.max.z *= scale.z;
    bounds.min.x *= scale.x;
    bounds.min.y *= scale.y;
    bounds.min.z *= scale.z;

    // Backup the original transform so that it can be (optionally) restored
    // at a later time through a call to 'restoreHierarchy()'.
    cgTransform originalTransform = currentNode->getWorldTransform();

    // Strip scale from the current node's transform before applying.
    cgTransform t = currentNode->getWorldTransform(false);
    t.setLocalScale( 1, 1, 1 );

    // Create a shape for this bone.
    //cgPhysicsShape * shape = new cgCapsuleShape( mWorld, bounds );
    cgPhysicsShape * shape = new cgSphereShape( mWorld, bounds );

    // Define the properties of the rigid body to represent this bone.
    cgRigidBodyCreateParams cp;
    cp.model            = cgPhysicsModel::RigidDynamic;
    cp.initialTransform = t;
    cp.quality          = cgSimulationQuality::Default;
    cp.mass             = 25; // 25kg by default // ToDo: Get actual node mass.

    // Create the rigid body.
    cgRigidBody * rigidBody = new cgRigidBody( mWorld, shape, cp );
    rigidBody->addReference( CG_NULL );
    rigidBody->enableCustomGravity( true );
    rigidBody->setCustomGravity( cgVector3( 0, -19.0f, 0 ) );

    // Assign to the correct material group.
    NewtonBodySetMaterialGroupID( rigidBody->getInternalBody(), mWorld->getDefaultMaterialGroupId( cgDefaultPhysicsMaterialGroup::Ragdoll ) );

    // If this is the first body, set it as the root.
    cgBallJoint * joint = CG_NULL;
    if ( parentBoneIndex < 0 )
    {
    
    } // End if root
    else
    {
        // Rotate pin axis as required.
        t.rotateLocal( properties.getProperty( _T("ragdoll_pinrotx"), 0), 
                       properties.getProperty( _T("ragdoll_pinroty"), 0),
                       properties.getProperty( _T("ragdoll_pinrotz"), 0) );

        // This is a child bone. Connect to parent via ball and socket joint.
        joint = new cgBallJoint( mWorld, rigidBody, mBones[parentBoneIndex].body, t );
        joint->enableBodyCollision( false );
        joint->enableLimits( true );
        joint->setTwistLimits( properties.getProperty( _T("ragdoll_mintwist"), 0), properties.getProperty( _T("ragdoll_maxtwist"), 0) );
        joint->setConeLimit( properties.getProperty( _T("ragdoll_cone"), 20) );
        joint->addReference( CG_NULL );

    } // End if !root

    // Register as an event listener for this body so that we can
    // receive transform events.
    rigidBody->registerEventListener( static_cast<cgPhysicsBodyEventListener*>(this) );

    // Add data describing this bone.
    Bone data;
    data.body               = rigidBody;
    data.joint              = joint;
    data.node               = currentNode;
    data.scale              = scale;
    data.parentBoneIndex    = parentBoneIndex;
    data.originalTransform  = originalTransform;
    data.collidable         = properties.getProperty( _T("ragdoll_collide"), false );
    mBoneLUT[rigidBody]     = mBones.size();
    mBones.push_back( data );
    
    // Recurse into children.
    parentBoneIndex = (cgInt)mBones.size() - 1;
    cgObjectNodeList::iterator itNode;
    cgObjectNodeList & children = currentNode->getChildren();
    for ( itNode = children.begin(); itNode != children.end(); ++itNode )
        constructHierarchy( *itNode, parentBoneIndex );
}

//-----------------------------------------------------------------------------
// Name : onPhysicsBodyTransformed ( )
/// <summary>
/// Triggered whenever the physics body designed represent this object during
/// scene dynamics processing is transformed as a result of its simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgRagdollController::onPhysicsBodyTransformed( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e )
{
    // Ignore updates that were not due to dynamics, and did not 
    // originate from our local physics bodies.
    if ( !e->dynamicsUpdate )
        return;
    BoneLUT::const_iterator itIndex = mBoneLUT.find( (cgRigidBody*)sender );
    if ( itIndex == mBoneLUT.end() )
        return;

    // Get the bone data.
    Bone & data = mBones[itIndex->second];

    // Smooth transform.
    // ToDo: Make optional, and use configurable smoothing rate.
    cgQuaternion oldRotation, newRotation;
    cgVector3 oldTranslation, newTranslation;
    cgTransform oldTransform = data.node->getWorldTransform();
    oldTransform.decompose( oldRotation, oldTranslation );
    e->newTransform.decompose( newRotation, newTranslation );
    cgQuaternion::slerp( newRotation, oldRotation, newRotation, (1/60.0f) * 20);
    cgVector3::lerp( newTranslation, oldTranslation, newTranslation, (1/60.0f) * 20);

    // Apply the new transform to the scene node, restoring original scale.
    //cgTransform t = e->newTransform;
    cgTransform t( newRotation, newTranslation );
    t.scaleLocal( data.scale.x, data.scale.y, data.scale.z );
    data.node->setWorldTransform( t, cgTransformSource::Dynamics );
}