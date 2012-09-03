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
// Name : cgJointObject.cpp                                                  //
//                                                                           //
// Desc : Our physics joint class implemented as a scene object that can be  //
//        managed and controlled in the same way as any other object.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgJointObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgJointObject.h>
#include <World/Objects/cgCameraObject.h>

///////////////////////////////////////////////////////////////////////////////
// cgJointObject Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgJointObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointObject::cgJointObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgJointObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointObject::cgJointObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgJointObject * pObject = (cgJointObject*)pInit;
}

//-----------------------------------------------------------------------------
//  Name : ~cgJointObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointObject::~cgJointObject()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgJointObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources

    // Dispose base.
    if ( bDisposeBase == true )
        cgWorldObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_JointObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointObject::getSubElementCategories( cgObjectSubElementCategory::Map & Categories ) const
{
    // Joints do not have support for sub-elements. They cannot
    // have collision shapes associated with them because they
    // cannot be rigid bodies.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgJointObject::applyObjectRescale( cgFloat fScale )
{
    // Nothing to re-scale in the base class.

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

///////////////////////////////////////////////////////////////////////////////
// cgJointNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointNode::cgJointNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Joint%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointNode::cgJointNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables to sensible defaults
    cgJointNode * pNode = (cgJointNode*)pInit;
}

//-----------------------------------------------------------------------------
//  Name : ~cgJointNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgJointNode::~cgJointNode()
{
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_JointNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgJointNode::canScale( ) const
{
    // Joints cannot be scaled by default.
    return false;
}

//-----------------------------------------------------------------------------
// Name : validateAttachment ( ) (Virtual)
/// <summary>
/// Allows nodes to describe whether or not a particular attachment of
/// another node as either a child or parent of this node is valid.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointNode::validateAttachment( cgObjectNode * pNode, bool bNodeAsChild )
{
    // Joints cannot be attached to other joints in any fashion.
    if ( pNode->queryObjectType( RTID_JointObject ) )
        return false;

    // Valid
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSandboxIconInfo ( ) (Virtual)
/// <summary>
/// Retrieve information about the iconic representation of this object as it
/// is to be displayed in the sandbox rendering viewports.
/// </summary>
//-----------------------------------------------------------------------------
bool cgJointNode::getSandboxIconInfo( cgCameraNode * pCamera, const cgSize & ViewportSize, cgString & strAtlas, cgString & strFrame, cgVector3 & vIconOrigin )
{
    strAtlas  = _T("sys://Textures/ObjectBillboardSheet.xml");
    strFrame  = _T("Joint");
    
    // Position icon
    vIconOrigin = getPosition(false);
    cgFloat fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, vIconOrigin, 2.5f );
    vIconOrigin += (pCamera->getXAxis() * 0.0f * fZoomFactor) + (pCamera->getYAxis() * 17.0f * fZoomFactor);
    return true;
}
