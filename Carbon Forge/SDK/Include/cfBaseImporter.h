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
// File: cfBaseImporter.h                                                    //
//                                                                           //
// Desc: Geometry loading code is separated out into classes that are        //
//       responsible for indexing geometry file data, and loading            //
//       individual portions of that data as requested. This file contains   //
//       the base class from which format specific importers should be       //
//       derived.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfBaseImporter Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

// CGE Includes
#include <cgBaseTypes.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;
class cgThread;

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    class cfWorldDoc;
    class cfProgressHandler;

    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cfBaseImporter (Base Class)
    /// <summary>
    /// This is the base interface class through which the application will
    /// access mesh data from the geometry data file specified. This allows
    /// multiple mesh data formats to be supported through a single
    /// application interface.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfBaseImporter
	{
	public:
        //---------------------------------------------------------------------
        // Public Typedefs, Structures & Enumerators
        //---------------------------------------------------------------------
        enum ImportType
        {
            /// <summary>The importer is a single 'active' scene importer -- which means it requires a single scene to be loaded.</summary>
            SingleSceneImport,
            /// <summary>The importer will manipulate the database directly.</summary>
            WorldImport

        }; // End Enum ImportType
        enum ImportDialogType
        {
            /// <summary>System should not display any import options dialog.</summary>
            NoDialog,
            /// <summary>System should display the standard 'simple' import options dialog.</summary>
            SimpleDialog,
            /// <summary>Importer wants to display its own custom import options dialog.</summary>
            CustomDialog

        }; // End Enum ImportDialogType
        enum OptionStatus
        {
            /// <summary>The importer requires this option / information / process and it must be imported.</summary>
            Required,
            /// <summary>This option has been selected.</summary>
            Selected,
            /// <summary>This option / information / process should be skipped.</summary>
            Skip
        
        }; // End Enum OptionStatus
        CGE_MAP_DECLARE(cgString, OptionStatus, NamedOptionMap)

        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
                 cfBaseImporter( cfWorldDoc * pDocument, const cgString & strSourceFile );
        virtual ~cfBaseImporter();

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        void                            Dispose             ( );
        bool                            IsIndexed           ( ) const;
        const cgString                & GetSourceFile       ( ) const;
        const NamedOptionMap          & GetSupportedOptions ( ) const;
        NamedOptionMap                & GetSupportedOptions ( );
        const NamedOptionMap          & GetAvailableOptions ( ) const;
        NamedOptionMap                & GetAvailableOptions ( );
        cgObjectNodeMap               & GetCreatedObjects   ( );
        const cgObjectNodeMap         & GetCreatedObjects   ( ) const;
        cgMaterialHandleSet           & GetCreatedMaterials ( );
        const cgMaterialHandleSet     & GetCreatedMaterials ( ) const;

        //---------------------------------------------------------------------
        // Public Virtual Methods
        //---------------------------------------------------------------------
        virtual ImportType          GetImportType       ( ) const = 0;
        virtual ImportDialogType    GetImportDialogType ( ) const;
        virtual bool                ShowImportDialog    ( );
        virtual bool                IsValidFile         ( ) = 0;
        virtual bool                Load                ( cgScene * pScene, cgThread * pThread, cfProgressHandler * pProgress ) = 0;
        virtual bool                Index               ( );

    protected:
        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>The document into which we are importing data.</summary>
        cfWorldDoc            * m_pDocument;
        /// <summary>Source file from which data will be loaded.</summary>
        cgString                m_strSource;
        /// <summary>Has the source been indexed yet?</summary>
        bool                    m_bSourceIndexed;
        /// <summary>Options supported by this importer type.</summary>
        NamedOptionMap          m_SupportedOptions;
        /// <summary>Options that have been added due to elements discovered in the source file.</summary>
        NamedOptionMap          m_AvailableOptions;
        /// <summary>List of all object nodes that were actually created during the import.</summary>
        cgObjectNodeMap         m_CreatedObjects;
        /// <summary>List of all materials that were created during the import. These will be added to the scene's active material list automatically.</summary>
        cgMaterialHandleSet     m_CreatedMaterials;

    }; // End Class cfBaseImporter

} // End Namespace CarbonForge