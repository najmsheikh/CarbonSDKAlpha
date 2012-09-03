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
// Name : cgCylinderShape.cpp                                                //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        cylinder shape.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgCylinderShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgCylinderShape.h>
#include <Physics/cgPhysicsWorld.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgMathUtility.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgCylinderShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCylinderShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCylinderShape::cgCylinderShape( cgPhysicsWorld * pWorld, cgFloat fRadius, cgFloat fHeight ) : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    fHeight = max(CGE_EPSILON_1MM, fHeight);

    // Convert dimensions to physics system scale.
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Compute offset matrix for cylinders. Newton cylinders
    // are constructed along the 'X' axis . We want them aligned 
    // to the Y axis by default.
    cgTransform CylinderOffset;
    CylinderOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );
    
    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCylinder( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CylinderOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCylinderShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCylinderShape::cgCylinderShape( cgPhysicsWorld * pWorld, cgFloat fRadius, cgFloat fHeight, const cgTransform & Offset ) : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    fHeight = max(CGE_EPSILON_1MM, fHeight);

    // Convert dimensions and offset to physics system scale.
    cgTransform CylinderOffset = Offset;
    CylinderOffset.position() *= mWorld->toPhysicsScale();
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the cylinder dimensions.
    cgVector3 vScale = CylinderOffset.localScale();
    CylinderOffset.setLocalScale( 1, 1, 1 );
    fRadius *= max(vScale.x,vScale.z);
    fHeight *= vScale.y;
    
    // Newton cylinders are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    CylinderOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCylinder( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CylinderOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCylinderShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCylinderShape::cgCylinderShape( cgPhysicsWorld * pWorld, const cgVector3 & Dimensions )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgFloat fRadius = ((Dimensions.x > Dimensions.z) ? Dimensions.x : Dimensions.z) * 0.5f;
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    cgFloat fHeight = max(CGE_EPSILON_1MM, Dimensions.y);

    // Convert dimensions to physics system scale.
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Newton cylinders are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    cgTransform CylinderOffset;
    CylinderOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );
    
    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCylinder( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CylinderOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCylinderShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCylinderShape::cgCylinderShape( cgPhysicsWorld * pWorld, const cgVector3 & Dimensions, const cgTransform & Offset )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgFloat fRadius = ((Dimensions.x > Dimensions.z) ? Dimensions.x : Dimensions.z) * 0.5f;
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    cgFloat fHeight = max(CGE_EPSILON_1MM, Dimensions.y);

    // Convert dimensions and offset to physics system scale.
    cgTransform CylinderOffset = Offset;
    CylinderOffset.position() *= mWorld->toPhysicsScale();
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the cylinder dimensions.
    cgVector3 vScale = CylinderOffset.localScale();
    CylinderOffset.setLocalScale( 1, 1, 1 );
    fRadius *= max(vScale.x,vScale.z);
    fHeight *= vScale.y;

    // Newton cylinders are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    CylinderOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCylinder( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CylinderOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCylinderShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCylinderShape::cgCylinderShape( cgPhysicsWorld * pWorld, const cgBoundingBox & Bounds )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    const cgVector3 & Dimensions = Bounds.getDimensions();
    cgFloat fRadius = ((Dimensions.x > Dimensions.z) ? Dimensions.x : Dimensions.z) * 0.5f;
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    cgFloat fHeight = max(CGE_EPSILON_1MM, Dimensions.y);

    // Convert dimensions to physics system scale.
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();
    
    // Transfer bounding cylinder origin offset into the transform.
    cgTransform CylinderOffset;
    CylinderOffset.translateLocal( mWorld->toPhysicsScale( Bounds.getCenter() ) );

    // Newton cylinders are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    CylinderOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCylinder( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CylinderOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgCylinderShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCylinderShape::cgCylinderShape( cgPhysicsWorld * pWorld, const cgBoundingBox & Bounds, const cgTransform & Offset )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    const cgVector3 & Dimensions = Bounds.getDimensions();
    cgFloat fRadius = ((Dimensions.x > Dimensions.z) ? Dimensions.x : Dimensions.z) * 0.5f;
    fRadius = max(CGE_EPSILON_1MM, fRadius);
    cgFloat fHeight = max(CGE_EPSILON_1MM, Dimensions.y);

    // Convert dimensions and offset to physics system scale.
    cgTransform CylinderOffset = Offset;
    CylinderOffset.position() *= mWorld->toPhysicsScale();
    fRadius *= mWorld->toPhysicsScale();
    fHeight *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the cylinder dimensions.
    cgVector3 vScale = CylinderOffset.localScale();
    CylinderOffset.setLocalScale( 1, 1, 1 );
    fRadius *= max(vScale.x,vScale.z);
    fHeight *= vScale.y;
    
    // Transfer bounding cylinder origin offset into the transform.
    CylinderOffset.translateLocal( mWorld->toPhysicsScale( Bounds.getCenter() ) );

    // Newton cylinders are constructed along the 'X' axis . 
    // We want them aligned to the Y axis by default.
    CylinderOffset.rotateLocal( 0, 0, CGEToRadian(90.0f) );
    
    // Construct the wrapped Newton shape.
    mShape = NewtonCreateCylinder( pWorld->getInternalWorld(), fRadius, fHeight, getReferenceId(), (cgMatrix)CylinderOffset );
}

//-----------------------------------------------------------------------------
//  Name : ~cgCylinderShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCylinderShape::~cgCylinderShape()
{

}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCylinderShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_CylinderShape )
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
cgInt cgCylinderShape::compare( cgPhysicsShape * pShape ) const
{
    // Compare base properties first.
    cgInt nResult = cgPhysicsShape::compare( pShape );
    if ( nResult != 0 )
        return nResult;

    // Now compare custom properties.
    cgCylinderShape * pCylinderShape = (cgCylinderShape*)pShape;
    cgMatrix mtxOffsetSrc, mtxOffsetCmp;
    cgVector3 vMinSrc, vMaxSrc, vMinCmp, vMaxCmp;
    NewtonCollisionCalculateAABB( mShape, mtxOffsetSrc, vMinSrc, vMaxSrc );
    NewtonCollisionCalculateAABB( pCylinderShape->mShape, mtxOffsetCmp, vMinCmp, vMaxCmp );
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