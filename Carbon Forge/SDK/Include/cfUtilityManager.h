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
// File: cfUtilityManager.h                                                  //
//                                                                           //
// Desc: Contains classes which provide a database through which any         //
//       utility extensions can be registered / triggered. Utilities are     //
//       essentially custom processors designed to perform some 'one off'    //
//       scene related task such as compiling effects, lightmaps, etc.       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfUtilityManager Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

// CGE Includes
#include <System/cgEventDispatcher.h>

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    class cfBaseUtility;
    class cfWorldDoc;

    //-------------------------------------------------------------------------
    // Delegate Definitions
    //-------------------------------------------------------------------------
    struct UtilityEventArgs
    {
        UtilityEventArgs( const cgUID & _Identifier ) : Identifier(_Identifier) {}
        cgUID Identifier;
    };
    
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //  Name : cfUtilityManagerEventListener (Class)
    /// <summary>
    /// Abstract interface class from which other classes can derive in order 
    /// to recieve messages whenever utility manager events occur.
    /// </summary>
    //-----------------------------------------------------------------------------
    class cfUtilityManagerEventListener : public cgEventListener
    {
    public:
        //-------------------------------------------------------------------------
        // Public Virtual Methods
        //-------------------------------------------------------------------------
        virtual void    OnUtilityAdded  ( UtilityEventArgs * e ) {}
        virtual void    OnUtilityRemoved( UtilityEventArgs * e ) {}
    };

    //-------------------------------------------------------------------------
    // Name : cfUtilityManager (Class)
    /// <summary>
    /// Responsible for maintaining a list of available utility classes
    /// and / or extensions that can be used to perform secondary scene related 
    /// tasks.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfUtilityManager : public cgEventDispatcher
	{
	public:
        //---------------------------------------------------------------------
        // Typedefs, Structures & Enumerators
        //---------------------------------------------------------------------
        typedef cfBaseUtility* (*UtilityAllocFunc)( const cgUID & Identifier, cfWorldDoc * pDocument );

        /// <summary>Structure responsible for storing information about a unique utility.</summary>
        struct UtilityDesc
        {
            /// <summary>Globally unique identifier for this utility.</summary>
            cgUID               Identifier;
            /// <summary>Category into which this utility's menu entry should be placed (empty for none).</summary>
            cgString            Category;
            /// <summary>Name of the utility as it will appear in the editor's Tools menu.</summary>
            cgString            Name;
            /// <summary>Allocation callback for this utility. Called just prior to execution to instantiate the required class.</summary>
            UtilityAllocFunc    pAllocFunc;
            /// <summary>Any supplied context object that will be available to the utility.</summary>
            void              * pContext;
            
        }; // End Struct UtilityDesc

        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
         cfUtilityManager( );
        ~cfUtilityManager( );

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        const UtilityDesc     * GetUtilityData      ( const cgUID & Identifier ) const;
        cfWorldDoc            * GetDocument         ( ) const;
        void                    SetDocument         ( cfWorldDoc * pDocument );
        void                    RegisterUtility     ( const cgUID & Identifier, UtilityAllocFunc pAllocFunc, void * pContext, const cgString & Name, const cgString & Category /* = cgString::Empty */ );
        void                    UnregisterUtility   ( const cgUID & Identifier );
        bool                    LaunchUtility       ( const cgUID & Identifier );
        
    protected:
        //---------------------------------------------------------------------
        // Protected Typedefs
        //---------------------------------------------------------------------
        CGE_MAP_DECLARE( cgUID, UtilityDesc, UtilityDescMap )

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>Collection of utility descriptors using the identifier as key.</summary>
        UtilityDescMap      m_Utilities;
        /// <summary>Document on which the tools may currently operate (if any).</summary>
        cfWorldDoc        * m_pDocument;
        
    }; // End Class cfUtilityManager

} // End Namespace CarbonForge