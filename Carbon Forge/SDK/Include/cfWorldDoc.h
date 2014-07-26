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
// File: cfWorldDoc.h                                                        //
//                                                                           //
// Desc: Contains a derived document class which manages and process data    //
//       associated with a CGE world definition database.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2007 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfWorldDoc Header Includes
//-----------------------------------------------------------------------------
#include "cfBaseDocument.h"

// CGE Includes
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>
#include <World/cgWorld.h>
#include <World/cgWorldTypes.h>
#include <World/cgScene.h>

//-----------------------------------------------------------------------------
// Global Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;
class cgThread;
class cgCriticalSection;

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    class cfProgressHandler;
    class cfBaseImporter;
    class cfBaseExporter;
    class cfToolManager;
    ref class cfUISceneView;
    ref class cfUIAppView;
    ref class frmProgress;

    // ToDo: 9999 Should we just derive from cgWorld and avoid this wrapping malarky?

    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    struct cfSceneViewChangeEventArgs
    {
        cfSceneViewChangeEventArgs( cfUISceneView ^ _oNewView, cfUISceneView ^ _oOldView ) :
            oView( _oNewView ), oOldView( _oOldView ){}
        gcrootx<cfUISceneView^> oView;
        gcrootx<cfUISceneView^> oOldView;
    };
    
    //-------------------------------------------------------------------------
    // Name : cgWorldDocEventListener (Class)
    /// <summary>
    /// Provides integration between events raised by cgWorld and the editor's
    /// cfBaseDocument class.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfWorldDocEventListener : public cfDocumentEventListener
    {
    public:
        //-------------------------------------------------------------------------
        // Public Virtual Methods
        //-------------------------------------------------------------------------
        virtual void    OnSceneAdded            ( cgSceneUpdateEventArgs * e ) {};
		virtual void    OnSceneRemoved			( cgSceneUpdateEventArgs * e ) {};
        virtual void    OnSceneLoading          ( cgSceneLoadEventArgs * e ) {};
        virtual void    OnSceneLoadFailed       ( cgSceneLoadEventArgs * e ) {};
        virtual void    OnSceneLoaded           ( cgSceneLoadEventArgs * e ) {};
        virtual void    OnSceneUnloading        ( cgSceneUnloadEventArgs * e ) {};
        virtual void    OnActiveSceneViewChange ( cfSceneViewChangeEventArgs * e ) {};
        virtual void    OnImportBegin           ( cgSceneLoadEventArgs * e ) {};
        virtual void    OnImportEnd             ( cgSceneLoadEventArgs * e ) {};
    };

    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cfWorldDoc (Class)
    /// <summary>
    /// Derived document class which manages and process data associated with 
    /// a CGE world definition database. 
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfWorldDoc : public cfBaseDocument, public cgWorldEventListener, public cgSceneEventListener
	{
	public:
        //---------------------------------------------------------------------
        // Public Structures, Typedefs and Enumerations
        //---------------------------------------------------------------------
        typedef cfBaseImporter * (*ImporterAllocFunc)( const cgUID &, cfWorldDoc*, const cgString& );
        typedef cfBaseExporter * (*ExporterAllocFunc)( const cgUID &, cfWorldDoc*, const cgString& );
        
        /// <summary>Structure responsible for storing information about registered importer types / formats.</summary>
        struct ImporterTypeDesc
        {
            /// <summary>Globally unique identifier for this importer type.</summary>
            cgUID               Identifier;
            /// <summary>Wildcard to use for identifying files of this type, separated by semi-colon (i.e. "*.x;*.xof").</summary>
            cgString            Wildcard;
            /// <summary>Publicly visible name used to denote an importer of this type / format (i.e. "DirectX X Files (.x)").</summary>
            cgString            PublicName;
            /// <summary>Pointer to static / global allocation function that will be used to create an instance of this importer class.</summary>
            ImporterAllocFunc   ImporterAlloc;
        
        }; // End Struct ImporterTypeDesc
        CGE_MAP_DECLARE(cgUID, ImporterTypeDesc, ImporterTypeDescMap)

        /// <summary>Structure responsible for storing information about registered exporter types / formats.</summary>
        struct ExporterTypeDesc
        {
            /// <summary>Globally unique identifier for this exporter type.</summary>
            cgUID               Identifier;
            /// <summary>Wildcard to use for identifying files of this type, separated by semi-colon (i.e. "*.x;*.xof").</summary>
            cgString            Wildcard;
            /// <summary>Publicly visible name used to denote an exporter of this type / format (i.e. "DirectX X Files (.x)").</summary>
            cgString            PublicName;
            /// <summary>Pointer to static / global allocation function that will be used to create an instance of this exporter class.</summary>
            ExporterAllocFunc   ExporterAlloc;
        
        }; // End Struct ExporterTypeDesc
        CGE_MAP_DECLARE(cgUID, ExporterTypeDesc, ExporterTypeDescMap)

        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
                 cfWorldDoc();
        virtual ~cfWorldDoc();
        
        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        bool                        NewWorld                    ( cgWorldType::Base FileType );
        bool                        OpenWorld                   ( const cgString & strFile );
        bool                        SaveWorld                   ( const cgString & strFile );
        cgWorld                   * GetWorld                    ( ) const;
        cgUInt32                    CreateScene                 ( const cgSceneDescriptor & Desc );
        cgScene                   * LoadScene                   ( cgUInt32 nSceneID );
		bool						DeleteScene					( cgUInt32 nSceneID );
        void                        PopulateDefaultScene        ( cgScene * pScene );
        void                        FrameAdvance                ( );
        cfToolManager             * GetToolManager              ( ) const;
        void                        SetToolManager              ( cfToolManager * pTools );
        bool                        GetAutoRedraw               ( ) const;
        void                        SetAutoRedraw               ( bool bRedraw );
        cgFloat                     GetGridSize                 ( ) const;
        void                        SetGridSize                 ( cgFloat fSize );
        void                        ShowSelectionProperties     ( cgScene * pScene );
        void                        RefreshSelectionProperties  ( cgScene * pScene );
        void                        SetActiveSceneView          ( gcrootx<cfUISceneView^> oView );
        gcrootx<cfUISceneView^>     GetActiveSceneView          ( );
        void                        SetActiveAppView            ( gcrootx<cfUIAppView^> oView );
        gcrootx<cfUIAppView^>       GetActiveAppView            ( );
        gcrootx<cfUISceneView^>     GetSceneView                ( cgUInt32 nSceneID );
        cgScene                   * GetActiveScene              ( ) const;

        cgOperationSpace::Base      GetTranslationSpace         ( ) const;
        cgOperationSpace::Base      GetRotationSpace            ( ) const;
        cgOperationSpace::Base      GetScaleSpace               ( ) const;
        cgTransformMethod::Base     GetTransformMethod          ( ) const;
        void                        SetTranslationSpace         ( cgOperationSpace::Base Space );
        void                        SetRotationSpace            ( cgOperationSpace::Base Space );
        void                        SetScaleSpace               ( cgOperationSpace::Base Space );
        void                        SetTransformMethod          ( cgTransformMethod::Base Method );
        const cgVector3           & GetRelativeTransform        ( ) const;
        void                        SetRelativeTransform        ( const cgVector3 & value );
        void                        SetPositionSnapping         ( bool enable );
        void                        SetRotationSnapping         ( bool enable );
        bool                        GetPositionSnapping         ( ) const;
        bool                        GetRotationSnapping         ( ) const;

        void                        RegisterImporterType        ( const cgUID & Identifier, const cgString & strWildcard, const cgString & strPublicName, ImporterAllocFunc pAllocFunc );
        const ImporterTypeDescMap & GetImporterTypes            ( ) const;
        cfBaseImporter            * CreateImporter              ( const cgUID & TypeIdentifier, const cgString & strSource );
        bool                        ImportData                  ( const cgString & strSource, bool bDisplayOptions );

        void                        RegisterExporterType        ( const cgUID & Identifier, const cgString & strWildcard, const cgString & strPublicName, ExporterAllocFunc pAllocFunc );
        const ExporterTypeDescMap & GetExporterTypes            ( ) const;
        cfBaseExporter            * CreateExporter              ( const cgUID & TypeIdentifier, const cgString & strDestination );
        bool                        ExportData                  ( const cgString & strDestination, const cgUID & Type, bool bDisplayOptions );
        
        //---------------------------------------------------------------------
        // Public Virtual Methods
        //---------------------------------------------------------------------
        virtual void                OnActiveSceneViewChange     ( cfSceneViewChangeEventArgs * e );
        virtual void                OnImportBegin               ( cgSceneLoadEventArgs * e );
        virtual void                OnImportEnd                 ( cgSceneLoadEventArgs * e );

        //---------------------------------------------------------------------
        // Public Virtual Methods (Overrides cfBaseDocument)
        //---------------------------------------------------------------------
        virtual bool                ValidDocumentViewType       ( gcrootx<cfBaseDocumentView^> documentView );
        virtual void                OnDocumentViewRemoved       ( DocumentViewEventArgs * e );

        //---------------------------------------------------------------------
        // Public Virtual Methods (Overrides cgWorldEventListener)
        //---------------------------------------------------------------------
        virtual void                onSceneAdded                ( cgSceneUpdateEventArgs * e );
		virtual void                onSceneRemoved				( cgSceneUpdateEventArgs * e );
        virtual void                onSceneLoading              ( cgSceneLoadEventArgs * e );
        virtual void                onSceneLoadFailed           ( cgSceneLoadEventArgs * e );
        virtual void                onSceneLoaded               ( cgSceneLoadEventArgs * e );
        virtual void                onSceneUnloading            ( cgSceneUnloadEventArgs * e );
        virtual void                onSelectionCleared          ( cgSceneEventArgs * e );
        virtual void                onSelectionUpdated          ( cgSelectionUpdatedEventArgs * e );

        //---------------------------------------------------------------------
        // Public Virtual Methods (Overrides cgSceneEventListener)
        //---------------------------------------------------------------------
        virtual void                onSceneDirtyChange          ( cgSceneEventArgs * e );

    protected:
        //---------------------------------------------------------------------
        // Protected Structures
        //---------------------------------------------------------------------
        struct ScenePreviewData
        {
            CGE_ARRAY_DECLARE(ScenePreviewData, Array)

            cgUInt32    nSceneId;
            bool        bAttached;
            bool        bUnloadRequested;
        };

        //---------------------------------------------------------------------
        // Protected Static Functions
        //---------------------------------------------------------------------
        static cgUInt32             ImportThreadFunc            ( cgThread * pThread, void * pContext );
        static cgUInt32             ExportThreadFunc            ( cgThread * pThread, void * pContext );

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>The object that describes the current world definition, and manages any loaded scenes.</summary>
        cgWorld                   * m_pWorld;
        /// <summary>The tool modes container that defines tools available for modifying this document.</summary>
        cfToolManager             * m_pToolManager;
        /// <summary>Currently active scene view control.</summary>
        gcptr(cfUISceneView^)       m_ActiveSceneView;
        /// <summary>Attached views should automatically redraw on frame advance?</summary>
        bool                        m_bAutoRedraw;
        /// <summary>The size of the grid currently being used for guidance / snapping.</summary>
        cgFloat                     m_fGridSize;
        /// <summary>The space in which translation operations should occur.</summary>
        cgOperationSpace::Base      m_TranslationSpace;
        /// <summary>The space in which rotation operations should occur.</summary>
        cgOperationSpace::Base      m_RotationSpace;
        /// <summary>The space in which scale operations should occur.</summary>
        cgOperationSpace::Base      m_ScaleSpace;
        /// <summary>A status tracking variable that can be used by tools to describe a transformation that has occured (for informational purposes only).</summary>
        cgVector3                 m_vRelativeTransform;
        /// <summary>The mechanism used to transform objects based on the user's current selection.</summary>
        cgTransformMethod::Base     m_TransformMethod;
        /// <summary>Handles the interaction between the system and various components such as processors and importers when reporting progress.</summary>
        cfProgressHandler         * m_pProgress;
        /// <summary>Enable or disable grid snapping for position related tools such as move, scale, etc.</summary>
        bool                        m_bPositionSnapping;
        /// <summary>Enable or disable increment snapping for orientation related tools such as rotate, etc.</summary>
        bool                        m_bRotationSnapping;

        // Preview Handling
        /// <summary>If we are currently in the process of previewing an application, this stores a reference to the active view.</summary>
        gcptr(cfUIAppView^)         m_ActiveAppView;
        /// <summary>List of all scenes that were already loaded when a preview began.</summary>
        ScenePreviewData::Array     m_PreLoadedScenes;
        
        // Import Handling
        /// <summary>Collection of available importer types using the identifier as key.</summary>
        ImporterTypeDescMap         m_ImporterTypes;
        /// <summary>Import data used by the importer thread during processing.</summary>
        cfBaseImporter            * m_pImporter;
        /// <summary>The scene into which data should be imported (if any).</summary>
        cgScene                   * m_pImportScene;
        /// <summary>Critical section wrapped around the opening / closing of the wait screen to avoid race condition.</summary>
        cgCriticalSection         * m_pWaitScreenSection;
        /// <summary>Result of the import process.</summary>
        volatile bool               m_bImportResult;

        // Export Handling
        /// <summary>Collection of available exporter types using the identifier as key.</summary>
        ExporterTypeDescMap         m_ExporterTypes;
        /// <summary>Export data used by the exporter thread during processing.</summary>
        cfBaseExporter            * m_pExporter;
        /// <summary>The scene from which data should be exported (if any).</summary>
        cgScene                   * m_pExportScene;
        /// <summary>Result of the export process.</summary>
        volatile bool               m_bExportResult;
    
    }; // End Class cfWorldDoc

} // End Namespace CarbonForge