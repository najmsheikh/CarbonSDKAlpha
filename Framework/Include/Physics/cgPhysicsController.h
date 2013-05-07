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
// Name : cgPhysicsController.h                                              //
//                                                                           //
// Desc : Base classes from which specific types of physics controller       //
//        should derive. A physics controller is associated with an          //
//        individual scene object and handles the integration and update     //
//        process of that object in the physics world.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSCONTROLLER_H_ )
#define _CGE_CGPHYSICSCONTROLLER_H_

//-----------------------------------------------------------------------------
// cgPhysicsController Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsWorld;
class cgVector3;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPhysicsController (Class)
/// <summary>
/// Base class from which specific types of physics controllers should
/// derive. A physics controller is associated with an individual game 
/// object and handles the actual update process of that object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsController : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgPhysicsController, "PhysicsController" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgObjectNode;

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    typedef cgPhysicsController* (*ControllerAllocFunc)( const cgString&, cgPhysicsWorld* );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPhysicsController( cgPhysicsWorld * world );
    virtual ~cgPhysicsController( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static void                 registerType        ( const cgString & typeName, ControllerAllocFunc func );
    static cgPhysicsController* createInstance      ( const cgString & typeName, cgPhysicsWorld * world );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    preStep                 ( cgFloat timeDelta );
    virtual void    postStep                ( cgFloat timeDelta );
    virtual bool    initialize              ( );
    virtual void    applyForce              ( const cgVector3 & force );
    virtual bool    supportsInputChannels   ( ) const;
    virtual void    setCollidable           ( bool collidable );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            setParentObject         ( cgObjectNode * parentObject );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgObjectNode      * mParentObject;      // The object node that we're handling physics for.
    cgPhysicsWorld    * mWorld;             // The parent physics world on which this controller is operating.
    bool                mIsCollidable;      // Is collision currently enabled (defaults to true)?

private:
    //-------------------------------------------------------------------------
    // Private Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgString, ControllerAllocFunc, ControllerAllocTypeMap)

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static ControllerAllocTypeMap   mRegisteredControllers; // All of the controller types registered with the system
};

#endif // !_CGE_CGPHYSICSCONTROLLER_H_