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
// Name : cgExtrudedBoundingBox.h                                            //
//                                                                           //
// Desc : Provides support for an "extruded" bounding box that, similar to   //
//        shadow volumes, extrudes an axis aligned bounding box by a         //
//        specified amount away from a single projection point.              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGEXTRUDEDBOUNDINGBOX_H_ )
#define _CGE_CGEXTRUDEDBOUNDINGBOX_H_

//-----------------------------------------------------------------------------
// cgExtrudedBoundingBox Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBoundingBox;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgExtrudedBoundingBox (Class)
/// <summary>
/// Storage for extruded box values and wraps up common functionality
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgExtrudedBoundingBox
{
public:
	//-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgExtrudedBoundingBox( );
    cgExtrudedBoundingBox( const cgBoundingBox & sourceBounds, const cgVector3 & origin, cgFloat range, const cgTransform * transform = CG_NULL );

	//-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void            reset       ( );
    void            extrude     ( const cgBoundingBox & sourceBounds, const cgVector3 & origin, cgFloat range, const cgTransform * transform = CG_NULL );
    bool            getEdge     ( cgUInt32 edge, cgVector3 & v1, cgVector3 & v2 ) const;
    bool            testLine    ( const cgVector3 & v1, const cgVector3 & v2 ) const;
    bool            testSphere  ( const cgVector3 & center, cgFloat radius ) const;
    
    //-------------------------------------------------------------------------
	// Public Variables
	//-------------------------------------------------------------------------
    cgVector3       sourceMin;              // Source bounding box minimum extents
	cgVector3       sourceMax;              // Source bounding box maximum extents
    cgVector3       projectionPoint;        // Origin of the extrusion / projection.
    cgFloat         projectionRange;        // Distance to extrude / project.
    cgPlane         extrudedPlanes[6];      // The 6 final extruded planes.
	cgUInt32        silhouetteEdges[6][2];  // Flags denoting the points used for each extruded (silhouette) edge.
    cgUInt32        edgeCount;              // Number of edges extruded
};

#endif // !_CGE_CGEXTRUDEDBOUNDINGBOX_H_