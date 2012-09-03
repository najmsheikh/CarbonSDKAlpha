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
// Name : cgSphereShape.cpp                                                  //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        sphere shape.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSphereShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgSphereShape.h>
#include <Physics/cgPhysicsWorld.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgMathUtility.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgSphereShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSphereShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereShape::cgSphereShape( cgPhysicsWorld * pWorld, cgFloat fRadiusX, cgFloat fRadiusY, cgFloat fRadiusZ ) : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    fRadiusX = max(CGE_EPSILON_1MM, fRadiusX);
    fRadiusY = max(CGE_EPSILON_1MM, fRadiusY);
    fRadiusZ = max(CGE_EPSILON_1MM, fRadiusZ);

    // Convert dimensions to physics system scale.
    fRadiusX *= mWorld->toPhysicsScale();
    fRadiusY *= mWorld->toPhysicsScale();
    fRadiusZ *= mWorld->toPhysicsScale();    

    // Construct the wrapped Newton shape.
    cgMatrix mtxIdentity;
    cgMatrix::identity( mtxIdentity );
    mShape = NewtonCreateSphere( pWorld->getInternalWorld(), fRadiusX, fRadiusY, fRadiusZ, getReferenceId(), mtxIdentity );
}

//-----------------------------------------------------------------------------
//  Name : cgSphereShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereShape::cgSphereShape( cgPhysicsWorld * pWorld, cgFloat fRadiusX, cgFloat fRadiusY, cgFloat fRadiusZ, const cgTransform & Offset ) : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    fRadiusX = max(CGE_EPSILON_1MM, fRadiusX);
    fRadiusY = max(CGE_EPSILON_1MM, fRadiusY);
    fRadiusZ = max(CGE_EPSILON_1MM, fRadiusZ);

    // Convert dimensions and offset to physics system scale.
    cgTransform SphereOffset = Offset;
    SphereOffset.position() *= mWorld->toPhysicsScale();
    fRadiusX *= mWorld->toPhysicsScale();
    fRadiusY *= mWorld->toPhysicsScale();
    fRadiusZ *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the sphere radii.
    cgVector3 vScale = SphereOffset.localScale();
    SphereOffset.setLocalScale( 1, 1, 1 );
    fRadiusX *= vScale.x;
    fRadiusY *= vScale.y;
    fRadiusZ *= vScale.z;

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateSphere( pWorld->getInternalWorld(), fRadiusX, fRadiusY, fRadiusZ, getReferenceId(), (cgMatrix)SphereOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgSphereShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereShape::cgSphereShape( cgPhysicsWorld * pWorld, const cgVector3 & Dimensions )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgVector3 SphereRadii = Dimensions * 0.5f;
    cgFloat fRadiusX = max(CGE_EPSILON_1MM, SphereRadii.x);
    cgFloat fRadiusY = max(CGE_EPSILON_1MM, SphereRadii.y);
    cgFloat fRadiusZ = max(CGE_EPSILON_1MM, SphereRadii.z);

    // Convert dimensions and offset to physics system scale.
    fRadiusX *= mWorld->toPhysicsScale();
    fRadiusY *= mWorld->toPhysicsScale();
    fRadiusZ *= mWorld->toPhysicsScale();    

    // Construct the wrapped Newton shape.
    cgMatrix mtxIdentity;
    cgMatrix::identity( mtxIdentity );
    mShape = NewtonCreateSphere( pWorld->getInternalWorld(), fRadiusX, fRadiusY, fRadiusZ, getReferenceId(), mtxIdentity );
}

//-----------------------------------------------------------------------------
//  Name : cgSphereShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereShape::cgSphereShape( cgPhysicsWorld * pWorld, const cgVector3 & Dimensions, const cgTransform & Offset )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgVector3 SphereRadii = Dimensions * 0.5f;
    cgFloat fRadiusX = max(CGE_EPSILON_1MM, SphereRadii.x);
    cgFloat fRadiusY = max(CGE_EPSILON_1MM, SphereRadii.y);
    cgFloat fRadiusZ = max(CGE_EPSILON_1MM, SphereRadii.z);

    // Convert dimensions and offset to physics system scale.
    cgTransform SphereOffset = Offset;
    SphereOffset.position() *= mWorld->toPhysicsScale();
    fRadiusX *= mWorld->toPhysicsScale();
    fRadiusY *= mWorld->toPhysicsScale();
    fRadiusZ *= mWorld->toPhysicsScale();

    // Remove scale from supplied offset and apply it to the sphere radii.
    cgVector3 vScale = SphereOffset.localScale();
    SphereOffset.setLocalScale( 1, 1, 1 );
    fRadiusX *= vScale.x;
    fRadiusY *= vScale.y;
    fRadiusZ *= vScale.z;

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateSphere( pWorld->getInternalWorld(), fRadiusX, fRadiusY, fRadiusZ, getReferenceId(), (cgMatrix)SphereOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgSphereShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereShape::cgSphereShape( cgPhysicsWorld * pWorld, const cgBoundingBox & Bounds )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgVector3 SphereRadii = Bounds.getDimensions() * 0.5f;
    cgFloat fRadiusX = max(CGE_EPSILON_1MM, SphereRadii.x);
    cgFloat fRadiusY = max(CGE_EPSILON_1MM, SphereRadii.y);
    cgFloat fRadiusZ = max(CGE_EPSILON_1MM, SphereRadii.z);

    // Convert dimensions and offset to physics system scale.
    fRadiusX *= mWorld->toPhysicsScale();
    fRadiusY *= mWorld->toPhysicsScale();
    fRadiusZ *= mWorld->toPhysicsScale();    
    
    // Transfer bounding sphere origin offset into the transform.
    cgTransform SphereOffset;
    SphereOffset.translateLocal( mWorld->toPhysicsScale( Bounds.getCenter() ) );

    // Construct the wrapped Newton shape.
    mShape = NewtonCreateSphere( pWorld->getInternalWorld(), fRadiusX, fRadiusY, fRadiusZ, getReferenceId(), (cgMatrix)SphereOffset );
}

//-----------------------------------------------------------------------------
//  Name : cgSphereShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereShape::cgSphereShape( cgPhysicsWorld * pWorld, const cgBoundingBox & Bounds, const cgTransform & Offset )  : cgConvexShape( pWorld )
{
    // Ensure that none of the dimensions are degenerate (1mm minimum).
    cgVector3 SphereRadii = Bounds.getDimensions() * 0.5f;
    cgFloat fRadiusX = max(CGE_EPSILON_1MM, SphereRadii.x);
    cgFloat fRadiusY = max(CGE_EPSILON_1MM, SphereRadii.y);
    cgFloat fRadiusZ = max(CGE_EPSILON_1MM, SphereRadii.z);

    // Convert dimensions and offset to physics system scale.
    cgTransform SphereOffset = Offset;
    SphereOffset.position() *= mWorld->toPhysicsScale();
    fRadiusX *= mWorld->toPhysicsScale();
    fRadiusY *= mWorld->toPhysicsScale();
    fRadiusZ *= mWorld->toPhysicsScale(); 

    // Remove scale from supplied offset and apply it to the sphere radii.
    cgVector3 vScale = SphereOffset.localScale();
    SphereOffset.setLocalScale( 1, 1, 1 );
    fRadiusX *= vScale.x;
    fRadiusY *= vScale.y;
    fRadiusZ *= vScale.z;
    
    // Transfer bounding sphere origin offset into the transform.
    SphereOffset.translateLocal( mWorld->toPhysicsScale( Bounds.getCenter() ) );
    
    // Construct the wrapped Newton shape.
    mShape = NewtonCreateSphere( pWorld->getInternalWorld(), fRadiusX, fRadiusY, fRadiusZ, getReferenceId(), (cgMatrix)SphereOffset );
}

//-----------------------------------------------------------------------------
//  Name : ~cgSphereShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereShape::~cgSphereShape()
{

}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSphereShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SphereShape )
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
cgInt cgSphereShape::compare( cgPhysicsShape * pShape ) const
{
    // Compare base properties first.
    cgInt nResult = cgPhysicsShape::compare( pShape );
    if ( nResult != 0 )
        return nResult;

    // Now compare custom properties.
    cgSphereShape * pSphereShape = (cgSphereShape*)pShape;
    cgMatrix mtxOffsetSrc, mtxOffsetCmp;
    cgVector3 vMinSrc, vMaxSrc, vMinCmp, vMaxCmp;
    NewtonCollisionCalculateAABB( mShape, mtxOffsetSrc, vMinSrc, vMaxSrc );
    NewtonCollisionCalculateAABB( pSphereShape->mShape, mtxOffsetCmp, vMinCmp, vMaxCmp );
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