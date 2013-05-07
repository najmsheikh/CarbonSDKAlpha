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
// Name : cgHullCollisionShapeElement.h                                      //
//                                                                           //
// Desc : Class that provides an automatically generated convex hull mesh    //
//        available for use as a collision primitive and exposed as an       //
//        object sub-element. This provides the integration between the      //
//        application (such as the editing environment) and the relevant     //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGHULLCOLLISIONSHAPEELEMENT_H_ )
#define _CGE_CGHULLCOLLISIONSHAPEELEMENT_H_

//-----------------------------------------------------------------------------
// cgHullCollisionShapeElement Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgCollisionShapeElement.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {3D5C75AC-6993-415E-B898-5E1F23885B61}
const cgUID RTID_HullCollisionShapeElement = {0x3D5C75AC, 0x6993, 0x415E, {0xB8, 0x98, 0x5E, 0x1F, 0x23, 0x88, 0x5B, 0x61}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgHullCollisionShapeElement (Class)
/// <summary>
/// Class that provides an automatically generated convex hull mesh available 
/// for use as a collision primitive and exposed as an object sub-element. This
/// provides the integration between the application (such as the editing 
/// environment) and the relevant sub-component of the selected object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgHullCollisionShapeElement : public cgCollisionShapeElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgHullCollisionShapeElement, cgCollisionShapeElement, "HullCollisionShapeElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgHullCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject );
             cgHullCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );
    virtual ~cgHullCollisionShapeElement( );

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
    virtual void                setTransform            ( const cgTransform & t );
    virtual cgPhysicsShape    * generatePhysicsShape    ( cgPhysicsWorld * world );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_HullCollisionShapeElement; }
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
    bool                        insertComponentData     ( void * pSerializedData, cgUInt32 nDataSize );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgCollisionShape)
    //-------------------------------------------------------------------------
    virtual bool                createSandboxMesh       ( );
    virtual void                computeShapeTransform   ( cgObjectNode * issuer, cgTransform & t );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32    mSerializedDataSource;      // Is there serialized data available for this shape, and if so where?
    cgFloat     mCollapseTolerance;         // Tolerance value used to describe how the point cloud should be collapsed / simplified.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertHullCollisionShape;
    static cgWorldQuery mUpdateSerializedData;
    static cgWorldQuery mLoadHullCollisionShape;
};

#endif // !_CGE_CGHULLCOLLISIONSHAPEELEMENT_H_