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
// Name : cgDummyObject.h                                                    //
//                                                                           //
// Desc : Contains simple classes responsible for providing dummy hierarchy  //
//        frames. These can be useful for providing alternative              //
//        transformation points of reference for animation (i.e. spinning a  //
//        wheel on more than one axis).                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDUMMYOBJECT_H_ )
#define _CGE_CGDUMMYOBJECT_H_

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
// {2D239346-1E8A-4F3E-A7D9-7703985A8F6C}
const cgUID RTID_DummyNode   = { 0x2D239346, 0x1E8A, 0x4F3E, {0xA7, 0xD9, 0x77, 0x03, 0x98, 0x5A, 0x8F, 0x6C } };
// {4B6669F2-8213-4ED6-BA4F-651C250A959C}
const cgUID RTID_DummyObject = { 0x4B6669F2, 0x8213, 0x4ED6, { 0xBA, 0x4F, 0x65, 0x1C, 0x25, 0x0A, 0x95, 0x9C } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDummyObject (Class)
/// <summary>
/// A simple, empty dummy object. Usually used to provide additional parent 
/// coordinate system frames of reference for animation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDummyObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDummyObject, cgWorldObject, "DummyObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDummyObject( cgUInt32 referenceId, cgWorld * world );
             cgDummyObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgDummyObject( );

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
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
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
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_DummyObject; }
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
//  Name : cgDummyNode (Class)
/// <summary>
/// Custom node type for the dummy object. Manages additional properties
/// that may need to be override by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDummyNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDummyNode, cgObjectNode, "DummyNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDummyNode( cgUInt32 referenceId, cgScene * scene );
             cgDummyNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgDummyNode( );

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
        ((cgDummyObject*)mReferencedObject)->setSize( size );
    }

    // Object Property 'Get' Routing
    inline cgFloat getSize( ) const
    {
        return ((cgDummyObject*)mReferencedObject)->getSize( );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_DummyNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
};

#endif // !_CGE_CGDUMMYOBJECT_H_