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
// Name : cgSceneElement.h                                                   //
//                                                                           //
// Desc : Base class from which all scene element types derive. A scene      //
//        element provides custom/extended data and/or functionality for a   //
//        loaded scene. Examples of a custom scene element might include     //
//        the configuration and rendering of a sky box/dome/plane, data for  //
//        post processes, configuration of scene navigation data, etc.       //
//        It essentially gives the user the ability to extend and expose     //
//        new scene related functionality to the engine / editor such that   //
//        it can be manipulated.                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCENEELEMENT_H_ )
#define _CGE_CGSCENEELEMENT_H_

//-----------------------------------------------------------------------------
// cgSceneElement Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldComponent.h>
#include <World/cgWorldTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;
class cgCameraNode;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {C6F134B2-EB38-497E-A366-2116E7CE6C43}
const cgUID RTID_SceneElement = { 0xC6F134B2, 0xEB38, 0x497E, { 0xA3, 0x66, 0x21, 0x16, 0xE7, 0xCE, 0x6C, 0x43 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSceneElement (Class)
/// <summary>
/// Base class from which all scene element types derive. A scene element 
/// provides custom/extended data and/or functionality for a loaded scene. 
/// Examples of a custom scene element might include the configuration and 
/// rendering of a sky box/dome/plane, data for post processes, configuration 
/// of scene navigation data, etc. It essentially gives the user the ability to
/// extend and expose new scene related functionality to the engine / editor
/// such that it can be manipulated.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSceneElement : public cgWorldComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSceneElement, cgWorldComponent, "SceneElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSceneElement( cgUInt32 referenceId, cgScene * scene );
    virtual ~cgSceneElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static const cgSceneElementTypeDesc::Map  & getRegisteredTypes  ( );
    static void                                 registerType        ( const cgUID & globalIdentifier, const cgString & name, 
                                                                      cgSceneElementTypeDesc::AllocNewFunc sceneElementAllocNew );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgScene                   * getParentScene          ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                update                  ( cgFloat timeDelta );
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SceneElement; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgScene   * mParentScene;  // Parent scene to which this scene element belongs.

private:
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgSceneElementTypeDesc::Map  mRegisteredSceneElementTypes;  // All of the scene element types registered with the system.
};

#endif // !_CGE_CGSCENEELEMENT_H_