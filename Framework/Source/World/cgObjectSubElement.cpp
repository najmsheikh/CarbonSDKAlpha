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
// Name : cgObjectSubElement.cpp                                             //
//                                                                           //
// Desc : Base class from which all object sub-element types derive. A       //
//        sub-element represents any editable property of an object which    //
//        might include (for instance) faces of a mesh, collision shapes for //
//        a rigid body object, etc. It essentially gives each object type    //
//        the ability to expose parts of itself to the engine / editor such  //
//        that it can be manipulated.                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgObjectSubElement Module Includes
//-----------------------------------------------------------------------------
#include <World/cgObjectSubElement.h>
#include <World/cgWorldObject.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgObjectSubElementTypeDesc::Map cgObjectSubElement::mRegisteredObjectSubElementTypes;

///////////////////////////////////////////////////////////////////////////////
// cgObjectSubElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgObjectSubElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement::cgObjectSubElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject ) : cgWorldComponent( nReferenceId, pParentObject->getParentWorld() )
{
    // Initialize variables to sensible defaults
    mParentObject = pParentObject;
    mSelected     = false;
}

//-----------------------------------------------------------------------------
//  Name : cgObjectSubElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement::cgObjectSubElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject, cgObjectSubElement * pInit ) : cgWorldComponent( nReferenceId, pParentObject->getParentWorld(), pInit )
{
    // Initialize variables to sensible defaults
    mParentObject = pParentObject;
    mSelected     = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgObjectSubElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement::~cgObjectSubElement()
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
void cgObjectSubElement::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    
    // Dispose base class if requested.
    if ( bDisposeBase == true )
        cgWorldComponent::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : registerType() (Static)
/// <summary>
/// Allows the application to register all of the various object sub-element
/// types that are supported by the application. These types can then be 
/// instantiated and / or referenced directly within the world database.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectSubElement::registerType( const cgUID & GlobalIdentifier, const cgString & strName, 
                                       cgObjectSubElementTypeDesc::AllocNewFunc pObjectSubElementAllocNew, 
                                       cgObjectSubElementTypeDesc::AllocCloneFunc pObjectSubElementAllocClone )
{
    // Build the descriptor
    cgObjectSubElementTypeDesc Desc;
    Desc.globalIdentifier   = GlobalIdentifier;
    Desc.localIdentifier    = 0; // 0 = Unassigned. This will be filled out in the local duplicate.
    Desc.name               = strName;
    Desc.elementAllocNew    = pObjectSubElementAllocNew;
    Desc.elementAllocClone  = pObjectSubElementAllocClone;
    
    // Store
    mRegisteredObjectSubElementTypes[ GlobalIdentifier ] = Desc;
}

//-----------------------------------------------------------------------------
//  Name : getRegisteredTypes() (Static)
/// <summary>
/// Retrieve the map containing descriptions of all registered object sub-
/// element types that are supported by the application. 
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectSubElementTypeDesc::Map & cgObjectSubElement::getRegisteredTypes( )
{
    return mRegisteredObjectSubElementTypes;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectSubElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ObjectSubElement )
        return true;

    // Supported by base?
    return cgWorldComponent::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the sub element to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment assuming
/// element editing for this type is active.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectSubElement::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : applyElementRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this element. For instance,
/// in the case of a collision shape, its dimensions will be scaled.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectSubElement::applyElementRescale( cgFloat fScale )
{
    // Nothing in base implementation
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgObjectSubElement::getDatabaseTable( ) const
{
    // Sub-elements are not required to serialize anything
    // but can if they wish.
    return cgString::Empty;
}

//-----------------------------------------------------------------------------
//  Name : isSelected () (Virtual)
/// <summary>
/// Is the shape currently selected for manipulation?
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectSubElement::isSelected( ) const
{
    return mSelected;
}

//-----------------------------------------------------------------------------
//  Name : setSelected () (Virtual)
/// <summary>
/// Set the shape's selection status.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectSubElement::setSelected( bool bSelected )
{
    // Update selected flag
    mSelected = bSelected;
}

//-----------------------------------------------------------------------------
//  Name : getParentObject ()
/// <summary>
/// Retrieve a reference to the object to which this sub-element belongs.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgObjectSubElement::getParentObject( ) const
{
    return mParentObject;
}

//-----------------------------------------------------------------------------
// Name : clone ()
/// <summary>
/// Make a duplicate of this object sub-element.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectSubElement::clone( cgWorldObject * pObject, bool bInternal, cgObjectSubElement *& pObjectSubElementOut )
{
    // Be polite and clear output variables
    pObjectSubElementOut = CG_NULL;

    // Clones are always 'internal' when not in full sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        bInternal = true;

    // Clone the sub-element
    cgWorld * pWorld = pObject->getParentWorld();
    pObjectSubElementOut = pWorld->createObjectSubElement( bInternal, getReferenceType(), pObject, this );

    // Success?
    return (pObjectSubElementOut != CG_NULL);
}

//-----------------------------------------------------------------------------
// Name : getDisplayName ()
/// <summary>
/// Get the name that will be displayed in the sub element list for this
/// particular object sub element. By default this will simply return 
/// "<Unnamed>", but derived classes can override this behavior when required.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgObjectSubElement::getDisplayName( ) const
{
    return _T("<Unnamed>");
}