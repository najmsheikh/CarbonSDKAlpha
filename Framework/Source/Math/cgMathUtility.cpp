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
// Name : cgMathUtility.cpp                                                  //
//                                                                           //
// Desc : Simple namespace containing various different mathematics utility  //
//        functions.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgMathUtility Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgMathUtility.h>
#include <Math/cgMathTypes.h>
#include <Math/cgRandom.h>

//-----------------------------------------------------------------------------
// Name : distanceToLineSegment ()
// Desc : Calculates the distance from the position specified, and
//        the line segment passed into this function.
// Note : Returns a value that is out of range if the point is outside of the
//        extents of vecStart & vecEnd.
//-----------------------------------------------------------------------------
float cgMathUtility::distanceToLineSegment( const cgVector3& vecPoint, const cgVector3& vecStart, const cgVector3& vecEnd )
{
    cgVector3 c, v;
    float     d, t;

    // Determine t (the length of the vector from ‘vecStart’ to ‘vecPoint’ projected onto segment)
    c = vecPoint - vecStart;
    v = vecEnd - vecStart;   
    d = cgVector3::length( v );

    // Normalize V
    v /= d;

    // Calculate final t value
    t = cgVector3::dot( v, c );

    // Check to see if ‘t’ is beyond the extents of the line segment
    if (t < 0) return FLT_MAX;
    if (t > d) return FLT_MAX;

    // Calculate intersection point on the line
    v.x = vecStart.x + (v.x * t);
    v.y = vecStart.y + (v.y * t);
    v.z = vecStart.z + (v.z * t);

    // Return the length
    return cgVector3::length( vecPoint - v );
}

//-----------------------------------------------------------------------------
// Name : closestPointOnLineSegment ()
// Desc : Calculates the closest point on a line segment from the position 
//        specified.
//-----------------------------------------------------------------------------
cgVector3 cgMathUtility::closestPointOnLineSegment( const cgVector3& vecPoint, const cgVector3& vecStart, const cgVector3& vecEnd )
{
    cgVector3 c, v;
    float     d, t;

    // Determine t (the length of the vector from ‘vecStart’ to ‘vecPoint’ projected onto segment)
    c = vecPoint - vecStart;
    v = vecEnd - vecStart;   
    d = cgVector3::length( v );

    // Normalize V
    v /= d;

    // Calculate final t value
    t = cgVector3::dot( v, c );

    // Check to see if ‘t’ is beyond the extents of the line segment
    if (t < 0)
        return vecStart;
    if (t > d)
        return vecEnd;

    // Calculate intersection point on the line
    v.x = vecStart.x + (v.x * t);
    v.y = vecStart.y + (v.y * t);
    v.z = vecStart.z + (v.z * t);
    return v;
}

//-----------------------------------------------------------------------------
// Name : distanceToPlane ()
// Desc : Compute the distance from the test point to the specified plane
//        along the supplied direction vector.
//-----------------------------------------------------------------------------
cgFloat cgMathUtility::distanceToPlane( const cgVector3 & Point, const cgPlane & Plane, const cgVector3 & Dir )
{
    cgFloat PlaneDistance = cgPlane::dotCoord( Plane, Point );
    cgFloat fDot = cgVector3::dot( (cgVector3&)Plane, Dir );
    if ( fabsf( fDot ) < CGE_EPSILON )
        return PlaneDistance;
    return PlaneDistance / fDot;
}

//-----------------------------------------------------------------------------
//  Name : buildLookAtMatrix ()
/// <summary>
/// Safely builds a look-at (view) matrix.
/// </summary>
//-----------------------------------------------------------------------------
void cgMathUtility::buildLookAtMatrix( cgMatrix *pOut, cgVector3 *pEye, cgVector3 *pAt )
{
	cgVector3 LookVector, UpVector, RightVector, WorldUp( 0.0f, 1.0f, 0.0f );
	cgFloat Distance;

	LookVector = *pAt - *pEye;
	cgVector3::normalize( LookVector, LookVector );

	Distance = cgVector3::dot( WorldUp, LookVector );
	if ( fabs( Distance ) < ( 1.0f - CGE_EPSILON ) )
		UpVector = WorldUp - (LookVector * Distance);
	else
	{
		WorldUp  = cgVector3( 1.0f, 0.0f, 0.0f );
		Distance = cgVector3::dot( WorldUp, LookVector );
		UpVector = WorldUp - (LookVector * Distance);
	}

	cgVector3::normalize( UpVector, UpVector );
	cgVector3::cross( RightVector, UpVector, LookVector );

	pOut->_11 = RightVector.x; pOut->_12 = UpVector.x; pOut->_13 = LookVector.x; pOut->_14 = 0.0f;
	pOut->_21 = RightVector.y; pOut->_22 = UpVector.y; pOut->_23 = LookVector.y; pOut->_24 = 0.0f;
	pOut->_31 = RightVector.z; pOut->_32 = UpVector.z; pOut->_33 = LookVector.z; pOut->_34 = 0.0f;
	pOut->_41 = -cgVector3::dot( *pEye, RightVector );
	pOut->_42 = -cgVector3::dot( *pEye, UpVector );
	pOut->_43 = -cgVector3::dot( *pEye, LookVector );
	pOut->_44 = 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : matrixSwapYZ ()
/// <summary>
/// Swap the Y & Z axis components of the specified matrix.
/// </summary>
//-----------------------------------------------------------------------------
cgMatrix * cgMathUtility::matrixSwapYZ( cgMatrix * pOut, const cgMatrix * pIn )
{
    cgVector3 vX = ((cgVector3&)pIn->_11);
    cgVector3 vY = ((cgVector3&)pIn->_21);
    cgVector3 vZ = ((cgVector3&)pIn->_31);
    cgVector3 vO = ((cgVector3&)pIn->_41);
    (cgVector3&)pOut->_11 = cgVector3( vX.x, vX.z, vX.y );
    (cgVector3&)pOut->_21 = cgVector3( vZ.x, vZ.z, vZ.y );
    (cgVector3&)pOut->_31 = cgVector3( vY.x, vY.z, vY.y );
    (cgVector3&)pOut->_41 = cgVector3( vO.x, vO.z, vO.y );
    return pOut;
}

//-----------------------------------------------------------------------------
//  Name : matrixSwapHandedness ()
/// <summary>
/// Swap the handedness of the specified matrix (LH2RH or RH2LH)
/// </summary>
//-----------------------------------------------------------------------------
cgMatrix * cgMathUtility::matrixSwapHandedness( cgMatrix * pOut, const cgMatrix * pIn )
{
    // Duplicate existing values into output matrix
    if ( pOut != pIn )
        *pOut = *pIn;

    // Negate Z signs.
    pOut->_13 = -pOut->_13;
    pOut->_23 = -pOut->_23;
    pOut->_33 = -pOut->_33;
    pOut->_43 = -pOut->_43;

    // Compute the current Z scale
    cgFloat fZScale = cgVector3::length( (cgVector3&)pOut->_31 );

    // Recompute Z direction
    cgVector3 vZ;
    cgVector3::cross( vZ, (cgVector3&)pOut->_11,  (cgVector3&)pOut->_21 );

    // Re-apply scale as required
    cgVector3::normalize( vZ, vZ );
    vZ *= fZScale;

    // Store and return
    (cgVector3&)pOut->_31 = vZ;
    return pOut;
}

//-----------------------------------------------------------------------------
//  Name : InsertEdge( ) 
/// <summary>
/// Attempts to add an edge to our silhouette list. If it is a duplicate, 
/// we remove both edges.
/// </summary>
//-----------------------------------------------------------------------------
void InsertEdge( cgUInt16 *pSilhouetteEdges, cgUInt32& NumSilhouetteEdges, cgUInt16 v0, cgUInt16 v1 )
{
	// If edge is already in the list, remove it and don't add this one.
	for ( cgUInt32 i = 0; i < NumSilhouetteEdges; i++ )
	{
		// Test both edge directions...
		if ( ( pSilhouetteEdges[ 2 * i + 0 ] == v0 && pSilhouetteEdges[ 2 * i + 1 ] == v1 ) || 
			 ( pSilhouetteEdges[ 2 * i + 0 ] == v1 && pSilhouetteEdges[ 2 * i + 1 ] == v0 ) )
		{
			// If we only have one edge, we just decrement, else...
			if ( NumSilhouetteEdges > 1 )
			{
				pSilhouetteEdges[ 2 * i + 0 ] = pSilhouetteEdges[ 2 * (NumSilhouetteEdges-1) + 0 ];
				pSilhouetteEdges[ 2 * i + 1 ] = pSilhouetteEdges[ 2 * (NumSilhouetteEdges-1) + 1 ];
			}

			// Decrement number of edges
			NumSilhouetteEdges--;

			// Exit
			return;
		}
	}

	// Add new edge to list
	pSilhouetteEdges[ 2 * NumSilhouetteEdges + 0 ] = v0;
	pSilhouetteEdges[ 2 * NumSilhouetteEdges + 1 ] = v1;
	NumSilhouetteEdges++;
}

//-----------------------------------------------------------------------------
//  Name : clipProjectionMatrix()
/// <summary>
/// Adjusts the projection matrix such that the near clip plane is
/// going to essentially correspond to a user-defined plane. This is 
/// a technique called Oblique Frustum Clipping and it is used in our
/// system as a substitute for the less reliable hardware clipping planes.
/// </summary>
//-----------------------------------------------------------------------------
cgMatrix cgMathUtility::clipProjectionMatrix( cgMatrix& matView, cgMatrix& matProj, cgPlane& clip_plane )
{
    cgMatrix matClipProj;
    cgMatrix WorldToProjection;
    cgVector4 projClipPlane;

    // m_clip_plane is plane definition (world space)
    cgPlane::normalize( clip_plane, clip_plane );
    cgVector4 clipPlane( clip_plane.a, clip_plane.b, clip_plane.c, clip_plane.d );

	// Compute world to projection space matrix    
    WorldToProjection = matView * matProj;

	// Transform clip plane into projection space
    cgMatrix::inverse( WorldToProjection, WorldToProjection );
    cgMatrix::transpose( WorldToProjection, WorldToProjection );
    cgVector4::transform( projClipPlane, clipPlane, WorldToProjection );

    if ( projClipPlane.w == 0 )  // or less than a really small value
    {
		return matProj;
    }

    if ( projClipPlane.w > 0 )
    {
        // Flip plane to point away from eye
        cgVector4 clipPlane( -clip_plane.a, -clip_plane.b, -clip_plane.c, -clip_plane.d );

        // Transform clip plane into projection space
        cgVector4::transform( projClipPlane, clipPlane, WorldToProjection );
    }

    // Put projection space clip plane in Z column
	cgMatrix::identity( matClipProj );
    matClipProj(0, 2) = projClipPlane.x;
    matClipProj(1, 2) = projClipPlane.y;
    matClipProj(2, 2) = projClipPlane.z;
    matClipProj(3, 2) = projClipPlane.w;

    // Multiply results into projection matrix
    cgMatrix projClipMatrix = matProj * matClipProj;

	// Return adjusted matrix
	return projClipMatrix;
}


//-----------------------------------------------------------------------------
//  Name : compareVectors()
/// <summary>
/// Determine if the two specified 2D vectors are equal or similar.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgMathUtility::compareVectors( const cgVector2 & v1, const cgVector2 & v2, cgFloat fEpsilon /* = 0.0f */ )
{
    cgFloat fDifference = v1.x - v2.x;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    fDifference = v1.y - v2.y;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : compareVectors()
/// <summary>
/// Determine if the two specified 3D vectors are equal or similar.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgMathUtility::compareVectors( const cgVector3 & v1, const cgVector3 & v2, cgFloat fEpsilon /* = 0.0f */ )
{
    cgFloat fDifference = v1.x - v2.x;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    fDifference = v1.y - v2.y;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    fDifference = v1.z - v2.z;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : compareVectors()
/// <summary>
/// Determine if the two specified 4D vectors are equal or similar.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgMathUtility::compareVectors( const cgVector4 & v1, const cgVector4 & v2, cgFloat fEpsilon /* = 0.0f */ )
{
    cgFloat fDifference = v1.x - v2.x;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    fDifference = v1.y - v2.y;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    fDifference = v1.z - v2.z;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    fDifference = v1.w - v2.w;
    if ( fabsf( fDifference ) > fEpsilon )
        return (fDifference < 0) ? -1 : 1;
    return 0;
}

//-----------------------------------------------------------------------------
// Name : randomColor () (Static)
/// <summary>
/// Helper function to generate useful random colors from a predefined set.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgMathUtility::randomColor( )
{
    // Build list of potential colors
    static const cgColorValue Colors[] = 
    {
        cgColorValue( 1.0f, 0.0f, 0.0f, 1 ),                   // Red
        cgColorValue( 0.0f, 1.0f, 0.0f, 1 ),                   // Green
        cgColorValue( 0.0f, 0.0f, 1.0f, 1 ),                   // Blue
        cgColorValue( 1.0f, 1.0f, 0.0f, 1 ),                   // Yellow
        cgColorValue( 0.5f, 1.0f, 0.0f, 1 ),                   // Chartreuse
        cgColorValue( 0.0f, 0.545098f, 0.545098f, 1 ),         // DarkCyan
        cgColorValue( 0.6f, 0.196078f, 0.8f, 1 ),              // DarkOrchid
        cgColorValue( 0.133333f, 0.545098f, 0.133333f, 1 ),    // ForestGreen
        cgColorValue( 1.0f, 0.627450f, 0.478431f, 1 ),         // LightSalmon
        cgColorValue( 0.690196f, 0.768627f, 0.870588f, 1 ),    // LightSteelBlue
        cgColorValue( 0.5f, 0.0f, 0.0f, 1 ),                   // Maroon
        cgColorValue( 1.0f, 0.647058f, 0.0f, 1 ),              // Orange
        cgColorValue( 0.596078f, 0.984313f, 0.596078f, 1 ),    // PaleGreen
        cgColorValue( 0.250980f, 0.878431f, 0.815686f, 1 ),    // Turquoise
    };

    // Compute a random number to select the color from the list
    static cgRandom::ParkMiller Random( true );
    cgInt nColor = (cgInt)Random.next( 0, (cgDouble)((sizeof(Colors) / sizeof(cgColorValue)) - 1) );
    
    // Return the color
    return Colors[ nColor ];
}

//-----------------------------------------------------------------------------
// Name : solveQuadratic () (Static)
/// <summary>
/// Solve a quadratic equation in the form ax^2+bx+c=0 for x.
/// </summary>
//-----------------------------------------------------------------------------
void cgMathUtility::solveQuadratic( cgDouble a, cgDouble b, cgDouble c, cgInt * pNumSolutions, cgDouble * pSolutions )
{
    if ( !a )
    {
        if ( !b )
        {
            *pNumSolutions = 0;
            return;
        }
        else
        {
            // bx+c=0
            // x=-c/b
            *pNumSolutions = 1;
            pSolutions[0] = -c/b;
            return;
        }
    
    } // End if !quadratic

    // Basic equation solving
    const cgDouble d = sqrt(b*b - 4*a*c);
    cgDouble one_over_two_a = 1.0 / (2.0 * a);

    // Calculate the two possible roots
    *pNumSolutions = 2;
    pSolutions[0] = (-b - d) * one_over_two_a;
    pSolutions[1] = (-b + d) * one_over_two_a;
}

//-----------------------------------------------------------------------------
// Name : solveCubic () (Static)
/// <summary>
/// Solve a cubic equation in the form ax^3+bx^2+cx+d=0 for x.
/// </summary>
//-----------------------------------------------------------------------------
void cgMathUtility::solveCubic( cgDouble a, cgDouble b, cgDouble c, cgDouble d, cgInt * pNumSolutions, cgDouble * pSolutions )
{
    if ( !a )
    {
        // bx^2+cx+d=0
        // x = (sqrt(c^2-4*b*d)-c)/(2*b)
        // or
        // x = -(sqrt(c^2-4*b*d)-c)/(2*b)
        solveQuadratic( b, c, d, pNumSolutions, pSolutions );
        return;

    } // End if !cubic

    const cgDouble a1 = b/a, a2 = c/a, a3 = d/a;
    const cgDouble Q = (a1*a1 - 3.0*a2)/9.0;
    const cgDouble R = (2.0*a1*a1*a1 - 9.0*a1*a2 + 27.0*a3)/54.0;
    const cgDouble R2_Q3 = R*R - Q*Q*Q;
    if (R2_Q3 <= 0)
    {
        *pNumSolutions = 3;
        const cgDouble theta = acos(R/sqrt(Q*Q*Q));
        pSolutions[0] = -2.0*sqrt(Q)*cos(theta/3.0) - a1/3.0;
        pSolutions[1] = -2.0*sqrt(Q)*cos((theta+2.0*CGE_PI)/3.0) - a1/3.0;
        pSolutions[2] = -2.0*sqrt(Q)*cos((theta+4.0*CGE_PI)/3.0) - a1/3.0;
    }
    else
    {
        *pNumSolutions = 1;
        pSolutions[0] = pow(sqrt(R2_Q3)+abs(R), 1/3.0);
        pSolutions[0] += Q/pSolutions[0];
        pSolutions[0] *= (R < 0.0) ? 1 : -1;
        pSolutions[0] -= a1/3.0;
    }
}
