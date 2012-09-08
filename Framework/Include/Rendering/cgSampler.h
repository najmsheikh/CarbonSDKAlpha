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
// File : cgSampler.h                                                        //
//                                                                           //
// Desc : Primary texture sampling classes. Samplers allows the application  //
//        to define how the rendering device should sample from the attached //
//        texture, and provides the means to bind the texture data to any    //
//        necessary effect parameters.                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSAMPLER_H_ )
#define _CGE_CGSAMPLER_H_

//-----------------------------------------------------------------------------
// cgSampler Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Rendering/cgRenderingTypes.h>
#include <World/cgWorldComponent.h>
#include <World/cgWorldQuery.h>
#include <Resources/cgResourceHandles.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5B5A1F05-6E02-46E8-84E9-95AF64089BD8}
const cgUID RTID_Sampler = {0x5B5A1F05, 0x6E02, 0x46E8, {0x84, 0xE9, 0x95, 0xAF, 0x64, 0x8, 0x9B, 0xD8}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSampler (Class)
/// <summary>
/// Texture sampler wrapper class.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSampler : public cgWorldComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSampler, cgWorldComponent, "Sampler" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgSampler( cgSampler * init ); 
     cgSampler( cgRenderDriver * driver, const cgString & name, const cgSurfaceShaderHandle & shader ); 
     cgSampler( cgRenderDriver * driver, const cgSurfaceShaderHandle & shader, cgSampler * init ); 
     cgSampler( cgUInt32 referenceId, cgWorld * world, cgRenderDriver * driver, const cgString & name, const cgSurfaceShaderHandle & shader );
     cgSampler( cgUInt32 referenceId, cgWorld * world, cgRenderDriver * driver, const cgSurfaceShaderHandle & shader, cgSampler * init );
    ~cgSampler();

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
	bool                            apply                   ( );
    bool                            apply                   ( cgUInt32 samplerRegister );
    bool                            apply                   ( const cgTextureHandle & texture );
    bool                            apply                   ( cgUInt32 samplerRegister, const cgTextureHandle & texture );
    bool                            setTexture              ( const cgTextureHandle & texture );
    bool                            setTexture              ( const cgTextureHandle & texture, bool noSerialize );
    bool                            loadTexture             ( const cgInputStream & stream, cgUInt32 flags );
    bool                            loadTexture             ( const cgInputStream & stream, cgUInt32 flags, const cgDebugSourceInfo & _debugSource );
    const cgTextureHandle         & getTexture              ( ) const;
    cgTextureHandle                 getTexture              ( );
    const cgSurfaceShaderHandle   & getSurfaceShader        ( ) const;
    cgSurfaceShaderHandle           getSurfaceShader        ( );
    bool                            isSystemSampler         ( ) const;
    bool                            isTextureValid          ( ) const;
    void                            setStrength             ( cgFloat strength );
    void                            setStates               ( const cgSamplerStateDesc & states );
    void                            setAddressU             ( cgAddressingMode::Base value );
    void                            setAddressV             ( cgAddressingMode::Base value );
    void                            setAddressW             ( cgAddressingMode::Base value );
    void                            setBorderColor          ( const cgColorValue & value );
    void                            setMagnificationFilter  ( cgFilterMethod::Base value );
    void                            setMinificationFilter   ( cgFilterMethod::Base value );
    void                            setMipmapFilter         ( cgFilterMethod::Base value );
    void                            setMipmapLODBias        ( cgFloat value );
    void                            setMaximumMipmapLOD     ( cgFloat value );
    void                            setMinimumMipmapLOD     ( cgFloat value );
    void                            setMaximumAnisotropy    ( cgInt32 value );
    cgFloat                         getStrength             ( ) const;
    const cgSamplerStateDesc      & getStates               ( ) const;
    const cgString                & getName                 ( ) const;
    cgInt                           compare                 ( const cgSampler & sampler ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_Sampler; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool            onComponentCreated  ( cgComponentCreatedEventArgs * e );
    virtual bool            onComponentLoading  ( cgComponentLoadingEventArgs * e );
    virtual cgString        getDatabaseTable    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    prepareQueries      ( );
    bool                    insertComponentData ( );
    void                    defaultSamplerStates( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgInt32                 mSamplerIndex;  // Index of the sampler register to which this will be applied.
	cgString                mName;          // Sampler base name (e.g., Diffuse, Light, Normal, etc.)
    cgRenderDriver        * mDriver;        // Render driver associated with this sampler
	cgTextureHandle         mTexture;       // Handle of the physical texture resource to apply
	cgSamplerStateDesc      mStateDesc;     // Description of the device sampler sampler states to apply.
    cgSamplerStateHandle    mStates;        // Handle to device state object to be applied.
    cgSurfaceShaderHandle   mShader;        // Any applicable surface shader to use during application.
    cgFloat                 mStrength;      // Strength of the sampled data / how much it should influence the result.
    // ToDo: 9999 - I don't think the system sampler concept even applies any more. Remove?
	bool                    mSystemSampler; // Does the sampler destination belong to the core shader lib?
    bool                    mStatesDirty;   // Sampler states have been updated since the last application?

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertSampler;
    static cgWorldQuery mUpdateTexture;
    static cgWorldQuery mUpdateAddressModes;
    static cgWorldQuery mUpdateFilterMethods;
    static cgWorldQuery mUpdateLODDetails;
    static cgWorldQuery mUpdateMaxAnisotropy;
    static cgWorldQuery mUpdateBorderColor;
    static cgWorldQuery mLoadSampler;
};

#endif // !_CGE_CGSAMPLER_H_