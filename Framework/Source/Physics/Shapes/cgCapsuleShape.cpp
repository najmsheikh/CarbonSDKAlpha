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
// Name : cgCapsuleShape.cpp                                                 //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        capsule shape.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgCapsuleShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgCapsuleShape.h>
#include <Physics/cgPhysicsWorld.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgMathUtility.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgCapsuleShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCapsuleShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleShape::cgCapsuleShape( cgPhysicsWorld * pWorld, cgFloat fRadius, cgFloat fHeight ) : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    // Height must be at least 2*radius (Newton requirement)
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    fHeight = max(2.0f * fRadius, fHeight);

    // Convert dimensions to physics system scale.
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Compute offset matrix for capsules. Newton capsules
    // are constructed along the 'X' axis . We want them aligned 
    // to the Y axis by default.
    cgTransform CapsuleOffset;
    CapsuleOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );
    
    // Construct the wrapped Newton shape.
    cgMatrix mtxIdentity;
    cgMatrix::identity( mtxIdentity );
    mShape = NewtonCreateCapsule( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CapsuleOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCapsuleShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleShape::cgCapsuleShape( cgPhysicsWorld * pWorld, cgFloat fRadius, cgFloat fHeight, const cgTransform & Offset ) : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    // Height must be at least 2*radius (Newton requirement)
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    fHeight = max(2.0f * fRadius, fHeight);

    // Convert dimensions and offset to physics system scale.
    cgTransform CapsuleOffset = Offset;
    CapsuleOffset.position() *= mWorld->toPhysicsScale();
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the capsule dimensions.
    cgVector3 vScale = CapsuleOffset.localScale();
    CapsuleOffset.setLocalScale( 1, 1, 1 );
    fRadius *= max(vScale.x,vScale.z);
    fHeight *= vScale.y;
    
    // Newton capsules are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    CapsuleOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCapsule( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CapsuleOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCapsuleShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleShape::cgCapsuleShape( cgPhysicsWorld * pWorld, const cgVector3 & Dimensions )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    // Height must be at least 2*radius (Newton requirement)
    cgFloat fRadius = ((Dimensions.x > Dimensions.z) ? Dimensions.x : Dimensions.z) * 0.5f;
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    cgFloat fHeight = max(2.0f * fRadius, Dimensions.y);

    // Convert dimensions to physics system scale.
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Newton capsules are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    cgTransform CapsuleOffset;
    CapsuleOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );
    
    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCapsule( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CapsuleOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCapsuleShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleShape::cgCapsuleShape( cgPhysicsWorld * pWorld, const cgVector3 & Dimensions, const cgTransform & Offset )  : cgConvexShape( pWorld )
{
    // Remove scale from supplied offset and apply it to the capsule dimensions.
    cgVector3 FinalDimensions;
    cgTransform CapsuleOffset = Offset;
    cgVector3 vScale = CapsuleOffset.localScale();
    CapsuleOffset.setLocalScale( 1, 1, 1 );
    FinalDimensions.x = Dimensions.x * vScale.x;
    FinalDimensions.y = Dimensions.y * vScale.y;
    FinalDimensions.z = Dimensions.z * vScale.z;
    
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    // Height must be at least 2*radius (Newton requirement)
    cgFloat fRadius = ((FinalDimensions.x > FinalDimensions.z) ? FinalDimensions.x : FinalDimensions.z) * 0.5f;
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    cgFloat fHeight = max(2.0f * fRadius, FinalDimensions.y);

    // Convert dimensions and offset to physics system scale.
    CapsuleOffset.position() *= mWorld->toPhysicsScale();
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Newton capsules are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    CapsuleOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCapsule( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CapsuleOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCapsuleShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleShape::cgCapsuleShape( cgPhysicsWorld * pWorld, const cgBoundingBox & Bounds )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    // Height must be at least 2*radius (Newton requirement)
    const cgVector3 & Dimensions = Bounds.getDimensions();
    cgFloat fRadius = ((Dimensions.x > Dimensions.z) ? Dimensions.x : Dimensions.z) * 0.5f;
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    cgFloat fHeight = max(2.0f * fRadius, Dimensions.y);

    // Convert dimensions to physics system scale.
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Transfer bounding capsule origin offset into the transform.
    cgTransform CapsuleOffset;
    CapsuleOffset.translateLocal( mWorld->toPhysicsScale( Bounds.getCenter() ) );

    // Newton capsules are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    CapsuleOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCapsule( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CapsuleOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCapsuleShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleShape::cgCapsuleShape( cgPhysicsWorld * pWorld, const cgBoundingBox & Bounds, const cgTransform & Offset )  : cgConvexShape( pWorld )
{
    // Remove scale from supplied offset and apply it to the capsule dimensions.
    const cgVector3 & Dimensions = Bounds.getDimensions();
    cgVector3 FinalDimensions;
    cgTransform CapsuleOffset = Offset;
    cgVector3 vScale = CapsuleOffset.localScale();
    CapsuleOffset.setLocalScale( 1, 1, 1 );
    FinalDimensions.x = Dimensions.x * vScale.x;
    FinalDimensions.y = Dimensions.y * vScale.y;
    FinalDimensions.z = Dimensions.z * vScale.z;

    // Ensure that none of the dimensions are degenerate (1mm minimum).
    // Height must be at least 2*radius (Newton requirement)
    cgFloat fRadius = ((FinalDimensions.x > FinalDimensions.z) ? FinalDimensions.x : FinalDimensions.z) * 0.5f;
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    cgFloat fHeight = max(2.0f * fRadius, FinalDimensions.y);

    // Convert dimensions and offset to physics system scale.
    CapsuleOffset.position() *= mWorld->toPhysicsScale();
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Transfer bounding capsule origin offset into the transform.
    CapsuleOffset.translateLocal( mWorld->toPhysicsScale( Bounds.getCenter() ) );

    // Newton capsules are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    CapsuleOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );
    
    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCapsule( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CapsuleOffset );
}

//-----------------------------------------------------------------------------
//  Name : ~cgCapsuleShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleShape::~cgCapsuleShape()
{

}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCapsuleShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_CapsuleShape )
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
cgInt cgCapsuleShape::compare( cgPhysicsShape * pShape ) const
{
    // Compare base properties first.
    cgInt nResult = cgPhysicsShape::compare( pShape );
    if ( nResult != 0 )
        return nResult;

    // Now compare custom properties.
    cgCapsuleShape * pCapsuleShape = (cgCapsuleShape*)pShape;
    cgMatrix mtxOffsetSrc, mtxOffsetCmp;
    cgVector3 vMinSrc, vMaxSrc, vMinCmp, vMaxCmp;
    NewtonCollisionCalculateAABB( mShape, mtxOffsetSrc, vMinSrc, vMaxSrc );
    NewtonCollisionCalculateAABB( pCapsuleShape->mShape, mtxOffsetCmp, vMinCmp, vMaxCmp );
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