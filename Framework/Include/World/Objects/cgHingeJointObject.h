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
// Name : cgHingeJointObject.h                                               //
//                                                                           //
// Desc : Class implementing a single axis hinge joint. This joint can be    //
//        used to connect two bodies around a central axis and allow them    //
//        to rotate freely with optional angle constraints.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGHINGEJOINTOBJECT_H_ )
#define _CGE_CGHINGEJOINTOBJECT_H_

//-----------------------------------------------------------------------------
// cgHingeJointObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldQuery.h>
#include <World/Objects/cgJointObject.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgHingeJoint;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {ABABB216-2296-482B-9A80-A8B34B6499A7}
const cgUID RTID_HingeJointNode   = {0xABABB216, 0x2296, 0x482B, {0x9A, 0x80, 0xA8, 0xB3, 0x4B, 0x64, 0x99, 0xA7}};
// {908EFCA6-73CB-48DA-862F-B230369FD9A5}
const cgUID RTID_HingeJointObject = {0x908EFCA6, 0x73CB, 0x48DA, {0x86, 0x2F, 0xB2, 0x30, 0x36, 0x9F, 0xD9, 0xA5}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgHingeJointObject (Class)
/// <summary>
/// Class implementing a single axis hinge joint. This joint can be used to 
/// connect two bodies around a central axis and allow them to rotate freely 
/// with optional angle constraints.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgHingeJointObject : public cgJointObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgHingeJointObject, cgJointObject, "HingeJointObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgHingeJointObject( cgUInt32 referenceId, cgWorld * world );
             cgHingeJointObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgHingeJointObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        enableLimits                ( bool enable );
    void                        setLimits                   ( cgFloat minDegrees, cgFloat maxDegrees );
    void                        setLimits                   ( const cgRangeF & range );
    bool                        isLimited                   ( ) const;
    cgRangeF                    getLimits                   ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distanceOut );
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
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_HingeJointObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );
    bool                        createSandboxMesh       ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool            mUseLimits;
    cgFloat         mMinimumAngle;      // Degrees
    cgFloat         mMaximumAngle;      // Degrees
    cgFloat         mInitialAngle;      // Degrees
    cgMeshHandle    mSandboxMesh;       // Representation of this joint for sandbox rendering.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertJoint;
    static cgWorldQuery     mLoadJoint;
    static cgWorldQuery     mUpdateLimits;
};

//-----------------------------------------------------------------------------
//  Name : cgHingeJointNode (Class)
/// <summary>
/// Custom node type for the hinge joint object. Manages additional 
/// properties that may need to be overridden by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgHingeJointNode : public cgJointNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgHingeJointNode, cgJointNode, "HingeJointNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgHingeJointNode( cgUInt32 referenceId, cgScene * scene );
             cgHingeJointNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgHingeJointNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setBody0                ( cgUInt32 nodeReferenceId );
    void                        setBody1                ( cgUInt32 nodeReferenceId );
    cgUInt32                    getBody0                ( ) const;
    cgUInt32                    getBody1                ( ) const;
    cgObjectNode              * getBody0Node            ( ) const;
    cgObjectNode              * getBody1Node            ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_HingeJointNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                getSandboxIconInfo      ( cgCameraNode * camera, const cgSize & viewportSize, cgString & strAtlas, cgString & strFrame, cgVector3 & vIconOrigin );
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual bool                setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );
    virtual bool                onNodeCreated           ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeLoading           ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeInit              ( const cgUInt32IndexMap & nodeReferenceRemap );
    virtual bool                onNodeDeleted           ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsBodyEventListener)
    //-------------------------------------------------------------------------
    virtual void                onPhysicsBodyTransformed( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Get' Routing
    inline bool isLimited( ) const
    {
        cgAssert(mReferencedObject);
        return ((cgHingeJointObject*)mReferencedObject)->isLimited( );
    }
    inline cgRangeF getLimits( ) const
    {
        cgAssert(mReferencedObject);
        return ((cgHingeJointObject*)mReferencedObject)->getLimits( );
    }
    
    // Object Property 'Set' Routing
    inline void enableLimits( bool enable )
    {
        cgAssert(mReferencedObject);
        ((cgHingeJointObject*)mReferencedObject)->enableLimits( enable );
    }
    inline void setLimits( cgFloat minDegrees, cgFloat maxDegrees )
    {
        cgAssert(mReferencedObject);
        ((cgHingeJointObject*)mReferencedObject)->setLimits( minDegrees, maxDegrees );
    }
    inline void setLimits( const cgRangeF & range )
    {
        cgAssert(mReferencedObject);
        ((cgHingeJointObject*)mReferencedObject)->setLimits( range );
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        rebuildJoint            ( );
    void                        prepareQueries          ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgHingeJoint  * mJoint;             // The actual physics joint.
    cgUInt32        mBody0RefId;        // Reference ID of the first joint body.
    cgUInt32        mBody1RefId;        // Reference ID of the second joint body.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertInstanceData;
    static cgWorldQuery mDeleteInstanceData;
    static cgWorldQuery mUpdateInstanceData;
    static cgWorldQuery mLoadInstanceData;
};

#endif // !_CGE_CGHINGEJOINTOBJECT_H_