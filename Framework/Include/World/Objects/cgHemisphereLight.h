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
// Name : cgHemisphereLight.h                                                //
//                                                                           //
// Desc : Hemisphere light source classes.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGHEMISPHERELIGHT_H_ )
#define _CGE_CGHEMISPHERELIGHT_H_

//-----------------------------------------------------------------------------
// cgHemisphereLight Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgLightObject.h>
#include <World/Lighting/cgShadowGenerator.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {A30147BC-92D5-4FAF-98D5-E729B9942057}
const cgUID RTID_HemisphereLightNode   = {0xA30147BC, 0x92D5, 0x4FAF, {0x98, 0xD5, 0xE7, 0x29, 0xB9, 0x94, 0x20, 0x57}};
// {58AC6038-C67D-4EFB-8CA1-2E81600DD201}
const cgUID RTID_HemisphereLightObject = {0x58AC6038, 0xC67D, 0x4EFB, {0x8C, 0xA1, 0x2E, 0x81, 0x60, 0x0D, 0xD2, 0x01}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgHemisphereLightObject (Class)
// Desc : Hemisphere light source object.
//-----------------------------------------------------------------------------
class CGE_API cgHemisphereLightObject : public cgLightObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgHemisphereLightObject, cgLightObject, "HemisphereLightObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgHemisphereLightObject( cgUInt32 referenceId, cgWorld * world );
             cgHemisphereLightObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgHemisphereLightObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject          * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject          * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    void                            setOuterRange           ( cgFloat range );
    void                            setRangeAdjust          ( cgFloat rangeAdjust );
    void                            setInnerRange           ( cgFloat range );
    void                            setDiffuseBackHDRScale  ( cgFloat scale );
    void                            setSpecularBackHDRScale ( cgFloat scale );
    void                            setDiffuseBackColor     ( const cgColorValue & color );
    void                            setSpecularBackColor    ( const cgColorValue & color );
    cgFloat                         getOuterRange           ( ) const;
    cgFloat                         getRangeAdjust          ( ) const;
    cgFloat                         getInnerRange           ( ) const;
    cgFloat                         getDiffuseBackHDRScale  ( ) const;
    cgFloat                         getSpecularBackHDRScale ( ) const;
    const cgColorValue            & getDiffuseBackColor     ( ) const;
    const cgColorValue            & getSpecularBackColor    ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual LightType               getLightType            ( ) const;
    virtual void                    setDiffuseColor         ( const cgColorValue & color );
    virtual void                    setSpecularColor        ( const cgColorValue & color );
    
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
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_HemisphereLightObject; }
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
    cgFloat                 mOuterRange;                // Outer range of this light source outside of which no light is cast.
    cgFloat                 mInnerRange;                // Inner range of this light source inside of which light is full-bright.
    cgFloat                 mRangeAdjust;               // Range error adjustment for use when drawing low-res geometric light shape.
    cgColorValue            mDiffuseBackColor;          // Back lighting (hemisphere) diffuse color value.
    cgColorValue            mSpecularBackColor;         // Back lighting (hemisphere) specular color value.
    
    // HDR settings.
    cgFloat                 mDiffuseBackHDRScale;       //< Defines the intensity of the diffuse back lighting component when HDR is in use.
    cgFloat                 mSpecularBackHDRScale;      //< Defines the intensity of the specular back lighting component when HDR is in use.;
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertHemisphereLight;
    static cgWorldQuery     mUpdateRanges;
    static cgWorldQuery     mUpdateHemisphereHDRScalars;
    static cgWorldQuery     mUpdateDiffuseBackColor;
    static cgWorldQuery     mUpdateSpecularBackColor;
    static cgWorldQuery     mLoadHemisphereLight;
};

//-----------------------------------------------------------------------------
// Name : cgHemisphereLightNode (Class)
// Desc : Point light source object node.
//-----------------------------------------------------------------------------
class CGE_API cgHemisphereLightNode : public cgLightNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgHemisphereLightNode, cgLightNode, "HemisphereLightNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgHemisphereLightNode( cgUInt32 referenceId, cgScene * scene );
             cgHemisphereLightNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgHemisphereLightNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode           * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode           * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightNode)
    //-------------------------------------------------------------------------
    virtual bool                    boundsInVolume          ( const cgBoundingBox & bounds );
    virtual bool                    pointInVolume           ( const cgVector3 & point, cgFloat errorRadius = 0.0f );
    virtual bool                    frustumInVolume         ( const cgFrustum & frustum );
    virtual bool                    testObjectShadowVolume  ( cgObjectNode * object, const cgFrustum & viewFrustum );
    
    virtual void                    computeLevelOfDetail    ( cgCameraNode * camera );

    virtual cgInt32                 beginLighting           ( cgTexturePool * pool, bool applyShadows, bool deferred );
    virtual LightingOp              beginLightingPass       ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endLightingPass         ( );
    virtual bool                    endLighting             ( );

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
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_HemisphereLightNode; }
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
        ((cgHemisphereLightObject*)mReferencedObject)->setOuterRange( range );
    }
    inline void setRangeAdjust( cgFloat adjust )
    {
        ((cgHemisphereLightObject*)mReferencedObject)->setRangeAdjust( adjust );
    }
    inline void setInnerRange( cgFloat range )
    {
        ((cgHemisphereLightObject*)mReferencedObject)->setInnerRange( range );
    }
    inline void setDiffuseBackHDRScale( cgFloat scale )
    {
        ((cgHemisphereLightObject*)mReferencedObject)->setDiffuseBackHDRScale( scale );
    }
    inline void setSpecularBackHDRScale( cgFloat scale )
    {
        ((cgHemisphereLightObject*)mReferencedObject)->setSpecularBackHDRScale( scale );
    }
    inline void setDiffuseBackColor( const cgColorValue & color )
    {
        ((cgHemisphereLightObject*)mReferencedObject)->setDiffuseBackColor( color );
    }
    inline void setSpecularBackColor( const cgColorValue & color )
    {
        ((cgHemisphereLightObject*)mReferencedObject)->setSpecularBackColor( color );
    }
    
    // Object Property 'Get' Routing
    inline cgFloat getOuterRange( ) const
    {
        return ((cgHemisphereLightObject*)mReferencedObject)->getOuterRange( );
    }
    inline cgFloat getRangeAdjust( ) const
    {
        return ((cgHemisphereLightObject*)mReferencedObject)->getRangeAdjust( );
    }
    inline cgFloat getInnerRange( ) const
    {
        return ((cgHemisphereLightObject*)mReferencedObject)->getInnerRange( );
    }
    inline cgFloat getDiffuseBackHDRScale( ) const
    {
        return ((cgHemisphereLightObject*)mReferencedObject)->getDiffuseBackHDRScale( );
    }
    inline cgFloat getSpecularBackHDRScale( ) const
    {
        return ((cgHemisphereLightObject*)mReferencedObject)->getSpecularBackHDRScale( );
    }
    inline const cgColorValue & getDiffuseBackColor( ) const
    {
        return ((cgHemisphereLightObject*)mReferencedObject)->getDiffuseBackColor( );
    }
    inline const cgColorValue & getSpecularBackColor( ) const
    {
        return ((cgHemisphereLightObject*)mReferencedObject)->getSpecularBackColor( );
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual void                setClipPlanes           ( );
    virtual bool                updateLightConstants    ( );
    virtual bool                updateSystemConstants   ( );
    virtual bool                postCreate              ( );
};

#endif // !_CGE_CGHEMISPHERELIGHT_H_

/*
//---------------------------------------------------------------------------//
//                     __  __           _       _        ___ ___ ___         //
//        __ _ _ __   |  \/  | ___   __| |_   _| | ___  |_ _|_ _|_ _|        //
//       / _` | '_ \  | |\/| |/ _ \ / _` | | | | |/ _ \  | | | | | |         //
//      | (_| | |_) | | |  | | (_) | (_| | |_| | |  __/  | | | | | |         //
//       \__, | .__/  |_|  |_|\___/ \__,_|\__,_|_|\___| |___|___|___|        //
//       |___/|_|      Game Institute Graphics Programming Module III        //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : gpHemisphereLight.h                                                //
//                                                                           //
// Desc : Hemisphere light source classes.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _GPF_GPHEMISPHERELIGHT_H_ )
#define _GPF_GPHEMISPHERELIGHT_H_

//-----------------------------------------------------------------------------
// gpHemisphereLight Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/gpLightObject.h>
#include <Math/gpFrustum.h>
#include <Math/gpCurve.h>
#include <System/gpXML.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class gpShadowFrustum;
class gpSampler;

//-----------------------------------------------------------------------------
// Globally Unique Identifier values for light object.
//-----------------------------------------------------------------------------
// Message Target Id(s)
// {97BC1C0C-35E8-4A79-9395-3727237166A4}
const gpUID MTID_HemisphereLight        = { 0x97BC1C0C, 0x35E8, 0x4A79, { 0x93, 0x95, 0x37, 0x27, 0x23, 0x71, 0x66, 0xA4 } };

// Scene Object Id(s)
// {58647882-BA0E-4868-91F2-267B4F2903C3} - Prescience UID
const gpUID SOID_HemisphereLight        = { 0x58647882, 0xBA0E, 0x4868, { 0x91, 0xF2, 0x26, 0x7B, 0x4F, 0x29, 0x03, 0xC3 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : gpHemisphereLight (Class)
// Desc : Hemisphere light source object.
//-----------------------------------------------------------------------------
class gpHemisphereLight : public gpLightObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( gpHemisphereLight, gpLightObject, "HemisphereLight" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             gpHemisphereLight( const gpString & strInstanceName, gpScene * pScene );
    virtual ~gpHemisphereLight( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static gpSceneObject      * Allocate            ( const gpString & strTypeName, const gpString & strInstanceName, gpScene * pScene );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        SetAttenuationNear  ( gpFloat fValue );
    void                        SetAttenuationFar   ( gpFloat fValue );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides gpLightObject)
    //-------------------------------------------------------------------------
    virtual LightType           getLightType        ( ) const;
    virtual void                SetRange            ( gpFloat range );
    virtual bool                boundsInVolume      ( const gpBoundingBox & AABB );
    virtual bool                pointInVolume       ( const cgVector3 & Point, gpFloat fErrRadius = 0.0f );
    virtual bool                frustumInVolume     ( const gpFrustum & Frustum );
    virtual void                computeVisibility   ( );
    virtual void                computeLevelOfDetail( gpCameraObject * pCamera );
    virtual bool                SetLightParameters  ( );
	
    virtual void                ComputeShadowSets   ( gpCameraNode * pCamera );
    virtual bool                ReassignShadowMaps  ( gpShadowMapPool * pPool );
    virtual gpInt32             BeginShadowFill     ( gpShadowMapPool * pPool );
    virtual bool                BeginShadowFillPass ( gpInt32 nPass, gpVisibilitySet *& pRenderSetOut );
    virtual bool                EndShadowFillPass   ( );
    virtual bool                EndShadowFill       ( );

    virtual gpInt32             beginLighting       ( gpShadowMapPool * pPool, bool bApplyShadows, bool bDeferred );
    virtual LightingOp          beginLightingPass   ( gpInt32 nPass, gpVisibilitySet *& pRenderSetOut );
    virtual bool                endLightingPass     ( );
    virtual bool                endLighting         ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides gpSceneObject)
    //-------------------------------------------------------------------------
    virtual void                SetObjectMatrix     ( const cgMatrix & mtx );
    virtual gpBoundingBox       GetBoundingBox      ( );
    virtual bool                QueryObjectType     ( const gpUID & Type );
    virtual bool                Deserialize         ( const gpXMLNode & InitData, gpSceneLoader * pLoader );
    virtual bool                InitObject          ( bool bAutoAddObject = true );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides gpMessageTarget)
    //-------------------------------------------------------------------------
    virtual gpUID               GetMessageTargetType( ) const { return MTID_HemisphereLight; }

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (gpLightObject)
    //-------------------------------------------------------------------------
   	virtual void                setClipPlanes       ( );
	
	//-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
	gpColorValue        m_DiffuseColor2;            // Diffuse color to be emitted by this light source type.  (Lower hemisphere)
    gpColorValue        m_AmbientColor2;            // Ambient color to be emitted by this light source type.  (Lower hemisphere)
    gpColorValue        m_SpecularColor2;           // Specular color to be emitted by this light source type. (Lower hemisphere)    

    gpFloat             m_fRange;                   // Range of this light source outside of which no light is cast.
	gpFloat             mRangeAdjust;             // Adjust range during light shape drawing.
	gpFloat             m_fAttenuationNear;         // The attenuation start distance.
	gpFloat             m_fAttenuationFar;          // The attenuation end distance. 

    gpFrustum           m_Frustum;                  // Details regarding the light frustum.
	gpShadowFrustum   * m_pShadowFrustum;           // The shadow frustums we need for a spot light shadowing implementation.
    gpUInt32            m_nShadowUpdateRate;        // The minimum rate at which child shadow maps will update in frames per second.
    gpFloat             m_fShadowTimeSinceLast;     // Amount of time that has elapsed since the last shadow update.
    gpCurve             m_DistanceAttenCurve;       // Curve that describes the attenuation to apply over distance.
    gpXMLNode           m_xAttenMaskData;           // Provides sampler data for the attenuation mask to apply (if any)
    
    // Rendering
    ResourceHandle      m_hDistanceAttenTex;        // 1D texture that describes the distance attenuation curve (if any).
    gpSampler         * m_pDistanceAttenSampler;    // Sampler used to supply distance attenuation as required.
    gpSampler         * m_pAttenMaskSampler;        // Sampler used to supply an attenuation mask that will be projected onto the environment.

	// Image based lighting (IBL)
    gpXMLNode           m_xDiffuseIBLData;          // Provides sampler data for the diffuse IBL color texture to project (if any).
    gpXMLNode           m_xSpecularIBLData;         // Provides sampler data for the specular IBL color texture to project (if any).
	gpSampler         * m_pDiffuseIBLSampler;       // Sampler used for the diffuse IBL cubemap.
    gpSampler         * m_pSpecularIBLSampler;      // Sampler used for the specular IBL cubemap.
};

#endif // !_GPF_GPHEMISPHERELIGHT_H_*/