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
// Name : cgImage.cpp                                                        //
//                                                                           //
// Desc : Simple image classes, designed to manage and allow manipulation of //
//        image data at the pixel level.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2010 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgImage Module Includes
//-----------------------------------------------------------------------------
#include <System/cgImage.h>
#include <System/Platform/cgWinImage.h>
#include <Resources/cgBufferFormatEnum.h>

//-----------------------------------------------------------------------------
//  Name : cgImage () (Constructor)
/// <summary>
/// cgImage Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgImage::cgImage()
{
	mWidth            = 0;
    mHeight           = 0;
    mPitch            = 0;
    mFormat            = cgBufferFormat::Unknown;
    mBuffer           = CG_NULL;
    mDrawing          = false;
    mPenWidth         = 1.0f;
    mPenColor          = cgColorValue( 1, 1, 1, 1 );
    mPenStartCapStyle  = PenCapSquare;
    mPenEndCapStyle    = PenCapSquare;
    mBrushColor        = cgColorValue( 1, 1, 1, 1 );
}

//-----------------------------------------------------------------------------
//  Name : cgImage () (Destructor)
/// <summary>
/// cgImage Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgImage::~cgImage()
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
void cgImage::dispose( bool bDisposeBase )
{
    // Finish drawing if active.
    endDraw();

    // Clean up allocated memory.
    delete []mBuffer;

    // Clear variables
    mWidth    = 0;
    mHeight   = 0;
    mPitch    = 0;
    mFormat    = cgBufferFormat::Unknown;
    mBuffer   = CG_NULL;

    // Dispose of base class
    if ( bDisposeBase == true )
        DisposableScriptObject::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgImage * cgImage::createInstance()
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinImage();
    
    } // End Switch platform
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgImage * cgImage::createInstance( cgUInt32 Width, cgUInt32 Height, cgBufferFormat::Base Format, bool bHardwareBuffer /* = false */ )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinImage( Width, Height, Format, bHardwareBuffer );
    
    } // End Switch platform
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getWidth () 
/// <summary>
/// Returns the width the underlying image data in pixels
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgImage::getWidth( ) const
{
    return mWidth;
}

//-----------------------------------------------------------------------------
//  Name : getHeight () 
/// <summary>
/// Returns the height the underlying image data in pixels
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgImage::getHeight( ) const
{
    return mHeight;
}

//-----------------------------------------------------------------------------
//  Name : getPitch () 
/// <summary>
/// Returns the pitch of an individual row in the data set, in bytes.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgImage::getPitch( ) const
{
    return mPitch;
}

//-----------------------------------------------------------------------------
//  Name : getFormat () 
/// <summary>
/// Retrieve the pixel format of the image data.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgImage::getFormat( ) const
{
    return mFormat;
}

//-----------------------------------------------------------------------------
//  Name : getBitsPerPixel () 
/// <summary>
/// Retrieve the number of bits consumed by each pixel in the image data.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt8 cgImage::getBitsPerPixel( ) const
{
    return cgBufferFormatEnum::formatBitsPerPixel( mFormat );
}

//-----------------------------------------------------------------------------
//  Name : getBytesPerPixel () 
/// <summary>
/// Retrieve the number of bytes consumed by each pixel in the image data.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt8 cgImage::getBytesPerPixel( ) const
{
    return cgBufferFormatEnum::formatBitsPerPixel( mFormat ) / 8;
}

//-----------------------------------------------------------------------------
//  Name : isValid () 
/// <summary>
/// Determine if the image data is valid and ready to read / write.
/// </summary>
//-----------------------------------------------------------------------------
bool cgImage::isValid( ) const
{
    return (mBuffer != CG_NULL);
}

//-----------------------------------------------------------------------------
//  Name : getBuffer () 
/// <summary>
/// Retrieve a pointer to the underlying image buffer.
/// </summary>
//-----------------------------------------------------------------------------
const cgByte * cgImage::getBuffer( ) const
{
    return mBuffer;
}

//-----------------------------------------------------------------------------
//  Name : getBuffer () 
/// <summary>
/// Retrieve a pointer to the underlying image buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgByte * cgImage::getBuffer( )
{
    return mBuffer;
}

//-----------------------------------------------------------------------------
//  Name : getPenWidth () 
/// <summary>
/// Retrieve the current drawing pen width in pixels.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgImage::getPenWidth( ) const
{
    return mPenWidth;
}

//-----------------------------------------------------------------------------
//  Name : getPenColor () 
/// <summary>
/// Retrieve the current drawing pen color.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgImage::getPenColor( ) const
{
    return mPenColor;
}

//-----------------------------------------------------------------------------
//  Name : getPenStartCap () 
/// <summary>
/// Retrieve the cap style to use at the start of a line / curve.
/// </summary>
//-----------------------------------------------------------------------------
cgImage::PenCapStyle cgImage::getPenStartCap( ) const
{
    return mPenStartCapStyle;
}

//-----------------------------------------------------------------------------
//  Name : getPenEndCap () 
/// <summary>
/// Retrieve the cap style to use at the end of a line / curve.
/// </summary>
//-----------------------------------------------------------------------------
cgImage::PenCapStyle cgImage::getPenEndCap( ) const
{
    return mPenEndCapStyle;
}

//-----------------------------------------------------------------------------
//  Name : getBrushColor () 
/// <summary>
/// Retrieve the current filling brush color.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgImage::getBrushColor( ) const
{
    return mBrushColor;
}
    
//-----------------------------------------------------------------------------
//  Name : setPenWidth () (Virtual)
/// <summary>
/// Set the current drawing pen width in pixels.
/// </summary>
//-----------------------------------------------------------------------------
void cgImage::setPenWidth( cgFloat fWidth )
{
    mPenWidth = fWidth;
}

//-----------------------------------------------------------------------------
//  Name : setPenColor () (Virtual)
/// <summary>
/// Set the current drawing pen color.
/// </summary>
//-----------------------------------------------------------------------------
void cgImage::setPenColor( const cgColorValue & Color )
{
    mPenColor = Color;
}

//-----------------------------------------------------------------------------
//  Name : setPenStartCap () (Virtual)
/// <summary>
/// Set the cap style to use at the start of a line / curve.
/// </summary>
//-----------------------------------------------------------------------------
void cgImage::setPenStartCap( PenCapStyle Style )
{
    mPenStartCapStyle = Style;
}

//-----------------------------------------------------------------------------
//  Name : setPenEndCap () (Virtual)
/// <summary>
/// Set the cap style to use at the start of a line / curve.
/// </summary>
//-----------------------------------------------------------------------------
void cgImage::setPenEndCap( PenCapStyle Style )
{
    mPenEndCapStyle = Style;
}

//-----------------------------------------------------------------------------
//  Name : setBrushColor () (Virtual)
/// <summary>
/// Set the current filling brush color.
/// </summary>
//-----------------------------------------------------------------------------
void cgImage::setBrushColor( const cgColorValue & Color )
{
    mBrushColor = Color;
}

//-----------------------------------------------------------------------------
//  Name : createImage () (Virtual)
/// <summary>
/// Allocate memory for storage of the image with the specified dimensions
/// and in the appropriate format. Derived classes must supply all creation
/// functionality if the user requested a hardware buffer and should only
/// call this base class method if they did not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgImage::createImage( cgUInt32 Width, cgUInt32 Height, cgBufferFormat::Base Format, bool bHardwareBuffer /* = false */ )
{
    // Validate requirements.
    if ( Width == 0 || Height == 0 || bHardwareBuffer == true )
        return false;

    // Fail if the bits per pixel for this format is not a multiple of 8 (i.e. cgBufferFormat::R1)
    cgUInt32 nBitsPerPixel = cgBufferFormatEnum::formatBitsPerPixel( Format );
    if ( nBitsPerPixel % 8 )
        return false;

    // Compute the necessary pitch for the desired format.
    mPitch = Width * (nBitsPerPixel / 8);

    // Valid resulting pitch?
    if ( mPitch == 0 )
        return false;
    
    // Release any prior buffer.
    delete []mBuffer;

    // Allocate new buffer.
    mBuffer = new cgByte[mPitch * Height];

    // Store selected values.
    mWidth    = Width;
    mHeight   = Height;
    mFormat    = Format;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginDraw () (Virtual)
/// <summary>
/// Begin drawing to the buffer (line, circle, etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgImage::beginDraw( )
{
    mDrawing = true;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : drawLine () (Virtual)
/// <summary>
/// Draw a line between the specified points with the currently applied pen
/// details.
/// </summary>
//-----------------------------------------------------------------------------
void cgImage::drawLine( cgFloat fFromX, cgFloat fFromY, cgFloat fToX, cgFloat fToY )
{
    // Not currently implemented in base / software buffer class.
}

//-----------------------------------------------------------------------------
//  Name : fillCircle () (Virtual)
/// <summary>
/// Fill a circle at the specified coordinate with the currently applied brush
/// details.
/// </summary>
//-----------------------------------------------------------------------------
void cgImage::fillCircle( cgFloat fX, cgFloat fY, cgFloat fRadius )
{
    // Not currently implemented in base / software buffer class.
}

//-----------------------------------------------------------------------------
//  Name : endDraw () (Virtual)
/// <summary>
/// Complete drawing to the buffer (line, circle, etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgImage::endDraw( )
{
    mDrawing = false;
}