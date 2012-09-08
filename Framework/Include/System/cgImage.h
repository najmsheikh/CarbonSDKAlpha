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
// Name : cgImage.h                                                          //
//                                                                           //
// Desc : Simple image classes, designed to manage and allow manipulation of //
//        image data at the pixel level.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2010 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGIMAGE_H_ )
#define _CGE_CGIMAGE_H_

//-----------------------------------------------------------------------------
// cgImage Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Resources/cgResourceTypes.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgImage (Class)
/// <summary>
/// Simple image classes, designed to manage and allow manipulation of image 
/// data at the pixel level.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgImage : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgImage, "Image" )

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum PenCapStyle
    {
        PenCapFlat    = 0,
        PenCapSquare  = 1,
        PenCapRound   = 2
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
             cgImage( );
	virtual ~cgImage();

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgImage        * createInstance      ( );
    static cgImage        * createInstance      ( cgUInt32 width, cgUInt32 height, cgBufferFormat::Base format, bool hardwareBuffer = false );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    cgUInt32                getWidth            ( ) const;
    cgUInt32                getHeight           ( ) const;
    cgUInt32                getPitch            ( ) const;
    cgBufferFormat::Base    getFormat           ( ) const;
    cgUInt8                 getBitsPerPixel     ( ) const;
    cgUInt8                 getBytesPerPixel    ( ) const;
    bool                    isValid             ( ) const;
    const cgByte          * getBuffer           ( ) const;
    cgByte                * getBuffer           ( );
    cgFloat                 getPenWidth         ( ) const;
    const cgColorValue    & getPenColor         ( ) const;
    PenCapStyle             getPenStartCap      ( ) const;
    PenCapStyle             getPenEndCap        ( ) const;
    const cgColorValue    & getBrushColor       ( ) const;

    //-------------------------------------------------------------------------
	// Public Virtual Methods
	//-------------------------------------------------------------------------
    virtual bool            createImage         ( cgUInt32 width, cgUInt32 height, cgBufferFormat::Base format, bool hardwareBuffer = false );
    virtual bool            isHardwareBuffer    ( ) const = 0;
    virtual void            setPenWidth         ( cgFloat width );
    virtual void            setPenColor         ( const cgColorValue & color );
    virtual void            setPenStartCap      ( PenCapStyle style );
    virtual void            setPenEndCap        ( PenCapStyle style );
    virtual void            setBrushColor       ( const cgColorValue & color );
    virtual bool            beginDraw           ( );
    virtual void            drawLine            ( cgFloat fromX, cgFloat fromY, cgFloat toX, cgFloat toY );
    virtual void            fillCircle          ( cgFloat x, cgFloat y, cgFloat radius );
    virtual void            endDraw             ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Variables
	//-------------------------------------------------------------------------
    cgUInt32                mWidth;                 // Width of the buffer in pixels
    cgUInt32                mHeight;                // Height of the buffer in pixels
    cgUInt32                mPitch;                 // Pitch of the buffer in bytes (the size of a single row)
    cgBufferFormat::Base    mFormat;                // Format of the pixel data. Not all formats are necessarily supported.
    cgByte                * mBuffer;                // Image data buffer.
    
    // Drawing
    bool                    mDrawing;               // Currently in the process of drawing to the buffer (i.e. line, etc.)
    cgFloat                 mPenWidth;              // Width of drawing pen.
    cgColorValue            mPenColor;              // Color of drawing pen.
    PenCapStyle             mPenStartCapStyle;      // Style to use for line / curve start cap
    PenCapStyle             mPenEndCapStyle;        // Style to use for line / curve end cap
    cgColorValue            mBrushColor;            // Color of filling brush.
};

#endif // !_CGE_CGIMAGE_H_