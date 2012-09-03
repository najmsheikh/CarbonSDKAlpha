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
// Name : cgSphereCollisionShapeElement.h                                    //
//                                                                           //
// Desc : Class that provides a sphere shaped collision primitive exposed    //
//        as an object sub-element. This provides the integration between    //
//        the application (such as the editing environment) and the relevant //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSPHERECOLLISIONSHAPEELEMENT_H_ )
#define _CGE_CGSPHERECOLLISIONSHAPEELEMENT_H_

//-----------------------------------------------------------------------------
// cgSphereCollisionShapeElement Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgCollisionShapeElement.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5E7C1343-44E9-4BB1-92E2-1B983D740DBA}
const cgUID RTID_SphereCollisionShapeElement = {0x5E7C1343, 0x44E9, 0x4BB1, {0x92, 0xE2, 0x1B, 0x98, 0x3D, 0x74, 0xD, 0xBA}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSphereCollisionShapeElement (Class)
/// <summary>
/// Class that provides a sphere shaped collision primitive exposed as an
/// object sub-element. This provides the integration between the application
/// (such as the editing environment) and the relevant sub-component of the
/// selected object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSphereCollisionShapeElement : public cgCollisionShapeElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSphereCollisionShapeElement, cgCollisionShapeElement, "SphereCollisionShapeElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSphereCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject );
             cgSphereCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );
    virtual ~cgSphereCollisionShapeElement( );
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
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SphereCollisionShapeElement; }
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
    static cgWorldQuery     mInsertSphereCollisionShape;
    static cgWorldQuery     mLoadSphereCollisionShape;
};

#endif // !_CGE_CGSPHERECOLLISIONSHAPEELEMENT_H_