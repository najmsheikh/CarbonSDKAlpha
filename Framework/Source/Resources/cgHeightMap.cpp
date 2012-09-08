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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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
cgHeightMap::cgHeightMap( const cgHeightMap & Init )
{
    // Clone heightmap.
    mSize       = Init.mSize;
    mImageData = Init.mImageData;
}

//-----------------------------------------------------------------------------
// Name : cgHeightMap() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgHeightMap::cgHeightMap( const cgSize & Size )
{
    // Initialize variables to sensible defaults
    mImageData.resize( Size.width * Size.height );
    mSize = Size;
}

//-----------------------------------------------------------------------------
// Name : cgHeightMap() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgHeightMap::cgHeightMap( const cgSize & Size, cgInt16 nInit )
{
    // Initialize variables to sensible defaults
    mImageData.resize( Size.width * Size.height, nInit );
    mSize = Size;
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
        cgFloat fTopLeft, fTopRight, fBottomLeft, fBottomRight;

        // First retrieve the Heightmap Points
        cgInt iX = (cgInt)x;
        cgInt iY = (cgInt)y;
    	
        // Calculate the remainder (percent across quad)
        cgFloat fPercentX = x - (cgFloat)iX;
        cgFloat fPercentY = y - (cgFloat)iY;

        // First retrieve the height of each point in the dividing edge
        fTopLeft     = (cgFloat)mImageData[ iX + iY * mSize.width ];
        fBottomRight = (cgFloat)mImageData[ (iX + 1) + (iY + 1) * mSize.width ];

        // Which triangle of the quad are we in ?
        if ( fPercentX < fPercentY )
        {
            fBottomLeft = (cgFloat)mImageData[ iX + (iY + 1) * mSize.width ];
	        fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
        
        } // End if left Triangle
        else
        {
            fTopRight   = (cgFloat)mImageData[ (iX + 1) + iY * mSize.width ];
	        fBottomLeft = fTopLeft + (fBottomRight - fTopRight);

        } // End if Right Triangle
        
        // Calculate the height interpolated across the top and bottom edges
        cgFloat fTopHeight    = fTopLeft    + ((fTopRight - fTopLeft) * fPercentX );
        cgFloat fBottomHeight = fBottomLeft + ((fBottomRight - fBottomLeft) * fPercentX );

        // Calculate the resulting height interpolated between the two heights
        return fTopHeight + ((fBottomHeight - fTopHeight) * fPercentY );
    
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
void cgHeightMap::setCell( cgInt32 x, cgInt32 y, cgInt16 nValue )
{
    // Set the height
    mImageData[ x + y * mSize.width ] = nValue;
}

//-----------------------------------------------------------------------------
// Name : offsetCell ()
/// <summary>
/// Adjust the height of the pixel at the location specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::offsetCell( cgInt32 x, cgInt32 y, cgInt32 nOffset )
{
    cgInt32 nNewHeight;

    // Validate.
    if ( x >= mSize.width || y >= mSize.height )
        return;

    // Generate the new height value
    nNewHeight  = (cgInt32)mImageData[ x + y * mSize.width ];
    nNewHeight += nOffset;

    // Clamp it
    if ( nNewHeight < MinCellHeight ) nNewHeight = MinCellHeight;
    if ( nNewHeight > MaxCellHeight ) nNewHeight = MaxCellHeight;

    // Store the new height
    mImageData[ x + y * mSize.width ] = (cgInt16)nNewHeight;
}

//-----------------------------------------------------------------------------
// Name : loadRaw ()
/// <summary>
/// Load raw data from the file specified and populate our internal buffer.
/// The width, height and format details specified must match those of the raw 
/// file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHeightMap::loadRaw( cgInputStream Stream, const cgSize & Size, RawFormat Format )
{
    cgInt64 nDataLength = 0, nExpectedLength = 0;
    cgInt32 nRowPitch = 0, nCounter = 0;
    
    // Validate requirements
    if ( Size.width <= 0 || Size.height <= 0 )
        return false;

    // Compute the expected size of the RAW file (in bytes)
    switch ( Format )
    {
        case Gray8:
            nExpectedLength = (Size.width * Size.height);
            nRowPitch = Size.width;
            break;

        case Gray16:
            nExpectedLength = (Size.width * Size.height) * 2;
            nRowPitch = Size.width * 2;
            break;
        
        default:
            // No such format
            return false;
    
    } // End format switch

    // Open the stream
    if ( Stream.open( ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to open requested raw image file '%s' for import.\n"), Stream.getName() );
        return false;
    
    } // End if failed

     // Retrieve the size of the file so that we can validate their options
    nDataLength = Stream.getLength();

    // Sizes do not match?
    if ( nExpectedLength != nDataLength )
    {
        Stream.close( );
        cgAppLog::write( cgAppLog::Error, _T("RAW image file size %i did not match the expected %i size based on supplied parameters.\n"), nDataLength, nExpectedLength );
        return false;
    
    } // End if mismatch

    // Allocate image data
    mImageData.clear();
    mImageData.resize( (size_t)nDataLength );
    mSize = Size;

    // Load the raw data from file a row at a time.
    cgByte * pRow = new cgByte[nRowPitch], * pData;
    for ( cgInt32 y = 0; y < mSize.height; ++y )
    {
        // Read a whole row of data.
        Stream.read( pRow, nRowPitch );
        pData = pRow;

        // Extract pixels
        for ( cgInt32 x = 0; x < mSize.width; ++x )
        {
            if ( Format == Gray8 )
            {
                // scale up to the MinCellHeight->MaxCellHeight range.
                mImageData[nCounter++] = (cgInt16)((*pData++) * (MaxCellHeight/255));

            } // End if 8 bit grayscale
            else if ( Format == Gray16 )
            {
                mImageData[nCounter++] = *(cgInt16*)pData;
                pData += 2;

            } // End if 16 bit grayscale

        } // Next Column

    } // Next Row

    // Clean up
    delete pRow;
    Stream.close();
    
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
bool cgHeightMap::loadSquareRaw( cgInputStream Stream, RawFormat Format )
{
    // Retrieve the size of the file so that we can validate
    cgInt64 nDataLength = Stream.getLength();

    // Compute the the dimensions of the RAW file (in pixels) given the specified format.
    cgInt32 nImageSide = 0;
    switch ( Format )
    {
        case Gray8:
            nImageSide = (cgInt32)sqrtf( (cgFloat)nDataLength );
            break;

        case Gray16:
            nImageSide = (cgInt32)sqrtf( (cgFloat)nDataLength / 2 );
            break;
        
        default:
            // No such format
            return false;
    
    } // End format switch

    // Pass through to standard load function/
    return loadRaw( Stream, cgSize(nImageSide, nImageSide), Format );
}

//-----------------------------------------------------------------------------
// Name : normalize ()
/// <summary>
/// normalize the heightmap into the specified range.
/// </summary>
//-----------------------------------------------------------------------------
void cgHeightMap::normalize( cgInt32 nMin, cgInt32 nMax )
{
    cgInt32   x, y;
    cgInt32   nValue;
    cgInt16 * pHeightMap = &mImageData[0];

    // First determine the current min / max range.
    cgInt32 nOrigMin = MaxCellHeight, nOrigMax = MinCellHeight;
    for ( y = 0; y < mSize.height; ++y )
    {
        for ( x = 0; x < mSize.width; ++x, ++pHeightMap )
        {
            if ( *pHeightMap < nOrigMin )
                nOrigMin = *pHeightMap;
            if ( *pHeightMap > nOrigMax )
                nOrigMax = *pHeightMap;

        } // Next Column

    } // Next Row

    // If landscape is completely flat, we can't normalize.
    if ( nOrigMax == nOrigMin )
        return;

    // Compute scale
    cgFloat fScale = (cgFloat)(nMax - nMin) / (cgFloat)(nOrigMax - nOrigMin);

    // normalize values
    pHeightMap = &mImageData[0];
    for ( y = 0; y < mSize.height; ++y )
    {
        for ( x = 0; x < mSize.width; ++x, ++pHeightMap )
        {
            nValue = (cgInt32)((cgFloat)((cgInt32)*pHeightMap - nOrigMin) * fScale + nMin);
            if ( nValue > MaxCellHeight )
                nValue = MaxCellHeight;
            if ( nValue < MinCellHeight )
                nValue = MinCellHeight;
            *pHeightMap = (cgInt16)nValue;

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
void cgHeightMap::scale( cgInt16 nScale )
{
    cgInt32   x, y;
    cgInt32   nValue;
    cgInt16 * pHeightMap = &mImageData[0];

    // Scale values
    for ( y = 0; y < mSize.height; ++y )
    {
        for ( x = 0; x < mSize.width; ++x, ++pHeightMap )
        {
            nValue = ((cgInt32)*pHeightMap) * nScale;
            if ( nValue > MaxCellHeight )
                nValue = MaxCellHeight;
            if ( nValue < MinCellHeight )
                nValue = MinCellHeight;
            *pHeightMap = (cgInt16)nValue;

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
bool cgHeightMap::loadRawSection( cgInputStream Stream, const cgSize & Size, RawFormat Format, const cgRect & rcSection, cgInt16Array & aDataOut )
{
    cgRect      rcIntersect;
    cgInt32     nSectionWidth, nSectionHeight, nPixelStride, nCounter = 0;
    cgInt64     nDataLength = 0, nExpectedLength = 0;

    // Validate requirements
    if ( Size.width <= 0 || Size.height <= 0 )
        return false;

    // Compute the expected size of the RAW file (in bytes)
    switch ( Format )
    {
        case Gray8:
            nExpectedLength = (Size.width * Size.height);
            nPixelStride    = 1;
            break;

        case Gray16:
            nExpectedLength = (Size.width * Size.height) * 2;
            nPixelStride    = 2;
            break;
        
        default:
            // No such format
            return false;
    
    } // End format switch

    // Pre-compute width and height values for efficient testing
    nSectionWidth  = rcSection.width();
    nSectionHeight = rcSection.height();

    // Calculate the intersection of the heightmap total rectangle and the specified
    // rectangle to ensure that we are not attempting to read out of bounds.
    cgRect rcHeightMap( 0, 0, Size.width, Size.height );
    rcIntersect = cgRect::intersect( rcHeightMap, rcSection );
    
    // Ensure that the specified rectangle is valid in position and size.
    if ( rcIntersect != rcSection || nSectionWidth < 1 || nSectionHeight < 1 )
        return false;

    // Retrieve the size of the file so that we can validate their options
    nDataLength = Stream.getLength();

    // Sizes do not match?
    if ( nExpectedLength != nDataLength )
    {
        cgAppLog::write( cgAppLog::Error, _T("RAW image file size %i did not match the expected %i size based on supplied parameters.\n"), nDataLength, nExpectedLength );
        return false;
    
    } // End if mismatch

    // Open the stream
    if ( Stream.open( ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to open requested raw image file '%s' for import.\n"), Stream.getName() );
        return false;
    
    } // End if failed

    // Ensure output buffer is large enough to contain the image data.
    if ( aDataOut.size() < (size_t)(nSectionWidth * nSectionHeight) )
        aDataOut.resize( (nSectionWidth * nSectionHeight) );
    
    // Seek to the correct initial position in the heightmap
    cgInt32 nInputPitch  = Size.width * nPixelStride;
    cgInt32 nOutputPitch = nSectionWidth * nPixelStride;
    Stream.seek( (rcSection.left * nPixelStride) + (rcSection.top * nInputPitch), cgInputStream::Begin );
        
    // Load the raw data from file a row at a time.
    cgByte * pRow = new cgByte[nOutputPitch], * pData;
    for ( cgInt32 y = 0; y < nSectionHeight; ++y )
    {
        // Read a whole row of data.
        Stream.read( pRow, nOutputPitch );
        pData = pRow;

        // Extract pixels
        for ( cgInt32 x = 0; x < nSectionWidth; ++x )
        {
            if ( Format == Gray8 )
            {
                // Scale up to the MinCellHeight->MaxCellHeight range.
                aDataOut[nCounter++] = (cgInt16)((*pData++) * (MaxCellHeight/255));

            } // End if 8 bit grayscale
            else if ( Format == Gray16 )
            {
                aDataOut[nCounter++] = *(cgInt16*)pData;
                pData += 2;

            } // End if 16 bit grayscale

        } // Next Column

        // Seek to the next position in the heightmap
        Stream.seek( nInputPitch - nOutputPitch, cgInputStream::Current );

    } // Next Row
        
    // Success!
    return true;
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