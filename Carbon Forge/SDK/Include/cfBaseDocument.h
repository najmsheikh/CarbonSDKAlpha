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
// File: cfBaseDocument.h                                                    //
//                                                                           //
// Desc: Contains base interface classes from which application documents    //
//       can be derived. This is essentially a very slim implementation of   //
//       a common document/view model.                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfBaseDocument Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

// CGE Includes
#include <System/cgEventDispatcher.h>

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    class cfBaseDocument;
    ref class cfBaseDocumentView;

    //-------------------------------------------------------------------------
    // Delegate Definitions
    //-------------------------------------------------------------------------
    struct DocumentViewEventArgs
    {
        DocumentViewEventArgs( cfBaseDocumentView^ _oView) { oView = _oView; }
        gcrootx<cfBaseDocumentView^> oView;
    };
    
    struct DocumentEventArgs
    {
    public:
        DocumentEventArgs( cfBaseDocument * _pDocument ) { pDocument = _pDocument; }
        cfBaseDocument * pDocument;
    };
    
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //  Name : cfDocumentEventListener (Class)
    /// <summary>
    /// Abstract interface class from which other classes can derive in order 
    /// to recieve messages whenever document events occur.
    /// </summary>
    //-----------------------------------------------------------------------------
    class CARBONFORGE_API cfDocumentEventListener : public cgEventListener
    {
    public:
        //-------------------------------------------------------------------------
        // Public Virtual Methods
        //-------------------------------------------------------------------------
        virtual void        OnDocumentClosing       ( DocumentEventArgs * e ) {}
        virtual void        OnDocumentViewAdded     ( DocumentViewEventArgs * e ) {}
        virtual void        OnDocumentViewRemoved   ( DocumentViewEventArgs * e ) {}
        virtual void        OnDocumentDirtyChange   ( DocumentEventArgs * e ) {}
        virtual void        OnDocumentNameChange    ( DocumentEventArgs * e ) {}
        virtual void        OnDocumentPathChange    ( DocumentEventArgs * e ) {}
    };

    //-------------------------------------------------------------------------
    // Name : cfBaseDocument (Class)
    /// <summary>
    /// Base document class from which all document types should derive.
    /// Provides basic document methods in addition to acting as an interface 
    /// to derived functionality.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfBaseDocument : public cgEventDispatcher
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
         cfBaseDocument( );
        ~cfBaseDocument();
        
        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        bool                         AddDocumentView         ( gcrootx<cfBaseDocumentView^> oDocumentView );
        void                         RemoveDocumentView      ( gcrootx<cfBaseDocumentView^> oDocumentView );
        gcrootx<cfBaseDocumentView^> GetViewByType           ( gcrootx<System::Type^> oType );
        void                         RedrawAllViews          ( );
        void                         Close                   ( );
        bool                         IsDirty                 ( ) const;
        bool                         IsSerialized            ( ) const;
        const cgString             & GetName                 ( ) const;
        const cgString             & GetSerializePath        ( ) const;
        void                         SetName                 ( const cgString & strName );
        void                         SetSerializePath        ( const cgString & strPath );
        void                         SetDirty                ( bool bDirty );

        //---------------------------------------------------------------------
        // Public Virtual Methods
        //---------------------------------------------------------------------
        virtual bool                 ValidDocumentViewType   ( gcrootx<cfBaseDocumentView^> oDocumentView );

    protected:
        //---------------------------------------------------------------------
        // Protected Typedefs
        //---------------------------------------------------------------------
        CGE_ARRAY_DECLARE(gcrootx<cfBaseDocumentView^>*, DocumentViewArray)

        //---------------------------------------------------------------------
        // Protected Virtual Methods
        //---------------------------------------------------------------------
        virtual void        OnDocumentClosing       ( DocumentEventArgs * e );
        virtual void        OnDocumentViewAdded     ( DocumentViewEventArgs * e );
        virtual void        OnDocumentViewRemoved   ( DocumentViewEventArgs * e );
        virtual void        OnDocumentDirtyChange   ( DocumentEventArgs * e );
        virtual void        OnDocumentNameChange    ( DocumentEventArgs * e );
        virtual void        OnDocumentPathChange    ( DocumentEventArgs * e );

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>All currently open views for this document.</summary>
        DocumentViewArray       m_aDocumentViews;
        /// <summary>The name assigned to this document.</summary>
        cgString                m_strDocumentName;
        /// <summary>Value indicating whether or not this document is 'dirty' (i.e. has been altered since it was last saved).</summary>
        bool                    m_bDocumentDirty;
        /// <summary>The path to the opened document file (if any).</summary>
        cgString                m_strPath;
        /// <summary>Is there a serialized version of the file yet?</summary>
        bool                    m_bSerialized;
    
    }; // End Class cfBaseDocument

} // End Namespace CarbonForge