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
// Name : cgMeshObject.cpp                                                   //
//                                                                           //
// Desc : Simple cgSceneObject type designed for storage and rendering of    //
//        straight forward cgMesh objects.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgMeshObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgMeshObject.h>
#include <World/cgVisibilitySet.h>
#include <World/cgScene.h>
#include <World/Objects/Elements/cgHullCollisionShapeElement.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgMesh.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/Shapes/cgMeshShape.h>
#include <Physics/Bodies/cgRigidBody.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgMeshObject::mInsertMesh;
cgWorldQuery cgMeshObject::mUpdateMeshData;
cgWorldQuery cgMeshObject::mUpdateProcessStages;
cgWorldQuery cgMeshObject::mLoadMesh;

//-----------------------------------------------------------------------------
//  Name : cgMeshObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshObject::cgMeshObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize defaults
    mShadowStage = cgSceneProcessStage::Both;

    // The mesh handle should manage the resource as an 'owner' handle. 
    // This will ensure that soft (database) reference counting will be
    // considered and that the object ownership is correctly recorded.
    mMesh.setOwnerDetails( this );
    mMesh.enableDatabaseUpdate( (isInternalReference() == false) );
}

//-----------------------------------------------------------------------------
//  Name : cgMeshObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshObject::cgMeshObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // The mesh handle should manage the resource as an 'owner' handle. 
    // This will ensure that soft (database) reference counting will be
    // considered and that the object ownership is correctly recorded.
    mMesh.setOwnerDetails( this );
    mMesh.enableDatabaseUpdate( (isInternalReference() == false) );

    // Duplicate values from object to clone.
    cgMeshObject * pObject = (cgMeshObject*)pInit;
    mShadowStage = pObject->mShadowStage;
    // ToDo: 9999 - Clone
    /*m_Lighting              = pObject->m_Lighting;
    m_ReceiveShadows        = pObject->m_ReceiveShadows;*/

    // Clone any mesh.
    if ( pObject->mMesh.isValid() == true )
    {
        if ( InitMethod == cgCloneMethod::Copy )
        {
            // Get source mesh and ensure it is loaded.
            cgMesh * pMesh = (cgMesh*)pObject->mMesh.getResource( true );
            
            // Allocate a new mesh into which we will copy the data.
            cgMesh * pNewMesh = CG_NULL;
            if ( mWorld != CG_NULL )
                pNewMesh = new cgMesh( mWorld->generateRefId( isInternalReference() ), mWorld, pMesh );
            else
                pNewMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL, pMesh );

            // Add for management.
            cgResourceManager * pResources = cgResourceManager::getInstance();
            pResources->addMesh( &mMesh, pNewMesh, cgResourceFlags::ForceNew, pMesh->getResourceName(), cgDebugSource() );
        
        } // End if copy
        else if ( InitMethod == cgCloneMethod::DataInstance )
        {
            // Just duplicate the reference. Our owner mode resource handle
            // will automatically add a reference appropriately.
            mMesh = pObject->mMesh;

        } // End if reference

    } // End if source mesh valid

    // ToDo: 9999 - Clone
    
}

//-----------------------------------------------------------------------------
//  Name : ~cgMeshObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshObject::~cgMeshObject()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // We should simply 'disconnect' from the underlying mesh, never
    // physically remove the reference from the database.
    mMesh.enableDatabaseUpdate( false );

    // Release the resources we're managing (if any).
    mMesh.close();

    // Re-enable mesh handle database updates in case
    // the object is resurrected.
    mMesh.enableDatabaseUpdate( (isInternalReference() == false) );

    // Call base class implementation
    if ( bDisposeBase == true )
        cgWorldObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgMeshObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgMeshObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgMeshObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    return new cgMeshObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : setMesh ()
/// <summary>
/// Set an already managed mesh resource to this mesh container object.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshObject::setMesh( const cgMeshHandle & hMesh )
{
    // Store the new reference to the specified mesh.
    // This will dereference the old mesh automatically
    // which may lead to it being removed from the database.
    mMesh = hMesh;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MeshData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getMesh ()
/// <summary>
/// Retrieve the mesh resource being managed by this mesh container object.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshHandle cgMeshObject::getMesh( ) const
{
    return mMesh;
}

//-----------------------------------------------------------------------------
// Name : createBox()
/// <summary>
/// Simple utility function to create a box mesh of the specified size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::createBox( cgFloat fWidth, cgFloat fHeight, cgFloat fDepth, cgUInt32 nWidthSegs, cgUInt32 nHeightSegs, cgUInt32 nDepthSegs, bool bInverted, cgMeshCreateOrigin::Base Origin )
{
    // Size must not be entirely degenerate on more than one axis.
    // If it is, the welding process undertaken during 'EndPrepare()'
    // will collapse the entire mesh and remove all faces.
    if ( (fabsf(fWidth) < CGE_EPSILON_1MM && fabsf(fHeight) < CGE_EPSILON_1MM) || 
         (fabsf(fWidth) < CGE_EPSILON_1MM && fabsf(fDepth) < CGE_EPSILON_1MM) || 
         (fabsf(fHeight) < CGE_EPSILON_1MM && fabsf(fDepth) < CGE_EPSILON_1MM))
        return false;

    // Create new mesh data container if one does not already exist.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( !pMesh )
    {
        if ( mWorld != CG_NULL )
            pMesh = new cgMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
        else
            pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Generate mesh data
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createBox( pFormat, fWidth, fHeight, fDepth, nWidthSegs, nHeightSegs, nDepthSegs, 
                               bInverted, Origin, true, pResources ) == true )
            pResources->addMesh( &mMesh, pMesh, cgResourceFlags::ForceNew, _T("Core::Mesh::Box"), cgDebugSource() );
        
        // Did the box creation succeed?
        if ( mMesh.isValid() == false )
        {
            pMesh->deleteReference();
            return false;
        
        } // End if failed

    } // End if no mesh
    else
    {
        // Clear and re-create.
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createBox( pFormat, fWidth, fHeight, fDepth, nWidthSegs, nHeightSegs, nDepthSegs, 
                               bInverted, Origin, true, pResources ) == false )
        {
            mMesh.close( true );
            return false;
        
        } // End if failed

    } // End if mesh exists
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MeshData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : createCylinder()
/// <summary>
/// Simple utility function to create a cylinder mesh of the specified size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::createCylinder( cgFloat fRadius, cgFloat fHeight, cgUInt32 nStacks, cgUInt32 nSlices, bool bInverted, cgMeshCreateOrigin::Base Origin )
{
    // Size must not be entirely degenerate on more than one axis.
    // If it is, the welding process undertaken during 'EndPrepare()'
    // will collapse the entire mesh and remove all faces.
    if ( (fabsf(fRadius) < CGE_EPSILON_1MM && fabsf(fHeight) < CGE_EPSILON_1MM) )
        return false;

    // Create new mesh data container if one does not already exist.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL )
    {
        if ( mWorld != CG_NULL )
            pMesh = new cgMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
        else
            pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Generate mesh data
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createCylinder( pFormat, fRadius, fHeight, nStacks, nSlices, bInverted, Origin, true, pResources ) == true )
            pResources->addMesh( &mMesh, pMesh, cgResourceFlags::ForceNew, _T("Core::Mesh::Cylinder"), cgDebugSource() );
        
        // Did the box creation succeed?
        if ( mMesh.isValid() == false )
        {
            pMesh->deleteReference();
            return false;
        
        } // End if failed

    } // End if no mesh
    else
    {
        // Clear and re-create.
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createCylinder( pFormat, fRadius, fHeight, nStacks, nSlices, bInverted, Origin, true, pResources ) == false )
        {
            mMesh.close( true );
            return false;
        
        } // End if failed

    } // End if mesh exists
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MeshData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : createCapsule()
/// <summary>
/// Simple utility function to create a capsule mesh of the specified size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::createCapsule( cgFloat fRadius, cgFloat fHeight, cgUInt32 nStacks, cgUInt32 nSlices, bool bInverted, cgMeshCreateOrigin::Base Origin )
{
    // Size must not be entirely degenerate on more than one axis.
    // If it is, the welding process undertaken during 'EndPrepare()'
    // will collapse the entire mesh and remove all faces.
    if ( (fabsf(fRadius) < CGE_EPSILON_1MM && fabsf(fHeight) < CGE_EPSILON_1MM) )
        return false;

    // Create new mesh data container if one does not already exist.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL )
    {
        if ( mWorld != CG_NULL )
            pMesh = new cgMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
        else
            pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Generate mesh data
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createCapsule( pFormat, fRadius, fHeight, nStacks, nSlices, bInverted, Origin, true, pResources ) == true )
            pResources->addMesh( &mMesh, pMesh, cgResourceFlags::ForceNew, _T("Core::Mesh::Capsule"), cgDebugSource() );
        
        // Did the box creation succeed?
        if ( mMesh.isValid() == false )
        {
            pMesh->deleteReference();
            return false;
        
        } // End if failed

    } // End if no mesh
    else
    {
        // Clear and re-create.
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createCapsule( pFormat, fRadius, fHeight, nStacks, nSlices, bInverted, Origin, true, pResources ) == false )
        {
            mMesh.close( true );
            return false;
        
        } // End if failed

    } // End if mesh exists
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MeshData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : createSphere()
/// <summary>
/// Simple utility function to create a sphere mesh of the specified size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::createSphere( cgFloat fRadius, cgUInt32 nStacks, cgUInt32 nSlices, bool bInverted, cgMeshCreateOrigin::Base Origin )
{
    // Size must not be entirely degenerate on more than one axis.
    // If it is, the welding process undertaken during 'EndPrepare()'
    // will collapse the entire mesh and remove all faces.
    if ( fabsf(fRadius) < CGE_EPSILON_1MM )
        return false;

    // Create new mesh data container if one does not already exist.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL )
    {
        if ( mWorld != CG_NULL )
            pMesh = new cgMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
        else
            pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Generate mesh data
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createSphere( pFormat, fRadius, nStacks, nSlices, bInverted, Origin, true, pResources ) == true )
            pResources->addMesh( &mMesh, pMesh, cgResourceFlags::ForceNew, _T("Core::Mesh::Sphere"), cgDebugSource() );
        
        // Did the box creation succeed?
        if ( mMesh.isValid() == false )
        {
            pMesh->deleteReference();
            return false;
        
        } // End if failed
    
    } // End if no mesh
    else
    {
        // Clear and re-create.
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createSphere( pFormat, fRadius, nStacks, nSlices, bInverted, Origin, true, pResources ) == false )
        {
            mMesh.close( true );
            return false;
        
        } // End if failed

    } // End if mesh exists
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MeshData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : createCone()
/// <summary>
/// Simple utility function to create a cone mesh of the specified size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::createCone( cgFloat fRadius, cgFloat fRadiusTip, cgFloat fHeight, cgUInt32 nStacks, cgUInt32 nSlices, bool bInverted, cgMeshCreateOrigin::Base Origin )
{
    // Size must not be entirely degenerate on more than one axis.
    // If it is, the welding process undertaken during 'EndPrepare()'
    // will collapse the entire mesh and remove all faces.
    if ( fabsf(fRadius) < CGE_EPSILON_1MM && fabsf(fRadiusTip) < CGE_EPSILON_1MM && fHeight < CGE_EPSILON_1MM )
        return false;

    // Create new mesh data container if one does not already exist.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL )
    {
        if ( mWorld != CG_NULL )
            pMesh = new cgMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
        else
            pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Generate mesh data
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createCone( pFormat, fRadius, fRadiusTip, fHeight, nStacks, nSlices, bInverted, Origin, true, pResources ) == true )
            pResources->addMesh( &mMesh, pMesh, cgResourceFlags::ForceNew, _T("Core::Mesh::Cone"), cgDebugSource() );
        
        // Did the box creation succeed?
        if ( mMesh.isValid() == false )
        {
            pMesh->deleteReference();
            return false;
        
        } // End if failed
    
    } // End if no mesh
    else
    {
        // Clear and re-create.
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createCone( pFormat, fRadius, fRadiusTip, fHeight, nStacks, nSlices, bInverted, Origin, true, pResources ) == false )
        {
            mMesh.close( true );
            return false;
        
        } // End if failed

    } // End if mesh exists
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MeshData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : createTorus()
/// <summary>
/// Simple utility function to create a cone mesh of the specified size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::createTorus( cgFloat fOuterRadius, cgFloat fInnerRadius, cgUInt32 nBands, cgUInt32 nSides, bool bInverted, cgMeshCreateOrigin::Base Origin )
{
    // Size must not be entirely degenerate on more than one axis.
    // If it is, the welding process undertaken during 'EndPrepare()'
    // will collapse the entire mesh and remove all faces.
    if ( fabsf(fOuterRadius) < CGE_EPSILON_1MM )
        return false;

    // Create new mesh data container if one does not already exist.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL )
    {
        if ( mWorld != CG_NULL )
            pMesh = new cgMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
        else
            pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Generate mesh data
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createTorus( pFormat, fOuterRadius, fInnerRadius, nBands, nSides, bInverted, Origin, true, pResources ) == true )
            pResources->addMesh( &mMesh, pMesh, cgResourceFlags::ForceNew, _T("Core::Mesh::Torus"), cgDebugSource() );
        
        // Did the box creation succeed?
        if ( mMesh.isValid() == false )
        {
            pMesh->deleteReference();
            return false;
        
        } // End if failed

    } // End if no mesh
    else
    {
        // Clear and re-create.
        cgResourceManager * pResources = cgResourceManager::getInstance();
        cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( pMesh->createTorus( pFormat, fOuterRadius, fInnerRadius, nBands, nSides, bInverted, Origin, true, pResources ) == false )
        {
            mMesh.close( true );
            return false;
        
        } // End if failed

    } // End if mesh exists
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MeshData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
    
    // Success!
    return true;
}


//-----------------------------------------------------------------------------
// Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, const cgVector3 & vWireTolerance, cgFloat & fDistance )
{
    // Retrieve the underlying mesh resource and pick if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL || pMesh->isLoaded() == false )
        return false;
    
    // Pass through
    return pMesh->pick( vOrigin, vDir, bWireframe, vWireTolerance, fDistance );
}

//-----------------------------------------------------------------------------
// Name : pickFace ( ) (Virtual)
/// <summary>
/// Similar to pick() but is used to simply determine which face was 
/// intersected and which material is assigned to it.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::pickFace( const cgVector3 & vOrigin, const cgVector3 & vDir, cgVector3 & vIntersection, cgUInt32 & nFace, cgMaterialHandle & hMaterial )
{
    // Retrieve the underlying mesh resource and pick if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL || pMesh->isLoaded() == false )
        return false;
    
    // Pass through
    return pMesh->pickFace( vOrigin, vDir, vIntersection, nFace, hMaterial );
}

//-----------------------------------------------------------------------------
// Name : setMeshMaterial() (Virtual)
/// <summary>
/// Apply the specified material to every face in the mesh, replacing that
/// previously assigned.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::setMeshMaterial( const cgMaterialHandle & hMaterial )
{
    // Pass through to mesh object if valid.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL || pMesh->isLoaded() == false )
        return false;

    // Pass through
    if ( pMesh->setMeshMaterial( hMaterial ) == true )
    {
        // Notify listeners that object data has changed.
        static const cgString strContext = _T("MeshData");
        onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
        return true;
    
    } // End if success
    return false;
}

//-----------------------------------------------------------------------------
// Name : setFaceMaterial() (Virtual)
/// <summary>
/// Apply the specified material to the relevant face, replacing that
/// previously assigned.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::setFaceMaterial( cgUInt32 nFace, const cgMaterialHandle & hMaterial )
{
    // Pass through to mesh object if valid.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() )
        return false;

    // Pass through
    if ( pMesh->setFaceMaterial( nFace, hMaterial ) )
    {
        // Notify listeners that object data has changed.
        static const cgString strContext = _T("MeshData");
        onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
        return true;
    
    } // End if success
    return false;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Render the world object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::render( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer )
{
    // Get access to required systems
    cgRenderDriver * pDriver = mWorld->getRenderDriver();

    // Retrieve the underlying mesh resource and render if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() )
        return false;

    // Signal a straight draw of the object
    pDriver->setConstantBufferAuto( pIssuer->getRenderTransformBuffer() );
    pMesh->setDefaultColor( pIssuer->getNodeColor() );
    pMesh->draw( );
    
    // Drawn
    return true;
}

//-----------------------------------------------------------------------------
// Name : renderSubset ()
/// <summary>
/// Render only the section of this object that relates to the specified 
/// material (used for material batched / sorted rendering).
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::renderSubset( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer, const cgMaterialHandle & hMaterial )
{
    // Get access to required systems
    cgRenderDriver * pDriver = mWorld->getRenderDriver();

    // Retrieve the underlying mesh resource and render if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() )
        return false;
    
    // Signal a straight draw of the object
    pDriver->setConstantBufferAuto( pIssuer->getRenderTransformBuffer() );
    cgToDo( "Effect Overhaul", "Commit changes removed. It /should/ no longer be required, but just confirm." )
    //pDriver->CommitChanges(); // Required for batching, effect pass already begun.
    pMesh->setDefaultColor( pIssuer->getNodeColor() );
    pMesh->drawSubset( hMaterial, cgMeshDrawMode::Simple );

    // Drawn
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // Sandbox render is only necessary in wireframe mode unless the
    // node is hidden. Standard scene rendering behavior applies in other modes.
    if ( pIssuer->isRenderable() && !(flags & cgSandboxRenderFlags::Wireframe) )
    {
        cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
        return;
    
    } // End if !wireframe

    // Get access to required systems.
    cgRenderDriver    * pDriver    = cgRenderDriver::getInstance();
    cgResourceManager * pResources = cgResourceManager::getInstance();

    // Retrieve the underlying mesh resource if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL || pMesh->isLoaded() == false )
    {
        cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
        return;
    
    } // End if no mesh

    // Setup constants.
    cgColorValue Color = (pIssuer->isSelected() == false) ? pIssuer->getNodeColor() : cgColorValue( 0xFFFFFFFF );
    cgConstantBuffer * pConstants = pDriver->getSandboxConstantBuffer().getResource( true );
    pConstants->setVector( _T("diffuseReflectance"), (cgVector4&)Color );
    pDriver->setConstantBufferAuto( pIssuer->getRenderTransformBuffer() );
    pDriver->setConstantBufferAuto( pDriver->getSandboxConstantBuffer() );
    
    // Execute technique.
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);
    if ( pShader->beginTechnique( _T("drawWireframeMesh") ) )
    {
        while ( pShader->executeTechniquePass( ) == cgTechniqueResult::Continue )
            pMesh->draw( cgMeshDrawMode::Simple );
        pShader->endTechnique();
    
    } // End if success

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshObject::applyObjectRescale( cgFloat fScale )
{
    // Retrieve the underlying mesh resource.
    cgMesh * pMesh = mMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() ) 
        return;

    // Pass on the scale request.
    pMesh->scaleMeshData( fScale );

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ApplyRescale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
// Name : getShadowStage()
/// <summary>
/// Retrieve the state that describes whether or not the object will cast
/// shadows onto surrounding objects, and during which lighting stage that
/// should occur (if any).
/// </summary>
//-----------------------------------------------------------------------------
cgSceneProcessStage::Base cgMeshObject::getShadowStage( ) const
{
    return mShadowStage;
}

//-----------------------------------------------------------------------------
// Name : setShadowStage()
/// <summary>
/// Set the state that describes whether or not the object will cast
/// shadows onto surrounding objects, and during which lighting stage that
/// should occur (if any).
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshObject::setShadowStage( cgSceneProcessStage::Base stage )
{
    // Is this a no-op?
    if ( mShadowStage == stage )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateProcessStages.bindParameter( 1, (cgInt32)0 );     // LightingStage
        mUpdateProcessStages.bindParameter( 2, (cgInt32)stage ); // ShadowCastStage
        mUpdateProcessStages.bindParameter( 3, (cgInt32)0 );     // ShadowReceiveStage
        mUpdateProcessStages.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( mUpdateProcessStages.step( true ) == false )
        {
            cgString strError;
            mUpdateProcessStages.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow rendering stage data for mesh object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mShadowStage = stage;

    // Notify listeners that property was altered
    static const cgString strContext = _T("ShadowStage");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgMeshObject::getLocalBoundingBox( )
{
    // Retrieve the underlying mesh resource (returns
    // degenerate local origin <0,0,0> if not loaded)
    cgMesh * pMesh = mMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() ) 
        return cgBoundingBox::Empty;

    // Retrieve the bounding box properties
    return pMesh->getBoundingBox( );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_MeshObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgMeshObject::getDatabaseTable( ) const
{
    return _T("Objects::Mesh");
}

//-----------------------------------------------------------------------------
//  Name : isRenderable ()
/// <summary>
/// Determine if this object is currently renderable or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::isRenderable() const
{
	// All meshes can render by default (assuming there is any data).
    return mMesh.isValid();
}

//-----------------------------------------------------------------------------
//  Name : isShadowCaster () (Virtual)
/// <summary>
/// Is the object capable of casting shadows? (i.e. a camera may not be)
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::isShadowCaster() const
{
	// Can this mesh cast shadows at runtime (assuming there is any data).
    return mMesh.isValid() && (mShadowStage == cgSceneProcessStage::Runtime || mShadowStage == cgSceneProcessStage::Both);
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("MeshObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertMesh.bindParameter( 1, mReferenceId );
        mInsertMesh.bindParameter( 2, mMesh.getReferenceId() );
        mInsertMesh.bindParameter( 3, (cgInt32)0 ); // LightingStage
        mInsertMesh.bindParameter( 4, (cgInt32)mShadowStage );
        mInsertMesh.bindParameter( 5, (cgInt32)0 ); // ShadowReceiveStage
        mInsertMesh.bindParameter( 6, mSoftRefCount );
        
        // Execute
        if ( !mInsertMesh.step( true ) )
        {
            cgString strError;
            mInsertMesh.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for mesh object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("MeshObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("MeshObject::insertComponentData") );

    } // End if !internal

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentLoading() (Virtual)
/// <summary>
/// Virtual method called when the component is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the mesh data.
    prepareQueries();
    mLoadMesh.bindParameter( 1, e->sourceRefId );
    if ( !mLoadMesh.step( ) || !mLoadMesh.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadMesh.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for mesh object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for mesh object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadMesh.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadMesh;

    // ToDo: 9999
    // Grab basic object properties.
    cgUInt32 nStage = 0;
    //m_oqLoadMesh->getColumn( L"LightStage", nStage );
    //m_Lighting = (LightingStage)nStage;
    mLoadMesh.getColumn( L"ShadowCastStage", nStage );
    mShadowStage = (cgSceneProcessStage::Base)nStage;
    //m_oqLoadMesh->getColumn( L"ShadowReceiveStage", nStage );
    //m_ReceiveShadows = (LightingStage)nStage;

    // Load the referenced mesh data source.
    cgUInt32 nMeshRefId = 0;
    mLoadMesh.getColumn( _T("DataSourceId"), nMeshRefId );
    if ( nMeshRefId != 0 )
    {
        // If we're cloning, we potentially need a new copy of the mesh data,
        // otherwise we can load it as a straight forward wrapped resource.
        cgUInt32 nFlags = 0;
        bool bMeshAsInternal = false;
        if ( e->cloneMethod == cgCloneMethod::Copy )
        {
            bMeshAsInternal = isInternalReference();
            nFlags = cgResourceFlags::ForceNew;
        
        } // End if copying
        
        // Load the mesh.
        cgMeshHandle hMesh;
        cgResourceManager * pResources = cgResourceManager::getInstance();
        if ( !pResources->loadMesh( &hMesh, mWorld, nMeshRefId, bMeshAsInternal, true, nFlags, cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to instantiate or load mesh data source '0x%x' for mesh object '0x%x'. Refer to any previous errors for more information.\n"), nMeshRefId, mReferenceId  );
            mLoadMesh.reset();
            return false;
        
        } // End if failed

        // If this is not a new mesh (i.e. we're somehow simply re-loading an existing mesh)
        // then just skip the resource handle swap to save a headache :)
        if ( hMesh != mMesh )
        {
            // We should now take ownership of this resource. First close
            // our existing mesh handle since we need to change the handle
            // options (see below) without interfering with the release.
            mMesh.close();
            
            // Since we're reloading prior information, the database ref count should
            // NOT be incremented when we attach (i.e. we're just reconnecting).
            mMesh.enableDatabaseUpdate( false );

            // Take ownership and then restore handle options.
            mMesh = hMesh;
            mMesh.enableDatabaseUpdate( (isInternalReference() == false) );

        } // End if different mesh

    } // End if valid identifier

    // Call base class implementation to read remaining data.
    if ( !cgWorldObject::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        if ( !insertComponentData() )
            return false;

    } // End if cloned

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshObject::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // Update world database
    if ( shouldSerialize() && (e->context == _T("MeshData") || e->context == _T("ApplyRescale")) )
    {
        prepareQueries();
        mUpdateMeshData.bindParameter( 1, mMesh.getReferenceId() );
        mUpdateMeshData.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdateMeshData.step( true ) == false )
        {
            cgString strError;
            mUpdateMeshData.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update data reference for mesh object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
        
        } // End if failed
    
    } // End if serialize

    // Call base class implementation
    cgWorldComponent::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshObject::onComponentDeleted( )
{
    // Remove our physical reference to any mesh data. Full database update 
    // and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    mMesh.close();
    
    // Call base class implementation last.
    cgWorldObject::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( mInsertMesh.isPrepared() == false )
            mInsertMesh.prepare( mWorld, _T("INSERT INTO 'Objects::Mesh' VALUES(?1,?2,?3,?4,?5,NULL)"), true );
        if ( mUpdateMeshData.isPrepared() == false )
            mUpdateMeshData.prepare( mWorld, _T("UPDATE 'Objects::Mesh' SET DataSourceId=?1 WHERE RefId=?2"), true );
        if ( mUpdateProcessStages.isPrepared() == false )
            mUpdateProcessStages.prepare( mWorld, _T("UPDATE 'Objects::Mesh' SET LightStage=?1, ShadowCastStage=?2, ShadowReceiveStage=?3 WHERE RefId=?4"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadMesh.isPrepared() == false )
        mLoadMesh.prepare( mWorld, _T("SELECT * FROM 'Objects::Mesh' WHERE RefId=?1"), true );
}

/*ToDo: Clean up
//-----------------------------------------------------------------------------
// Name : QueryCollisionGeometry () (Virtual)
// Desc : Allows other parts of the system to query this object for geometry
//        that can be used during the process of collision detection.
//-----------------------------------------------------------------------------
cgCollision::QueryResult cgMeshObject::QueryCollisionGeometry( const cgBoundingBox & Bounds, cgCollision::QueryData & Data )
{
    // Retrieve the underlying mesh resource if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL || pMesh->isLoaded() == false )
        return cgCollision::Query_None;

    // Retrieve mesh geometry data
    Data.pVertexFormat = pMesh->GetVertexFormat();
    Data.pVertices     = pMesh->GetSystemVB();
    Data.pIndices      = pMesh->GetSystemIB();
    Data.nTriCount     = pMesh->GetFaceCount();

    // Attempt to collide against all triangles by default
    return cgCollision::Query_All;
}

//-----------------------------------------------------------------------------
// Name : GetCollisionGeometry () (Virtual)
// Desc : Allows other parts of the system to retrieve the mesh representation
//        of this object that can be used during any collision detection phase.
//-----------------------------------------------------------------------------
bool cgMeshObject::GetCollisionGeometry( CollisionGeom & Data )
{
    // Retrieve the underlying mesh resource if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( pMesh == CG_NULL || pMesh->isLoaded() == false )
        return cgCollision::Query_None;

    // Retrieve mesh geometry data
    Data.pVertexFormat = pMesh->GetVertexFormat();
    Data.pVertices     = pMesh->GetSystemVB();
    Data.nVertexCount  = pMesh->GetVertexCount();
    Data.pIndices      = pMesh->GetSystemIB();
    Data.nTriCount     = pMesh->GetFaceCount();

    // Collision geometry available.
    return true;
}*/

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::getSubElementCategories( cgObjectSubElementCategory::Map & Categories ) const
{
    // Call base class implementation to populate base elements.
    cgWorldObject::getSubElementCategories( Categories );

    // Mesh objects also add a convex hull shape type.
    cgObjectSubElementCategory & Category = Categories[ OSECID_CollisionShapes ];
    {
        cgObjectSubElementDesc & Type = Category.supportedTypes[ RTID_HullCollisionShapeElement ];
        Type.identifier = RTID_HullCollisionShapeElement;
        Type.name       = _T("Convex Hull");
    
    } // End Convex Hull
        
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : supportsSubElement () (Virtual)
/// <summary>
/// Determine if the specified object sub element type is supported by this
/// world object. Derived object types should implement this to extend the
/// allowable sub element types.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshObject::supportsSubElement( const cgUID & Category, const cgUID & Identifier ) const
{
    // Call base class implementation first.
    if ( cgWorldObject::supportsSubElement( Category, Identifier ) )
        return true;

    // Validate supported categories.
    if ( Category == OSECID_CollisionShapes )
    {
        // Validate supported types.
        if ( Identifier == RTID_HullCollisionShapeElement )
             return true;

    } // End OSECID_AnimationSets
    
    // Unsupported
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// cgMeshNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgMeshNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshNode::cgMeshNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    mWorldSize = 0.0f;

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Mesh%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgMeshNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshNode::cgMeshNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Recompute world size
    mWorldSize = 0.0f;
    computeWorldSize();
}

//-----------------------------------------------------------------------------
//  Name : ~cgMeshNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshNode::~cgMeshNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgMeshNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgMeshNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgMeshNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgMeshNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_MeshNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : computeWorldSize ( ) (Protected)
/// <summary>
/// Recompute the approximate world space size of this node based on the
/// internal mesh data. This size is used for processes such as distanced based
/// visibility culling and LOD (see cgObjectNode::getObjectSize())
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshNode::computeWorldSize()
{
    cgBoundingBox Bounds = getLocalBoundingBox( );
    if ( Bounds.isPopulated()  == true )
    {
        // We'll take the size as the diagonal.
        cgVector3 vSize = Bounds.getDimensions();

        // Scale up / down as necessary based on the node scale.
        cgVector3 vScale = getScale();
        vSize.x *= vScale.x;
        vSize.y *= vScale.y;
        vSize.z *= vScale.z;

        // Set the final size.
        mWorldSize = cgVector3::length(vSize);
        return;
    
    } // End if valid

    // Auto cull
    mWorldSize = 0.0f; 
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // What was modified?
    if ( e->context == _T("MeshData") || e->context == _T("ApplyRescale") )
    {
        // Recompute world size for auto distance culling.
        computeWorldSize();

        // Re-build the physics body if necessary.
        if ( mPhysicsModel == cgPhysicsModel::CollisionOnly )
            buildPhysicsBody();

        // Mesh node is now 'dirty' and should always 
        // re-trigger a shadow map fill during the next render
        // process if necessary.
        nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );

    } // End if MeshData | ApplyRescale
    else if ( e->context == _T("ShadowStage") )
    {
        // Mesh node is now 'dirty' and should re-trigger a shadow map fill 
        // during the next render process if necessary.
        nodeUpdated( 0, 0 );
    
    } // End if ShadowStage

    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : onNodeLoading ( ) (Virtual)
/// <summary>
/// Virtual method called when the node is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshNode::onNodeLoading( const cgUID & ObjectType, cgWorldQuery * pNodeData, cgSceneCell * pParentCell, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeLoading( ObjectType, pNodeData, pParentCell, CloneMethod ) )
        return false;

    // Compute initial world size for auto distance culling.
    computeWorldSize();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : registerVisibility () (Virtual)
/// <summary>
/// This node has been deemed visible during testing, but this method gives the
/// node a final say on how it gets registered with the visibility set. The
/// default behavior is simply to insert directly into object visibility list,
/// paying close attention to filtering rules.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshNode::registerVisibility( cgVisibilitySet * pSet, cgUInt32 nFlags )
{
    // Allow base class to perform basic tests against filters and
    // add itself to the list of visible objects where necessary.
    if ( !cgObjectNode::registerVisibility( pSet, nFlags ) )
        return false;

    // Since this node type supports material based subset rendering,
    // we should also register ourselves with the visible material list
    // in the visibility set. First get the list of materials used by
    // the referenced mesh object.
    if ( nFlags & cgVisibilitySearchFlags::CollectMaterials )
    {
        cgMesh * pMesh = (cgMesh*)getMesh().getResource(false);
        if ( pMesh )
        {
            cgMaterialHandleArray & aMaterials = pMesh->getMaterials();

            // Register each of these materials with the visibility set.
            for ( size_t i = 0; i < aMaterials.size(); ++i )
                pSet->addVisibleMaterial( aMaterials[i], this );
        
        } // End if valid / loaded

    } // End if materials required
    
    // We modified the visibility set.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform()
/// <summary>
/// Update our internal cell matrix with that specified here. Optionally, you
/// can disable the recomputation of the local (parent relative) transformation
/// matrix (defaults to true). This is generally only used internally in order
/// to reduce introducing accumulated errors into the transformation matrix but
/// can be supplied if you know that this does not need to be performed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation first. This ensures that
    // all of our internal data is up-to-date.
    if ( !cgObjectNode::setCellTransform( Transform, Source ) )
        return false;

    // If physics body update was requested, and we're in collision only
    // mode, we must take special action to apply any scale / shear to the
    // user mesh collision shape (since the physics engine will not consider
    // it in this 'non-convex' case).
    if ( mPhysicsBody && (Source != cgTransformSource::Dynamics) && (mPhysicsModel == cgPhysicsModel::CollisionOnly) )
    {
        // Has a "user mesh" shape been applied to the rigid body?
        // (This is not guaranteed. If the mesh data was invalid or not yet loaded 
        // it will currently have a 'Null' shape assigned).
        cgPhysicsShape * pShape = mPhysicsBody->getShape();
        if ( pShape->queryReferenceType( RTID_MeshShape ) )
        {
            // Extract the scale and shear from the current node cell transform.
            // This will be used as the 'offset' transform for the collision
            // shape, with the position and orientation applied to the body itself.
            cgTransform OffsetTransform;
            cgQuaternion qRotation;
            cgVector3 vScale, vShear, vTranslation;
            cgTransform::decompose( vScale, vShear, qRotation, vTranslation, getWorldTransform(false) );
            OffsetTransform.compose( vScale, vShear, cgQuaternion(0,0,0,1), cgVector3(0,0,0) );

            //// Adjust its offset transform.
            ((cgMeshShape*)pShape)->setOffsetTransform( OffsetTransform );

        } // End if valid mesh shape
            
    } // End if collision only

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onResolvePendingUpdates () (Virtual)
/// <summary>
/// Triggered when any pending deferred updates need to be resolved. Allows 
/// derived classes to take additional action during this phase.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshNode::onResolvePendingUpdates( cgUInt32 nUpdates )
{
    // Call base class implementation first.
    cgObjectNode::onResolvePendingUpdates( nUpdates );

    // Recompute our world size based on the mesh data bounding box
    if ( nUpdates & cgDeferredUpdateFlags::BoundingBox )
        computeWorldSize();
}

//-----------------------------------------------------------------------------
// Name : getObjectSize ( ) (Virtual )
/// <summary>
/// Retrieve the approximate world space size of this node, used for certain
/// LOD computations that may be applied. If the size is not / cannot be known
/// return FLT_MAX to effectively disable culling.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgMeshNode::getObjectSize( )
{
    // Make sure that the world size is up-to-date before we return it.
    resolvePendingUpdates( cgDeferredUpdateFlags::BoundingBox );

    // Return size.
    return mWorldSize;
}

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshNode::getSubElementCategories( cgObjectSubElementCategory::Map & categoriesOut ) const
{
    // Call base class implementation first (passes through to referenced object).
    if ( !cgObjectNode::getSubElementCategories( categoriesOut ) )
        return false;

    // For meshes, the 'CollisionOnly' model uses an automatic collision mesh
    // based on the renderable geometry. As a result, no collision shape editing 
    // should be made available in this mode (shapes support is already disabled
    // by the base class when model is set to 'None').
    if ( mPhysicsModel == cgPhysicsModel::CollisionOnly )
    {
        cgObjectSubElementCategory::Map::iterator itCategory = categoriesOut.find( OSECID_CollisionShapes );
        categoriesOut.erase( itCategory );
    
    } // End if non-dynamics model

    // Any sub-elements remaining?
    return (!categoriesOut.empty());
}

//-----------------------------------------------------------------------------
//  Name : supportsSubElement () (Virtual)
/// <summary>
/// Determine if the specified object sub element type is supported by this
/// world object. Derived object types should implement this to extend the
/// allowable sub element types.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshNode::supportsSubElement( const cgUID & Category, const cgUID & Identifier ) const
{
    // For meshes, the 'CollisionOnly' model uses an automatic collision mesh
    // based on the renderable geometry. As a result, no collision shape editing
    // should be made available in this mode (shapes support is already disabled 
    // by the base class when model is set to 'None')
    if ( mPhysicsModel == cgPhysicsModel::CollisionOnly && Category == OSECID_CollisionShapes )
        return false;

    // Call base class implementation last.
    return cgObjectNode::supportsSubElement( Category, Identifier );
}

//-----------------------------------------------------------------------------
// Name : buildPhysicsBody ( ) (Virtual, Protected)
/// <summary>
/// Construct the internal physics body designed to represent this node.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshNode::buildPhysicsBody( )
{
    cgToDo( "Physics", "When a body is reconstructed because the shape was modified, any child joint is not notified and thus the constraints begin to fail." );

    // Override 'CollisionOnly' case for meshes.
    if ( mPhysicsModel == cgPhysicsModel::CollisionOnly && getMesh().isValid() )
    {
        // Extract the scale and shear from the current node cell transform.
        // This will be used as the 'offset' transform for the collision
        // shape, with the position and orientation applied to the body itself.
        cgTransform OffsetTransform;
        cgQuaternion qRotation;
        cgVector3 vScale, vShear, vTranslation;
        cgTransform::decompose( vScale, vShear, qRotation, vTranslation, getWorldTransform(false) );
        OffsetTransform.compose( vScale, vShear, cgQuaternion(0,0,0,1), cgVector3(0,0,0) );
        
        // Create a new "user mesh" physics shape to represent this mesh.
        cgMeshShape * pShape = new cgMeshShape( mParentScene->getPhysicsWorld(), getMesh(), OffsetTransform );

        /*// Does a shape with these properties already exist?
        cgPhysicsWorld * pPhysicsWorld = mParentScene->getPhysicsWorld();
        cgMeshShape * pExistingShape = (cgMeshShape*)pPhysicsWorld->GetExistingShape( cgPhysicsShapeCacheKey( pShape ) );
        if ( pExistingShape )
        {
            delete pShape;
            pShape = pExistingShape;

        } // End if existing shape*/

        // Configure new rigid body.
        cgRigidBody::ConstructData cd;
        cd.model            = mPhysicsModel;
        cd.initialTransform = getWorldTransform(false);
        cd.quality          = cgSimulationQuality::Default;
        cd.mass             = 0.0f;

        // Construct a new rigid body object.
        cgRigidBody * pRigidBody = new cgRigidBody( mParentScene->getPhysicsWorld(), pShape, cd );

        // Release the previous physics body (if any).
        if ( mPhysicsBody )
        {
            mPhysicsBody->unregisterEventListener( (cgPhysicsBodyEventListener*)this );
            mPhysicsBody->removeReference( this );
        
        } // End if exists

        // Take ownership of the new rigid body.
        mPhysicsBody = pRigidBody;
        mPhysicsBody->addReference( this );

        // Listen for events
        mPhysicsBody->registerEventListener( (cgPhysicsBodyEventListener*)this );

    } // End if mesh collision
    else
    {
        // Fall through to base class implementation
        cgObjectNode::buildPhysicsBody( );

    } // End if other model

}