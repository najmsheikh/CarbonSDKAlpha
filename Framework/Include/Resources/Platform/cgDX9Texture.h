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
// Name : cgDX9Texture.h                                                     //
//                                                                           //
// Desc : Contains classes responsible for loading and managing texture      //
//        resource data (DX9 implementation).                                //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX9TEXTURE_H_ )
#define _CGE_CGDX9TEXTURE_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9Texture Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceTypes.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <Resources/Platform/cgDX9BufferFormatEnum.h>
#include <System/Platform/cgWinAppWindow.h>
#include <System/cgImage.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct IDirect3DBaseTexture9;
class cgTexture;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {DF8F6B50-9ED3-448D-AE31-10E03089335F}
const cgUID RTID_DX9TextureResource = {0xDF8F6B50, 0x9ED3, 0x448D, {0xAE, 0x31, 0x10, 0xE0, 0x30, 0x89, 0x33, 0x5F}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9Texture (Class)
/// <summary>
/// Wrapper for managing texture resources (DX9 implementation).
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
class CGE_API cgDX9Texture : public _BaseClass
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9Texture( cgUInt32 referenceId, const cgInputStream & stream, cgRenderDriver * driver, cgInt32 mipLevels = -1 );
             cgDX9Texture( cgUInt32 referenceId, const cgImageInfo & description );
    virtual ~cgDX9Texture( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DBaseTexture9 * getD3DTexture       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgTexture)
    //-------------------------------------------------------------------------
    virtual void            update              ( );
    virtual void          * lock                ( cgUInt32 & pitch, cgUInt32 flags );
    virtual void          * lock                ( const cgRect & bounds, cgUInt32 & pitch, cgUInt32 flags );
    virtual void            unlock              ( bool updateMips = false );
    virtual bool            getImageData        ( cgImage & imageOut );
    virtual bool            updateMipLevels     ( );
    virtual bool            clone               ( cgTexture * destinationTexture, const cgRect & sourceRectangle, const cgRect & destinationRectangle );
    virtual bool            isValid             ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgResource)
    //-------------------------------------------------------------------------
    virtual void            deviceLost          ( );
    virtual void            deviceRestored      ( );
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9TextureResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool            createTexture       ( );
    virtual void            releaseTexture      ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    IDirect3DBaseTexture9 * mTexture;   // Underlying Resource
};

///////////////////////////////////////////////////////////////////////////////
// cgDX9Texture Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9Texture () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
cgDX9Texture<_BaseClass>::cgDX9Texture( cgUInt32 referenceId, const cgInputStream & stream, cgRenderDriver * driver, cgInt32 mipLevels /* = -1 */ ) : _BaseClass( referenceId, stream, driver, mipLevels )
{
    D3DXIMAGE_INFO D3DXInfo;

    // Initialize variables to sensible defaults
    mTexture = CG_NULL;
    
    // Attempt to retrieve the file information
    if ( mInputStream.getType() == cgStreamType::File )
    {
        giEXInit( _T(""), _T("") );
        mMediaDecoder = new giMediaDecoder();

        // Only 'cgWinAppWindow' type is supported.
        cgWinAppWindow * window = dynamic_cast<cgWinAppWindow*>(driver->getFocusWindow());
        if ( window != NULL )
        {
            // Initialize media decoding system (for videos)
            giMediaDecoderConfig mediaConfig;
            mediaConfig.audioDecodeMethod = GIADM_None;
            mMediaDecoder->setDecoderConfig( mediaConfig );
            mMediaDecoder->setFocusWindow( window->getWindowHandle() );

            // Attempt to open as a video first.
            if ( GIEXFAILED( mMediaDecoder->openMediaContainer( mInputStream.getSourceFile().c_str() ) ) )
            {
                delete mMediaDecoder;
                mMediaDecoder = CG_NULL;

            } // End if not video
            else
            {
                // Retrieve details of the specified media.
                giMediaInfo mediaInfo;
                mMediaDecoder->getMediaInfo( mediaInfo );

                // Record required information.
                mInfo.width      = mediaInfo.frameDimensions.width;
                mInfo.height     = mediaInfo.frameDimensions.height;
                mInfo.mipLevels  = 1;

            } // End if video
        
        } // End if windows
        else
        {
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("The media decoder is currently only supported on the Windows(tm) platform.\n"));

        } // End if invalid cast
     
        // If not a video, try standard image file.
        if ( mMediaDecoder == CG_NULL )
        {
        
            // Attempt to get standard image information using D3DX.
            if ( FAILED( D3DXGetImageInfoFromFile( mInputStream.getSourceFile().c_str(), &D3DXInfo ) ) )
                cgAppLog::write( cgAppLog::Warning, _T("Unable to retrieve texture information for '%s'. File was not found or is of an unrecognized texture format.\n"), mInputStream.getName().c_str() );

        } // End if not video
        
    } // End if file based stream
    else
    {
        size_t   dataLength = 0;
        cgByte * buffer     = mInputStream.getBuffer( dataLength );

        // ToDo: Support video for non-file streams

        // Get file information using D3DX.
        if ( FAILED( D3DXGetImageInfoFromFileInMemory( buffer, (UINT)dataLength, &D3DXInfo ) ) )
            cgAppLog::write( cgAppLog::Warning, _T("Unable to retrieve texture information for '%s'. File was not found or is of an unrecognized texture format.\n"), mInputStream.getName().c_str() );

        // We're finished with the stream buffer
        mInputStream.releaseBuffer();

    } // End if memory / memory mapped stream

    // Setup additional texture details
    if ( !mMediaDecoder )
    {
        // Copy into our internal image info type.
        mInfo.width     = D3DXInfo.Width;
        mInfo.height    = D3DXInfo.Height;
        mInfo.depth     = D3DXInfo.Depth;
        mInfo.mipLevels = D3DXInfo.MipLevels;
        mInfo.format    = cgDX9BufferFormatEnum::formatFromNative(D3DXInfo.Format);

        // DDS types simply use the file specified format directly, otherwise 
        // we auto-detect the best format (based on the original format of the file).
        if ( D3DXInfo.ImageFileFormat != D3DXIFF_DDS )
            mInfo.autoDetectFormat = true;
        
        // Resource type specifics
        switch ( D3DXInfo.ResourceType )
        {
            case D3DRTYPE_TEXTURE:
                
                // Standard texture
                if ( mInfo.height == 1 )
                    mInfo.type = cgBufferType::Texture1D;
                else
                    mInfo.type = cgBufferType::Texture2D;
                break;
            
            case D3DRTYPE_VOLUMETEXTURE:
            
                // Volume texture
                mInfo.type = cgBufferType::Texture3D;
                break;

            case D3DRTYPE_CUBETEXTURE:
                
                // Cube map texture
                mInfo.type = cgBufferType::TextureCube;
                break;

        } // End Switch ResourceType

        // If custom mip count supplied (nMipLevels >= 0), overwrite mip 
        // levels from file with those requested
        if ( mipLevels >= 0 )
            mInfo.mipLevels = mipLevels;

    } // End if not video
    else
    {
        mInfo.type = cgBufferType::Video;
        mInfo.pool = cgMemoryPool::Default;
        mInfo.autoGenerateMipmaps = true;
        
        // Register us as a listener for new media data.
        mMediaDecoder->registerListener( this );

    } // End if video

}

//-----------------------------------------------------------------------------
//  Name : cgDX9Texture () (Overload Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
cgDX9Texture<_BaseClass>::cgDX9Texture( cgUInt32 referenceId, const cgImageInfo & description ) : _BaseClass( referenceId, description )
{
    // Initialize variables to sensible defaults
    mTexture = CG_NULL;
    
    // ToDo: Support video type?
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9Texture () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
cgDX9Texture<_BaseClass>::~cgDX9Texture( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
template <class _BaseClass>
void cgDX9Texture<_BaseClass>::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( disposeBase )
        _BaseClass::dispose( true );
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
template <class _BaseClass>
bool cgDX9Texture<_BaseClass>::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9TextureResource )
        return true;

    // Supported by base?
    return _BaseClass::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : lock () (Virtual)
/// <summary>
/// Lock the entire top-level surface and return a pointer to the 
/// underlying memory.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void * cgDX9Texture<_BaseClass>::lock( cgUInt32 & pitch, cgUInt32 flags )
{
    D3DLOCKED_RECT lockRect;

    // Cannot lock the resource if it is not loaded, or is already locked elsewhere
    if ( !mTexture || mLocked )
        return CG_NULL;

    // Strip 'WriteOnly' flag (not a D3D9 matching flag).
    flags &= ~cgLockFlags::WriteOnly;

    // We need to understand the type of the texture in order
    // to lock it correctly.
    switch ( mInfo.type )
    {
        case cgBufferType::Texture1D:
        case cgBufferType::Texture2D:
        case cgBufferType::Video:
        {
            LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)mTexture;
            if ( FAILED( texture->LockRect( 0, &lockRect, CG_NULL, flags )) )
                return CG_NULL;        
            break;
        
        } // End Case Texture1D | Texture2D | Video
        case cgBufferType::TextureCube:
        {
            LPDIRECT3DCUBETEXTURE9 texture = (LPDIRECT3DCUBETEXTURE9)mTexture;
            if ( FAILED( texture->LockRect( (D3DCUBEMAP_FACES)mCurrentCubeFace, 0, &lockRect, NULL, flags )) )
                return CG_NULL;
            break;
        
        } // End Case TextureCube
        default:
            // Not currently supported on anything but standard textures, cube maps or video.
            return CG_NULL;

    } // End Switch type
    
    // Return lock data.
    mLockedBuffer     = lockRect.pBits;
    mLockedCubeFace   = mCurrentCubeFace;
    mLocked           = true;
    pitch             = (cgUInt32)lockRect.Pitch;

    // Return the underlying buffer pointer
    return mLockedBuffer;
}

//-----------------------------------------------------------------------------
//  Name : lock () (Virtual)
/// <summary>
/// Lock the specified portion of the top-level surface and return a pointer
/// to the underlying memory.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void * cgDX9Texture<_BaseClass>::lock( const cgRect & bounds, cgUInt32 & pitch, cgUInt32 flags )
{
    D3DLOCKED_RECT lockRect;

    // Cannot lock the resource if it is not loaded, or is already locked elsewhere
    if ( !mTexture || mLocked )
        return CG_NULL;

    // Strip 'WriteOnly' flag (not a D3D9 matching flag).
    flags &= ~cgLockFlags::WriteOnly;

    // We need to understand the type of the texture in order
    // to lock it correctly.
    switch ( mInfo.type )
    {
        case cgBufferType::Texture1D:
        case cgBufferType::Texture2D:
        case cgBufferType::Video:
        {
            LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)mTexture;
            if ( FAILED( texture->LockRect( 0, &lockRect, (const RECT*)&bounds, flags )) )
                return CG_NULL;        
            break;
        
        } // End Case Texture1D | Texture2D | Video
        case cgBufferType::TextureCube:
        {
            LPDIRECT3DCUBETEXTURE9 texture = (LPDIRECT3DCUBETEXTURE9)mTexture;
            if ( FAILED( texture->LockRect( (D3DCUBEMAP_FACES)mCurrentCubeFace, 0, &lockRect, (const RECT*)&bounds, flags )) )
                return CG_NULL;
            break;
        
        } // End Case TextureCube
        default:
            // Not currently supported on anything but standard textures, cube maps or video.
            return CG_NULL;

    } // End Switch type
    
    // Return lock data.
    mLockedBuffer     = lockRect.pBits;
    mLockedCubeFace   = mCurrentCubeFace;
    mLocked           = true;
    pitch             = (cgUInt32)lockRect.Pitch;

    // Return the underlying buffer pointer
    return mLockedBuffer;
}

//-----------------------------------------------------------------------------
//  Name : unlock () (Virtual)
/// <summary>
/// Unlock the buffer if previously locked.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void cgDX9Texture<_BaseClass>::unlock( bool updateMips /* = false */ )
{
    // Cannot unlock if it was not already locked
    if ( !mTexture || !mLocked )
        return;

    // We need to understand the type of the texture in order
    // to unlock it correctly.
    switch ( mInfo.type )
    {
        case cgBufferType::Texture1D:
        case cgBufferType::Texture2D:
        case cgBufferType::Video:
        {
            LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)mTexture;
            texture->UnlockRect( 0 );
            break;
        
        } // End Case Texture1D | Texture2D | Video
        case cgBufferType::TextureCube:
        {
            LPDIRECT3DCUBETEXTURE9 texture = (LPDIRECT3DCUBETEXTURE9)mTexture;
            texture->UnlockRect( (D3DCUBEMAP_FACES)mLockedCubeFace, 0 );
            break;
        
        } // End Case TextureCube
        default:
            // Not currently supported on anything but standard textures, cube maps or video.
            return;

    } // End Switch type

    // Re-generate mip maps if required
    if ( updateMips == true )
        D3DXFilterTexture( mTexture, CG_NULL, 0, D3DX_DEFAULT );

    // Item is no longer locked
    mLocked       = false;
    mLockedBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getD3DTexture ()
/// <summary>
/// Retrieve the internal D3D9 specific texture resource object.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
IDirect3DBaseTexture9 * cgDX9Texture<_BaseClass>::getD3DTexture( ) const
{
    // Add reference, we're returning a pointer
    if ( mTexture )
        mTexture->AddRef();

    // Return the resource item
    return mTexture;
}

//-----------------------------------------------------------------------------
//  Name : createTexture () (Protected, Virtual)
/// <summary>
/// Create the internal texture resource.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX9Texture<_BaseClass>::createTexture( )
{
    LPDIRECT3DBASETEXTURE9 texture = CG_NULL;
    HRESULT                result  = D3DERR_INVALIDCALL;
    
    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for texture creation (required DX9 class driver)
    IDirect3DDevice9 * device;
    cgDX9RenderDriver * driver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(device = driver->getD3DDevice()) )
        return false;
    
    // Retrieve buffer formats helper
    const cgBufferFormatEnum & formats = mManager->getBufferFormats();
    bool preferCompressed = mManager->getConfig().compressTextures;

    // Test for auto generation of mip-level support if requested
    if ( mInfo.autoGenerateMipmaps && mInfo.type != cgBufferType::Video )
    {
        if ( !(formats.getFormatCaps( mInfo.type, mInfo.format ) & cgBufferFormatCaps::CanAutoGenMipMaps ) )
        {
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Automatic mip-map generation was requested but not supported for texture resource '%s'.\n"), getResourceName().c_str() );
            mInfo.autoGenerateMipmaps = false;
        
        } // End if unsupported
        else
		{
			// Ensure mip levels is set to 1 before proceeding
			mInfo.mipLevels = 1;

		} // End if supported
	
    } // End if multiple mip levels

    // Loading from disk or creating manually?
    if ( mSource == Source_Stream )
    {
        // When auto-detecting format, we pay attention to source specific details
        // such as the image data (or requested format) containing an alpha channel, and 
        // select a good format based on our application configuration.
        if ( mInfo.autoDetectFormat )
            mInfo.format = formats.getBestFourChannelFormat( mInfo.type, false, formats.formatHasAlpha( mInfo.format ), 
                                                             formats.formatIsCompressed( mInfo.format ) | preferCompressed );
        D3DFORMAT finalFormat = (D3DFORMAT)cgDX9BufferFormatEnum::formatToNative(mInfo.format);

        // Where are we loading the texture from?
        if ( mInputStream.getType() == cgStreamType::Memory || mInputStream.getType() == cgStreamType::MappedFile )
        {
            // Get access to data
            size_t   dataLength;
            cgByte * textureData = mInputStream.getBuffer( dataLength );

            // ToDo: support video loading.

            // Loading from memory / mapped file
            switch ( mInfo.type )
            {
                case cgBufferType::Texture1D:
                case cgBufferType::Texture2D:
                    result = D3DXCreateTextureFromFileInMemoryEx( device, textureData, (UINT)dataLength, D3DX_DEFAULT, D3DX_DEFAULT, mInfo.mipLevels, 0,
                                                                  finalFormat, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, CG_NULL, CG_NULL, 
                                                                 (LPDIRECT3DTEXTURE9*)&texture );
                    break;

                case cgBufferType::Texture3D:
                    result = D3DXCreateVolumeTextureFromFileInMemoryEx( device, textureData, (UINT)dataLength, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, mInfo.mipLevels, 0, 
                                                                        finalFormat, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, CG_NULL, CG_NULL, 
                                                                        (LPDIRECT3DVOLUMETEXTURE9*)&texture );
                    break;

                case cgBufferType::TextureCube:
                    result = D3DXCreateCubeTextureFromFileInMemoryEx( device, textureData, (UINT)dataLength, D3DX_DEFAULT, mInfo.mipLevels, 0, 
                                                                      finalFormat, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, CG_NULL, CG_NULL, 
                                                                      (LPDIRECT3DCUBETEXTURE9*)&texture );
                    break;

            } // End texture type

            // We're done with the stream buffer
            mInputStream.releaseBuffer();

            // If we were loading from memory, reset the stream we no longer need
            // to have it maintain references etc.
            if ( mInputStream.getType() == cgStreamType::Memory )
                mInputStream.reset();

        } // End if loading from memory
        else if ( mInputStream.getType() == cgStreamType::File )
        {
            cgUInt32 usage = 0;

            // Loading from disk
            switch ( mInfo.type )
            {
                case cgBufferType::Texture1D:
                case cgBufferType::Texture2D:
                    result = D3DXCreateTextureFromFileEx( device, mInputStream.getSourceFile().c_str(), D3DX_DEFAULT, D3DX_DEFAULT, mInfo.mipLevels, 0,
                                                          finalFormat, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, CG_NULL, CG_NULL, 
                                                          (LPDIRECT3DTEXTURE9*)&texture );
                    break;

                case cgBufferType::Texture3D:
                    result = D3DXCreateVolumeTextureFromFileEx( device, mInputStream.getSourceFile().c_str(), D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, mInfo.mipLevels, 0, 
                                                                finalFormat, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, CG_NULL, CG_NULL, 
                                                                (LPDIRECT3DVOLUMETEXTURE9*)&texture );
                    break;

                case cgBufferType::TextureCube:
                    result = D3DXCreateCubeTextureFromFileEx( device, mInputStream.getSourceFile().c_str(), D3DX_DEFAULT, mInfo.mipLevels, 0, 
                                                              finalFormat, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, CG_NULL, CG_NULL, 
                                                              (LPDIRECT3DCUBETEXTURE9*)&texture );
                    break;

                case cgBufferType::Video:

                    // Turn off automatic mip generation if it is not supported
                    if ( !(formats.getFormatCaps( cgBufferType::Texture2D, cgBufferFormat::B8G8R8X8 ) & cgBufferFormatCaps::CanAutoGenMipMaps ) )
                        mInfo.autoGenerateMipmaps = false;
                    
                    // Select correct usage.
                    usage = D3DUSAGE_DYNAMIC;
                    if ( mInfo.autoGenerateMipmaps == true )
                        usage |= D3DUSAGE_AUTOGENMIPMAP;

                    // Create an empty dynamic texture.
                    cgToDo( "Carbon General", "Check texture format is supported and potentially ask video codec to swizzle as necessary." );
                    result = device->CreateTexture( mInfo.width, mInfo.height, 1, usage, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, (LPDIRECT3DTEXTURE9*)&texture, CG_NULL );
                    break;

            } // End texture type
            
        } // End if loading from disk
        else
        {
            // If we're in sandbox mode (preview or full), we should treat a missing 
            // file as a success irrespective of the fact that we aren't able to load 
            // it. This ensures that the texture's information is not lost.
            if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
                result = D3D_OK;

        } // End if invalid stream

        // Resource is no longer lost.
        mResourceLost = false;

    } // End if loading texture
    else
    {
        cgUInt32 usage = 0;

        // Type of texture to create.
        switch ( mInfo.type )
        {
            case cgBufferType::DepthStencil:
            case cgBufferType::ShadowMap:
            case cgBufferType::RenderTarget:
            case cgBufferType::Texture1D:
            case cgBufferType::Texture2D:
                
                // Build usage flags.
                if ( mInfo.type == cgBufferType::RenderTarget )
                    usage = D3DUSAGE_RENDERTARGET;
                else if ( mInfo.type == cgBufferType::DepthStencil ||
                          mInfo.type == cgBufferType::ShadowMap )
                    usage = D3DUSAGE_DEPTHSTENCIL;
                if ( mInfo.autoGenerateMipmaps )
                    usage |= D3DUSAGE_AUTOGENMIPMAP;
                if ( mInfo.dynamic )
                    usage |= D3DUSAGE_DYNAMIC;

                // Create the texture
                result = device->CreateTexture( mInfo.width, mInfo.height, mInfo.mipLevels, usage, 
                                                (D3DFORMAT)cgDX9BufferFormatEnum::formatToNative(mInfo.format), 
                                                (D3DPOOL)mInfo.pool, (LPDIRECT3DTEXTURE9*)&texture, CG_NULL );
                break;

            case cgBufferType::RenderTargetCube:
            case cgBufferType::TextureCube:
                
                // Build usage flags.
                if ( mInfo.type == cgBufferType::RenderTargetCube )
                    usage = D3DUSAGE_RENDERTARGET;
                if ( mInfo.autoGenerateMipmaps )
                    usage |= D3DUSAGE_AUTOGENMIPMAP;
                if ( mInfo.dynamic )
                    usage |= D3DUSAGE_DYNAMIC;

                // Create the texture
                result = device->CreateCubeTexture( mInfo.width, mInfo.mipLevels, usage, 
                                                   (D3DFORMAT)cgDX9BufferFormatEnum::formatToNative(mInfo.format),
                                                   (D3DPOOL)mInfo.pool, (LPDIRECT3DCUBETEXTURE9*)&texture, CG_NULL );
                break;

            case cgBufferType::Texture3D:
                
                // Build usage flags.
                if ( mInfo.autoGenerateMipmaps )
                    usage |= D3DUSAGE_AUTOGENMIPMAP;
                if ( mInfo.dynamic )
                    usage |= D3DUSAGE_DYNAMIC;

                // Create the texture
                result = device->CreateVolumeTexture( mInfo.width, mInfo.height, mInfo.depth, mInfo.mipLevels, usage,
                                                     (D3DFORMAT)cgDX9BufferFormatEnum::formatToNative(mInfo.format),
                                                     (D3DPOOL)mInfo.pool, (LPDIRECT3DVOLUMETEXTURE9*)&texture, CG_NULL );
                break;

            case cgBufferType::DeviceBackBuffer:
                // Device back buffer is "smart" managed (see cgDX9RenderTarget::GetD3DTargetSurface())
                result = D3D_OK;
                break;

        } // End Switch type

    } // End if creating texture

    // Release D3D Device
    device->Release();

    // Success?
    if (FAILED(result))
    {
        if ( texture != CG_NULL )
            texture->Release();
        return false;
    
    } // End if failed

    // Resource has now been loaded
    mTexture = texture;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : releaseTexture () (Protected, Virtual)
/// <summary>
/// Release the internal texture data.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void cgDX9Texture<_BaseClass>::releaseTexture( )
{
    // We should release our underlying resource
    if ( mTexture )
        mTexture->Release();
    mTexture = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void cgDX9Texture<_BaseClass>::deviceLost()
{
    // Nothing to do?
    if ( mInfo.pool != cgMemoryPool::Default )
        return;

    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseTexture();
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void cgDX9Texture<_BaseClass>::deviceRestored()
{
    // Bail if this is not in the default pool
    if ( mInfo.pool != cgMemoryPool::Default )
        return;

    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createTexture();

    // Resource data (that contained on the surface) is now lost
    setResourceLost( true );
}

//-----------------------------------------------------------------------------
//  Name : getImageData ()
/// <summary>
/// Populates the specified image with the optionally resampled data contained 
/// in the top level surface of the texture. If the image' internal buffer has 
/// not yet been allocated (to a specific size / format) then it will be 
/// initialized with matching properties to that of the texture.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX9Texture<_BaseClass>::getImageData( cgImage & imageOut )
{
    // ToDo: 9999 - Add error handling.
    LPDIRECT3DSURFACE9  textureSurface = NULL, sourceSurface = NULL;
    D3DLOCKED_RECT      sourceData;
    D3DSURFACE_DESC     surfaceDesc;

    // Validate Requirements
    if ( !mTexture )
        return false;
    
    // Make sure the texture is up to date.
    update();

    // Is this a valid 2D or cube texture?
    if ( mInfo.type == cgBufferType::Texture1D || mInfo.type == cgBufferType::Texture2D || 
         mInfo.type == cgBufferType::Video || mInfo.type == cgBufferType::RenderTarget )
    {
        LPDIRECT3DTEXTURE9 texture = NULL;
        if ( FAILED( mTexture->QueryInterface( IID_IDirect3DTexture9, (void**)&texture ) ) )
            return false;
    
        // Read from the top level surface
        texture->GetSurfaceLevel( 0, &textureSurface );
        texture->Release();

    } // End if standard / render target texture
    else if ( mInfo.type == cgBufferType::TextureCube || mInfo.type == cgBufferType::RenderTargetCube )
    {
        LPDIRECT3DCUBETEXTURE9 texture = NULL;
        if ( FAILED( mTexture->QueryInterface( IID_IDirect3DCubeTexture9, (void**)&texture ) ) )
            return false;
    
        // Read from the top level surface of the relevant face
        texture->GetCubeMapSurface( (D3DCUBEMAP_FACES)mCurrentCubeFace, 0, &textureSurface );
        texture->Release();

    } // End if cube map
    else
    {
        // Invalid operation
        return false;
    
    } // End if other

    // If the output image buffer has not yet been allocated, we must do so first.
    textureSurface->GetDesc( &surfaceDesc );
    if ( !imageOut.isValid() )
    {
        if ( !imageOut.createImage( surfaceDesc.Width, surfaceDesc.Height, mInfo.format, false ) )
            return false;
    
    } // End if allocate
    cgUInt32 width = imageOut.getWidth();
    cgUInt32 height = imageOut.getHeight();

    // If this is a render target texture, we must first copy it into a system memory surface.
    if ( mInfo.type == cgBufferType::RenderTarget || mInfo.type == cgBufferType::RenderTargetCube )
    {
        IDirect3DDevice9 * device;
        
        // We need to decode the texture into a new surface.
        textureSurface->GetDevice( &device );
        device->CreateOffscreenPlainSurface( surfaceDesc.Width, surfaceDesc.Height, surfaceDesc.Format, D3DPOOL_SYSTEMMEM, &sourceSurface, CG_NULL );
        device->GetRenderTargetData( textureSurface, sourceSurface );
        device->Release();

        // Original surface can now be released.
        textureSurface->Release();
        textureSurface = CG_NULL;

        // The system memory surface now becomes our source surface.
        textureSurface = sourceSurface;
        sourceSurface = CG_NULL;

    } // End if render target

    // If the current texture format matches that required (in the image buffer) and the
    // requested size is identical, then we can directly copy from the texture's surface. 
    // If not, we must convert and/or downsample it first.
    if ( mInfo.format == imageOut.getFormat() && surfaceDesc.Width == width && surfaceDesc.Height == height )
    {
        // Just use the texture's surface.
        sourceSurface = textureSurface;
        textureSurface = NULL;

    } // End if matched surface
    else
    {
        IDirect3DDevice9 * device;
        
        // We need to decode the texture into a new surface.
        textureSurface->GetDevice( &device );
        cgToDo( "DX11", "Consider supported formats!" );
        device->CreateOffscreenPlainSurface( width, height, 
                                             (D3DFORMAT)cgDX9BufferFormatEnum::formatToNative(imageOut.getFormat()),
                                             D3DPOOL_SYSTEMMEM, &sourceSurface, NULL );
        D3DXLoadSurfaceFromSurface( sourceSurface, NULL, NULL, textureSurface, NULL, NULL, D3DX_DEFAULT, 0 );
        device->Release();

        // Original surface can now be released.
        textureSurface->Release();
        textureSurface = NULL;

    } // End if mismatched surface

    // Lock the surface so that we can copy pixel data out.
    sourceSurface->LockRect( &sourceData, NULL, D3DLOCK_READONLY );
    
    // Copy image data.
    cgUInt32 pixelStride       = imageOut.getBytesPerPixel();
    cgByte * sourceBuffer      = (cgByte*)sourceData.pBits;
    cgByte * destinationBuffer = (cgByte*)imageOut.getBuffer();
    cgUInt32 destinationPitch  = imageOut.getPitch();
    for ( cgUInt32 y = 0; y < height; ++y )
    {
        // Duplicate current scanline
        memcpy( destinationBuffer, sourceBuffer, width * pixelStride );
        
        // Move down to next row
        sourceBuffer  += sourceData.Pitch;
        destinationBuffer += destinationPitch;

    } // Next Row

    // Clean up
    sourceSurface->UnlockRect( );
    sourceSurface->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : clone () (Virtual)
/// <summary>
/// Copy the specified region of this texture into the specified region of the
/// destination texture.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX9Texture<_BaseClass>::clone( cgTexture * destinationTexture, const cgRect & sourceRectangle, const cgRect & destinationRectangle )
{
    cgToDo( "Carbon General", "DepthStencil cannot always be cloned and this will fail in such cases." );

    static int cloneCount = 0;

    // Types must be equivalent.
    if ( mInfo.type != destinationTexture->getInfo().type )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to clone texture '%s' into new texture '%s'. Types are not equivalent.\n"), this->getResourceName().c_str(), destinationTexture->getResourceName().c_str() );
        return false;
    
    } // End if mismatched types
    
    // Perform the clone.
    switch ( mInfo.type )
    {
        case cgBufferType::Texture1D:
        case cgBufferType::Texture2D:
        case cgBufferType::RenderTarget:
        case cgBufferType::DepthStencil:
        case cgBufferType::ShadowMap:
        {
            LPDIRECT3DTEXTURE9 sourceD3DTexture = CG_NULL, destinationD3DTexture = CG_NULL;
            LPDIRECT3DSURFACE9 sourceSurface = CG_NULL, destinationSurface = CG_NULL;
            
            // Retrieve both textures.
            sourceD3DTexture = (LPDIRECT3DTEXTURE9)getD3DTexture();
            destinationD3DTexture = (LPDIRECT3DTEXTURE9)((cgDX9Texture<_BaseClass>*)destinationTexture)->getD3DTexture();

            // Clone all mip levels
            cgUInt32 minimumLevels = min( sourceD3DTexture->GetLevelCount(), destinationD3DTexture->GetLevelCount() );
            for ( cgUInt32 i = 0; i < minimumLevels; ++i )
            {
                sourceD3DTexture->GetSurfaceLevel( i, &sourceSurface );
                destinationD3DTexture->GetSurfaceLevel( i, &destinationSurface );

                // Perform the clone.
                D3DXLoadSurfaceFromSurface( destinationSurface, CG_NULL, (RECT*)&destinationRectangle, sourceSurface, CG_NULL, (RECT*)&sourceRectangle, D3DX_DEFAULT, 0 );

                // Clean up
                sourceSurface->Release();
                destinationSurface->Release();

            } // Next mip level

            // Clean up
            sourceD3DTexture->Release();
            destinationD3DTexture->Release();
            break;

        } // End Case Texture1D | Texture2D | RenderTarget | DepthStencil
        case cgBufferType::TextureCube:
        case cgBufferType::RenderTargetCube:
        {
            LPDIRECT3DCUBETEXTURE9 sourceD3DTexture = CG_NULL, destinationD3DTexture = CG_NULL;
            LPDIRECT3DSURFACE9 sourceSurface = CG_NULL, destinationSurface = CG_NULL;
            
            // Retrieve both textures.
            sourceD3DTexture = (LPDIRECT3DCUBETEXTURE9)getD3DTexture();
            destinationD3DTexture = (LPDIRECT3DCUBETEXTURE9)((cgDX9Texture<_BaseClass>*)destinationTexture)->getD3DTexture();

            // Clone all faces at all mip levels
            cgUInt32 minimumLevels = min( sourceD3DTexture->GetLevelCount(), destinationD3DTexture->GetLevelCount() );
            for ( cgUInt32 i = 0; i < 6; ++i )
            {
                for ( cgUInt32 j = 0; j < minimumLevels; ++j )
                {

                    sourceD3DTexture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, j, &sourceSurface );
                    destinationD3DTexture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, j, &destinationSurface );

                    // Perform the clone.
                    D3DXLoadSurfaceFromSurface( destinationSurface, CG_NULL, (RECT*)&destinationRectangle, sourceSurface, CG_NULL, (RECT*)&sourceRectangle, D3DX_DEFAULT, 0 );

                    // Clean up
                    sourceSurface->Release();
                    destinationSurface->Release();

                } // Next mip level

            } // Next cube face

            // Clean up
            sourceD3DTexture->Release();
            destinationD3DTexture->Release();
            break;

        } // End Case TextureCube | RenderTargetCube
        case cgBufferType::Texture3D:
        {
            LPDIRECT3DVOLUMETEXTURE9 sourceD3DTexture = CG_NULL, destinationD3DTexture = CG_NULL;
            LPDIRECT3DVOLUME9 sourceVolume = CG_NULL, destinationVolume = CG_NULL;
            
            // Retrieve both textures.
            sourceD3DTexture = (LPDIRECT3DVOLUMETEXTURE9)getD3DTexture();
            destinationD3DTexture = (LPDIRECT3DVOLUMETEXTURE9)((cgDX9Texture<_BaseClass>*)destinationTexture)->getD3DTexture();

            // Clone all mip levels
            cgUInt32 minimumLevels = min( sourceD3DTexture->GetLevelCount(), destinationD3DTexture->GetLevelCount() );
            for ( cgUInt32 i = 0; i < minimumLevels; ++i )
            {
                sourceD3DTexture->GetVolumeLevel( i, &sourceVolume );
                destinationD3DTexture->GetVolumeLevel( i, &destinationVolume );

                // Perform the clone.
                cgToDo( "Carbon General", "Support selectable regions for volume textures." );
                D3DXLoadVolumeFromVolume( destinationVolume, CG_NULL, CG_NULL, sourceVolume, CG_NULL, CG_NULL, D3DX_DEFAULT, 0 );

                // Clean up
                sourceVolume->Release();
                destinationVolume->Release();

            } // Next mip level

            // Clean up
            sourceD3DTexture->Release();
            destinationD3DTexture->Release();
            break;
        
        } // End Case Volume
        default:
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to clone texture '%s' into new texture '%s'. Unknown type detected.\n"), this->getResourceName().c_str(), destinationTexture->getResourceName().c_str() );
            return false;
    
    } // End Switch Type

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Allows the texture to update if it is decoding media perhaps.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void cgDX9Texture<_BaseClass>::update()
{
    // Only video textures need further updating.
    if ( mInfo.type != cgBufferType::Video )
        return;

    // Call base class implementation first
    _BaseClass::update();

    // If automatic generation of mip maps is required, we should hint to the driver 
    // that we need new mip levels generating at this point (depending on how often
    // we would like them updating and whether or not a new video frame was supplied).
    if ( mInfo.autoGenerateMipmaps && mMipsDirty )
    {
        cgDouble currentTime = mMediaDecoder->getPlayheadTime();
        if ( mMediaConfig.mipmapUpdateRate <= CGE_EPSILON || abs(currentTime - mLastMipUpdateTime) >= (1.0 / (cgDouble)mMediaConfig.mipmapUpdateRate) )
        {
            updateMipLevels();

            // Mips have been generated and are no longer dirty.
            mLastMipUpdateTime = currentTime;
        
        } // End if time to update
    
    } // End if update mips
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX9Texture<_BaseClass>::loadResource( )
{
    // Is resource already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the texture
    if ( !createTexture( ) )
        return false;

    // Call base class implementation.
    return _BaseClass::loadResource();
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX9Texture<_BaseClass>::unloadResource( )
{
    // Release the texture
    releaseTexture();

    // Call base class implementation.
    return _BaseClass::unloadResource();
}

//-----------------------------------------------------------------------------
//  Name : updateMipLevels ()
/// <summary>
/// Force an update of mip map level data
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX9Texture<_BaseClass>::updateMipLevels( )
{
    // ToDo: 6767 -- Query interface?

	// Mark the whole surface area as dirty
    if ( mInfo.type == cgBufferType::TextureCube || mInfo.type == cgBufferType::RenderTargetCube )
        ((LPDIRECT3DCUBETEXTURE9)mTexture)->AddDirtyRect( (D3DCUBEMAP_FACES)mCurrentCubeFace, CG_NULL ); 
    else
        ((LPDIRECT3DTEXTURE9)mTexture)->AddDirtyRect( CG_NULL ); 

	// Ask the texture to update its mip levels
	mTexture->GenerateMipSubLevels();

	// Mips have been generated and are no longer dirty.
	mMipsDirty = false;

	// Return success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : isValid ()
/// <summary>
/// Underlying texture is currently resident. This method is separate from
/// isLoaded(), which may actually return a positive result in sandbox mode
/// even if the texture failed to load.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX9Texture<_BaseClass>::isValid( ) const
{
    return (mTexture != CG_NULL);
}

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9TEXTURE_H_