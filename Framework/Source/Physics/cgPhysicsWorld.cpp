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
// Name : cgPhysicsWorld.cpp                                                 //
//                                                                           //
// Desc : Manages an individual physics world / scene. All interactive       //
//        dynamic or collidable objects will be registered here.             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPhysicsWorld Module Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsWorld.h>
#include <Physics/cgPhysicsEngine.h>
#include <Physics/cgPhysicsEntity.h>
#include <Physics/cgPhysicsController.h>
#include <Math/cgMathTypes.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgPhysicsWorld Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPhysicsWorld () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsWorld::cgPhysicsWorld( cgPhysicsEngine * pEngine )
{
    // Initialize variables to sensible defaults
    mEngine           = pEngine;
    mWorld            = CG_NULL;
    mToPhysicsScale   = 1.0f;
    mFromPhysicsScale = 1.0f;
    mStepAccumulator  = 0.0f;
    mDefaultGravity   = cgVector3(0,-9.81f,0);
}

//-----------------------------------------------------------------------------
//  Name : ~cgPhysicsWorld () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsWorld::~cgPhysicsWorld()
{
    // Shut down
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by the physics world.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::dispose( bool bDisposeBase )
{
    // Release all registered entities.
    mEntities.clear();

    // Release all registered controllers
    mControllers.clear();

    // Release all cached shapes.
    mShapeCache.clear();
    
    // Shut down physics world.
    if ( mWorld )
        NewtonDestroy( mWorld );

    // Clear variables
    mEngine           = CG_NULL;
    mWorld            = CG_NULL;
    mToPhysicsScale   = 1.0f;
    mFromPhysicsScale = 1.0f;
    mStepAccumulator  = 0.0f;
    mDefaultGravity   = cgVector3(0,-9.81f,0);

    // Dispose of base classes on request.
    if ( bDisposeBase )
        cgEventDispatcher::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : initialize()
/// <summary>
/// Initialize the physics world ready for use.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsWorld::initialize( const cgBoundingBox & WorldSize )
{
    // Bail if already initialized
    if ( mWorld )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Attempted to intialize physics world '%s' for a second time. This call has been ignored.\n"));
        return false;
    
    } // End if already initialized

    // create the Newton World
	mWorld = NewtonCreate();

	// Use the standard x87 floating point model  
	NewtonSetPlatformArchitecture( mWorld, 0 );

	// Set a fixed initial world size
	NewtonSetWorldSize( mWorld, toPhysicsScale( WorldSize.min ), toPhysicsScale( WorldSize.max ) ); 

	// Configure the Newton world to use iterative solve mode 0
	// this is the most efficient but the less accurate mode
	NewtonSetSolverModel( mWorld, 1 );

    //cgInt32 defaultMaterialId = NewtonMaterialGetDefaultGroupID( mWorld );
    //NewtonMaterialSetDefaultElasticity( mWorld, defaultMaterialId, defaultMaterialId, -1000 );
    //NewtonMaterialSetDefaultSoftness( mWorld, defaultMaterialId, defaultMaterialId, 0.0f );
    //NewtonMaterialSetSurfaceThickness( mWorld, defaultMaterialId, defaultMaterialId, 0.001f );

    // Retrieve / create default material group Ids
    mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Standard ]        = NewtonMaterialGetDefaultGroupID( mWorld );
    mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Character ]       = NewtonMaterialCreateGroupID( mWorld );
    mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::PlayerCharacter ] = NewtonMaterialCreateGroupID( mWorld );
    mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::CastOnly ]        = NewtonMaterialCreateGroupID( mWorld );
    mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Ragdoll ]         = NewtonMaterialCreateGroupID( mWorld );

    // Setup default properties.
    NewtonMaterialSetDefaultElasticity( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Character ], 
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Standard ], -1000 );
    NewtonMaterialSetDefaultElasticity( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::PlayerCharacter ], 
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Standard ], -1000 );

    // Disable interaction between certain groups.
    NewtonMaterialSetDefaultCollidable( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Character ],
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Ragdoll ], 0 );
    NewtonMaterialSetDefaultCollidable( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::PlayerCharacter ],
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Ragdoll ], 0 );
    NewtonMaterialSetDefaultCollidable( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Ragdoll ],
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Ragdoll ], 0 );
    
    // Cast only type (ray cast, convex cast) does not interact with anything by default.
    NewtonMaterialSetDefaultCollidable( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Character ],
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::CastOnly ], 0 );
    NewtonMaterialSetDefaultCollidable( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::PlayerCharacter ],
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::CastOnly ], 0 );
    NewtonMaterialSetDefaultCollidable( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Standard ],
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::CastOnly ], 0 );
    NewtonMaterialSetDefaultCollidable( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::Ragdoll ],
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::CastOnly ], 0 );
    NewtonMaterialSetDefaultCollidable( mWorld, mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::CastOnly ],
                                        mDefaultMaterialIds[ cgDefaultPhysicsMaterialGroup::CastOnly ], 0 );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createMaterialGroup()
/// <summary>
/// Create a new custom physics material group to which bodies can be assigned.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgPhysicsWorld::createMaterialGroup( )
{
    return NewtonMaterialCreateGroupID( mWorld );
}

//-----------------------------------------------------------------------------
//  Name : enableMaterialCollision()
/// <summary>
/// Enable / disable collision between bodies belonging to specified material
/// groups.
/// </summary>
//----------------------------------------------------------------------------
void cgPhysicsWorld::enableMaterialCollision( cgInt32 group1, cgInt32 group2, bool collidable )
{
    NewtonMaterialSetDefaultCollidable( mWorld, group1, group2, collidable ? 1 : 0 );
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Allow the physics world to process.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::update( cgFloat fTimeElapsed )
{
    // Initialized?
    if ( mWorld == CG_NULL || fTimeElapsed <= 0 )
        return;

    // Increment the time accumulator
    mStepAccumulator += fTimeElapsed;
    
    // Allow the world to step the simulation at a rate of 60fps by default.
    // ToDo: 6767 - Configurable rate and maximum steps (0 = unlimited, warning)!
    cgDouble fRate     = 1.0 / 60.0;
    cgInt    nMaxSteps = 10; //(cgInt)ceil( mStepAccumulator / fRate );
    for ( cgInt i = 0; i < nMaxSteps && mStepAccumulator >= fRate; ++i, mStepAccumulator -= fRate )
    {
        /*// Allow controllers to update.
        ControllerSet::iterator itController;
        for ( itController = mControllers.begin(); itController != mControllers.end(); ++itController )
            (*itController)->update( (cgFloat)fRate );*/

        // Trigger 'onPrePhysicsStep' of all listeners (duplicate list in case
        // it is altered in response to event).
        EventListenerList::iterator itListener;
        EventListenerList Listeners = mEventListeners;
        cgPhysicsWorldStepEventArgs Args( (cgFloat)fRate );
        for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
            (static_cast<cgPhysicsWorldEventListener*>(*itListener))->onPrePhysicsStep( this, &Args );

        // Allow controllers to update.
        ControllerSet::iterator itController;
        for ( itController = mControllers.begin(); itController != mControllers.end(); ++itController )
            (*itController)->preStep( (cgFloat)fRate );

        // Simulate newton.
        NewtonUpdate( mWorld, (cgFloat)fRate );

        // Trigger 'onPhysicsStep' of all listeners.
        Listeners = mEventListeners;
        for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
            (static_cast<cgPhysicsWorldEventListener*>(*itListener))->onPhysicsStep( this, &Args );

        // Allow controllers to update.
        for ( itController = mControllers.begin(); itController != mControllers.end(); ++itController )
            (*itController)->postStep( (cgFloat)fRate );

        // Trigger 'onPostPhysicsStep' of all listeners.
        Listeners = mEventListeners;
        for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
            (static_cast<cgPhysicsWorldEventListener*>(*itListener))->onPostPhysicsStep( this, &Args );

        // Clear out any accumulated forces.
        clearForces();
    
    } // Next Step
}

//-----------------------------------------------------------------------------
//  Name : clearForces ()
/// <summary>
/// Clear all forces from the system.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::clearForces( )
{
    // ToDo: Iterate only over objects that are awake?
    EntitySet::iterator itEntity;
    for ( itEntity = mEntities.begin(); itEntity != mEntities.end(); ++itEntity )
        (*itEntity)->clearForces();
}

//-----------------------------------------------------------------------------
//  Name : getInternalWorld()
/// <summary>
/// Retrieve the internal world object specific to Newton. This is not
/// technically part of the public interface and should not be called
/// by the application directly.
/// </summary>
//-----------------------------------------------------------------------------
NewtonWorld * cgPhysicsWorld::getInternalWorld( )
{
    return mWorld;
}

//-----------------------------------------------------------------------------
//  Name : setSystemScale()
/// <summary>
/// Set the amount by which the physics simulation will be scaled up / down
/// depending on the requirements of the application -- for instance, in cases
/// where simulation of extremely small objects is required. Specifying a value
/// of '2' in this case will cause the simulation to treat a 4cm entity as if
/// it were 8cm in size. All values will still be supplied by the application 
/// in the same scale, but modifications are made internally to ensure numerical 
/// stability at this scale. All forces will also be scaled up by an 
/// appropriate amount. This method should only be called *prior* to adding
/// entities to the physics world (bodies, joints, etc.), creating collision
/// shapes or creating / adding physics controllers.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::setSystemScale( cgFloat fScale )
{
    // Invalid if already initialized.
    if ( !mEntities.empty() || !mControllers.empty() )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Application attempted to alter the physics system scale after entities or controllers had already been added. This operation is not currently supported.\n") );
        return;
    
    } // End if failed

    // Generate the conversion factor from game units 
    // to the physics system scale.
    mToPhysicsScale = fScale;
    
    // Reciprocal conversion
    if ( fabs(mToPhysicsScale) < CGE_EPSILON )
        mFromPhysicsScale = 0.0f;
    else
        mFromPhysicsScale = 1.0 / mToPhysicsScale;
}

//-----------------------------------------------------------------------------
//  Name : setDefaultGravity()
/// <summary>
/// Set the default force of gravity applied to all bodies that exist within
/// this physics world. Individual bodies can supply their own gravity values
/// to override this default. See 'cgPhysicsBody::setCustomGravity()' and
/// 'cgPhysicsBody::enableCustomGravity()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::setDefaultGravity( const cgVector3 & gravity )
{
    mDefaultGravity = gravity;
}

//-----------------------------------------------------------------------------
//  Name : getDefaultGravity()
/// <summary>
/// Get the default force of gravity applied to all bodies that exist within
/// this physics world. Individual bodies can supply their own gravity values
/// to override this default. See 'cgPhysicsBody::setCustomGravity()' and
/// 'cgPhysicsBody::enableCustomGravity()'.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgPhysicsWorld::getDefaultGravity( ) const
{
    return mDefaultGravity;
}

//-----------------------------------------------------------------------------
//  Name : addEntity()
/// <summary>
/// Add the specified entity (body, joint, etc.) to the physics world.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::addEntity( cgPhysicsEntity * pEntity )
{
    // Add to our internal list of registered entities.
    mEntities.insert( pEntity );
}

//-----------------------------------------------------------------------------
//  Name : removeEntity()
/// <summary>
/// Remove the specified entity (body, joint, etc.) from the physics world.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::removeEntity( cgPhysicsEntity * pEntity )
{
    // Remove from our internal list of registered entities.
    mEntities.erase( pEntity );
}

//-----------------------------------------------------------------------------
//  Name : addController()
/// <summary>
/// Add the specified controller to the physics world.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::addController( cgPhysicsController * pController )
{
    // Add to our internal list of registered controllers.
    mControllers.insert( pController );
}

//-----------------------------------------------------------------------------
//  Name : removeController()
/// <summary>
/// Remove the specified controller from the physics world.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::removeController( cgPhysicsController * pController )
{
    // Remove from our internal list of registered controllers.
    mControllers.erase( pController );
}

//-----------------------------------------------------------------------------
//  Name : getExistingShape()
/// <summary>
/// Find a shape with the specified properties if it exists in the shape
/// cache.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsShape * cgPhysicsWorld::getExistingShape( const cgPhysicsShapeCacheKey & Key ) const
{
    PhysicsShapeCacheMap::const_iterator itShape = mShapeCache.find( Key );
    if ( itShape == mShapeCache.end() )
        return CG_NULL;
    return itShape->second;
}

//-----------------------------------------------------------------------------
//  Name : addShapeToCache()
/// <summary>
/// Add the specified shape to the shape cache if one with matching properties
/// does not already exist.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsWorld::addShapeToCache( cgPhysicsShape * pShape )
{
    if ( !getExistingShape( cgPhysicsShapeCacheKey(pShape) ) )
    {
        mShapeCache[ cgPhysicsShapeCacheKey(pShape) ] = pShape;
        return true;
    
    } // End if !exists

    // Already exists
    return false;
}

//-----------------------------------------------------------------------------
//  Name : removeShapeFromCache()
/// <summary>
/// Remove the specified shape from the shape cache.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::removeShapeFromCache( cgPhysicsShape * pShape )
{
    mShapeCache.erase( cgPhysicsShapeCacheKey(pShape) );
}

//-----------------------------------------------------------------------------
//  Name : rayCastClosest()
/// <summary>
/// Find the closest intersected physics body along the specified ray.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsWorld::rayCastClosest( const cgVector3 & from, const cgVector3 & to, cgCollisionContact & closestContact )
{
    // Reset contact structure.
    closestContact = cgCollisionContact();

    // Run the query.
    NewtonWorldRayCast( mWorld, from, to, rayCastClosestFilter, &closestContact, CG_NULL );

    // Anything found?
    return ( closestContact.body != CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : rayCastAll()
/// <summary>
/// Collect a list of all intersected physics body along the specified ray. The
/// list of contacts can optionally be sorted by distance with the closest
/// intersection first.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsWorld::rayCastAll( const cgVector3 & from, const cgVector3 & to, bool sortContacts, cgCollisionContact::Array & contacts )
{
    // Reset contact list.
    contacts.clear();

    // Run the query.
    NewtonWorldRayCast( mWorld, from, to, rayCastAllFilter, &contacts, CG_NULL );

    // ToDo: Sort the contacts by distance.

    // Anything found?
    return !contacts.empty();
}

//-----------------------------------------------------------------------------
//  Name : rayCast()
/// <summary>
/// Perform a ray cast and execute the specified callback whenever a new
/// potential contact is encountered.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsWorld::rayCast( const cgVector3 & from, const cgVector3 & to, RayCastPreFilterCallback preFilter, RayCastFilterCallback filter, void * userData )
{
    // Build the user data.
    RayCastFilterCallbackData data;
    data.filterCallback = filter;
    data.preFilterCallback = preFilter;
    data.userData = userData;

    // Run the query.
    NewtonWorldRayCast( mWorld, from, to, (filter) ? rayCastFilter : CG_NULL, &data, (preFilter) ? rayCastPreFilter : CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : rayCastAllFilter() (Protected, Static)
/// <summary>
/// TODO
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgPhysicsWorld::rayCastAllFilter( const NewtonBody* const body, const cgFloat * const hitNormal, cgInt collisionId, void* const userData, cgFloat intersectParam )
{
    // Build contact.
    cgCollisionContact contact;
    contact.intersectParam = intersectParam;
    contact.body           = (cgPhysicsBody*)NewtonBodyGetUserData( body );
    contact.contactNormal  = hitNormal;
    contact.collisionId    = (cgInt32)collisionId;
    
    // Add to list.
    cgCollisionContact::Array * contacts = (cgCollisionContact::Array*)userData;
    contacts->push_back( contact );
    
    // Find all
    return 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : rayCastClosestFilter() (Protected, Static)
/// <summary>
/// TODO
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgPhysicsWorld::rayCastClosestFilter( const NewtonBody* const body, const cgFloat * const hitNormal, cgInt collisionId, void* const userData, cgFloat intersectParam )
{
    cgCollisionContact * contact = (cgCollisionContact*)userData;
    if ( intersectParam < contact->intersectParam )
    {
        contact->intersectParam = intersectParam;
        contact->body           = (cgPhysicsBody*)NewtonBodyGetUserData( body );
        contact->contactNormal  = hitNormal;
        contact->collisionId    = (cgInt32)collisionId;
    
    } // End if closer

    // Find closest
    return intersectParam;
}

//-----------------------------------------------------------------------------
//  Name : rayCastFilter() (Protected, Static)
/// <summary>
/// TODO
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgPhysicsWorld::rayCastFilter( const NewtonBody* const body, const cgFloat * const hitNormal, cgInt collisionId, void* const userData, cgFloat intersectParam )
{
    cgPhysicsBody * outBody = static_cast<cgPhysicsBody*>(NewtonBodyGetUserData( body ));
    RayCastFilterCallbackData * outData = static_cast<RayCastFilterCallbackData*>(userData);

    // Pass through to user callback.
    return outData->filterCallback( outBody, hitNormal, collisionId, outData->userData, intersectParam );
}

//-----------------------------------------------------------------------------
//  Name : rayCastPreFilter() (Protected, Static)
/// <summary>
/// TODO
/// </summary>
//-----------------------------------------------------------------------------
cgUInt cgPhysicsWorld::rayCastPreFilter( const NewtonBody* const body, const NewtonCollision * const collision, void* const userData )
{
    cgPhysicsBody * outBody = static_cast<cgPhysicsBody*>(NewtonBodyGetUserData( body ));
    cgPhysicsShape * outShape = CG_NULL; //static_cast<cgPhysicsShape*>(NewtonCollisionGetUserData( collision ));
    RayCastFilterCallbackData * outData = static_cast<RayCastFilterCallbackData*>(userData);

    // ToDo: Temporary.
    // Ray casting ignores all non player character materials, but
    // allows for intersection with player characters and others.
    if ( !outBody || 
        NewtonBodyGetMaterialGroupID(body) == outBody->getPhysicsWorld()->getDefaultMaterialGroupId( cgDefaultPhysicsMaterialGroup::Character ) )
        return false;

    // Pass through to user callback.
    return (outData->preFilterCallback( outBody, outShape, outData->userData ) != 0);
}

//-----------------------------------------------------------------------------
//  Name : getDefaultMaterialGroupId()
/// <summary>
/// Get the material group id of the specified default material group that
/// will be automatically assigned (by default) to certain types of objects
/// in the world.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgPhysicsWorld::getDefaultMaterialGroupId( cgDefaultPhysicsMaterialGroup::Base group ) const
{
    return mDefaultMaterialIds[ (size_t)group ];
}