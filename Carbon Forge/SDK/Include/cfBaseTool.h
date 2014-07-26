//---------------------------------------------------------------------------//
//         ____           _                   _____                          //
//        / ___|__ _ _ __| |__   ___  _ __   |  ___|__  _ __ __ _  ___       //
//       | |   / _` | '__| '_ \ / _ \| '_ \  | |_ / _ \| '__/ _` |/ _ \      //
//       | |__| (_| | |  | |_) | (_) | | | | |  _| (_) | | | (_| |  __/      //
//        \____\__,_|_|  |_.__/ \___/|_| |_| |_|  \___/|_|  \__, |\___|      //
//                   Game Institute - Carbon Engine Sandbox |___/            //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// File: cfBaseTool.h                                                        //
//                                                                           //
// Desc: Contains base class interfaces through which new tools can be       //
//       implemented and accessed by the application. Tools might include    //
//       creating primitives, new objects or custom picking behavior.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfBaseTool Header Includes
//-----------------------------------------------------------------------------
#include "cfToolManager.h"

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Namespace Promotion
    //-------------------------------------------------------------------------
    using namespace System;
    using namespace System::Drawing;
    using namespace System::Windows::Forms;

    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    class cfRenderViewport;
    class cfWorldDoc;
    
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cfBaseToolMode (Base Class)
    /// <summary>
    /// Base interface through which tool mode types can be implemented and 
    /// accessed.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfBaseToolMode
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
                 cfBaseToolMode( );
        virtual ~cfBaseToolMode( );

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        const cfToolManager::ToolMode * GetModeData         ( ) const;
        void                            SetModeData         ( const cfToolManager::ToolMode * pData );
        cfWorldDoc                    * GetDocument         ( ) const;
        void                            SetDocument         ( cfWorldDoc * pDocument );
        cfToolManager                 * GetParent           ( ) const;
        void                            SetParent           ( cfToolManager * pParent );
        cfRenderViewport              * GetActiveViewport   ( ) const;
        void                            SetActiveViewport   ( cfRenderViewport * pViewport );
        
        //---------------------------------------------------------------------
        // Public Virtual Methods
        //---------------------------------------------------------------------
        virtual bool OnMouseDown        ( cgUInt8 nButtons, const cgPoint & Position, cgUInt32 nModifiers, cfRenderViewport * pViewport );
        virtual bool OnMouseMove        ( cgUInt8 nButtons, const cgPoint & Position, const cgPoint & PrevCursorPos, cgUInt32 nModifiers, cfRenderViewport * pViewport, bool & bWrapCursor );
        virtual bool OnMouseUp          ( cgUInt8 nButtons, const cgPoint & Position, const cgPoint & PrevCursorPos, cgUInt32 nModifiers, cfRenderViewport * pViewport );
        virtual bool OnSetCursor        ( cfRenderViewport * pViewport );
        virtual void OnTick             ( cgFloat fTimeElapsed );
		virtual void OnDrawPreClear     ( cfRenderViewport * pViewport );
        virtual void OnDrawPostClear    ( cfRenderViewport * pViewport );
        virtual void OnInit             ( );
        virtual void OnRelease          ( );

    protected:
        //---------------------------------------------------------------------
        // Protected Methods
        //---------------------------------------------------------------------
        
        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>Tool data object associated with this tool (if any).</summary>
        const cfToolManager::ToolMode * m_pModeData;
        /// <summary>Document on which this tool is currently operating (if any).</summary>
        cfWorldDoc                    * m_pDocument;
        /// <summary>Parent tool modes container.</summary>
        cfToolManager                 * m_pParent;
        /// <summary>Viewport that the tool was most recently associated with (cursor over).</summary>
        cfRenderViewport              * m_pActiveViewport;

    }; // End Class cfBaseToolMode

} // End Namespace CarbonForge