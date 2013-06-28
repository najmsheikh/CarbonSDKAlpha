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
// Name : cgStandardMaterial.h                                               //
//                                                                           //
// Desc : Provides classes which outline the various standard material       //
//        properties that should be applied to individual surfaces within a  //
//        scene. These include properties such as light reflectance,         //
//        samplers and potentially even physical properties (friction etc.)  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSTANDARDMATERIAL_H_ )
#define _CGE_CGSTANDARDMATERIAL_H_

//-----------------------------------------------------------------------------
// cgStandardMaterial Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgMaterial.h>
#include <Rendering/cgRenderingTypes.h>
#include <World/cgWorldQuery.h>
#include <System/cgPropertyContainer.h>
#include <Math/cgBezierSpline.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-------------------------------------------------------------------   ----------
class cgSampler;
class cgBezierSpline2;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {7C52DB87-9AE9-4B3B-929A-F257E48DC6C1}
const cgUID RTID_StandardMaterial = {0x7C52DB87, 0x9AE9, 0x4B3B, {0x92, 0x9A, 0xF2, 0x57, 0xE4, 0x8D, 0xC6, 0xC1}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgStandardMaterial (Class)
/// <summary>
/// Describes physical properties, for both physical processing and
/// rendering purposes, of any given surface in the world.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgStandardMaterial : public cgMaterial
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgStandardMaterial, cgMaterial, "StandardMaterial" )

public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerators
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE(cgSampler*, SamplerArray)

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgStandardMaterial( cgUInt32 referenceId, cgWorld * world );
             cgStandardMaterial( cgUInt32 referenceId, cgWorld * world, cgStandardMaterial * init );
             cgStandardMaterial( cgUInt32 referenceId, cgWorld * world, cgUInt32 sourceRefId );
    virtual ~cgStandardMaterial( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool                unloadResource              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_StandardMaterial; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldResourceComponent)
    //-------------------------------------------------------------------------
    virtual void                onComponentDeleted          ( );
    virtual cgString            getDatabaseTable            ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgMaterial)
    //-------------------------------------------------------------------------
    virtual cgInt                   compare                     ( const cgMaterial & material ) const;
    virtual bool                    loadMaterial                ( cgUInt32 sourceRefId, cgResourceManager * resourceManager = CG_NULL );
    virtual bool                    apply                       ( cgRenderDriver * driver );
    virtual void                    reset                       ( cgRenderDriver * driver );
    virtual bool                    setSurfaceShader            ( const cgSurfaceShaderHandle & shader );
    virtual bool                    buildPreviewImage           ( cgScene * scene, cgRenderView * view );
    virtual cgSurfaceShaderHandle   getSurfaceShader            ( bool assignedOnly );

    // Promote remaining base class method overloads.
    using cgMaterial::getSurfaceShader;
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setMaterialTerms            ( const cgMaterialTerms & terms );
    void                        setDiffuse                  ( const cgColorValue & color );
    void                        setAmbient                  ( const cgColorValue & color );
    void                        setSpecular                 ( const cgColorValue & color );
    void                        setGloss                    ( cgFloat scale );
    void                        setEmissive                 ( const cgColorValue & color );
    void                        setEmissiveHDRScale         ( cgFloat scale );
    void                        setOpacity                  ( cgFloat opacity );
    void                        setSpecularOpacity          ( cgFloat opacity );
    void                        setSpecularOpacityLinked    ( bool linked );
    void                        setMetalnessTerms           ( cgFloat amount, cgFloat diffuseIntensity, cgFloat specularIntensity );
    void                        setFresnelTerms             ( cgFloat diffuse, cgFloat specular, cgFloat reflection, cgFloat opacity, cgFloat exponent );
    void                        setRimLightTerms            ( cgFloat amount, cgFloat exponent );
    void                        setReflectionMode           ( cgReflectionMode::Base mode );
    void                        setReflectionTerms          ( cgFloat amount, cgFloat bumpiness, cgFloat mipLevel );
    void                        setOpacityMapContributions  ( cgFloat diffuseAmount, cgFloat specularAmount );
    void                        setTransmissionCurve        ( const cgBezierSpline2 & curve );
    const cgBezierSpline2     & getTransmissionCurve        ( ) const;
    const cgMaterialTerms     & getMaterialTerms            ( ) const;
    bool                        isSpecularOpacityLinked     ( ) const;
    cgReflectionMode::Base      getReflectionMode           ( ) const;
    const SamplerArray        & getSamplers                 ( ) const;
    cgSampler                 * getSamplerByName            ( const cgString & name );
    cgSampler                 * addSampler                  ( const cgString & name );
    cgSampler                 * addSampler                  ( const cgString & name, const cgTextureHandle & texture );
    cgSampler                 * addSampler                  ( const cgString & name, cgInputStream texture, cgUInt32 textureLoadFlags = 0, const cgDebugSourceInfo & debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        removeSampler               ( const cgString & name );
    void                        applySamplers               ( );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    inline const cgColorValue & getDiffuse( ) const
    {
        return mMaterialTerms.diffuse;
    }
    inline const cgColorValue & getAmbient( ) const
    {
        return mMaterialTerms.ambient;
    }
    inline const cgColorValue & getSpecular( ) const
    {
        return mMaterialTerms.specular;
    }
    inline const cgColorValue & getEmissive( ) const
    {
        return mMaterialTerms.emissive;
    }
    inline cgFloat getGloss( ) const
    {
        return mMaterialTerms.gloss;
    }
    inline cgFloat getOpacity( ) const
    {
        return mMaterialTerms.diffuse.a;
    }
    inline cgFloat getSpecularOpacity( ) const
    {
        return mMaterialTerms.specular.a;
    }
    inline cgFloat getEmissiveHDRScale( ) const
    {
        return mMaterialTerms.emissiveHDRScale;
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    enum SerializeDirtyFlags
    {
        DiffuseDirty            = 0x80,
        AmbientDirty            = 0x100,
        SpecularDirty           = 0x200,
        EmissiveDirty           = 0x400,
        BlendingDirty           = 0x800,
        MetalnessDirty          = 0x1000,
        RimLightDirty           = 0x2000,
        ReflectionDirty         = 0x4000,
        FresnelDirty            = 0x8000,
        TransmissionDirty       = 0x10000,
        SamplersDirty           = 0x20000,
        ParametersDirty         = 0x40000
    };
    
    CGE_MAP_DECLARE(cgString, cgSampler*, NameSamplerMap)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle     & getActiveSurfaceShader      ( );
    void                        prepareQueries              ( );
    void                        onSamplerAdded              ( const cgString & name, cgSampler * sampler );
    void                        onSamplerRemoved            ( const cgString & name );

    //-------------------------------------------------------------------------
    // Protected Static Methods
    //-------------------------------------------------------------------------
    static bool                 samplerSortPredicate        ( const cgSampler * lhs, const cgSampler * rhs );
    
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgMaterial)
    //-------------------------------------------------------------------------
    virtual bool                serializeMaterial           ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // Material data.
    cgSurfaceShaderHandle       mDefaultShader;             // Unique instance of the default surface shader if no actual shader is supplied.
    cgMaterialTerms             mMaterialTerms;             // Material / lighting reflectance properties.
    cgReflectionMode::Base      mReflectionMode;            // The manner in which reflections will be handled for geometry utilizing this material.
    bool                        mSpecularOpacityLinked;     // When this is enabled, the specular opacity term will always match the standard diffuse opacity.
    SamplerArray                mSamplers;                  // The textures/samplers defined for this material
	NameSamplerMap              mNamedSamplers;             // Table of samplers for fast name-based lookup (same samplers as above array)
    cgBezierSpline2             mTransmissionCurve;         // The curve that describes the amount of light which is transmitted through the surface in relation to its per-pixel opacity.
    cgPropertyContainer         mUserProperties;            // Optional user-defined properties 

    // Component availability
    cgSampler                 * mExplicitDiffuseSampler;    // Any explicit diffuse sampler applied to this material.
    cgSampler                 * mExplicitNormalSampler;     // Any explicit normal sampler applied to this material.
    cgSampler                 * mExplicitOpacitySampler;    // Any explicit opacity sampler applied to this material.
    bool                        mHasEmissiveTexture;        // Is emissive texture available?
    bool                        mHasOpacityTexture;         // Is opacity texture available?
    cgInt32                     mSpecularTextureChannels;   // The number of channels in the specular texture (determines type)
    cgInt32                     mNormalsType;               // The type of normals provided (0 = vertex, 1 = normal map, 2 = parallax).
    cgInt32                     mLightmapsType;             // The type of lightmaps in use (0 = none, 1 = standard, 2 = rnm).
    
    cgInt32                     mDiffuseSamplerRegister;    // Register index for the surface shader's diffuse sampler (if any).
    cgInt32                     mNormalSamplerRegister;     // Register index for the surface shader's normal sampler (if any).
    bool                        mRegisterIndicesCached;     // Have the above sampler register indices currently been queried?
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertMaterial;
    static cgWorldQuery mInsertSampler;
    static cgWorldQuery mDeleteSamplers;
    static cgWorldQuery mUpdateName;
    static cgWorldQuery mUpdateProperties;
    static cgWorldQuery mUpdateDiffuse;
    static cgWorldQuery mUpdateSpecular;
    static cgWorldQuery mUpdateAmbient;
    static cgWorldQuery mUpdateEmissive;
    static cgWorldQuery mUpdateMetalness;
    static cgWorldQuery mUpdateRimLight;
    static cgWorldQuery mUpdateReflection;
    static cgWorldQuery mUpdateFresnel;
    static cgWorldQuery mUpdateBlending;
    static cgWorldQuery mUpdateTransmission;
    static cgWorldQuery mUpdateShader;
    static cgWorldQuery mUpdatePreview;
    static cgWorldQuery mLoadMaterial;
    static cgWorldQuery mLoadSamplers;
};

#endif // !_CGE_CGSTANDARDMATERIAL_H_