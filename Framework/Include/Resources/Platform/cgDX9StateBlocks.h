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
// Name : cgDX9StateBlocks.h                                                 //
//                                                                           //
// Desc : Contains classes that represent the various state blocks that can  //
//        be applied to the render driver / hardware. Such states include    //
//        sampler, rasterizer, depth stencil and blend state objects. See    //
//        cgResourceTypes.h for information relating to the individual       //
//        description structures that are used to initialize these blocks.   //
//        (DX9 implementation).                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX9STATEBLOCKS_H_ )
#define _CGE_CGDX9STATEBLOCKS_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9StateBlocks Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgStateBlocks.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct IDirect3DStateBlock9;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {90D335A7-770B-45B1-9426-A5CA93F50FF8}
const cgUID RTID_DX9SamplerStateResource      = {0x90D335A7, 0x770B, 0x45B1, {0x94, 0x26, 0xA5, 0xCA, 0x93, 0xF5, 0x0F, 0xF8}};
// {552023E0-0842-447B-91BA-4B98CBB010C8}
const cgUID RTID_DX9DepthStencilStateResource = {0x552023E0, 0x0842, 0x447B, {0x91, 0xBA, 0x4B, 0x98, 0xCB, 0xB0, 0x10, 0xC8}};
// {35316C70-B991-4000-9773-347D7F05B993}
const cgUID RTID_DX9RasterizerStateResource   = {0x35316C70, 0xB991, 0x4000, {0x97, 0x73, 0x34, 0x7D, 0x7F, 0x05, 0xB9, 0x93}};
// {4E040028-82C2-4156-A889-F637D925BFEA}
const cgUID RTID_DX9BlendStateResource        = {0x4E040028, 0x82C2, 0x4156, {0xA8, 0x89, 0xF6, 0x37, 0xD9, 0x25, 0xBF, 0xEA}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9SamplerState (Class)
/// <summary>
/// State block object designed to maintain a set of values which represent the 
/// state of an individual DX9 device sampler. See cgSamplerStateDesc for 
/// details on the state values which can be supplied.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9SamplerState : public cgSamplerState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9SamplerState( cgUInt32 referenceId, const cgSamplerStateDesc & states );
    virtual ~cgDX9SamplerState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DStateBlock9  * getD3DStateBlock    ( cgUInt32 samplerIndex );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual void            deviceLost          ( );
    virtual void            deviceRestored      ( );
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9SamplerStateResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                    createStateBlock    ( cgUInt32 samplerIndex );
    void                    releaseStateBlocks  ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    IDirect3DStateBlock9  * mStateBlocks[16];   // D3D9 specific state block object(s)
};

//-----------------------------------------------------------------------------
//  Name : cgDX9DepthStencilState (Class)
/// <summary>
/// State block object designed to maintain a set of values which represent the 
/// DX9 depth stencil buffering features. See cgDepthStencilStateDesc for 
/// details on the state values which can be supplied.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9DepthStencilState : public cgDepthStencilState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9DepthStencilState( cgUInt32 referenceId, const cgDepthStencilStateDesc & states );
    virtual ~cgDX9DepthStencilState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DStateBlock9  * getD3DStateBlock    ( bool frontCounterClockwise );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual void            deviceLost          ( );
    virtual void            deviceRestored      ( );
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9DepthStencilStateResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                    createStateBlock    ( bool frontCounterClockwise );
    void                    releaseStateBlocks  ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    IDirect3DStateBlock9  * mStateBlocks[2];    // D3D9 specific state block object(s)
};

//-----------------------------------------------------------------------------
//  Name : cgDX9RasterizerState (Class)
/// <summary>
/// State block object designed to maintain a set of values which represent the 
/// DX9 depth stencil buffering features. See cgRasterizerStateDesc for 
/// details on the state values which can be supplied.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9RasterizerState : public cgRasterizerState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9RasterizerState( cgUInt32 referenceId, const cgRasterizerStateDesc & states );
    virtual ~cgDX9RasterizerState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DStateBlock9  * getD3DStateBlock    ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual void            deviceLost          ( );
    virtual void            deviceRestored      ( );
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9RasterizerStateResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                    createStateBlock    ( );
    void                    releaseStateBlock   ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    IDirect3DStateBlock9  * mStateBlock;    // D3D9 specific state block object
};

//-----------------------------------------------------------------------------
//  Name : cgDX9BlendState (Class)
/// <summary>
/// State block object designed to maintain a set of values which represent the 
/// DX9 depth stencil buffering features. See cgBlendStateDesc for 
/// details on the state values which can be supplied.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9BlendState : public cgBlendState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9BlendState( cgUInt32 referenceId, const cgBlendStateDesc & states );
    virtual ~cgDX9BlendState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DStateBlock9  * getD3DStateBlock    ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual void            deviceLost          ( );
    virtual void            deviceRestored      ( );
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9BlendStateResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                    createStateBlock    ( );
    void                    releaseStateBlock   ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    IDirect3DStateBlock9  * mStateBlock;    // D3D9 specific state block object
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9STATEBLOCKS_H_