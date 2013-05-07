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
// Name : cgBoundingSphere.cpp                                               //
//                                                                           //
// Desc : Bounding sphere class. Simple but efficient representation of a    //
//        sphere shaped volume with methods for intersection testing, etc.   //
//        Portions by Jack Ritter from "Graphics Gems", Academic Press, 1990 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBoundingSphere Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgBoundingSphere.h>

///////////////////////////////////////////////////////////////////////////////
// Static Member Definitions
///////////////////////////////////////////////////////////////////////////////
cgBoundingSphere cgBoundingSphere::Empty( 0, 0, 0, 0 );
#define BIGNUMBER 100000000.0  		// Hundred million

///////////////////////////////////////////////////////////////////////////////
// cgBoundingSphere Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : fromPoints ()
/// <summary>
/// Calculates a tight fitting bounding sphere from the points supplied.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingSphere& cgBoundingSphere::fromPoints( const cgByte * pointBuffer, cgUInt32 pointCount, cgUInt32 pointStride )
{
    cgVector3 xmin(BIGNUMBER,BIGNUMBER,BIGNUMBER),
              xmax(-BIGNUMBER,-BIGNUMBER,-BIGNUMBER),
              ymin(BIGNUMBER,BIGNUMBER,BIGNUMBER),
              ymax(-BIGNUMBER,-BIGNUMBER,-BIGNUMBER),
              zmin(BIGNUMBER,BIGNUMBER,BIGNUMBER),
              zmax(-BIGNUMBER,-BIGNUMBER,-BIGNUMBER),
              dia1,dia2;

    // FIRST PASS: find 6 minima/maxima points
    const cgByte * points = pointBuffer;
    for ( cgUInt32 i = 0; i < pointCount; ++i, points += pointStride )
    {
        const cgVector3 & p = *(cgVector3*)points;
        if (p.x<xmin.x) xmin = p; // New xminimum point
        if (p.x>xmax.x) xmax = p;
        if (p.y<ymin.y) ymin = p;
        if (p.y>ymax.y) ymax = p;
        if (p.z<zmin.z) zmin = p;
        if (p.z>zmax.z) zmax = p;
    }

    // Set xspan = distance between the 2 points xmin & xmax (squared)
    cgFloat dx = xmax.x - xmin.x;
    cgFloat dy = xmax.y - xmin.y;
    cgFloat dz = xmax.z - xmin.z;
    cgFloat xspan = dx*dx + dy*dy + dz*dz;

    // Same for y & z spans
    dx = ymax.x - ymin.x;
    dy = ymax.y - ymin.y;
    dz = ymax.z - ymin.z;
    cgFloat yspan = dx*dx + dy*dy + dz*dz;

    dx = zmax.x - zmin.x;
    dy = zmax.y - zmin.y;
    dz = zmax.z - zmin.z;
    cgFloat zspan = dx*dx + dy*dy + dz*dz;

    // Set points dia1 & dia2 to the maximally separated pair
    dia1 = xmin;
    dia2 = xmax; // assume xspan biggest
    cgFloat maxspan = xspan;

    if (yspan>maxspan)
    {
        maxspan = yspan;
        dia1 = ymin;
        dia2 = ymax;
    }

    if (zspan>maxspan)
    {
        dia1 = zmin;
        dia2 = zmax;
    }


    // dia1,dia2 is a diameter of initial sphere
    // calc initial center
    position.x = (dia1.x+dia2.x)*0.5f;
    position.y = (dia1.y+dia2.y)*0.5f;
    position.z = (dia1.z+dia2.z)*0.5f;
    // calculate initial radius**2 and radius
    dx = dia2.x-position.x; // x component of radius vector
    dy = dia2.y-position.y; // y component of radius vector
    dz = dia2.z-position.z; // z component of radius vector
    cgFloat radiusSq = dx*dx + dy*dy + dz*dz;
    radius = cgFloat(sqrt(radiusSq));

    // SECOND PASS: increment current sphere
    points = pointBuffer;
    for ( cgUInt32 i = 0; i < pointCount; ++i, points += pointStride )
    {
        const cgVector3 & p = *(cgVector3*)points;
        dx = p.x-position.x;
        dy = p.y-position.y;
        dz = p.z-position.z;
        cgFloat old_to_p_sq = dx*dx + dy*dy + dz*dz;
        if (old_to_p_sq > radiusSq) // do r**2 test first
        {
            // this point is outside of current sphere
            cgFloat old_to_p = cgFloat(sqrt(old_to_p_sq));
            // calc radius of new sphere
            radius = (radius + old_to_p) * 0.5f;
            radiusSq = radius*radius; 	// for next r**2 compare
            cgFloat old_to_new = old_to_p - radius;
            // calc center of new sphere
            cgFloat recip = 1.0f /old_to_p;

            position.x = (radius*position.x + old_to_new*p.x) * recip;
            position.y = (radius*position.y + old_to_new*p.y) * recip;
            position.z = (radius*position.z + old_to_new*p.z) * recip;
        }
    }
    return *this;
}