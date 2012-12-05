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
// Name : cgSpotLight.h                                                      //
//                                                                           //
// Desc : Spot light source classes.                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSPOTLIGHT_H_ )
#define _CGE_CGSPOTLIGHT_H_

//-----------------------------------------------------------------------------
// cgSpotLight Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgLightObject.h>
#include <World/Lighting/cgShadowGenerator.h>
#include <Math/cgFrustum.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {4F2C7358-5B0B-46FA-B280-FE906EB46806}
const cgUID RTID_SpotLightNode   = { 0x4F2C7358, 0x5B0B, 0x46FA, { 0xB2, 0x80, 0xFE, 0x90, 0x6E, 0xB4, 0x68, 0x06 } };
// {9F4B0288-D70E-49CA-853E-6D1148397A58}
const cgUID RTID_SpotLightObject = { 0x9F4B0288, 0xD70E, 0x49CA, { 0x85, 0x3E, 0x6D, 0x11, 0x48, 0x39, 0x7A, 0x58 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgSpotLightObject (Class)
// Desc : Spot light source object.
//-----------------------------------------------------------------------------
class CGE_API cgSpotLightObject : public cgLightObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSpotLightObject, cgLightObject, "SpotLightObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSpotLightObject( cgUInt32 referenceId, cgWorld * world );
             cgSpotLightObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgSpotLightObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject          * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject          * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                            setInnerCone            ( cgFloat value );
    void                            setOuterCone            ( cgFloat value );
    void                            setFalloff              ( cgFloat value );
    void                            setOuterRange           ( cgFloat range );
    void                            setInnerRange           ( cgFloat range );
    void                            setShadowUpdateRate     ( cgUInt32 rate );
    cgFloat                         getInnerCone            ( ) const;
    cgFloat                         getOuterCone            ( ) const;
    cgFloat                         getFalloff              ( ) const;
    cgFloat                         getOuterRange           ( ) const;
    cgFloat                         getInnerRange           ( ) const;
    cgUInt32                        getShadowUpdateRate     ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual LightType               getLightType            ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox           getLocalBoundingBox     ( );
    virtual void                    sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                    pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual void                    applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                    onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                    onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString                getDatabaseTable        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_SpotLightObject; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                            prepareQueries          ( );
    bool                            insertComponentData     ( );

	//-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgFloat                 mOuterRange;        // Range of this light source outside of which no light is cast.
    cgFloat                 mInnerRange;        // Inner range of this light source inside of which light is full-bright.
    cgFloat                 mInnerCone;         // Inner cone angle in degrees.
    cgFloat                 mOuterCone;         // Outer cone angle in degrees.
    cgFloat                 mFalloff;           // Inner/outer cone falloff
    cgUInt32                mShadowUpdateRate;  // The minimum rate at which child shadow maps will update in frames per second.
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertSpotLight;
    static cgWorldQuery     mUpdateRanges;
    static cgWorldQuery     mUpdateConeAngles;
    static cgWorldQuery     mUpdateFalloff;
    static cgWorldQuery     mUpdateShadowRate;
    static cgWorldQuery     mLoadSpotLight;
};

//-----------------------------------------------------------------------------
// Name : cgSpotLightNode (Base Class)
/// <summary>
/// An individual spot light source object node.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSpotLightNode : public cgLightNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSpotLightNode, cgLightNode, "SpotLightNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSpotLightNode( cgUInt32 referenceId, cgScene * scene );
             cgSpotLightNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgSpotLightNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode           * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode           * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgFrustum               & getFrustum              ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                    boundsInVolume          ( const cgBoundingBox & bounds );
    virtual bool                    pointInVolume           ( const cgVector3 & point, cgFloat errorRadius = 0.0f );
    virtual bool                    frustumInVolume         ( const cgFrustum & frustum );
    virtual bool                    testObjectShadowVolume  ( cgObjectNode * object, const cgFrustum & viewFrustum );

    virtual void                    computeShadowSets       ( cgCameraNode * camera );
    // ToDo: 6767 - Remove
    virtual void                    computeVisibility       ( );
    virtual void                    computeLevelOfDetail    ( cgCameraNode * camera );

    // Direct lighting
    virtual bool                    reassignShadowMaps      ( cgTexturePool * pool );
    virtual cgInt32                 beginShadowFill         ( cgTexturePool * pool );
    virtual bool                    beginShadowFillPass     ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endShadowFillPass       ( );
    virtual bool                    endShadowFill           ( );

    virtual cgInt32                 beginLighting           ( cgTexturePool * pool, bool applyShadows, bool deferred );
    virtual LightingOp              beginLightingPass       ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endLightingPass         ( );
    virtual bool                    endLighting             ( );

    // Indirect lighting
	virtual void					updateIndirectSettings       ( );
    virtual bool                    reassignIndirectMaps         ( cgTexturePool * pool );
    virtual cgInt32                 beginIndirectFill            ( cgTexturePool * pool );
    virtual bool                    beginIndirectFillPass        ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endIndirectFillPass          ( );
    virtual bool                    endIndirectFill              ( );

	virtual cgInt32                 beginIndirectLighting        ( cgTexturePool * pool );
    virtual LightingOp              beginIndirectLightingPass    ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endIndirectLightingPass      ( );
    virtual bool                    endIndirectLighting          ( );

	virtual	void                    setIndirectLightingMethod    ( cgUInt32 method );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                    setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                    onComponentModified     ( cgComponentModifiedEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_SpotLightNode; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setOuterRange( cgFloat range )
    {
        ((cgSpotLightObject*)mReferencedObject)->setOuterRange( range );
    }
    inline void setInnerRange( cgFloat range )
    {
        ((cgSpotLightObject*)mReferencedObject)->setInnerRange( range );
    }
    inline void setFalloff( cgFloat value )
    {
        ((cgSpotLightObject*)mReferencedObject)->setFalloff( value );
    }
    inline void setOuterCone( cgFloat value )
    {
        ((cgSpotLightObject*)mReferencedObject)->setOuterCone( value );
    }
    inline void setInnerCone( cgFloat value )
    {
        ((cgSpotLightObject*)mReferencedObject)->setInnerCone( value );
    }
    inline void setShadowUpdateRate( cgUInt32 nValue )
    {
        ((cgSpotLightObject*)mReferencedObject)->setShadowUpdateRate( nValue );
    }
    
    // Object Property 'Get' Routing
    inline cgFloat getOuterRange( ) const
    {
        return ((cgSpotLightObject*)mReferencedObject)->getOuterRange( );
    }
    inline cgFloat getInnerRange( ) const
    {
        return ((cgSpotLightObject*)mReferencedObject)->getInnerRange( );
    }
    inline cgFloat getFalloff( ) const
    {
        return ((cgSpotLightObject*)mReferencedObject)->getFalloff( );
    }
    inline cgFloat getOuterCone( ) const
    {
        return ((cgSpotLightObject*)mReferencedObject)->getOuterCone( );
    }
    inline cgFloat getInnerCone( ) const
    {
        return ((cgSpotLightObject*)mReferencedObject)->getInnerCone( );
    }
    inline cgUInt32 getShadowUpdateRate( ) const
    {
        return ((cgSpotLightObject*)mReferencedObject)->getShadowUpdateRate( );
    }
    
protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct _cbSpotLight
    {
        cgVector4 spotTerms;
    
    }; // End Struct : _cbSpotLight

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                            createLightShape            ( );
    void                            updateFrustum               ( );
    
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgLightNode)
    //-------------------------------------------------------------------------
   	virtual void                    setClipPlanes               ( );
    virtual bool                    updateLightConstants        ( );
    virtual bool                    postCreate                  ( );
	
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgConstantBufferHandle      mSpotLightConstants;        // Custom spot lighting terms.
    cgShadowGenerator         * mShadowFrustum;             // The shadow frustums we need for a spot light shadowing implementation.
    cgFrustum                   mFrustum;                   // Details regarding the light frustum.
    cgFloat                     mShadowTimeSinceLast;       // Amount of time that has elapsed since the last shadow update.
    cgReflectanceGenerator    * mIndirectFrustum;           // The reflective shadow frustums we need for dynamic ambient lighting
};

#endif // !_CGE_CGSPOTLIGHT_H_