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
// Name : cgAppLog.h                                                         //
//                                                                           //
// Desc : These classes provide support for logging events within the        //
//        engine and host application. Custom 'LogOutput' classes can be     //
//        derived to route messages to alternate destinations (such as an    //
//        in-game console or stdout console window.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAPPLOG_H_ )
#define _CGE_CGAPPLOG_H_

//-----------------------------------------------------------------------------
// cgAppLog Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <fstream>
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgLogOutput;
class cgScriptObject;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAppLog (Class)
/// <summary>
/// Class for handling the output of engine and application logging
/// events for output in a variety of ways.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAppLog
{
public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum EventType
    {
        Normal      = 0x0,
        Internal    = 0x1,
        Debug       = 0x2,
        Info        = 0x20,
        Warning     = 0x40,
        Error       = 0x80
    
    }; // End Enum EventType

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    static bool             beginLogging    ( const cgString & fileName = _T("") );
    static void             endLogging      ( );
    static void             write           ( const cgTChar * formatPattern, ... );
    static void             write           ( cgUInt32 type, const cgTChar * formatPattern, ... );
    static void             writeSeparator  ( cgUInt32 type = Normal );
    static bool             registerOutput  ( cgLogOutput * output );
    static void             removeOutput    ( cgLogOutput * output, bool destroy = true );
    static size_t           getOutputCount  ( );
    static cgLogOutput    * getOutput       ( cgUInt32 index );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE(cgLogOutput*, LogOutputArray)

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    static LogOutputArray       mOutputChannels;    // List of registered cgLogOutput items
    static bool                 mLogging;           // Are we currently logging?
};

//-----------------------------------------------------------------------------
//  Name : cgLogOutput (Interface Class)
/// <summary>
/// The base class from which a log output source can derive and can
/// subsequently be registered with the cgAppLog class.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLogOutput
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    virtual ~cgLogOutput() {};
    
    //-------------------------------------------------------------------------
    // Public Pure Virtual Methods
    //-------------------------------------------------------------------------
    virtual void        writeHeader     ( ) = 0;
    virtual void        writeFooter     ( ) = 0;
    virtual void        write           ( cgUInt32 type, const cgString & message ) = 0;
    virtual void        writeSeparator  ( cgUInt32 type ) = 0;
    virtual bool        isOpen          ( ) const = 0;
    virtual cgUInt32    getOutputWidth  ( ) const = 0;
};

//-----------------------------------------------------------------------------
//  Name : cgLogOutputFile (Class)
/// <summary>
/// Provides support for writing logged messages to a standard output
/// file on disk.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLogOutputFile : public cgLogOutput
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgLogOutputFile( const cgString & fileName, const cgString & applicationTitle = _T("Carbon Game Engine"), const cgString & logTitle = _T("Carbon Game Engine: Information Log") );
    ~cgLogOutputFile();

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgLogOutput).
    //-------------------------------------------------------------------------
    void        writeHeader     ( );
    void        writeFooter     ( );
    void        write           ( cgUInt32 type, const cgString & message );
    void        writeSeparator  ( cgUInt32 type );
    bool        isOpen          ( ) const;
    cgUInt32    getOutputWidth  ( ) const { return 0; }

private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    void        detectHardware  ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
#if defined( UNICODE )
    std::wofstream mFile;
#else
    std::ofstream  mFile;
#endif

    cgString    mApplicationTitle;  // The application title as it appears in the log
    cgString    mLogTitle;          // The title of the HTML page that the class outputs to
    cgString    mIconPath;          // Path to the icon images.
};

//-----------------------------------------------------------------------------
//  Name : cgLogOutputStd (Class)
/// <summary>
/// Provides support for writing logged messages to the 'stdout' stream
/// (i.e. via cout)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLogOutputStd : public cgLogOutput
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgLogOutputStd( bool openConsole = false );
    ~cgLogOutputStd();

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgLogOutput)
    //-------------------------------------------------------------------------
    void        writeHeader     ( );
    void        writeFooter     ( );
    void        write           ( cgUInt32 type, const cgString & message );
    void        writeSeparator  ( cgUInt32 type );
    bool        isOpen          ( ) const { return true; }
    cgUInt32    getOutputWidth  ( ) const { return 79; }

private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    bool   mConsoleOwned;   // Did we open a console window?
};

//-----------------------------------------------------------------------------
//  Name : cgLogOutputScript (Class)
/// <summary>
/// Provides support for forwarding logged messages to a script.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLogOutputScript : public cgLogOutput
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgLogOutputScript( const cgString & headerMethod, const cgString & footerMethod, const cgString & writeMethod, const cgString & separatorMethod, cgScriptObject * scriptObject );
    ~cgLogOutputScript( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgLogOutput)
    //-------------------------------------------------------------------------
    void        writeHeader     ( );
    void        writeFooter     ( );
    void        write           ( cgUInt32 type, const cgString & message );
    void        writeSeparator  ( cgUInt32 type );
    bool        isOpen          ( ) const { return true; }
    cgUInt32    getOutputWidth  ( ) const { return 0; }

private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgScriptObject* mScriptObject;
    cgString        mWriteMethod;
    cgString        mSeparatorMethod;
    cgString        mHeaderMethod;
    cgString        mFooterMethod;
};

#endif // !_CGE_CGAPPLOG_H_