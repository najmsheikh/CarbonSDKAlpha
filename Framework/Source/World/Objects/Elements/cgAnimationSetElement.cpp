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
// Name : cgAnimationSetElement.cpp                                          //
//                                                                           //
// Desc : Class that provides an object assigned animation set exposed as    //
//        an object sub-element. This provides the integration between the   //
//        application (such as the editing environment) and the relevant     //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAnimationSetElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgAnimationSetElement.h>
#include <Resources/cgAnimationSet.h>
/*#include <World/cgObjectNode.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgMesh.h>*/

///////////////////////////////////////////////////////////////////////////////
// cgAnimationSetElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAnimationSetElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSetElement::cgAnimationSetElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject ) : cgObjectSubElement( nReferenceId, pParentObject )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgAnimationSetElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSetElement::cgAnimationSetElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject, cgObjectSubElement * pInit ) : cgObjectSubElement( nReferenceId, pParentObject, pInit )
{
    // Initialize variables to sensible defaults
    cgAnimationSetElement * pElement = (cgAnimationSetElement*)pInit;
    mAnimationSet = pElement->mAnimationSet;
}

//-----------------------------------------------------------------------------
//  Name : ~cgAnimationSetElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSetElement::~cgAnimationSetElement()
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
void cgAnimationSetElement::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    mAnimationSet.close();
    
    // Dispose base class if requested.
    if ( bDisposeBase == true )
        cgObjectSubElement::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgAnimationSetElement::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject )
{
    return new cgAnimationSetElement( nReferenceId, pObject );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type, cloned from the
/// provided element.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgAnimationSetElement::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject, cgObjectSubElement * pInit )
{
    return new cgAnimationSetElement( nReferenceId, pObject, pInit );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSetElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AnimationSetElement )
        return true;

    // Supported by base?
    return cgObjectSubElement::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getElementCategory () (Virtual)
/// <summary>
/// Retrieve the unique identifier for the sub-element category to which this
/// type belongs.
/// </summary>
//-----------------------------------------------------------------------------
const cgUID & cgAnimationSetElement::getElementCategory( ) const
{
    return OSECID_AnimationSets;
}

//-----------------------------------------------------------------------------
//  Name : setAnimationSet ()
/// <summary>
/// Supply the handle to the animation set represented by this sub-element 
/// instance.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSetElement::setAnimationSet( cgAnimationSetHandle animationSet )
{
    mAnimationSet = animationSet;
}

//-----------------------------------------------------------------------------
//  Name : getAnimationSet ()
/// <summary>
/// Get the handle to the animation set represented by this sub-element 
/// instance.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSetHandle cgAnimationSetElement::getAnimationSet( ) const
{
    return mAnimationSet;
}

//-----------------------------------------------------------------------------
// Name : getDisplayName ()
/// <summary>
/// Get the name that will be displayed in the sub element list for this
/// particular object sub element. By default this will simply return 
/// "<Unnamed>", but derived classes can override this behavior when required.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgAnimationSetElement::getDisplayName( ) const
{
    if ( !mAnimationSet.isValid() )
        return cgObjectSubElement::getDisplayName();
    const cgAnimationSet * animationSet = mAnimationSet.getResourceSilent();
    const cgString & displayName = animationSet->getName();
    if ( displayName.empty() )
        return cgString::format( _T("<Unnamed Set> (0x%x)"), animationSet->getReferenceId() );
    else
        return displayName;
}