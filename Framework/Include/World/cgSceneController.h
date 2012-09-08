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
// Name : cgSceneController.h                                                //
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

#pragma once
#if !defined( _CGE_CGSCENECONTROLLER_H_ )
#define _CGE_CGSCENECONTROLLER_H_

//-----------------------------------------------------------------------------
// cgSceneController Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSceneController (Base Class)
/// <summary>
/// Base class for controllers that will manipulate / control any loaded
/// scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSceneController : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgSceneController, "SceneController" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSceneController( cgScene * parentScene );
    virtual ~cgSceneController( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgScene       * getParentScene      ( );
    bool            isControllerEnabled ( ) const;
    void            setControllerEnabled( bool enabled );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    update              ( cgFloat timeDelta );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgScene   * mParentScene;       // The parent scene that owns this controller.
    bool        mControllerEnabled; // Is the controller currently enabled.
};

#endif // !_CGE_CGSCENECONTROLLER_H_