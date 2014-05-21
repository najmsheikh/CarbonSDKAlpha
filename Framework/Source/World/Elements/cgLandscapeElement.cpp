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
// Name : cgLandscapeElement.cpp                                             //
//                                                                           //
// Desc : Class that provides configuration and management of scene          //
//        landscape data, exposed as a scene element type. This provides     //
//        the integration between the application (such as the editing       //
//        environment) and the relevant components of the scene renderer.    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgLandscapeElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Elements/cgLandscapeElement.h>

///////////////////////////////////////////////////////////////////////////////
// cgLandscapeElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLandscapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeElement::cgLandscapeElement( cgUInt32 referenceId, cgScene * scene ) : cgSceneElement( referenceId, scene )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgLandscapeElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeElement::~cgLandscapeElement()
{
    // Clean up object resources.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgLandscapeElement::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base class if requested.
    if ( disposeBase == true )
        cgSceneElement::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a scene element of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneElement * cgLandscapeElement::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgLandscapeElement( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_LandscapeElement )
        return true;

    // Supported by base?
    return cgSceneElement::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : getLandscape ( )
/// <summary>
/// Retrieve the landscape being represented by this scene element.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscape * cgLandscapeElement::getLandscape( )
{
    return (mParentScene) ? mParentScene->getLandscape() : CG_NULL;
}