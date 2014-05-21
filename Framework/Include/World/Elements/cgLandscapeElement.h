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
// Name : cgLandscapeElement.h                                               //
//                                                                           //
// Desc : Class that provides configuration and management of scene          //
//        landscape data, exposed as a scene element type. This provides     //
//        the integration between the application (such as the editing       //
//        environment) and the relevant components of the scene renderer.    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLANDSCAPEELEMENT_H_ )
#define _CGE_CGLANDSCAPEELEMENT_H_

//-----------------------------------------------------------------------------
// cgLandscapeElement Header Includes
//-----------------------------------------------------------------------------
#include <World/cgSceneElement.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgLandscape;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {CE77E072-F3DD-4C3B-A8D4-D0719DB8D69A}
const cgUID RTID_LandscapeElement = { 0xce77e072, 0xf3dd, 0x4c3b, { 0xa8, 0xd4, 0xd0, 0x71, 0x9d, 0xb8, 0xd6, 0x9a } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgLandscapeElement (Class)
/// <summary>
/// Class that provides configuration and management of scene landscape data,
/// exposed as a scene element type. This provides the integration between the
/// application (such as the editing environment) and the relevant components 
/// of the scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLandscapeElement : public cgSceneElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgLandscapeElement, cgSceneElement, "LandscapeElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgLandscapeElement( cgUInt32 referenceId, cgScene * scene );
    virtual ~cgLandscapeElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgSceneElement     * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgLandscape               * getLandscape            ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_LandscapeElement; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGLANDSCAPEELEMENT_H_