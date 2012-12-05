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
// Name : cgKinematicControllerJointObject.h                                 //
//                                                                           //
// Desc : Class implementing a simple kinematic controller joint as a scene  //
//        object. This joint can be used to anchor a body to a specific      //
//        point in space. This anchor point can then be manipulated          //
//        dynamically at runtime resulting in the body following.            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGKINEMATICCONTROLLERJOINTOBJECT_H_ )
#define _CGE_CGKINEMATICCONTROLLERJOINTOBJECT_H_

//-----------------------------------------------------------------------------
// cgKinematicControllerJointObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldQuery.h>
#include <World/Objects/cgJointObject.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {936065F4-EC6E-4C06-80CE-8CA4C2FDF474}
const cgUID RTID_KinematicControllerJointNode   = {0x936065F4, 0xEC6E, 0x4C06, {0x80, 0xCE, 0x8C, 0xA4, 0xC2, 0xFD, 0xF4, 0x74}};
// {2A7019DE-C66C-4FD5-9985-07CFC28B5750}
const cgUID RTID_KinematicControllerJointObject = {0x2A7019DE, 0xC66C, 0x4FD5, {0x99, 0x85, 0x7, 0xCF, 0xC2, 0x8B, 0x57, 0x50}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgKinematicControllerJointObject (Class)
/// <summary>
/// Class implementing a simple kinematic controller joint as a scene object. 
/// This joint can be used to anchor a body to a specific point in space. This 
/// anchor point can then be manipulated dynamically at runtime resulting in 
/// the body following.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgKinematicControllerJointObject : public cgJointObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgKinematicControllerJointObject, cgJointObject, "KinematicControllerJointObject" )

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum ConstraintMode
    {
        SinglePoint = 0,
        PreserveOrientation
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgKinematicControllerJointObject( cgUInt32 referenceId, cgWorld * world );
             cgKinematicControllerJointObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgKinematicControllerJointObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setConstraintMode       ( ConstraintMode mode );
    void                        setMaxAngularFriction   ( cgFloat friction );
    void                        setMaxLinearFriction    ( cgFloat friction );
    ConstraintMode              getConstraintMode       ( ) const;
    cgFloat                     getMaxAngularFriction   ( ) const;
    cgFloat                     getMaxLinearFriction    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual void                applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_KinematicControllerJointObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgFloat         mMaxLinearFriction;     // Maximum linear friction to be considered.
    cgFloat         mMaxAngularFriction;    // Maximum angular friction to be considered.
    ConstraintMode  mConstraintMode;        // The mode in which the constraint is to be applied.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertJoint;
    static cgWorldQuery     mUpdateFrictions;
    static cgWorldQuery     mUpdateConstraintMode;
    static cgWorldQuery     mLoadJoint;
};

//-----------------------------------------------------------------------------
//  Name : cgKinematicControllerJointNode (Class)
/// <summary>
/// Custom node type for the kinematic controller joint object. Manages 
/// additional properties that may need to be overriden by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgKinematicControllerJointNode : public cgJointNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgKinematicControllerJointNode, cgJointNode, "KinematicControllerJointNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgKinematicControllerJointNode( cgUInt32 referenceId, cgScene * scene );
             cgKinematicControllerJointNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgKinematicControllerJointNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setConstraintMode( cgKinematicControllerJointObject::ConstraintMode Mode )
    {
        ((cgKinematicControllerJointObject*)mReferencedObject)->setConstraintMode( Mode );
    }
    inline void setMaxLinearFriction( cgFloat fFriction )
    {
        ((cgKinematicControllerJointObject*)mReferencedObject)->setMaxLinearFriction( fFriction );
    }
    inline void setMaxAngularFriction( cgFloat fFriction )
    {
        ((cgKinematicControllerJointObject*)mReferencedObject)->setMaxAngularFriction( fFriction );
    }
    
    // Object Property 'Get' Routing
    inline cgKinematicControllerJointObject::ConstraintMode getConstraintMode( ) const
    {
        return ((cgKinematicControllerJointObject*)mReferencedObject)->getConstraintMode( );
    }
    inline cgFloat getMaxLinearFriction( ) const
    {
        return ((cgKinematicControllerJointObject*)mReferencedObject)->getMaxLinearFriction( );
    }
    inline cgFloat getMaxAngularFriction( ) const
    {
        return ((cgKinematicControllerJointObject*)mReferencedObject)->getMaxAngularFriction( );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_KinematicControllerJointNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
};

#endif // !_CGE_CGKINEMATICCONTROLLERJOINTOBJECT_H_