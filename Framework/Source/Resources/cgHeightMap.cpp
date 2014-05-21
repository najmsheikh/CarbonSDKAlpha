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
// File : cgHeightMap.cpp                                                    //
//                                                                           //
// Desc : Contains classes and utilities for the creation and management of  //
//        heightmap data, including various functions for heightmap          //
//        modification.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgHeightMap Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgHeightMap.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
// cgHeightMap Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgHeightMap() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgHeightMap::cgHeightMap( )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
// Name : cgHeightMap() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgHeightMap::cgHeightMap( const cgHeightMap & init )
{
    // Clone heightmap.
    mSize      = init.mSize;
    mImageData = init.mImageData;
}

//-----------------------------------------------------------------------------
// Name : cgHeightMap() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgHeightMap::cgHeightMap( const cgSize & size )
{
    // Initialize variables to sensible defaults
    mImageData.resize( size.width * size.height );
    mSize = size;
}

//-----------------------------------------------------------------------------
// Name : cgHeightMap() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgHeightMap::cgHeightMap( const cgSize & size, cgInt16 initValue )
{
    // Initialize variables to sensible defaults
    mImageData.resize( size.width * size.height, initValue );
    mSize = size;
}

//-----------------------------------------------------------------------------
// Name : ~cgHeightMap() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgHeightMap::~cgHeightMap( )
{
    mImageData.clear();
}

//-----------------------------------------------------------------------------
// Name : getCell ()
/// <summary>
/// Retrieve the height at the specified pixel location.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgHeightMap::getCell( cgInt32 x, cgInt32 y )
{
    // Retrieve the underlying pixel
    return mImageData[ x + y * mSize.width ];
}

//-----------------------------------------------------------------------------
// Name : getInterpolatedCell ()
/// <summary>
/// Retrieve the height at the specified pixel location using interpolation.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgHeightMap::getInterpolatedCell( cgFloat x, cgFloat y )
{
    // Retrieve the underlying pixel
    if ( x >= 0 && x < (mSize.width-1) && y >= 0 && y < (mSize.height-1) )
    {
        cgFloat topLeft, topRight, bottomLeft, bottomRight;

        // First retrieve the Heightmap Points
        cgInt ix = (cgInt)x;
        cgInt iy = (cgInt)y;
    	
        // Calculate the remainder (percent across quad)
        cgFloat deltaX = x - (cgFloat)ix;
        cgFloat deltaY = y - (cgFloat)iy;

        // First retrieve the height of each point in the dividing edge
        topLeft     = (cgFloat)mImageData[ ix + iy * mSize.width ];
        bottomRight = (cgFloat)mImageData[ (ix + 1) + (iy + 1) * mSize.width ];

        // Which triangle of the quad are we in ?
        if ( deltaX < deltaY )
        {
            bottomLeft = (cgFloat)mImageData[ ix + (iy + 1) * mSize.width ];
	        topRight = topLeft + (bottomRight - bottomLeft);
        
        } // End if left Triangle
        else
        {
            topRight   = (cgFloat)mImageData[ (ix + 1) + iy * mSize.width ];
	        bottomLeft = topLeft + (bottomRight - topRight);

        } // End if Right Triangle
        
        // Calculate the height interpolated across the top and bottom edges
        cgFloat topHeight    = topLeft    + ((topRight - topLeft) * deltaX );
        cgFloat bottomHeight = bottomLeft + ((bottomRight - bottomLeft) * deltaX );

        // Calculate the resulting height interpolated between the two heights
        return topHeight + ((bottomHeight - topHeight) * deltaY );
    
    } // End if valid

    // Invalid offset / heightmap
    return 0.0f;
}

//-----------------------------------------------------------------------------
// Name : setCell ()
/// <summary>
/// Set the height at the specified pixel location.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::setCell( cgInt32 x, cgInt32 y, cgInt16 value )
{
    // Set the height
    mImageData[ x + y * mSize.width ] = value;
}

//-----------------------------------------------------------------------------
// Name : offsetCell ()
/// <summary>
/// Adjust the height of the pixel at the location specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::offsetCell( cgInt32 x, cgInt32 y, cgInt32 offset )
{
    // Validate.
    if ( x >= mSize.width || y >= mSize.height )
        return;

    // Generate the new height value
    cgInt32 newHeight = (cgInt32)mImageData[ x + y * mSize.width ];
    newHeight += offset;

    // Clamp it
    if ( newHeight < MinCellHeight )
        newHeight = MinCellHeight;
    if ( newHeight > MaxCellHeight )
        newHeight = MaxCellHeight;

    // Store the new height
    mImageData[ x + y * mSize.width ] = (cgInt16)newHeight;
}

//-----------------------------------------------------------------------------
// Name : loadRaw ()
/// <summary>
/// Load raw data from the file specified and populate our internal buffer.
/// The width, height and format details specified must match those of the raw 
/// file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHeightMap::loadRaw( cgInputStream stream, const cgSize & size, RawFormat format )
{
    // Validate requirements
    if ( size.width <= 0 || size.height <= 0 )
        return false;

    // Compute the expected size of the RAW file (in bytes)
    cgInt64 expectedLength = 0;
    cgInt32 rowPitch = 0;
    switch ( format )
    {
        case Gray8:
            expectedLength = (size.width * size.height);
            rowPitch = size.width;
            break;

        case Gray16:
            expectedLength = (size.width * size.height) * 2;
            rowPitch = size.width * 2;
            break;
        
        default:
            // No such format
            return false;
    
    } // End format switch

    // Open the stream
    if ( !stream.open( ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to open requested raw image file '%s' for import.\n"), stream.getName().c_str() );
        return false;
    
    } // End if failed

     // Retrieve the size of the file so that we can validate their options
    cgInt64 dataLength = stream.getLength();

    // Sizes do not match?
    if ( expectedLength != dataLength )
    {
        stream.close( );
        cgAppLog::write( cgAppLog::Error, _T("RAW image file size %i did not match the expected %i size based on supplied parameters.\n"), dataLength, expectedLength );
        return false;
    
    } // End if mismatch

    // Allocate image data
    mImageData.clear();
    mImageData.resize( (size_t)dataLength );
    mSize = size;

    // Load the raw data from file a row at a time.
    cgByte * row = new cgByte[rowPitch], * data;
    for ( cgInt32 y = 0, counter = 0; y < mSize.height; ++y )
    {
        // Read a whole row of data.
        stream.read( row, rowPitch );
        data = row;

        // Extract pixels
        for ( cgInt32 x = 0; x < mSize.width; ++x )
        {
            if ( format == Gray8 )
            {
                // scale up to the MinCellHeight->MaxCellHeight range.
                mImageData[counter++] = (cgInt16)((*data++) * (MaxCellHeight/255));

            } // End if 8 bit grayscale
            else if ( format == Gray16 )
            {
                mImageData[counter++] = *(cgInt16*)data;
                data += 2;

            } // End if 16 bit grayscale

        } // Next Column

    } // Next Row

    // Clean up
    delete []row;
    stream.close();
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : loadSquareRaw ()
/// <summary>
/// Loads raw data in the same fashion as the 'loadRaw' function, however
/// the width and height of the image will be automatically detected.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHeightMap::loadSquareRaw( cgInputStream stream, RawFormat format )
{
    // Retrieve the size of the file so that we can validate
    cgInt64 dataLength = stream.getLength();

    // Compute the the dimensions of the RAW file (in pixels) given the specified format.
    cgInt32 imageSide = 0;
    switch ( format )
    {
        case Gray8:
            imageSide = (cgInt32)sqrtf( (cgFloat)dataLength );
            break;

        case Gray16:
            imageSide = (cgInt32)sqrtf( (cgFloat)dataLength / 2 );
            break;
        
        default:
            // No such format
            return false;
    
    } // End format switch

    // Pass through to standard load function/
    return loadRaw( stream, cgSize(imageSide, imageSide), format );
}

//-----------------------------------------------------------------------------
// Name : computeHeightRange ()
/// <summary>
/// Compute the minimum and maximum height values currently stored in the
/// height map buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::computeHeightRange( cgInt32 & minOut, cgInt32 & maxOut ) const
{
    computeHeightRange( minOut, maxOut, cgRect(0,0,mSize.width,mSize.height) );
}

//-----------------------------------------------------------------------------
// Name : computeHeightRange ()
/// <summary>
/// Compute the minimum and maximum height values currently stored in the
/// specified region of the height map buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::computeHeightRange( cgInt32 & minOut, cgInt32 & maxOut, const cgRect & section ) const
{
    minOut = MaxCellHeight;
    maxOut = MinCellHeight;
    const cgInt16 * buffer = &mImageData[section.left + section.top * mSize.width];
    for ( cgInt32 y = section.top; y < section.bottom; ++y )
    {
        for ( cgInt32 x = section.left; x < section.right; ++x, ++buffer )
        {
            if ( *buffer < minOut )
                minOut = *buffer;
            if ( *buffer > maxOut )
                maxOut = *buffer;

        } // Next Column

        // Move to start of next row.
        buffer += mSize.width - section.width();

    } // Next Row
}

//-----------------------------------------------------------------------------
// Name : normalize ()
/// <summary>
/// normalize the heightmap into the specified range.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::normalize( cgInt32 minimum, cgInt32 maximum )
{
    // First determine the current min / max range.
    cgInt32 originalMin, originalMax;
    computeHeightRange( originalMin, originalMax );

    // If landscape is completely flat, we can't normalize.
    if ( originalMax == originalMin )
        return;

    // Compute scale
    cgFloat scale = (cgFloat)(maximum - minimum) / (cgFloat)(originalMax - originalMin);

    // normalize values
    cgInt16 * heightMap = &mImageData[0];
    for ( cgInt32 y = 0; y < mSize.height; ++y )
    {
        for ( cgInt32 x = 0; x < mSize.width; ++x, ++heightMap )
        {
            cgInt32 value = (cgInt32)((cgFloat)((cgInt32)*heightMap - originalMin) * scale + minimum);
            if ( value > MaxCellHeight )
                value = MaxCellHeight;
            if ( value < MinCellHeight )
                value = MinCellHeight;
            *heightMap = (cgInt16)value;

        } // Next Column

    } // Next Row
}

//-----------------------------------------------------------------------------
// Name : scale ()
/// <summary>
/// Scale the heightmap cells by the specified amount. Values will be clamped
/// at the min / max limits.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::scale( cgInt16 scale )
{
    // Scale values
    cgInt16 * heightMap = &mImageData[0];
    for ( cgInt32 y = 0; y < mSize.height; ++y )
    {
        for ( cgInt32 x = 0; x < mSize.width; ++x, ++heightMap )
        {
            cgInt32 value = ((cgInt32)*heightMap) * scale;
            if ( value > MaxCellHeight )
                value = MaxCellHeight;
            if ( value < MinCellHeight )
                value = MinCellHeight;
            *heightMap = (cgInt16)value;

        } // Next Column

    } // Next Row
}

//-----------------------------------------------------------------------------
// Name : loadRawSection () (Static)
/// <summary>
/// Utility function to load raw 16 bit heightmap data from a specific location 
/// in a file, without having to load the entire heightmap first.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHeightMap::loadRawSection( cgInputStream stream, const cgSize & size, RawFormat format, const cgRect & section, cgInt16Array & dataOut )
{
    // Validate requirements
    if ( size.width <= 0 || size.height <= 0 )
        return false;

    // Compute the expected size of the RAW file (in bytes)
    cgInt64 expectedLength = 0;
    cgInt32 pixelStride;
    switch ( format )
    {
        case Gray8:
            expectedLength = (size.width * size.height);
            pixelStride    = 1;
            break;

        case Gray16:
            expectedLength = (size.width * size.height) * 2;
            pixelStride    = 2;
            break;
        
        default:
            // No such format
            return false;
    
    } // End format switch

    // Pre-compute width and height values for efficient testing
    cgInt32 sectionWidth  = section.width();
    cgInt32 sectionHeight = section.height();

    // Calculate the intersection of the heightmap total rectangle and the specified
    // rectangle to ensure that we are not attempting to read out of bounds.
    cgRect intersect = cgRect::intersect( cgRect( 0, 0, size.width, size.height ), section );
    
    // Ensure that the specified rectangle is valid in position and size.
    if ( intersect != section || sectionWidth < 1 || sectionHeight < 1 )
        return false;

    // Retrieve the size of the file so that we can validate their options
    cgInt64 dataLength = stream.getLength();

    // Sizes do not match?
    if ( expectedLength != dataLength )
    {
        cgAppLog::write( cgAppLog::Error, _T("RAW image file size %i did not match the expected %i size based on supplied parameters.\n"), dataLength, expectedLength );
        return false;
    
    } // End if mismatch

    // Open the stream
    if ( stream.open( ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to open requested raw image file '%s' for import.\n"), stream.getName().c_str() );
        return false;
    
    } // End if failed

    // Ensure output buffer is large enough to contain the image data.
    if ( dataOut.size() < (size_t)(sectionWidth * sectionHeight) )
        dataOut.resize( (sectionWidth * sectionHeight) );
    
    // Seek to the correct initial position in the heightmap
    cgInt32 inputPitch  = size.width * pixelStride;
    cgInt32 outputPitch = sectionWidth * pixelStride;
    stream.seek( (section.left * pixelStride) + (section.top * inputPitch), cgInputStream::Begin );
        
    // Load the raw data from file a row at a time.
    cgByte * row = new cgByte[outputPitch], * data;
    for ( cgInt32 y = 0, counter = 0; y < sectionHeight; ++y )
    {
        // Read a whole row of data.
        stream.read( row, outputPitch );
        data = row;

        // Extract pixels
        for ( cgInt32 x = 0; x < sectionWidth; ++x )
        {
            if ( format == Gray8 )
            {
                // Scale up to the MinCellHeight->MaxCellHeight range.
                dataOut[counter++] = (cgInt16)((*data++) * (MaxCellHeight/255));

            } // End if 8 bit grayscale
            else if ( format == Gray16 )
            {
                dataOut[counter++] = *(cgInt16*)data;
                data += 2;

            } // End if 16 bit grayscale

        } // Next Column

        // Seek to the next position in the heightmap
        stream.seek( inputPitch - outputPitch, cgInputStream::Current );

    } // Next Row
        
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : fill ( )
/// <summary>
/// Fill the entire heightmap with the specified value.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::fill( cgInt16 value )
{
    if ( !mImageData.empty() )
        std::fill( mImageData.begin(), mImageData.end(), value );
}

//-----------------------------------------------------------------------------
// Name : getSize ( )
/// <summary>
/// Retrieve the size of the loaded heightmap data in pixels.
/// </summary>
//-----------------------------------------------------------------------------
const cgSize & cgHeightMap::getSize( ) const
{
    return mSize;
}

//-----------------------------------------------------------------------------
// Name : getImageData ( )
/// <summary>
/// Retrieve the underlying 16 bit grayscale heightmap pixel data.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16Array & cgHeightMap::getImageData( )
{
    return mImageData;
}

//-----------------------------------------------------------------------------
// Name : getImageData ( ) (Const overload)
/// <summary>
/// Retrieve the underlying 16 bit grayscale heightmap pixel data.
/// </summary>
//-----------------------------------------------------------------------------
const cgInt16Array & cgHeightMap::getImageData( ) const
{
    return mImageData;
}