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
// Name : cGamePlayState.cpp                                                 //
//                                                                           //
// Desc : Primary game state used to supply most functionality for the       //
//        demonstration project.                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// cGamePlayState Module Includes
//-----------------------------------------------------------------------------
#include "States/cGamePlayState.h"

// CGE Includes
#include <cgBase.h>
#include <Rendering/cgRenderDriver.h>
#include <Input/cgInputDriver.h>
#include <World/cgWorld.h>
#include <World/cgScene.h>
#include <World/Objects/cgCameraObject.h>

//-----------------------------------------------------------------------------
// Name : cGamePlayState () (Constructor)
/// <summary> cGamePlayState Class Constructor </summary>
//-----------------------------------------------------------------------------
cGamePlayState::cGamePlayState( const cgString & stateId ) : cgAppState( stateId )
{
    // Initialize variables to sensible defaults
    mScene          = CG_NULL;
    mSceneView      = CG_NULL;
    mCamera         = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : ~cGamePlayState () (Destructor)
/// <summary> cGamePlayState Class Destructor </summary>
//-----------------------------------------------------------------------------
cGamePlayState::~cGamePlayState()
{
}

//-----------------------------------------------------------------------------
// Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy supports a
/// particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cGamePlayState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_Sample_GamePlayState )
        return true;

    // Supported by base?
    return cgAppState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : initialize () (Virtual)
/// <summary>
/// The state has been registered and is being initialized. This function is 
/// somewhat like a constructor in that it is now part of the game state system
/// and any registration-time processing can take place.
/// </summary>
//-----------------------------------------------------------------------------
bool cGamePlayState::initialize( )
{
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : begin () (Virtual)
/// <summary>
/// This signifies that the state has actually been selected and activated by 
/// the state management system. This will generally be the point at which any 
/// specific resources relevant for the execution of this state will be 
/// built/loaded.
/// </summary>
//-----------------------------------------------------------------------------
bool cGamePlayState::begin( )
{
    // Allocate a new scene rendering view. This represents a collection of
    // surfaces / render targets, into which the scene will be rendered. It
    // is possible to create more than one scene view for multiple outputs
    // (perhaps split screen multi player) but in this case we only need one
    // view that spans the entire screen.
    cgRenderDriver * renderDriver = cgRenderDriver::getInstance();
    if ( !(mSceneView = renderDriver->createRenderView( _T("Sample View"), cgScaleMode::Relative, cgRectF(0,0,1,1) ) ) )
        return false;

    // Load the scene
    if ( !loadScene( ) )
        return false;

    // Call base class implementation last.
    return cgAppState::begin();
}

//-----------------------------------------------------------------------------
// Name : End () (Virtual)
// Desc : This state is no longer required / running, and should clean up any
//        allocated resources.
//-----------------------------------------------------------------------------
void cGamePlayState::end( )
{
    // Dispose of scene rendering view
    if ( mSceneView )
        mSceneView->deleteReference();
    mSceneView = CG_NULL;

    // Unload the scene
    if ( mScene )
        mScene->unload();
    mScene = CG_NULL;

    // Call base class implementation
    cgAppState::end();
}

//-----------------------------------------------------------------------------
// Name : update () (Virtual)
/// <summary>
/// Called by the game state manager in order to allow this state (and all 
/// other states) to perform any processing in its entirety prior to the 
/// rendering process beginning.
/// </summary>
//-----------------------------------------------------------------------------
void cGamePlayState::update( )
{
    // Run the update process if the state is not currently suspended.
    if ( mScene && !isSuspended() )
    {
        // Allow the scene to perform any necessary update tasks.
        mScene->update();
    
    } // End if !suspended

    // Exit event?
    cgInputDriver * inputDriver = cgInputDriver::getInstance();
    if ( inputDriver->isKeyPressed( cgKeys::Escape ) )
        raiseEvent( _T("Exit") );

    // Call base class implementation
    cgAppState::update();
}

//-----------------------------------------------------------------------------
// Name : loadScene () (Private)
/// <summary>
/// Contains code responsible for loading the main scene.
/// </summary>
//-----------------------------------------------------------------------------
bool cGamePlayState::loadScene( )
{
    // Get access to required systems.
    cgWorld * world = cgWorld::getInstance();

    // Load the first scene from the file.
    if ( !(mScene = world->loadScene( 0x1 )) )
        return false;

    // Create a camera that can view the world.
    mCamera = static_cast<cgCameraNode*>(mScene->createObjectNode( true, RTID_CameraObject, false ));
    
    // Setup camera properties
    mCamera->setFOV( 75.0f );
    mCamera->setNearClip( 0.2f );
    mCamera->setFarClip( 10000.01f );
    mCamera->setPosition( cgVector3(0.0f, 2.0f, -4.0f) );
    mCamera->setUpdateRate( cgUpdateRate::Always );

    // Set up the scene ready for rendering
    mScene->setActiveCamera( mCamera );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : render () (Virtual)
// Desc : Called by the game state manager in order to allow this state (and
//        all other states) to render whatever is necessary.
//-----------------------------------------------------------------------------
void cGamePlayState::render( )
{
    if ( mSceneView )
    {
        // Start rendering to our created scene view (full screen output in this case).
        if ( mSceneView->begin() )
        {
            // Allow the scene to render if one was loaded
            if ( mScene )
                mScene->render();

            // Present the view to the frame buffer
            mSceneView->end( );

        } // End if begun

    } // End if valid view

    // Call base class implementation
    cgAppState::render();
}