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
// File: cfBaseExporter.h                                                    //
//                                                                           //
// Desc: Geometry export code is separated out into classes that are         //
//       responsible for constructing geometry file data and exporting       //
//       individual portions of that data as requested. This file contains   //
//       the base class from which format specific exporters should be       //
//       derived.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfBaseExporter Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

// CGE Includes
#include <cgBaseTypes.h>

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
    // Name : cfBaseExporter (Base Class)
    /// <summary>
    /// This is the base interface class through which the application will
    /// export data to the data file specified. This allows multiple data
    /// formats to be supported through a single application interface.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfBaseExporter
	{
	public:
        //---------------------------------------------------------------------
        // Public Typedefs, Structures & Enumerators
        //---------------------------------------------------------------------
        enum ExportType
        {
            /// <summary>The exporter is a single 'active' scene exporter -- which means it will only export a single scene to the file.</summary>
            SingleSceneExport,
            /// <summary>The exporter will export the entire contents of the database.</summary>
            WorldExport

        }; // End Enum ExportType
        enum ExportDialogType
        {
            /// <summary>System should not display any export options dialog.</summary>
            NoDialog,
            /// <summary>System should display the standard 'simple' export options dialog.</summary>
            SimpleDialog,
            /// <summary>Exporter wants to display its own custom export options dialog.</summary>
            CustomDialog

        }; // End Enum ExportDialogType
        enum OptionStatus
        {
            /// <summary>The exporter requires this option / information / process and it must be exported.</summary>
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
                 cfBaseExporter( cfWorldDoc * pDocument, const cgString & strDestinationFile );
        virtual ~cfBaseExporter();

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        void                        Dispose             ( );
        const cgString            & GetDestinationFile  ( ) const;
        const NamedOptionMap      & GetSupportedOptions ( ) const;
        NamedOptionMap            & GetSupportedOptions ( );
        const NamedOptionMap      & GetAvailableOptions ( ) const;
        NamedOptionMap            & GetAvailableOptions ( );

        //---------------------------------------------------------------------
        // Public Virtual Methods
        //---------------------------------------------------------------------
        virtual ExportType          GetExportType       ( ) const = 0;
        virtual ExportDialogType    GetExportDialogType ( ) const;
        virtual bool                ShowExportDialog    ( );
        virtual bool                Save                ( cgScene * pScene, cgThread * pThread, cfProgressHandler * pProgress ) = 0;

    protected:
        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>The document from which we are exporting data.</summary>
        cfWorldDoc        * m_pDocument;
        /// <summary>Destination file to which data will be exported.</summary>
        cgString            m_strDestination;
        /// <summary>Options supported by this exporter type.</summary>
        NamedOptionMap      m_SupportedOptions;
        /// <summary>Options that have been added due to elements discovered in the scene.</summary>
        NamedOptionMap      m_AvailableOptions;

    }; // End Class cfBaseExporter

} // End Namespace CarbonForge