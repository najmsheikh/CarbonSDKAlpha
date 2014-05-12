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
// Name : cgBillboardObject.h                                                //
//                                                                           //
// Desc : Contains classes responsible for managing a single billboard as a  //
//        scene object that can be created and manipulated both at runtime   //
//        and in advance within the editor.                                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBILLBOARDOBJECT_H_ )
#define _CGE_CGBILLBOARDOBJECT_H_

//-----------------------------------------------------------------------------
// cgBillboardObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5FBCFCBC-C77E-4440-9731-BDDFDD45BA94}
const cgUID RTID_BillboardNode   = { 0x5fbcfcbc, 0xc77e, 0x4440, { 0x97, 0x31, 0xbd, 0xdf, 0xdd, 0x45, 0xba, 0x94 } };
// {D3A52B6E-5CD9-4D75-8116-703FB400E592}
const cgUID RTID_BillboardObject = { 0xd3a52b6e, 0x5cd9, 0x4d75, { 0x81, 0x16, 0x70, 0x3f, 0xb4, 0x0, 0xe5, 0x92 } };

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBillboardBuffer;
class cgBillboard3D;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBillboardObject (Class)
/// <summary>
/// A simple entity designed to represent a single billboard as a world object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBillboardObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBillboardObject, cgWorldObject, "BillboardObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBillboardObject( cgUInt32 referenceId, cgWorld * world );
             cgBillboardObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgBillboardObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgSizeF             & getSize                 ( ) const;
    void                        setSize                 ( const cgSizeF & size );
    const cgVector2           & getScale                ( ) const;
    void                        setScale                ( const cgVector2 & scale );
    cgUInt32                    getColor                ( ) const;
    void                        setColor                ( cgUInt32 color );
    cgFloat                     getRotation             ( ) const;
    void                        setRotation             ( cgFloat degrees );
    cgFloat                     getHDRScale             ( ) const;
    void                        setHDRScale             ( cgFloat scale );
    bool                        isBillboardAligned      ( ) const;
    void                        enableBillboardAlignment( bool enable );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                applyObjectRescale      ( cgFloat scale );
    virtual bool                render                  ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer );
    virtual bool                renderSubset            ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer, const cgMaterialHandle & hMaterial );
    virtual bool                isRenderable            ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_BillboardObject; }
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
    cgSizeF     mSize;              // The base size of the billboard.
    cgVector2   mScale;             // The scaling factor to apply to the billboard
    cgUInt32    mColor;             // The color of the billboard.
    cgInt16     mFrameGroup;        // Frame group currently used for generating texture coords
    cgInt16     mFrame;             // The actual frame index within the texture for generating tex coords.
    cgFloat     mRotation;          // Rotation angle in degrees
    cgFloat     mHDRScale;          // Intensity scalar for HDR rendering.
    bool        mAlignBillboard;    // Align the billboard to the Z axis.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    //static cgWorldQuery     mInsertBillboard;
    //static cgWorldQuery     mLoadBillboard;
};

//-----------------------------------------------------------------------------
//  Name : cgBillboardNode (Class)
/// <summary>
/// Custom node type for the billboard object. Manages additional properties
/// that may need to be override by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBillboardNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBillboardNode, cgObjectNode, "BillboardNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBillboardNode( cgUInt32 referenceId, cgScene * scene );
             cgBillboardNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgBillboardNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgBillboardBuffer         * getBuffer               ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                registerVisibility      ( cgVisibilitySet * visibilityData );
    virtual bool                onNodeCreated           ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeLoading           ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );
    virtual bool                setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setSize( const cgSizeF & size )
    {
        ((cgBillboardObject*)mReferencedObject)->setSize( size );
    }
    inline void setScale( const cgVector2 & scale )
    {
        ((cgBillboardObject*)mReferencedObject)->setScale( scale );
    }
    inline void setColor( cgUInt32 color )
    {
        ((cgBillboardObject*)mReferencedObject)->setColor( color );
    }
    inline void setRotation( cgFloat degrees )
    {
        ((cgBillboardObject*)mReferencedObject)->setRotation( degrees );
    }
    inline void setHDRScale( cgFloat scale )
    {
        ((cgBillboardObject*)mReferencedObject)->setHDRScale( scale );
    }
    inline void enableBillboardAlignment( bool enable )
    {
        ((cgBillboardObject*)mReferencedObject)->enableBillboardAlignment( enable );
    }

    // Object Property 'Get' Routing
    inline const cgSizeF & getSize( ) const
    {
        return ((cgBillboardObject*)mReferencedObject)->getSize( );
    }
    inline const cgVector2 & getScale( ) const
    {
        return ((cgBillboardObject*)mReferencedObject)->getScale( );
    }
    inline cgUInt32 getColor( ) const
    {
        return ((cgBillboardObject*)mReferencedObject)->getColor( );
    }
    inline cgFloat getRotation( ) const
    {
        return ((cgBillboardObject*)mReferencedObject)->getRotation( );
    }
    inline cgFloat getHDRScale( ) const
    {
        return ((cgBillboardObject*)mReferencedObject)->getHDRScale( );
    }
    inline bool isBillboardAligned( ) const
    {
        return ((cgBillboardObject*)mReferencedObject)->isBillboardAligned( );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_BillboardNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                        createBillboard         ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBillboardBuffer     * mBillboardBuffer;
    cgBillboard3D         * mBillboard;
};

#endif // !_CGE_CGBILLBOARDOBJECT_H_