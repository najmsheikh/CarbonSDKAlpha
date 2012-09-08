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
#include <Input/cgInputDriver.h>
#include <World/cgWorld.h>
#include <Interface/cgUIManager.h>

//-----------------------------------------------------------------------------
// Name : cGamePlayState () (Constructor)
/// <summary> cGamePlayState Class Constructor </summary>
//-----------------------------------------------------------------------------
cGamePlayState::cGamePlayState( const cgString & stateId ) : cgAppState( stateId )
{
    // Initialize variables to sensible defaults
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
    // Exit event?
    cgInputDriver * inputDriver = cgInputDriver::getInstance();
    if ( inputDriver->isKeyPressed( cgKeys::Escape ) )
        raiseEvent( _T("Exit") );

    // Call base class implementation
    cgAppState::update();
}

//-----------------------------------------------------------------------------
// Name : render () (Virtual)
// Desc : Called by the game state manager in order to allow this state (and
//        all other states) to render whatever is necessary.
//-----------------------------------------------------------------------------
void cGamePlayState::render( )
{
    // Build a string to display.
    cgWorld * world = cgWorld::getInstance();
    cgString output = cgString::format( _T("Hello World!\n\nThe world has been loaded and it contains %i scene(s)."), world->getSceneCount() ); 

    // Print it to the screen.
    cgUIManager * interfaceManager = cgUIManager::getInstance();
    interfaceManager->selectFont( _T("fixed_v01_white") );
    interfaceManager->printText( cgPoint(10,10), output, cgTextFlags::Multiline, 0xFFFF0000 );

    // Call base class implementation
    cgAppState::render();
}