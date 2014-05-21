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
// Name : cgMaterial.h                                                       //
//                                                                           //
// Desc : Provides base classes from which all material types should be      //
//        derived. Application resource manager will manage these types      //
//        via the material base class.                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGMATERIAL_H_ )
#define _CGE_CGMATERIAL_H_

//-----------------------------------------------------------------------------
// cgMaterial Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldResourceComponent.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {1A6BDC60-49ED-4D95-A42F-0BA745AC4C31}
const cgUID RTID_MaterialResource = {0x1A6BDC60, 0x49ED, 0x4D95, {0xA4, 0x2F, 0xB, 0xA7, 0x45, 0xAC, 0x4C, 0x31}};

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;
class cgRenderView;
class cgScene;
class cgImage;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgMaterial (Class)
/// <summary>
/// Describes physical properties, for both physical processing and
/// rendering purposes, of any given surface in the world.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgMaterial : public cgWorldResourceComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgMaterial, cgWorldResourceComponent, "Material" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgMaterial( cgUInt32 referenceId, cgWorld * world );
             cgMaterial( cgUInt32 referenceId, cgWorld * world, cgMaterial * init );
             cgMaterial( cgUInt32 referenceId, cgWorld * world, cgUInt32 sourceRefId );
    virtual ~cgMaterial( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool                    loadResource                ( );
    virtual bool                    unloadResource              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType            ( ) const { return RTID_MaterialResource; }
    virtual bool                    queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                     ( bool disposeBase );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgString                & getName                     ( ) const;
    cgUInt64                        getMaterialProperties       ( ) const;
    cgSurfaceShaderHandle           getSurfaceShader            ( );
    cgImage                       * getPreviewImage             ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                    canBatch                    ( ) const { return false; }
    virtual bool                    apply                       ( cgRenderDriver * driver ) = 0;
    virtual bool                    loadMaterial                ( cgUInt32 sourceRefId, cgResourceManager * resourceManager = CG_NULL ) = 0;
    virtual void                    reset                       ( cgRenderDriver * driver );
    virtual cgInt                   compare                     ( const cgMaterial & material ) const;
    virtual void                    setName                     ( const cgString & name );
    virtual void                    setMaterialProperties       ( cgUInt64 properties );
    virtual bool                    addMaterialProperty         ( const cgString & identifier );
    virtual bool                    removeMaterialProperty      ( const cgString & identifier );
    virtual bool                    setSurfaceShader            ( const cgSurfaceShaderHandle & shader );
    virtual void                    beginPopulate               ( );
    virtual void                    beginPopulate               ( cgResourceManager * resourceManager );
    virtual void                    endPopulate                 ( );
    virtual bool                    buildPreviewImage           ( cgScene * scene, cgRenderView * view );
    virtual cgSurfaceShaderHandle   getSurfaceShader            ( bool assignedOnly );

protected:
    //-------------------------------------------------------------------------
    // Protected Static Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 NameDirty       = 0x1;
    static const cgUInt32 PropertiesDirty = 0x2;
    static const cgUInt32 ShaderFileDirty = 0x4;
    static const cgUInt32 AllDirty        = 0xFFFFFFFF;

    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                serializeMaterial           ( ) = 0;

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    void                        setManagementData           ( cgResourceManager * resourceManager, cgUInt32 flags );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // Resource loading properties.
    cgUInt32                mSourceRefId;           // Reference identifier of the material source data from which we should load (if any).

    // Serialization.
    bool                    mMaterialSerialized;    // Has data for this material been serialized yet? (i.e. the initial insert).
    cgUInt32                mDBDirtyFlags;          // Which components of the material are dirty?
    
    // Material data.
    cgString                mName;                  // The editor name of the material.
    cgUInt64                mProperties;            // Material property bits. Used to identify and filter materials based on user customizable properties.

    // resources
    cgSurfaceShaderHandle   mSurfaceShader;         // The underlying surface shader we should use for rendering.

    // Sandbox: Preview rendering
    cgImage               * mPreviewImage;          // Preview image data for the sandbox material editor.
};

#endif // !_CGE_CGMATERIAL_H_