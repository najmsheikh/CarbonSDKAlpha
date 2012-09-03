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
// Name : cgTexture.cpp                                                      //
//                                                                           //
// Desc : Contains classes responsible for loading and managing texture      //
//        resource data.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgTexture Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgTexture.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/cgResourceTypes.h>
#include <System/cgThreading.h>

// Platform specific implementations
#include <Resources/Platform/cgDX9Texture.h>
#include <Resources/Platform/cgDX11Texture.h>

// ToDo: Video playback support needs to provide looping ability.

///////////////////////////////////////////////////////////////////////////////
// cgTexture Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTexture () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTexture::cgTexture( cgUInt32 referenceId, const cgInputStream & stream, cgRenderDriver * driver, cgInt32 mipLevels /* = -1 */ ) : cgResource( referenceId )
{
    // Initialize variables to sensible defaults
    mInputStream          = stream;
    mMipLevels            = mipLevels;
    mSource               = Source_Stream;
    mLocked               = false;
    mLockedBuffer         = CG_NULL;
    mMediaDecoder         = CG_NULL;
    mVideoSyncEvent       = CG_NULL;
    mLastMipUpdateTime    = 0.0;
    mMipsDirty            = true;
    mCurrentCubeFace      = PositiveX;

    // Select default texture details.
    mInfo.pool   = cgMemoryPool::Managed;
    mInfo.format = cgBufferFormat::Unknown;
    
    // Select default media configuration.
    mMediaConfig.mipmapUpdateRate = 20.0f;
    
    // Cached responses
    mResourceType = cgResourceType::Texture;
    mCanEvict     = ( mInputStream.getType() == cgStreamType::File || mInputStream.getType() == cgStreamType::MappedFile );
}

//-----------------------------------------------------------------------------
//  Name : cgTexture () (Overload Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTexture::cgTexture( cgUInt32 referenceId, const cgImageInfo & description ) : cgResource( referenceId )
{
    // Initialize variables to sensible defaults
    mLocked             = false;
    mLockedBuffer       = CG_NULL;
    mMediaDecoder       = CG_NULL;
    mVideoSyncEvent     = CG_NULL;
    mLastMipUpdateTime  = 0.0;
    mMipsDirty          = true;
    mInfo               = description;
    mCurrentCubeFace    = PositiveX;
    
    // Select default media configuration.
    mMediaConfig.mipmapUpdateRate = 15.0f;

    // ToDo: Support video type?
    
    // Cached responses
    mSource       = Source_Created;
    mResourceType = cgResourceType::Texture;
    mCanEvict     = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgTexture () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTexture::~cgTexture( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgTexture * cgTexture::createInstance( cgUInt32 referenceId, const cgInputStream & stream, cgRenderDriver * driver, cgInt32 mipLevels /* = -1 */ )
{
    // Determine which state type we should create.
    const CGEConfig & config = cgGetEngineConfig();
    if ( config.platform == cgPlatform::Windows )
    {
        switch ( config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )
            
            case cgRenderAPI::DirectX9:
                return new cgDX9Texture<cgTexture>( referenceId, stream, driver, mipLevels );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11Texture<cgTexture>( referenceId, stream, driver, mipLevels );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgTexture * cgTexture::createInstance( cgUInt32 referenceId, const cgImageInfo & description )
{
    // Determine which state type we should create.
    const CGEConfig & config = cgGetEngineConfig();
    if ( config.platform == cgPlatform::Windows )
    {
        switch ( config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9Texture<cgTexture>( referenceId, description );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )
            
            case cgRenderAPI::DirectX11:
                return new cgDX11Texture<cgTexture>( referenceId, description );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgTexture::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated memory
    if ( mMediaDecoder != CG_NULL )
        delete mMediaDecoder;
    if ( mVideoSyncEvent != CG_NULL )
        delete mVideoSyncEvent;

    // Clear variables
    mInputStream.reset();
    mMediaDecoder   = CG_NULL;
    mVideoSyncEvent = CG_NULL;

    // Dispose base(s).
    if ( disposeBase )
        cgResource::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexture::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_TextureResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getInfo ()
/// <summary>
/// Retrieve information about the texture such as size, format etc.
/// </summary>
//-----------------------------------------------------------------------------
const cgImageInfo & cgTexture::getInfo( ) const
{
    return mInfo;
}

//-----------------------------------------------------------------------------
//  Name : getSize ()
/// <summary>
/// Retrieve the dimensions of the texture in pixels.
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgTexture::getSize( ) const
{
    return cgSize( mInfo.width, mInfo.height );
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexture::loadResource( )
{
    // Create the notification event handle in the case of a video
    // and start playing.
    if ( mMediaDecoder )
    {
        mVideoSyncEvent = cgEvent::createInstance( );
        mMediaDecoder->play();
    
    } // End if media

    // We're now loaded
    mResourceLoaded = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexture::unloadResource( )
{
    // Stop any video.
    if ( mMediaDecoder )
        mMediaDecoder->stop();

    // Release any handles
    delete mVideoSyncEvent;
    mVideoSyncEvent = CG_NULL;

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Allows the texture to update if it is decoding media perhaps.
/// </summary>
//-----------------------------------------------------------------------------
void cgTexture::update()
{
	// Only video textures need further updating.
    if ( mInfo.type != cgBufferType::Video )
        return;

    // Determine if data update event has been signaled
    if ( mVideoSyncEvent->hasSignaled() )
    {
        // Lock the surface.
        cgUInt32 pitch = 0;
        void   * data  = lock( pitch, cgLockFlags::Discard | cgLockFlags::NoDirtyUpdate );

        // Retrieve data if lock was successful.
        if ( data )
        {
            mMediaDecoder->getCurrentVideoFrame( data, pitch );
            unlock( false );

            // Are the mip maps dirty now?
            if ( mInfo.autoGenerateMipmaps == true )
                mMipsDirty = true;
        
        } // End if locked

    } // End if new data available
}

//-----------------------------------------------------------------------------
//  Name : LockVideoFrame () (Virtual)
/// <summary>
/// A new video frame is ready to be displayed.
/// </summary>
//-----------------------------------------------------------------------------
giEXResult cgTexture::onNewVideoFrame( void * data, cgUInt32 size )
{
    // Signal our main thread that a new frame of video is available.
    if ( mVideoSyncEvent != CG_NULL )
        mVideoSyncEvent->signal();

    // Success!
    return GIEX_Success;
}

//-----------------------------------------------------------------------------
//  Name : configureMedia ()
/// <summary>
/// Allows the user to adjust the media playback settings (i.e. for
/// video playback etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgTexture::configureMedia( const MediaConfig & config )
{
    mMediaConfig = config;
}

//-----------------------------------------------------------------------------
// Name : getCurrentCubeFace ( )
/// <summary>
/// Retrieve the cube face that is currently set for use when reading from or 
/// manipulating individual faces of a cube map texture (i.e. lock())
/// </summary>
//-----------------------------------------------------------------------------
cgTexture::CubeFace cgTexture::getCurrentCubeFace( ) const
{
    return mCurrentCubeFace;
}

//-----------------------------------------------------------------------------
// Name : setCurrentCubeFace ( )
/// <summary>
/// Select the cube face to use when reading from or manipulating individual
/// faces of a cube map texture (i.e. lock())
/// </summary>
//-----------------------------------------------------------------------------
void cgTexture::setCurrentCubeFace( CubeFace Face )
{
    mCurrentCubeFace = Face;
}