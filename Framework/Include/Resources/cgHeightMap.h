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
// File : cgHeightMap.h                                                      //
//                                                                           //
// Desc : Contains classes and utilities for the creation and management of  //
//        heightmap data, including various functions for heightmap          //
//        modification.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGHEIGHTMAP_H_ )
#define _CGE_CGHEIGHTMAP_H_

//-----------------------------------------------------------------------------
// cgHeightMap Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgFileSystem.h>
  
//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgHeightMap (Class)
/// <summary>
/// HeightMap creation / management class. Contains the actual heightmap
/// "image" data.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgHeightMap
{
public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum RawFormat
    {
        Gray8,
        Gray16
    };

    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgInt32 MaxCellHeight =  0x7FFF;
    static const cgInt32 MinCellHeight = -0x7FFF;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgHeightMap( );
             cgHeightMap( const cgHeightMap & init );
             cgHeightMap( const cgSize & size );
             cgHeightMap( const cgSize & size, cgInt16 initValue );
    virtual ~cgHeightMap( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgInt16             getCell             ( cgInt32 x, cgInt32 y );
    cgFloat             getInterpolatedCell ( cgFloat x, cgFloat y );
    void                setCell             ( cgInt32 x, cgInt32 y, cgInt16 value );
    void                offsetCell          ( cgInt32 x, cgInt32 y, cgInt32 offset );
    bool                loadRaw             ( cgInputStream stream, const cgSize & size, RawFormat format );
    bool                loadSquareRaw       ( cgInputStream stream, RawFormat format );
    void                normalize           ( cgInt32 minimum, cgInt32 maximum );
    void                scale               ( cgInt16 scale );
    void                fill                ( cgInt16 value );
    void                computeHeightRange  ( cgInt32 & minOut, cgInt32 & maxOut ) const;
    void                computeHeightRange  ( cgInt32 & minOut, cgInt32 & maxOut, const cgRect & section ) const;
    
    // Properties
    const cgSize      & getSize             ( ) const;
    const cgInt16Array& getImageData        ( ) const;
    cgInt16Array      & getImageData        ( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static bool         loadRawSection      ( cgInputStream stream, const cgSize & size, RawFormat format, const cgRect & bounds, cgInt16Array & dataOut );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>Size of the image data (in pixels).</summary>
    cgSize          mSize;
    /// <summary>Underlying image data (16 bit grayscale).</summary>
    cgInt16Array    mImageData;

}; // End Class cgHeightMap

#endif // !_CGE_CGHEIGHTMAP_H_