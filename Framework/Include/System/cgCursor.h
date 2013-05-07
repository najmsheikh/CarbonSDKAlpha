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
// Name : cgCursor.h                                                         //
//                                                                           //
// Desc : Base cursor class that provides an interface between the engine    //
//        and any platform specific cursor management logic.                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCURSOR_H_ )
#define _CGE_CGCURSOR_H_

//-----------------------------------------------------------------------------
// cgCursor Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgReference.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {360A06FE-6ADA-4A6f-9D5F-E3AA6CD91E0C}
const cgUID RTID_Cursor = { 0x360a06fe, 0x6ada, 0x4a6f, { 0x9d, 0x5f, 0xe3, 0xaa, 0x6c, 0xd9, 0x1e, 0xc } };

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgImage;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCursor (Class)
/// <summary>
/// Base cursor class that provides an interface between the engine and any 
/// platform specific cursor management logic.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCursor : public cgReference
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
	         cgCursor();
             cgCursor( const cgPoint & hotSpot, const cgRect & source, const cgImage & image );
	virtual ~cgCursor();

    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgCursor       * createInstance          ( );
    static cgCursor       * createInstance          ( const cgPoint & hotSpot, const cgRect & source, const cgImage & image );

    //-------------------------------------------------------------------------
	// Public Virtual Methods
	//-------------------------------------------------------------------------
    virtual bool            create                  ( const cgPoint & hotSpot, const cgRect & source, const cgImage & image ) = 0;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_Cursor; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;
};

#endif // !_CGE_CGCURSOR_H_