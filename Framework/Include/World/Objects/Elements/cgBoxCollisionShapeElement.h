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
// Name : cgBoxCollisionShapeElement.h                                       //
//                                                                           //
// Desc : Class that provides a box shaped collision primitive exposed as an //
//        object sub-element. This provides the integration between the      //
//        application (such as the editing environment) and the relevant     //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBOXCOLLISIONSHAPEELEMENT_H_ )
#define _CGE_CGBOXCOLLISIONSHAPEELEMENT_H_

//-----------------------------------------------------------------------------
// cgBoxCollisionShapeElement Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgCollisionShapeElement.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {71B3555F-8440-4775-A768-8F46730384D6}
const cgUID RTID_BoxCollisionShapeElement = {0x71B3555F, 0x8440, 0x4775, {0xA7, 0x68, 0x8F, 0x46, 0x73, 0x3, 0x84, 0xD6}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBoxCollisionShapeElement (Class)
/// <summary>
/// Class that provides a box shaped collision primitive exposed as an object 
/// sub-element. This provides the integration between the application (such as
/// the editing environment) and the relevant sub-component of the selected 
/// object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBoxCollisionShapeElement : public cgCollisionShapeElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBoxCollisionShapeElement, cgCollisionShapeElement, "BoxCollisionShapeElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBoxCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject );
             cgBoxCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );
    virtual ~cgBoxCollisionShapeElement( );

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
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_BoxCollisionShapeElement; }
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
    static cgWorldQuery     mInsertBoxCollisionShape;
    static cgWorldQuery     mLoadBoxCollisionShape;
};

#endif // !_CGE_CGBOXCOLLISIONSHAPEELEMENT_H_