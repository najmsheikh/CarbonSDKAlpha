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
// Name : cgResampleChain.h                                                  //
//                                                                           //
// Desc : Provides basic support for managing render target chains.          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRESAMPLECHAIN_H_ )
#define _CGE_CGRESAMPLECHAIN_H_

//-----------------------------------------------------------------------------
// cgResampleChain Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgResourceHandles.h>
#include <Rendering/cgSampler.h>

//-----------------------------------------------------------------------------
// cgResampleChain Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderView;

//-----------------------------------------------------------------------------
// Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgResampleChain (Class)
/// <summary>
/// Helper class used to maintain a chain of render targets for the purposes
/// of resampling or other operations.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgResampleChain : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgResampleChain, "ResampleChain" )

public:
    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 DefaultDownSampleLevels = 0;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgResampleChain();
    virtual ~cgResampleChain();

    //-------------------------------------------------------------------------
    // Public Functions
    //-------------------------------------------------------------------------
    bool                        setSource           ( const cgRenderTargetHandle & texture );
    bool                        setSource           ( const cgRenderTargetHandle & texture, cgUInt32 levels );
    bool                        setSource           ( const cgRenderTargetHandle & texture, const cgString & instanceId );
    bool                        setSource           ( cgRenderView * view, const cgRenderTargetHandle & texture, const cgString & instanceId );
    bool                        setSource           ( cgRenderView * view, const cgRenderTargetHandle & texture, cgUInt32 levels = DefaultDownSampleLevels, const cgString & instanceId = cgString::Empty );
    void                        setLevel            ( cgUInt32 level, const cgRenderTargetHandle & texture );
    void                        restoreLevel        ( cgUInt32 level );
    void                        restoreLevels       ( );
    cgUInt32                    getLevelCount       ( ) const; 
    cgInt32                     getLevelIndex       ( cgUInt32 width, cgUInt32 height );
    const cgRenderTargetHandle &getLevel            ( cgUInt32 level ) const; 
	bool						hasOddDimensions    ( cgUInt32 level ) const;

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgUInt32             computeLevelCount   ( cgUInt32 width, cgUInt32 height );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>The chain of render target textures based on the top level texture.</summary>
    cgRenderTargetHandleArray   mChain;                      
    /// <summary>The level count requested the last time the caller set a new texture.</summary>
    cgUInt32                    mLastLevelCountRequest;      
    /// <summary>Backup handles for levels that the user temporarily swapped out with their own targets.</summary>
    cgRenderTargetHandleArray   mLevelCache;                      
};

//-----------------------------------------------------------------------------
//  Name : cgResampleChainMRT (Class)
/// <summary>
/// Helper class used to maintain a chain of (multiple) render targets for the 
/// purposes of resampling or other operations.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgResampleChainMRT : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgResampleChainMRT, "ResampleChainMRT" )

public:

    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 DefaultDownSampleLevels = 0;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgResampleChainMRT();
             cgResampleChainMRT( cgInt32 numTargets );
    virtual ~cgResampleChainMRT();

    //-------------------------------------------------------------------------
    // Public Functions
    //-------------------------------------------------------------------------
    bool						 setSource        ( const cgRenderTargetHandleArray & textures );
    bool						 setSource        ( const cgRenderTargetHandleArray & textures, cgUInt32 levels );
    bool						 setSource        ( const cgRenderTargetHandleArray & textures, const cgString & instanceId );
    bool						 setSource        ( cgRenderView * view, const cgRenderTargetHandleArray & textures, const cgString & instanceId );
    bool						 setSource        ( cgRenderView * view, const cgRenderTargetHandleArray & textures, cgUInt32 levels = DefaultDownSampleLevels, const cgString & instanceId = cgString::Empty );
	void						 setLevel         ( cgUInt32 level, const cgRenderTargetHandleArray & textures );
    void						 restoreLevel     ( cgUInt32 level );
    void						 restoreLevels    ( );
	cgUInt32                     getLevelCount    ( ) const; 
    cgInt32                      getLevelIndex    ( cgUInt32 width, cgUInt32 height );
    cgRenderTargetHandleArray    getLevel         ( cgUInt32 level ); 
	void				         allocateChain    ( cgUInt32 targets );
    void                         clearChain       ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void				 dispose         ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
	CGE_VECTOR_DECLARE( cgResampleChain *, ChainArray )
	
	ChainArray mResampleChains;
};

#endif // !_CGE_CGRESAMPLECHAIN_H_
