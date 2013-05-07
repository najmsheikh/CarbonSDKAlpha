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
// Name : cgLandscapeLayerMaterial.h                                         //
//                                                                           //
// Desc : Provides classes which outline layer material data for use with    //
//        the the scene landscape system. Such materials can either be       //
//        manually applied to the terrain by the user with the landscape     //
//        layer painting tools, or applied procedurally by creating a        //
//        procedural layer material.                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLANDSCAPELAYERMATERIAL_H_ )
#define _CGE_CGLANDSCAPELAYERMATERIAL_H_

//-----------------------------------------------------------------------------
// cgLandscapeLayerMaterial Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgMaterial.h>
#include <Rendering/cgRenderingTypes.h>
#include <World/cgWorldQuery.h>
#include <System/cgPropertyContainer.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-------------------------------------------------------------------   ----------
class cgSampler;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {6E0FD2D7-0BB6-4682-A85A-7B5072C96598}
const cgUID RTID_LandscapeLayerMaterial = {0x6E0FD2D7, 0xBB6, 0x4682, {0xA8, 0x5A, 0x7B, 0x50, 0x72, 0xC9, 0x65, 0x98}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgLandscapeLayerMaterial (Class)
/// <summary>
/// Describes properties, for both physical processing and rendering purposes, 
/// of a manually painted landscape terrain layer.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLandscapeLayerMaterial : public cgMaterial
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgLandscapeLayerMaterial, cgMaterial, "LandscapeLayerMaterial" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgLandscapeLayerMaterial( cgUInt32 referenceId, cgWorld * world );
             cgLandscapeLayerMaterial( cgUInt32 referenceId, cgWorld * world, cgLandscapeLayerMaterial * init );
             cgLandscapeLayerMaterial( cgUInt32 referenceId, cgWorld * world, cgUInt32 sourceRefId );
    virtual ~cgLandscapeLayerMaterial( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool                unloadResource              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_LandscapeLayerMaterial; }
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
    virtual cgInt               compare                     ( const cgMaterial & material ) const;
    virtual bool                loadMaterial                ( cgUInt32 sourceRefId, cgResourceManager * manager = CG_NULL );
    virtual bool                apply                       ( cgRenderDriver * driver );
    virtual void                reset                       ( cgRenderDriver * driver );
    virtual bool                setSurfaceShader            ( const cgSurfaceShaderHandle & shader );
    virtual bool                buildPreviewImage           ( cgScene * scene, cgRenderView * view );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgSampler                 * getColorSampler             ( ) const;
    cgSampler                 * getNormalSampler            ( ) const;
    const cgVector2           & getScale                    ( ) const;
    const cgVector2           & getBaseScale                ( ) const;
    const cgVector2           & getOffset                   ( ) const;
    cgFloat                     getAngle                    ( ) const;
    bool                        getTilingReduction          ( ) const;
    void                        setColorSampler             ( cgSampler * sampler );
    void                        setNormalSampler            ( cgSampler * sampler );
    void                        setScale                    ( const cgVector2 & scale );
    void                        setBaseScale                ( const cgVector2 & scale );
    void                        setOffset                   ( const cgVector2 & offset );
    void                        setAngle                    ( cgFloat angle );
    void                        enableTilingReduction       ( bool enable );
    bool                        loadColorSampler            ( cgInputStream stream );
    bool                        loadNormalSampler           ( cgInputStream stream );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    enum SerializeDirtyFlags
    {
        SamplersDirty           = 0x8,
        BaseScaleDirty          = 0x10,
        ScaleDirty              = 0x20,
        OffsetDirty             = 0x40,
        AngleDirty              = 0x80,
        TilingReductionDirty    = 0x100
    };

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries              ( );
    
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgMaterial)
    //-------------------------------------------------------------------------
    virtual bool                serializeMaterial           ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>The color map to apply to this layer (i.e. rock, grass, etc.)</summary>
    cgSampler         * mColorSampler;
    /// <summary>The normal map to associated with this layer.</summary>
    cgSampler         * mNormalSampler;
    /// <summary>Planar mapped texture coordinate scale.</summary>
    cgVector2           mScale, mBaseScale;
    /// <summary>Planar mapped texture coordinate offset / translation.</summary>
    cgVector2           mOffset;
    /// <summary>Planar mapped texture coordinate rotation angle (in degrees).</summary>
    cgFloat             mAngle;
    /// <summary>Tiling reduction is enabled / disabled for this layer.</summary>
    bool                mTilingReduction;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertMaterial;
    static cgWorldQuery mUpdateName;
    static cgWorldQuery mUpdateSamplers;
    static cgWorldQuery mUpdateBaseScale;
    static cgWorldQuery mUpdateScale;
    static cgWorldQuery mUpdateOffset;
    static cgWorldQuery mUpdateAngle;
    static cgWorldQuery mUpdateTilingReduction;
    static cgWorldQuery mUpdatePreview;
    static cgWorldQuery mLoadMaterial;
};

#endif // !_CGE_CGLANDSCAPELAYERMATERIAL_H_