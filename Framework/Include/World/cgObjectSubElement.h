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
// Name : cgObjectSubElement.h                                               //
//                                                                           //
// Desc : Base class from which all object sub-element types derive. A       //
//        sub-element represents any editable property of an object which    //
//        might include (for instance) faces of a mesh, collision shapes for //
//        a rigid body object, etc. It essentially gives each object type    //
//        the ability to expose parts of itself to the engine / editor such  //
//        that it can be manipulated.                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGOBJECTSUBELEMENT_H_ )
#define _CGE_CGOBJECTSUBELEMENT_H_

//-----------------------------------------------------------------------------
// cgObjectSubElement Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldComponent.h>
#include <World/cgWorldTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgCameraNode;
class cgVisibilitySet;
class cgObjectNode;
class cgPlane;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {8ABED30D-422F-45AE-93DF-EF6D3877F151}
const cgUID RTID_ObjectSubElement = {0x8ABED30D, 0x422F, 0x45AE, {0x93, 0xDF, 0xEF, 0x6D, 0x38, 0x77, 0xF1, 0x51}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgObjectSubElement (Class)
/// <summary>
/// Base class from which all object sub-element types derive. A sub-element 
/// represents any editable property of an object which might include (for 
/// instance) faces of a mesh, collision shapes for a rigid body object, etc. 
/// It essentially gives each object type the ability to expose parts of itself
/// to the engine / editor such  that it can be manipulated.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgObjectSubElement : public cgWorldComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgObjectSubElement, cgWorldComponent, "ObjectSubElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgObjectSubElement( cgUInt32 referenceId, cgWorldObject * parentObject );
             cgObjectSubElement( cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );
    virtual ~cgObjectSubElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static const cgObjectSubElementTypeDesc::Map  & getRegisteredTypes  ( );
    static void                                     registerType        ( const cgUID & globalIdentifier, const cgString & name, 
                                                                          cgObjectSubElementTypeDesc::AllocNewFunc objectSubElementAllocNew,
                                                                          cgObjectSubElementTypeDesc::AllocCloneFunc objectSubElementAllocClone );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgWorldObject             * getParentObject         ( ) const;
    bool                        clone                   ( cgWorldObject * object, bool internal, cgObjectSubElement *& objectSubElementOut );
    void                        setSelected             ( bool selected );
    bool                        isSelected              ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual cgString            getDisplayName          ( ) const;
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual const cgUID       & getElementCategory      ( ) const = 0;
    virtual void                applyElementRescale     ( cgFloat fScale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ObjectSubElement; }
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
    // Sandbox Integration
    bool                mSelected;      // Is this sub element selected for editing?
    cgWorldObject     * mParentObject;  // Parent object to which this sub-element belongs.

private:
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgObjectSubElementTypeDesc::Map  mRegisteredObjectSubElementTypes;  // All of the object sub-element types registered with the system.
};

#endif // !_CGE_CGOBJECTSUBELEMENT_H_