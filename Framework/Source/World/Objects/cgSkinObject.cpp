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
// Name : cgSkinObject.cpp                                                   //
//                                                                           //
// Desc : Contains managed mesh and associated classes used to maintain      //
//        skinned mesh data.                                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSkinObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgSkinObject.h>
#include <World/Objects/cgBoneObject.h>
#include <World/cgScene.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgMesh.h>
#include <Rendering/cgRenderDriver.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgSkinObject::mInsertSkin;
cgWorldQuery cgSkinObject::mUpdateMeshData;
cgWorldQuery cgSkinObject::mUpdateProcessStages;
cgWorldQuery cgSkinObject::mLoadSkin;
cgWorldQuery cgSkinNode::mInsertAttachedBone;
cgWorldQuery cgSkinNode::mDeleteAttachedBones;
cgWorldQuery cgSkinNode::mLoadAttachedBones;

///////////////////////////////////////////////////////////////////////////////
// cgSkinObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSkinObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinObject::cgSkinObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgMeshObject( nReferenceId, pWorld )
{
}

//-----------------------------------------------------------------------------
//  Name : cgSkinObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinObject::cgSkinObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgMeshObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgSkinObject * pObject = (cgSkinObject*)pInit;
    // ToDo: 9999 - Clone
    /*m_Lighting              = pObject->m_Lighting;
    m_CastShadows           = pObject->m_CastShadows;
    m_ReceiveShadows        = pObject->m_ReceiveShadows;*/    
}

//-----------------------------------------------------------------------------
//  Name : ~cgSkinObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinObject::~cgSkinObject()
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
void cgSkinObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Call base class implementation
    if ( bDisposeBase == true )
        cgMeshObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgSkinObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgSkinObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgSkinObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgSkinObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SkinObject )
        return true;

    // Supported by base?
    return cgMeshObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgSkinObject::getDatabaseTable( ) const
{
    return _T("Objects::Skin");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last. Skip straight over the mesh
    // object base class to the world object. This is not a bug.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("SkinObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertSkin.bindParameter( 1, mReferenceId );
        mInsertSkin.bindParameter( 2, mMesh.getReferenceId() );
        mInsertSkin.bindParameter( 3, (cgInt32)0 ); // m_Lighting
        mInsertSkin.bindParameter( 4, (cgInt32)0 ); // m_CastShadows
        mInsertSkin.bindParameter( 5, (cgInt32)0 ); // m_ReceiveShadows
        mInsertSkin.bindParameter( 6, mSoftRefCount );
        
        // Execute
        if ( !mInsertSkin.step( true ) )
        {
            cgString strError;
            mInsertSkin.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for skin object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("SkinObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("SkinObject::insertComponentData") );

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
bool cgSkinObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the mesh data.
    prepareQueries();
    mLoadSkin.bindParameter( 1, e->sourceRefId );
    if ( !mLoadSkin.step( ) || !mLoadSkin.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadSkin.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for skin object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for skin object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadSkin.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadSkin;

    // ToDo: 9999
    /*// Grab basic object properties.
    cgUInt32 nValue = 0;
    m_oqLoadMesh->getColumn( L"LightStage", nValue );
    m_Lighting = (LightingStage)nValue;
    m_oqLoadMesh->getColumn( L"ShadowCastStage", nValue );
    m_CastShadows = (LightingStage)nValue;
    m_oqLoadMesh->getColumn( L"ShadowReceiveStage", nValue );
    m_ReceiveShadows = (LightingStage)nValue;*/

    // Load the referenced mesh data source.
    cgUInt32 nMeshRefId = 0;
    mLoadSkin.getColumn( _T("DataSourceId"), nMeshRefId );
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
            cgAppLog::write( cgAppLog::Error, _T("Failed to instantiate or load mesh data source '0x%x' for skin object '0x%x'. Refer to any previous errors for more information.\n"), nMeshRefId, mReferenceId  );
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

    // Call base class implementation to read remaining data. Skip straight 
    // over the mesh object base class to the world object. This is not a bug.
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
void cgSkinObject::onComponentModified( cgComponentModifiedEventArgs * e )
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
            cgAppLog::write( cgAppLog::Error, _T("Failed to update data reference for skin object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
        
        } // End if failed
    
    } // End if serialize

    // Call base class implementation last. Skip straight over the mesh
    // object base class to the world object. This is not a bug.
    cgWorldComponent::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinObject::onComponentDeleted( )
{
    // Remove our physical reference to any mesh data. Full database update 
    // and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    mMesh.close();
    
    // Call base class implementation last. Skip straight over the mesh
    // object base class to the world object. This is not a bug.
    cgWorldObject::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( mInsertSkin.isPrepared() == false )
            mInsertSkin.prepare( mWorld, _T("INSERT INTO 'Objects::Skin' VALUES(?1,?2,?3,?4,?5,NULL)"), true );
        if ( mUpdateMeshData.isPrepared() == false )
            mUpdateMeshData.prepare( mWorld, _T("UPDATE 'Objects::Skin' SET DataSourceId=?1 WHERE RefId=?2"), true );
        if ( mUpdateProcessStages.isPrepared() == false )
            mUpdateProcessStages.prepare( mWorld, _T("UPDATE 'Objects::Skin' SET LightStage=?1, ShadowCastStage=?2, ShadowReceiveStage=?3 WHERE RefId=?4"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadSkin.isPrepared() == false )
        mLoadSkin.prepare( mWorld, _T("SELECT * FROM 'Objects::Skin' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinObject::getSubElementCategories( cgObjectSubElementCategory::Map & Categories ) const
{
    // Call base class implementation to populate base elements.
    // Skip straight over the mesh object base class to the world object. 
    // This is not a bug.
    cgWorldObject::getSubElementCategories( Categories );

    // Skinned meshes cannot have collision shapes at all.
    Categories.erase( OSECID_CollisionShapes );
        
    // Any categories?
    return !Categories.empty();
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Render the world object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinObject::render( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer )
{
    // Get access to required systems
    cgRenderDriver * pDriver = mWorld->getRenderDriver();

    // Retrieve the underlying mesh resource and render if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() )
        return false;

    // Has skinning data?
    cgSkinBindData * pBindData = pMesh->getSkinBindData();
    if ( !pBindData )
        return false;

    /*// Signal a straight draw of the object
    pDriver->setWorldTransform( pIssuer->getWorldTransform( false ) );
    pMesh->setDefaultColor( pIssuer->getNodeColor() );
    pMesh->Draw( );*/

    // Begin to draw the relevant subset of this object
    pDriver->setWorldTransform( CG_NULL );
    pMesh->setDefaultColor( pIssuer->getNodeColor() );

    // Build an array containing all of the bones that are required
    // by the binding data in the skinned mesh.
    const cgSkinBindData::BoneArray & BindList = pBindData->getBones();
    const cgSkinNode::BoneInstanceMap & BoneNodes = ((cgSkinNode*)pIssuer)->mBoneInstanceIdentifiers;
    cgObjectNodeArray aBones( BindList.size(), CG_NULL );
    for ( size_t i = 0; i < BindList.size(); ++i )
    {
        // Get the requested bone.
        cgSkinNode::BoneInstanceMap::const_iterator itBone = BoneNodes.find( BindList[i]->boneIdentifier );
        if ( itBone == BoneNodes.end() )
            continue;

        // Assign the bone with this identifier.
        cgBoneNode * pBone = (cgBoneNode*)cgReferenceManager::getReference( itBone->second );
        aBones[i] = pBone;
        
    } // Next Influence

    /*// Process each palette in the skin with a matching attribute.
    cgMaterialHandle hMaterial, hPrevMaterial;
    const cgMesh::BonePaletteArray & Palettes = pMesh->getBonePalettes();
    for ( size_t i = 0; i < Palettes.size(); ++i )
    {
        cgBonePalette * pPalette = Palettes[i];
        
        // Apply the bone palette.
        pPalette->apply( aBones, pMesh->getSkinBindData(), pDriver );
        
        // Draw (don't apply attribute a second time if it isn't necessary)!
        hMaterial = pPalette->getMaterial();
        pMesh->drawSubset( hMaterial, pPalette->getDataGroup(), (hMaterial == hPrevMaterial) ? cgMeshDrawMode::ShaderPasses : cgMeshDrawMode::Automatic );
        hPrevMaterial = hMaterial;

    } // Next Palette*/

    // Disable vertex blending
    pDriver->setVertexBlendData( CG_NULL, CG_NULL, 0, -1 );
    
    // Drawn
    return true;

    /* ResourceHandle hAttribute, hPrevAttribute;

    // Get access to required systems
    gpRenderDriver * pDriver = mParentScene->getRenderDriver();

    // Bail if not yet processed
    if ( mMesh.IsValid() == false )
        return false;

    // Call base class render
    if ( gpSceneObject::render( pCamera ) == false )
        return false;

    // Retrieve the underlying mesh resource and render if available
    gpMesh * pMesh = mMesh;
    if ( pMesh->isLoaded() == false )
        return false;

    // Begin any profiler timing
    ProfileRecordBegin();

    // Begin to draw the relevant subset of this object
    pDriver->setWorldTransform( GP_NULL );
    
    // Process each palette in the skin with a matching attribute.
    const gpMesh::BonePaletteArray & Palettes = pMesh->getBonePalettes();
    for ( size_t i = 0; i < Palettes.size(); ++i )
    {
        gpBonePalette * pPalette = Palettes[i];
        
        // Apply the bone palette.
        pPalette->apply( mBoneReferences, pMesh->getSkinBindData(), pDriver );
        
        // Draw (don't apply attribute a second time if it isn't necessary)!
        hAttribute = pPalette->GetAttribute();
        pMesh->drawSubset( hAttribute, pPalette->getDataGroup(), (hAttribute == hPrevAttribute) ? gpMeshDrawMode::EffectPasses : gpMeshDrawMode::Automatic );
        hPrevAttribute = hAttribute;

    } // Next Palette

    // Disable vertex blending
    pDriver->setVertexBlendData( GP_NULL, 0, -1 );
    
    // Complete any profiler timing
    ProfileRecordEnd();

    // Drawn
    return true;*/
}

//-----------------------------------------------------------------------------
// Name : renderSubset ()
/// <summary>
/// Render only the section of this object that relates to the specified 
/// material (used for material batched / sorted rendering).
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinObject::renderSubset( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer, const cgMaterialHandle & hMaterial )
{
    // Get access to required systems
    cgRenderDriver * pDriver = mWorld->getRenderDriver();

    // Retrieve the underlying mesh resource and render if available
    cgMesh * pMesh = mMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() )
        return false;

    // Has skinning data?
    cgSkinBindData * pBindData = pMesh->getSkinBindData();
    if ( !pBindData )
        return false;

    /*// Signal a straight draw of the object
    pDriver->setWorldTransform( pIssuer->getWorldTransform( false ) );
    cgToDo( "Effect Overhaul", "Commit changes removed. It /should/ no longer be required, but just confirm." )
    //pDriver->commitChanges(); // Required for batching, effect pass already begun.
    pMesh->setDefaultColor( pIssuer->getNodeColor() );
    pMesh->drawSubset( hMaterial, cgMeshDrawMode::Simple );*/

    // Begin to draw the relevant subset of this object
    pDriver->setWorldTransform( CG_NULL );
    pMesh->setDefaultColor( pIssuer->getNodeColor() );
    
    // Build an array containing all of the bones that are required
    // by the binding data in the skinned mesh.
    const cgSkinBindData::BoneArray & BindList = pBindData->getBones();
    const cgSkinNode::BoneInstanceMap & BoneNodes = ((cgSkinNode*)pIssuer)->mBoneInstanceIdentifiers;
    cgObjectNodeArray aBones( BindList.size(), CG_NULL );
    for ( size_t i = 0; i < BindList.size(); ++i )
    {
        // Get the requested bone.
        cgSkinNode::BoneInstanceMap::const_iterator itBone = BoneNodes.find( BindList[i]->boneIdentifier );
        if ( itBone == BoneNodes.end() )
            continue;

        // Assign the bone with this identifier.
        cgBoneNode * pBone = (cgBoneNode*)cgReferenceManager::getReference( itBone->second );
        aBones[i] = pBone;
        
    } // Next Influence

    // Process each palette in the skin with a matching attribute.
    const cgMesh::BonePaletteArray & Palettes = pMesh->getBonePalettes();
    for ( size_t i = 0; i < Palettes.size(); ++i )
    {
        cgBonePalette * pPalette = Palettes[i];
        
        // Skip any palette that does not have a matching attribute
        if ( pPalette->getMaterial() != hMaterial )
            continue;

        // Apply the bone palette.
        pPalette->apply( aBones, pMesh->getSkinBindData(), false, pDriver );

        // Have the currently assigned surface shader re-select its states and or shaders based on
        // the modifications that were made by the palette apply method (primarily 'MaxBlendIndex')
        cgToDo( "Carbon General", "Only re-commit if the max blend index changes?" );
        pDriver->commitChanges( );

        // Draw!
        pMesh->drawSubset( hMaterial, pPalette->getDataGroup(), cgMeshDrawMode::Simple );

    } // Next Palette

    // Disable vertex blending
    pDriver->setVertexBlendData( CG_NULL, CG_NULL, 0, -1 );
    cgToDo( "Carbon General", "Only re-commit if the max blend index changes?" );
    pDriver->commitChanges( );

    // Drawn
    return true;

    /* // Bail if object not visible
    if ( IsRenderable() == false )
        return false;

    // Get access to required systems
    gpRenderDriver * pDriver = gpRenderDriver::getInstance();

    // Retrieve the underlying mesh resource and render if available
    gpMesh * pMesh = mMesh;
    if ( pMesh == GP_NULL || pMesh->isLoaded() == false )
        return false;

    // Begin any profiler timing
    ProfileRecordBegin();

    // Begin to draw the relevant subset of this object
    pDriver->setWorldTransform( GP_NULL );
    
    // Process each palette in the skin with a matching attribute.
    const gpMesh::BonePaletteArray & Palettes = pMesh->getBonePalettes();
    for ( size_t i = 0; i < Palettes.size(); ++i )
    {
        gpBonePalette * pPalette = Palettes[i];
        
        // Skip any palette that does not have a matching attribute
        if ( pPalette->GetAttribute() != hAttribute )
            continue;

        // Apply the bone palette.
        pPalette->apply( mBoneReferences, pMesh->getSkinBindData(), pDriver );
        pDriver->commitChanges(); // Required for batching, effect pass already begun.

        // Draw!
        pMesh->drawSubset( hAttribute, pPalette->getDataGroup(), gpMeshDrawMode::Simple );

    } // Next Palette

    // Disable vertex blending
    pDriver->setVertexBlendData( GP_NULL, 0, -1 );
    
    // Complete any profiler timing
    ProfileRecordEnd();

    // Drawn
    return true;*/
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgSkinObject::getLocalBoundingBox( )
{
    // Opt-out of the bounding box process at the object level.
    // The various associated nodes will return the bounding box based
    // on the attached bones.
    return cgBoundingBox();
}

///////////////////////////////////////////////////////////////////////////////
// cgSkinNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSkinNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinNode::cgSkinNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgMeshNode( nReferenceId, pScene )
{
    // Default all new skins to a physics model of 'None'. This cannot change.
    mPhysicsModel = cgPhysicsModel::None;

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Skin%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgSkinNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinNode::cgSkinNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgMeshNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Force all skins to a physics model of 'None'. This cannot change.
    mPhysicsModel = cgPhysicsModel::None;

    // Duplicate data.
    cgSkinNode * pSkin = (cgSkinNode*)pInit;
    mBoneReferences            = pSkin->mBoneReferences;
    mBoneInstanceIdentifiers   = pSkin->mBoneInstanceIdentifiers;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSkinNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinNode::~cgSkinNode()
{
    // Clean up.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear instance data.
    mBoneReferences.clear();
    mBoneInstanceIdentifiers.clear();

    // Call base class implementation
    if ( bDisposeBase == true )
        cgMeshNode::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgSkinNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgSkinNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgSkinNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgSkinNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SkinNode )
        return true;

    // Supported by base?
    return cgMeshNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : buildPhysicsBody ( ) (Virtual, Protected)
/// <summary>
/// Construct the internal physics body designed to represent this node.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinNode::buildPhysicsBody( )
{
    // Skins cannot have a physics body.
}

//-----------------------------------------------------------------------------
// Name : attachBone ( )
/// <summary>
/// Attach the specified bone to this skin as one of its potential influencing
/// transforms.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinNode::attachBone( cgBoneNode * pBone )
{
    // Invalid bone or already exists?
    if ( !pBone || mBoneReferences.find( pBone->getReferenceId() ) != mBoneReferences.end() )
        return false;

    // Insert a reference to this bone into the instance data table.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mInsertAttachedBone.bindParameter( 1, mReferencedObject->getReferenceId() );
        mInsertAttachedBone.bindParameter( 2, mReferenceId );
        mInsertAttachedBone.bindParameter( 3, pBone->getReferenceId() );
        if ( mInsertAttachedBone.step( true ) == false )
        {
            cgString strError;
            mInsertAttachedBone.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert attached bone identifier for skin object node '0x%x'. Error: %s"), mReferenceId, strError.c_str() );
            return false;

        } // End if failed
        
    } // End if serialize

    // Insert into our in-memory list.
    mBoneReferences.insert( pBone->getReferenceId() );
    mBoneInstanceIdentifiers[ pBone->getInstanceIdentifier() ] = pBone->getReferenceId();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overridden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinNode::onNodeCreated( const cgUID & ObjectType, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgMeshNode::onNodeCreated( ObjectType, CloneMethod ) )
        return false;
    
    // Insert references to all attached bones if there are any
    // already defined (i.e. cloning?)
    if ( shouldSerialize() )
    {
        cgWorld * pWorld = mParentScene->getParentWorld();
        pWorld->beginTransaction( _T("onSkinNodeCreated") );

        // Insert each referenced bone.
        prepareQueries();
        cgUInt32Set::const_iterator itBone;
        for ( itBone = mBoneReferences.begin(); itBone != mBoneReferences.end(); ++itBone )
        {
            // Insert!
            mInsertAttachedBone.bindParameter( 1, mReferencedObject->getReferenceId() );
            mInsertAttachedBone.bindParameter( 2, mReferenceId );
            mInsertAttachedBone.bindParameter( 3, *itBone );
            if ( mInsertAttachedBone.step( true ) == false )
            {
                cgString strError;
                mInsertAttachedBone.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert attached bone identifier for skin object node '0x%x'. Error: %s"), mReferenceId, strError.c_str() );
                pWorld->rollbackTransaction( _T("onSkinNodeCreated") );
                return false;

            } // End if failed

        } // Next Bone

        // Commit changes
        pWorld->commitTransaction( _T("onSkinNodeCreated") );
        
    } // End if serialize
    
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
bool cgSkinNode::onNodeLoading( const cgUID & ObjectType, cgWorldQuery * pNodeData, cgSceneCell * pParentCell, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgMeshNode::onNodeLoading( ObjectType, pNodeData, pParentCell, CloneMethod ) )
        return false;

    // Retrieve the *original* reference identifier we're loading from. We need
    // this so that we can load the identifiers of the bones that were attached
    // to the original node.
    cgUInt32 nOriginalRefId = 0;
    pNodeData->getColumn( _T("RefId"), nOriginalRefId );

    // Load the attached bone list.
    prepareQueries();
    mLoadAttachedBones.bindParameter( 1, nOriginalRefId );
    if ( !mLoadAttachedBones.step( ) )
    {
        // Log any error.
        cgString strError;
        if ( mLoadAttachedBones.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve attached bone references for skin object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve attached bone references for skin object node '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadAttachedBones.reset();
        return false;
    
    } // End if failed
    
    // Iterate through each row returned and retrieve each reference.
    for ( ; mLoadAttachedBones.nextRow(); )
    {
        cgUInt32 nBoneRefId = 0;
        mLoadAttachedBones.getColumn( _T("BoneRefId"), nBoneRefId );

        // Insert into the set of attached bones. 
        mBoneReferences.insert( nBoneRefId );

    } // Next sampler

    // Clean up
    mLoadAttachedBones.reset();

    
    cgToDo( "Spawning", "Insert cloned bone references? Take care because the references may change in 'onNodeInit()'." )

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeInit () (Virtual)
/// <summary>
/// Optional method called both after creation and during loading to allow the
/// node to finally initialize itself with all relevant scene data it may
/// depend upon being available. In cases where reference identifiers pointing 
/// to nodes may have been remapped during loading (i.e. in cases where 
/// performing a cloned load), information about the remapping is provided to 
/// allow the  node (or its underlying object) to take appropriate action.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinNode::onNodeInit( const cgUInt32IndexMap & NodeReferenceRemap )
{
    // Remap bone references appropriately.
    if ( !NodeReferenceRemap.empty() )
    {
        cgUInt32Set NewReferences;
        cgUInt32Set::iterator itBone;
        for ( itBone = mBoneReferences.begin(); itBone != mBoneReferences.end(); ++itBone )
        {
            cgUInt32IndexMap::const_iterator itRef = NodeReferenceRemap.find( *itBone );
            if ( itRef != NodeReferenceRemap.end() )
                NewReferences.insert( itRef->second );
            else
                NewReferences.insert( *itBone );
        
        } // Next bone reference
        mBoneReferences.swap( NewReferences );

    } // End if remapping

    // Build the final map of instance identifiers -> reference identifiers
    mBoneInstanceIdentifiers.clear();
    cgUInt32Set::iterator itBone;
    for ( itBone = mBoneReferences.begin(); itBone != mBoneReferences.end(); ++itBone )
    {
        cgReference * pRef = cgReferenceManager::getReference( *itBone );
        if ( !pRef || !pRef->queryReferenceType( RTID_BoneNode ) )
            continue;
        mBoneInstanceIdentifiers[ ((cgBoneNode*)pRef)->getInstanceIdentifier() ] = *itBone;

    } // Next bone reference

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeDeleted () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// deleted in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkinNode::onNodeDeleted( )
{
    // Delete attached bone reference list.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mDeleteAttachedBones.bindParameter( 1, mReferenceId );
        if ( !mDeleteAttachedBones.step( true ) )
        {
            // Log any error.
            cgString strError;
            if ( mDeleteAttachedBones.getLastError( strError ) == false )
                cgAppLog::write( cgAppLog::Error, _T("Failed to delete attached bone references for skin object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
            else
                cgAppLog::write( cgAppLog::Error, _T("Failed to delete attached bone references for skin object node '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return false;

        } // End if failed

    } // End if should serialize
    
    // Call base class implementation last.
    return cgMeshNode::onNodeDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinNode::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    cgWorld * pWorld = mParentScene->getParentWorld();
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( !mInsertAttachedBone.isPrepared() )
            mInsertAttachedBone.prepare( pWorld, _T("INSERT INTO 'Objects::Skin::InstanceData::Bones' VALUES(NULL,?1,?2,?3)"), true );
        if ( !mDeleteAttachedBones.isPrepared() )
            mDeleteAttachedBones.prepare( pWorld, _T("DELETE FROM 'Objects::Skin::InstanceData::Bones' WHERE NodeRefId=?1"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadAttachedBones.isPrepared() )
        mLoadAttachedBones.prepare( pWorld, _T("SELECT * FROM 'Objects::Skin::InstanceData::Bones' WHERE NodeRefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox () (Virtual)
/// <summary>
/// Retrieve the axis aligned bounding box for this node (encompassing
/// its referenced object) as it exists in the object's local space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgSkinNode::getLocalBoundingBox( )
{
    cgBoundingBox Bounds, ChildBounds;
    cgTransform InverseSkinTransform;
    cgTransform::inverse( InverseSkinTransform, getWorldTransform() );

    // Process all attached bones.
    cgUInt32Set::const_iterator itBoneRef;
    for ( itBoneRef = mBoneReferences.begin(); itBoneRef != mBoneReferences.end(); ++itBoneRef )
    {
        cgBoneNode * pBone = (cgBoneNode*)cgReferenceManager::getReference( *itBoneRef );
        if ( !pBone ) continue;

        // Retrieve the bounding box of the bone.
        ChildBounds = pBone->getLocalBoundingBox( );
        if ( ChildBounds.isPopulated() )
        {
            // Transform the bounding box into the space of the skin.
            // In order to do so we must compute the relative transformation
            // to transform from one space to the other.
            cgTransform RelativeTransform;
            cgTransform::multiply( RelativeTransform, pBone->getWorldTransform(), InverseSkinTransform );
            ChildBounds.transform( RelativeTransform );
            
            // Grow the current bounding box as required
            Bounds.addPoint( ChildBounds.min );
            Bounds.addPoint( ChildBounds.max );

        } // End if has bounding box
    
    } // Next Bone Ref

    // Return computed bounding box.
    return Bounds;
}