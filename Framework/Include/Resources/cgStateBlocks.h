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
// Name : cgStateBlocks.h                                                    //
//                                                                           //
// Desc : Contains classes that represent the various state blocks that can  //
//        be applied to the render driver / hardware. Such states include    //
//        sampler, rasterizer, depth stencil and blend state objects. See    //
//        cgResourceTypes.h for information relating to the individual       //
//        description structures that are used to initialize these blocks.   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSTATEBLOCKS_H_ )
#define _CGE_CGSTATEBLOCKS_H_

//-----------------------------------------------------------------------------
// cgStateBlocks Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResource.h>
#include <Rendering/cgRenderingTypes.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {7950F95F-DD38-4460-AE73-ABF26B752E67}
const cgUID RTID_SamplerStateResource      = {0x7950F95F, 0xDD38, 0x4460, {0xAE, 0x73, 0xAB, 0xF2, 0x6B, 0x75, 0x2E, 0x67}};
// {E830149E-91A3-4365-9EF6-30965F43F3FA}
const cgUID RTID_DepthStencilStateResource = {0xE830149E, 0x91A3, 0x4365, {0x9E, 0xF6, 0x30, 0x96, 0x5F, 0x43, 0xF3, 0xFA}};
// {2AF8F595-C367-4C27-AE50-F1F65C866367}
const cgUID RTID_RasterizerStateResource   = {0x2AF8F595, 0xC367, 0x4C27, {0xAE, 0x50, 0xF1, 0xF6, 0x5C, 0x86, 0x63, 0x67}};
// {137D8547-7B98-4A22-A387-3B1A32218214}
const cgUID RTID_BlendStateResource        = {0x137D8547, 0x7B98, 0x4A22, {0xA3, 0x87, 0x3B, 0x1A, 0x32, 0x21, 0x82, 0x14}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSamplerState (Base Class)
/// <summary>
/// Base state block object designed to maintain a set of values which
/// represent the state of an individual device sampler. See cgSamplerStateDesc
/// for details on the state values which can be supplied. This base class is
/// designed to be inherited by API specific versions (in 'Platform' directory)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSamplerState : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSamplerState, cgResource, "SamplerState" )

public:
    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgSamplerState * createInstance      ( cgUInt32 referenceId, const cgSamplerStateDesc & states );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSamplerState( cgUInt32 referenceId, const cgSamplerStateDesc & states );
    virtual ~cgSamplerState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgSamplerStateDesc  & getValues       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_SamplerStateResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSamplerStateDesc  mValues;     // State values
};

//-----------------------------------------------------------------------------
//  Name : cgDepthStencilState (Base Class)
/// <summary>
/// Base state block object designed to maintain a set of values which
/// represent the manner in which the device's depth / stencil buffering
/// features will function. See cgDepthStencilStateDesc for details on the 
/// state values which can be supplied. This base class is designed to be 
/// inherited by API specific versions (in 'Platform' directory)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDepthStencilState : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDepthStencilState, cgResource, "DepthStencilState" )

public:
    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgDepthStencilState    * createInstance  ( cgUInt32 referenceId, const cgDepthStencilStateDesc & states );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDepthStencilState( cgUInt32 referenceId, const cgDepthStencilStateDesc & states );
    virtual ~cgDepthStencilState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgDepthStencilStateDesc & getValues       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DepthStencilStateResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgDepthStencilStateDesc mValues;     // State values
};

//-----------------------------------------------------------------------------
//  Name : cgRasterizerState (Base Class)
/// <summary>
/// Base state block object designed to maintain a set of values which
/// represent the manner in which the device's rasterization features will 
/// function. See cgRasterizerStateDesc for details on the state values which 
/// can be supplied. This base class is designed to be inherited by API 
/// specific versions (in 'Platform' directory)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgRasterizerState : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgRasterizerState, cgResource, "RasterizerState" )

public:
    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgRasterizerState      * createInstance  ( cgUInt32 referenceId, const cgRasterizerStateDesc & states );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgRasterizerState( cgUInt32 referenceId, const cgRasterizerStateDesc & states );
    virtual ~cgRasterizerState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgRasterizerStateDesc   & getValues       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_RasterizerStateResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgRasterizerStateDesc   mValues;     // State values
};

//-----------------------------------------------------------------------------
//  Name : cgBlendState (Base Class)
/// <summary>
/// Base state block object designed to maintain a set of values which
/// represent the manner in which the device's blending / output merging 
/// features will function. See cgBlendStateDesc for details on the state 
/// values which can be supplied. This base class is designed to be inherited 
/// by API specific versions (in 'Platform' directory)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBlendState : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBlendState, cgResource, "BlendState" )

public:
    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgBlendState   * createInstance  ( cgUInt32 referenceId, const cgBlendStateDesc & states );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBlendState( cgUInt32 referenceId, const cgBlendStateDesc & states );
    virtual ~cgBlendState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgBlendStateDesc& getValues       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_BlendStateResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBlendStateDesc    mValues;     // State values
};

#endif // !_CGE_CGSTATEBLOCKS_H_