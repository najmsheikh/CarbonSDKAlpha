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
// Name : cgInputDriver.cpp                                                  //
//                                                                           //
// Desc : Input class wraps the properties, initialization and management of //
//        our input device. This includes the enumeration, creation and      //
//        destruction of our DirectInput devices and associated resources.   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgInputDriver Module Includes
//-----------------------------------------------------------------------------
#include <Input/cgInputDriver.h>
#include <System/cgStringUtility.h>
#include <System/cgMessageTypes.h>
#include <System/Platform/cgWinAppWindow.h>
#include <algorithm>

// Use DirectInput v8.0
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <dxerr.h>
#include <Mmsystem.h> // timeGetTime

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgInputDriver * cgInputDriver::mSingleton = CG_NULL;

//-----------------------------------------------------------------------------
//  Name : cgInputDriver () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputDriver::cgInputDriver() : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // initialize variables to sensible defaults
    mConfigLoaded     = false;
    mInitialized      = false;
    mDI               = CG_NULL;
    mDIKeyboard       = CG_NULL;
    mDIMouse          = CG_NULL;
    mMousePosition.x   = 0;
    mMousePosition.y   = 0;
    mMouseMode         = cgMouseHandlerMode::Cursor;

    // Clear any memory as required
    memset( &mConfig, 0, sizeof(InitConfig) );
    memset( mKeyStates, 0, sizeof(mKeyStates) );
    memset( mPrevKeyStates, 0, sizeof(mPrevKeyStates) );
    memset( mMouseButtonStates, 0, sizeof(mMouseButtonStates) );
    memset( mPrevMouseButtonStates, 0, sizeof(mPrevMouseButtonStates) );
}

//-----------------------------------------------------------------------------
//  Name : ~cgInputDriver () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputDriver::~cgInputDriver()
{
    // Release allocated memory
    Dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : Dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::Dispose()" />
//-----------------------------------------------------------------------------
void cgInputDriver::Dispose( bool bDisposeBase )
{
    // Release DirectX objects
    if ( mDIKeyboard ) mDIKeyboard->Release();
    if ( mDIMouse    ) mDIMouse->Release();
    if ( mDI         ) mDI->Release();

    // Remove any defined clip region
    if ( mInitialized == true )
        ClipCursor( CG_NULL );

    // Re-show the cursor.
    if ( mInitialized == true && mConfig.hideSystemCursor == true )
        ShowCursor( TRUE );
    
    // Reset any variables
    mConfigLoaded = false;
    mInitialized  = false;
    mDI           = CG_NULL;
    mDIKeyboard   = CG_NULL;
    mDIMouse      = CG_NULL;

    // Clear any memory as required
    memset( &mConfig, 0, sizeof(InitConfig) );
    memset( mKeyStates, 0, sizeof(mKeyStates) );
    memset( mPrevKeyStates, 0, sizeof(mPrevKeyStates) );
    memset( mMouseButtonStates, 0, sizeof(mMouseButtonStates) );
    memset( mPrevMouseButtonStates, 0, sizeof(mPrevMouseButtonStates) );
    mKeyRepeatData.clear();

    // Dispose of base classes if requested.
    if ( bDisposeBase )
        cgReference::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgInputDriver * cgInputDriver::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createSingleton () (Static)
/// <summary>
/// Creates the singleton. You would usually allocate the singleton in
/// the static member definition, however sometimes it's necessary to
/// call for allocation to allow for correct allocation ordering
/// and destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
        mSingleton = new cgInputDriver;
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::destroySingleton( )
{
    // Destroy (unless script referencing)!
    if ( mSingleton != CG_NULL )
        mSingleton->scriptSafeDispose();
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputDriver::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_InputDriver )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : loadConfig ()
/// <summary>
/// Load the input driver configuration from the file specified.
/// </summary>
//-----------------------------------------------------------------------------
cgConfigResult::Base cgInputDriver::loadConfig( const cgString & strFileName )
{
    HRESULT         hRet;
    const cgTChar * strSection;

    // Fail if config already loaded
    if ( mConfigLoaded == true )
        return cgConfigResult::Error;

    // Retrieve configuration options if provided
    if ( strFileName.empty() == false )
    {
        strSection = _T("InputDriver");
        mConfig.ignoreWindowsKey   = GetPrivateProfileInt( strSection, _T("IgnoreWindowsKey"), true, strFileName.c_str() ) > 0;
        mConfig.keyRepeatDelay     = GetPrivateProfileInt( strSection, _T("KeyRepeatDelay"), 500, strFileName.c_str() );
        mConfig.keyRepeatRate      = GetPrivateProfileInt( strSection, _T("KeyRepeatRate"), 50, strFileName.c_str() );
        mConfig.keyboardBufferSize = max( 0, GetPrivateProfileInt( strSection, _T("KeyboardBufferSize"), 512, strFileName.c_str() ) );
        mConfig.mouseSensitivity   = cgStringUtility::getPrivateProfileFloat( strSection, _T("MouseSensitivity"), 1.0f, strFileName.c_str() );
        mConfig.hideSystemCursor   = GetPrivateProfileInt( strSection, _T("HideSystemCursor"), true, strFileName.c_str() ) > 0;

    } // End if config provided

    // Release previous DirectInput object if already created
    if ( mDI ) mDI->Release();

    // Create a DirectInput Object (We may need this to query properties etc).
    hRet = DirectInput8Create( GetModuleHandle(CG_NULL), DIRECTINPUT_VERSION,
                               IID_IDirectInput8, (void**)&mDI, CG_NULL );
    if ( FAILED(hRet) ) 
    {
        cgAppLog::write( cgAppLog::Error, _T( "No compatible DirectInput object could be created.\n") );
        return cgConfigResult::Error;

    } // End if failure
    
    // Signal that we have settled on good config options
    mConfigLoaded = true;
    
    // Options are valid. Success!!
    return cgConfigResult::Valid;
}

//-----------------------------------------------------------------------------
//  Name : loadDefaultConfig ()
/// <summary>
/// Load a default configuration for the input driver.
/// </summary>
//-----------------------------------------------------------------------------
cgConfigResult::Base cgInputDriver::loadDefaultConfig( )
{
    // Pick sensible defaults for the driver
    mConfig.ignoreWindowsKey   = true;
    mConfig.mouseSensitivity   = 1.0f;
    mConfig.keyRepeatDelay     = 500;
    mConfig.keyRepeatRate      = 50;
    mConfig.keyboardBufferSize = 512;
    mConfig.hideSystemCursor   = true;
    
    // Pass through to the loadConfig function
    return loadConfig( _T("") );
}

//-----------------------------------------------------------------------------
//  Name : saveConfig ()
/// <summary>
/// Save the input driver configuration to the file specified.
/// Note : When specifying the save filename, it's important to either use a
/// full path ("C:\\{Path}\\Config.ini") or a path relative to the
/// current directory INCLUDING the first period (".\\Config.ini"). If
/// not, windows will place the ini in the windows directory rather than
/// the application dir.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputDriver::saveConfig( const cgString & strFileName )
{
    LPCTSTR strSection = _T("InputDriver");

    // Validate requirements
    if ( strFileName.empty() == true )
        return false;

    // Save configuration options
    using namespace cgStringUtility;
    writePrivateProfileIntEx( strSection, _T("IgnoreWindowsKey")  , mConfig.ignoreWindowsKey  , strFileName.c_str() );
    writePrivateProfileIntEx( strSection, _T("KeyRepeatDelay")    , mConfig.keyRepeatDelay    , strFileName.c_str() );
    writePrivateProfileIntEx( strSection, _T("KeyRepeatRate")     , mConfig.keyRepeatRate     , strFileName.c_str() );
    writePrivateProfileIntEx( strSection, _T("KeyboardBufferSize"), mConfig.keyboardBufferSize, strFileName.c_str() );
    writePrivateProfileFloat( strSection, _T("MouseSensitivity")  , mConfig.mouseSensitivity  , strFileName.c_str() );
    writePrivateProfileIntEx( strSection, _T("HideSystemCursor")  , mConfig.hideSystemCursor  , strFileName.c_str() );

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getConfig ()
/// <summary>
/// Retrieve the configuration options for the input driver.
/// </summary>
//-----------------------------------------------------------------------------
cgInputDriver::InitConfig cgInputDriver::getConfig( ) const
{
    return mConfig;
}

//-----------------------------------------------------------------------------
//  Name : setMouseMode ()
/// <summary>
/// Set the way in which mouse movement will be interpreted / handled.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::setMouseMode( cgMouseHandlerMode::Base Mode )
{
    // Is this a no-op?
    mMouseMode = Mode;

    // De-activate cursor clipping if switching out of direct mode.
    if ( Mode != cgMouseHandlerMode::Direct )
        ::ClipCursor( CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : getMouseMode ()
/// <summary>
/// Retrieve the current mouse mode.
/// </summary>
//-----------------------------------------------------------------------------
cgMouseHandlerMode::Base cgInputDriver::getMouseMode( ) const
{
    return mMouseMode;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the input driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputDriver::initialize( cgAppWindow * pFocusWnd, bool bWindowed )
{
    HRESULT  hRet;
    cgUInt32 nCoopFlags = 0;
    cgPoint  ptCursor;

    // Store the focus window handle
    mFocusWnd = pFocusWnd;
    mWindowed = bWindowed;

    // Configuration must be loaded
    if ( !mConfigLoaded )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Input driver configuration must be loaded prior to initialization!\n"));
        return false;
    } // End if no config loaded
    
    if ( isInitialized() )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Input driver must be released before it can be initialized for a second time!\n"));
        return false;
    
    } // End if already initialized

    // Exclusive only in fullscreen
    if( !bWindowed )
        nCoopFlags = DISCL_EXCLUSIVE;
    else
        nCoopFlags = DISCL_NONEXCLUSIVE;

    cgToDo( "CarbonWE", "Web engine fails to acquire input in Google Chrome when using foreground (background works)." );
    // Only process input if the window has focus
    nCoopFlags |= DISCL_FOREGROUND;

    // Obtain an interface to the system mouse device.
    hRet = mDI->CreateDevice( GUID_SysMouse, &mDIMouse, CG_NULL );
    if ( FAILED( hRet ) ) 
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create system mouse device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // Set the data format to the predefined "mouse format".
    hRet = mDIMouse->SetDataFormat( &c_dfDIMouse2 );
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set the mouse data format. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;

    } // End if failed

    // Only 'cgWinAppWindow' type is supported.
    cgWinAppWindow * pWinAppWindow = dynamic_cast<cgWinAppWindow*>(pFocusWnd);
    if ( pWinAppWindow == NULL )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The DirectX input driver is only supported on the Windows(tm) platform.\n"));
        return false;

    } // End if invalid cast

    // Find the top-level (non-child) window.
    HWND hwndCurrent = pWinAppWindow->getWindowHandle();
    while ( hwndCurrent && GetWindowLong( hwndCurrent, GWL_STYLE ) & WS_CHILD )
        hwndCurrent = GetParent( hwndCurrent );

    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
    hRet = mDIMouse->SetCooperativeLevel( hwndCurrent, nCoopFlags );
   /* if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set the cooperativity level of the mouse device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;

    } // End if failed*/

    // Retrieve the current position of the cursor and convert to client space
    ::GetCursorPos( (POINT*)&ptCursor );
    mMousePosition = pFocusWnd->screenToClient( ptCursor );
    
    // Disable the windows key if requested (only available in windowed mode)
    if( mConfig.ignoreWindowsKey && bWindowed)
        nCoopFlags |= DISCL_NOWINKEY;

    // Obtain an interface to the system keyboard device.
    hRet = mDI->CreateDevice( GUID_SysKeyboard, &mDIKeyboard, CG_NULL );
    if ( FAILED( hRet ) ) 
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create system keyboard device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed
    
    // Set the data format to the predefined "keyboard format".
    hRet = mDIKeyboard->SetDataFormat( &c_dfDIKeyboard );
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set the keyboard data format. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;

    } // End if failed
    
    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
    hRet = mDIKeyboard->SetCooperativeLevel( hwndCurrent, nCoopFlags );
    /*if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set the cooperativity level of the keyboard device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;

    } // End if failed*/

    // Enable buffered keyboard input
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = mConfig.keyboardBufferSize;
    
    // Set to keyboard device
    hRet = mDIKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set the keyboard buffer size. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;

    } // End if failed

    // Acquire the newly created devices
    mDIMouse->Acquire();
    mDIKeyboard->Acquire();

    // Hide the cursor
    if ( mConfig.hideSystemCursor == true )
        ShowCursor( FALSE );

    // Success!
    mInitialized = true;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : poll ()
/// <summary>
/// Poll all registered input devices and retrieve their states.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::poll(  )
{
    HRESULT         hRet;
    DIMOUSESTATE2   MouseState;
    cgUInt32        nTime, nModifiers;

    // Poll mouse state
    if ( mDIMouse )
    {
        // Backup the current mouse button states
        memcpy( mPrevMouseButtonStates, mMouseButtonStates, sizeof( mMouseButtonStates ) );

        // Clear out the mouse state structure
        memset( &MouseState, 0, sizeof(MouseState) );
        
        // Retrieve the state of the mouse
        hRet = mDIMouse->GetDeviceState( sizeof(MouseState), &MouseState );
        
        // Failed to get state?
        if( FAILED(hRet) ) 
        {
            // Attempt to re-acquire the device
            hRet = mDIMouse->Acquire();
            
        } // End if failed
        else
        {
            // Copy the current mouse button states over
            memcpy( mMouseButtonStates, MouseState.rgbButtons, sizeof( mMouseButtonStates ) );

            // Did the mouse move?
            if ( MouseState.lX != 0 || MouseState.lY != 0 )
            {
                cgRect   rcClient;
                cgPoint  ptClient;
                cgPointF Offset;

                // Compute the offset value
                Offset.x = (cgFloat)MouseState.lX * mConfig.mouseSensitivity;
                Offset.y = (cgFloat)MouseState.lY * mConfig.mouseSensitivity;

                // Retrieve the client rectangle of our focus window and the current cursor position
                rcClient = mFocusWnd->getClientRect( );
                ::GetCursorPos( (POINT*)&ptClient );
                ptClient = mFocusWnd->screenToClient( ptClient );

                // Just store the current position of the cursor in client space
                mMousePosition = ptClient;

                // If we're in direct mouse mode, we should clip (restrict) the
                // cursor to the interior of the window.
                if ( mMouseMode == cgMouseHandlerMode::Direct )
                {
                    cgRect rcClip = mFocusWnd->clientToScreen( rcClient );
                    ::ClipCursor( (RECT*)&rcClip );

                } // End if outside client area

                // Notify all required items
                onMouseMove( mMousePosition, Offset );

            } // End if mouse moved

            // Did the mouse wheel move?
            if ( MouseState.lZ != 0 )
            {
                // Notify all required items
                onMouseWheelScroll( (MouseState.lZ / 120), mMousePosition );

            } // End if mouse wheel moved

            // Any mouse states changed?
            for ( cgInt32 i = 0; i < 5; ++i )
            {
                // Changed?
                if ( (mMouseButtonStates[i] & 0x80) != (mPrevMouseButtonStates[i] & 0x80) )
                {
                    if ( mMouseButtonStates[i] & 0x80 )
                        onMouseButtonDown( (cgInt32)mouseButtonIdxToEnum(i), mMousePosition );
                    else
                        onMouseButtonUp( (cgInt32)mouseButtonIdxToEnum(i), mMousePosition );

                } // End if state of button changed

            } // Next Mouse Button

        } // End if success

    } // End if mouse available

    // Retrieve current time reference for repeat rate processing
    nTime = timeGetTime();
    
    // Poll keyboard state
    if ( mDIKeyboard )
    {
        cgUInt32            nDataElements = mConfig.keyboardBufferSize;
        DIDEVICEOBJECTDATA *pData = CG_NULL;

        // Buffered?
        if ( mConfig.keyboardBufferSize == 0 )
        {
            // Copy old key state data into "previous state" buffer
            memcpy( mPrevKeyStates, mKeyStates, sizeof(mKeyStates) );

            // Clear out the key state buffer
            memset( mKeyStates, 0, sizeof(mKeyStates) );
            
            // Retrieve the state of all keys
            hRet = mDIKeyboard->GetDeviceState( sizeof(mKeyStates), mKeyStates );
            
            // Failed to get state?
            if( FAILED(hRet) ) 
            {
                // Attempt to re-acquire the device
                hRet = mDIKeyboard->Acquire();
            
            } // End if failed

            // Build 'modifier' key states
            nModifiers = 0;
            if ( isKeyPressed( cgKeys::LControl ) ) nModifiers |= cgModifierKeys::LeftControl;
            if ( isKeyPressed( cgKeys::RControl ) ) nModifiers |= cgModifierKeys::RightControl;
            if ( isKeyPressed( cgKeys::LShift   ) ) nModifiers |= cgModifierKeys::LeftShift;
            if ( isKeyPressed( cgKeys::RShift   ) ) nModifiers |= cgModifierKeys::RightShift;
            if ( isKeyPressed( cgKeys::LAlt     ) ) nModifiers |= cgModifierKeys::LeftAlt;
            if ( isKeyPressed( cgKeys::RAlt     ) ) nModifiers |= cgModifierKeys::RightAlt;

            // Any key states changed?
            for ( cgInt32 i = 0; i < 256; ++i )
            {
                // Changed?
                if ( (mKeyStates[i] & 0x80) != (mPrevKeyStates[i] & 0x80) )
                {
                    if ( mKeyStates[i] & 0x80 )
                    {
                        onKeyDown( i, nModifiers );
                        onKeyPressed( i, nModifiers );

                        // Store information about repeat onKeyPressed events that need to be sent
                        KeyRepeatInfo Repeat;
                        Repeat.timePressed       = nTime;
                        Repeat.timeLastSignalled = nTime;
                        mKeyRepeatData[ i ]      = Repeat;

                    } // End if key was pressed
                    else
                    {
                        onKeyUp( i, nModifiers );
                        
                        // Remove from repeat data list
                        mKeyRepeatData.erase( i );

                    } // End if key was released

                } // End if state of button changed

            } // Next Key

        } // End if not buffering keyboard input
        else
        {
            // Copy old key state data into "previous state" buffer
            memcpy( mPrevKeyStates, mKeyStates, sizeof(mKeyStates) );

            // Allocate enough room for buffered data
            pData = new DIDEVICEOBJECTDATA[ mConfig.keyboardBufferSize ];
            
            // Retrieve the device data
            hRet = mDIKeyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), pData, &nDataElements, 0 );

            // Failed to get state?
            if( FAILED(hRet) ) 
            {
                // Attempt to re-acquire the device
                hRet = mDIKeyboard->Acquire();
                nDataElements = 0;
            
            } // End if failed

            // Update current key state buffer
            if ( nDataElements > 0 )
            {
                // Loop through buffered state changes
                for ( cgUInt32 i = 0; i < nDataElements; ++i )
                {
                    // Update state
                    mKeyStates[ pData[i].dwOfs ] = (cgUInt8)pData[i].dwData;

                    // Build 'modifier' key states
                    nModifiers = 0;
                    if ( isKeyPressed( cgKeys::LControl ) ) nModifiers |= cgModifierKeys::LeftControl;
                    if ( isKeyPressed( cgKeys::RControl ) ) nModifiers |= cgModifierKeys::RightControl;
                    if ( isKeyPressed( cgKeys::LShift   ) ) nModifiers |= cgModifierKeys::LeftShift;
                    if ( isKeyPressed( cgKeys::RShift   ) ) nModifiers |= cgModifierKeys::RightShift;
                    if ( isKeyPressed( cgKeys::LAlt     ) ) nModifiers |= cgModifierKeys::LeftAlt;
                    if ( isKeyPressed( cgKeys::RAlt     ) ) nModifiers |= cgModifierKeys::RightAlt;

                    // Pressed or released?
                    if ( (pData[i].dwData & 0x80) )
                    {
                        onKeyDown( (cgInt32)pData[i].dwOfs, nModifiers );
                        onKeyPressed( (cgInt32)pData[i].dwOfs, nModifiers );

                        // Store information about repeat onKeyPressed events that need to be sent
                        KeyRepeatInfo Repeat;
                        Repeat.timePressed       = nTime;
                        Repeat.timeLastSignalled = nTime;
                        mKeyRepeatData[ (cgInt32)pData[i].dwOfs ] = Repeat;

                    } // End if key was pressed
                    else
                    {
                        onKeyUp( (cgInt32)pData[i].dwOfs, nModifiers );
                        
                        // Remove from repeat data list
                        mKeyRepeatData.erase( (cgInt32)pData[i].dwOfs );

                    } // End if key was released

                } // Next state change
            
            } // End if anything changed

        } // End if buffering keyboard input

        // Release buffered data storage array if allocated
        if ( pData != CG_NULL )
            delete []pData;

        // Build current 'control' key states for repeat processing
        nModifiers = 0;
        if ( isKeyPressed( cgKeys::LControl ) ) nModifiers |= cgModifierKeys::LeftControl;
        if ( isKeyPressed( cgKeys::RControl ) ) nModifiers |= cgModifierKeys::RightControl;
        if ( isKeyPressed( cgKeys::LShift   ) ) nModifiers |= cgModifierKeys::LeftShift;
        if ( isKeyPressed( cgKeys::RShift   ) ) nModifiers |= cgModifierKeys::RightShift;
        if ( isKeyPressed( cgKeys::LAlt     ) ) nModifiers |= cgModifierKeys::LeftAlt;
        if ( isKeyPressed( cgKeys::RAlt     ) ) nModifiers |= cgModifierKeys::RightAlt;

        // Iterate through the repeat messages that need to be sent.
        KeyRepeatMap::iterator itRepeat;
        for ( itRepeat = mKeyRepeatData.begin(); itRepeat != mKeyRepeatData.end(); ++itRepeat )
        {
            KeyRepeatInfo & Info  = itRepeat->second;

            // Delay expired?
            if ( nTime > (Info.timePressed + mConfig.keyRepeatDelay) )
            {
                // Has the first 'post-delay' message been sent yet?
                if ( Info.timeLastSignalled == Info.timePressed )
                {
                    // Send the message
                    onKeyPressed( itRepeat->first, nModifiers );
                    Info.timeLastSignalled += mConfig.keyRepeatDelay;
                
                } // End if post-delay message not sent
                else if ( nTime > (Info.timeLastSignalled + mConfig.keyRepeatRate) )
                {
                    // Send the message
                    onKeyPressed( itRepeat->first, nModifiers );
                    Info.timeLastSignalled += mConfig.keyRepeatRate;

                } // End if apply repeat
            
            } // End if delay has expired

        } // Next key

    } // End if keyboard available
}

//-----------------------------------------------------------------------------
//  Name : onMouseMove () (Private)
/// <summary>
/// Called by the input driver whenever the mouse moves at all.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    cgMessage                   Msg;
    cgMouseMoveEventArgs        Data;
    EventListenerList::iterator itListener;

    // Trigger 'onMouseMove' of all listeners.
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
        ((cgInputListener*)(*itListener))->onMouseMove( Position, Offset );

    // Build the message for anyone listening via messaging system
    Data.position   = Position;
    Data.offset     = Offset;
    Msg.messageId   = cgSystemMessages::InputDriver_MouseMoved;
    Msg.messageData = &Data;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_MouseInput, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Private)
/// <summary>
/// Called by the input driver whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
{
    cgMessage                   Msg;
    cgMouseButtonEventArgs      Data;
    EventListenerList::iterator itListener;

    // Trigger 'onMouseButtonDown' of all listeners.
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
        ((cgInputListener*)(*itListener))->onMouseButtonDown( nButtons, Position );

    // Build the message for anyone listening via messaging system
    Data.buttons     = nButtons;
    Data.position     = Position;
    Msg.messageId     = cgSystemMessages::InputDriver_MouseButtonDown;
    Msg.messageData   = &Data;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_MouseInput, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonUp () (Private)
/// <summary>
/// Called by the input driver whenever a mouse button is released.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
{
    cgMessage                   Msg;
    cgMouseButtonEventArgs      Data;
    EventListenerList::iterator itListener;

    // Trigger 'onMouseButtonUp' of all listeners.
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
        ((cgInputListener*)(*itListener))->onMouseButtonUp( nButtons, Position );

    // Build the message for anyone listening via messaging system
    Data.buttons     = nButtons;
    Data.position     = Position;
    Msg.messageId     = cgSystemMessages::InputDriver_MouseButtonUp;
    Msg.messageData   = &Data;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_MouseInput, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onMouseWheelScroll () (Private)
/// <summary>
/// Called by the input driver whenever the mouse wheel is scrolled.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::onMouseWheelScroll( cgInt32 nDelta, const cgPoint & Position )
{
    cgMessage                   Msg;
    cgMouseWheelScrollEventArgs Data;
    EventListenerList::iterator itListener;

    // Trigger 'onMouseButtonUp' of all listeners.
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
        ((cgInputListener*)(*itListener))->onMouseWheelScroll( nDelta, Position );

    // Build the message for anyone listening via messaging system
    Data.delta     = nDelta;
    Data.position   = Position;
    Msg.messageId   = cgSystemMessages::InputDriver_MouseWheelScroll;
    Msg.messageData = &Data;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_MouseInput, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onKeyDown () (Private)
/// <summary>
/// Called by the input driver whenever a key is first pressed.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::onKeyDown( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    cgMessage                   Msg;
    cgKeyEventArgs              Data;
    EventListenerList::iterator itListener;

    // Trigger 'onKeyDown' of all listeners.
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
        ((cgInputListener*)(*itListener))->onKeyDown( nKeyCode, nModifiers );

    // Build the message for anyone listening via messaging system
    Data.keyCode      = nKeyCode;
    Data.modifiers    = nModifiers;
    Msg.messageId      = cgSystemMessages::InputDriver_KeyDown;
    Msg.messageData    = &Data;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_KeyboardInput, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onKeyUp () (Private)
/// <summary>
/// Called by the input driver whenever a key is released.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::onKeyUp( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    cgMessage                   Msg;
    cgKeyEventArgs              Data;
    EventListenerList::iterator itListener;

    // Trigger 'onKeyUp' of all listeners.
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
        ((cgInputListener*)(*itListener))->onKeyUp( nKeyCode, nModifiers );

    // Build the message for anyone listening via messaging system
    Data.keyCode      = nKeyCode;
    Data.modifiers    = nModifiers;
    Msg.messageId      = cgSystemMessages::InputDriver_KeyUp;
    Msg.messageData    = &Data;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_KeyboardInput, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onKeyPressed () (Private)
/// <summary>
/// Called by the input driver whenever a key is first pressed OR if the
/// key repeat is fired.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputDriver::onKeyPressed( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    cgMessage                   Msg;
    cgKeyEventArgs              Data;
    EventListenerList::iterator itListener;

    // Trigger 'onKeyPressed' of all listeners.
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
        ((cgInputListener*)(*itListener))->onKeyPressed( nKeyCode, nModifiers );

    // Build the message for anyone listening via messaging system
    Data.keyCode      = nKeyCode;
    Data.modifiers    = nModifiers;
    Msg.messageId      = cgSystemMessages::InputDriver_KeyPressed;
    Msg.messageData    = &Data;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_KeyboardInput, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : wasKeyPressed ()
/// <summary>
/// Simple query routine to determine if a key was pressed in the
/// previous call to poll.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputDriver::wasKeyPressed( cgInt32 nKeyCode ) const
{
    // Was key pressed?
    return ((mPrevKeyStates[ nKeyCode ] & 0x80) != 0);
}

//-----------------------------------------------------------------------------
//  Name : isKeyPressed ()
/// <summary>
/// Simple query routine to determine if a key is currently pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputDriver::isKeyPressed( cgInt32 nKeyCode ) const
{
    // Is key pressed?
    return ((mKeyStates[ nKeyCode ] & 0x80) != 0);
}

//-----------------------------------------------------------------------------
//  Name : wasMouseButtonPressed ()
/// <summary>
/// Simple query routine to determine if a mouse button was previously
/// pressed in the previous call to poll.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputDriver::wasMouseButtonPressed( cgMouseButtons::Base Button ) const
{
    cgInt32 nIndex = mouseButtonEnumToIdx( Button );
    if ( nIndex < 0 )
        return false;

    // Is button pressed?
    return ((mPrevMouseButtonStates[ nIndex ] & 0x80) != 0);
}

//-----------------------------------------------------------------------------
//  Name : isMouseButtonPressed ()
/// <summary>
/// Simple query routine to determine if a mouse button is currently 
/// pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputDriver::isMouseButtonPressed( cgMouseButtons::Base Button ) const
{
    cgInt32 nIndex = mouseButtonEnumToIdx( Button );
    if ( nIndex < 0 )
        return false;
    
    // Is button pressed?
    return ((mMouseButtonStates[ nIndex ] & 0x80) != 0);
}

//-----------------------------------------------------------------------------
//  Name : getCursorPosition ()
/// <summary>
/// Retrieve the current position of the mouse cursor when in cursor mouse mode.
/// </summary>
//-----------------------------------------------------------------------------
const cgPoint & cgInputDriver::getCursorPosition( ) const
{
    return mMousePosition;
}

//-----------------------------------------------------------------------------
//  Name : keyCodeToCharacter ()
/// <summary>
/// Given the specified key code, convert it into a character we can
/// apply to a string.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgInputDriver::keyCodeToCharacter( cgInt32 nKeyCode )
{
    static HKL     KeyboardLayout = GetKeyboardLayout(CG_NULL);
    static cgUInt8 KeyboardState[256];
    
    // ToDo : Note - "Tab key not currently supported in text renderer, so we do not return it as a valid character here.

    // Some known codes?
    switch ( nKeyCode )
    {
        case DIK_RETURN:
            return _T("\n");
        case DIK_TAB:
            return _T("");
        case DIK_ESCAPE:
            return _T("");
    
    } // End Switch nKeyCode

    // Bail if control or alt keys are held down
    if ( (mKeyStates[ DIK_LMENU ] | mKeyStates[ DIK_RMENU ]) != 0 ||
         (mKeyStates[ DIK_LCONTROL ] | mKeyStates[ DIK_RCONTROL ]) != 0 )
         return _T("");

    // Populate important keyboard states from our own state (in case of buffering, the standard
    // method for calling GetKeyboardState will not necessarily contain the correct data at the
    // time this method was called)
    memset( KeyboardState, 0, 256 );
    KeyboardState[ VK_SHIFT   ] = mKeyStates[ DIK_LSHIFT ] | mKeyStates[ DIK_RSHIFT ];
    KeyboardState[ VK_CAPITAL ] = (GetKeyState( VK_CAPITAL ) & 0x1); // Toggle State

    // Convert scan code to a virtual key code
    cgUInt nVirtualKeyCode = MapVirtualKeyEx( nKeyCode, 1, KeyboardLayout );

    // Convert to an ascii character code
    cgUInt16 nAsciiChar;
    cgInt nResult = ToAsciiEx( nVirtualKeyCode, nKeyCode, KeyboardState, &nAsciiChar, 0, KeyboardLayout );

    // Test the result of the conversion
    if ( nResult == 0 )
    {
        return _T("");
    
    } // End if no conversion
    else if ( nResult == 1 || nResult == 2 )
    {
        // Potentially add deadcode conversion. For now just 
        // return the string representation of the ascii character 
        return cgString( 1, (cgString::value_type)(((cgByte*)&nAsciiChar)[0]) );

    } // End if 1 byte result
    
    // Not representable
    return _T("");
}

//-----------------------------------------------------------------------------
//  Name : mouseButtonIdxToEnum () (Static)
/// <summary>
/// Convert the specified mouse button index into a 'flags' compatible enum
/// item.
/// </summary>
//-----------------------------------------------------------------------------
cgMouseButtons::Base cgInputDriver::mouseButtonIdxToEnum( cgInt32 nButton )
{
    switch ( nButton )
    {
        case 0:
            return cgMouseButtons::Left;
        case 1:
            return cgMouseButtons::Right;
        case 2:
            return cgMouseButtons::Middle;
        case 3:
            return cgMouseButtons::XButton1;
        case 4:
            return cgMouseButtons::XButton2;
    
    } // End switch index
    
    // Unknown
    return cgMouseButtons::None;
}

//-----------------------------------------------------------------------------
//  Name : mouseButtonEnumToIdx () (Static)
/// <summary>
/// Convert the specified mouse button enum into its original index.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgInputDriver::mouseButtonEnumToIdx( cgMouseButtons::Base Button )
{
    switch ( Button )
    {
        case cgMouseButtons::Left:
            return 0;
        case cgMouseButtons::Right:
            return 1;
        case cgMouseButtons::Middle:
            return 2;
        case cgMouseButtons::XButton1:
            return 3;
        case cgMouseButtons::XButton2:
            return 4;
    
    } // End switch index
    
    // Unknown
    return -1;
}