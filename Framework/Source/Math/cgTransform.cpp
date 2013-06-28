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
// Name : cgTransform.cpp                                                    //
//                                                                           //
// Desc : General purpose transformation class designed to maintain each     //
//        component of the transformation separate (translation, rotation,   //
//        scale and shear) whilst providing much of the same functionality   //
//        provided by standard matrices.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgTransform Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgTransform.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define MinAxisLength 1e-5f

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
const cgTransform cgTransform::Identity; // Defaults to identity

///////////////////////////////////////////////////////////////////////////////
// cgTransform Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTransform () (Default Constructor)
/// <summary>
/// cgTransform Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgTransform::cgTransform( ) :
    _m( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 )
{
}

//-----------------------------------------------------------------------------
//  Name : cgTransform () (Constructor)
/// <summary>
/// cgTransform Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgTransform::cgTransform( const cgTransform & t ) :
    _m( t._m )
{
}

//-----------------------------------------------------------------------------
//  Name : cgTransform () (Constructor)
/// <summary>
/// cgTransform Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgTransform::cgTransform( const cgMatrix & m ) :
    _m( m )
{
}

//-----------------------------------------------------------------------------
//  Name : cgTransform () (Constructor)
/// <summary>
/// cgTransform Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgTransform::cgTransform( const cgVector3 & vTranslation )
{
    cgMatrix::identity( _m );
    position() = vTranslation;
}

//-----------------------------------------------------------------------------
//  Name : cgTransform () (Constructor)
/// <summary>
/// cgTransform Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgTransform::cgTransform( const cgQuaternion & qOrientation, const cgVector3 & vTranslation )
{
    cgMatrix::rotationQuaternion( _m, qOrientation );
    position() = vTranslation;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgMatrix&)
/// <summary>
/// Overloaded assignment operator. This operator decomposes the specified
/// matrix. The matrix must be a non perspective matrix where the fourth 
/// column is identity (<0,0,0,1>)!
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::operator=( const cgMatrix & m )
{
    _m = m;
    
    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;
    
    // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator== () (const cgTransform&)
/// <summary>
/// Equality comparison operator
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::operator==( const cgTransform & t ) const
{
    return ((_m == t._m) != 0);
}

//-----------------------------------------------------------------------------
//  Name : operator!= () (const cgTransform&)
/// <summary>
/// Inequality comparison operator
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::operator!=( const cgTransform & t ) const
{
    return ((_m != t._m) != 0);
}

//-----------------------------------------------------------------------------
//  Name : operator const cgMatrix &()
/// <summary>
/// Overloaded cast operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform::operator const cgMatrix&() const
{
    // Return internal matrix
    return _m;
}

//-----------------------------------------------------------------------------
//  Name : operator cgMatrix*()
/// <summary>
/// Overloaded cast operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform::operator cgMatrix*()
{
    // Return address of internal matrix
    return &_m;
}

//-----------------------------------------------------------------------------
//  Name : operator const cgMatrix*()
/// <summary>
/// Overloaded cast operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform::operator const cgMatrix*() const
{
    // Return address of internal matrix
    return &_m;
}

//-----------------------------------------------------------------------------
//  Name : operator * (cgTransform)
/// <summary>
/// Transformation concatenation operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform cgTransform::operator* (const cgTransform & t) const
{
    cgTransform tOut;
    cgTransform::multiply( tOut, *this, t );
    
    // Make sure identity column remains pure.
    tOut._m._14 = tOut._m._24 = tOut._m._34 = 0.0f;
    tOut._m._44 = 1.0f;
    return tOut;
}

//-----------------------------------------------------------------------------
//  Name : operator * (cgTransform)
/// <summary>
/// Transformation scale operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform cgTransform::operator* ( cgFloat f ) const
{
    cgTransform tOut( _m * f );
    
    // Make sure identity column remains pure.
    tOut._m._14 = tOut._m._24 = tOut._m._34 = 0.0f;
    tOut._m._44 = 1.0f;
    return tOut;
}

//-----------------------------------------------------------------------------
//  Name : operator + (cgTransform)
/// <summary>
/// Transformation addition operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform cgTransform::operator+ (const cgTransform & t) const
{
    cgTransform tOut;
    cgTransform::add( tOut, *this, t );
    return tOut;
}

//-----------------------------------------------------------------------------
//  Name : operator *= (cgTransform)
/// <summary>
/// Transformation concatenation operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::operator*= (const cgTransform & t)
{
    multiply( *this, t );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator *= (cgFloat)
/// <summary>
/// Transformation scale operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::operator*= (cgFloat f)
{
    _m *= f;
    
    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator += (cgTransform)
/// <summary>
/// Transformation addition operator.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::operator+= (const cgTransform & t)
{
    add( *this, t );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : localScale()
/// <summary>
/// Retrieve the scale of the transformation along its local axes.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgTransform::localScale( ) const
{
    return cgVector3( cgVector3::length( xAxis() ), cgVector3::length( yAxis() ), cgVector3::length( zAxis() ) );
}

//-----------------------------------------------------------------------------
//  Name : position()
/// <summary>
/// Retrieve the parent relative position of this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::position( )
{
    return (cgVector3&)_m._41;
}

//-----------------------------------------------------------------------------
//  Name : position()
/// <summary>
/// Retrieve the parent relative position of this transform.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgTransform::position( ) const
{
    return (cgVector3&)_m._41;
}

//-----------------------------------------------------------------------------
//  Name : xAxis()
/// <summary>
/// Retrieve the X axis orientation vector for this transform.
/// This axis vector is not necessarily unit length and may be scaled.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgTransform::xAxis( ) const
{
    return (cgVector3&)_m._11;
}

//-----------------------------------------------------------------------------
//  Name : YXAxis()
/// <summary>
/// Retrieve the Y axis orientation vector for this transform.
/// This axis vector is not necessarily unit length and may be scaled.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgTransform::yAxis( ) const
{
    return (cgVector3&)_m._21;
}

//-----------------------------------------------------------------------------
//  Name : ZXAxis()
/// <summary>
/// Retrieve the Z axis orientation vector for this transform.
/// This axis vector is not necessarily unit length and may be scaled.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgTransform::zAxis( ) const
{
    return (cgVector3&)_m._31;
}

//-----------------------------------------------------------------------------
//  Name : xUnitAxis()
/// <summary>
/// Retrieve the unit length X axis orientation vector for this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgTransform::xUnitAxis( ) const
{
    cgVector3 v;
    cgVector3::normalize( v, (cgVector3&)_m._11 );
    return v;
}

//-----------------------------------------------------------------------------
//  Name : yUnitAxis()
/// <summary>
/// Retrieve the unit length Y axis orientation vector for this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgTransform::yUnitAxis( ) const
{
    cgVector3 v;
    cgVector3::normalize( v, (cgVector3&)_m._21 );
    return v;
}

//-----------------------------------------------------------------------------
//  Name : zUnitAxis()
/// <summary>
/// Retrieve the unit length Z axis orientation vector for this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgTransform::zUnitAxis( ) const
{
    cgVector3 v;
    cgVector3::normalize( v, (cgVector3&)_m._31 );
    return v;
}

//-----------------------------------------------------------------------------
//  Name : orientation()
/// <summary>
/// Retrieve the orientation quaternion for this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgQuaternion cgTransform::orientation( ) const
{
    cgVector3 vScale, vShear, vTranslation;
    cgQuaternion qRotation;
    if ( decompose( vScale, vShear, qRotation, vTranslation ) )
        return qRotation;
    else
        return cgQuaternion(0,0,0,1);
}

//-----------------------------------------------------------------------------
//  Name : zero()
/// <summary>
/// Reset the transform back to a completely empty state.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::zero( )
{
    memset( &_m, 0, sizeof(cgMatrix) );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : identity()
/// <summary>
/// Reset the transform back to its identity state.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::identity( )
{
    cgMatrix::identity( _m );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : identity() (Static)
/// <summary>
/// Generate an identity transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::identity( cgTransform & tOut )
{
    return tOut.identity();
}

//-----------------------------------------------------------------------------
//  Name : decompose()
/// <summary>
/// Decompose a transform into its component parts: scale, rotation and 
/// translation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::decompose( cgVector3 & vScale, cgQuaternion & qRotation, cgVector3 & vTranslation ) const
{
    cgVector3 vShear;
    return decompose( vScale, vShear, qRotation, vTranslation );
}

//-----------------------------------------------------------------------------
//  Name : decompose()
/// <summary>
/// Decompose a transform into its component parts: rotation and translation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::decompose( cgQuaternion & qRotation, cgVector3 & vTranslation ) const
{
    cgVector3 vScale, vShear;
    return decompose( vScale, vShear, qRotation, vTranslation );
}

//-----------------------------------------------------------------------------
//  Name : decompose() (Static)
/// <summary>
/// Decompose a transform into its component parts: scale, rotation and 
/// translation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::decompose( cgVector3 & vScale, cgQuaternion & qRotation, cgVector3 & vTranslation, const cgTransform & t )
{
    cgVector3 vShear;
    return t.decompose( vScale, vShear, qRotation, vTranslation );
}

//-----------------------------------------------------------------------------
//  Name : decompose() (Static)
/// <summary>
/// Decompose a transform into its component parts: rotation and translation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::decompose( cgQuaternion & qRotation, cgVector3 & vTranslation, const cgTransform & t )
{
    cgVector3 vScale, vShear;
    return t.decompose( vScale, vShear, qRotation, vTranslation );
}

//-----------------------------------------------------------------------------
//  Name : decompose()
/// <summary>
/// Decompose a transform into its component parts: local scale, local shear, 
/// rotation and translation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::decompose( cgVector3 & vScale, cgVector3 & vShear, cgQuaternion & qRotation, cgVector3 & vTranslation ) const
{
    register int i, j, k;

    // Normalize the matrix.
    cgMatrix mtx = _m;
 	if ( !mtx.m[3][3] )
 		return false;
 	for ( i = 0; i < 4; ++i )
 		for ( j = 0; j < 4; ++j )
 			mtx.m[i][j] /= mtx.m[3][3];

    // Clear any perspective component.
    cgMatrix mtxP = mtx;
 	for ( i = 0; i < 3; ++i )
 		mtx.m[i][3] = 0;
 	mtx.m[3][3] = 1;

    // If this matrix has a determinant of 0, then it
    // has a singularity that cannot be resolved.
    if ( !cgMatrix::determinant( mtx ) )
 		return 0;
    
    // Extract (and remove) the translation component first.
    for ( i = 0; i < 3; ++i )
    {
        vTranslation[i] = mtx.m[3][i];
        mtx.m[3][i] = 0;

    } // Next component

    // Now compute scale and shear from the upper 3x3 portion of the matrix. 
    // Extract the rows to make things easier to calculate.
    cgVector3 Row[3];
    for ( i = 0; i < 3; ++i )
    {
        Row[i].x = mtx.m[i][0];
        Row[i].y = mtx.m[i][1];
        Row[i].z = mtx.m[i][2];
    
    } // Next Row

    // Compute scale along the X axis and normalize.
    vScale.x = cgVector3::length( Row[0] );
    if ( vScale.x != 0.0 )
        Row[0] /= vScale.x;

    // Now compute the XY shear factor (the shear between the X and Y rows)
    // and then subsequently orthogonalize the Y axis to the X axis using 
    // gram-schmidt orthogonolization.
    vShear.x = cgVector3::dot( Row[0], Row[1] );
    Row[1].x += Row[0].x * -vShear.x;
    Row[1].y += Row[0].y * -vShear.x;
    Row[1].z += Row[0].z * -vShear.x;

    // Compute scale along the Y axis and normalize.
    vScale.y = cgVector3::length( Row[1] );
    if ( vScale.y != 0.0 )
    {
        Row[1] /= vScale.y;
        vShear.x /= vScale.y;
    
    } // End if !0

    // Compute the XZ and YZ shear factors and orthogonalize the
    // Z axis to both using a process similar to the above.
    vShear.y = cgVector3::dot( Row[0], Row[2] );
    Row[2].x += Row[0].x * -vShear.y;
    Row[2].y += Row[0].y * -vShear.y;
    Row[2].z += Row[0].z * -vShear.y;
    vShear.z = cgVector3::dot( Row[1], Row[2] );
    Row[2].x += Row[1].x * -vShear.z;
    Row[2].y += Row[1].y * -vShear.z;
    Row[2].z += Row[1].z * -vShear.z;

    // Compute scale along the Z axis and normalize.
    vScale.z = cgVector3::length( Row[2] );
    if ( vScale.z != 0.0 )
    {
        Row[2] /= vScale.z;
        vShear.y /= vScale.z;
        vShear.z /= vScale.z;
    
    } // End if !0

    // Now that each of the axes are orthonormal, we need to check the "parity" 
    // of the coordinate system to see if it is inverted. If it is, then we need
    // to invert the axes and the scaling factors.
    cgVector3 vCross;
    if ( cgVector3::dot( Row[0], *cgVector3::cross( vCross, Row[1], Row[2]) ) < 0 )
    {
        for ( i = 0; i < 3; ++i )
        {
            vScale[i] *= -1;
            Row[i].x *= -1;
            Row[i].y *= -1;
            Row[i].z *= -1;
        
        } // Next Component
    
    } // End if flipped

    // Finally, compute the rotation quaternion from the three axis vectors.
    cgFloat fRoot, fTrace = Row[0].x + Row[1].y + Row[2].z;
    if ( fTrace > 0.0f )
    {
        fRoot       = sqrtf( fTrace + 1.0f );
        qRotation.w  = 0.5f * fRoot;
        fRoot       = 0.5f / fRoot;
        qRotation.x  = fRoot * (Row[1].z - Row[2].y);
        qRotation.y  = fRoot * (Row[2].x - Row[0].z);
        qRotation.z  = fRoot * (Row[0].y - Row[1].x);
    
    } // End if > 0
    else
    {
        static int Next[3] = { 1, 2, 0 };
        i = 0;
        if (Row[1].y > Row[0].x) i = 1;
        if (Row[2].z > Row[i][i]) i = 2;
        j = Next[i];
        k = Next[j];

        fRoot = sqrtf(Row[i][i] - Row[j][j] - Row[k][k] + 1.0f);

        qRotation[i] = 0.5f * fRoot;
        fRoot        = 0.5f / fRoot;
        qRotation[j] = fRoot * (Row[i][j] + Row[j][i]);
        qRotation[k] = fRoot * (Row[i][k] + Row[k][i]);
        qRotation.w  = fRoot * (Row[j][k] - Row[k][j]);

    } // End if <= 0

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : decompose() (Static)
/// <summary>
/// Decompose a transform into its component parts: local scale, local shear, 
/// rotation and translation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::decompose( cgVector3 & vScale, cgVector3 & vShear, cgQuaternion & qRotation, cgVector3 & vTranslation, const cgTransform & t )
{
    return t.decompose( vScale, vShear, qRotation, vTranslation );
}

//-----------------------------------------------------------------------------
//  Name : compose()
/// <summary>
/// Compose a new transform from its component parts: local scale, local shear, 
/// rotation and translation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::compose( const cgVector3 & vScale, const cgVector3 & vShear, const cgQuaternion & qRotation, const cgVector3 & vTranslation )
{
    cgFloat a1, a2;
    cgMatrix m, mR;

    // Convert rotation quaternion to matrix form.
    cgMatrix::rotationQuaternion( mR, qRotation );

    /*// Shear matrix definition
    _11 = 1; _12 = 0; _13 = 0; _14 = 0;
    _21 = XYShear; _22 = 1; _23 = 0; _24 = 0;
    _31 = XZShear; _32 = YZShear; _33 = 1; _34 = 0;
    _41 = 0; _42 = 0; _43 = 0; _44 = 1;*/

    // (localScale * LocalShear) * rotation
    // 1st row * 1st column
    _m._11 = vScale.x * mR._11;
    _m._12 = vScale.x * mR._12;
    _m._13 = vScale.x * mR._13;

    // 2nd row * 2nd column
    a1 = vScale.y * vShear.x;
    _m._21 = a1 * mR._11 + vScale.y * mR._21;
    _m._22 = a1 * mR._12 + vScale.y * mR._22;
    _m._23 = a1 * mR._13 + vScale.y * mR._23;

    // 3rd row * 3rd column
    a1 = vScale.z * vShear.y; a2 = vScale.z * vShear.z;
    _m._31 = a1 * mR._11 + a2 * mR._21 + vScale.z * mR._31;
    _m._32 = a1 * mR._12 + a2 * mR._22 + vScale.z * mR._32;
    _m._33 = a1 * mR._13 + a2 * mR._23 + vScale.z * mR._33;

    // translation
    _m._41 = vTranslation.x; _m._42 = vTranslation.y; _m._43 = vTranslation.z;

    // identity fourth column.
    _m._14 = 0.0f; _m._24 = 0.0f; _m._34 = 0.0f; _m._44 = 1.0f;

    // Return reference to self in order to allow consecutive operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : compose() (Static)
/// <summary>
/// Compose a new transform from its component parts: local scale, local shear, 
/// rotation and translation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::compose( cgTransform & tOut, const cgVector3 & vScale, const cgVector3 & vShear, const cgQuaternion & qRotation, const cgVector3 & vTranslation )
{
    return tOut.compose( vScale, vShear, qRotation, vTranslation );
}

//-----------------------------------------------------------------------------
//  Name : compose()
/// <summary>
/// Compose a new transform from its component parts: rotation and translation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::compose( const cgQuaternion & qRotation, const cgVector3 & vTranslation )
{
    // Convert rotation quaternion to matrix form.
    cgMatrix::rotationQuaternion( _m, qRotation );
    
    // translation
    _m._41 = vTranslation.x; _m._42 = vTranslation.y; _m._43 = vTranslation.z;

    // Return reference to self in order to allow consecutive operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : compose() (Static)
/// <summary>
/// Compose a new transform from its component parts: rotation and translation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::compose( cgTransform & tOut, const cgQuaternion & qRotation, const cgVector3 & vTranslation )
{
    return tOut.compose( qRotation, vTranslation );
}

//-----------------------------------------------------------------------------
//  Name : add()
/// <summary>
/// Determines the sum of this transform object and that specified.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::add( cgTransform & tOut, const cgTransform & t ) const
{
    tOut._m = _m + t._m;
    
    // Make sure identity column remains pure.
    tOut._m._14 = tOut._m._24 = tOut._m._34 = 0.0f;
    tOut._m._44 = 1.0f;
    return tOut;
}

//-----------------------------------------------------------------------------
//  Name : add() (Static)
/// <summary>
/// Determines the sum of two transforms.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::add( cgTransform & tOut, const cgTransform & t1, const cgTransform & t2 )
{
    tOut._m = t1._m + t2._m;

    // Make sure identity column remains pure.
    tOut._m._14 = tOut._m._24 = tOut._m._34 = 0.0f;
    tOut._m._44 = 1.0f;

    return tOut;
}

//-----------------------------------------------------------------------------
//  Name : multiply()
/// <summary>
/// Determines the product of this transform object and that specified.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::multiply( cgTransform & tOut, const cgTransform & t ) const
{
    tOut._m = _m * t._m;
    
    // Make sure identity column remains pure.
    tOut._m._14 = tOut._m._24 = tOut._m._34 = 0.0f;
    tOut._m._44 = 1.0f;
    return tOut;
}

//-----------------------------------------------------------------------------
//  Name : multiply() (Static)
/// <summary>
/// Determines the product two transforms.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::multiply( cgTransform & tOut, const cgTransform & t1, const cgTransform & t2 )
{
    return t1.multiply( tOut, t2 );
}

//-----------------------------------------------------------------------------
//  Name : inverse()
/// <summary>
/// Compute the inverse of this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::inverse( cgTransform & tOut ) const
{
    cgMatrix::inverse( tOut._m, _m );
    
    // Make sure identity column remains pure.
    tOut._m._14 = tOut._m._24 = tOut._m._34 = 0.0f;
    tOut._m._44 = 1.0f;
    return tOut;
}

//-----------------------------------------------------------------------------
//  Name : inverse() (Static)
/// <summary>
/// Compute the inverse of the specified transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::inverse( cgTransform & tOut, const cgTransform & t )
{
    return t.inverse( tOut );
}

//-----------------------------------------------------------------------------
//  Name : invert()
/// <summary>
/// Invert this transform object.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::invert( )
{
    cgMatrix::inverse( _m, _m );
    
    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : transformCoord()
/// <summary>
/// Transforms a 3D vector by the values in this transform object, projecting 
/// the result back into w = 1.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::transformCoord( cgVector3 & vOut, const cgVector3 & v ) const
{
    cgVector3::transformCoord( vOut, v, _m );
    return vOut;
}

//-----------------------------------------------------------------------------
//  Name : transformCoord() (Static)
/// <summary>
/// Transforms a 3D vector by a given transform object, projecting the result 
/// back into w = 1.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::transformCoord( cgVector3 & vOut, const cgVector3 & v, const cgTransform & t )
{
    return t.transformCoord( vOut, v );
}

//-----------------------------------------------------------------------------
//  Name : inverseTransformCoord()
/// <summary>
/// Transforms a 3D vector by the inverse of the values in this transform 
/// object, projecting the result back into w = 1.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::inverseTransformCoord( cgVector3 & vOut, const cgVector3 & v ) const
{
    cgMatrix im;
    cgMatrix::inverse( im, _m );
    cgVector3::transformCoord( vOut, v, im );
    return vOut;
}

//-----------------------------------------------------------------------------
//  Name : inverseTransformCoord() (Static)
/// <summary>
/// Transforms a 3D vector by the inverse of a given transform object,
/// projecting the result back into w = 1.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::inverseTransformCoord( cgVector3 & vOut, const cgVector3 & v, const cgTransform & t )
{
    return t.inverseTransformCoord( vOut, v );
}
    
//-----------------------------------------------------------------------------
//  Name : transformNormal()
/// <summary>
/// Transforms the 3D vector normal by the values in this transform object.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::transformNormal( cgVector3 & vOut, const cgVector3 & v ) const
{
    cgVector3::transformNormal( vOut, v, _m );
    return vOut;
}

//-----------------------------------------------------------------------------
//  Name : transformNormal() (Static)
/// <summary>
/// Transforms the 3D vector normal by the given transform object.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::transformNormal( cgVector3 & vOut, const cgVector3 & v, const cgTransform & t )
{
    return t.transformNormal( vOut, v );
}

//-----------------------------------------------------------------------------
//  Name : inverseTransformNormal()
/// <summary>
/// Transforms the 3D vector normal by inverse of the values in this transform
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::inverseTransformNormal( cgVector3 & vOut, const cgVector3 & v ) const
{
    cgMatrix im;
    cgMatrix::inverse( im, _m );
    cgVector3::transformNormal( vOut, v, im );
    return vOut;
}

//-----------------------------------------------------------------------------
//  Name : inverseTransformNormal() (Static)
/// <summary>
/// Transforms the 3D vector normal by the inverse of the given transform
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 & cgTransform::inverseTransformNormal( cgVector3 & vOut, const cgVector3 & v, const cgTransform & t )
{
    return t.inverseTransformNormal( vOut, v );
}

//-----------------------------------------------------------------------------
//  Name : rotateLocal ()
/// <summary>
/// rotate the transform around its own local axes.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotateLocal( cgFloat x, cgFloat y, cgFloat z )
{
    // No - op?
    if ( !x && !y && !z )
        return *this;

    // In order to maintain local-space relative scale, perform an affine 
    // rotation in the space of the parent, rotating about the parent space 
    // relative axis vectors. First we need to set up the rotation such that 
    // it will be  relative to the transform's local origin.
    const cgVector3 vPos = position();
    _m._41 = 0.0f; _m._42 = 0.0f; _m._43 = 0.0f;
    
    // Rotate about each axis individually. First yaw.
    cgMatrix  mtxRot;
    if ( y )
    {
        cgMatrix::rotationAxis( mtxRot, yUnitAxis(), y );
        _m = _m * mtxRot;
    
    } // End if yaw

    // Next, pitch.
    if ( x )
    {
        cgMatrix::rotationAxis( mtxRot, xUnitAxis(), x );
        _m = _m * mtxRot;
    
    } // End if pitch

    // Finally, roll.
    if ( z )
    {
        cgMatrix::rotationAxis( mtxRot, zUnitAxis(), z );
        _m  = _m * mtxRot;
    
    } // End if roll

    // Restore world space position.
    position() = vPos;

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow consecutive operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : rotate ()
/// <summary>
/// Rotate the transform around its parent axes and origin.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotate( cgFloat x, cgFloat y, cgFloat z )
{
    // No - op?
    if ( !x && !y && !z )
        return *this;
    
    // Store old position and clear
    const cgVector3 vPos = position();
    _m._41 = 0.0f; _m._42 = 0.0f; _m._43 = 0.0f;
    
    // Rotate the transform
    cgMatrix mtxRot;
    cgMatrix::rotationYawPitchRoll( mtxRot, y, x, z );
    _m = _m * mtxRot;

    // Restore the position
    position() = vPos;

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : rotate ()
/// <summary>
/// Rotate the transform around its parent axes and the specified point.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotate( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & RotationCenter )
{
    // No - op?
    if ( !x && !y && !z )
        return *this;

    // Subtract rotation center.
    position() -= RotationCenter;
    
    // Rotate the transform
    cgMatrix mtxRot;
    cgMatrix::rotationYawPitchRoll( mtxRot, y, x, z );
    _m = _m * mtxRot;

    // Restore the position
    position() += RotationCenter;

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : rotateAxis ()
/// <summary>
/// Rotate the transform around the parent origin and the specified parent 
/// relative axis.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotateAxis( cgFloat a, const cgVector3 & v )
{
    // No - op?
    if ( !a )
        return *this;
    
    // Store old position and clear
    const cgVector3 vPos = position();
    _m._41 = 0.0f; _m._42 = 0.0f; _m._43 = 0.0f;
    
    // Rotate the transform
    cgMatrix mtxRot;
    cgMatrix::rotationAxis( mtxRot, v, a );
    _m = _m * mtxRot;

    // Restore the position
    position() = vPos;

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : rotateAxis ()
/// <summary>
/// Rotate the transform around the specified parent relative axis and origin.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotateAxis( cgFloat a, const cgVector3 & v, const cgVector3 & RotationCenter )
{
    // No - op?
    if ( !a )
        return *this;

    // Subtract rotation center.
    position() -= RotationCenter;
    
    // Rotate the transform
    cgMatrix mtxRot;
    cgMatrix::rotationAxis( mtxRot, v, a );
    _m = _m * mtxRot;

    // Restore the position
    position() += RotationCenter;

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : scaleLocal ()
/// <summary>
/// Scale the transform along its own local axes.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::scaleLocal( cgFloat x, cgFloat y, cgFloat z )
{
    // No - op?
    if ( x == 1.0f && y == 1.0f && z == 1.0f )
        return *this;

    // Retrieve the current local scale (length of axes).
    cgVector3 vCurrentLength = localScale();

    // Compute the final length of the axes after
    // scaling has been applied.
    cgVector3 vFinalLength( vCurrentLength.x * fabsf(x), vCurrentLength.y * fabsf(y), vCurrentLength.z * fabsf(z) );

    // If any fall below the minimum scale requirements, clamp the 
    // scaling operation.
    if ( (vFinalLength.x <= MinAxisLength) && vCurrentLength.x )
        x = (x < 0) ? MinAxisLength / vCurrentLength.x : MinAxisLength / -vCurrentLength.x;
    if ( vFinalLength.y <= MinAxisLength && vCurrentLength.y )
        y = (y < 0) ? MinAxisLength / vCurrentLength.y : MinAxisLength / -vCurrentLength.y;
    if ( (vFinalLength.z <= MinAxisLength) && vCurrentLength.z )
        z = (z < 0) ? MinAxisLength / vCurrentLength.z : MinAxisLength / -vCurrentLength.z;

    // Apply scale
    cgMatrix mtxScale;
    cgMatrix::scaling( mtxScale, x, y, z );
    _m = mtxScale * _m;

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : scaleLocal ()
/// <summary>
/// Scale the transform along its own local axes.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::scaleLocal( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & localCenter )
{
    // No - op?
    if ( x == 1.0f && y == 1.0f && z == 1.0f )
        return *this;

    // Retrieve the current local scale (length of axes).
    cgVector3 vCurrentLength = localScale();

    // Compute the final length of the axes after
    // scaling has been applied.
    cgVector3 vFinalLength( vCurrentLength.x * fabsf(x), vCurrentLength.y * fabsf(y), vCurrentLength.z * fabsf(z) );

    // If any fall below the minimum scale requirements, clamp the 
    // scaling operation.
    if ( (vFinalLength.x <= MinAxisLength) && vCurrentLength.x )
        x = (x < 0) ? MinAxisLength / vCurrentLength.x : MinAxisLength / -vCurrentLength.x;
    if ( vFinalLength.y <= MinAxisLength && vCurrentLength.y )
        y = (y < 0) ? MinAxisLength / vCurrentLength.y : MinAxisLength / -vCurrentLength.y;
    if ( (vFinalLength.z <= MinAxisLength) && vCurrentLength.z )
        z = (z < 0) ? MinAxisLength / vCurrentLength.z : MinAxisLength / -vCurrentLength.z;

    cgVector3 vOriginalCenter;
    transformCoord( vOriginalCenter, localCenter );

    // Apply scale
    cgMatrix mtxScale;
    cgMatrix::scaling( mtxScale, x, y, z );
    _m = mtxScale * _m;

    // Compute new scaled center position.
    cgVector3 vScaledCenter;
    transformCoord( vScaledCenter, localCenter );
    
    // Reposition the object
    position() = position() - (vScaledCenter - vOriginalCenter);

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : scale ()
/// <summary>
/// Scale the transform along its parent axes and origin.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::scale( cgFloat x, cgFloat y, cgFloat z )
{
    // No - op?
    if ( x == 1.0f && y == 1.0f && z == 1.0f )
        return *this;

     // Store old position and clear
    const cgVector3 vPos = position();
    _m._41 = 0.0f; _m._42 = 0.0f; _m._43 = 0.0f;
    
    // Scale the transform
    cgMatrix mtxScale;
    cgMatrix::scaling( mtxScale, x, y, z );
    _m = _m * mtxScale;

    // Restore the position
    position() = vPos;

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : scale ()
/// <summary>
/// Scale the transform along its parent axes and the specified point.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::scale( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & ScalingCenter )
{
    // No - op?
    if ( x == 1.0f && y == 1.0f && z == 1.0f )
        return *this;

     // Subtract scaling center.
    position() -= ScalingCenter;
    
    // Scale the transform
    cgMatrix mtxScale;
    cgMatrix::scaling( mtxScale, x, y, z );
    _m = _m * mtxScale;

    // Restore the position
    position() += ScalingCenter;

    // Make sure identity column remains pure.
    _m._14 = _m._24 = _m._34 = 0.0f;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : translate ()
/// <summary>
/// Apply a translation to this transform in its parent space.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::translate( cgFloat x, cgFloat y, cgFloat z )
{
    _m._41 += x;
    _m._42 += y;
    _m._43 += z;
    
    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : translate ()
/// <summary>
/// Apply a translation to this transform in its parent space.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::translate( const cgVector3 & v )
{
    position() += v;
    
    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : translateLocal ()
/// <summary>
/// Apply a translation to this transform along its own local axes.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::translateLocal( cgFloat x, cgFloat y, cgFloat z )
{
    position() += xAxis() * x;
    position() += yAxis() * y;
    position() += zAxis() * z;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : translateLocal ()
/// <summary>
/// Apply a translation to this transform along its own local axes.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::translateLocal( const cgVector3 & v )
{
    position() += xAxis() * v.x;
    position() += yAxis() * v.y;
    position() += zAxis() * v.z;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : setPosition ()
/// <summary>
/// Set the position of the origin of this transform with respect its parent.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::setPosition( cgFloat x, cgFloat y, cgFloat z )
{
    _m._41 = x;
    _m._42 = y;
    _m._43 = z;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : setPosition ()
/// <summary>
/// Set the position of the origin of this transform with respect its parent.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::setPosition( const cgVector3 & v )
{
    position() = v;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : setLocalScale ()
/// <summary>
/// Set the scale of the local axes defined by this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::setLocalScale( cgFloat x, cgFloat y, cgFloat z )
{
    // Clamp the various scales to the minimum length.
    x = max( MinAxisLength, x );
    y = max( MinAxisLength, y );
    z = max( MinAxisLength, z );

    // Generate the new axis vectors;
    (cgVector3&)_m._11 = xUnitAxis() * x;
    (cgVector3&)_m._21 = yUnitAxis() * y;
    (cgVector3&)_m._31 = zUnitAxis() * z;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : setLocalScale ()
/// <summary>
/// Set the scale of the local axes defined by this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::setLocalScale( const cgVector3 & v )
{
    return setLocalScale( v.x, v.y, v.z );
}

//-----------------------------------------------------------------------------
//  Name : setLocalShear ()
/// <summary>
/// Set the shear of the local axes defined by this transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::setLocalShear( cgFloat xy, cgFloat xz, cgFloat yz )
{
    cgVector3 vScale, vShear, vTranslation;
    cgQuaternion qRotation;

    // Decompose the matrix into its component parts
    decompose( vScale, vShear, qRotation, vTranslation );

    // Replace the shear
    vShear.x = xy;
    vShear.y = xz;
    vShear.z = yz;

    // Recompose a new matrix
    compose( vScale, vShear, qRotation, vTranslation );
    
    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : setOrientation ()
/// <summary>
/// Update the transform's orientation using the quaternion provided. This will
/// also have the effect of removing any shear applied to the transform, but
/// local axis scales will be maintained.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::setOrientation( const cgQuaternion & q )
{
    cgMatrix m;
    cgMatrix::rotationQuaternion( m, q );
    
    // Maintain relative scale.
    cgVector3 vLength = localScale();
    
    // Scale to original length
    (cgVector3&)m._11 *= vLength.x;
    (cgVector3&)m._21 *= vLength.y;
    (cgVector3&)m._31 *= vLength.z;
    
    // Duplicate position.
    (cgVector3&)m._41 = position();

    // Store new matrix.
    _m = m;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : setOrientation ()
/// <summary>
/// Update the transform's orientation using the axis vectors provided. This
/// will also have the effect of replacing any shear applied to the transform
/// with that of the axes supplied, but local axis scales will be maintained.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::setOrientation( const cgVector3 & vX, const cgVector3 & vY, const cgVector3 & vZ )
{
    // Get current scale so that it can be preserved.
    cgVector3 vScale = localScale();

    // Set the new axis vectors (normalized)
    cgVector3::normalize( (cgVector3&)_m._11, vX );
    cgVector3::normalize( (cgVector3&)_m._21, vY );
    cgVector3::normalize( (cgVector3&)_m._31, vZ );

    // Scale back to original length
    (cgVector3&)_m._11 *= vScale.x;
    (cgVector3&)_m._21 *= vScale.y;
    (cgVector3&)_m._31 *= vScale.z;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : lookAt()
/// <summary>
/// Generate a transform oriented toward the specified point.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::lookAt( const cgVector3 & vEye, const cgVector3 & vAt )
{
    cgVector3 vZ, vY, vX, vWorldY( 0.0f, 1.0f, 0.0f );
	cgFloat   fDistance;

    // Auto-generate appropriate up vector
	cgVector3::normalize( vZ, vAt - vEye );
	fDistance = cgVector3::dot( vWorldY, vZ );
	if ( fabs( fDistance ) < ( 1.0f - CGE_EPSILON ) )
    {
		vY = vWorldY - (vZ * fDistance);
    
    } // End if not degenerate
	else
	{
		vWorldY   = cgVector3( 1.0f, 0.0f, 0.0f );
		fDistance = cgVector3::dot( vWorldY, vZ );
		vY        = vWorldY - (vZ * fDistance);
	
    } // End if degenerate

    // Auto-generate right vector
	cgVector3::normalize( vY, vY );
	cgVector3::cross( vX, vY, vZ );

    // Populate new internal properties
    (cgVector3&)_m._11 = vX;
    (cgVector3&)_m._21 = vY;
    (cgVector3&)_m._31 = vZ;
    (cgVector3&)_m._41 = vEye;
    _m._14 = _m._24 = _m._34 = 0;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : lookAt() (Static)
/// <summary>
/// Generate a transform oriented toward the specified point.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::lookAt( cgTransform & tOut, const cgVector3 & vEye, const cgVector3 & vAt )
{
    return tOut.lookAt( vEye, vAt );
}

//-----------------------------------------------------------------------------
//  Name : lookAt()
/// <summary>
/// Generate a transform oriented toward the specified point.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::lookAt( const cgVector3 & vEye, const cgVector3 & vAt, const cgVector3 & vUpAlign )
{
    cgVector3 vX, vY, vZ;

    cgVector3::normalize( vZ, vAt - vEye );
    vY = vUpAlign - vZ * cgVector3::dot( vUpAlign, vZ );
    if ( cgVector3::length(vY) < CGE_EPSILON)
    {
        vY.x =      - vZ.y * vZ.x;
        vY.y = 1.0f - vZ.y * vZ.y;
        vY.z =      - vZ.y * vZ.z;

        if ( cgVector3::length(vY) < CGE_EPSILON )
        {
            vY.x =      - vZ.z * vZ.x;
            vY.y =      - vZ.z * vZ.y;
            vY.z = 1.0f - vZ.z * vZ.z;
        }
    }

    // Auto-generate right vector
    cgVector3::normalize(vY, vY);
    cgVector3::cross(vX, vY, vZ );

    // Populate new internal properties
    (cgVector3&)_m._11 = vX;
    (cgVector3&)_m._21 = vY;
    (cgVector3&)_m._31 = vZ;
    (cgVector3&)_m._41 = vEye;
    _m._14 = _m._24 = _m._34 = 0;
    _m._44 = 1.0f;

    // Return reference to self in order to allow multiple operations (i.e. a.rotate(...).scale(...))
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : lookAt() (Static)
/// <summary>
/// Generate a transform oriented toward the specified point.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::lookAt( cgTransform & tOut, const cgVector3 & vEye, const cgVector3 & vAt, const cgVector3 & vUpAlign )
{
    return tOut.lookAt( vEye, vAt, vUpAlign );
}

//-----------------------------------------------------------------------------
//  Name : scaling()
/// <summary>
/// Generate a new scaling transform, replacing the existing transformation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::scaling( cgFloat x, cgFloat y, cgFloat z )
{
    cgMatrix::scaling( _m, x, y, z );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : scaling() (Static)
/// <summary>
/// Generate a new scaling transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::scaling( cgTransform & tOut, cgFloat x, cgFloat y, cgFloat z )
{
    return tOut.scaling( x, y, z );
}

//-----------------------------------------------------------------------------
//  Name : rotation()
/// <summary>
/// Generate a new rotation transform, replacing the existing transformation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotation( cgFloat x, cgFloat y, cgFloat z )
{
    cgMatrix::rotationYawPitchRoll( _m, y, x, z );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : rotation() (Static)
/// <summary>
/// Generate a new rotation transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotation( cgTransform & tOut, cgFloat x, cgFloat y, cgFloat z )
{
    return tOut.rotation( x, y, z );
}

//-----------------------------------------------------------------------------
//  Name : rotationAxis()
/// <summary>
/// Generate a new axis rotation transform, replacing the existing 
/// transformation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotationAxis( cgFloat a, const cgVector3 & v )
{
    cgMatrix::rotationAxis( _m, v, a );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : rotationAxis() (Static)
/// <summary>
/// Generate a new axis rotation transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::rotationAxis( cgTransform & tOut, cgFloat a, const cgVector3 & v )
{
    return tOut.rotationAxis( a, v );
}

//-----------------------------------------------------------------------------
//  Name : translation()
/// <summary>
/// Generate a new translation transform, replacing the existing transformation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::translation( cgFloat x, cgFloat y, cgFloat z )
{
    cgMatrix::translation( _m, x, y, z );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : translation() (Static)
/// <summary>
/// Generate a new translation transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::translation( cgTransform & tOut, cgFloat x, cgFloat y, cgFloat z )
{
    return tOut.translation( x, y, z );
}

//-----------------------------------------------------------------------------
//  Name : translation()
/// <summary>
/// Generate a new translation transform, replacing the existing transformation.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::translation( const cgVector3 & v )
{
    cgMatrix::translation( _m, v.x, v.y, v.z );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : translation() (Static)
/// <summary>
/// Generate a new translation transform.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgTransform::translation( cgTransform & tOut, const cgVector3 & v )
{
    return tOut.translation( v );
}

//-----------------------------------------------------------------------------
//  Name : compare()
/// <summary>
/// Compare this transform against the provided transform in order to determine
/// if they are equivalent (within the specified tolerance).
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgTransform::compare( const cgTransform & t, cgFloat fTolerance ) const
{
    cgFloat fDifference;
    for ( cgInt i = 0; i < 4; ++i )
    {
        for ( cgInt j = 0; j < 4; ++j )
        {
            fDifference = _m.m[i][j] - t._m.m[i][j];
            if ( fabsf( fDifference ) > fTolerance )
                return (fDifference < 0) ? -1 : 1;
        }
    }
    
    // Equivalent
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : compare()
/// <summary>
/// Compare this transform against the provided transform in order to determine
/// if they are equivalent.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgTransform::compare( const cgTransform & t ) const
{
    return memcmp( &_m, &t._m, sizeof(_m) );
}

//-----------------------------------------------------------------------------
//  Name : isIdentity()
/// <summary>
/// Determine if the transform is in an identity state.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTransform::isIdentity( ) const
{
    static const cgTransform IdentityTransform;
    return ( (_m == IdentityTransform._m) == TRUE );
}