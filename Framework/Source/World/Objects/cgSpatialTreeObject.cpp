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
// Name : cgSpatialTreeObject.cpp                                            //
//                                                                           //
// Desc : Base world object and scene node types designed for storage and    //
//        rendering of data organized using a spatial tree structure.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSpatialTreeObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgSpatialTreeObject.h>

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeObject() (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeObject::cgSpatialTreeObject( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld ), cgSpatialTree()
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeObject::cgSpatialTreeObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod ), cgSpatialTree( (cgSpatialTree*)pInit )
{
    // Initialize variables to sensible defaults.
}

//-----------------------------------------------------------------------------
//  Name : ~cgSpatialTreeObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeObject::~cgSpatialTreeObject()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgSpatialTreeObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Call base class implementation
    if ( bDisposeBase == true )
    {
        cgWorldObject::dispose( true );
        cgSpatialTree::dispose( true );
    
    } // End if dispose base
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
bool cgSpatialTreeObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SpatialTreeObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // ToDo: 9999
    /*// Insert the new object.
    if ( ShouldSerialize() == true )
    {
        PrepareQueries();
        m_qInsertBaseLight.BindParameter( 1, m_nReferenceId );
        m_qInsertBaseLight.BindParameter( 2, m_DiffuseColor.r );
        m_qInsertBaseLight.BindParameter( 3, m_DiffuseColor.g );
        m_qInsertBaseLight.BindParameter( 4, m_DiffuseColor.b );
        m_qInsertBaseLight.BindParameter( 5, m_fDiffuseHDRScale );
        m_qInsertBaseLight.BindParameter( 6, m_SpecularColor.r );
        m_qInsertBaseLight.BindParameter( 7, m_SpecularColor.g );
        m_qInsertBaseLight.BindParameter( 8, m_SpecularColor.b );
        m_qInsertBaseLight.BindParameter( 9, m_fSpecularHDRScale );
        m_qInsertBaseLight.BindParameter( 10, m_AmbientColor.r );
        m_qInsertBaseLight.BindParameter( 11, m_AmbientColor.g );
        m_qInsertBaseLight.BindParameter( 12, m_AmbientColor.b );
        m_qInsertBaseLight.BindParameter( 13, m_fAmbientHDRScale );
        m_qInsertBaseLight.BindParameter( 14, (cgInt32)m_LightingStage );
        m_qInsertBaseLight.BindParameter( 15, (cgInt32)m_ShadowStage );
        m_qInsertBaseLight.BindParameter( 16, m_fSpecularMinDistance );
        m_qInsertBaseLight.BindParameter( 17, m_fSpecularMaxDistance );
        m_qInsertBaseLight.BindParameter( 18, m_fShadowMinDistance );
        m_qInsertBaseLight.BindParameter( 19, m_fShadowMaxDistance );
        m_qInsertBaseLight.BindParameter( 20, m_fShadowLODMinDistance );
        m_qInsertBaseLight.BindParameter( 21, m_fShadowLODMaxDistance );

        // ToDo: 9999 - Scale/Bias for N.L and N.H (m_CosineAdjust)
        
        // Execute
        if ( m_qInsertBaseLight.Step( true ) == false )
        {
            cgString strError;
            m_qInsertBaseLight.GetLastError( strError );
            cgAppLog::Write( cgAppLog::Error, _T("Failed to insert light object '0x%x' base data into database. Error: %s\n"), m_nReferenceId, strError.c_str() );
            return false;
        
        } // End if failed

    } // End if !internal

    // Load the core effect file.
    cgResourceManager * pResources = m_pWorld->GetResourceManager();
    if ( pResources->LoadEffectFile( &m_hEffect, _T("sys://Shaders/LightSource.sh"), 0, cgDebugSource() ) == false )
    {
        cgAppLog::Write( cgAppLog::Error, _T("Failed to load/compile light source file while creating object 0x%x.\n"), m_nReferenceId );
        return false;
    
    } // End if failed

    // Create attenuation samplers if they are not already allocated.
    if ( m_pAttenDistSampler == CG_NULL )
        m_pAttenDistSampler = pResources->CreateSampler( _T("LightDistanceAtten"), m_hEffect );
    if ( m_pAttenMaskSampler == CG_NULL )
        m_pAttenMaskSampler = pResources->CreateSampler( _T("LightAtten"), m_hEffect );*/

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
//  Name : onComponentLoading() (Virtual)
/// <summary>
/// Virtual method called when the component is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // ToDo: 9999
    /*// Load the base light data.
    PrepareQueries();
    m_qLoadBaseLight.BindParameter( 1, e->nSourceRefId );
    if ( m_qLoadBaseLight.Step( ) == false || m_qLoadBaseLight.NextRow() == false )
    {
        // Log any error.
        cgString strError;
        if ( m_qLoadBaseLight.GetLastError( strError ) == false )
            cgAppLog::Write( cgAppLog::Error, _T("Failed to retrieve base data for light object '0x%x'. World database has potentially become corrupt.\n"), m_nReferenceId );
        else
            cgAppLog::Write( cgAppLog::Error, _T("Failed to retrieve base data for light object '0x%x'. Error: %s\n"), m_nReferenceId, strError.c_str() );

        // Release any pending read operation.
        m_qLoadBaseLight.Reset();
        return false;
    
    } // End if failed
    
    // Update our local members
    m_qLoadBaseLight.getColumn( _T("DiffuseR"), m_DiffuseColor.r );
    m_qLoadBaseLight.getColumn( _T("DiffuseG"), m_DiffuseColor.g );
    m_qLoadBaseLight.getColumn( _T("DiffuseB"), m_DiffuseColor.b );
    m_qLoadBaseLight.getColumn( _T("DiffuseHDRScalar"), m_fDiffuseHDRScale );
    m_qLoadBaseLight.getColumn( _T("SpecularR"), m_SpecularColor.r );
    m_qLoadBaseLight.getColumn( _T("SpecularG"), m_SpecularColor.g );
    m_qLoadBaseLight.getColumn( _T("SpecularB"), m_SpecularColor.b );
    m_qLoadBaseLight.getColumn( _T("SpecularHDRScalar"), m_fSpecularHDRScale );
    m_qLoadBaseLight.getColumn( _T("AmbientR"), m_AmbientColor.r );
    m_qLoadBaseLight.getColumn( _T("AmbientG"), m_AmbientColor.g );
    m_qLoadBaseLight.getColumn( _T("AmbientB"), m_AmbientColor.b );
    m_qLoadBaseLight.getColumn( _T("AmbientHDRScalar"), m_fAmbientHDRScale );
    m_qLoadBaseLight.getColumn( _T("LightStage"), (cgInt32&)m_LightingStage );
    m_qLoadBaseLight.getColumn( _T("ShadowCastStage"), (cgInt32&)m_ShadowStage );
    m_qLoadBaseLight.getColumn( _T("SpecularAttenuateBegin"), m_fSpecularMinDistance );
    m_qLoadBaseLight.getColumn( _T("SpecularAttenuateEnd"), m_fSpecularMaxDistance );
    m_qLoadBaseLight.getColumn( _T("ShadowAttenuateBegin"), m_fShadowMinDistance );
    m_qLoadBaseLight.getColumn( _T("ShadowAttenuateEnd"), m_fShadowMaxDistance );
    m_qLoadBaseLight.getColumn( _T("ShadowLODBegin"), m_fShadowLODMinDistance );
    m_qLoadBaseLight.getColumn( _T("ShadowLODEnd"), m_fShadowLODMaxDistance );
    m_qLoadBaseLight.reset();

    // Load the core effect file.
    cgResourceManager * pResources = m_pWorld->GetResourceManager();
    if ( pResources->LoadEffectFile( &m_hEffect, _T("sys://Shaders/LightSource.sh"), 0, cgDebugSource() ) == false )
    {
        cgAppLog::Write( cgAppLog::Error, _T("Failed to load/compile light source file while creating object 0x%x.\n"), m_nReferenceId );
        return false;
    
    } // End if failed

    // Create attenuation samplers if they are not already allocated.
    if ( m_pAttenDistSampler == CG_NULL )
        m_pAttenDistSampler = pResources->CreateSampler( _T("LightDistanceAtten"), m_hEffect );
    if ( m_pAttenMaskSampler == CG_NULL )
        m_pAttenMaskSampler = pResources->CreateSampler( _T("LightAtten"), m_hEffect );

    // Should we enable diffuse / specular?
    m_bDiffuseSource  = ( m_DiffuseColor.r > 1e-3f || m_DiffuseColor.g > 1e-3f || m_DiffuseColor.b > 1e-3f );
    m_bSpecularSource = ( m_SpecularColor.r > 1e-3f || m_SpecularColor.g > 1e-3f || m_SpecularColor.b > 1e-3f );*/

    // Call base class implementation to read remaining data.
    if ( !cgWorldObject::onComponentLoading( e ) )
        return false;

    /*// If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( m_nReferenceId != e->nSourceRefId )
    {
        if ( !InsertComponentData() )
            return false;

    } // End if cloned*/

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createTypeTables() (Virtual)
/// <summary>
/// When this type of component is being inserted into the database for the
/// first time, this method will be called in order to allow it to create any 
/// necessary tables.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeObject::createTypeTables( const cgUID & TypeIdentifier )
{
    // ToDo: 9999
    // Ensure this base class table is created first.
    if ( cgWorldObject::createTypeTables( RTID_SpatialTreeObject ) == false )
        return false;

    // Then allow derived type to create as normal.
    return cgWorldObject::createTypeTables( TypeIdentifier );
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Render the object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeObject::render( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer )
{
    // ToDo: 9999 - Fix me!
    /*// Bail if not yet processed
    if ( m_hGeometry.IsValid() == false )
        return;

    // ToDo: The referenced DrawSubset method needs to be implemented fully.
    bUseVisibility = false;

    // Retrieve the underlying mesh resource and render if available
    cgMesh * pMesh = m_hGeometry;
    if ( pMesh->IsLoaded() == false )
        return;

    // Using visibility data?
    if ( bUseVisibility == false )
        pMesh->Draw( );
    else
        pMesh->DrawSubset( m_AppliedDataGroups );*/

    // Drawn
    return true;
}

//-----------------------------------------------------------------------------
//  Name : renderSubset ()
/// <summary>
/// Render only the section of this object that relates to the specified
/// material (used for material batched / sorted rendering).
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeObject::renderSubset( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer, const cgMaterialHandle & hMaterial )
{
    // ToDo: 9999 - Fix me!
    /*// Bail if not yet processed
    if ( m_hGeometry.IsValid() == false )
        return;

    // Retrieve the underlying mesh resource and render if available
    cgMesh * pMesh = m_hGeometry;
    if ( pMesh->IsLoaded() == false )
        return;

    // Using visibility data?
    if ( bUseVisibility == false )
        pMesh->DrawSubset( hMaterial, Mode, PassCount );
    else
        pMesh->DrawSubset( hMaterial, m_AppliedDataGroups, Mode, PassCount );*/

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
void cgSpatialTreeObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // ToDo: 9999
    /*// Get access to required systems.
    cgRenderDriver    * pDriver    = cgRenderDriver::GetInstance();
    cgResourceManager * pResources = cgResourceManager::GetInstance();

    // Retrieve the underlying mesh resource if available
    cgMesh * pMesh = m_hMesh;
    if ( pMesh == CG_NULL || pMesh->IsLoaded() == false )
    {
        cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
        return;
    } // End if no mesh

    // Load the sandbox rendering shader if it is not already loaded.
    if ( m_hSandboxEffect.IsValid() == false )
        pResources->LoadEffectFile( &m_hSandboxEffect, _T("sys://Shaders/SandboxElements.sh"), 0, cgDebugSource() );
    
    // Use wireframe drawing for every subset using mesh color
    cgEffectFile * pEffect = m_hSandboxEffect;
    if ( pEffect == CG_NULL || pEffect->IsLoaded() == false )
    {
        cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
        return;

    } // End if no effect
    pEffect->SetTechnique( _T("drawWireframeMesh") );

    // Select the color to render with
    cgColorValue Color = (pIssuer->IsSelected() == false) ? pIssuer->GetNodeColor() : cgColorValue( 0xFFFFFFFF );
    pEffect->SetValue( _T("DiffuseReflectance"), &Color, sizeof(cgColorValue) );

    // Draw the entire mesh
    pDriver->FlushMaterials();
    pDriver->SetEffect( m_hSandboxEffect );
    pDriver->SetWorldTransform( &pIssuer->GetWorldTransform( false ) );
    pMesh->Draw( cgMeshDrawMode::EffectPasses );
    
    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
    */
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeObject::applyObjectRescale( cgFloat fScale )
{
    cgToDoAssert( "Carbon General", "cgSpatialTreeObject::applyObjectRescale()" );

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox () (Virtual)
/// <summary>
/// Retrieve the axis aligned bounding box for this node (encompassing
/// its referenced object) as it exists in the object's local space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgSpatialTreeObject::getLocalBoundingBox( )
{
    // By default, the local space bounding box is degenerate but positioned 
    // at the object's 'local' origin (always <0,0,0> in object space).
    if ( !mRootNode )
        return cgBoundingBox::Empty;

    // Retrieve the root node bounding box.
    return mRootNode->getBoundingBox();
}

///////////////////////////////////////////////////////////////////////////////
// cgSpatialTreeNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeNode::cgSpatialTreeNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene ), cgSpatialTreeInstance()
{
    // Initialize variables to sensible defaults.

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("SpatialTree%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeNode::cgSpatialTreeNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform ), cgSpatialTreeInstance( (cgSpatialTreeInstance*)pInit )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : ~cgSpatialTreeNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeNode::~cgSpatialTreeNode()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgSpatialTreeNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Call base class implementation
    if ( bDisposeBase == true )
    {
        cgObjectNode::dispose( true );
        cgSpatialTreeInstance::dispose( true );
    
    } // End if clear base
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
bool cgSpatialTreeNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SpatialTreeNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeNode::onNodeCreated( const cgUID & ObjectType, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeCreated( ObjectType, CloneMethod ) )
        return false;

    // Update 'cgSpatialTreeInstance' base class with the pointer
    // to the created tree object.
    mTree = (cgSpatialTree*)mReferencedObject;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeLoading ( ) (Virtual)
/// <summary>
/// Virtual method called when the node is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeNode::onNodeLoading( const cgUID & ObjectType, cgWorldQuery * pNodeData, cgSceneCell * pParentCell, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeLoading( ObjectType, pNodeData, pParentCell, CloneMethod ) )
        return false;

    // Update 'cgSpatialTreeInstance' base class with the pointer
    // to the created tree object.
    mTree = (cgSpatialTree*)mReferencedObject;

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
void cgSpatialTreeNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    /*// What was modified?
    if ( e->strContext == _T("MeshData") )
    {
        // Recompute world size for auto distance culling.
        ComputeWorldSize();

    } // End if MeshData

    // Mesh node is now 'dirty' and should always 
    // re-trigger a shadow map fill during the next render
    // process if necessary.
    NodeUpdated();*/

    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
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
bool cgSpatialTreeNode::registerVisibility( cgVisibilitySet * pSet, cgUInt32 nFlags )
{
    // Allow base class to perform basic tests against filters and
    // add itself to the list of visible objects where necessary.
    // This includes registering active materials if there is
    // any currently active spatial tree geometry.
    if ( !cgObjectNode::registerVisibility( pSet, nFlags ) )
        return false;

    // We modified the visibility set.
    return true;
}