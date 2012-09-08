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
// Name : cgBoxShape.cpp                                                     //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        box shape.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBoxShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgBoxShape.h>
#include <Physics/cgPhysicsWorld.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgMathUtility.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgBoxShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBoxShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxShape::cgBoxShape( cgPhysicsWorld * pWorld, cgFloat fWidth, cgFloat fHeight, cgFloat fLength ) : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    fWidth  = max(CGE_EPSILON_1MM, fWidth);
    fHeight = max(CGE_EPSILON_1MM, fHeight);
    fLength = max(CGE_EPSILON_1MM, fLength);

    // Convert dimensions to physics system scale.
    fWidth  *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();
    fLength *= mWorld->toPhysicsScale();

    // Construct the wrapped Newton shape.
    cgMatrix mtxIdentity;
    cgMatrix::identity( mtxIdentity );
    mShape = NewtonCreateBox( pWorld->getInternalWorld(), fWidth, fHeight, fLength, getReferenceId(), mtxIdentity );
}

//-----------------------------------------------------------------------------
//  Name : cgBoxShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxShape::cgBoxShape( cgPhysicsWorld * pWorld, cgFloat fWidth, cgFloat fHeight, cgFloat fLength, const cgTransform & Offset ) : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    fWidth  = max(CGE_EPSILON_1MM, fWidth);
    fHeight = max(CGE_EPSILON_1MM, fHeight);
    fLength = max(CGE_EPSILON_1MM, fLength);

    // Convert dimensions and offset to physics system scale.
    cgTransform BoxOffset = Offset;
    BoxOffset.position() *= mWorld->toPhysicsScale();
    fWidth  *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();
    fLength *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the box dimensions.
    cgVector3 vScale = BoxOffset.localScale();
    BoxOffset.setLocalScale( 1, 1, 1 );
    fWidth *= vScale.x;
    fHeight *= vScale.y;
    fLength *= vScale.z;

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateBox( pWorld->getInternalWorld(), fWidth, fHeight, fLength, getReferenceId(), (cgMatrix)BoxOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgBoxShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxShape::cgBoxShape( cgPhysicsWorld * pWorld, const cgVector3 & Dimensions )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgFloat fWidth  = max(CGE_EPSILON_1MM, Dimensions.x);
    cgFloat fHeight = max(CGE_EPSILON_1MM, Dimensions.y);
    cgFloat fLength = max(CGE_EPSILON_1MM, Dimensions.z);

    // Convert dimensions to physics system scale.
    fWidth  *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();
    fLength *= mWorld->toPhysicsScale();

    // Construct the wrapped Newton shape.
    cgMatrix mtxIdentity;
    cgMatrix::identity( mtxIdentity );
    mShape = NewtonCreateBox( pWorld->getInternalWorld(), fWidth, fHeight, fLength, getReferenceId(), mtxIdentity );
}

//-----------------------------------------------------------------------------
//  Name : cgBoxShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxShape::cgBoxShape( cgPhysicsWorld * pWorld, const cgVector3 & Dimensions, const cgTransform & Offset )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgFloat fWidth  = max(CGE_EPSILON_1MM, Dimensions.x);
    cgFloat fHeight = max(CGE_EPSILON_1MM, Dimensions.y);
    cgFloat fLength = max(CGE_EPSILON_1MM, Dimensions.z);

    // Convert dimensions and offset to physics system scale.
    cgTransform BoxOffset = Offset;
    BoxOffset.position() *= mWorld->toPhysicsScale();
    fWidth  *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();
    fLength *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the box dimensions.
    cgVector3 vScale = BoxOffset.localScale();
    BoxOffset.setLocalScale( 1, 1, 1 );
    fWidth *= vScale.x;
    fHeight *= vScale.y;
    fLength *= vScale.z;

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateBox( pWorld->getInternalWorld(), fWidth, fHeight, fLength, getReferenceId(), (cgMatrix)BoxOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgBoxShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxShape::cgBoxShape( cgPhysicsWorld * pWorld, const cgBoundingBox & Bounds )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgVector3 Dimensions = Bounds.getDimensions();
    cgFloat fWidth  = max(CGE_EPSILON_1MM, Dimensions.x);
    cgFloat fHeight = max(CGE_EPSILON_1MM, Dimensions.y);
    cgFloat fLength = max(CGE_EPSILON_1MM, Dimensions.z);

    // Convert dimensions to physics system scale.
    fWidth  *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();
    fLength *= mWorld->toPhysicsScale();
    
    // Transfer bounding box origin offset into the transform.
    cgTransform BoxOffset;
    BoxOffset.translateLocal( mWorld->toPhysicsScale( Bounds.getCenter() ) );

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateBox( pWorld->getInternalWorld(), fWidth, fHeight, fLength, getReferenceId(), (cgMatrix)BoxOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgBoxShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxShape::cgBoxShape( cgPhysicsWorld * pWorld, const cgBoundingBox & Bounds, const cgTransform & Offset )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    const cgVector3 & Dimensions = Bounds.getDimensions();
    cgFloat fWidth  = max(CGE_EPSILON_1MM, Dimensions.x);
    cgFloat fHeight = max(CGE_EPSILON_1MM, Dimensions.y);
    cgFloat fLength = max(CGE_EPSILON_1MM, Dimensions.z);

    // Convert dimensions and offset to physics system scale.
    cgTransform BoxOffset = Offset;
    BoxOffset.position() *= mWorld->toPhysicsScale();
    fWidth  *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();
    fLength *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the box dimensions.
    cgVector3 vScale = BoxOffset.localScale();
    BoxOffset.setLocalScale( 1, 1, 1 );
    fWidth *= vScale.x;
    fHeight *= vScale.y;
    fLength *= vScale.z;

    // Transfer bounding box origin offset into the transform.
    BoxOffset.translateLocal( mWorld->toPhysicsScale( Bounds.getCenter() ) );
    
    // Construct the wrapped Newton shape.
    mShape = NewtonCreateBox( pWorld->getInternalWorld(), fWidth, fHeight, fLength, getReferenceId(), (cgMatrix)BoxOffset );
}

//-----------------------------------------------------------------------------
//  Name : ~cgBoxShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxShape::~cgBoxShape()
{

}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoxShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BoxShape )
        return true;

    // Supported by base?
    return cgConvexShape::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : compare () (Virtual)
/// <summary>
/// Compare the physics shapes to see if they describe the same data.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::Dispose()" />
//-----------------------------------------------------------------------------
cgInt cgBoxShape::compare( cgPhysicsShape * pShape ) const
{
    // Compare base properties first.
    cgInt nResult = cgPhysicsShape::compare( pShape );
    if ( nResult != 0 )
        return nResult;

    // Now compare custom properties.
    cgBoxShape * pBoxShape = (cgBoxShape*)pShape;
    cgMatrix mtxOffsetSrc, mtxOffsetCmp;
    cgVector3 vMinSrc, vMaxSrc, vMinCmp, vMaxCmp;
    NewtonCollisionCalculateAABB( mShape, mtxOffsetSrc, vMinSrc, vMaxSrc );
    NewtonCollisionCalculateAABB( pBoxShape->mShape, mtxOffsetCmp, vMinCmp, vMaxCmp );
    cgTransform tOffsetSrc( mtxOffsetSrc ), tOffsetCmp( mtxOffsetCmp );
    nResult = tOffsetSrc.compare( tOffsetCmp, CGE_EPSILON );
    if ( nResult != 0 ) return nResult;
    nResult = cgMathUtility::compareVectors( vMinSrc, vMinCmp );
    if ( nResult != 0 ) return nResult;
    nResult = cgMathUtility::compareVectors( vMaxSrc, vMaxCmp );
    if ( nResult != 0 ) return nResult;

    // Equivalent
    return 0;
}