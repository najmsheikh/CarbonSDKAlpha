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
// Name : cgObjectRenderContext.h                                            //
//                                                                           //
// Desc : When rendering sections of the node render queue, render control   //
//        script side "techniques" can be supplied that customize the        //
//        necessary rendering behaviors. The context class is provided to    //
//        these 'techniques' as a means of describing the objects currently  //
//        being processed, as well as supplying functionality to operate on  //
//        them (i.e. culling, rendering, etc.)                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGOBJECTRENDERCONTEXT_H_)
#define _CGE_CGOBJECTRENDERCONTEXT_H_

//-----------------------------------------------------------------------------
// cgObjectRenderContext Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>
#include <Rendering/cgObjectRenderQueue.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgLightNode;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgObjectRenderContext (Class)
/// <summary>
/// When rendering sections of the object render queue, render control script 
/// side "techniques" can be supplied that customize the necessary rendering 
/// behaviors. The context class is provided to these 'techniques' as a means 
/// of describing the objects currently being processed, as well as supplying 
/// functionality to operate on them (i.e. culling, rendering, etc.)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgObjectRenderContext : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgObjectRenderContext, "ObjectRenderContext" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgObjectRenderQueue;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgObjectRenderContext( cgObjectRenderQueue * queue, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler, const cgString & scriptCallback );
    virtual ~cgObjectRenderContext( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool            step                    ( );
    void            render                  ( );
    void            light                   ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE     ( cgMaterialHandle, cgUInt32, MaterialBatchLUT )
    CGE_MAP_DECLARE     ( cgLightNode*, MaterialBatchLUT, LightBatchMap )
    CGE_VECTOR_DECLARE  ( cgObjectNodeArray, RenderBatchArray )
    CGE_VECTOR_DECLARE  ( cgLightNode*, LightArray );

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            prepare                         ( );
    bool            stepDefault                     ( );
    bool            stepDepthSortedBlending         ( );
    void            renderDefault                   ( const MaterialBatchLUT & materials );
    void            renderDepthSortedBlending       ( );
    void            lightDefault                    ( );
    void            lightDepthSortedBlending        ( );
    void            insertObjectsByMaterial         ( const cgMaterialHandle & material, const cgObjectNodeList & objects );
    void            insertObjectsByLightAndMaterial ( cgLightNode * light, const cgMaterialHandle & material, const cgObjectNodeList & objects );
    void            insertObject                    ( cgObjectNode * object );
    void            insertLight                     ( cgLightNode * light );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgObjectRenderQueue           * mQueue;
    cgQueueLightingHandler::Base    mLightingHandler;
    cgQueueMaterialHandler::Base    mMaterialHandler;
    cgString                        mScriptCallback;
    LightBatchMap                   mLightBatches;
    LightArray                      mLights;
    MaterialBatchLUT                mMaterials;
    RenderBatchArray                mRenderBatches;
    cgMaterialHandle                mCollapseMaterial;
    cgFilterExpression            * mMaterialFilter;
    cgFloat                         mSortKey;

    // Context execution tracking
    cgInt32                         mCurrentStep;
    
};

#endif // !_CGE_CGOBJECTRENDERCONTEXT_H_