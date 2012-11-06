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
// Name : cgDX11Texture.h                                                    //
//                                                                           //
// Desc : Contains classes responsible for loading and managing texture      //
//        resource data (DX11 implementation).                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11TEXTURE_H_ )
#define _CGE_CGDX11TEXTURE_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11Texture Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceTypes.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX11RenderDriver.h>
#include <Resources/Platform/cgDX11BufferFormatEnum.h>
#include <System/Platform/cgWinAppWindow.h>
#include <System/cgImage.h>
#include <Math/cgMathTypes.h>
#include <D3D11.h>
#include <D3DX11.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct ID3D11Resource;
struct ID3D11ShaderResourceView;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {AA15838F-8D5E-47B4-8029-4F4443614678}
const cgUID RTID_DX11TextureResource = {0xAA15838F, 0x8D5E, 0x47B4, {0x80, 0x29, 0x4F, 0x44, 0x43, 0x61, 0x46, 0x78}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11Texture (Class)
/// <summary>
/// Wrapper for managing texture resources (DX11 implementation).
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
class CGE_API cgDX11Texture : public _BaseClass
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11Texture( cgUInt32 referenceId, const cgInputStream & stream, cgRenderDriver * driver, cgInt32 mipLevels = -1 );
             cgDX11Texture( cgUInt32 referenceId, const cgImageInfo & description );
    virtual ~cgDX11Texture( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11Resource            * getD3DTexture       ( ) const;
    ID3D11ShaderResourceView  * getD3DShaderView    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgTexture)
    //-------------------------------------------------------------------------
    virtual void            update              ( );
    virtual void          * lock                ( cgUInt32 & pitch, cgUInt32 flags );
    virtual void          * lock                ( const cgRect & bounds, cgUInt32 & pitch, cgUInt32 flags );
    virtual void            unlock              ( bool updateMips = false );
    virtual bool            getImageData        ( cgImage & imageOut );
    virtual bool            updateMipLevels     ( );
    
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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX11TextureResource; }
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
    ID3D11Resource            * mTexture;       // Underlying resource (Texture1D, Texture2D, Texture3D)
    ID3D11ShaderResourceView  * mShaderView;    // Default shader resource view
};

///////////////////////////////////////////////////////////////////////////////
// cgDX11Texture Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11Texture () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
cgDX11Texture<_BaseClass>::cgDX11Texture( cgUInt32 referenceId, const cgInputStream & stream, cgRenderDriver * driver, cgInt32 mipLevels /* = -1 */ ) : _BaseClass( referenceId, stream, driver, mipLevels )
{
    D3DX11_IMAGE_INFO D3DXInfo;

    // Initialize variables to sensible defaults
    mTexture    = CG_NULL;
    mShaderView = CG_NULL;
    
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
            if ( FAILED( D3DX11GetImageInfoFromFile( mInputStream.getSourceFile().c_str(), CG_NULL, &D3DXInfo, CG_NULL ) ) )
                cgAppLog::write( cgAppLog::Warning, _T("Unable to retrieve texture information for '%s'. File was not found or is of an unrecognized texture format.\n"), mInputStream.getName().c_str() );

        } // End if not video
        
    } // End if file based stream
    else
    {
        size_t   dataLength = 0;
        cgByte * buffer     = mInputStream.getBuffer( dataLength );

        // ToDo: Support video for non-file streams

        // Get file information using D3DX.
        if ( FAILED( D3DX11GetImageInfoFromMemory( buffer, (UINT)dataLength, CG_NULL, &D3DXInfo, CG_NULL ) ) )
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
        mInfo.format    = cgDX11BufferFormatEnum::formatFromNative(D3DXInfo.Format);

        // DDS types simply use the file specified format directly, otherwise 
        // we auto-detect the best format (based on the original format of the file).
        if ( D3DXInfo.ImageFileFormat != D3DX11_IFF_DDS )
            mInfo.autoDetectFormat = true;
        
        // Resource type specifics
        switch ( D3DXInfo.ResourceDimension )
        {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
                mInfo.type = cgBufferType::Texture1D;
                break;
            
            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
                mInfo.type = cgBufferType::Texture2D;
                break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
                mInfo.type = cgBufferType::Texture3D;
                break;

        } // End Switch ResourceDimension

        // Is cube map?
        if ( D3DXInfo.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE && D3DXInfo.ArraySize == 6 )
            mInfo.type = cgBufferType::TextureCube;

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
//  Name : cgDX11Texture () (Overload Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
cgDX11Texture<_BaseClass>::cgDX11Texture( cgUInt32 referenceId, const cgImageInfo & description ) : _BaseClass( referenceId, description )
{
    // Initialize variables to sensible defaults
    mTexture      = CG_NULL;
    mShaderView   = CG_NULL;
    
    // ToDo: Support video type?
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11Texture () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
cgDX11Texture<_BaseClass>::~cgDX11Texture( )
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
void cgDX11Texture<_BaseClass>::dispose( bool disposeBase )
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
bool cgDX11Texture<_BaseClass>::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11TextureResource )
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
void * cgDX11Texture<_BaseClass>::lock( cgUInt32 & pitch, cgUInt32 flags )
{
    // Cannot lock the resource if it is not loaded, or is already locked elsewhere
    if ( !mTexture || mLocked )
        return CG_NULL;

    // Cannot currently lock a volume resource.
    if ( mInfo.type == cgBufferType::Texture3D )
        return CG_NULL;

    // Generate the D3D11 compatible lock / map operation.
    D3D11_MAP mapType;
    if ( flags & cgLockFlags::ReadOnly )
        mapType = D3D11_MAP_READ;
    else if ( flags & cgLockFlags::WriteOnly )
    {
        mapType = D3D11_MAP_WRITE;
        if ( flags & cgLockFlags::Discard )
            mapType = D3D11_MAP_WRITE_DISCARD;
        else if ( flags & cgLockFlags::NoOverwrite )
            mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
    
    } // End if write only
    else
        mapType = D3D11_MAP_READ_WRITE;

    // And the lock / map flags
    cgUInt32 mapFlags = 0;
    if ( flags & cgLockFlags::DoNotWait )
        mapFlags |= D3D11_MAP_FLAG_DO_NOT_WAIT;

    // If this is a cube texture, select the correct sub-resource.
    cgUInt subResource = 0;
    if ( mInfo.type == cgBufferType::TextureCube )
        subResource = D3D11CalcSubresource( 0, (cgUInt)mCurrentCubeFace, mInfo.mipLevels );

    // Get the device so that we can request the lock
    ID3D11DeviceContext * deviceContext = CG_NULL;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(deviceContext = driver->getD3DDeviceContext()) )
        return CG_NULL;

    // Attempt the map.
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if ( FAILED( deviceContext->Map( mTexture, subResource, mapType, mapFlags, &mappedResource ) ) )
    {
        deviceContext->Release();
        return CG_NULL;
    
    } // End if failed

    // Clean up
    deviceContext->Release();
    
    // Return lock data.
    mLockedBuffer   = mappedResource.pData;
    mLockedCubeFace = mCurrentCubeFace;
    mLocked         = true;
    pitch           = (cgUInt32)mappedResource.RowPitch;

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
void * cgDX11Texture<_BaseClass>::lock( const cgRect & bounds, cgUInt32 & pitch, cgUInt32 flags )
{
    cgToDo( "DX11", "Remove this function from all texture classes." );

    // Perform the regular lock operation on the whole resource.
    if ( !lock( pitch, flags ) )
        return CG_NULL;

    // Adjust the offset of the locked data to match their requested region.
    cgUInt32 bytesPerPixel = cgBufferFormatEnum::formatBitsPerPixel( mInfo.format ) / 8;
    mLockedBuffer = ((cgByte*)mLockedBuffer) + bounds.top * pitch;
    mLockedBuffer = ((cgByte*)mLockedBuffer) + bounds.left * bytesPerPixel;
    
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
void cgDX11Texture<_BaseClass>::unlock( bool updateMips /* = false */ )
{
    // Cannot unlock if it was not already locked
    if ( !mTexture || !mLocked )
        return;

    // If this is a cube texture, select the correct sub-resource.
    cgUInt subResource = 0;
    if ( mInfo.type == cgBufferType::TextureCube )
        subResource = D3D11CalcSubresource( 0, (cgUInt)mLockedCubeFace, mInfo.mipLevels );

    // Get the device so that we can request the umap
    ID3D11DeviceContext * deviceContext = CG_NULL;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(deviceContext = driver->getD3DDeviceContext()) )
        return;

    // Unmap
    deviceContext->Unmap( mTexture, subResource );

    // Clean up
    deviceContext->Release();

    // Re-generate mip maps if required
    cgToDo( "DX11", "Update mip-levels." )
    /*if ( updateMips == true )
        D3DXFilterTexture( mTexture, CG_NULL, 0, D3DX_DEFAULT );*/

    // Item is no longer locked
    mLocked       = false;
    mLockedBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getD3DTexture ()
/// <summary>
/// Retrieve the internal D3D11 specific texture resource object.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
ID3D11Resource * cgDX11Texture<_BaseClass>::getD3DTexture( ) const
{
    // Back buffer mapped texture?
    if ( mInfo.type == cgBufferType::DeviceBackBuffer )
        return ((cgDX11RenderDriver*)mManager->getRenderDriver())->getD3DBackBuffer( );

    // Add reference, we're returning a pointer
    if ( mTexture )
        mTexture->AddRef();

    // Return the resource item
    return mTexture;
}

//-----------------------------------------------------------------------------
//  Name : getD3DShaderView ()
/// <summary>
/// Retrieve the D3D11 specific shader resource view for this texture.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
ID3D11ShaderResourceView * cgDX11Texture<_BaseClass>::getD3DShaderView( ) const
{
    // Add reference, we're returning a pointer
    if ( mShaderView )
        mShaderView->AddRef();

    // Return the view item
    return mShaderView;
}

//-----------------------------------------------------------------------------
//  Name : createTexture () (Protected, Virtual)
/// <summary>
/// Create the internal texture resource.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX11Texture<_BaseClass>::createTexture( )
{
    ID3D11Resource* texture = CG_NULL;
    HRESULT         result = D3DERR_INVALIDCALL;
    DXGI_FORMAT     resourceFormat, viewFormat;
    
    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for texture creation (required DX11 class driver)
    ID3D11Device * device;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
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
    bool supportsShaderView = true;
    if ( mSource == Source_Stream )
    {
        // When auto-detecting format, we pay attention to source specific details
        // such as the image data (or requested format) containing an alpha channel, and 
        // select a good format based on our application configuration.
        if ( mInfo.autoDetectFormat )
            mInfo.format = formats.getBestFourChannelFormat( mInfo.type, false, formats.formatHasAlpha( mInfo.format ), 
                                                             formats.formatIsCompressed( mInfo.format ) | preferCompressed );
        resourceFormat = (DXGI_FORMAT)cgDX11BufferFormatEnum::formatToNative(mInfo.format);
        viewFormat = resourceFormat;

        // Build the info structure for the texture as we would like it to exist.
        // i.e. Format may be different to that of the file, etc.
        D3DX11_IMAGE_LOAD_INFO fileInfo;
        fileInfo.Width          = mInfo.width;
        fileInfo.Height         = mInfo.height;
        fileInfo.Depth          = mInfo.depth;
        fileInfo.FirstMipLevel  = 0;
        fileInfo.MipLevels      = mInfo.mipLevels;
        fileInfo.Usage          = D3D11_USAGE_DEFAULT;
        fileInfo.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
        fileInfo.CpuAccessFlags = 0;
        fileInfo.MiscFlags      = ( mInfo.type == cgBufferType::TextureCube ) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
        fileInfo.Format         = resourceFormat;
        fileInfo.Filter         = D3DX11_FILTER_LINEAR;
        fileInfo.MipFilter      = D3DX11_FILTER_LINEAR;

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
                case cgBufferType::Texture3D:
                case cgBufferType::TextureCube:
                    result = D3DX11CreateTextureFromMemory( device, textureData, (UINT)dataLength, &fileInfo, CG_NULL, &texture, CG_NULL );
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
            // Loading from disk
            switch ( mInfo.type )
            {
                case cgBufferType::Texture1D:
                case cgBufferType::Texture2D:
                case cgBufferType::Texture3D:
                case cgBufferType::TextureCube:
                    result = D3DX11CreateTextureFromFile( device, mInputStream.getSourceFile().c_str(), &fileInfo, CG_NULL, &texture, CG_NULL );
                    break;

                case cgBufferType::Video:
                {
                    // Turn off automatic mip generation if it is not supported
                    if ( !(formats.getFormatCaps( cgBufferType::Texture2D, cgBufferFormat::B8G8R8X8 ) & cgBufferFormatCaps::CanAutoGenMipMaps ) )
                        mInfo.autoGenerateMipmaps = false;

                    // Build custom texture description for empty dynamic texture.
                    D3D11_TEXTURE2D_DESC desc;
                    desc.Width          = fileInfo.Width;
                    desc.Height         = fileInfo.Height;
                    desc.MipLevels      = fileInfo.MipLevels;
                    desc.ArraySize      = 1;
                    desc.Format         = fileInfo.Format;
                    desc.SampleDesc.Count = 1;
                    desc.SampleDesc.Quality = 0;
                    desc.Usage          = D3D11_USAGE_DYNAMIC;
                    desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    desc.MiscFlags      = 0;
                    
                    cgToDo( "DX11", "Support automatic mip generation!" );
                    //if ( mInfo.autoGenerateMipmaps == true )
                        //desc.Usage |= D3DUSAGE_AUTOGENMIPMAP;
                    
                    // Create an empty dynamic texture.
                    cgToDo( "Carbon General", "Check texture format is supported and potentially ask video codec to swizzle as necessary." );
                    ID3D11Texture2D * tempTexture = CG_NULL;
                    result = device->CreateTexture2D( &desc, CG_NULL, &tempTexture );
                    if ( SUCCEEDED( result ) )
                    {
                        tempTexture->QueryInterface( __uuidof( ID3D11Resource ), (void**)&texture );
                        tempTexture->Release();
                    
                    } // End if success
                    break;

                } // End case Video

            } // End texture type
            
        } // End if loading from disk
        else
        {
            // If we're in sandbox mode (preview or full), we should treat a missing file 
            // as a success irrespective of the fact that we aren't able to load it. This 
            // ensures that the texture's information is not lost.
            if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
                result = D3D_OK;

        } // End if invalid stream

        // Resource is no longer lost.
        mResourceLost = false;

    } // End if loading texture
    else
    {
        resourceFormat = (DXGI_FORMAT)cgDX11BufferFormatEnum::formatToNative(mInfo.format);
        viewFormat = resourceFormat;

        // We always map depth stencil format to the equivalent typeless 
        // color format so that it can be bound as a shader resource.
        // The DepthStencilView will set the appropriate depth format however
        // so that it can be used in calls to BeginTargetRender().
        if ( mInfo.type == cgBufferType::DepthStencil ||
             mInfo.type == cgBufferType::ShadowMap )
        {
            switch ( mInfo.format )
            {
                case cgBufferFormat::D32_Float_S8X24_UInt:
                    resourceFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
                    viewFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                    break;
                case cgBufferFormat::D32_Float:
                    resourceFormat = DXGI_FORMAT_R32_TYPELESS;
                    viewFormat = DXGI_FORMAT_R32_FLOAT;
                    break;
                case cgBufferFormat::D24_UNorm_S8_UInt:
                    resourceFormat = DXGI_FORMAT_R24G8_TYPELESS;
                    viewFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                    break;
                case cgBufferFormat::D24_UNorm_X8_Typeless:
                    resourceFormat = DXGI_FORMAT_R24G8_TYPELESS;
                    viewFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                    break;
                case cgBufferFormat::D16:
                    resourceFormat = DXGI_FORMAT_R16_TYPELESS;
                    viewFormat = DXGI_FORMAT_R16_UNORM;
                    break;

            } // End switch format

        } // End if DepthStencil | ShadowMap


        // Select correct usages and bindings depending on the buffer type.
        switch ( mInfo.type )
        {
            case cgBufferType::Texture1D:
            {
                // Build the info structure for the texture as we would like it to exist.
                D3D11_TEXTURE1D_DESC desc;
                desc.Width          = mInfo.width;
                desc.MipLevels      = mInfo.mipLevels;
                desc.ArraySize      = 1;
                desc.Format         = resourceFormat;
                desc.Usage          = (mInfo.dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
                desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = (mInfo.dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
                desc.MiscFlags      = 0;
                
                cgToDo( "DX11", "Support automatic mip generation!" );
                //if ( mInfo.autoGenerateMipmaps == true )
                    //desc.Usage |= D3DUSAGE_AUTOGENMIPMAP;

                // Create an empty texture.
                ID3D11Texture1D * tempTexture = CG_NULL;
                result = device->CreateTexture1D( &desc, CG_NULL, &tempTexture );
                if ( SUCCEEDED( result ) )
                {
                    tempTexture->QueryInterface( __uuidof( ID3D11Resource ), (void**)&texture );
                    tempTexture->Release();
                
                } // End if success
                break;

            } // End case Texture1D
            case cgBufferType::DepthStencil:
            case cgBufferType::ShadowMap:
            case cgBufferType::RenderTarget:
            case cgBufferType::RenderTargetCube:
            case cgBufferType::TextureCube:
            case cgBufferType::Texture2D:
            {
                // Build the info structure for the texture as we would like it to exist.
                D3D11_TEXTURE2D_DESC desc;
                desc.Width          = mInfo.width;
                desc.Height         = mInfo.height;
                desc.MipLevels      = mInfo.mipLevels;
                desc.ArraySize      = 1;
                desc.Format         = resourceFormat;
                desc.SampleDesc.Count   = 1;
                desc.SampleDesc.Quality = 0;
                desc.Usage          = (mInfo.dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
                desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = (mInfo.dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
                desc.MiscFlags      = 0;
                
                cgToDo( "DX11", "Support automatic mip generation!" );
                //if ( mInfo.autoGenerateMipmaps == true )
                    //desc.Usage |= D3DUSAGE_AUTOGENMIPMAP;

                // Customize settings and compatible bindings depending on type
                if ( mInfo.type == cgBufferType::RenderTarget )
                {
                    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                    desc.Usage = D3D11_USAGE_DEFAULT;
                    desc.CPUAccessFlags = 0;
                
                } // End if RenderTarget
                else if ( mInfo.type == cgBufferType::RenderTargetCube )
                {
                    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                    desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
                    desc.Usage = D3D11_USAGE_DEFAULT;
                    desc.CPUAccessFlags = 0;
                    desc.ArraySize  = 6;
                    
                } // End if RenderTargetCube
                else if ( mInfo.type == cgBufferType::TextureCube )
                {
                    desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
                    desc.ArraySize  = 6;
                    
                } // End if TextureCube
                else if ( mInfo.type == cgBufferType::DepthStencil ||
                          mInfo.type == cgBufferType::ShadowMap )
                {
                    desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
                    desc.Usage = D3D11_USAGE_DEFAULT;
                    desc.CPUAccessFlags = 0;
                
                } // End if DepthStencil | ShadowMap

                // Create an empty texture.
                ID3D11Texture2D * tempTexture = CG_NULL;
                result = device->CreateTexture2D( &desc, CG_NULL, &tempTexture );
                if ( SUCCEEDED( result ) )
                {
                    tempTexture->QueryInterface( __uuidof( ID3D11Resource ), (void**)&texture );
                    tempTexture->Release();
                
                } // End if success
                break;

            } // End case Texture2D | RenderTarget | ShadowMap | DepthStencil
            case cgBufferType::Texture3D:
            {
                // Build the info structure for the texture as we would like it to exist.
                D3D11_TEXTURE3D_DESC desc;
                desc.Width          = mInfo.width;
                desc.Height         = mInfo.height;
                desc.Depth          = mInfo.depth;
                desc.MipLevels      = mInfo.mipLevels;
                desc.Format         = resourceFormat;
                desc.Usage          = (mInfo.dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
                desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = (mInfo.dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
                desc.MiscFlags      = 0;
                
                cgToDo( "DX11", "Support automatic mip generation!" );
                //if ( mInfo.autoGenerateMipmaps == true )
                    //desc.Usage |= D3DUSAGE_AUTOGENMIPMAP;

                // Create an empty texture.
                ID3D11Texture3D * tempTexture = CG_NULL;
                result = device->CreateTexture3D( &desc, CG_NULL, &tempTexture );
                if ( SUCCEEDED( result ) )
                {
                    tempTexture->QueryInterface( __uuidof( ID3D11Resource ), (void**)&texture );
                    tempTexture->Release();
                
                } // End if success
                break;
            
            } // End case Texture3D
            case cgBufferType::DeviceBackBuffer:
            {
                // Device back buffer is "smart" managed (see cgDX11RenderTarget::createTexture())
                result = D3D_OK;
                supportsShaderView = false;
                break;
            
            } // End case DeviceBackBuffer

        } // End Switch type

    } // End if creating texture

    // Create the shader resource view for this texture.
    ID3D11ShaderResourceView * shaderView = CG_NULL;
    if ( SUCCEEDED( result ) && supportsShaderView )
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = viewFormat;
        
        switch ( mInfo.type )
        {
            case cgBufferType::Texture1D:
                viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
                viewDesc.Texture1D.MostDetailedMip = 0;
                viewDesc.Texture1D.MipLevels       = -1;
                break;

            case cgBufferType::Video:
            case cgBufferType::Texture2D:
            case cgBufferType::RenderTarget:
            case cgBufferType::DepthStencil:
            case cgBufferType::ShadowMap:
                viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                viewDesc.Texture2D.MostDetailedMip = 0;
                viewDesc.Texture2D.MipLevels       = -1;
                break;

            case cgBufferType::Texture3D:
                viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                viewDesc.Texture3D.MostDetailedMip = 0;
                viewDesc.Texture3D.MipLevels       = -1;
                break;

            case cgBufferType::TextureCube:
            case cgBufferType::RenderTargetCube:
                viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                viewDesc.TextureCube.MostDetailedMip = 0;
                viewDesc.TextureCube.MipLevels       = -1;
                break;

            case cgBufferType::DeviceBackBuffer:
                cgAssertEx( false, "Buffer type does not currently support shader resource view creation." );
                break;

        } // End resource type

        // Create the view
        result = device->CreateShaderResourceView( texture, &viewDesc, &shaderView );

    } // End if create view
    
    // Release D3D Device
    device->Release();

    // Success?
    if (FAILED(result))
    {
        if ( shaderView )
            shaderView->Release();
        if ( texture )
            texture->Release();
        return false;
    
    } // End if failed

    // Resource has now been loaded
    mTexture = texture;
    mShaderView = shaderView;

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
void cgDX11Texture<_BaseClass>::releaseTexture( )
{
    // We should release our underlying resource
    if ( mTexture )
        mTexture->Release();
    mTexture = CG_NULL;
    if ( mShaderView )
        mShaderView->Release();
    mShaderView = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void cgDX11Texture<_BaseClass>::deviceLost()
{
    // 7777 - Is this even necessary any more?
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
void cgDX11Texture<_BaseClass>::deviceRestored()
{
    // 7777 - Is this even necessary any more?
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
bool cgDX11Texture<_BaseClass>::getImageData( cgImage & imageOut )
{
    /*// ToDo: 9999 - Add error handling.
    LPDIRECT3DSURFACE9  pTextureSurface = NULL, pSrcSurface = NULL;
    D3DLOCKED_RECT      SrcData;
    D3DSURFACE_DESC     desc;

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
        texture->GetSurfaceLevel( 0, &pTextureSurface );
        texture->Release();

    } // End if standard / render target texture
    else if ( mInfo.type == cgBufferType::TextureCube || mInfo.type == cgBufferType::RenderTargetCube )
    {
        LPDIRECT3DCUBETEXTURE9 texture = NULL;
        if ( FAILED( mTexture->QueryInterface( IID_IDirect3DCubeTexture9, (void**)&texture ) ) )
            return false;

        // Read from the top level surface of the relevant face
        texture->GetCubeMapSurface( (D3DCUBEMAP_FACES)m_CurrentCubeFace, 0, &pTextureSurface );
        texture->Release();

    } // End if cube map
    else
    {
        // Invalid operation
        return false;
    
    } // End if other

    // If the output image buffer has not yet been allocated, we must do so first.
    pTextureSurface->GetDesc( &desc );
    if ( !ImageOut.IsValid() )
    {
        if ( !ImageOut.CreateImage( desc.width, desc.height, mInfo.format, false ) )
            return false;
    
    } // End if allocate
    cgUInt32 nWidth = ImageOut.GetWidth();
    cgUInt32 nHeight = ImageOut.GetHeight();

    // If this is a render target texture, we must first copy it into a system memory surface.
    if ( mInfo.type == cgBufferType::RenderTarget || mInfo.type == cgBufferType::RenderTargetCube )
    {
        IDirect3DDevice9 * device;
        
        // We need to decode the texture into a new surface.
        pTextureSurface->GetDevice( &pDevice );
        pDevice->CreateOffscreenPlainSurface( desc.width, desc.height, desc.format, D3DPOOL_SYSTEMMEM, &pSrcSurface, CG_NULL );
        pDevice->GetRenderTargetData( pTextureSurface, pSrcSurface );
        pDevice->Release();

        // Original surface can now be released.
        pTextureSurface->Release();
        pTextureSurface = CG_NULL;

        // The system memory surface now becomes our source surface.
        pTextureSurface = pSrcSurface;
        pSrcSurface = CG_NULL;

    } // End if render target

    // If the current texture format matches that required (in the image buffer) and the
    // requested size is identical, then we can directly copy from the texture's surface. 
    // If not, we must convert and/or downsample it first.
    if ( mInfo.format == ImageOut.GetFormat() && description.width == nWidth && description.height == nHeight )
    {
        // Just use the texture's surface.
        pSrcSurface = pTextureSurface;
        pTextureSurface = NULL;

    } // End if matched surface
    else
    {
        IDirect3DDevice9 * device;
        
        // We need to decode the texture into a new surface.
        pTextureSurface->GetDevice( &pDevice );
        cgToDoAssert( "DX11", "Consider supported formats!" );
        pDevice->CreateOffscreenPlainSurface( nWidth, nHeight, 
                                             (D3DFORMAT)cgDX11BufferFormatEnum::formatToNative(ImageOut.GetFormat()),
                                             D3DPOOL_SYSTEMMEM, &pSrcSurface, NULL );
        D3DXLoadSurfaceFromSurface( pSrcSurface, NULL, NULL, pTextureSurface, NULL, NULL, D3DX_DEFAULT, 0 );
        pDevice->Release();

        // Original surface can now be released.
        pTextureSurface->Release();
        pTextureSurface = NULL;

    } // End if mismatched surface

    // Lock the surface so that we can copy pixel data out.
    pSrcSurface->LockRect( &SrcData, NULL, D3DLOCK_READONLY );
    
    // Copy image data.
    cgByte * pSrcBuffer   = (cgByte*)SrcData.pBits;
    cgByte * pDestBuffer  = (cgByte*)ImageOut.getBuffer();
    cgUInt32 nPixelStride = ImageOut.GetBytesPerPixel();
    cgUInt32 nDestPitch   = ImageOut.GetPitch();
    for ( cgUInt32 y = 0; y < nHeight; ++y )
    {
        // Duplicate current scanline
        memcpy( pDestBuffer, pSrcBuffer, nWidth * nPixelStride );
        
        // Move down to next row
        pSrcBuffer  += SrcData.Pitch;
        pDestBuffer += nDestPitch;

    } // Next Row

    // Clean up
    pSrcSurface->UnlockRect( );
    pSrcSurface->Release();

    // Success!
    return true;*/
    // 7777
    return false;
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Allows the texture to update if it is decoding media perhaps.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
void cgDX11Texture<_BaseClass>::update()
{
    // Only video textures need further updating.
    if ( mInfo.type != cgBufferType::Video )
        return;

    // Call base class implementation first
    _BaseClass::update();

    // 7777
    /*// If automatic generation of mip maps is required, we should hint to the driver 
    // that we need new mip levels generating at this point (depending on how often
    // we would like them updating and whether or not a new video frame was supplied).
    if ( mInfo.autoGenerateMipmaps && m_bMipsDirty )
    {
        cgDouble fCurrentTime = mMediaDecoder->GetPlayheadTime();
        if ( m_MediaConfig.MipUpdateRate <= CGE_EPSILON || abs(fCurrentTime - m_fLastMipUpdateTime) >= (1.0 / (cgDouble)m_MediaConfig.MipUpdateRate) )
        {
            updateMipLevels();

            // Mips have been generated and are no longer dirty.
            m_fLastMipUpdateTime = fCurrentTime;
        
        } // End if time to update
    
    } // End if update mips*/
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
template <class _BaseClass>
bool cgDX11Texture<_BaseClass>::loadResource( )
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
bool cgDX11Texture<_BaseClass>::unloadResource( )
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
bool cgDX11Texture<_BaseClass>::updateMipLevels( )
{
	/*
	// Mark the whole surface area as dirty
    if ( mInfo.type == cgBufferType::TextureCube || mInfo.type == cgBufferType::RenderTargetCube )
        ((LPDIRECT3DCUBETEXTURE9)mTexture)->AddDirtyRect( (D3DCUBEMAP_FACES)m_CurrentCubeFace, CG_NULL ); 
    else
        ((LPDIRECT3DTEXTURE9)mTexture)->AddDirtyRect( CG_NULL ); 

	// Ask the texture to update its mip levels
	mTexture->GenerateMipSubLevels();
	*/

	// Mips have been generated and are no longer dirty.
	mMipsDirty = false;

	// Return success
	return true;
}

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11TEXTURE_H_