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
// Name : cgWinCursor.cpp                                                    //
//                                                                           //
// Desc : Custom derived cursor class specific to the Windows(tm) platform.  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWinCursor Module Includes
//-----------------------------------------------------------------------------
#include <System/Platform/cgWinCursor.h>
#include <System/cgImage.h>

///////////////////////////////////////////////////////////////////////////////
// cgWinCursor Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWinCursor () (Constructor)
/// <summary>
/// cgWinCursor Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinCursor::cgWinCursor()
{
    // Clear variables
    mCursor = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgWinCursor () (Constructor)
/// <summary>
/// cgWinCursor Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinCursor::cgWinCursor( const cgPoint & hotSpot, const cgRect & source, const cgImage & image ) : cgCursor( hotSpot, source, image )
{
    // Clear variables
    create( hotSpot, source, image );
}

//-----------------------------------------------------------------------------
//  Name : ~cgWinCursor () (Destructor)
/// <summary>
/// cgWinCursor Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinCursor::~cgWinCursor()
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
void cgWinCursor::dispose( bool bDisposeBase )
{
    // Clean up window data.
    if ( mCursor )
        DeleteObject( mCursor );

    // Clear variables
    mCursor = CG_NULL;

    // Call base class implementation
    if ( bDisposeBase )
        cgCursor::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : getCursorHandle ()
/// <summary>
/// Retrieve the OS specific cursor handle being wrapped by this cursor
/// object instance.
/// </summary>
//-----------------------------------------------------------------------------
HCURSOR cgWinCursor::getCursorHandle( ) const
{
    return mCursor;
}

//-----------------------------------------------------------------------------
//  Name : create () (Virtual)
/// <summary>
/// Create the cursor from the specified image ready for use.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinCursor::create( const cgPoint & hotSpot, const cgRect & source, const cgImage & image )
{
    // Construct the bitmap header for the DIB section.
    BITMAPV5HEADER bi;
    memset( &bi, 0, sizeof(BITMAPV5HEADER) );
    bi.bV5Size          = sizeof(BITMAPV5HEADER);
    bi.bV5Width         = 32;
    bi.bV5Height        = -32;
    bi.bV5Planes        = 1;
    bi.bV5BitCount      = 32;
    bi.bV5Compression   = BI_BITFIELDS;
    bi.bV5RedMask       =  0x00FF0000;
    bi.bV5GreenMask     =  0x0000FF00;
    bi.bV5BlueMask      =  0x000000FF;
    bi.bV5AlphaMask     =  0xFF000000; 

    // Create a DIB section with an alpha channel.
    cgByte * pDIBBits;
    HDC hDC = GetDC(GetDesktopWindow());
    HBITMAP hDIBSection = CreateDIBSection(hDC, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&pDIBBits, NULL, (DWORD)0 );
    ReleaseDC(GetDesktopWindow(),hDC);

    // Clear the DIB section initially.
    memset( pDIBBits, 0, 32 * 32 * 4 );

    // Copy a row at a time from the image buffer.
    cgUInt32 nSourceRowPitch = image.getWidth() * 4;
    cgUInt32 nDIBRowPitch = 32 * 4;
    cgUInt32 nRowBytes = min( source.width(), 32 ) * 4;
    cgUInt32 nRows = min( source.height(), 32 );
    const cgByte * pBits = image.getBuffer() + (source.left * 4) + (source.top * nSourceRowPitch);
    for ( cgUInt32 nRow = 0; nRow < nRows; ++nRow )
        memcpy( pDIBBits + (nRow * nDIBRowPitch), pBits + (nRow * nSourceRowPitch), nRowBytes );

    // Create an empty mask bitmap.
    HBITMAP hMonoMask = CreateBitmap( 32, 32, 1, 1, CG_NULL);

    // Build the 'icon' description.
    ICONINFO ii;
    ii.fIcon = FALSE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmMask = hMonoMask;
    ii.hbmColor = hDIBSection;

    // Create the cursor from the RGBA DIB section.
    mCursor = CreateIconIndirect(&ii);

    // Clean up
    DeleteObject(hDIBSection);          
    DeleteObject(hMonoMask);

    // We're done
    return true;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinCursor::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_WinCursor )
        return true;

    // Unsupported. Try base.
    return cgCursor::queryReferenceType( type );
}