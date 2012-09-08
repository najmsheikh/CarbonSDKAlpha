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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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

    // Success!
    return true;
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
    if ( mWorld == CG_NULL || fTimeElapsed == 0.0f )
        return;

    // Increment the time accumulator
    mStepAccumulator += fTimeElapsed;
    
    // Allow the world to step the simulation at a rate of 60fps by default.
    // ToDo: 6767 - Configurable maximum steps (0 = unlimited, warning)!
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

        // Simulate
        NewtonUpdate( mWorld, (cgFloat)fRate );

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