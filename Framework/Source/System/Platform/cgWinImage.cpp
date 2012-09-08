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
// Name : cgWinImage.cpp                                                     //
//                                                                           //
// Desc : Provides image functionality specific to the Windows(tm) platform, //
//        with hardware buffer support supplied by GDI.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2010 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWinImage Module Includes
//-----------------------------------------------------------------------------
#include <System/Platform/cgWinImage.h>
#include <Resources/cgBufferFormatEnum.h>

// Windows platform Includes
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gdiplus.h>
#define WIN32_LEAN_AND_MEAN
#else
#include <windows.h>
#include <gdiplus.h>
#endif

//-----------------------------------------------------------------------------
//  Name : cgWinImage () (Constructor)
/// <summary>
/// cgWinImage Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinImage::cgWinImage() : 
    mHardwareBuffer(false), 
    mBitmap( CG_NULL ),
    mGraphics( CG_NULL ),
    cgImage()
{
}

//-----------------------------------------------------------------------------
//  Name : cgWinImage () (Constructor)
/// <summary>
/// cgWinImage Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinImage::cgWinImage( cgUInt32 Width, cgUInt32 Height, cgBufferFormat::Base Format, bool bHardwareBuffer /* = false */ ) :
    mHardwareBuffer( false ),
    mBitmap( CG_NULL ),
    mGraphics( CG_NULL ),
    cgImage( )
{

    // Create the image buffers.
    createImage( Width, Height, Format, bHardwareBuffer );
}

//-----------------------------------------------------------------------------
//  Name : cgWinImage () (Destructor)
/// <summary>
/// cgWinImage Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinImage::~cgWinImage()
{
    // Clean up allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgWinImage::dispose( bool bDisposeBase )
{
    // Clean up drawing.
    endDraw();

    // Destroy hardware bitmap data if it was created.
    delete mBitmap;
    
    // Clear variables
    mBitmap = CG_NULL;
    mHardwareBuffer = false;

    // Dispose of base class
    if ( bDisposeBase == true )
        cgImage::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : createImage () (Virtual)
/// <summary>
/// Allocate memory for storage of the image with the specified dimensions
/// and in the appropriate format.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinImage::createImage( cgUInt32 Width, cgUInt32 Height, cgBufferFormat::Base Format, bool bHardwareBuffer /* = false */ )
{
    // Validate requirements
    if ( Width == 0 || Height == 0 )
        return false;

    // If this is to be a hardware buffer, it is our responsibility to
    // perform all allocation and management.
    if ( bHardwareBuffer == true )
    {
        // Supported format?
        Gdiplus::PixelFormat gdiplusFormat;
        switch ( Format )
        {
            case cgBufferFormat::B8G8R8A8:
                gdiplusFormat = PixelFormat32bppARGB;
                break;
            case cgBufferFormat::B8G8R8X8:
                gdiplusFormat = PixelFormat32bppRGB;
                break;
            case cgBufferFormat::B8G8R8:
                gdiplusFormat = PixelFormat24bppRGB;
                break;
            case cgBufferFormat::B5G5R5A1:
                gdiplusFormat = PixelFormat16bppARGB1555;
                break;
            case cgBufferFormat::B5G5R5X1:
                gdiplusFormat = PixelFormat16bppRGB555;
                break;
            case cgBufferFormat::B5G6R5:
                gdiplusFormat = PixelFormat16bppRGB565;
                break;
            default:
                // Unsupported
                return false;
        
        } // End switch Format
        
        // In the case of GDI+ wrapped bitmap, the buffer must have
        // a row size in multiples of 4 bytes (32 bits).
        cgUInt8  nBytesPerPixel = cgBufferFormatEnum::formatBitsPerPixel( Format ) / 8;
        cgUInt32 nPitch = ((Width * nBytesPerPixel) + 3) & 0xFFFFFFFC;

        // Valid resulting (or supplied) pitch?
        if ( nPitch == 0 )
            return false;

        // Dispose of all prior data
        dispose( false );

        // Allocate the new underlying buffer.
        mBuffer = new cgByte[ nPitch * Height ];
        memset( mBuffer, 0, nPitch * Height );

        // Allocate a new bitmap to wrap this buffer
        mBitmap = new Gdiplus::Bitmap( Width, Height, nPitch, gdiplusFormat, mBuffer );

        // Store selected values.
        mWidth            = Width;
        mHeight           = Height;
        mFormat            = Format;
        mPitch            = nPitch;
        mHardwareBuffer   = true;

        // Success!
        return true;

    } // End if hardware
    
    // Otherwise, base class can handle it.
    return cgImage::createImage( Width, Height, Format, false );
}

//-----------------------------------------------------------------------------
//  Name : isHardwareBuffer () (Virtual)
/// <summary>
/// Determine if the image is using a hardware buffer internally.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinImage::isHardwareBuffer( ) const
{
    return mHardwareBuffer;
}

//-----------------------------------------------------------------------------
//  Name : beginDraw () (Virtual)
/// <summary>
/// Begin drawing to the buffer (line, circle, etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinImage::beginDraw( )
{
    // No-op?
    if ( mDrawing == true )
        return true;

    // Hardware rendering?
    if ( mHardwareBuffer == true )
    {
        // No valid bitmap?
        if ( mBitmap == CG_NULL )
            return false;

        // Create a graphics context with which we can draw.
        mGraphics = Gdiplus::Graphics::FromImage( mBitmap );

    } // End if hardware

    // Call base class implementation last
    return cgImage::beginDraw( );
}

//-----------------------------------------------------------------------------
//  Name : drawLine () (Virtual)
/// <summary>
/// Draw a line between the specified points with the currently applied pen
/// details.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinImage::drawLine( cgFloat fFromX, cgFloat fFromY, cgFloat fToX, cgFloat fToY )
{
    // Hardware rendering?
    if ( mHardwareBuffer == true )
    {
        // Validate requirements
        if ( mGraphics == CG_NULL )
            return;
        
        // ToDo: Only generate new pen when details change (i.e. SetPenWidth() etc.)
        // Generate GDI+ compatible line drawing pen.
        Gdiplus::Pen * pPenLine = new Gdiplus::Pen( Gdiplus::Color(mPenColor), mPenWidth );
        pPenLine->SetStartCap( (Gdiplus::LineCap)mPenStartCapStyle );
        pPenLine->SetEndCap( (Gdiplus::LineCap)mPenEndCapStyle );

        // Draw the line.
        mGraphics->DrawLine( pPenLine, fFromX, fFromY, fToX, fToY );

        // Clean up
        delete pPenLine;

    } // End if hardware buffer.

    // Otherwise, simply call base class implementation.
    cgImage::drawLine( fFromX, fFromY, fToX, fToY );
}

//-----------------------------------------------------------------------------
//  Name : fillCircle () (Virtual)
/// <summary>
/// Draw a filled circle at the specified coordinate with the currently applied 
/// brush details.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinImage::fillCircle( cgFloat fX, cgFloat fY, cgFloat fRadius )
{
    // Hardware rendering?
    if ( mHardwareBuffer == true )
    {
        // Validate requirements
        if ( mGraphics == CG_NULL )
            return;
        
        // ToDo: Only generate new brush when details change (i.e. SetBrushColor() etc.)
        // Generate GDI+ compatible filling brush.
        Gdiplus::Brush * pBrushCircle = new Gdiplus::SolidBrush( Gdiplus::Color(mBrushColor) );
        
        // Draw the circle.
        mGraphics->FillEllipse( pBrushCircle, fX - fRadius, fY - fRadius, fRadius * 2, fRadius * 2 );

        // Clean up
        delete pBrushCircle;

    } // End if hardware buffer.

    // Otherwise, simply call base class implementation.
    cgImage::fillCircle( fX, fY, fRadius );
}

//-----------------------------------------------------------------------------
//  Name : endDraw () (Virtual)
/// <summary>
/// Complete drawing to the buffer (line, circle, etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgWinImage::endDraw( )
{
    // No-op?
    if ( mDrawing == false )
        return;

    // Release buffer(s) when hardware rendering.
    if ( mHardwareBuffer == true )
    {
        // Destroy our graphics context.
        delete mGraphics;
        mGraphics = CG_NULL;
        
    } // End if hardware

    // Call base class implementation last
    cgImage::endDraw( );
}