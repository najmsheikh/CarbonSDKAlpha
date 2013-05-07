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
// Name : cgPhysicsEngine.h                                                  //
//                                                                           //
// Desc : Provides all of the physical interaction and movement functions    //
//        used for manipulating dynamic objects within the world.            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSENGINE_H_ )
#define _CGE_CGPHYSICSENGINE_H_

//-----------------------------------------------------------------------------
// cgPhysicsEngine Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsWorld;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPhysicsEngine (Class)
/// <summary>
/// The primary physics system used to provide the motion and dynamics
/// of any registered scene objects.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsEngine : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgPhysicsEngine, "PhysicsEngine" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgPhysicsWorld;

public:
    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    struct CGE_API InitConfig               // The selected physics engine configuration options
    {
        bool        useVDB;                 // Integrate with the visual debugger?
        bool        disableDeactivation;    // Prevent bodies from deactivating at rest. This allows us to view timings in the VDB.
        bool        verboseOutput;          // Log debug messages.
        
        // Constructor
        InitConfig()
        {
            useVDB              = false;
            disableDeactivation = false;
            verboseOutput       = false;
        
        } // End Constructor
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPhysicsEngine( );
    virtual ~cgPhysicsEngine( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgPhysicsEngine    * getInstance         ( );
    static void                 createSingleton     ( );
    static void                 destroySingleton    ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgConfigResult::Base    loadConfig              ( const cgString & fileName );
    cgConfigResult::Base    loadDefaultConfig       ( );
    bool                    saveConfig              ( const cgString & fileName );
    InitConfig              getConfig               ( ) const;
    bool                    initialize              ( );

    // World management
    cgPhysicsWorld        * createWorld             ( );
    cgUInt32                getWorldCount           ( ) const;
    cgPhysicsWorld        * getWorld                ( cgUInt32 index );
    void                    removeWorld             ( cgPhysicsWorld * world, bool dispose = true );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE(cgPhysicsWorld*, WorldArray )

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    InitConfig  mConfig;        // Physics engine configuration settings.
    bool        mConfigLoaded;  // Has the configuration been loaded yet?
    WorldArray  mWorlds;        // Array of allocated world objects.
    
private:
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgPhysicsEngine * mSingleton;   // Static singleton object instance.
};

#endif // !_CGE_CGPHYSICSENGINE_H_