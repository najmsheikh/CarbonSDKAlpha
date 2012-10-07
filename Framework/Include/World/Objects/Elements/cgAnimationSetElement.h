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
// Name : cgAnimationSetElement.h                                            //
//                                                                           //
// Desc : Class that provides an object assigned animation set exposed as    //
//        an object sub-element. This provides the integration between the   //
//        application (such as the editing environment) and the relevant     //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGANIMATIONSETELEMENT_H_ )
#define _CGE_CGANIMATIONSETELEMENT_H_

//-----------------------------------------------------------------------------
// cgAnimationSetElement Header Includes
//-----------------------------------------------------------------------------
#include <World/cgObjectSubElement.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {FDE03341-7209-44c9-8A73-D8B71ABEDA74}
const cgUID RTID_AnimationSetElement = { 0xFDE03341, 0x7209, 0x44C9, {0x8A, 0x73, 0xD8, 0xB7, 0x1A, 0xBE, 0xDA, 0x74}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAnimationSetElement (Class)
/// <summary>
/// Class that provides an object assigned animation set exposed as an object 
/// sub-element. This provides the integration between the application (such as
/// the editing environment) and the relevant sub-component of the selected 
/// object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAnimationSetElement : public cgObjectSubElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAnimationSetElement, cgObjectSubElement, "AnimationSetElement" )

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAnimationSetElement( cgUInt32 referenceId, cgWorldObject * parentObject );
             cgAnimationSetElement( cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );
    virtual ~cgAnimationSetElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectSubElement * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorldObject * parentObject );
    static cgObjectSubElement * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setAnimationSet         ( cgAnimationSetHandle animationSet );
    cgAnimationSetHandle        getAnimationSet         ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectSubElement)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getElementCategory      ( ) const;
    virtual cgString            getDisplayName          ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_AnimationSetElement; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgAnimationSetHandle    mAnimationSet;  // Handle to the animation set represented by this sub-element instance
};

#endif // !_CGE_CGANIMATIONSETELEMENT_H_