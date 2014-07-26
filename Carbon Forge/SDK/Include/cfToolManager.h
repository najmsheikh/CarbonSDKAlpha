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
// File: cfToolManager.h                                                     //
//                                                                           //
// Desc: Contains classes which provide a database through which the myriad  //
//       different states that the application can currently be in can be    //
//       managed (i.e. Select, Create Box, Create Light and so on.)          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfToolManager Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

// CGE Includes
#include <System/cgEventDispatcher.h>
#include <Input/cgInputTypes.h>

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Namespace Promotion
    //-------------------------------------------------------------------------
    using namespace System;
    using namespace System::Windows::Forms;

    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    ref class cfUIPropertyExplorer;
    class cfRenderViewport;
    class cfBaseToolMode;
    class cfBaseToolAction;
    class cfWorldDoc;

    //-------------------------------------------------------------------------
    // Delegate Definitions
    //-------------------------------------------------------------------------
    struct ToolModeChangeEventArgs
    {
        ToolModeChangeEventArgs( const cgUID & _OldMode, const cgUID & _NewMode )
        {
            OldMode = _OldMode;
            NewMode = _NewMode;
        }
        cgUID OldMode, NewMode;
    };

    struct ToolActionTriggerEventArgs
    {
        ToolActionTriggerEventArgs( const cgUID & _Action )
        {
            Action = _Action;
        }
        cgUID Action;
    };
    
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //  Name : cfToolManagerEventListener (Class)
    /// <summary>
    /// Abstract interface class from which other classes can derive in order 
    /// to recieve messages whenever tool mode events occur.
    /// </summary>
    //-----------------------------------------------------------------------------
    class cfToolManagerEventListener : public cgEventListener
    {
    public:
        //-------------------------------------------------------------------------
        // Public Virtual Methods
        //-------------------------------------------------------------------------
        virtual void    OnToolModeChange    ( ToolModeChangeEventArgs * e ) {}
        virtual void    OnToolActionTrigger ( ToolActionTriggerEventArgs * e ) {}
    };

    //-------------------------------------------------------------------------
    // Name : cfToolManager (Class)
    /// <summary>
    /// Responsible for maintaining a list of available tool modes, as well as
    /// managing the application's current state.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfToolManager : public cgEventDispatcher
	{
	public:
        //---------------------------------------------------------------------
        // Typedefs, Structures & Enumerators
        //---------------------------------------------------------------------
        struct ToolAction;
        typedef cfBaseToolMode  * (*ToolModeAllocFunc)( void );
        typedef bool (*ToolActionExecuteFunc)( cfToolManager * pManager, cfWorldDoc * pDocument, const ToolAction * pData );
        typedef bool (*ToolActionAvailabilityFunc)( cfToolManager * pManager, cfWorldDoc * pDocument, const ToolAction * pData );

        /// <summary>Structure responsible for storing information about a unique tool mode.</summary>
        struct ToolMode
        {
            /// <summary>Category to which this tool type is constrained.</summary>
            cgUID               Category;
            /// <summary>Globally unique identifier for this mode.</summary>
            cgUID               Identifier;
            /// <summary>Name of the mode (i.e. Create Spotlight).</summary>
            cgString            Name;
            /// <summary>Name of the listener / tool object to create when activated (including namespace in the form "Namespace.Class").</summary>
            ToolModeAllocFunc   pAllocFunc;
            /// <summary>Any supplied context object that will be available to the tool.</summary>
            void              * pContext;
            /// <summary>The control -- if any -- which the user selects to enable this mode.</summary>
            gcrootx<Object^>    oSelectionControl;
            /// <summary>Object to be displayed in the application property grid relating to this tool (object stays resident in order to maintain options).</summary>
            gcrootx<Object^>    oToolOptions;
        
        }; // End Struct ToolMode

        /// <summary>Structure responsible for storing information about a unique tool action.</summary>
        struct ToolAction
        {
            /// <summary>Category to which this tool type is constrained.</summary>
            cgUID                       Category;
            /// <summary>Globally unique identifier for this mode.</summary>
            cgUID                       Identifier;
            /// <summary>Name of the mode (i.e. Create Spotlight).</summary>
            cgString                    Name;
            /// <summary>Callback function used to determine if the action is available.</summary>
            ToolActionAvailabilityFunc  pAvailabilityFunc;
            /// <summary>Callback function to be triggered when the action is executed.</summary>
            ToolActionExecuteFunc       pExecuteFunc;
            /// <summary>Any supplied context object that will be available to the tool.</summary>
            void                      * pContext;
            /// <summary>The control -- if any -- which the user selects to trigger this action.</summary>
            gcrootx<Object^>            oSelectionControl;
        
        }; // End Struct ToolAction

        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
         cfToolManager( );
        ~cfToolManager( );

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        cfWorldDoc            * GetDocument                 ( ) const;
        void                    SetDocument                 ( cfWorldDoc * pDocument );
        void                    ResetMode                   ( );
        void                    RestorePreviousMode         ( );
        gcrootx<cfUIPropertyExplorer^> GetPropertyExplorer  ( ) const;
        void                    SetPropertyExplorer         ( gcrootx<cfUIPropertyExplorer^> oProperties );
        gcrootx<Object^>        GetSenderControl            ( ) const;
        void                    SetSenderControl            ( gcrootx<Object^> oComponent );
        void                    SetCategoryConstraint       ( const cgUID & Type );
        const cgUID           & GetCategoryConstraint       ( ) const;
        
        // Modes
        void                    AddMode                     ( const cgUID & Identifier, ToolModeAllocFunc pAllocFunc, void * pContext, const cgString & Name, gcrootx<Object^> SelectionControl );
        void                    AddMode                     ( const cgUID & Identifier, ToolModeAllocFunc pAllocFunc, void * pContext, const cgString & Name, gcrootx<Object^> SelectionControl, gcrootx<Object^> ToolOptions );
        void                    AddMode                     ( const cgUID & Identifier, const cgUID & Category, ToolModeAllocFunc pAllocFunc, void * pContext, const cgString & Name, gcrootx<Object^> SelectionControl );
        void                    AddMode                     ( const cgUID & Identifier, const cgUID & Category, ToolModeAllocFunc pAllocFunc, void * pContext, const cgString & Name, gcrootx<Object^> SelectionControl, gcrootx<Object^> ToolOptions );
        void                    RemoveMode                  ( const cgUID & Identifier );
        void                    RemoveMode                  ( const cgUID & Identifier, const cgUID & Category );
        const ToolMode        * GetModeData                 ( const cgUID & Identifier ) const;
        const ToolMode        * GetModeData                 ( const cgUID & Identifier, const cgUID & Category ) const;
        const cgUID           & GetSelectedMode             ( ) const;
        void                    SetSelectedMode             ( const cgUID & Identifier );
        const cgUID           & GetDefaultMode              ( ) const;
        void                    SetDefaultMode              ( const cgUID & Identifier );
        cfBaseToolMode        * GetSelectedTool             ( ) const;
        
        // Actions
        void                    AddAction                   ( const cgUID & Identifier, ToolActionAvailabilityFunc pAvailabilityFunc, ToolActionExecuteFunc pExecuteFunc, void * pContext, const cgString & Name, gcrootx<Object^> SelectionControl );
        void                    AddAction                   ( const cgUID & Identifier, const cgUID & Category, ToolActionAvailabilityFunc pAvailabilityFunc, ToolActionExecuteFunc pExecuteFunc, void * pContext, const cgString & Name, gcrootx<Object^> SelectionControl );
        void                    RemoveAction                ( const cgUID & Identifier );
        void                    RemoveAction                ( const cgUID & Identifier, const cgUID & Category );
        const ToolAction      * GetActionData               ( const cgUID & Identifier ) const;
        const ToolAction      * GetActionData               ( const cgUID & Identifier, const cgUID & Category ) const;
        bool                    ExecuteAction               ( const cgUID & Identifier );
        bool                    IsActionAvailable           ( const cgUID & Identifier );
        void                    SetActionControl            ( const cgUID & Identifier, gcrootx<Object^> oComponent );

        // Notifications
        bool                    NotifyMouseDown             ( cgUInt8 nButtons, const cgPoint & Position, cgUInt32 nModifiers, cfRenderViewport * pViewport );
        bool                    NotifyMouseMove             ( cgUInt8 nButtons, const cgPoint & Position, const cgPoint & PrevCursorPos, cgUInt32 nModifiers, cfRenderViewport * pViewport, bool & bWrapCursor );
        bool                    NotifyMouseUp               ( cgUInt8 nButtons, const cgPoint & Position, const cgPoint & PrevCursorPos, cgUInt32 nModifiers, cfRenderViewport * pViewport );
        void                    NotifyDraw                  ( cfRenderViewport * pViewport, bool bPostClear );
        void                    NotifyTick                  ( cgFloat fTimeElapsed );
        bool                    NotifySetCursor             ( cfRenderViewport * pViewport );
        

    protected:
        //---------------------------------------------------------------------
        // Protected Typedefs
        //---------------------------------------------------------------------
        CGE_MAP_DECLARE( cgUID, ToolMode, ToolModeMap )
        CGE_MAP_DECLARE( cgUID, ToolModeMap, ConstraintModeMap )
        CGE_MAP_DECLARE( cgUID, ToolAction, ToolActionMap )
        CGE_MAP_DECLARE( cgUID, ToolActionMap, ConstraintActionMap )

        //---------------------------------------------------------------------
        // Protected Methods
        //---------------------------------------------------------------------
        void            SetControlChecked   ( gcrootx<Object^> oControl, bool bChecked );
        void            SetControlEnabled   ( gcrootx<Object^> oControl, bool bEnabled );

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>Current category type tool constraint. This dictates the types of tools which are available / selected.</summary>
        cgUID                           m_CategoryConstraint;
        /// <summary>The default mode to select when switching to default state, or an invalid mode is specified.</summary>
        cgUID                           m_DefaultMode;
        /// <summary>The currently selected tool mode.</summary>
        cgUID                           m_SelectedMode;
        /// <summary>The mode previously selected prior to the most recent.</summary>
        cgUID                           m_PreviousMode;
        /// <summary>Collection of ToolMode objects using the constraint identifier as top level key, and the tool identifier itself as the secondary.</summary>
        ConstraintModeMap               m_ToolModes;
        /// <summary>Collection of ToolAction objects using the constraint identifier as top level key, and the tool identifier itself as the secondary.</summary>
        ConstraintActionMap             m_ToolActions;
        /// <summary>An instance of the tool class associated with the selected mode (if any).</summary>
        cfBaseToolMode                * m_pSelectedTool;
        /// <summary>Document on which the tools may currently operate (if any).</summary>
        cfWorldDoc                    * m_pDocument;
        /// <summary>The property explorer control in which to display tool options.</summary>
        gcrootx<cfUIPropertyExplorer^>* m_pPropertyExplorer;
        /// <summary>Sender control to select if none is available in the mode description.</summary>
        gcrootx<Object^>              * m_pSenderControl;
        /// <summary>Previously selected control.</summary>
        gcrootx<Object^>              * m_pSelectedControl;
        
    }; // End Class cfToolManager

} // End Namespace CarbonForge