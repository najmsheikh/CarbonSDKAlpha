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
// Name : cgBoundingSphere.h                                                 //
//                                                                           //
// Desc : Bounding sphere class. Simple but efficient representation of a    //
//        sphere shaped volume with methods for intersection testing, etc.   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBOUNDINGSPHERE_H_ )
#define _CGE_CGBOUNDINGSPHERE_H_

//-----------------------------------------------------------------------------
// cgBoundingSphere Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBoundingSphere (Class)
/// <summary>
/// Provides storage for common representation of spherical bounding volume, 
/// and wraps up common functionality.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBoundingSphere
{
public:
	//-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgBoundingSphere( ) :
      position(0,0,0), radius(0) {}
    cgBoundingSphere( const cgVector3 & _position, cgFloat _radius ) :
      position( _position ), radius( _radius ) {}
    cgBoundingSphere( cgFloat x, cgFloat y, cgFloat z, cgFloat _radius ) :
      position(x,y,z), radius(_radius) {}

	//-------------------------------------------------------------------------
	// Public Inline Methods
	//-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : containsPoint()
    /// <summary>
    /// Tests to see if the specified point falls within this bounding sphere 
    /// or not. A point precisely on the boundary is also considered to be
    /// contained.
    /// </summary>
    //-------------------------------------------------------------------------
    bool containsPoint( const cgVector3 & point ) const
    {
        return (cgVector3::lengthSq( position - point ) <= (radius*radius));
    }

    //-------------------------------------------------------------------------
    //  Name : containsPoint()
    /// <summary>
    /// Tests to see if the specified point falls within this bounding sphere 
    /// or not, taking into account the provided tolerance. A point precisely 
    /// on the boundary is also considered to be contained.
    /// </summary>
    //-------------------------------------------------------------------------
    bool containsPoint( const cgVector3 & point, cgFloat tolerance ) const
    {
        return (cgVector3::lengthSq( position - point ) <= ((radius + tolerance)*(radius + tolerance)) );
    }

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    cgBoundingSphere  & fromPoints( const cgByte * pointBuffer, cgUInt32 pointCount, cgUInt32 pointStride );

    //-------------------------------------------------------------------------
	// Public Inline Operators
	//-------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //  Name : operator+=()
    /// <summary>
    /// Adjusts the position of the bounding sphere by the specified amount.
    /// </summary>
    //-----------------------------------------------------------------------------
    inline cgBoundingSphere & operator += ( const cgVector3 & shift )
    {
        position += shift;
        return *this;
    }
    
    //-----------------------------------------------------------------------------
    //  Name : operator-=()
    /// <summary>
    /// Adjusts the position of the bounding sphere by the specified amount.
    /// </summary>
    //-----------------------------------------------------------------------------
    inline cgBoundingSphere & operator -= ( const cgVector3 & shift )
    {
        position -= shift;
        return *this;
    }

    //-----------------------------------------------------------------------------
    //  Name : operator!=()
    /// <summary>
    /// Test for inequality between this bounding sphere and another.
    /// </summary>
    //-----------------------------------------------------------------------------
    inline bool operator != ( const cgBoundingSphere & bounds ) const
    {
        return (position != bounds.position || radius != bounds.radius);
    }

    //-----------------------------------------------------------------------------
    //  Name : operator==()
    /// <summary>
    /// Test for equality between this bounding sphere and another.
    /// </summary>
    //-----------------------------------------------------------------------------
    inline bool operator == ( const cgBoundingSphere & bounds ) const
    {
        return (position == bounds.position && radius == bounds.radius);
    }

    //-------------------------------------------------------------------------
	// Public Variables
	//-------------------------------------------------------------------------
    cgVector3       position;
	cgFloat         radius;

    //-------------------------------------------------------------------------
	// Public Static Variables
	//-------------------------------------------------------------------------
    static cgBoundingSphere Empty;
};

#endif // !_CGE_CGBOUNDINGSPHERE_H_