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
// Name : cgObjectRenderQueue.h                                              //
//                                                                           //
// Desc : Utility class used for the automatic rendering of object nodes     //
//        in a defined, or sorted order including the ability to batch by    //
//        material and control the rendering behavior through scripted       //
//        render control "techniques".                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGOBJECTRENDERQUEUE_H_)
#define _CGE_CGOBJECTRENDERQUEUE_H_

//-----------------------------------------------------------------------------
// cgObjectRenderQueue Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;
class cgVisibilitySet;
class cgFilterExpression;

//-----------------------------------------------------------------------------
// Global Enumerations
//-----------------------------------------------------------------------------
namespace cgQueueProcessHandler
{
    enum Base
    {
        Default,
        DepthSortedBlending
    };

} // End Namespace : cgQueueProcessHandler

namespace cgQueueMaterialHandler
{
    enum Base
    {
        Default,
        Collapse
    };

} // End Namespace : cgQueueMaterialHandler

namespace cgQueueLightingHandler
{
    enum Base
    {
        None,
        Default
    };

} // End Namespace : cgQueueLightingHandler

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgObjectRenderQueue (Class)
/// <summary>
/// Utility class used for the automatic rendering of object nodes in a 
/// defined, or sorted order including the ability to batch by material and 
/// control the rendering behavior through scripted render control "techniques"
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgObjectRenderQueue : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgObjectRenderQueue, "ObjectRenderQueue" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgObjectRenderContext;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgObjectRenderQueue( cgScene * scene );
    virtual ~cgObjectRenderQueue( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // Queue population
    void            begin                   ( cgVisibilitySet * visibilityData );
    void            begin                   ( cgVisibilitySet * visibilityData, cgQueueProcessHandler::Base process );
    void            end                     ( );
    void            end                     ( bool flushQueue );
    void            clear                   ( );
    
    void            renderClass             ( const cgString & className );
    void            renderClass             ( const cgString & className, const cgString & callback );
    void            renderClass             ( const cgString & className, cgQueueMaterialHandler::Base materialHandler );
    void            renderClass             ( const cgString & className, cgQueueMaterialHandler::Base materialHandler, const cgString & callback );
    void            renderClass             ( const cgString & className, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler );
    void            renderClass             ( const cgString & className, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler, const cgString & callback );

    // Setup
    void            setCollapseMaterial     ( const cgMaterialHandle & material );
    void            setMaterialFilter       ( cgFilterExpression * filter );
    bool            setMaterialFilter       ( const cgString & expression );

    // Execution
    void            flush                   ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    /*struct Batch
    {
        cgQueueLightingHandler::Base    lightingHandler;
        cgQueueMaterialHandler::Base    materialHandler;
    };
    CGE_LIST_DECLARE( Batch, BatchList );

    struct Process
    {
        cgQueueProcessHandler::Base ProcessHandler;
        //BatchList                   Batches;
    };
    CGE_LIST_DECLARE( Process, ProcessList );*/

    CGE_LIST_DECLARE( cgObjectRenderContext*, ContextList )


    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            renderClassDefault              ( cgUInt32 classId, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler, const cgString& callback );
    void            renderClassDepthSortedBlending  ( cgUInt32 classId, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler, const cgString& callback );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgScene                       * mScene;                     // Scene from which nodes are being rendered.
    bool                            mPopulating;                // The queue is in the process of being populated.
    cgQueueProcessHandler::Base     mCurrentProcessHandler;
    cgVisibilitySet               * mCurrentVisibilitySet;
    //cgVisibilitySet               * mFilteredVisibilitySet;
    cgFilterExpression            * mCurrentMaterialFilter;     // The filter expression to use for excluding material types (if any).
    cgMaterialHandle                mCollapseMaterial;          // Material used in the case of 'cgQueueMaterialHandler::Collapse'
    ContextList                     mContextList;               // List of currently active render contexts.
};

#endif // !_CGE_CGOBJECTRENDERQUEUE_H_