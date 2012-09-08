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
// Name : cgAnimationTarget.cpp                                              //
//                                                                           //
// Desc : Base class from which any 'entity' in the system should derive     //
//        if it wishes to be the target for any animation data being applied //
//        by an active animation controller.                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAnimationController Module Includes
//-----------------------------------------------------------------------------
#include <Animation/cgAnimationTarget.h>
#include <System/cgExceptions.h>

///////////////////////////////////////////////////////////////////////////////
// cgAnimationTarget Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAnimationTarget () (Constructor)
/// <summary>
/// cgAnimationTarget Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationTarget::cgAnimationTarget( cgUInt32 referenceId ) : cgReference( referenceId )
{
}

//-----------------------------------------------------------------------------
//  Name : cgAnimationTarget () (Constructor)
/// <summary>
/// cgAnimationTarget Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationTarget::cgAnimationTarget( cgUInt32 referenceId, cgAnimationTarget * init ) : cgReference( referenceId )
{
    // Ensure this is a valid operation
    if ( !init || !init->queryReferenceType( this->getReferenceType() ) )
        throw cgExceptions::ResultException( _T("Unable to clone. Specified reference is of incompatible type."), cgDebugSource() );

    // Clone data.
    mInstanceIdentifier = init->mInstanceIdentifier;
}

//-----------------------------------------------------------------------------
//  Name : ~cgAnimationTarget () (Destructor)
/// <summary>
/// cgAnimationTarget Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationTarget::~cgAnimationTarget()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgAnimationTarget::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose of base classes
    if ( bDisposeBase == true )
        cgReference::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationTarget::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AnimationTarget )
        return true;

    // Unsupported
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getInstanceIdentifier ()
/// <summary>
/// Retrieve the string that allows the animation and other systems to identify
/// this animation target, and instances of it.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgAnimationTarget::getInstanceIdentifier( ) const
{
    return mInstanceIdentifier;
}

//-----------------------------------------------------------------------------
//  Name : setInstanceIdentifier ()
/// <summary>
/// Set the string that allows the animation and other systems to identify
/// this animation target, and instances of it.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationTarget::setInstanceIdentifier( const cgString & strIdentifier )
{
    mInstanceIdentifier = strIdentifier;
}