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
// Name : cgSceneElement.cpp                                                 //
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

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSceneElement Module Includes
//-----------------------------------------------------------------------------
#include <World/cgSceneElement.h>
#include <World/cgWorldObject.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgSceneElementTypeDesc::Map cgSceneElement::mRegisteredSceneElementTypes;

///////////////////////////////////////////////////////////////////////////////
// cgSceneElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSceneElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneElement::cgSceneElement( cgUInt32 referenceId, cgScene * scene ) : cgWorldComponent( referenceId, scene->getParentWorld() )
{
    // Initialize variables to sensible defaults
    mParentScene    = scene;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSceneElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneElement::~cgSceneElement()
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
void cgSceneElement::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;
    
    // Dispose base class if requested.
    if ( disposeBase == true )
        cgWorldComponent::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : registerType() (Static)
/// <summary>
/// Allows the application to register all of the various scene element types
/// that are supported by the application. These types can then be instantiated
/// and / or referenced directly within the world database.
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneElement::registerType( const cgUID & globalIdentifier, const cgString & name, 
                                   cgSceneElementTypeDesc::AllocNewFunc sceneElementAllocNew )
{
    // Build the descriptor
    cgSceneElementTypeDesc desc;
    desc.globalIdentifier   = globalIdentifier;
    desc.localIdentifier    = 0; // 0 = Unassigned. This will be filled out in the local duplicate.
    desc.name               = name;
    desc.elementAllocNew    = sceneElementAllocNew;
    
    // Store
    mRegisteredSceneElementTypes[ globalIdentifier ] = desc;
}

//-----------------------------------------------------------------------------
//  Name : getRegisteredTypes() (Static)
/// <summary>
/// Retrieve the map containing descriptions of all registered scene element 
/// types that are supported by the application. 
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneElementTypeDesc::Map & cgSceneElement::getRegisteredTypes( )
{
    return mRegisteredSceneElementTypes;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSceneElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SceneElement )
        return true;

    // Supported by base?
    return cgWorldComponent::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgSceneElement::getDatabaseTable( ) const
{
    // Scene elements are not required to serialize anything
    // but can if they wish.
    return cgString::Empty;
}

//-----------------------------------------------------------------------------
//  Name : getParentScene ()
/// <summary>
/// Retrieve a reference of the scene to which this element belongs.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgSceneElement::getParentScene( ) const
{
    return mParentScene;
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Allow the element to perform any necessary updates during the scene's
/// update process.
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneElement::update( cgFloat timeDelta )
{
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the element to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgSceneElement::sandboxRender( cgUInt32 flags, cgCameraNode * camera )
{
}