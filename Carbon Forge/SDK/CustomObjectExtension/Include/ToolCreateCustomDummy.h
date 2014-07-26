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
// File: ToolCreateCustomDummy.h                                             //
//                                                                           //
// Desc: Classes responsible for creating a simple dummy frame and adding it //
//       to the scene based on user interaction with the render viewports.   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfToolCreateDummy Header Includes
//-----------------------------------------------------------------------------
#include "cfBaseTool.h"

// CGE Includes
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Global Forward Declarations
//-----------------------------------------------------------------------------
class CustomDummyNode;

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Globally Unique Type ID(s)
    //-------------------------------------------------------------------------
    // {88C90F76-4052-451D-A40B-A9AE79588624}
    const cgUID TMID_CreateCustomDummy = { 0x88C90F76, 0x4052, 0x451D, { 0xA4, 0x0B, 0xA9, 0xAE, 0x79, 0x58, 0x86, 0x24 } };

    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    class cfRenderViewport;

    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : ToolCreateCustomDummy (Class)
    /// <summary>
    /// Tool class responsible for creating a simple dummy frame / object.
    /// </summary>
    //-------------------------------------------------------------------------
    class ToolCreateCustomDummy : public cfBaseToolMode
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
                 ToolCreateCustomDummy( );
        virtual ~ToolCreateCustomDummy( );

        //---------------------------------------------------------------------
        // Public Static Functions
        //---------------------------------------------------------------------
        static cfBaseToolMode * Allocate    ( );

        //---------------------------------------------------------------------
        // Public Virtual Methods (Overrides cfBaseToolMode)
        //---------------------------------------------------------------------
        virtual bool OnMouseDown        ( cgUInt8 nButtons, const cgPoint & Position, cgUInt32 nModifiers, cfRenderViewport * pViewport );
        virtual bool OnMouseMove        ( cgUInt8 nButtons, const cgPoint & Position, const cgPoint & PrevCursorPos, cgUInt32 nModifiers, cfRenderViewport * pViewport, bool & bWrapCursor );
        virtual bool OnMouseUp          ( cgUInt8 nButtons, const cgPoint & Position, const cgPoint & PrevCursorPos, cgUInt32 nModifiers, cfRenderViewport * pViewport );

    protected:
        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>Boolean denoting whether or not we are currently in the process of creating an object.</summary>
        bool                m_bCreating;
        /// <summary>Which step of the creation process are we on (0 = initial click etc.)</summary>
        cgInt32             m_nCurrentStep;
        /// <summary>The new dummy object being created (if any)</summary>
        CustomDummyNode   * m_pDummy;
        /// <summary>The world space positions at which clicks occured.</summary>
        cgVector3           m_vOrigin, m_vBaseEnd;
        /// <summary>User clicked and dragged on the first click?</summary>
        bool                m_bDragged;
        
    }; // End Class ToolCreateCustomDummy

} // End Namespace CarbonForge