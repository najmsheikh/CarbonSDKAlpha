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
// Name : cgCapsuleCollisionShapeElement.h                                   //
//                                                                           //
// Desc : Class that provides a capsule shaped collision primitive exposed   //
//        as an object sub-element. This provides the integration between    //
//        the application (such as the editing environment) and the relevant //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCAPSULECOLLISIONSHAPEELEMENT_H_ )
#define _CGE_CGCAPSULECOLLISIONSHAPEELEMENT_H_

//-----------------------------------------------------------------------------
// cgCapsuleCollisionShapeElement Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgCollisionShapeElement.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {12E48001-F868-4318-A9D6-E03F6B2656CC}
const cgUID RTID_CapsuleCollisionShapeElement = {0x12E48001, 0xF868, 0x4318, {0xA9, 0xD6, 0xE0, 0x3F, 0x6B, 0x26, 0x56, 0xCC}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCapsuleCollisionShapeElement (Class)
/// <summary>
/// Class that provides a capsule shaped collision primitive exposed as an
/// object sub-element. This provides the integration between the application
/// (such as the editing environment) and the relevant sub-component of the
/// selected object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCapsuleCollisionShapeElement : public cgCollisionShapeElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgCapsuleCollisionShapeElement, cgCollisionShapeElement, "CapsuleCollisionShapeElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCapsuleCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject );
             cgCapsuleCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );
    virtual ~cgCapsuleCollisionShapeElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectSubElement * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorldObject * parentObject );
    static cgObjectSubElement * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setDimensions           ( cgFloat radius, cgFloat height );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgCollisionShapeElement)
    //-------------------------------------------------------------------------
    virtual void                autoFit                 ( AutoFitType type );
    virtual void                setTransform            ( const cgTransform & transform );
    virtual cgPhysicsShape    * generatePhysicsShape    ( cgPhysicsWorld * world );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectSubElement)
    //-------------------------------------------------------------------------
    virtual void                applyElementRescale     ( cgFloat scale );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_CapsuleCollisionShapeElement; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgCollisionShape)
    //-------------------------------------------------------------------------
    virtual bool                createSandboxMesh       ( );
    virtual void                computeShapeTransform   ( cgObjectNode * issuer, cgTransform & t );

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertCapsuleCollisionShape;
    static cgWorldQuery     mLoadCapsuleCollisionShape;
};

#endif // !_CGE_CGCAPSULECOLLISIONSHAPEELEMENT_H_