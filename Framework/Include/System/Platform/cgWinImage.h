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
// Name : cgWinImage.h                                                       //
//                                                                           //
// Desc : Provides image functionality specific to the Windows(tm) platform, //
//        with hardware buffer support supplied by GDI.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2010 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWINIMAGE_H_ )
#define _CGE_CGWINIMAGE_H_

//-----------------------------------------------------------------------------
// cgWinImage Header Includes
//-----------------------------------------------------------------------------
#include <System/cgImage.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
namespace Gdiplus
{
    class Bitmap;
    class Graphics;

}; // End namespace Gdiplus

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgWinImage (Class)
/// <summary>
/// Provides image functionality specific to the Windows(tm) platform, with 
/// hardware buffer support supplied by GDI.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWinImage : public cgImage
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgWinImage, cgImage, "WinImage" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
             cgWinImage( );
             cgWinImage( cgUInt32 width, cgUInt32 height, cgBufferFormat::Base format, bool hardwareBuffer = false );
	virtual ~cgWinImage( );

    //-------------------------------------------------------------------------
	// Public Virtual Methods
	//-------------------------------------------------------------------------
    virtual bool            createImage         ( cgUInt32 width, cgUInt32 height, cgBufferFormat::Base format, bool hardwareBuffer = false );
    virtual bool            isHardwareBuffer    ( ) const;
    virtual bool            beginDraw           ( );
    virtual void            drawLine            ( cgFloat fromX, cgFloat fromY, cgFloat toX, cgFloat toY );
    virtual void            fillCircle          ( cgFloat x, cgFloat y, cgFloat radius );
    virtual void            endDraw             ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    bool                mHardwareBuffer;    // Is a hardware buffer required?
    Gdiplus::Bitmap   * mBitmap;            // Gdiplus bitmap handle representation of our buffer.
    
    // Drawing
    Gdiplus::Graphics * mGraphics;          // Graphics context for the hardware buffer that allows us to draw into it.
};

#endif // !_CGE_CGWINIMAGE_H_