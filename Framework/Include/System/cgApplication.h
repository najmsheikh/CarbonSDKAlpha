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
// Name : cgApplication.h                                                    //
//                                                                           //
// Desc : Game application class. This class is the central hub for all of   //
//        the main application processing. While no game specific logic is   //
//        contained in this class, it is primarily responsible for           //
//        initializing all of the systems that the application will rely on  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAPPLICATION_H_ )
#define _CGE_CGAPPLICATION_H_

//-----------------------------------------------------------------------------
// cgApplication Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgReference.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {EDEB8A5D-72CC-4879-9F98-75954DA412DF}
const cgUID RTID_Application = {0xEDEB8A5D, 0x72CC, 0x4879, {0x9F, 0x98, 0x75, 0x95, 0x4D, 0xA4, 0x12, 0xDF}};

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgThread;
class cgAppWindow;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgApplication (Class)
/// <summary>
/// Central application handling class. Initializes the app and handles
/// all core processes.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgApplication : public cgReference
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgApplication();
    virtual ~cgApplication();

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    frameAdvance            ( bool runSimulation = true );
    void                    setWindowIcon           ( cgUInt32 iconHandle );
    void                    setRootDataPath         ( const cgString & path );
    void                    setWindowTitle          ( const cgString & title );
    void                    setCopyrightData        ( const cgString & copyright );
    void                    setVersionData          ( const cgString & version );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool            initInstance            ( const cgString & commandLine );
    virtual int             begin                   ( cgThread * applicationThread );
    virtual bool            shutDown                ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_Application; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;
    virtual bool            processMessage          ( cgMessage * message );

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual void        loadConfig              ( const cgString & fileName );
    virtual bool        initDisplay             ( );
    virtual bool        initInterface           ( );
    virtual bool        initInput               ( );
    virtual bool        initAudio               ( );
    virtual bool        initPhysics             ( );
    virtual bool        initApplication         ( );
    virtual bool        frameBegin              ( bool runSimulation = true );
    virtual void        frameRender             ( );
    virtual void        frameEnd                ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgString            mWindowTitle;           // Title to be displayed on the device window.
    cgString            mVersion;               // Cached version string retrieved from resource string table.
    cgString            mCopyright;             // Cached copyright string retrieved from resource string table.
    cgUInt32            mWindowIcon;            // The icon to be displayed on the device window
    cgAppWindow       * mFocusWindow;           // The main focus window created if no override was supplied.
    cgAppWindow       * mFocusWindowOverride;   // Window to use for device initialization. If NULL, a new window will be opened.
    cgAppWindow       * mOutputWindowOverride;  // Window to output to during scene presentation.
    
    // Configuration
    cgString            mSystemConfig;          // Engine configuration file (set by loadConfig()).
    cgString            mRootDataDir;           // The root data directory used by the file system.
    bool                mAutoAddPackages;       // System should search for and automatically index *.pkg files during initialization process.
    cgFloat             mMaximumFPS;            // Maximum frame rate cap (0 = disabled)
    cgFloat             mMaximumSmoothedFPS;    // Maximum frame rate to attempt to smooth VSync input lag.

};

#endif // !_CGE_CGAPPLICATION_H_