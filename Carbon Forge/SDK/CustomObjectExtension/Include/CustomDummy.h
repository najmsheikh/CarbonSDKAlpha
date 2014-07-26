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
// Name : CustomDummy.h                                                      //
//                                                                           //
// Desc : Contains simple classes responsible for providing dummy hierarchy  //
//        frames. These can be useful for providing alternative              //
//        transformation points of reference for animation (i.e. spinning a  //
//        wheel on more than one axis).                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CUSTOMDUMMYOBJECT_H_ )
#define _CUSTOMDUMMYOBJECT_H_

//-----------------------------------------------------------------------------
// cgDummyObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5360DAA4-2846-4A60-8E0E-42509C9883D5}
const cgUID RTID_CustomDummyNode   = { 0x5360DAA4, 0x2846, 0x4A60, { 0x8E, 0x0E, 0x42, 0x50, 0x9C, 0x98, 0x83, 0xD5 } };
// {7CFB4CFE-4094-488b-80D5-6C03740002D2}
const cgUID RTID_CustomDummyObject = { 0x7CFB4CFE, 0x4094, 0x488B, { 0x80, 0xD5, 0x6C, 0x03, 0x74, 0x00, 0x02, 0xD2 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : CustomDummyObject (Class)
/// <summary>
/// A simple, empty dummy object. Usually used to provide additional parent 
/// coordinate system frames of reference for animation.
/// </summary>
//-----------------------------------------------------------------------------
class CustomDummyObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( CustomDummyObject, cgWorldObject, "CustomDummyObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             CustomDummyObject( cgUInt32 referenceId, cgWorld * world );
             CustomDummyObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~CustomDummyObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgFloat                     getSize                 ( ) const;
    void                        setSize                 ( cgFloat size );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual cgBoundingBox       getLocalBoundingBox     ( );
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
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_CustomDummyObject; }
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
    cgFloat mSize;    // Size of the dummy box (pre-scale) in meters (fixed units).

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertDummy;
    static cgWorldQuery     mUpdateSize;
    static cgWorldQuery     mLoadDummy;
};

//-----------------------------------------------------------------------------
//  Name : CustomDummyNode (Class)
/// <summary>
/// Custom node type for the dummy object. Manages additional properties
/// that may need to be override by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CustomDummyNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( CustomDummyNode, cgObjectNode, "CustomDummyNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             CustomDummyNode( cgUInt32 referenceId, cgScene * scene );
             CustomDummyNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~CustomDummyNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setSize( cgFloat size )
    {
        ((CustomDummyObject*)mReferencedObject)->setSize( size );
    }

    // Object Property 'Get' Routing
    inline cgFloat getSize( ) const
    {
        return ((CustomDummyObject*)mReferencedObject)->getSize( );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_CustomDummyNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
};

#endif // !_CUSTOMDUMMYOBJECT_H_