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
// Name : cgDX11StateBlocks.h                                                //
//                                                                           //
// Desc : Contains classes that represent the various state blocks that can  //
//        be applied to the render driver / hardware. Such states include    //
//        sampler, rasterizer, depth stencil and blend state objects. See    //
//        cgResourceTypes.h for information relating to the individual       //
//        description structures that are used to initialize these blocks.   //
//        (DX11 implementation).                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11STATEBLOCKS_H_ )
#define _CGE_CGDX11STATEBLOCKS_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11StateBlocks Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgStateBlocks.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5C498053-7DAD-4A57-8BCF-E36A35232770}
const cgUID RTID_DX11SamplerStateResource      = {0x5C498053, 0x7DAD, 0x4A57, {0x8B, 0xCF, 0xE3, 0x6A, 0x35, 0x23, 0x27, 0x70}};
// {D8A61517-5A88-4DDA-8EF8-0BB7D3BD36E3}
const cgUID RTID_DX11DepthStencilStateResource = {0xD8A61517, 0x5A88, 0x4DDA, {0x8E, 0xF8, 0xB, 0xB7, 0xD3, 0xBD, 0x36, 0xE3}};
// {3D67D6B9-9B09-493D-9128-DCD1BF68CCBD}
const cgUID RTID_DX11RasterizerStateResource   = {0x3D67D6B9, 0x9B09, 0x493D, {0x91, 0x28, 0xDC, 0xD1, 0xBF, 0x68, 0xCC, 0xBD}};
// {DE731D2B-7671-4B9B-A277-09BEF801DE5A}
const cgUID RTID_DX11BlendStateResource        = {0xDE731D2B, 0x7671, 0x4B9B, {0xA2, 0x77, 0x9, 0xBE, 0xF8, 0x1, 0xDE, 0x5A}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11SamplerState (Class)
/// <summary>
/// State block object designed to maintain a set of values which represent the 
/// state of an individual DX11 device sampler. See cgSamplerStateDesc for 
/// details on the state values which can be supplied.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11SamplerState : public cgSamplerState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11SamplerState( cgUInt32 referenceId, const cgSamplerStateDesc & states );
    virtual ~cgDX11SamplerState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11SamplerState    * getD3DStateBlock    ( );

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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX11SamplerStateResource; }
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
    ID3D11SamplerState* mStateBlock;    // D3D11 specific state block object
};

//-----------------------------------------------------------------------------
//  Name : cgDX11DepthStencilState (Class)
/// <summary>
/// State block object designed to maintain a set of values which represent the 
/// DX11 depth stencil buffering features. See cgDepthStencilStateDesc for 
/// details on the state values which can be supplied.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11DepthStencilState : public cgDepthStencilState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11DepthStencilState( cgUInt32 referenceId, const cgDepthStencilStateDesc & states );
    virtual ~cgDX11DepthStencilState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11DepthStencilState   * getD3DStateBlock    ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual void                deviceLost          ( );
    virtual void                deviceRestored      ( );
    virtual bool                loadResource        ( );
    virtual bool                unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType    ( ) const { return RTID_DX11DepthStencilStateResource; }
    virtual bool                queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose             ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                        createStateBlock    ( );
    void                        releaseStateBlock   ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    ID3D11DepthStencilState   * mStateBlock;    // D3D11 specific state block object(s)
};

//-----------------------------------------------------------------------------
//  Name : cgDX11RasterizerState (Class)
/// <summary>
/// State block object designed to maintain a set of values which represent the 
/// DX11 depth stencil buffering features. See cgRasterizerStateDesc for 
/// details on the state values which can be supplied.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11RasterizerState : public cgRasterizerState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11RasterizerState( cgUInt32 referenceId, const cgRasterizerStateDesc & states );
    virtual ~cgDX11RasterizerState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11RasterizerState * getD3DStateBlock    ( );

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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX11RasterizerStateResource; }
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
    ID3D11RasterizerState * mStateBlock;    // D3D11 specific state block object
};

//-----------------------------------------------------------------------------
//  Name : cgDX11BlendState (Class)
/// <summary>
/// State block object designed to maintain a set of values which represent the 
/// DX11 depth stencil buffering features. See cgBlendStateDesc for 
/// details on the state values which can be supplied.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11BlendState : public cgBlendState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11BlendState( cgUInt32 referenceId, const cgBlendStateDesc & states );
    virtual ~cgDX11BlendState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11BlendState      * getD3DStateBlock    ( );

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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX11BlendStateResource; }
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
    ID3D11BlendState  * mStateBlock;    // D3D11 specific state block object
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11STATEBLOCKS_H_