//---------------------------------------------------------------------------//
//                     __  __           _       _        ___ ___ ___         //
//        __ _ _ __   |  \/  | ___   __| |_   _| | ___  |_ _|_ _|_ _|        //
//       / _` | '_ \  | |\/| |/ _ \ / _` | | | | |/ _ \  | | | | | |         //
//      | (_| | |_) | | |  | | (_) | (_| | |_| | |  __/  | | | | | |         //
//       \__, | .__/  |_|  |_|\___/ \__,_|\__,_|_|\___| |___|___|___|        //
//       |___/|_|      Game Institute Graphics Programming Module III        //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : giEXCore.h                                                         //
//                                                                           //
// Desc : Core classes and entry point for the extension library core. This  //
//        provides the consumer with access to easy to use interfaces for    //
//        various closed source features such as decoding MPEG4 video        //
//        streams and more.                                                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2008 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#if !defined( _GIEXCORE_H_ )
#define _GIEXCORE_H_

//-----------------------------------------------------------------------------
// Library Directives
//-----------------------------------------------------------------------------
#if defined( _LIB )
#define GIEXCORE_API
#else   // _LIB
#if defined( GIEXCORE_EXPORTS )
#define GIEXCORE_API __declspec(dllexport)
#else   // GIEXCORE_EXPORTS
#define GIEXCORE_API __declspec(dllimport)
#endif  // !GIEXCORE_EXPORTS
#endif  // !_LIB

//-----------------------------------------------------------------------------
// Global Enumerators
//-----------------------------------------------------------------------------
enum giEXResult
{
    GIEX_Success                = 0,
    GIEX_LibraryNotInitialized  = -1,
    GIEX_NotSupported           = -2,
    GIEX_NoData                 = -3,
    GIEX_Failed                 = -4,
    GIEX_AlreadyRegistered      = -5,

    GIEX_Force32bit             = 0x7FFFFFFF
};

//-----------------------------------------------------------------------------
// Global Macros
//-----------------------------------------------------------------------------
#define GIEXSUCCESS(r) ((int)(r) >= 0)
#define GIEXFAILED(r) ((int)(r) < 0)

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------
giEXResult GIEXCORE_API giEXInit           ( const char * lpszUser, const char * lpszLicense );
giEXResult GIEXCORE_API giEXInit           ( const wchar_t * lpszUser, const wchar_t * lpszLicense );
bool       GIEXCORE_API giEXIsInitialized  ( );

//-----------------------------------------------------------------------------
// Global Structures
//-----------------------------------------------------------------------------
// Utility structure providing easy access to size information.
struct giSize
{
    int width;
    int height;
};

// Utility structure used to transport information about the opened media.
struct giMediaInfo
{
    bool    containsVideo;      // Media contains video stream.
    giSize  frameDimensions;    // Dimensions, in pixels, of any video frames.
    bool    containsAudio;      // Media contains audio stream.
    double  duration;           // Duration of the media in seconds.
    double  frameRate;          // Frame rate of media
};

// Allows user to configure how the media will be decoded.
enum giAudioDecodeMethod
{
    GIADM_None,
    GIADM_Auto,
    GIADM_Notify
};
struct giMediaDecoderConfig
{
    giAudioDecodeMethod audioDecodeMethod;       // Audio decoding method.

    // Constructor
    giMediaDecoderConfig() : audioDecodeMethod( GIADM_Auto ) {}
};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : giMediaListener (Interface)
// Desc : Listener class used to communicate with host application.
//-----------------------------------------------------------------------------
class giMediaListener
{
public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual giEXResult onNewVideoFrame  ( void * data, unsigned long size ) = 0; 
};

//-----------------------------------------------------------------------------
// Name : giMediaDecoder (Class)
// Desc : Core interface to be used when decoding media containers such as AVI
//        for video / audio processing & integration.
//-----------------------------------------------------------------------------
class GIEXCORE_API giMediaDecoder
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     giMediaDecoder( );
    ~giMediaDecoder( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    giEXResult  setFocusWindow      ( void * windowHandle );
    giEXResult  setDecoderConfig    ( const giMediaDecoderConfig & config );
    giEXResult  supportedContainer  ( const char * container );
    giEXResult  supportedContainer  ( const wchar_t * container );
    giEXResult  openMediaContainer  ( const char * container );
    giEXResult  openMediaContainer  ( const wchar_t * container );
    giEXResult  closeMediaContainer (  );
    giEXResult  play                (  );
    giEXResult  stop                (  );
    giEXResult  pause               (  );
    giEXResult  resume              (  );
    giEXResult  registerListener    ( giMediaListener * listener );
    giEXResult  unregisterListener  ( giMediaListener * listener );
    giEXResult  getMediaInfo        ( giMediaInfo & info );
    giEXResult  getCurrentVideoFrame( void * destination, unsigned long pitch );
    bool        isPlaying           (  );
    double      getPlayheadTime     (  );
    void        setPlayheadTime     (  double position );

private:
    //-------------------------------------------------------------------------
    // Private Member Variables
    //-------------------------------------------------------------------------
    void * __coreInt;  // Reserved.
};

#endif // !_GIEXCORE_H_