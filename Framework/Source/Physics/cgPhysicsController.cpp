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
// Name : cgPhysicsController.cpp                                            //
//                                                                           //
// Desc : Base classes from which specific types of physics controller       //
//        should derive. A physics controller is associated with an          //
//        individual scene object and handles the integration and update     //
//        process of that object in the physics world.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPhysicsController Module Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsController.h>
#include <Physics/cgPhysicsWorld.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgPhysicsController::ControllerAllocTypeMap cgPhysicsController::mRegisteredControllers;

///////////////////////////////////////////////////////////////////////////////
// cgPhysicsController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPhysicsController () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsController::cgPhysicsController( cgPhysicsWorld * pWorld )
{
    // Initialize variables to sensible defaults
    mWorld        = pWorld;
    mParentObject = CG_NULL;
    mIsCollidable = true;

    // Add this controller to the world
    pWorld->addController( this );
}

//-----------------------------------------------------------------------------
//  Name : ~cgPhysicsController () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsController::~cgPhysicsController()
{
    dispose(false);
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by the physics world.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsController::dispose( bool bDisposeBase )
{
    // Remove this controller from the world
    if ( mWorld )
        mWorld->removeController( this );

    // Clear variables
    mWorld        = CG_NULL;
    mParentObject = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : registerType() (Static)
/// <summary>
/// Allows the application to register all of the various controller
/// types that are supported by the application. These types can then be 
/// referenced directly in the environment definition file by name / 
/// initialization data etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsController::registerType( const cgString & strTypeName, ControllerAllocFunc pFunction )
{
    // Store the function pointer
    mRegisteredControllers[ strTypeName ] = pFunction;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Call this function to allocate a controller of any given type, by
/// name. The controller will then ultimately be initialized based on the
/// XML based data passed (if applicable).
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsController * cgPhysicsController::createInstance( const cgString & strTypeName, cgPhysicsWorld * pWorld )
{
    ControllerAllocFunc Allocate = CG_NULL;
    ControllerAllocTypeMap::iterator itAlloc;

    // Find the allocation function based on the name specified
    itAlloc = mRegisteredControllers.find( strTypeName );
    if ( itAlloc == mRegisteredControllers.end() )
        return CG_NULL;

    // Extract the function pointer
    Allocate = itAlloc->second;
    if ( Allocate == CG_NULL )
        return CG_NULL;

    // Call the registered allocation function
    return Allocate( strTypeName, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : preStep ()
/// <summary>
/// Called just prior to the upcoming physics simulation step.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsController::preStep( cgFloat fElapsedTime )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : postStep ()
/// <summary>
/// Called just after the most recent physics simulation step.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsController::postStep( cgFloat fElapsedTime )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : setParentObject () (Protected)
/// <summary>
/// Call this function to update the controller's parent object.
/// Note : This is a protected function, and is called by the
/// cgObjectNode::SetPhysicsController function when attaching this to 
/// an object.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsController::setParentObject( cgObjectNode * pParentObject )
{
    mParentObject = pParentObject;
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Virtual)
/// <summary>
/// Allow the controller to initialize (called after setting up).
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsController::initialize( )
{
    // Nothing in base implementation currently.

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setCollidable () (Virtual)
/// <summary>
/// Enable / disable collision detection for this handled object.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsController::setCollidable( bool bCollidable )
{
    mIsCollidable = bCollidable;
}

//-----------------------------------------------------------------------------
//  Name : applyForce () (Virtual)
/// <summary>
/// Apply a force to the object managed by this controller.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsController::applyForce( const cgVector3 & vecForce )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : supportsInputChannels() (Virtual)
/// <summary>
/// Determine if this controller supports the reading of input channel
/// states from its parent object in order to apply motion (rather than
/// through direct force or velocity manipulation).
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsController::supportsInputChannels( ) const
{
    // Input channels not supported by default.
    return false;
}