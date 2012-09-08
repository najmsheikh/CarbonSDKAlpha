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
// Name : cgBoundingBox.h                                                    //
//                                                                           //
// Desc : Bounding Box class. Simple but efficient bounding box processing.  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBOUNDINGBOX_H_ )
#define _CGE_CGBOUNDINGBOX_H_

//-----------------------------------------------------------------------------
// cgBoundingBox Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBoundingBox (Class)
/// <summary>
/// Storage for box vector values and wraps up common functionality
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBoundingBox
{
public:
	//-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgBoundingBox( );
    cgBoundingBox( const cgVector3 & minimum, const cgVector3 & maximum );
    cgBoundingBox( cgFloat xMin, cgFloat yMin, cgFloat zMin, cgFloat xMax, cgFloat yMax, cgFloat zMax );

	//-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    cgVector3       getDimensions       ( ) const;
    cgVector3       getCenter           ( ) const;
    cgVector3       getExtents          ( ) const;
    cgPlane         getPlane            ( cgVolumePlane::Side side ) const;
    cgBoundingBox & calculateFromPolygon( const cgVector3 vertices[], cgUInt32 vertexCount, cgUInt32 vertexStride, bool reset = true );
    cgBoundingBox & addPoint            ( const cgVector3 & point );
    bool            intersect           ( const cgBoundingBox & bounds ) const;
    bool            intersect           ( const cgBoundingBox & bounds, bool & contained ) const;
    bool            intersect           ( const cgBoundingBox & bounds, cgBoundingBox & intersection ) const;
    bool            intersect           ( const cgBoundingBox & bounds, const cgVector3 & tolerance ) const;
    bool            intersect           ( const cgVector3 & origin, const cgVector3 & velocity, cgFloat & t, bool restrictRange = true ) const;
    bool            intersect           ( const cgVector3 & v0, const cgVector3 & v1, const cgVector3 & v2, const cgBoundingBox & triangleBounds ) const;
    bool            containsPoint       ( const cgVector3 & point ) const;
    bool            containsPoint       ( const cgVector3 & point, const cgVector3 & tolerance ) const;
    cgVector3       closestPoint        ( const cgVector3 & sourcePoint ) const;
    void            validate            ( );
    void            reset               ( );
    cgBoundingBox & transform           ( const cgTransform & t );
    void            inflate             ( cgFloat amount);
    void            inflate             ( const cgVector3 & amount);
    bool            isPopulated         ( ) const;
    bool            isDegenerate        ( ) const;
    inline cgFloat  x                   ( ) const { return min.x; }
    inline cgFloat  y                   ( ) const { return min.y; }
    inline cgFloat  z                   ( ) const { return min.z; }
    inline cgFloat  width               ( ) const { return max.x - min.x; }
    inline cgFloat  height              ( ) const { return max.y - min.y; }
    inline cgFloat  depth               ( ) const { return max.z - min.z; }

    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgBoundingBox    transform   ( cgBoundingBox bounds, const cgTransform & t );

    //-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    cgBoundingBox   operator *          ( cgFloat scale ) const;
    cgBoundingBox & operator +=         ( const cgVector3 & shift );
    cgBoundingBox & operator -=         ( const cgVector3 & shift );
    cgBoundingBox & operator *=         ( const cgTransform & t );
    cgBoundingBox & operator *=         ( cgFloat scale );
    bool            operator !=         ( const cgBoundingBox & bounds ) const;
    bool            operator ==         ( const cgBoundingBox & bounds ) const;

    //-------------------------------------------------------------------------
	// Public Variables
	//-------------------------------------------------------------------------
    cgVector3       min;
	cgVector3       max;

    //-------------------------------------------------------------------------
	// Public Static Variables
	//-------------------------------------------------------------------------
    static cgBoundingBox Empty;
};

#endif // !_CGE_CGBOUNDINGBOX_H_