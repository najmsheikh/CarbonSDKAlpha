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
// Name : cgAnimationTarget.h                                                //
//                                                                           //
// Desc : Base class from which any 'entity' in the system should derive     //
//        if it wishes to be the target for any animation data being applied //
//        by an active animation controller.                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGANIMATIONTARGET_H_ )
#define _CGE_CGANIMATIONTARGET_H_

//-----------------------------------------------------------------------------
// cgAnimationTarget Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgReference.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {6F31CFFC-5877-41D9-ACE0-AFD6B26C12C7}
const cgUID RTID_AnimationTarget = {0x6F31CFFC, 0x5877, 0x41D9, {0xAC, 0xE0, 0xAF, 0xD6, 0xB2, 0x6C, 0x12, 0xC7}};

//-----------------------------------------------------------------------------
// Forward Declaration
//-----------------------------------------------------------------------------
class cgTransform;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAnimationTarget (Class)
/// <summary>
/// Base class from which other classes within the engine or
/// application can derive in order to integrate with the animation
/// system.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAnimationTarget : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAnimationTarget, cgReference, "AnimationTarget" )

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgAnimationTarget( cgUInt32 referenceId );
             cgAnimationTarget( cgUInt32 referenceId, cgAnimationTarget * init );
	virtual ~cgAnimationTarget( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgString        & getInstanceIdentifier       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType            ( ) const { return RTID_AnimationTarget; }
    virtual bool            queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                     ( bool disposeBase );

    //-------------------------------------------------------------------------
	// Public Virtual Methods
	//-------------------------------------------------------------------------
    virtual void            setInstanceIdentifier       ( const cgString & identifier );
    virtual void            onAnimationTransformUpdated ( const cgTransform & t ) = 0;

protected:
    //-------------------------------------------------------------------------
	// Protected Variables
	//-------------------------------------------------------------------------
    cgString    mInstanceIdentifier;
};

#endif // !_CGE_CGANIMATIONTARGET_H_