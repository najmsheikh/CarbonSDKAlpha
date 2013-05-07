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
// Name : cgWinCursor.h                                                      //
//                                                                           //
// Desc : Custom derived cursor class specific to the Windows(tm) platform.  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWINCURSOR_H_ )
#define _CGE_CGWINCURSOR_H_

//-----------------------------------------------------------------------------
// cgWinCursor Header Includes
//-----------------------------------------------------------------------------
#include <System/cgCursor.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5172951A-31E1-4415-9C0C-7D682599AFD8}
const cgUID RTID_WinCursor = { 0x5172951a, 0x31e1, 0x4415, { 0x9c, 0xc, 0x7d, 0x68, 0x25, 0x99, 0xaf, 0xd8 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgWinCursor (Class)
/// <summary>
/// Custom derived cursor class specific to the Windows(tm) platform.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWinCursor : public cgCursor
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
	         cgWinCursor();
             cgWinCursor( const cgPoint & hotSpot, const cgRect & source, const cgImage & image );
	virtual ~cgWinCursor();

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    HCURSOR         getCursorHandle ( ) const;

    //-------------------------------------------------------------------------
	// Public Virtual Methods (Overrides cgCursor)
	//-------------------------------------------------------------------------
    virtual bool    create          ( const cgPoint & hotSpot, const cgRect & source, const cgImage & image );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_WinCursor; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Variables;
	//-------------------------------------------------------------------------
    HCURSOR mCursor;    // OS cursor being wrapped.
};

#endif // !_CGE_CGWINCURSOR_H_