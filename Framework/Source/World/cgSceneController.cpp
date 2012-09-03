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
// Name : cgSceneController.cpp                                              //
//                                                                           //
// Desc : Contains base classes that allow developers to add custom          //
//        behaviors for the modification and management of data at the scene //
//        level. Relevant examples of scene controllers might provide        //
//        animation of scene objects, or management of clutter for instance  //
//        (adding and removing objects when necessary).                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSceneController Module Includes
//-----------------------------------------------------------------------------
#include <World/cgSceneController.h>

///////////////////////////////////////////////////////////////////////////////
// cgSceneController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSceneController () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneController::cgSceneController( cgScene * pParentScene )
{
    // Initialize variables to sensible defaults
    mParentScene          = pParentScene;
    mControllerEnabled    = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSceneController () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneController::~cgSceneController()
{
    // Clear variables
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgSceneController::dispose( bool bDisposeBase )
{
    // Clear variables
    mParentScene = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Perform this controllers update step.
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneController::update( cgFloat fElapsedTime )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : getParentScene ()
/// <summary>
/// Retrieve the parent scene that owns this controller.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgSceneController::getParentScene( )
{
    return mParentScene;
}

//-----------------------------------------------------------------------------
//  Name : isControllerEnabled ()
/// <summary>
/// Is this controller currently enabled?
/// </summary>
//-----------------------------------------------------------------------------
bool cgSceneController::isControllerEnabled( ) const
{
    return mControllerEnabled;
}

//-----------------------------------------------------------------------------
//  Name : setControllerEnabled ()
/// <summary>
/// Enable or disable this scene controller (prevents update method
/// from being triggered).
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneController::setControllerEnabled( bool bEnabled )
{
    mControllerEnabled = bEnabled;
}