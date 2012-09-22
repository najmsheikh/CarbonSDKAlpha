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
// Name : cgCollisionShapeElement.h                                          //
//                                                                           //
// Desc : Class that provides a collision shape primitive exposed as an      //
//        object sub-element. This provides the integration between the      //
//        application (such as the editing environment) and the relevant     //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCOLLISIONSHAPEELEMENT_H_ )
#define _CGE_CGCOLLISIONSHAPEELEMENT_H_

//-----------------------------------------------------------------------------
// cgCollisionShapeElement Header Includes
//-----------------------------------------------------------------------------
#include <World/cgObjectSubElement.h>
#include <Math/cgBoundingBox.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsShape;
class cgPhysicsWorld;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {A7F8231B-BDCB-4954-B0D8-5FC206081B0E}
const cgUID RTID_CollisionShapeElement = {0xA7F8231B, 0xBDCB, 0x4954, {0xB0, 0xD8, 0x5F, 0xC2, 0x6, 0x8, 0x1B, 0xE}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCollisionShapeElement (Class)
/// <summary>
/// Class that provides a collision shape primitive exposed as an object 
/// sub-element. This provides the integration between the application (such as
/// the editing environment) and the relevant sub-component of the selected 
/// object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCollisionShapeElement : public cgObjectSubElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgCollisionShapeElement, cgObjectSubElement, "CollisionShapeElement" )

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum AutoFitType
    {
        CurrentOrientation = 0,
        XAxis,
        YAxis,
        ZAxis
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject );
             cgCollisionShapeElement( cgUInt32 referenceId, cgWorldObject * parentObject, cgObjectSubElement * init );
    virtual ~cgCollisionShapeElement( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgBoundingBox       & getShapeBoundingBox     ( ) const;
    const cgTransform         & getTransform            ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual void                setTransform            ( const cgTransform & transform );
    virtual void                autoFit                 ( AutoFitType type ) = 0;
    virtual cgPhysicsShape    * generatePhysicsShape    ( cgPhysicsWorld * world ) = 0;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectSubElement)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getElementCategory      ( ) const;
    virtual void                applyElementRescale     ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual bool                createTypeTables        ( const cgUID & typeIdentifier );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_CollisionShapeElement; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

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
    cgBoundingBox               getAutoFitBounds        ( );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                createSandboxMesh       ( ) = 0;
    virtual void                computeShapeTransform   ( cgObjectNode * issuer, cgTransform & t ) = 0;
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBoundingBox           mBounds;        // Bounding box representing the overall object space dimensions of this shape.
    cgTransform             mTransform;     // Parent relative offset / transform matrix for this collision shape.

    // Sandbox Integration
    cgMeshHandle            mSandboxMesh;   // Custom mesh object used to represent the collision shape during sandbox rendering.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertBaseCollisionShape;
    static cgWorldQuery     mUpdateTransformAndBounds;
    static cgWorldQuery     mLoadBaseCollisionShape;
};

#endif // !_CGE_CGBOXCOLLISIONSHAPEELEMENT_H_