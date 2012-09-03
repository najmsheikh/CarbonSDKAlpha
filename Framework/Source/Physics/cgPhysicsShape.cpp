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
// Name : cgPhysicsShape.cpp                                                 //
//                                                                           //
// Desc : Base class from which specific physics shapes should derive.       //
//        Shapes provide the means by which a body can describes its         //
//        interior volume(s) or geometry used for the provision of both      //
//        collision detection and dynamics response. The shape itself does   //
//        not exist within the physics world, but can be attached to a body  //
//        that does.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPhysicsShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsShape.h>
#include <Physics/cgPhysicsWorld.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgPhysicsShape Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPhysicsShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsShape::cgPhysicsShape( cgPhysicsWorld * pWorld ) : cgReference( cgReferenceManager::generateInternalRefId() )
{
    // Initialize variables to sensible defaults
    mWorld        = pWorld;
    mShape        = CG_NULL;
    mShapeCached  = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgPhysicsShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsShape::~cgPhysicsShape()
{
    // Clean up.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgPhysicsShape::dispose( bool bDisposeBase )
{
    // Release wrapped newton shape.
    if ( mShape )
        NewtonReleaseCollision( mWorld->getInternalWorld(), mShape );
    
    // Clear variables
    mShape        = CG_NULL;
    mShapeCached  = false;

    // Call base if requested.
    if ( bDisposeBase == true )
        cgReference::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_PhysicsShape )
        return true;

    // Unsupported
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getInternalShape()
/// <summary>
/// Retrieve the internal shape object specific to Newton. This is not
/// technically part of the public interface and should not be called
/// by the application directly.
/// </summary>
//-----------------------------------------------------------------------------
NewtonCollision * cgPhysicsShape::getInternalShape( )
{
    return mShape;
}

//-----------------------------------------------------------------------------
//  Name : compare () (Virtual)
/// <summary>
/// Compare the physics shapes to see if they describe the same data.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
cgInt cgPhysicsShape::compare( cgPhysicsShape * pShape ) const
{
    if ( getReferenceType() == pShape->getReferenceType() )
        return 0;
    else if ( getReferenceType() < pShape->getReferenceType() )
        return -1;
    else
        return 1;
}

///////////////////////////////////////////////////////////////////////////////
// cgPhysicsShapeCacheKey Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPhysicsShapeCacheKey () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsShapeCacheKey::cgPhysicsShapeCacheKey( cgPhysicsShape * pShape ) : 
    mShape( pShape )
{
}

//-----------------------------------------------------------------------------
//  Name : operator< () (Virtual)
/// <summary>
/// Less-than comparison operator.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsShapeCacheKey::operator<( const cgPhysicsShapeCacheKey & Key ) const
{
    cgAssert( mShape != CG_NULL && Key.mShape != CG_NULL );
    return (Key.mShape->compare( mShape ) < 0);
}