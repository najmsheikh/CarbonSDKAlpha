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
// Name : cgDX9BufferFormatEnum.h                                            //
//                                                                           //
// Desc : A relatively simple class that provides simple enumeration and     //
//        and transport of data relating to buffer formats supported by the  //
//        end user hardware (DX9 class).                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX9BUFFERFORMATENUM_H_ )
#define _CGE_CGDX9BUFFERFORMATENUM_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9BufferFormatEnum Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgBufferFormatEnum.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgDX9RenderDriver;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9BufferFormatEnum (Class)
/// <summary>
/// The purpose of this class is to provide an easy mechanism for the
/// application to query for various supported buffer formats / modes.
/// For example, what is the best compressed alpha texture format, or the
/// best non-compressed opaque format etc.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9BufferFormatEnum : public cgBufferFormatEnum
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgDX9BufferFormatEnum( );
    cgDX9BufferFormatEnum( const cgBufferFormatEnum & format );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgUInt32                 formatToNative      ( cgBufferFormat::Base format );
    static cgBufferFormat::Base     formatFromNative    ( cgUInt32 format );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgBufferFormatEnum)
    //-------------------------------------------------------------------------
    virtual bool                    enumerate           ( cgRenderDriver * driver );
    virtual size_t                  estimateBufferSize  ( const cgImageInfo & description ) const;
    virtual cgBufferFormat::Base    getBestFormat       ( cgBufferType::Base type, cgUInt32 searchFlags ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                            testBilinearSupport ( cgDX9RenderDriver * driver );

private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static void __stdcall           fillLinearTestTexture( D3DXVECTOR4 * outputColor, const D3DXVECTOR2 * texCoord, const D3DXVECTOR2 * texelSize, void * data );
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9BUFFERFORMATENUM_H_