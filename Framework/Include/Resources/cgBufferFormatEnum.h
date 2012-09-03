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
// Name : cgBufferFormatEnum.h                                               //
//                                                                           //
// Desc : A relatively simple class that provides simple enumeration and     //
//        and transport of data relating to buffer formats supported by the  //
//        end user hardware.                                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBUFFERFORMATENUM_H_ )
#define _CGE_CGBUFFERFORMATENUM_H_

//-----------------------------------------------------------------------------
// cgBufferFormatEnum Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBufferFormatEnum (Class)
/// <summary>
/// The purpose of this class is to provide an easy mechanism for the
/// application to query for various supported buffer formats / modes.
/// For example, what is the best compressed alpha texture format, or the
/// best non-compressed opaque format etc.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBufferFormatEnum
{
public:
    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgBufferFormatEnum * createInstance  ( );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBufferFormatEnum( );
             cgBufferFormatEnum( const cgBufferFormatEnum & format );
    virtual ~cgBufferFormatEnum( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                    enumerate               ( cgRenderDriver * driver ) = 0;
    virtual size_t                  estimateBufferSize      ( const cgImageInfo & info ) const = 0;
    virtual cgBufferFormat::Base    getBestFormat           ( cgBufferType::Base type, cgUInt32 searchFlags ) const = 0;

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    initialize              ( );
    bool                    isFormatSupported       ( cgBufferType::Base type, cgBufferFormat::Base format ) const;
    cgUInt32                getFormatCaps           ( cgBufferType::Base type, cgBufferFormat::Base format ) const;
    cgBufferFormat::Base    getBestDepthFormat      ( bool floatingPoint, bool requireStencil ) const;
    cgBufferFormat::Base    getBestOneChannelFormat ( cgBufferType::Base type, bool floatingPoint, bool requireAlpha, bool preferCompressed, bool allowPadding ) const;
    cgBufferFormat::Base    getBestTwoChannelFormat ( cgBufferType::Base type, bool floatingPoint, bool requireAlpha, bool preferCompressed, bool allowPadding ) const;
    cgBufferFormat::Base    getBestFourChannelFormat( cgBufferType::Base type, bool floatingPoint, bool requireAlpha, bool preferCompressed ) const;
    
    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    static bool             formatGroupMatches      ( cgBufferFormat::Base format1, cgBufferFormat::Base format2 );
    static bool             formatHasAlpha          ( cgBufferFormat::Base format );
    static bool             formatIsCompressed      ( cgBufferFormat::Base format );
    static bool             formatIsFloatingPoint   ( cgBufferFormat::Base format );
    static cgUInt8          formatBitsPerPixel      ( cgBufferFormat::Base format );
    static cgString         formatToString          ( cgBufferFormat::Base format );
    static cgUInt32         formatFromString        ( const cgString & format );
    static cgUInt32         formatChannelCount      ( cgBufferFormat::Base format );

    //-------------------------------------------------------------------------
    // Public Virtual Operators
    //-------------------------------------------------------------------------
    virtual const cgBufferFormatEnum & operator= ( const cgBufferFormatEnum & format );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgBufferFormat::Base, cgUInt32, FormatEnumMap)

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    FormatEnumMap   mTexture1D;            // Storage for 1D texture format capabilities.
    FormatEnumMap   mTexture2D;            // Storage for 2D texture format capabilities.
    FormatEnumMap   mTexture3D;            // Storage for 3D texture format capabilities.
    FormatEnumMap   mTextureCube;          // Storage for cube texture format capabilities.
    FormatEnumMap   mRenderTarget;         // Storage for render target texture capabilities.
    FormatEnumMap   mDepthStencil;         // Storage for depth stencil surface/texture capabilities.
};

#endif // !_CGE_CGBUFFERFORMATENUM_H_