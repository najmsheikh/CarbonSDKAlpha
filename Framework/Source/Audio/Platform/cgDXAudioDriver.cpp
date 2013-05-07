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
// Name : cgDXAudioDriver.cpp                                                //
//                                                                           //
// Desc : The main device through which we may load / play sound effects and //
//        music (DirectX class).                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// Module Local Defines
//-----------------------------------------------------------------------------
#define DIRECTSOUND_VERSION 0x0800

//-----------------------------------------------------------------------------
// cgDXAudioDriver Module Includes
//-----------------------------------------------------------------------------
#include <Audio/Platform/cgDXAudioDriver.h>
#include <System/cgStringUtility.h>
#include <System/Platform/cgWinAppWindow.h>
#include <Math/cgMathTypes.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Mmsystem.h>
#include <dsound.h>
#undef WIN32_LEAN_AND_MEAN

///////////////////////////////////////////////////////////////////////////////
// cgDXAudioDriver Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDXAudioDriver () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDXAudioDriver::cgDXAudioDriver()
{
    // Initialize variables to sensible defaults
    mDS         = CG_NULL;
    m3DListener = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDXAudioDriver () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDXAudioDriver::~cgDXAudioDriver()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDXAudioDriver::dispose( bool bDisposeBase )
{
    // Release DirectX Objects
    if ( m3DListener != CG_NULL )
        m3DListener->Release();
    if ( mDS != CG_NULL )
        mDS->Release();

    // Clear variables
    m3DListener   = CG_NULL;
    mDS           = CG_NULL;
    
    // Dispose base class if requested.
    if ( bDisposeBase == true )
        cgAudioDriver::dispose( true );

}

//-----------------------------------------------------------------------------
//  Name : loadConfig ()
/// <summary>
/// Load the audio driver configuration from the file specified.
/// </summary>
//-----------------------------------------------------------------------------
cgConfigResult::Base cgDXAudioDriver::loadConfig( const cgString & strFileName )
{
    DSCAPS Caps;
    
    // Fail if config already loaded
    if ( mConfigLoaded == true )
        return cgConfigResult::Error;

    // Have we already created a DirectSound object?
    if ( mDS == CG_NULL )
    {
        // Create a Direct Sound Object (This is needed for capabilities testing etc).
        if ( FAILED( DirectSoundCreate8( CG_NULL, &mDS, CG_NULL ) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T( "No compatible DirectSound object could be created.\n" ) );
            return cgConfigResult::Error;

        } // End if failure

    } // End if direct sound object exists

    // Retrieve primary device capabilities
    memset( &Caps, 0, sizeof(DSCAPS) );
    Caps.dwSize = sizeof( DSCAPS );
    if ( FAILED( mDS->GetCaps( &Caps ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T( "Failed to retrieve the capability information structure of the DirectSound device.\n" ) );
        return cgConfigResult::Error;
    
    } // End if failed to get caps

    // Retrieve configuration options if provided
    if ( strFileName.empty() == false )
    {
        mConfig.sampleRate = GetPrivateProfileInt( _T("AudioDriver"), _T("SampleRate"), min( 48000, Caps.dwMaxSecondarySampleRate ), strFileName.c_str() );
        mConfig.channels   = GetPrivateProfileInt( _T("AudioDriver"), _T("Channels"), (Caps.dwFlags & DSCAPS_PRIMARYSTEREO) ? 2 : 1, strFileName.c_str() );
        mConfig.bitRate    = GetPrivateProfileInt( _T("AudioDriver"), _T("BitRate"), (Caps.dwFlags & DSCAPS_PRIMARY16BIT) ? 16 : 8, strFileName.c_str() );
        
    } // End if config provided
    
    // Test the capabilities of the hardware to determine if the specified
    // configuration options can be succesfully applied.
    bool bMismatch = false;
    if ( mConfig.bitRate != 16 && mConfig.bitRate != 8 )
        bMismatch = true;
    else if ( mConfig.bitRate == 16 && (Caps.dwFlags & DSCAPS_PRIMARY16BIT) == 0 )
        bMismatch = true;
    else if ( mConfig.bitRate == 8 && (Caps.dwFlags & DSCAPS_PRIMARY8BIT) == 0 )
        bMismatch = true;

    // Bitrate mismatch?
    if ( bMismatch == true )
    {
        if ( strFileName.empty() == true )
        {
            cgAppLog::write( cgAppLog::Error, _T( "DirectSound device does not support the selected bitrate of %i (Capabilities: 0x%x).\n"), mConfig.bitRate, Caps.dwFlags );
            return cgConfigResult::Error;
        
        } // End if asked to find defaults

        // Simple mismatch, we'll find defaults later
        return cgConfigResult::Mismatch;
    
    } // End if bitrate unknown or unsupported

    bMismatch = false;
    if ( mConfig.channels != 2 && mConfig.channels != 1 )
        bMismatch = true;
    else if ( mConfig.channels == 2 && !(Caps.dwFlags & DSCAPS_PRIMARYSTEREO) )
        bMismatch = true;
    else if ( mConfig.channels == 1 && !(Caps.dwFlags & DSCAPS_PRIMARYMONO) )
        bMismatch = true;
    
    // Channel count mismatch?
    if ( bMismatch == true )
    {
        if ( strFileName.empty() == true )
        {
            cgAppLog::write( cgAppLog::Error, _T( "DirectSound device does not support the selected channel count of %i (Capabilities: 0x%x).\n"), mConfig.channels, Caps.dwFlags );
            return cgConfigResult::Error;

        } // End if asked to find defaults

        // Simple mismatch, we'll find defaults later
        return cgConfigResult::Mismatch;

    } // End if channel count unknown or unsupported

    // ToDo: Remove?
    /*if ( mConfig.sampleRate < Caps.dwMinSecondarySampleRate || mConfig.sampleRate > Caps.dwMaxSecondarySampleRate )
    {
        if ( pFileName == CG_NULL )
        {
            // Final failure.
            _tprintf(_T( "DirectSound device does not support the specified sample rate of %i (Capabilities: %i-%i).\n"), 
                            mConfig.sampleRate, Caps.dwMinSecondarySampleRate, Caps.dwMaxSecondarySampleRate );
            return ConfigStatus::Error;
        } // End if asked to find defaults
        
        // Simple mismatch, we'll find defaults later
        return ConfigStatus::Mismatch;
    
    } // End if sample rate outside supported range*/
    
    // Signal that we have settled on good config options
    mConfigLoaded = true;

    // Options are valid. Success!!
    return cgConfigResult::Valid;
}

//-----------------------------------------------------------------------------
//  Name : loadDefaultConfig ()
/// <summary>
/// Load a default configuration for the render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgConfigResult::Base cgDXAudioDriver::loadDefaultConfig( )
{
    DSCAPS Caps;

    // Have we already created a DirectSound object?
    if ( mDS == CG_NULL )
    {
        // Create a Direct Sound Object (This is needed for capabilities testing etc).
        if ( FAILED( DirectSoundCreate8( CG_NULL, &mDS, CG_NULL ) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T( "No compatible DirectSound object could be created.\n" ) );
            return cgConfigResult::Error;

        } // End if failure

    } // End if direct sound object exists

    // Retrieve primary device capabilities
    memset( &Caps, 0, sizeof(DSCAPS) );
    Caps.dwSize = sizeof( DSCAPS );
    if ( FAILED( mDS->GetCaps( &Caps ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T( "Failed to retrieve the capability information structure of the DirectSound device.\n" ) );
        return cgConfigResult::Error;
    
    } // End if failed to get caps

    // Build default configuration
    mConfig.sampleRate = min( 48000, Caps.dwMaxSecondarySampleRate );
    mConfig.channels   = (Caps.dwFlags & DSCAPS_PRIMARYSTEREO) ? 2 : 1;
    mConfig.bitRate    = (Caps.dwFlags & DSCAPS_PRIMARY16BIT) ? 16 : 8;

    // Pass through to the loadConfig function
    return loadConfig( _T("") );
}

//-----------------------------------------------------------------------------
//  Name : saveConfig ()
/// <summary>
/// Save the audio driver configuration from the file specified.
/// Note : When specifying the save filename, it's important to either use a
/// full path ("C:\\{Path}\\Config.ini") or a path relative to the
/// current directory INCLUDING the first period (".\\Config.ini"). If
/// not, windows will place the ini in the windows directory rather than
/// the application dir.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDXAudioDriver::saveConfig( const cgString & strFileName )
{
    // Validate requirements
    if ( strFileName.empty() == true )
        return false;

    // Save configuration options
    cgStringUtility::writePrivateProfileIntEx( _T("AudioDriver"), _T("SampleRate"), mConfig.sampleRate, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( _T("AudioDriver"), _T("Channels"), mConfig.channels, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( _T("AudioDriver"), _T("BitRate"), mConfig.bitRate, strFileName.c_str() );
    
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Virtual)
/// <summary>
/// Initialize the audio device
/// </summary>
//-----------------------------------------------------------------------------
bool cgDXAudioDriver::initialize( cgResourceManager * pResources, cgAppWindow * pFocusWnd )
{
    LPDIRECTSOUNDBUFFER pPrimaryBuffer;
    DSBUFFERDESC        BufferDesc;
    WAVEFORMATEX        Format;

    // Configuration must be loaded
    if ( mConfigLoaded == false )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Audio driver configuration must be loaded prior to initialization!\n"));
        return false;

    } // End if no config loaded

    // Only 'cgWinAppWindow' type is supported.
    cgWinAppWindow * pWinAppWindow = dynamic_cast<cgWinAppWindow*>(pFocusWnd);
    if ( pWinAppWindow == NULL )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The DirectX audio driver is only supported on the Windows(tm) platform.\n"));
        return false;

    } // End if invalid cast

    // Set direct sound device cooperative level
    if( FAILED( mDS->SetCooperativeLevel( pWinAppWindow->getWindowHandle(), DSSCL_PRIORITY ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set audio device to 'Priority' cooperative level.\n"));
        return false;

    } // End if failed to set coop level

    // Specify primary buffer description
    memset( &BufferDesc, 0, sizeof(DSBUFFERDESC) );
    BufferDesc.dwSize        = sizeof(DSBUFFERDESC);
    BufferDesc.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    BufferDesc.dwBufferBytes = 0;
    BufferDesc.lpwfxFormat   = CG_NULL;

    // Create the primary buffer
    if( FAILED( mDS->CreateSoundBuffer( &BufferDesc, &pPrimaryBuffer, CG_NULL ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to get access to primary audio buffer.\n"));
        return false;

    } // End if failed to create primary buffer

    // Specify primary buffer output format
    memset( &Format, 0, sizeof(WAVEFORMATEX) ); 
    Format.wFormatTag      = WAVE_FORMAT_PCM; 
    Format.nChannels       = mConfig.channels; 
    Format.nSamplesPerSec  = mConfig.sampleRate; 
    Format.wBitsPerSample  = mConfig.bitRate; 
    Format.nBlockAlign     = (Format.wBitsPerSample / 8) * Format.nChannels;
    Format.nAvgBytesPerSec = Format.nSamplesPerSec * (cgUInt32)Format.nBlockAlign;

    // Set the primary buffer format
    if( FAILED( pPrimaryBuffer->SetFormat( &Format ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set primary audio buffer format.\n") );

        // Clean up and bail
        pPrimaryBuffer->Release();
        return false;

    } // End if failed to set format

    // We're done with the buffer
    pPrimaryBuffer->Release();

    // Cache a reference to the 3D listener interface.
    m3DListener = get3DListenerInterface();
    if ( m3DListener == CG_NULL )
    {
        cgAppLog::write( cgAppLog::Error, _T("Primary DirectSound buffer does not implement the 3D listener interface.\n") );
        return false;

    } // End if no 3D Interface

    // Call base class
    return cgAudioDriver::initialize( pResources, pFocusWnd );
}

//-----------------------------------------------------------------------------
//  Name : set3DWorldScale () (Virtual)
/// <summary>
/// Set the scale of the world in which the sounds exist.
/// </summary>
//-----------------------------------------------------------------------------
void cgDXAudioDriver::set3DWorldScale( cgFloat fUnitsPerMeter )
{
    if ( m3DListener == CG_NULL )
        return;
    
    // Store parameter (don't apply immediately, wait for 'CommitDeferredSettings()')
    m3DListener->SetDistanceFactor( 1.0f / fUnitsPerMeter, DS3D_DEFERRED );
}

//-----------------------------------------------------------------------------
//  Name : set3DRolloffFactor () (Virtual)
/// <summary>
/// Set the rate at which audio is attenuated based on distance. 1.0 = real
/// world, 0.0 = no attenuation.
/// </summary>
//-----------------------------------------------------------------------------
void cgDXAudioDriver::set3DRolloffFactor( cgFloat factor )
{
    if ( m3DListener == CG_NULL )
        return;

    // Store parameter (don't apply immediately, wait for 'CommitDeferredSettings()')
    m3DListener->SetRolloffFactor( factor, DS3D_DEFERRED );
}

//-----------------------------------------------------------------------------
//  Name : set3DListenerTransform () (Virtual)
/// <summary>
/// Set the position and orientation of the listener relative to the 
/// world.
/// </summary>
//-----------------------------------------------------------------------------
void cgDXAudioDriver::set3DListenerTransform( const cgTransform & t )
{
    if ( m3DListener == CG_NULL )
        return;
    
    // Store parameters (don't apply immediately, wait for 'CommitDeferredSettings()')
    cgVector3 vecUp   = t.yUnitAxis();
    cgVector3 vecLook = t.zUnitAxis();
    cgVector3 vecPos  = t.position();
    m3DListener->SetOrientation( vecLook.x, vecLook.y, vecLook.z,
                                   vecUp.x, vecUp.y, vecUp.z, DS3D_DEFERRED );
    m3DListener->SetPosition( vecPos.x, vecPos.y, vecPos.z, DS3D_DEFERRED );

    // ToDo: Need velocity?
}

//-----------------------------------------------------------------------------
//  Name : get3DListenerInterface () (Protected)
/// <summary>
/// Internal function make accessing the DirectSound 3D interfaces
/// much less painful.
/// </summary>
//-----------------------------------------------------------------------------
LPDIRECTSOUND3DLISTENER cgDXAudioDriver::get3DListenerInterface( )
{
    DSBUFFERDESC            Desc;
    LPDIRECTSOUNDBUFFER     pDSPrimary    = CG_NULL;
    LPDIRECTSOUND3DLISTENER pDS3DListener = CG_NULL;

    // Validate requirements
    if ( mDS == CG_NULL )
        return CG_NULL;

    // First retrieve the primary buffer (with 3D control)
    ZeroMemory( &Desc, sizeof(DSBUFFERDESC) );
    Desc.dwSize = sizeof(DSBUFFERDESC);
    Desc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
    if ( FAILED( mDS->CreateSoundBuffer( &Desc, &pDSPrimary, CG_NULL ) ) )
        return CG_NULL;

    // Query for the 3D Listener interface
    if ( FAILED( pDSPrimary->QueryInterface( IID_IDirectSound3DListener, (void**)&pDS3DListener ) ) )
    {
        pDSPrimary->Release();
        return CG_NULL;
    
    } // End if failed query

    // Release the (temporary) primary buffer
    pDSPrimary->Release();

    // Setup necessary defaults for the listener.
    pDS3DListener->SetRolloffFactor( 1.0f, DS3D_IMMEDIATE );

    // Return the interface
    return pDS3DListener;
}

//-----------------------------------------------------------------------------
//  Name : apply3DSettings () (Protected Virtual)
/// <summary>
/// Apply any 3D settings that were specified after the last application.
/// </summary>
//-----------------------------------------------------------------------------
void cgDXAudioDriver::apply3DSettings( )
{
    if ( m3DListener == CG_NULL )
        return;

    // Commit the changes to the hardware (will also apply any 
    // changes made by individual audio buffers)
    m3DListener->CommitDeferredSettings( );
}

//-----------------------------------------------------------------------------
//  Name : getDirectSound()
/// <summary>
/// Retrieve the internal IDirectSound8 object.
/// </summary>
//-----------------------------------------------------------------------------
IDirectSound8 * cgDXAudioDriver::getDirectSound( )
{
    // Add a reference, we're returning a pointer.
    if ( mDS != NULL )
        mDS->AddRef();
    return mDS;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDXAudioDriver::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DXAudioDriver )
        return true;

    // Supported by base?
    return cgAudioDriver::queryReferenceType( type );
}