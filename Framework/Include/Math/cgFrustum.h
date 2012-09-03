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
// Name : cgFrustum.h                                                        //
//                                                                           //
// Desc : Frustum class. Simple but efficient frustum processing.            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGFRUSTUM_H_ )
#define _CGE_CGFRUSTUM_H_

//-----------------------------------------------------------------------------
// cgFrustum Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBoundingBox;
class cgExtrudedBoundingBox;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgFrustum (Class)
/// <summary>
/// Storage for frustum planes / values and wraps up common functionality
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFrustum
{
public:
	//-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgFrustum( );
    cgFrustum( const cgMatrix & view, const cgMatrix & proj );
    cgFrustum( const cgBoundingBox & sourceBounds );

    // ToDo: 6767 - Not a pleasant constructor. Do we still need this?
	cgFrustum( cgUInt8 cubeFaceIndex, const cgVector3 & origin, cgFloat farDistance, cgFloat nearDistance = 1.0f ); 
    
	//-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                    update              ( const cgMatrix & view, const cgMatrix & proj );
    void                    setPlanes           ( const cgPlane newPlanes[] );
    void                    recomputePoints     ( );
    cgVolumeQuery::Class    classifyAABB        ( const cgBoundingBox & bounds, const cgTransform * transform = CG_NULL, cgUInt8 * frustumBits = CG_NULL, cgInt8 * lastOutside = CG_NULL ) const;
    cgVolumeQuery::Class    classifySphere      ( const cgVector3 & center, cgFloat radius ) const;
	cgVolumeQuery::Class    classifyPlane       ( const cgPlane & plane ) const;
    bool                    testPoint           ( const cgVector3 & point) const;
    bool                    testAABB            ( const cgBoundingBox & bounds, const cgTransform * transform = CG_NULL ) const;
    bool                    testExtrudedAABB    ( const cgExtrudedBoundingBox & box ) const;
    bool                    testSphere          ( const cgVector3 & center, cgFloat radius ) const;
    bool                    testSweptSphere     ( const cgVector3 & center, cgFloat radius, const cgVector3 & sweepDirection ) const;
    bool                    testFrustum         ( const cgFrustum & frustum ) const;
    bool                    testLine            ( const cgVector3 & v1, const cgVector3 & v2 ) const;
    cgFrustum             & transform           ( const cgTransform & t );

    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgFrustum        transform           ( cgFrustum frustum, const cgTransform & t );

    //-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    bool operator == ( const cgFrustum & frustum ) const;

    //-------------------------------------------------------------------------
	// Public Variables
	//-------------------------------------------------------------------------
    cgPlane       planes[6];      // The 6 planes of the frustum.
    cgVector3       points[8];      // The 8 corner points of the frustum.
    cgVector3       position;       // The originating position of the frustum.

private:
    //-------------------------------------------------------------------------
	// Private Static Functions
	//-------------------------------------------------------------------------
    static bool   sweptSphereIntersectPlane( cgFloat & t0, cgFloat & t1, const cgPlane & plane, const cgVector3 & center, cgFloat radius, const cgVector3 & sweepDirection );
};

#endif // !_CGE_CGFRUSTUM_H_