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
// Name : cgConeCollisionShapeElement.h                                      //
//                                                                           //
// Desc : Class that provides a cone shaped collision primitive exposed      //
//        as an object sub-element. This provides the integration between    //
//        the application (such as the editing environment) and the relevant //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCONECOLLISIONSHAPEELEMENT_H_ )
#define _CGE_CGCONECOLLISIONSHAPEELEMENT_H_

//-----------------------------------------------------------------------------
// cgConeCollisionShapeElement Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgCollisionShapeElement.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {69C9A159-4F76-4973-B30E-408C9FFCE3B2}
const cgUID RTID_ConeCollisionShapeElement = {0x69C9A159, 0x4F76, 0x4973, {0xB3, 0xE, 0x40, 0x8C, 0x9F, 0xFC, 0xE3, 0xB2}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgConeCollisionShapeElement (Class)
/// <summary>
/// Class that provides a cone shaped collision primitive exposed as an
/// object sub-element. This provides the integration between the application
/// (such as the editing environment) and the relevant sub-component of the
/// selected object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgConeCollisionShapeElement : public cgCollisionShapeElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgConeCollisionShapeElement, cgCollisionShapeElement, "ConeCollisionShapeElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgConeCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject );
             cgConeCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );
    virtual ~cgConeCollisionShapeElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectSubElement * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorldObject * parentObject );
    static cgObjectSubElement * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgCollisionShape)
    //-------------------------------------------------------------------------
    virtual void                autoFit                 ( AutoFitType type );
    virtual cgPhysicsShape    * generatePhysicsShape    ( cgPhysicsWorld * world );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ConeCollisionShapeElement; }
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
    static cgWorldQuery     mInsertConeCollisionShape;
    static cgWorldQuery     mLoadConeCollisionShape;
};

#endif // !_CGE_CGCONECOLLISIONSHAPEELEMENT_H_