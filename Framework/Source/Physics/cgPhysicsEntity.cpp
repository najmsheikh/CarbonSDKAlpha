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
// Name : cgPhysicsEntity.cpp                                                //
//                                                                           //
// Desc : Base class from which all types of physics objects that have a     //
//        physical presence within the physics world should derive. This     //
//        most commonly includes bodies and joints for instance.             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPhysicsEntity Module Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsEntity.h>
#include <Physics/cgPhysicsWorld.h>

///////////////////////////////////////////////////////////////////////////////
// cgPhysicsEntity Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPhysicsEntity () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsEntity::cgPhysicsEntity( cgPhysicsWorld * pWorld ) : cgReference( cgReferenceManager::generateInternalRefId() )
{
    // Initialize variables to sensible defaults
    mWorld                = pWorld;
    mEntityPosition       = cgVector3(0,0,0);
    mPrevEntityPosition   = cgVector3(0,0,0);
    cgQuaternion::identity( mEntityRotation );
    cgQuaternion::identity( mPrevEntityRotation );

    // Add this entity to the world
    pWorld->addEntity( this );
}

//-----------------------------------------------------------------------------
//  Name : ~cgPhysicsEntity () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsEntity::~cgPhysicsEntity()
{
    // Clean up.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgPhysicsEntity::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Remove this entity from the world
    if ( mWorld )
        mWorld->removeEntity( this );

    // Clear variables
    mWorld = CG_NULL;

    // Call base if requested.
    if ( bDisposeBase == true )
        cgReference::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsEntity::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_PhysicsEntity )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getTransform () (Virtual)
/// <summary>
/// Retrieve the current transformation of the body within the physics 
/// world.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform cgPhysicsEntity::getTransform( cgDouble fInterp /* = 1.0f */ ) const
{
    // Generate the current position and orientation based on the requested
    // interpolation parameter.
    cgQuaternion qRot;
    cgQuaternion::slerp( qRot, mPrevEntityRotation, mEntityRotation, (cgFloat)fInterp );
    cgVector3 vPos = mPrevEntityPosition + ((mEntityPosition - mPrevEntityPosition) * (cgFloat)fInterp );

    // Generate the final transform.
    return cgTransform( qRot, vPos );
}

//-----------------------------------------------------------------------------
//  Name : setTransform () (Virtual)
/// <summary>
/// Update the current transformation of the body within the physics 
/// world.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsEntity::setTransform( const cgTransform & Transform )
{
    Transform.decompose( mEntityRotation, mEntityPosition );
    mPrevEntityRotation = mEntityRotation;
    mPrevEntityPosition = mEntityPosition;
}

//-----------------------------------------------------------------------------
//  Name : clearForces () (Virtual)
/// <summary>
/// Clear out any accumulated forces that have been processed.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsEntity::clearForces( )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : getPhysicsWorld ()
/// <summary>
/// Get the physics world in which this body was created.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsWorld * cgPhysicsEntity::getPhysicsWorld( ) const
{
    return mWorld;
}