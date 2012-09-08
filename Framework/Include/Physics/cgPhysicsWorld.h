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
// Name : cgPhysicsWorld.h                                                   //
//                                                                           //
// Desc : Manages an individual physics world / scene. All interactive       //
//        dynamic or collidable objects will be registered here.             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSWORLD_H_ )
#define _CGE_CGPHYSICSWORLD_H_

//-----------------------------------------------------------------------------
// cgPhysicsWorld Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Physics/cgPhysicsShape.h>
#include <Scripting/cgScriptInterop.h>
#include <System/cgEventDispatcher.h>
#include <Math/cgMathTypes.h>
#include <Math/cgBoundingBox.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsEngine;
class cgPhysicsEntity;
class cgPhysicsController;

// Newton Game Dynamisc.
struct NewtonWorld;

//-----------------------------------------------------------------------------
// Event Argument Definitions
//-----------------------------------------------------------------------------
struct CGE_API cgPhysicsWorldStepEventArgs
{
    cgPhysicsWorldStepEventArgs( cgFloat _step ) :
        step( _step ) {}
    cgFloat step;
};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPhysicsBodyEventListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever physics body events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsWorldEventListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPhysicsWorldEventListener, cgEventListener, "PhysicsWorldEventListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onPrePhysicsStep  ( cgPhysicsWorld * sender, cgPhysicsWorldStepEventArgs * e ) {};
    virtual void    onPostPhysicsStep ( cgPhysicsWorld * sender, cgPhysicsWorldStepEventArgs * e ) {};
};

//-----------------------------------------------------------------------------
//  Name : cgPhysicsWorld (Class)
/// <summary>
/// Describes an individual instance of a physics / dynamics world.
/// This world contains all of the physics bodies / objects that describe
/// how physical object interaction should take place. Multiple worlds
/// can be created where necessary.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsWorld : public cgEventDispatcher
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPhysicsWorld, cgEventDispatcher, "PhysicsWorld" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgPhysicsBody;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPhysicsWorld( cgPhysicsEngine * engine );
    virtual ~cgPhysicsWorld( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                initialize          ( const cgBoundingBox & worldSize );
    void                update              ( cgFloat timeDelta );
    void                setSystemScale      ( cgFloat scale );
    void                clearForces         ( );
    void                setDefaultGravity   ( const cgVector3 & gravity );
    const cgVector3   & getDefaultGravity   ( ) const;

    // Internal utilities
    NewtonWorld       * getInternalWorld    ( );

    // Entity management
    void                addEntity           ( cgPhysicsEntity * entity );
    void                removeEntity        ( cgPhysicsEntity * entity );

    // Controller management
    void                addController       ( cgPhysicsController * controller );
    void                removeController    ( cgPhysicsController * controller );

    // Shape management
    cgPhysicsShape    * getExistingShape    ( const cgPhysicsShapeCacheKey & key ) const;
    
    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    inline cgFloat          toPhysicsScale      ( ) const { return (cgFloat)mToPhysicsScale; }
    inline cgFloat          fromPhysicsScale    ( ) const { return (cgFloat)mFromPhysicsScale; }
    inline cgFloat          toPhysicsScale      ( cgFloat value ) const { return value * (cgFloat)mToPhysicsScale; }
    inline cgFloat          fromPhysicsScale    ( cgFloat value ) const { return value * (cgFloat)mFromPhysicsScale; }
    inline cgVector3        toPhysicsScale      ( const cgVector3 & value ) const { return value * (cgFloat)mToPhysicsScale; }
    inline cgVector3        fromPhysicsScale    ( const cgVector3 & value ) const { return value * (cgFloat)mFromPhysicsScale; }
    inline cgBoundingBox    toPhysicsScale      ( const cgBoundingBox & value ) const { return value * (cgFloat)mToPhysicsScale; }
    inline cgBoundingBox    fromPhysicsScale    ( const cgBoundingBox & value ) const { return value * (cgFloat)mFromPhysicsScale; }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_SET_DECLARE(cgPhysicsEntity*, EntitySet )
    CGE_SET_DECLARE(cgPhysicsController*, ControllerSet )
    CGE_MAP_DECLARE(cgPhysicsShapeCacheKey, cgPhysicsShape*, PhysicsShapeCacheMap )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                addShapeToCache     ( cgPhysicsShape * shape );
    void                removeShapeFromCache( cgPhysicsShape * shape );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgPhysicsEngine       * mEngine;            // Parent physics engine to which this world belongs.
    NewtonWorld           * mWorld;             // Newton's main physics world interface.
    cgDouble                mStepAccumulator;   // Accumulated time for stepping the physics simulation.
    cgDouble                mToPhysicsScale;    // Conversion scalar designed to convert values from game scale to physics system scale.
    cgDouble                mFromPhysicsScale;  // Conversion scalar designed to convert values from physics system scale to game scale.
    EntitySet               mEntities;          // List of entities registered with the world.
    ControllerSet           mControllers;       // List of controllers registered with the world.
    PhysicsShapeCacheMap    mShapeCache;        // Dictionary of all physics shapes with common properties.
    cgVector3               mDefaultGravity;    // Default gravity force for this physics world. Can be overridden by individual bodies.
};

#endif // !_CGE_CGPHYSICSWORLD_H_