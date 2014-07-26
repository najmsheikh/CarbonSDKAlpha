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
// File: cfPropertyManager.h                                                 //
//                                                                           //
// Desc: Contains classes which provide a database through which the myriad  //
//       different property wrappers for the various objects / nodes managed //
//       by the application can be instantiated and selected.                //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfPropertyManager Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

//-----------------------------------------------------------------------------
// Global Forward Declarations
//-----------------------------------------------------------------------------
class cgWorld;

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Namespace Promotion
    //-------------------------------------------------------------------------
    using namespace System;
    using namespace System::Drawing;
    using namespace System::Reflection;

    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    ref class cfUIPropertyExplorer;

    //-------------------------------------------------------------------------
    // Global Structures, Typedefs & Enumerations
    //-------------------------------------------------------------------------
    /// <summary>Structure responsible for storing information about a unique property wrapper.</summary>
    struct cfPropertyWrapperDesc
    {
        /// <summary>Globally unique identifier for the reference type to which this property wrapper applies.</summary>
        cgUID               Identifier;
        /// <summary>Name of the type wrapped by this property wrapper (i.e. Mesh).</summary>
        cgString            Name;
        /// <summary>Name of the property class to create when activated (including namespace in the form "Namespace.Class").</summary>
        cgString            PropertyType;
        /// <summary>The assembly that owns the property class.</summary>
        gcrootx<Assembly^>  PropertyAssembly;
        /// <summary>Image resource for display of the property type in various locations.</summary>
        gcrootx<Image^>     IconImage;
        
    }; // End Struct cfPropertyWrapperDesc

    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cfPropertyManager (Class)
    /// <summary>
    /// Responsible for maintaining and instantiating property wrappers for the
    /// various different object types registered with the system.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfPropertyManager
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
         cfPropertyManager( );
        ~cfPropertyManager( );

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        void                        AddType                     ( const cgUID & Identifier, const cgString & strName, const cgString & strWrapperType, gcrootx<Image^> oIconImage );
        void                        AddType                     ( const cgUID & Identifier, const cgString & strName, const cgString & strWrapperType, gcrootx<Image^> oIconImage, gcrootx<Assembly^> oAssembly );
        void                        RemoveType                  ( const cgUID & Identifier );
        const cfPropertyWrapperDesc*GetTypeData                 ( const cgUID & Identifier ) const;
        gcrootx<cfUIPropertyExplorer^> GetPropertyExplorer         ( ) const;
        void                        SetPropertyExplorer         ( gcrootx<cfUIPropertyExplorer^> oProperties );

        // Property selection
        void                        ShowSceneProperties         ( cgWorld * pWorld, cgUInt32 nSceneId );
        void                        ShowObjectProperties        ( cgObjectNodeMap & Nodes );
        void                        ShowSceneElementProperties  ( cgSceneElement * pElement );
        void                        ShowSubElementProperties    ( cgObjectNode * pActiveNode, cgObjectSubElementArray & SubElements );
        void                        ClearProperties             ( );

    protected:
        //---------------------------------------------------------------------
        // Protected Typedefs
        //---------------------------------------------------------------------
        typedef std::map<cgUID,cfPropertyWrapperDesc> PropertyWrapperMap;

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>Collection of cfPropertyWrapperDesc objects using the identifier as key.</summary>
        PropertyWrapperMap              m_PropertyWrappers;
        /// <summary>The property explorer control in which to display properties.</summary>
        gcptr(cfUIPropertyExplorer^)    m_PropertyExplorer;
        
    }; // End Class cfPropertyManager

} // End Namespace CarbonForge