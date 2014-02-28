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
// Name : cgMesh.cpp                                                         //
//                                                                           //
// Desc : Contains classes that provide support for loading, creating and    //
//        rendering geometry. Support is included for loading directly from  //
//        XML data structure, or building a mesh procedurally.               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgMesh Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgMesh.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgVertexBuffer.h>
#include <Resources/cgIndexBuffer.h>
#include <Resources/cgStandardMaterial.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgRenderingCapabilities.h>
#include <System/cgStringUtility.h>
#include <System/cgExceptions.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgObjectNode.h>
#include <World/cgWorldQuery.h>
#include <Math/cgMathUtility.h>
#include <Math/cgCollision.h>
#include <unordered_map>

// ToDo: 9999 - If any vertex format (including that specified to prepareMesh() and setVertexSource())
// does not have a position component, we should probably fail.
// ToDo: 9999 - Remove all the separate 'm_bDB*Dirty' bools and replace with a single uint16 and flags like the material.

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgMesh::mInsertMesh;
cgWorldQuery cgMesh::mDeleteSubsets;
cgWorldQuery cgMesh::mInsertSubset;
cgWorldQuery cgMesh::mUpdateIndices;
cgWorldQuery cgMesh::mUpdateVertexFormat;
cgWorldQuery cgMesh::mDeleteStreams;
cgWorldQuery cgMesh::mInsertStream;
cgWorldQuery cgMesh::mUpdateStream;
cgWorldQuery cgMesh::mDeleteSkinBindData;
cgWorldQuery cgMesh::mInsertSkinInfluence;
cgWorldQuery cgMesh::mDeleteBonePalettes;
cgWorldQuery cgMesh::mInsertBonePalette;
cgWorldQuery cgMesh::mLoadMesh;
cgWorldQuery cgMesh::mLoadStreams;
cgWorldQuery cgMesh::mLoadSubsets;
cgWorldQuery cgMesh::mLoadSkinBindData;
cgWorldQuery cgMesh::mLoadBonePalettes;

//-----------------------------------------------------------------------------
// Local Module Level Namespaces.
//-----------------------------------------------------------------------------
// Settings for the mesh optimizer.
namespace MeshOptimizer
{
    const cgFloat CacheDecayPower     = 1.5f;
    const cgFloat LastTriScore        = 0.75f;
    const cgFloat ValenceBoostScale   = 2.0f;
    const cgFloat ValenceBoostPower   = 0.5f;
    const cgInt32 MaxVertexCacheSize  = 32;

}; // End namespace MeshOptimizer

///////////////////////////////////////////////////////////////////////////////
// cgMesh Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgMesh () (Constructor)
/// <summary>
/// cgMesh Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgMesh::cgMesh( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldResourceComponent( nReferenceId, pWorld )
{
    // Initialize variable to sensible defaults
    mBoundingBox.reset();
    mPrepareStatus                = cgMeshStatus::NotPrepared;
    mFaceCount                    = 0;
    mVertexCount                  = 0;
    mHardwareMesh                 = false;
    mOptimizedMesh                = false;
    mDisableFinalSort             = false;
    mPrepareData.vertexSource     = CG_NULL;
    mPrepareData.ownsSource       = false;
    mPrepareData.sourceFormat     = CG_NULL;
    mPrepareData.triangleCount    = 0;
    mPrepareData.vertexCount      = 0;
    mPrepareData.computeNormals   = false;
    mPrepareData.computeBinormals = false;
    mPrepareData.computeTangents  = false;
    mSystemVB                     = CG_NULL;
    mVertexFormat                 = CG_NULL;
    mSystemIB                     = CG_NULL;
    mSkinBindData                 = CG_NULL;
    mFinalizeMesh                 = true;
    mForceTangentGen              = false;
    mForceNormalGen               = false;
    mDefaultColor                 = 0xFFFFFFFF;

    // Loading and serialization
    mSourceRefId                  = 0;
    mMeshSerialized               = false;
    mDBStreamId                   = 0;
    mDBIndicesDirty               = false;
    mDBFormatDirty                = false;
    mDBVertexStreamDirty          = false;
    mDBSubsetsDirty               = false;
    mDBSkinDataDirty              = false;
    mDBBonePalettesDirty          = false;

    // Cached responses
    mResourceType                 = cgResourceType::Mesh;
    mResourceLoaded               = false;
    mResourceLost                 = false;
    mCanEvict                     = (isInternalReference() == false);
}

//-----------------------------------------------------------------------------
//  Name : cgMesh () (Constructor)
/// <summary>
/// cgMesh Cloning Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgMesh::cgMesh( cgUInt32 nReferenceId, cgWorld * pWorld, cgMesh * pInit ) : cgWorldResourceComponent( nReferenceId, pWorld, pInit )
{
    // Initialize necessary variables to sensible defaults initially.
    mSystemVB             = CG_NULL;
    mSystemIB             = CG_NULL;
    mSkinBindData         = CG_NULL;

    // Loading and serialization
    mSourceRefId          = 0;
    mMeshSerialized       = false;
    mDBStreamId           = 0;
    mDBIndicesDirty       = false;
    mDBFormatDirty        = false;
    mDBVertexStreamDirty  = false;
    mDBSubsetsDirty       = false;
    mDBSkinDataDirty      = false;
    mDBBonePalettesDirty  = false;
    
    // Duplicate simple vars.
    mFinalizeMesh         = pInit->mFinalizeMesh;
    mForceTangentGen      = pInit->mForceTangentGen;
    mForceNormalGen       = pInit->mForceNormalGen;
    mDisableFinalSort     = pInit->mDisableFinalSort;
    mTriangleData         = pInit->mTriangleData;
    mDefaultMaterial      = pInit->mDefaultMaterial;
    mDefaultColor         = pInit->mDefaultColor;
    mVertexFormat         = pInit->mVertexFormat;
    mHardwareMesh         = pInit->mHardwareMesh;
    mOptimizedMesh        = pInit->mOptimizedMesh;
    mBoundingBox          = pInit->mBoundingBox;
    mFaceCount            = pInit->mFaceCount;
    mVertexCount          = pInit->mVertexCount;
    mObjectMaterials      = pInit->mObjectMaterials;
    mPrepareStatus        = pInit->mPrepareStatus;

    // Deep copy preparation data if necessary.
    if ( mPrepareStatus == cgMeshStatus::Preparing )
    {
        mPrepareData = pInit->mPrepareData;
        if ( mPrepareData.ownsSource == true && mPrepareData.vertexSource != CG_NULL )
        {
            cgUInt32 nBufferSize = mPrepareData.vertexCount * mVertexFormat->getStride();
            cgByte * pSource = new cgByte[ nBufferSize ];
            memcpy( pSource, mPrepareData.vertexSource, nBufferSize );
            mPrepareData.vertexSource = pSource;
        
        } // End if duplicate source

    } // End if preparing

    // Duplicate mesh subset information.
    std::tr1::unordered_map<void*,size_t> SrcSubsetIndices;
    mMeshSubsets.resize( pInit->mMeshSubsets.size() );
    for ( size_t i = 0; i < pInit->mMeshSubsets.size(); ++i )
    {
        MeshSubset * pSrcSubset = pInit->mMeshSubsets[i];

        // Mark the location of the source subset in its subset array.
        // This allows us to correctly reconstruct the other subset lookup 
        // tables (i.e. mDataGroups, mMaterials, mSubsetLookup, etc.)
        SrcSubsetIndices[ pSrcSubset ] = i;

        // Create new subset entry.
        MeshSubset * pNewSubset  = new MeshSubset();
        pNewSubset->material    = pSrcSubset->material;
        pNewSubset->dataGroupId = pSrcSubset->dataGroupId;
        pNewSubset->faceStart   = pSrcSubset->faceStart;
        pNewSubset->faceCount   = pSrcSubset->faceCount;
        pNewSubset->vertexStart = pSrcSubset->vertexStart;
        pNewSubset->vertexCount = pSrcSubset->vertexCount;
        mMeshSubsets[i] = pNewSubset;

    } // Next Subset

    // Duplicate data group subset lookup tables.
    DataGroupSubsetMap::iterator itDataGroup;
    for ( itDataGroup = pInit->mDataGroups.begin(); itDataGroup != pInit->mDataGroups.end(); ++itDataGroup )
    {
        mDataGroups[ itDataGroup->first ] = itDataGroup->second;
        
        // Remap original subset pointers to our new ones.
        SubsetArray & Subsets = mDataGroups[ itDataGroup->first ];
        for ( size_t j = 0; j < Subsets.size(); ++j )
            Subsets[j] = mMeshSubsets[SrcSubsetIndices[Subsets[j]]];

    } // Next Data Group

    // Duplicate material subset lookup tables.
    MaterialSubsetMap::iterator itMaterial;
    for ( itMaterial = pInit->mMaterials.begin(); itMaterial != pInit->mMaterials.end(); ++itMaterial )
    {
        mMaterials[ itMaterial->first ] = itMaterial->second;
        
        // Remap original subset pointers to our new ones.
        SubsetArray & Subsets = mMaterials[ itMaterial->first ];
        for ( size_t j = 0; j < Subsets.size(); ++j )
            Subsets[j] = mMeshSubsets[SrcSubsetIndices[Subsets[j]]];

    } // Next Material

    // Duplicate full subset lookup tables.
    mSubsetLookup = pInit->mSubsetLookup;
    SubsetKeyMap::iterator itSubset;
    for ( itSubset = mSubsetLookup.begin(); itSubset != mSubsetLookup.end(); ++itSubset )
        itSubset->second = mMeshSubsets[SrcSubsetIndices[itSubset->second]];

    // Duplicate system VB and IB data.
    if ( pInit->mSystemVB != CG_NULL )
    {
        cgUInt32 nBufferSize = mVertexCount * mVertexFormat->getStride();
        mSystemVB = new cgByte[ nBufferSize ];
        memcpy( mSystemVB, pInit->mSystemVB, nBufferSize );
    
    } // End if valid SystemVB
    if ( pInit->mSystemIB != CG_NULL )
    {
        mSystemIB = new cgUInt32[ mFaceCount * 3 ];
        memcpy( mSystemIB, pInit->mSystemIB, mFaceCount * 3 * sizeof(cgUInt32) );
    
    } // End if valid SystemIB

    // Create hardware VB/IB as necessary. We won't fill them directly, but will instead 
    // mark them as lost and /try/ to restore them (restoreBuffers()). If this fails, they
    // will still get correctly restored just prior to rendering.
    if ( pInit->mHardwareVB.isValid() == true )
    {
        cgVertexBuffer    * pBuffer    = pInit->mHardwareVB.getResource(true);
        cgResourceManager * pResources = pBuffer->getManager();
        pResources->createVertexBuffer( &mHardwareVB, pBuffer->getLength(), pBuffer->getUsage(),
                                        pBuffer->getFormat(), pBuffer->getPool(), cgDebugSource() );
        mHardwareVB.setResourceLost( true );

    } // End if VB valid
    if ( pInit->mHardwareIB.isValid() == true )
    {
        cgIndexBuffer     * pBuffer    = pInit->mHardwareIB.getResource(true);
        cgResourceManager * pResources = pBuffer->getManager();
        pResources->createIndexBuffer( &mHardwareIB, pBuffer->getLength(), pBuffer->getUsage(),
                                       pBuffer->getFormat(), pBuffer->getPool(), cgDebugSource() );
        mHardwareIB.setResourceLost( true );

    } // End if IB valid

    // Attempt to restore hardware buffers.
    restoreBuffers();

    // Duplicate skin binding data if any is available.
    if ( pInit->mSkinBindData != CG_NULL )
        mSkinBindData = new cgSkinBindData( *pInit->mSkinBindData );
    
    // Duplicate bone palette entries.
    mBonePalettes.resize( pInit->mBonePalettes.size() );
    for ( size_t i = 0; i < mBonePalettes.size(); ++i )
        mBonePalettes[i] = new cgBonePalette( *pInit->mBonePalettes[i] );

    // Cached resource responses.
    mResourceType     = cgResourceType::Mesh;
    mResourceLoaded   = false; // Note: Important that the mesh is not classed as loaded since its data may need to be serialized.
    mResourceLost     = false;
    mCanEvict         = (isInternalReference() == false);
}

//-----------------------------------------------------------------------------
//  Name : cgMesh () (Constructor)
/// <summary>
/// cgMesh Loading Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgMesh::cgMesh( cgUInt32 nReferenceId, cgWorld * pWorld, cgUInt32 nSourceRefId, bool bFinalizeMesh ) : cgWorldResourceComponent( nReferenceId, pWorld )
{
    // Initialize variable to sensible defaults
    mBoundingBox.reset();
    mPrepareStatus                = cgMeshStatus::NotPrepared;
    mFaceCount                    = 0;
    mVertexCount                  = 0;
    mHardwareMesh                 = false;
    mOptimizedMesh                = false;
    mDisableFinalSort             = false;
    mPrepareData.vertexSource     = CG_NULL;
    mPrepareData.ownsSource       = false;
    mPrepareData.sourceFormat     = CG_NULL;
    mPrepareData.triangleCount    = 0;
    mPrepareData.vertexCount      = 0;
    mPrepareData.computeNormals   = false;
    mPrepareData.computeBinormals = false;
    mPrepareData.computeTangents  = false;
    mSystemVB                     = CG_NULL;
    mVertexFormat                 = CG_NULL;
    mSystemIB                     = CG_NULL;
    mSkinBindData                 = CG_NULL;
    mFinalizeMesh                 = bFinalizeMesh;
    mForceTangentGen              = false;
    mForceNormalGen               = false;
    mDefaultColor                 = 0xFFFFFFFF;

    // Loading and serialization
    mSourceRefId                  = nSourceRefId;
    mMeshSerialized               = false;
    mDBStreamId                   = 0;
    mDBIndicesDirty               = false;
    mDBFormatDirty                = false;
    mDBVertexStreamDirty          = false;
    mDBSubsetsDirty               = false;
    mDBSkinDataDirty              = false;
    mDBBonePalettesDirty          = false;

    // Cached responses
    mResourceType                 = cgResourceType::Mesh;
    mResourceLoaded               = false;
    mResourceLost                 = false;
    mCanEvict                     = (isInternalReference() == false);
}

//-----------------------------------------------------------------------------
//  Name : ~cgMesh () (Destructor)
/// <summary>
/// cgMesh Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgMesh::~cgMesh()
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
void cgMesh::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    mDefaultMaterial.close();
    unloadResource();

    // Dispose base(s).
    if ( bDisposeBase )
        cgWorldResourceComponent::dispose( true );
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
bool cgMesh::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_MeshResource )
        return true;

    // Supported by base?
    return cgWorldResourceComponent::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgMesh::getDatabaseTable( ) const
{
    return _T("DataSources::Mesh");
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertMesh.isPrepared() )
            mInsertMesh.prepare( mWorld, _T("INSERT INTO 'DataSources::Mesh' VALUES(?1,?2,?3,?4,?5,?6)"), true );
        if ( !mDeleteSubsets.isPrepared() )
            mDeleteSubsets.prepare( mWorld, _T("DELETE FROM 'DataSources::Mesh::Subsets' WHERE DataSourceId=?1"), true );
        if ( !mInsertSubset.isPrepared() )
            mInsertSubset.prepare( mWorld, _T("INSERT INTO 'DataSources::Mesh::Subsets' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7)"), true );
        if ( !mUpdateIndices.isPrepared() )
            mUpdateIndices.prepare( mWorld, _T("UPDATE 'DataSources::Mesh' SET IndexCount=?1,Indices=?2 WHERE RefId=?3"), true );
        if ( !mUpdateVertexFormat.isPrepared() )
            mUpdateVertexFormat.prepare( mWorld, _T("UPDATE 'DataSources::Mesh' SET Declaration=?1 WHERE RefId=?2"), true );
        if ( !mDeleteStreams.isPrepared() )
            mDeleteStreams.prepare( mWorld, _T("DELETE FROM 'DataSources::Mesh::Streams' WHERE DataSourceId=?1"), true );
        if ( !mInsertStream.isPrepared() )
            mInsertStream.prepare( mWorld, _T("INSERT INTO 'DataSources::Mesh::Streams' VALUES(NULL,?1,?2,?3,?4,?5)"), true );
        if ( !mUpdateStream.isPrepared() )
            mUpdateStream.prepare( mWorld, _T("UPDATE 'DataSources::Mesh::Streams' SET EntryCount=?1,EntryStride=?2,StreamData=?3 WHERE StreamId=?4"), true );
        if ( !mDeleteSkinBindData.isPrepared() )
            mDeleteSkinBindData.prepare( mWorld, _T("DELETE FROM 'DataSources::Mesh::SkinBindData' WHERE DataSourceId=?1"), true );
        if ( !mInsertSkinInfluence.isPrepared() )
            mInsertSkinInfluence.prepare( mWorld, _T("INSERT INTO 'DataSources::Mesh::SkinBindData' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15)"), true );
        if ( !mDeleteBonePalettes.isPrepared() )
            mDeleteBonePalettes.prepare( mWorld, _T("DELETE FROM 'DataSources::Mesh::BonePalettes' WHERE DataSourceId=?1"), true );
        if ( !mInsertBonePalette.isPrepared() )
            mInsertBonePalette.prepare( mWorld, _T("INSERT INTO 'DataSources::Mesh::BonePalettes' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7)"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadMesh.isPrepared() )
        mLoadMesh.prepare( mWorld, _T("SELECT * FROM 'DataSources::Mesh' WHERE RefId=?1"), true );
    if ( !mLoadSubsets.isPrepared() )
        mLoadSubsets.prepare( mWorld, _T("SELECT * FROM 'DataSources::Mesh::Subsets' WHERE DataSourceId=?1"), true );
    if ( !mLoadStreams.isPrepared() )
        mLoadStreams.prepare( mWorld, _T("SELECT * FROM 'DataSources::Mesh::Streams' WHERE DataSourceId=?1 AND StreamIndex=0"), true );
    if ( !mLoadSkinBindData.isPrepared() )
        mLoadSkinBindData.prepare( mWorld, _T("SELECT * FROM 'DataSources::Mesh::SkinBindData' WHERE DataSourceId=?1"), true );
    if ( !mLoadBonePalettes.isPrepared() )
        mLoadBonePalettes.prepare( mWorld, _T("SELECT * FROM 'DataSources::Mesh::BonePalettes' WHERE DataSourceId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : getRenderDriver ()
/// <summary>
/// Retrieve the render driver to which this mesh is associated.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver * cgMesh::getRenderDriver()
{
    return ( getManager() == CG_NULL ) ? CG_NULL : getManager()->getRenderDriver();
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::loadResource( )
{
    bool bSerializeNecessary = false;

    // Is resource already loaded?
    if ( isLoaded() )
        return true;

    // If we should be loading data, do so now.
    if ( mSourceRefId != 0 )
    {
        // Load the mesh data.
        if ( !loadMesh( mSourceRefId, CG_NULL, mFinalizeMesh ) )
            return false;

        // If we are not simply wrapping the original database resource (i.e. the source
        // reference identifier does not match this resource's actual identifier) then we 
        // potentially need to insert the loaded data into a new set of database records.
        if ( mSourceRefId != mReferenceId )
            bSerializeNecessary = true;
        
    } // End if loading
    else
    {
        // Any data that has been provided before this call (i.e. the
        // application has added geometry) needs to be written to
        // the database.
        bSerializeNecessary = true;

    } // End if not loading

    // If we are not an internal resource, write any requested data.
    if ( bSerializeNecessary && shouldSerialize() )
    {
        if ( !serializeMesh( ) )
            return false;

    } // End if serialize data.

    // Mark as fully loaded.
    mResourceLoaded = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::unloadResource( )
{
    SubsetArray::iterator itSubset;
    size_t                i;

    // Iterate through the different subsets in the mesh and clean up
    for ( itSubset = mMeshSubsets.begin(); itSubset != mMeshSubsets.end(); ++itSubset )
    {
        if ( *itSubset != CG_NULL )
            delete (*itSubset);
    
    } // Next Subset
    mMeshSubsets.clear();
    mSubsetLookup.clear();
    mDataGroups.clear();
    mMaterials.clear();
    mObjectMaterials.clear();

    // Release bone palettes and skin data (if any)
    for ( i = 0; i < mBonePalettes.size(); ++i )
    {
        if ( mBonePalettes[i] != CG_NULL )
            delete mBonePalettes[i];
    
    } // Next Palette
    mBonePalettes.clear();
    if ( mSkinBindData != CG_NULL )
        delete mSkinBindData;

    // Clean up preparation data.
    if ( mPrepareData.ownsSource == true )
        delete []mPrepareData.vertexSource;
    mPrepareData.vertexSource = CG_NULL;
    mPrepareData.sourceFormat = CG_NULL;
    mPrepareData.ownsSource = false;
    mPrepareData.vertexData.clear();
    mPrepareData.vertexFlags.clear();
    mPrepareData.vertexRecords.clear();
    mPrepareData.triangleData.clear();

    // Release mesh data memory
    if ( mSystemVB != CG_NULL )
        delete []mSystemVB;
    if ( mSystemIB != CG_NULL )
        delete []mSystemIB;
    mTriangleData.clear();

    // Release resources
    mHardwareVB.close();
    mHardwareIB.close();
    
    // Clear variables
    mPrepareData.vertexSource     = CG_NULL;
    mPrepareData.ownsSource       = false;
    mPrepareData.sourceFormat     = CG_NULL;
    mPrepareData.triangleCount    = 0;
    mPrepareData.vertexCount      = 0;
    mPrepareData.computeNormals   = false;
    mPrepareData.computeBinormals = false;
    mPrepareData.computeTangents  = false;
    mPrepareStatus                 = cgMeshStatus::NotPrepared;
    mFaceCount                    = 0;
    mVertexCount                  = 0;
    mSystemVB                     = CG_NULL;
    mVertexFormat                 = CG_NULL;
    mSystemIB                     = CG_NULL;
    mSkinBindData                 = CG_NULL;
    mForceTangentGen              = false;
    mForceNormalGen               = false;

    // Loading and serialization
    // Note: Do not modify mMeshSerialized or mDBStreamId
    mDBIndicesDirty               = false;
    mDBFormatDirty                = false;
    mDBVertexStreamDirty          = false;
    mDBSubsetsDirty               = false;
    mDBSkinDataDirty              = false;
    mDBBonePalettesDirty          = false;

    // Reset structures
    mBoundingBox.reset();

    // Resource is no longer loaded, but if we are not internal
    // and the data WAS serialized, we can reload.
    if ( isInternalReference() == false && mMeshSerialized == true )
        mSourceRefId = mReferenceId;
    mResourceLoaded = false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadMesh ()
/// <summary>
/// Load the specified mesh from the supplied database file.
/// Note : This function requires you to specify the resource manager into which
/// you would like all additional resources to be loaded (such as
/// vertex buffers, textures, materials etc) if you are not loading this
/// mesh directly via the cgResourceManager::loadMesh() methods.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::loadMesh( cgUInt32 nSourceRefId, cgResourceManager * pManager /* = CG_NULL */, bool bFinalizeMesh /* = true */ )
{
    cgUInt32            nIndexCount, nSize, nVertexCount, nVertexStride, nSourceElementCount;
    D3DVERTEXELEMENT9 * pSourceFormat = CG_NULL, * pDeclarator = CG_NULL;
    cgUInt32          * pIndices      = CG_NULL;
    cgByte            * pVertices     = CG_NULL;

    // Dispose of any prior mesh data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // Handle exceptions
    try
    {
        // Load the primary mesh data entry.
        prepareQueries();
        mLoadMesh.bindParameter( 1, nSourceRefId );
        if ( !mLoadMesh.step() || !mLoadMesh.nextRow() )
            throw cgExceptions::ResultException( _T("Failed to retrieve mesh data. World database has potentially become corrupt."), cgDebugSource() );

        // Retrieve the current 'soft' reference count for this component
        // if we are literally wrapping the database entry.
        if ( nSourceRefId == mReferenceId )
            mLoadMesh.getColumn( _T("RefCount"), mSoftRefCount );

        // Retrieve the declarator
        mLoadMesh.getColumn( _T("Declaration"), (void**)&pSourceFormat, nSize );
        if ( pSourceFormat == CG_NULL || (nSize % sizeof(D3DVERTEXELEMENT9)) != 0 )
            throw cgExceptions::ResultException( _T("Mesh object geometry data contained invalid or corrupt format description."), cgDebugSource() );
        nSourceElementCount = nSize / sizeof(D3DVERTEXELEMENT9);
        
        // Construct the final vertex format (requires terminating entry).
        pDeclarator = new D3DVERTEXELEMENT9[nSourceElementCount + 1];
        memcpy( pDeclarator, pSourceFormat, nSize );
        memset( &pDeclarator[nSourceElementCount], 0, sizeof(D3DVERTEXELEMENT9) );
        pDeclarator[nSourceElementCount].Stream = 0xFF;
        pDeclarator[nSourceElementCount].Type   = D3DDECLTYPE_UNUSED;
        mVertexFormat = cgVertexFormat::formatFromDeclarator( pDeclarator );
        delete []pDeclarator;

        // Get index data.
        mLoadMesh.getColumn( _T("IndexCount"), nIndexCount );
        mLoadMesh.getColumn( _T("Indices"), (void**)&pIndices, nSize );
        if ( pIndices == CG_NULL || (nIndexCount % 3) != 0 || (nSize != (nIndexCount * sizeof(cgUInt32))) )
            throw cgExceptions::ResultException( _T("Mesh object geometry data contained invalid or corrupt index information."), cgDebugSource() );

        // Copy index data over.
        mSystemIB = new cgUInt32[nIndexCount];
        memcpy( mSystemIB, pIndices, nIndexCount * sizeof(cgUInt32) );
        mFaceCount = nIndexCount / 3;

        // We're done with the data from the mesh query
        mLoadMesh.reset();

        // Get the single supported stream data entry (StreamIndex=0).
        mLoadStreams.bindParameter( 1, nSourceRefId );
        if ( !mLoadStreams.step() || !mLoadStreams.nextRow() )
            throw cgExceptions::ResultException( _T("Mesh object geometry data contained invalid or corrupt stream information."), cgDebugSource() );

        // Retrieve the stream information.
        mLoadStreams.getColumn( _T("StreamId")   , mDBStreamId );
        mLoadStreams.getColumn( _T("EntryCount") , nVertexCount );
        mLoadStreams.getColumn( _T("EntryStride"), nVertexStride );
        mLoadStreams.getColumn( _T("StreamData") , (void**)&pVertices, nSize );
        if ( !pVertices || nVertexCount <= 0 || nVertexStride <= 0 || nSize != (nVertexCount * nVertexStride) )
             throw cgExceptions::ResultException( _T("Unable to decode mesh object geometry. Possible missing or corrupt data."), cgDebugSource() );

        // Copy vertex data over.
        mSystemVB = new cgByte[nVertexCount*nVertexStride];
        memcpy( mSystemVB, pVertices, nVertexCount * nVertexStride );
        mVertexCount = nVertexCount;
        
        // We're done with the data from the stream query.
        mLoadStreams.reset();

        // Now load the subset data.
        mLoadSubsets.reset();
        mLoadSubsets.bindParameter( 1, nSourceRefId );
        if ( !mLoadSubsets.step() )
            throw cgExceptions::ResultException( _T("Mesh object geometry data contained invalid or corrupt subset information."), cgDebugSource() );

        // Iterate through each row returned and generate new subsets
        // in addition to populating the triangle data array.
        DataGroupSubsetMap::iterator itDataGroup;
        MaterialSubsetMap::iterator itMaterial;
        mTriangleData.resize( mFaceCount );
        for ( ; mLoadSubsets.nextRow(); )
        {
            cgUInt32 nMaterial = 0;

            // Read subset data
            MeshSubset * pSubset = new MeshSubset();
            mLoadSubsets.getColumn( _T("MaterialId") , nMaterial );
            mLoadSubsets.getColumn( _T("DataGroup")  , pSubset->dataGroupId );
            mLoadSubsets.getColumn( _T("FaceStart")  , pSubset->faceStart );
            mLoadSubsets.getColumn( _T("FaceCount")  , pSubset->faceCount );
            mLoadSubsets.getColumn( _T("VertexStart"), pSubset->vertexStart );
            mLoadSubsets.getColumn( _T("VertexCount"), pSubset->vertexCount );

            // Select the final valid material.
            if ( nMaterial > 0 )
            {
                if ( mManager->loadMaterial( &pSubset->material, mWorld, cgMaterialType::Standard, nMaterial, false, 0, cgDebugSource() ) == false )
                    throw cgExceptions::ResultException( _T("cgResourceManager::loadMaterial"), cgDebugSource() );
            
            } // End if material assigned
            else
            {
                // Use default material if none was selected
                generateDefaultMaterial( );
                pSubset->material = mDefaultMaterial;

            } // End if no material assigned
            
            // Add to list for fast linear access, and lookup table for sorted search.
            mMeshSubsets.push_back( pSubset );
            mSubsetLookup[ MeshSubsetKey( pSubset->material, pSubset->dataGroupId ) ] = pSubset;

            // Add to data group lookup table
            itDataGroup = mDataGroups.find( pSubset->dataGroupId );
            if ( itDataGroup == mDataGroups.end() )
                mDataGroups[ pSubset->dataGroupId ].push_back( pSubset );
            else
                itDataGroup->second.push_back( pSubset );

            // Add to the material lookup table.
            mMaterials[ pSubset->material ].push_back( pSubset );
            
            // Populate data for all triangles
            for ( cgUInt32 i = pSubset->faceStart; i < (cgUInt32)(pSubset->faceStart + pSubset->faceCount); ++i )
            {
                mTriangleData[i].material    = pSubset->material;
                mTriangleData[i].dataGroupId = pSubset->dataGroupId;

            } // Next Triangle

        } // Next Subset

        // We're done with the subset data.
        mLoadSubsets.reset();

        // Now load the skin binding data if any.
        mLoadSkinBindData.reset();
        mLoadSkinBindData.bindParameter( 1, nSourceRefId );
        if ( !mLoadSkinBindData.step() )
            throw cgExceptions::ResultException( _T("Mesh object geometry data contained invalid or corrupt skin binding information."), cgDebugSource() );

        // Iterate through each row returned and retrieve any skin influence data.
        cgSkinBindData::BoneArray aBoneInfluences;
        for ( ; mLoadSkinBindData.nextRow(); )
        {
            cgQuaternion qRotation;
            cgVector3 vScale, vShear, vPosition;

            // Read influence data
            cgSkinBindData::BoneInfluence * pBoneInfluence = new cgSkinBindData::BoneInfluence();
            aBoneInfluences.push_back( pBoneInfluence );
            mLoadSkinBindData.getColumn( _T("BoneIdentifier") , pBoneInfluence->boneIdentifier );
            mLoadSkinBindData.getColumn( _T("BindPosePositionX"), vPosition.x );
            mLoadSkinBindData.getColumn( _T("BindPosePositionY"), vPosition.y );
            mLoadSkinBindData.getColumn( _T("BindPosePositionZ"), vPosition.z );
            mLoadSkinBindData.getColumn( _T("BindPoseRotationX"), qRotation.x );
            mLoadSkinBindData.getColumn( _T("BindPoseRotationY"), qRotation.y );
            mLoadSkinBindData.getColumn( _T("BindPoseRotationZ"), qRotation.z );
            mLoadSkinBindData.getColumn( _T("BindPoseRotationW"), qRotation.w );
            mLoadSkinBindData.getColumn( _T("BindPoseShearXY"), vShear.x );
            mLoadSkinBindData.getColumn( _T("BindPoseShearXZ"), vShear.y );
            mLoadSkinBindData.getColumn( _T("BindPoseShearYZ"), vShear.z );
            mLoadSkinBindData.getColumn( _T("BindPoseScaleX"), vScale.x );
            mLoadSkinBindData.getColumn( _T("BindPoseScaleY"), vScale.y );
            mLoadSkinBindData.getColumn( _T("BindPoseScaleZ"), vScale.z );
            
            // Compose remaining influence data.
            pBoneInfluence->bindPoseTransform.compose( vScale, vShear, qRotation, vPosition );

        } // Next influence

        // We're done with the skin binding data.
        mLoadSkinBindData.reset();

        // If any bone influences were found, recreate the skin binding data.
        if ( !aBoneInfluences.empty() )
        {
            mSkinBindData = new cgSkinBindData();
            for ( size_t i = 0; i < aBoneInfluences.size(); ++i )
                mSkinBindData->addBone( aBoneInfluences[i] );
            aBoneInfluences.clear();
        
        } // End if has influences

        // Now load the bone palette data if any.
        mLoadBonePalettes.reset();
        mLoadBonePalettes.bindParameter( 1, nSourceRefId );
        if ( !mLoadBonePalettes.step() )
            throw cgExceptions::ResultException( _T("Mesh object geometry data contained invalid or corrupt bone palette information."), cgDebugSource() );

        // Iterate through each row returned and retrieve any bone palette data.
        for ( ; mLoadBonePalettes.nextRow(); )
        {
            cgUInt32 nMaxSize, nBoneCount, nMaterial, nDataGroupId;
            cgInt32  nMaxBlendIndex;

            // Load the palette properties
            mLoadBonePalettes.getColumn( _T("MaterialId"), nMaterial );
            mLoadBonePalettes.getColumn( _T("DataGroup"), nDataGroupId );
            mLoadBonePalettes.getColumn( _T("MaximumSize"), nMaxSize );
            mLoadBonePalettes.getColumn( _T("MaxBlendIndex"), nMaxBlendIndex );
            mLoadBonePalettes.getColumn( _T("BoneCount"), nBoneCount );

            // Load the bone indices.
            cgUInt32 * pBones = CG_NULL, nBoneDataSize;
            mLoadBonePalettes.getColumn( _T("BoneIndices") , (void**)&pBones, nBoneDataSize );
            if ( !pBones || nBoneCount <= 0 || nBoneDataSize != (nBoneCount * sizeof(cgUInt32) ) )
                throw cgExceptions::ResultException( _T("Unable to decode mesh object bone palette information. Possible missing or corrupt data."), cgDebugSource() );

            // Select the final valid material.
            cgMaterialHandle hMaterial;
            if ( nMaterial > 0 )
            {
                if ( !mManager->loadMaterial( &hMaterial, mWorld, cgMaterialType::Standard, nMaterial, false, 0, cgDebugSource() ) )
                    throw cgExceptions::ResultException( _T("cgResourceManager::loadMaterial"), cgDebugSource() );
            
            } // End if material assigned
            else
            {
                // Use default material if none was selected
                generateDefaultMaterial( );
                hMaterial = mDefaultMaterial;

            } // End if no material assigned
            
            // Recreate the palette object
            cgBonePalette * pPalette = new cgBonePalette( nMaxSize );
            pPalette->setDataGroup( nDataGroupId );
            pPalette->setMaximumBlendIndex( nMaxBlendIndex );
            pPalette->setMaterial( hMaterial );
            
            // Assign bones
            if ( nBoneCount )
            {
                cgUInt32Array aBones( nBoneCount );
                memcpy( &aBones[0], pBones, nBoneDataSize );
                pPalette->assignBones( aBones );
            
            } // End if has bones
            
            // Add to palette list
            mBonePalettes.push_back( pPalette );

        } // Next palette

        // We're done with bone palette data.
        mLoadBonePalettes.reset();

        // ToDo: 9999 - This will eventually be removed.
        // Build the final list of materials used by this mesh (render control batching 
        // system requires that we cache this information in a specific format).
        mObjectMaterials.clear();
        mObjectMaterials.reserve( mMaterials.size() );
        for ( itMaterial = mMaterials.begin(); itMaterial != mMaterials.end(); ++itMaterial )
            mObjectMaterials.push_back( itMaterial->first );

        // Generate the bounding box data for the new geometry.
        cgInt32 nPositionOffset = mVertexFormat->getElementOffset(D3DDECLUSAGE_POSITION);
        if ( nPositionOffset >= 0 )
        {
            cgByte * pSrc = mSystemVB + nPositionOffset;
            for ( cgUInt32 i = 0; i < mVertexCount; ++i, pSrc += nVertexStride )
                mBoundingBox.addPoint( *((cgVector3*)pSrc) );
        
        } // End if has position

        // If any of the bone palettes exceed the maximum supported size, we need to rebind.
        // Warning: This can impact loading performance and should be avoided where possible.
        bool bHardwareCopy = true;
        bool bOptimize = true;
        cgRenderingCapabilities * pDriverCaps = mManager->getRenderDriver()->getCapabilities();
        cgUInt32 nMaxVBTs = pDriverCaps->getMaxBlendTransforms();
        for ( size_t i = 0; i < mBonePalettes.size(); ++i )
        {
            if ( mBonePalettes[i]->getBones().size() > nMaxVBTs )
            {
                cgAppLog::write( cgAppLog::Warning, _T("Performance Warning: Skinned mesh 0x%x contained a palette whose size (%i) exceeded the maximum number of blend transforms supported by the current render driver (%i) and the skin has been rebound.\n"), nSourceRefId, mBonePalettes[i]->getBones().size(), nMaxVBTs );

                // The skin information now needs to be rebound. In order to achieve this
                // the mesh data must be 'reverse engineered', and then the resulting
                // skin data re-bound. We can do so by rolling back the preparation with
                // a call to 'prepareMesh()', and then rebind with 'bindSkin()'. The remaining
                // mesh preparation will be performed automatically and thus we don't need
                // to generate vertex and index buffers in this case.
                mPrepareStatus = cgMeshStatus::Prepared;
                bHardwareCopy = false;
                prepareMesh( mVertexFormat, true );
                bindSkin( *mSkinBindData, true );
                break;
            
            } // End if oversize
        
        } // Next Palette

        // Create hardware VB/IB as necessary. 
        if ( bHardwareCopy )
        {
            // Create the vertex buffer.
            cgUInt32 nBufferSize = mVertexCount * mVertexFormat->getStride();
            if ( !mManager->createVertexBuffer( &mHardwareVB, nBufferSize, cgBufferUsage::WriteOnly, 
                                                   mVertexFormat, cgMemoryPool::Default, cgDebugSource() ) )
                throw cgExceptions::ResultException( cgString::format( _T("Failed to create vertex buffer of size %i bytes during 'loadMesh' call."), nBufferSize), cgDebugSource() );
            
            // Create the index buffer.
            nBufferSize = mFaceCount * 3 * sizeof(cgUInt32);
            if ( !mManager->createIndexBuffer( &mHardwareIB, nBufferSize, cgBufferUsage::WriteOnly, 
                                                 cgBufferFormat::Index32, cgMemoryPool::Default, cgDebugSource() ) )
                throw cgExceptions::ResultException( cgString::format( _T("Failed to create index buffer of size %i bytes during 'loadMesh' call."), nBufferSize), cgDebugSource() );

            // We won't fill these directly, but will instead  mark them as lost and /try/ to 
            // restore them (restoreBuffers()). If this fails, they will still get correctly 
            // restored just prior to rendering.
            mHardwareVB.setResourceLost( true );
            mHardwareIB.setResourceLost( true );
            
            // Attempt to populate hardware buffers.
            restoreBuffers();
            
        } // End if bHardwareCopy

        // If we are not simply wrapping the original database entry (i.e. the reference
        // identifier of this mesh resource doesn't match the source identifier) then we 
        // potentially need to serialize this data to the database next time 'serializeMesh()'
        // is called (assuming we are not an internal resource). See 'loadResource()'.
        if ( !isInternalReference() && nSourceRefId != mReferenceId )
        {
            // We are not wrapping. Everything should be serialized.
            mMeshSerialized       = false;
            mDBIndicesDirty       = true;
            mDBFormatDirty        = true;
            mDBVertexStreamDirty  = true;
            mDBSubsetsDirty       = true;
            mDBSkinDataDirty      = true;
            mDBBonePalettesDirty  = true;
            mDBStreamId           = 0;

        } // End if not wrapping
        else
        {
            // We are wrapping. Everything is already serialized.
            mMeshSerialized       = true;
            mDBIndicesDirty       = false;
            mDBFormatDirty        = false;
            mDBVertexStreamDirty  = false;
            mDBSubsetsDirty       = false;
            mDBSkinDataDirty      = false;
            mDBBonePalettesDirty  = false;
        
        } // End if wrapping
            
        // The mesh is now prepared
        mPrepareStatus   = cgMeshStatus::Prepared;
        mResourceLoaded = true;
        mHardwareMesh   = bHardwareCopy;
        mOptimizedMesh  = bOptimize;

        // Success!
        return true;

    } // End Try Block

    catch ( const cgExceptions::ResultException & e )
    {
        // Release any pending read operations.
        mLoadMesh.reset();
        mLoadStreams.reset();
        mLoadSubsets.reset();
        mLoadSkinBindData.reset();
        mLoadBonePalettes.reset();

        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;

    } // End Catch Block
}

//-----------------------------------------------------------------------------
// Name : serializeMesh() (Protected)
/// <summary>
/// Write or update the mesh data stored in the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::serializeMesh( )
{
    bool bTransaction = false;

    // Bail if we are not allowed to serialize any data.
    if ( !shouldSerialize() )
        return true;

    // Catch exceptions
    try
    {
        // Make sure database tables exist.
        if ( !mMeshSerialized && !createTypeTables( RTID_MeshResource ) )
            throw cgExceptions::ResultException( _T("cgWorldResourceComponent::createTypeTables"), cgDebugSource() );

        // If no mesh data has yet been inserted, serialize everything.
        prepareQueries();
        if ( !mMeshSerialized )
        {
            // Start a new transaction if we have not already.
            if ( !bTransaction )
                mWorld->beginTransaction( _T("serializeMesh") );
            bTransaction = true;

            // Mesh entry does not exist at all at this stage so insert it.
            mInsertMesh.bindParameter( 1, mReferenceId );
            mInsertMesh.bindParameter( 2, (cgUInt32)0 ); // ToDo: 9999 - Flags
            
            // Bind vertex format data.
            if ( !mVertexFormat )
                mInsertMesh.bindParameter( 3, CG_NULL, 0 );
            else
                mInsertMesh.bindParameter( 3, mVertexFormat->getDeclarator(), sizeof(D3DVERTEXELEMENT9) * mVertexFormat->getElementCount() );

            // Bind index data.
            if ( !mSystemIB )
            {
                mInsertMesh.bindParameter( 4, (cgUInt32)0 );
                mInsertMesh.bindParameter( 5, CG_NULL, 0 );
            
            } // End if no index data
            else
            {
                mInsertMesh.bindParameter( 4, mFaceCount * 3 );
                mInsertMesh.bindParameter( 5, mSystemIB, mFaceCount * 3 * sizeof(cgUInt32) );

            } // End if has index data

            // Database ref count (just in case it has already been adjusted)
            mInsertMesh.bindParameter( 6, mSoftRefCount );

            // Process!
            if ( !mInsertMesh.step( true ) )
            {
                cgString strError;
                mInsertMesh.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to insert data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Vertex streams, subsets and skin binding data should be serialized for the first time.
            mDBVertexStreamDirty = true;
            mDBSubsetsDirty      = true;
            mDBSkinDataDirty     = true;
            mDBBonePalettesDirty = true;

            // Indices and format have been serialized
            mDBIndicesDirty      = false;
            mDBFormatDirty       = false;
            mMeshSerialized      = true;
        
        } // End if not yet serialized
        else
        {
            // Updated indices?
            if ( mDBIndicesDirty )
            {
                // Start a new transaction if we have not already.
                if ( !bTransaction )
                    mWorld->beginTransaction( _T("serializeMesh") );
                bTransaction = true;

                // Mesh has previously been serialized, but the index data has been updated.
                if ( !mSystemIB )
                {
                    mUpdateIndices.bindParameter( 1, mFaceCount * 3 );
                    mUpdateIndices.bindParameter( 2, mSystemIB, mFaceCount * 3 * sizeof(cgUInt32) );

                } // End if no index data
                else
                {
                    mUpdateIndices.bindParameter( 1, mFaceCount * 3 );
                    mUpdateIndices.bindParameter( 2, mSystemIB, mFaceCount * 3 * sizeof(cgUInt32) );

                } // End if has index data
                mUpdateIndices.bindParameter( 3, mReferenceId );
                
                // Process!
                if ( !mUpdateIndices.step( true ) )
                {
                    cgString strError;
                    mUpdateIndices.getLastError( strError );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to update index data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                } // End if failed

                // Indices have been serialized
                mDBIndicesDirty = false;

            } // End if indices dirty

            // Vertex format must be updated?
            if ( mDBFormatDirty )
            {
                // Start a new transaction if we have not already.
                if ( !bTransaction )
                    mWorld->beginTransaction( _T("serializeMesh") );
                bTransaction = true;
                
                // Mesh has previously been serialized, but the index data has been updated.
                mUpdateVertexFormat.bindParameter( 1, mReferenceId );
            
                // Bind vertex format data.
                if ( !mVertexFormat )
                    mUpdateVertexFormat.bindParameter( 2, CG_NULL, 0 );
                else
                    mUpdateVertexFormat.bindParameter( 2, mVertexFormat->getDeclarator(), sizeof(D3DVERTEXELEMENT9) * mVertexFormat->getElementCount() );

                // Process!
                if ( !mUpdateVertexFormat.step( true ) )
                {
                    cgString strError;
                    mUpdateVertexFormat.getLastError( strError );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to update vertex format data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                } // End if failed

                // Format has been updated.
                mDBFormatDirty = true;

            }// End if vertex format dirty

        } // End if updating

        // Write subset data if it has been updated.
        if ( mDBSubsetsDirty )
        {
            // Start a new transaction if we have not already.
            if ( !bTransaction )
                mWorld->beginTransaction( _T("serializeMesh") );
            bTransaction = true;

            // Remove old subsets from database.
            mDeleteSubsets.bindParameter( 1, mReferenceId );
            if ( !mDeleteSubsets.step( true ) )
            {
                cgString strError;
                mDeleteSubsets.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to delete prior subset data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Insert new subsets into database
            for ( size_t i = 0; i < mMeshSubsets.size(); ++i )
            {
                MeshSubset * pSubset = mMeshSubsets[i];
                
                // Resolve material reference identifier. Make sure that the
                // default material is never referenced in the physical database.
                cgUInt32 nMaterial = ( pSubset->material == mDefaultMaterial ) ? 0 : pSubset->material.getReferenceId();
                
                // Insert!
                mInsertSubset.bindParameter( 1, mReferenceId );
                mInsertSubset.bindParameter( 2, nMaterial );
                mInsertSubset.bindParameter( 3, pSubset->dataGroupId );
                mInsertSubset.bindParameter( 4, pSubset->faceStart );
                mInsertSubset.bindParameter( 5, pSubset->faceCount );
                mInsertSubset.bindParameter( 6, pSubset->vertexStart );
                mInsertSubset.bindParameter( 7, pSubset->vertexCount );
                if ( !mInsertSubset.step( true ) )
                {
                    cgString strError;
                    mInsertSubset.getLastError( strError );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to insert subset data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                } // End if failed

            } // Next Subset

            // Subsets have been serialized
            mDBSubsetsDirty = false;

        } // End if subsets dirty

        // Update vertex stream data if updated.
        if ( mDBVertexStreamDirty )
        {
            // Start a new transaction if we have not already.
            if ( !bTransaction )
                mWorld->beginTransaction( _T("serializeMesh") );
            bTransaction = true;

            // Any vertex data?
            if ( mSystemVB != CG_NULL )
            {
                // Stream already exists for this data entry?
                if ( mDBStreamId == 0 )
                {
                    // No stream already exists so create a new one.
                    mInsertStream.bindParameter( 1, mReferenceId );
                    mInsertStream.bindParameter( 2, (cgUInt32)0 ); // StreamIndex
                    mInsertStream.bindParameter( 3, (cgUInt32)mVertexCount );
                    mInsertStream.bindParameter( 4, (cgUInt32)mVertexFormat->getStride() );
                    mInsertStream.bindParameter( 5, mSystemVB, mVertexCount * mVertexFormat->getStride() );
                    if ( !mInsertStream.step( true ) )
                    {
                        cgString strError;
                        mInsertStream.getLastError( strError );
                        throw cgExceptions::ResultException( cgString::format(_T("Failed to insert vertex stream data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                    } // End if failed

                    // Record new stream entry identifier
                    mDBStreamId = (cgUInt32)mInsertStream.getLastInsertId();
                
                } // End if no stream
                else
                {
                    // Just update the existing stream
                    mUpdateStream.bindParameter( 1, mVertexCount );
                    mUpdateStream.bindParameter( 2, (cgUInt32)mVertexFormat->getStride() );
                    mUpdateStream.bindParameter( 3, mSystemVB, mVertexCount * mVertexFormat->getStride() );
                    mUpdateStream.bindParameter( 4, mDBStreamId );
                    if ( !mUpdateStream.step( true ) )
                    {
                        cgString strError;
                        mUpdateStream.getLastError( strError );
                        throw cgExceptions::ResultException( cgString::format(_T("Failed to update vertex stream data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                    } // End if failed

                }  // End if existing stream
                
            } // End if mesh allocated
            else
            {
                // Stream was potentially destroyed. Delete stream data.
                prepareQueries();
                mDeleteStreams.bindParameter( 1, mReferenceId );
                if ( !mDeleteStreams.step( true ) )
                {
                    cgString strError;
                    mDeleteStreams.getLastError( strError );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to delete prior vertex stream data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                } // End if failed
                mDBStreamId = 0;

            } // End if no mesh

            // Vertex stream has been serialized
            mDBVertexStreamDirty = false;

        } // End if vertices dirty

        // Write skin binding data if it has been updated.
        if ( mDBSkinDataDirty )
        {
            // Start a new transaction if we have not already.
            if ( !bTransaction  )
                mWorld->beginTransaction( _T("serializeMesh") );
            bTransaction = true;

            // Remove old binding data from database.
            mDeleteSkinBindData.bindParameter( 1, mReferenceId );
            if ( !mDeleteSkinBindData.step( true ) )
            {
                cgString strError;
                mDeleteSkinBindData.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to delete prior skin influence data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Insert new influences into database
            if ( mSkinBindData )
            {
                const cgSkinBindData::BoneArray & aBones = mSkinBindData->getBones();
                for ( size_t i = 0; i < aBones.size(); ++i )
                {
                    const cgSkinBindData::BoneInfluence * pBone = aBones[i];

                    // Decompose the bind pose transform for serialization
                    cgQuaternion qRotation;
                    cgVector3 vScale, vShear, vPosition;
                    pBone->bindPoseTransform.decompose( vScale, vShear, qRotation, vPosition );
                    
                    // Insert!
                    mInsertSkinInfluence.bindParameter( 1, mReferenceId );
                    mInsertSkinInfluence.bindParameter( 2, pBone->boneIdentifier );
                    mInsertSkinInfluence.bindParameter( 3, vPosition.x );
                    mInsertSkinInfluence.bindParameter( 4, vPosition.y );
                    mInsertSkinInfluence.bindParameter( 5, vPosition.z );
                    mInsertSkinInfluence.bindParameter( 6, qRotation.x );
                    mInsertSkinInfluence.bindParameter( 7, qRotation.y );
                    mInsertSkinInfluence.bindParameter( 8, qRotation.z );
                    mInsertSkinInfluence.bindParameter( 9, qRotation.w );
                    mInsertSkinInfluence.bindParameter( 10, vShear.x );
                    mInsertSkinInfluence.bindParameter( 11, vShear.y );
                    mInsertSkinInfluence.bindParameter( 12, vShear.z );
                    mInsertSkinInfluence.bindParameter( 13, vScale.x );
                    mInsertSkinInfluence.bindParameter( 14, vScale.y );
                    mInsertSkinInfluence.bindParameter( 15, vScale.z );
                    if ( !mInsertSkinInfluence.step( true ) )
                    {
                        cgString strError;
                        mInsertSkinInfluence.getLastError( strError );
                        throw cgExceptions::ResultException( cgString::format(_T("Failed to insert skin influence data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                    } // End if failed

                } // Next Influence
            
            } // End if has data

            // Skin binding data has been serialized
            mDBSkinDataDirty = false;

        } // End if skin data dirty

        // Write bone palette data if it has been updated.
        if ( mDBBonePalettesDirty )
        {
            // Start a new transaction if we have not already.
            if ( !bTransaction  )
                mWorld->beginTransaction( _T("serializeMesh") );
            bTransaction = true;

            // Remove old palettes from database.
            mDeleteBonePalettes.bindParameter( 1, mReferenceId );
            if ( !mDeleteBonePalettes.step( true ) )
            {
                cgString strError;
                mDeleteBonePalettes.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to delete prior bone palette data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Insert new bone palettes into database
            if ( !mBonePalettes.empty() )
            {
                for ( size_t i = 0; i < mBonePalettes.size(); ++i )
                {
                    cgBonePalette * pPalette = mBonePalettes[i];

                    // Resolve material reference identifier. Make sure that the
                    // default material is never referenced in the physical database.
                    cgUInt32 nMaterial = ( pPalette->getMaterial() == mDefaultMaterial ) ? 0 : pPalette->getMaterial().getReferenceId();

                    // Insert!
                    const cgUInt32Array & aBoneIndices = pPalette->getBones();
                    mInsertBonePalette.bindParameter( 1, mReferenceId );
                    mInsertBonePalette.bindParameter( 2, nMaterial );
                    mInsertBonePalette.bindParameter( 3, pPalette->getDataGroup() );
                    mInsertBonePalette.bindParameter( 4, pPalette->getMaximumSize() );
                    mInsertBonePalette.bindParameter( 5, pPalette->getMaximumBlendIndex() );
                    mInsertBonePalette.bindParameter( 6, (cgUInt32)aBoneIndices.size() );
                    if ( aBoneIndices.empty() )
                        mInsertBonePalette.bindParameter( 7, CG_NULL, 0 );
                    else
                        mInsertBonePalette.bindParameter( 7, &aBoneIndices[0], aBoneIndices.size() * sizeof(cgUInt32) );
                    if ( !mInsertBonePalette.step( true ) )
                    {
                        cgString strError;
                        mInsertBonePalette.getLastError( strError );
                        throw cgExceptions::ResultException( cgString::format(_T("Failed to insert bone palette data for mesh resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                    } // End if failed

                } // Next palette
            
            } // End if has data

            // Bone palettes have been serialized
            mDBBonePalettesDirty = false;

        } // End if bone palettes dirty

        // Commit any changes recorded.
        if ( bTransaction )
            mWorld->commitTransaction( _T("serializeMesh") );

        // Success!
        return true;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        // Roll back any transaction
        if ( bTransaction )
            mWorld->rollbackTransaction( _T("serializeMesh") );
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch

}

//-----------------------------------------------------------------------------
// Name : bindSkin ()
// Desc : Given the specified skin binding data, convert the internal mesh data 
//        into the format required for skinning.
// Note : If the mesh has already been prepared, it will be rolled back prior
//        to binding the skin. This adds some overhead, so it is preferable to
//        avoid finalizing the mesh until the skin has been bound.
// Note : Take care to ensure that the skin data's vertex assignments have
//        been remapped correctly if you have finalized the mesh previously.
//        If you are not rolling back a prepared mesh, this method will
//        automatically take care of remapping the input data.
//-----------------------------------------------------------------------------
bool cgMesh::bindSkin( const cgSkinBindData & BindData, bool bFinalizeMesh /* = true */ )
{
    cgSkinBindData::BindVertexArray VertexTable;
    BoneCombinationMap              BoneCombinations;

    // Get access to required systems and limits
    cgRenderDriver * pDriver = getRenderDriver();
    cgRenderingCapabilities * pDriverCaps = pDriver->getCapabilities();
    cgUInt32 nPaletteSize = pDriverCaps->getMaxBlendTransforms();

    // Destroy any previous palette entries.
    for ( size_t i = 0; i < mBonePalettes.size(); ++i )
        delete mBonePalettes[i];
    mBonePalettes.clear();

    // Duplicate the binding data as required (no need to perform this step 
    // if someone is passing in the existing binding data a second time).
    if ( mSkinBindData != &BindData )
    {
        if ( mSkinBindData )
            delete mSkinBindData;
        mSkinBindData = new cgSkinBindData( BindData );

    } // End if !no-op

    // If the mesh has already been prepared, roll it back.
    if ( mPrepareStatus == cgMeshStatus::Prepared )
        prepareMesh( mVertexFormat, true );
    
    // Exception handling.
    try
    {
        FaceInfluences             UsedBones;
        MaterialUsageMap           Materials;
        MaterialUsageMap::iterator itMaterial;

        // Build a list of all bone indices and associated weights for each vertex.
        mSkinBindData->buildVertexTable( mPrepareData.vertexCount, mPrepareData.vertexRecords, VertexTable );

        // Vertex based influences are no longer required. Clear them to save space.
        mSkinBindData->clearVertexInfluences();

        // Now build a list of unique bone combinations that influences each face in the mesh
        TriangleArray & TriData = mPrepareData.triangleData;
        for ( cgUInt32 i = 0; i < mPrepareData.triangleCount; ++i )
        {
            // Clear out any previous bone references and set up for new run.
            UsedBones.bones.clear();
            UsedBones.material = TriData[i].material;

            // We need a list of the unique materials used by the mesh too.
            itMaterial = Materials.find( UsedBones.material );
            if ( itMaterial == Materials.end() )
                Materials.insert( MaterialUsageMap::value_type( UsedBones.material, 0 ) );
            else
                itMaterial->second++;

            // Collect all unique bone indices from each of the three face vertices.
            for ( cgUInt32 j = 0; j < 3; ++j )
            {
                cgUInt32 nVertex = TriData[i].indices[j];

                // Add each influencing bone if unique.
                for ( size_t k = 0; k < VertexTable[nVertex]->influences.size(); ++k )
                    UsedBones.bones[ VertexTable[nVertex]->influences[k] ] = 1; // Just set to any value, we're only using a key/value dictionary for accelleration

            } // Next Vertex

            // Now that we have a unique list of bones that influence this face,
            // determine if any faces with identical influences already exist and 
            // add it to the list. Alternatively this may be the first face with
            // these exact influences, in which case a new entry will be created.
            BoneCombinationMap::iterator itCombination = BoneCombinations.find( BoneCombinationKey( &UsedBones ) );
            if ( itCombination == BoneCombinations.end() )
            {
                cgUInt32Array  * pFaceList   = new cgUInt32Array();
                FaceInfluences * pInfluences = new FaceInfluences( UsedBones );
                BoneCombinations.insert( BoneCombinationMap::value_type( BoneCombinationKey( pInfluences ), pFaceList ) );

                // ToDo: SciFi_Test_Char2.psf vs US Ranger.psf gives 99 vs 68 combinations (Former has one extra material?)
                /*cgMaterial * pMaterial = (cgMaterial*)UsedBones.material.getResource();
                String strOutput = StringUtility::format( _T("%i - New combination added with %i bones for material %i.\n    "), (int)BoneCombinations.size(), (int)pInfluences->bones.size(), pMaterial->getMaterialId() );
                for ( BoneIndexMap::iterator itFoo = pInfluences->bones.begin(); itFoo != pInfluences->bones.end(); ++itFoo )
                {
                    strOutput += StringUtility::format( _T("%i,"), itFoo->first );
                }
                strOutput += _T("\n");

                cgAppLog::write( cgAppLog::Debug, strOutput.c_str() );*/
                
                // Assign face to this combination.
                pFaceList->push_back( i );
            
            } // End if doesn't exist yet
            else
            {
                // Assign face to this combination.
                itCombination->second->push_back( i );
            
            } // End if already exists

        } // Next Face

        // We now have a complete list of the unique combinations of bones that
        // influence every face in the mesh. The next task is to combine these
        // unique lists into as few combined bone "palettes" as possible, containing
        // as many bones as possible. To do so, we must continuously search for
        // face influence combinations that will fit into any existing palettes.
        for ( itMaterial = Materials.begin(); itMaterial != Materials.end(); ++itMaterial )
        {
            // Keep searching until we have consumed all unique face influence combinations.
            for ( ; BoneCombinations.empty() == false; )
            {
                BoneCombinationMap::iterator itCombination;
                BoneCombinationMap::iterator itBestCombination    = BoneCombinations.end();
                BoneCombinationMap::iterator itLargestCombination = BoneCombinations.end();
                int                          nMaxCommon = -1, nMaxBones = -1;
                int                          nPalette   = -1;

                // Search face influences for the next best combination to add to a palette. We search
                // for two specific properties; A) does it share a large amount in common with an existing 
                // palette and B) does this have the largest list of bones. We'll work out which of these 
                // properties we're interested in later.
                for ( itCombination = BoneCombinations.begin(); itCombination != BoneCombinations.end(); ++itCombination )
                {
                    FaceInfluences * pInfluences = itCombination->first.influences;
                    cgUInt32Array  * pFaceList   = itCombination->second;

                    // Skip any batch that does not share the same material we are interested in
                    if ( pInfluences->material != itMaterial->first )
                        continue;

                    // Record the combination with the largest set of bones in case we can't
                    // find a suitable palette to insert any influence into at this point.
                    if ( (int)pInfluences->bones.size() > nMaxBones )
                    {
                        itLargestCombination = itCombination;
                        nMaxBones            = (int)pInfluences->bones.size();

                    } // End if largest

                    // Test against each palette
                    for ( size_t i = 0; i < mBonePalettes.size(); ++i )
                    {
                        cgInt32         nCommonBones, nAdditionalBones, nRemainingSpace;
                        cgBonePalette * pPalette = mBonePalettes[i];

                        // Skip if this palette is assigned to an alternate material
                        if ( pPalette->getMaterial() != itMaterial->first )
                            continue;

                        // Also compute how the combination might fit into this palette if at all
                        pPalette->computePaletteFit( pInfluences->bones, nRemainingSpace, nCommonBones, nAdditionalBones );

                        // Now that we know how many bones this batch of faces has in common with the
                        // palette, and how many new bones it will add, we can work out if it's a good
                        // idea to use this batch next (assuming it will fit at all).
                        if ( nAdditionalBones <= nRemainingSpace )
                        {
                            // Does it have the most in common so far with this palette?
                            if ( nCommonBones > nMaxCommon )
                            {
                                nMaxCommon        = nCommonBones;
                                itBestCombination = itCombination;
                                nPalette          = (cgInt32)i;

                            } // End if better choice

                        } // End if smaller

                    } // Next Palette

                } // Next influence combination

                // If nothing was found then we have run out of influences relevant to the source material
                if ( itLargestCombination == BoneCombinations.end() )
                    break;

                // We should hopefully have selected at least one good candidate for insertion
                // into an existing palette that we can now process. If not, we must create
                // a new palette into which we will store the combination with the largest
                // number of bones discovered.
                if ( nPalette >= 0 )
                {
                    FaceInfluences * pInfluences = itBestCombination->first.influences;
                    cgUInt32Array  * pFaceList   = itBestCombination->second;

                    // At least one good combination was found that can be added to an existing palette.
                    mBonePalettes[nPalette]->assignBones( pInfluences->bones, *pFaceList );

                    // We should now remove the selected combination.
                    BoneCombinations.erase( itBestCombination );
                    delete pInfluences;
                    delete pFaceList;
                
                } // End if found fit
                else
                {
                    FaceInfluences * pInfluences = itLargestCombination->first.influences;
                    cgUInt32Array  * pFaceList   = itLargestCombination->second;

                    // No combination of face influences was able to fit into an existing palette.
                    // We must generate a new one and store the largest combination discovered.
                    cgBonePalette * pNewPalette  = new cgBonePalette( nPaletteSize );
                    pNewPalette->setMaterial( itMaterial->first );
                    pNewPalette->setDataGroup( (cgUInt32)mBonePalettes.size() );
                    pNewPalette->assignBones( pInfluences->bones, *pFaceList );
                    mBonePalettes.push_back( pNewPalette );

                    // We should now remove the selected combination.
                    BoneCombinations.erase( itLargestCombination );
                    delete pInfluences;
                    delete pFaceList;

                } // End if none found
            
            } // Next iteration

        } // Next Material

        // Bone palettes are now fully constructed. Mesh vertices must now be split as 
        // necessary in cases where they are shared by faces in alternate palettes.
        for ( size_t i = 0; i < mBonePalettes.size(); ++i )
        {
            cgBonePalette * pPalette = mBonePalettes[i];
            cgUInt32Array & Faces    = pPalette->getInfluencedFaces();
            for ( size_t j = 0; j < Faces.size(); ++j )
            {
                cgSkinBindData::VertexData * pData = CG_NULL;

                // Assign face to correct data group.
                cgUInt32 nFaceIndex = Faces[j];
                TriData[nFaceIndex].dataGroupId = pPalette->getDataGroup();

                // Update vertex information
                for ( cgUInt32 k = 0; k < 3; ++k )
                {
                    pData = VertexTable[ TriData[nFaceIndex].indices[k] ];

                    // Record the largest blend index necessary for processing this palette.
                    if ( ((int)pData->influences.size() - 1) > pPalette->getMaximumBlendIndex() )
                        pPalette->setMaximumBlendIndex( min( 3, ((int)pData->influences.size() - 1) ) );

                    // If this vertex has already been assigned to an alternative
                    // palette, then it must be duplicated.
                    if ( pData->palette != -1 && pData->palette != (cgInt32)i )
                    {
                        cgUInt32 nNewIndex = (cgUInt32)VertexTable.size();

                        // Split vertex
                        cgSkinBindData::VertexData * pNewVertex = new cgSkinBindData::VertexData( *pData );
                        pNewVertex->palette = (cgInt32)i;
                        VertexTable.push_back( pNewVertex );
                        
                        // Update triangle indices
                        TriData[nFaceIndex].indices[k] = nNewIndex;
                    
                    } // End if already assigned
                    else
                    {
                        // Assign to palette
                        pData->palette = (cgInt32)i;
                    
                    } // End if not assigned

                } // Next triangle vertex

            } // Next Face

            // We've finished with the palette's assigned face list. 
            // This is only a temporary array.
            pPalette->clearInfluencedFaces();

        } // Next Palette Entry

        // Vertex format must be adjusted to include blend weights and indices 
        // (if they don't already exist).
        cgVertexFormat NewFormat( *mVertexFormat );
        cgVertexFormat * pOriginalFormat = mVertexFormat;
        bool bHasWeights = (NewFormat.getElement( D3DDECLUSAGE_BLENDWEIGHT ) != CG_NULL );
        bool bHasIndices = (NewFormat.getElement( D3DDECLUSAGE_BLENDINDICES ) != CG_NULL );
        if ( !bHasWeights || !bHasIndices )
        {
            if ( !bHasWeights )
                NewFormat.addVertexElement( D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_BLENDWEIGHT );
            if ( !bHasIndices )
                NewFormat.addVertexElement( D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_BLENDINDICES );

            // Add to format database.
            mVertexFormat = cgVertexFormat::formatFromDeclarator( NewFormat.getDeclarator() );

            // Vertex format was updated.
            mDBFormatDirty = true;
        
        } // End if needs new format

        // Get access to final data offset information.
        cgInt nBlendIndicesOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_BLENDINDICES );
        cgInt nBlendWeightsOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_BLENDWEIGHT );
        cgInt nVertexStride       = (cgInt)mVertexFormat->getStride();

        // Now we need to update the vertex data as required. It's possible
        // that the buffer also needs to grow if / when vertices were split.
        // First convert original data into the new format and also ensure
        // that it is large enough to contain our entire final data set.
        cgUInt32 nOriginalVertexCount = (cgUInt32)mPrepareData.vertexCount;
        if ( *mVertexFormat != *pOriginalFormat )
        {
            // Format has changed, run conversion.
            cgByteArray OriginalBuffer( mPrepareData.vertexData );
            mPrepareData.vertexData.clear();
            mPrepareData.vertexData.resize( VertexTable.size() * nVertexStride );
            mPrepareData.vertexFlags.resize( VertexTable.size() );
            cgVertexFormat::convertVertices( &mPrepareData.vertexData[0], mVertexFormat,
                                             &OriginalBuffer[0], pOriginalFormat, nOriginalVertexCount );

        } // End if convert
        else
        {
            // No conversion required, just add space for the new vertices.
            mPrepareData.vertexData.resize( VertexTable.size() * nVertexStride );
            mPrepareData.vertexFlags.resize( VertexTable.size() );

        } // End if !convert

        // Populate with newly constructed information.
        cgByte * pSrcVertices = &mPrepareData.vertexData[0];
        for ( size_t i = 0; i < VertexTable.size(); ++i )
        {
            cgSkinBindData::VertexData * pData = VertexTable[i];
            cgBonePalette * pPalette = mBonePalettes[ pData->palette ];

            // If this is a new vertex, duplicate data from original vertex (it will have 
            // already been converted to its final format by this point).
            if ( i >= nOriginalVertexCount )
            {
                // This is a new vertex. Duplicate data from original vertex (it will have already been
                // converted to its final format by this point).
                memcpy( pSrcVertices + (i * nVertexStride), 
                        pSrcVertices + (pData->originalVertex * nVertexStride),
                        nVertexStride );

                // Also duplicate additional vertex data.
                mPrepareData.vertexFlags[i] = mPrepareData.vertexFlags[pData->originalVertex];
                
            } // End if new vertex

            // Assign bone indices (the index to the relevant entry in the palette,
            // not the main bone list index) and weights.
            cgVector4 vBlendWeights( 0, 0, 0, 0 );
            cgUInt32  nBlendIndices = 0, nMaxBones = min(4, pData->influences.size());
            for ( size_t j = 0; j < nMaxBones; ++j )
            {
                // Store vertex indices and weights
                nBlendIndices   |= (cgUInt32)((pPalette->translateBoneToPalette(pData->influences[j]) & 0xFF) << (8 * j));
                vBlendWeights[j] = pData->weights[j];
            
            } // Next Influence
            for ( size_t j = nMaxBones; j < 4; ++j )
                nBlendIndices |= (cgUInt32)(0xFF << (8 * j));
            
            // Push to vertex.
            *(cgUInt32*)(pSrcVertices + (i * nVertexStride) + nBlendIndicesOffset) = nBlendIndices;
            *(cgVector4*)(pSrcVertices + (i * nVertexStride) + nBlendWeightsOffset) = vBlendWeights;

        } // Next Vertex

        // Update vertex count to match final size.
        mPrepareData.vertexCount = (cgUInt32)VertexTable.size();

        // Clean up any data that is no longer required
        for ( size_t i = 0; i < VertexTable.size(); ++i )
        {
            if ( VertexTable[i] != CG_NULL )
                delete VertexTable[i];
        
        } // Next Vertex

        // Finally prepare the renderable data for the mesh
        bool bResult = false;
        if ( bFinalizeMesh == true )
            bResult = endPrepare( );
        else
            bResult = true;
        
        // Skin is now bound?
        return bResult;
        
    } // End try block

    catch ( const cgExceptions::ResultException & e )
    {
        // Don't forget to clean up any data that remains allocated.
        BoneCombinationMap::iterator itCombination;
        for ( itCombination = BoneCombinations.begin(); itCombination != BoneCombinations.end(); ++itCombination )
        {
            delete itCombination->first.influences;
            delete itCombination->second;
        
        } // Next Combination
        for ( size_t i = 0; i < VertexTable.size(); ++i )
        {
            if ( VertexTable[i] != CG_NULL )
                delete VertexTable[i];
        
        } // Next Vertex
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch
}

//-----------------------------------------------------------------------------
//  Name : prepareMesh ()
/// <summary>
/// Begin preparing the mesh. This consists of setting a vertex source
/// and adding primitives to the relevant buffers through the
/// 'AddPrimitive' method. Once preparation is complete, call 'endPrepare'
/// to allow the rendering buffers to be built.
/// Note : Calling prepareMesh will cause any existing data to be cleared.
/// The caller can however roll back an earlier call to 'endPrepare' by
/// passing a value of true to the 'bRollBackPrepare' parameter.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::prepareMesh( cgVertexFormat * pVertexFormat, bool bRollBackPrepare /* = false */, cgResourceManager * pManager /* = CG_NULL */ )
{
    // If we are already in the process of preparing, this is a no-op.
    if ( mPrepareStatus == cgMeshStatus::Preparing )
        return;

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // Should we roll back an earlier call to 'endPrepare' ?
    if ( mPrepareStatus == cgMeshStatus::Prepared && bRollBackPrepare == true )
    {
        // Reset required values.
        mPrepareData.triangleCount = 0;
        mPrepareData.triangleData.clear();
        mPrepareData.vertexCount = 0;
        mPrepareData.vertexData.clear();
        mPrepareData.vertexFlags.clear();
        mPrepareData.vertexRecords.clear();
        mPrepareData.computeNormals = false;
        mPrepareData.computeBinormals = false;
        mPrepareData.computeTangents = false;

        // We can release the prior prepared triangle data early as this information
        // will be reconstructed from the existing mesh subset table.
        mTriangleData.clear();

        // Release prior hardware buffers if they were constructed.
        mHardwareVB.close();
        mHardwareIB.close();

        // Set the size of the preparation buffer so that we can add
        // the existing buffer data to it.
        cgUInt32 nNewStride = pVertexFormat->getStride();
        mPrepareData.vertexData.resize( mVertexCount * nNewStride );
        mPrepareData.vertexCount = mVertexCount;

        // Create enough space in our triangle data array for 
        // the number of faces that existed in the prior final buffer.
        mPrepareData.triangleData.resize( mFaceCount );
        
        // Copy all of the vertex data back into the preparation 
        // structures, converting if necessary.
        if ( *pVertexFormat == *mVertexFormat )
            memcpy( &mPrepareData.vertexData[0], mSystemVB, mPrepareData.vertexData.size() );
        else
            cgVertexFormat::convertVertices( &mPrepareData.vertexData[0], pVertexFormat,
                                             mSystemVB, mVertexFormat, mVertexCount );

        // Clear out the vertex buffer
        delete []mSystemVB;
        mSystemVB    = CG_NULL;
        mVertexCount = 0;

        // Iterate through each subset and extract triangle data.
        SubsetArray::iterator itSubset;
        for ( itSubset = mMeshSubsets.begin(); itSubset != mMeshSubsets.end(); ++itSubset )
        {
            MeshSubset * pSubset = *itSubset;

            // Iterate through each face in the subset
            cgUInt32 * pCurrentIndex = &mSystemIB[ (pSubset->faceStart * 3) ];
            for ( cgInt32 i = 0; i < pSubset->faceCount; ++i, pCurrentIndex += 3 )
            {
                // Generate winding data
                Triangle & Data   = mPrepareData.triangleData[ mPrepareData.triangleCount++ ] ;
                Data.material    = pSubset->material;
                Data.dataGroupId = pSubset->dataGroupId;
                memcpy( Data.indices, pCurrentIndex, 3 * sizeof(cgUInt32) );

            } // Next Face

        } // Next subset

        // Release additional memory
        delete []mSystemIB;
        mSystemIB  = CG_NULL;
        mFaceCount = 0;

        // Determine which components the original vertex data actually contained.
        bool bSourceHasNormal   = (mVertexFormat->getElement( D3DDECLUSAGE_NORMAL ) != CG_NULL );
        bool bSourceHasBinormal = (mVertexFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
        bool bSourceHasTangent  = (mVertexFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );

        // The 'PreparationData::vertexFlags' array contains a record of the above for each vertex
        // that currently exists in the preparation buffer. This is required when performing processes 
        // such as the generation of vertex normals, etc.
        cgByte nVertexFlags = 0;
        if ( bSourceHasNormal ) 
            nVertexFlags |= PreparationData::SourceContainsNormal;
        if ( bSourceHasBinormal ) 
            nVertexFlags |= PreparationData::SourceContainsBinormal;
        if ( bSourceHasTangent ) 
            nVertexFlags |= PreparationData::SourceContainsTangent;
        
        // Record the information.
        mPrepareData.vertexFlags.resize( mPrepareData.vertexCount );
        for ( cgUInt32 i = 0; i < mPrepareData.vertexCount; ++i )
            mPrepareData.vertexFlags[i] = nVertexFlags;

        // If skin binding data was available (i.e. this is a skinned mesh), reverse engineer 
        // the existing data into a newly populated skin binding structure.
        if ( mSkinBindData )
        {
            // Clear out any previous influence data.
            mSkinBindData->clearVertexInfluences();

            // First, retrieve the offsets to the weight and bone palette index data
            // stored in each vertex in the mesh.
            cgInt nWeightOffset  = mVertexFormat->getElementOffset( D3DDECLUSAGE_BLENDWEIGHT );
            cgInt nIndexOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_BLENDINDICES );
            cgInt nVertexStride  = mVertexFormat->getStride();
            cgByte * pVertexData = &mPrepareData.vertexData.front();
        
            // Search through each bone palette to get influence data.
            cgSkinBindData::BoneArray & aBones = mSkinBindData->getBones();
            for ( size_t nPalette = 0; nPalette < mBonePalettes.size(); ++nPalette )
            {
                cgBonePalette * pPalette = mBonePalettes[nPalette];
                
                // Find the subset associated with this bone palette.
                MeshSubset * pSubset = mSubsetLookup[ MeshSubsetKey( pPalette->getMaterial(), pPalette->getDataGroup() ) ];

                // Process faces in this subset to retrieve referenced vertex data.
                cgInt32 nMaxBlendIndex = pPalette->getMaximumBlendIndex();
                cgUInt32Array aBoneReferences = pPalette->getBones();
                for ( cgInt32 nFace = pSubset->faceStart; nFace < (pSubset->faceStart + pSubset->faceCount); ++nFace )
                {
                    Triangle & Tri = mPrepareData.triangleData[ nFace ];
                    for ( size_t i = 0; i < 3; ++i )
                    {
                        cgFloat * pWeights = (cgFloat*)(pVertexData + (Tri.indices[i] * nVertexStride + nWeightOffset));
                        cgByte  * pIndices = (cgByte*)(pVertexData + (Tri.indices[i] * nVertexStride + nIndexOffset));

                        // Add influence data back to the referenced bones.
                        for ( cgInt32 j = 0; j <= nMaxBlendIndex; ++j )
                        {
                            if ( pIndices[j] != 0xFF )
                            {
                                cgSkinBindData::BoneInfluence * pBone = aBones[aBoneReferences[pIndices[j]]];
                                pBone->influences.push_back( cgSkinBindData::VertexInfluence( Tri.indices[i], pWeights[j] ) );

                                // Prevent duplicate insertions
                                pIndices[j] = 0xFF;

                            } // End if valid

                        } // Next blend reference
                        
                    } // Next triangle index

                } // Next face

            } // Next Palette

        } // End if is skin

        // Clean up heap allocated subset structures
        for ( itSubset = mMeshSubsets.begin(); itSubset != mMeshSubsets.end(); ++itSubset )
            delete *itSubset;
        
        // Reset prepared data arrays and variables.
        mDataGroups.clear();
        mMaterials.clear();
        mMeshSubsets.clear();
        mSubsetLookup.clear();
        mHardwareMesh = false;
        mOptimizedMesh = false;

    } // End if roll back an earlier prepare
    else if ( (mPrepareStatus != cgMeshStatus::Preparing && bRollBackPrepare) || !bRollBackPrepare )
    {
        // Clear out anything which is currently loaded in the mesh.
        dispose( false );
    
    } // End if not rolling back or no need to roll back

    // We are in the process of preparing the mesh
    mPrepareStatus = cgMeshStatus::Preparing;
    mVertexFormat = cgVertexFormat::formatFromDeclarator( pVertexFormat->getDeclarator() );
}

//-----------------------------------------------------------------------------
//  Name : prepareMesh ()
/// <summary>
/// Prepare the mesh immediately with the specified data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::prepareMesh( cgVertexFormat * pVertexFormat, void * pVertices, cgUInt32 nVertexCount, const TriangleArray & Faces, bool bHardwareCopy /* = true */, bool bWeld /* = true */, bool bOptimize /* = true */, cgResourceManager * pManager /* = CG_NULL */ )
{
    // Clear out anything which is currently loaded in the mesh.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // We are in the process of preparing the mesh
    mPrepareStatus = cgMeshStatus::Preparing;
    mVertexFormat = cgVertexFormat::formatFromDeclarator( pVertexFormat->getDeclarator() );

    // Populate preparation structures.
    mPrepareData.triangleCount = (cgUInt32)Faces.size();
    mPrepareData.triangleData   = Faces;
    mPrepareData.vertexCount   = nVertexCount;

    // Copy vertex data.
    mPrepareData.vertexData.resize( nVertexCount * pVertexFormat->getStride() );
    mPrepareData.vertexFlags.resize( nVertexCount );
    memset( &mPrepareData.vertexFlags[0], 0, nVertexCount );
    memcpy( &mPrepareData.vertexData[0], pVertices, nVertexCount * pVertexFormat->getStride() );

    // Generate the bounding box data for the new geometry.
    cgInt32 nPositionOffset = pVertexFormat->getElementOffset(D3DDECLUSAGE_POSITION);
    cgInt32 nStride         = (cgInt32)pVertexFormat->getStride();
    if ( nPositionOffset >= 0 )
    {
        cgByte * pSrc = ((cgByte*)pVertices) + nPositionOffset;
        for ( cgUInt32 i = 0; i < nVertexCount; ++i, pSrc += nStride )
            mBoundingBox.addPoint( *((cgVector3*)pSrc) );
    
    } // End if has position

    // Finish up
    return endPrepare( bHardwareCopy, bWeld, bOptimize );
}

//-----------------------------------------------------------------------------
//  Name : setVertexSource ()
/// <summary>
/// While preparing the mesh, this function should be called in order to
/// specify the source of the vertex buffer to pull data from.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::setVertexSource( void * pSource, cgUInt32 nVertexCount, cgVertexFormat * pSourceFormat )
{
    // We can only do this if we are in the process of preparing the mesh
    if ( mPrepareStatus != cgMeshStatus::Preparing )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Attempting to set a mesh vertex source without first calling 'prepareMesh' is not allowed.\n") );
        return false;
    
    } // End if not preparing

    // Clear any existing source information.
    if ( mPrepareData.ownsSource == true )
        delete []mPrepareData.vertexSource;
    mPrepareData.vertexSource = CG_NULL;
    mPrepareData.sourceFormat = CG_NULL;
    mPrepareData.ownsSource = false;
    mPrepareData.vertexRecords.clear();
    
    // If specifying CG_NULL (i.e. to clear) then we're done.
    if ( pSource == CG_NULL )
        return true;

    // Validate requirements
    if ( nVertexCount == 0 )
        return false;

    // If source format matches the format we're using to prepare
    // then just store the pointer for this vertex source. Otherwise
    // we need to allocate a temporary buffer and convert the data.
    mPrepareData.sourceFormat = pSourceFormat;
    if ( *pSourceFormat == *mVertexFormat )
    {
        mPrepareData.vertexSource = (cgByte*)pSource;
    
    } // End if matching
    else
    {
        mPrepareData.vertexSource = new cgByte[ nVertexCount * mVertexFormat->getStride() ];
        mPrepareData.ownsSource = true;
        cgVertexFormat::convertVertices( mPrepareData.vertexSource, mVertexFormat, 
                                         (cgByte*)pSource, pSourceFormat, nVertexCount );

    } // End if !matching
    
    // Allocate the vertex records for the new vertex buffer
    mPrepareData.vertexRecords.clear();
    mPrepareData.vertexRecords.resize( nVertexCount );

    // Fill with 0xFFFFFFFF initially to indicate that no vertex
    // originally in this location has yet been inserted into the
    // final vertex list.
    memset( &mPrepareData.vertexRecords[0], 0xFF, nVertexCount * sizeof(cgUInt32) );
    
    // Some data needs computing? These variables are essentially 'toggles'
    // that are set largely so that we can early out if it was NEVER necessary 
    // to generate these components (i.e. not one single vertex needed it).
    if ( pSourceFormat->getElement( D3DDECLUSAGE_NORMAL ) == CG_NULL && mVertexFormat->getElement( D3DDECLUSAGE_NORMAL ) != CG_NULL )
        mPrepareData.computeNormals = true;
    if ( pSourceFormat->getElement( D3DDECLUSAGE_BINORMAL ) == CG_NULL && mVertexFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL )
        mPrepareData.computeBinormals = true;
    if ( pSourceFormat->getElement( D3DDECLUSAGE_TANGENT ) == CG_NULL && mVertexFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL )
        mPrepareData.computeTangents = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addPrimitives ()
/// <summary>
/// Called by an external source (such as the geometry loader) in order
/// to populate the internal buffers ready for building the mesh.
/// Note : This may be called multiple times until the mesh is ready to be built
/// via the 'endPrepare' method.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::addPrimitives( const TriangleArray & aTriangles )
{
    cgUInt32 nOrigIndex, nIndex;

    // We can only do this if we are in the process of preparing the mesh
    if ( mPrepareStatus != cgMeshStatus::Preparing )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Attempting to add primitives to a mesh without first calling 'prepareMesh' is not allowed.\n") );
        return false;
    
    } // End if not preparing

    // Determine the correct offset to any relevant elements in the vertex
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

    // During the construction process we test to see if any specified
    // vertex normal contains invalid data. If the original source vertex
    // data did not contain a normal, we can optimize and skip this step.
    bool bSourceHasNormal   = (mPrepareData.sourceFormat->getElement( D3DDECLUSAGE_NORMAL ) != CG_NULL );
    bool bSourceHasBinormal = (mPrepareData.sourceFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
    bool bSourceHasTangent  = (mPrepareData.sourceFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );
    
    // In addition, we also record which of the required components each
    // vertex actually contained based on the following information.
    cgByte nVertexFlags = 0;
    if ( bSourceHasNormal == true ) 
        nVertexFlags |= PreparationData::SourceContainsNormal;
    if ( bSourceHasBinormal == true ) 
        nVertexFlags |= PreparationData::SourceContainsBinormal;
    if ( bSourceHasTangent == true ) 
        nVertexFlags |= PreparationData::SourceContainsTangent;

    // Loop through the specified faces and process them.
    cgByte * pSrcVertices = mPrepareData.vertexSource;
    cgUInt32 nFaceCount = (cgUInt32)aTriangles.size();
    for ( cgUInt32 i = 0; i < nFaceCount; ++i )
    {
        const Triangle & SrcTri = aTriangles[i];

        // Retrieve vertex positions (if there are any) so that we can peform degenerate testing.
        if ( nPositionOffset >= 0 )
        {
            cgVector3 * pVertex1 = (cgVector3*)(pSrcVertices + (SrcTri.indices[0] * nVertexStride) + nPositionOffset);
            cgVector3 * pVertex2 = (cgVector3*)(pSrcVertices + (SrcTri.indices[1] * nVertexStride) + nPositionOffset);
            cgVector3 * pVertex3 = (cgVector3*)(pSrcVertices + (SrcTri.indices[2] * nVertexStride) + nPositionOffset);
        
            // Skip triangle if it is degenerate.
            if ( cgMathUtility::compareVectors( *pVertex1, *pVertex2, CGE_EPSILON_1UM ) == 0 || cgMathUtility::compareVectors( *pVertex1, *pVertex3, CGE_EPSILON_1UM ) == 0 ||
                 cgMathUtility::compareVectors( *pVertex2, *pVertex3, CGE_EPSILON_1UM ) == 0 )
                 continue;

        } // End if has position.

        // Prepare a triangle structure ready for population
        mPrepareData.triangleCount++;
        mPrepareData.triangleData.resize( mPrepareData.triangleCount );
        Triangle & TriangleData = mPrepareData.triangleData[ mPrepareData.triangleCount - 1 ];
        
        // Set triangle's subset information.
        TriangleData.dataGroupId = SrcTri.dataGroupId;

        // If no material has been supplied, construct a 'default' one if it hasn't
        // already been constructed and apply it to these primitives.
        if ( SrcTri.material.isValid() == false )
        {
            generateDefaultMaterial( );
            TriangleData.material = mDefaultMaterial;
        
        } // End if no material
        else
        {
            TriangleData.material = SrcTri.material;
        
        } // End if supplied

        // For each index in the face
        for ( cgUInt32 j = 0; j < 3; ++j )
        {
            // Extract the original index from the specified index buffer
            nOrigIndex = SrcTri.indices[ j ];

            // Retrieve the vertex record for the original vertex
            nIndex = mPrepareData.vertexRecords[ nOrigIndex ];
            
            // Have we inserted this vertex into the vertex buffer previously?
            if ( nIndex == 0xFFFFFFFF )
            {
                // Vertex does not yet exist in the vertex buffer we are preparing
                // so copy the vertex in and record the index mapping for this vertex.
                nIndex = mPrepareData.vertexCount++;
                mPrepareData.vertexRecords[ nOrigIndex ] = nIndex;

                // Resize the output vertex buffer ready to hold this new data.
                size_t nInitialSize = mPrepareData.vertexData.size();
                mPrepareData.vertexData.resize( nInitialSize + nVertexStride );

                // Copy the data in.
                cgByte * pSrc = pSrcVertices + (nOrigIndex * nVertexStride);
                cgByte * pDst = &mPrepareData.vertexData[nInitialSize];
                memcpy( pDst, pSrc, nVertexStride );

                // Also record other pertenant details about this vertex.
                mPrepareData.vertexFlags.push_back( nVertexFlags );
                
                // Clear any invalid normals (completely messes up HDR if ANY NaNs make it this far)
                if ( nNormalOffset >= 0 && bSourceHasNormal == true )
                {
                    cgVector3 * pNormal = (cgVector3*)(pDst + nNormalOffset);
                    if ( _isnan( pNormal->x ) || _isnan( pNormal->y ) || _isnan( pNormal->z ) )
                        *pNormal = cgVector3( 0, 0, 0 );

                } // End if have normal
            
                // Grow the size of the bounding box
                if ( nPositionOffset >= 0 )
                    mBoundingBox.addPoint( *((cgVector3*)(pDst + nPositionOffset)) );
                
            } // End if vertex not recorded in this buffer yet
            
            // Copy the index in
            TriangleData.indices[j] = nIndex;
        
        } // Next Index

    } // Next Face

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addPrimitives ()
/// <summary>
/// Called by an external source (such as the geometry loader) in order
/// to populate the internal buffers ready for building the mesh.
/// Note : This may be called multiple times until the mesh is ready to be built
/// via the 'endPrepare' method.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::addPrimitives( cgPrimitiveType::Base DataOrder, cgUInt32 pIndexBuffer[], cgUInt32 nIndexStart, cgUInt32 nIndexCount, cgMaterialHandle hMaterial, cgUInt32 nDataGroupId /* = 0 */ )
{
    cgUInt32 nOrigIndex, nIndex;
    cgUInt32 nFaceCount = 0, pSrcIndices[3];

    // We can only do this if we are in the process of preparing the mesh
    if ( mPrepareStatus != cgMeshStatus::Preparing )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Attempting to add primitives to a mesh without first calling 'prepareMesh' is not allowed.\n") );
        return false;
    
    } // End if not preparing

    // Validate requirements
    if ( pIndexBuffer == CG_NULL || nIndexCount == 0 )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("An invalid parameter was passed to cgMesh::addPrimitives().\n") );
        return false;
    
    } // End if invalid parameter

    // If no material has been supplied, construct a 'default' one if it hasn't
    // already been constructed and apply it to these primitives.
    if ( hMaterial.isValid() == false )
    {
        generateDefaultMaterial( );
        hMaterial = mDefaultMaterial;
    
    } // End if no material

    // Determine the correct offset to any relevant elements in the vertex
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

    // During the construction process we test to see if any specified
    // vertex normal contains invalid data. If the original source vertex
    // data did not contain a normal, we can optimize and skip this step.
    bool bSourceHasNormal   = (mPrepareData.sourceFormat->getElement( D3DDECLUSAGE_NORMAL ) != CG_NULL );
    bool bSourceHasBinormal = (mPrepareData.sourceFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
    bool bSourceHasTangent  = (mPrepareData.sourceFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );
    
    // In addition, we also record which of the required components each
    // vertex actually contained based on the following information.
    cgByte nVertexFlags = 0;
    if ( bSourceHasNormal == true ) 
        nVertexFlags |= PreparationData::SourceContainsNormal;
    if ( bSourceHasBinormal == true ) 
        nVertexFlags |= PreparationData::SourceContainsBinormal;
    if ( bSourceHasTangent == true ) 
        nVertexFlags |= PreparationData::SourceContainsTangent;

    // Compute important data order dependant values
    switch ( DataOrder )
    {
        case cgPrimitiveType::TriangleList:
            if ( nIndexCount % 3 != 0 )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("An invalid parameter was passed to cgMesh::addPrimitives(). Specifically, the 'nIndexCount' parameter contained a value (%i) not divisible by 3 when specifying a data order of 'TriangleList'.\n"), nIndexCount );
                return false;
            
            } // End if invalid data
            nFaceCount = nIndexCount / 3;
            break;

        case cgPrimitiveType::TriangleFan:
            if ( nIndexCount < 3 )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("An invalid parameter was passed to cgMesh::addPrimitives(). Specifically, the 'nIndexCount' parameter contained a value (%i) which could not be used to form at least one triangle when specifying a data order of 'TriangleFan'.\n"), nIndexCount );
                return false;
            
            } // End if invalid data
            nFaceCount = nIndexCount - 2;
            break;

        case cgPrimitiveType::TriangleStrip:
            if ( nIndexCount < 3 )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("An invalid parameter was passed to cgMesh::addPrimitives(). Specifically, the 'nIndexCount' parameter contained a value (%i) which could not be used to form at least one triangle when specifying a data order of 'TriangleStrip'.\n"), nIndexCount );
                return false;
            
            } // End if invalid data
            nFaceCount = nIndexCount - 2;
            break;

    } // End switch DataOrder

    // Loop through the specified faces and process them.
    cgByte * pSrcVertices = mPrepareData.vertexSource;
    for ( cgUInt32 i = 0; i < nFaceCount; ++i )
    {
        // Compute data order dependant indices
        switch ( DataOrder )
        {
            case cgPrimitiveType::TriangleList:
                nIndex         = nIndexStart + (i*3);
                pSrcIndices[0] = pIndexBuffer[ nIndex ];
                pSrcIndices[1] = pIndexBuffer[ nIndex + 1 ];
                pSrcIndices[2] = pIndexBuffer[ nIndex + 2 ];
                break;

            case cgPrimitiveType::TriangleFan:
                nIndex         = nIndexStart + i;
                pSrcIndices[0] = pIndexBuffer[ nIndexStart ];
                pSrcIndices[1] = pIndexBuffer[ nIndex + 1 ];
                pSrcIndices[2] = pIndexBuffer[ nIndex + 2 ];
                break;

            case cgPrimitiveType::TriangleStrip:
                nIndex = nIndexStart + i;
                if ( (i&1) == 0 )
                {
                    pSrcIndices[0] = pIndexBuffer[ nIndex ];
                    pSrcIndices[1] = pIndexBuffer[ nIndex + 1 ];
                    pSrcIndices[2] = pIndexBuffer[ nIndex + 2 ];
                
                } // End if clockwise
                else
                {
                    pSrcIndices[0] = pIndexBuffer[ nIndex ];
                    pSrcIndices[1] = pIndexBuffer[ nIndex + 2 ];
                    pSrcIndices[2] = pIndexBuffer[ nIndex + 1 ];

                } // End if counter clockwise
                break;

        } // End switch DataOrder

        // Retrieve vertex positions (if there are any) so that we can peform degenerate testing.
        if ( nPositionOffset >= 0 )
        {
            cgVector3 * pVertex1 = (cgVector3*)(pSrcVertices + (pSrcIndices[0] * nVertexStride) + nPositionOffset);
            cgVector3 * pVertex2 = (cgVector3*)(pSrcVertices + (pSrcIndices[1] * nVertexStride) + nPositionOffset);
            cgVector3 * pVertex3 = (cgVector3*)(pSrcVertices + (pSrcIndices[2] * nVertexStride) + nPositionOffset);
        
            // Skip triangle if it is degenerate.
            if ( cgMathUtility::compareVectors( *pVertex1, *pVertex2, CGE_EPSILON_1UM ) == 0 || cgMathUtility::compareVectors( *pVertex1, *pVertex3, CGE_EPSILON_1UM ) == 0 ||
                 cgMathUtility::compareVectors( *pVertex2, *pVertex3, CGE_EPSILON_1UM ) == 0 )
                 continue;

        } // End if has position.

        // Prepare a triangle structure ready for population
        mPrepareData.triangleCount++;
        mPrepareData.triangleData.resize( mPrepareData.triangleCount );
        Triangle & TriangleData = mPrepareData.triangleData[ mPrepareData.triangleCount - 1 ];
        
        // Set triangle's subset information.
        TriangleData.dataGroupId = nDataGroupId;
        TriangleData.material    = hMaterial;

        // For each index in the face
        for ( cgUInt32 j = 0; j < 3; ++j )
        {
            // Extract the original index from the specified index buffer
            nOrigIndex = pSrcIndices[ j ];

            // Retrieve the vertex record for the original vertex
            nIndex = mPrepareData.vertexRecords[ nOrigIndex ];
            
            // Have we inserted this vertex into the vertex buffer previously?
            if ( nIndex == 0xFFFFFFFF )
            {
                // Vertex does not yet exist in the vertex buffer we are preparing
                // so copy the vertex in and record the index mapping for this vertex.
                nIndex = mPrepareData.vertexCount++;
                mPrepareData.vertexRecords[ nOrigIndex ] = nIndex;

                // Resize the output vertex buffer ready to hold this new data.
                size_t nInitialSize = mPrepareData.vertexData.size();
                mPrepareData.vertexData.resize( nInitialSize + nVertexStride );

                // Copy the data in.
                cgByte * pSrc = pSrcVertices + (nOrigIndex * nVertexStride);
                cgByte * pDst = &mPrepareData.vertexData[nInitialSize];
                memcpy( pDst, pSrc, nVertexStride );

                // Also record other pertenant details about this vertex.
                mPrepareData.vertexFlags.push_back( nVertexFlags );
                
                // Clear any invalid normals (completely messes up HDR if ANY NaNs make it this far)
                if ( nNormalOffset >= 0 && bSourceHasNormal == true )
                {
                    cgVector3 * pNormal = (cgVector3*)(pDst + nNormalOffset);
                    if ( _isnan( pNormal->x ) || _isnan( pNormal->y ) || _isnan( pNormal->z ) )
                        *pNormal = cgVector3( 0, 0, 0 );

                } // End if have normal
            
                // Grow the size of the bounding box
                if ( nPositionOffset >= 0 )
                    mBoundingBox.addPoint( *((cgVector3*)(pDst + nPositionOffset)) );
                
            } // End if vertex not recorded in this buffer yet
            
            // Copy the index in
            TriangleData.indices[j] = nIndex;
        
        } // Next Index

    } // Next Face

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createCylinder ()
/// <summary>
/// Create cylinder geometry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::createCylinder( cgVertexFormat * pFormat, cgFloat fRadius, cgFloat fHeight, cgUInt32 nStacks, cgUInt32 nSlices, bool bInverted, cgMeshCreateOrigin::Base Origin, bool bHardwareCopy /* = true */, cgResourceManager * pManager /* = NULL */ )
{
    cgVector3 vCurrentPos, vNormal;
    cgVector2 vCurrentTex;

    // Clear out old data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // We are in the process of preparing.
    mPrepareStatus = cgMeshStatus::Preparing;
    mVertexFormat = cgVertexFormat::formatFromDeclarator( pFormat->getDeclarator() );
    
    // Determine the correct offset to any relevant elements in the vertex
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nTexCoordOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_TEXCOORD );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

    // Compute the number of faces and vertices that will be required for this box
    mPrepareData.triangleCount = (nSlices * nStacks) * 2;
    mPrepareData.vertexCount   = (nSlices + 1) * (nStacks + 1);

    // Add vertices and faces for caps
    cgUInt32 nCapsStart = mPrepareData.vertexCount;
    mPrepareData.vertexCount   += nSlices * 2;
    mPrepareData.triangleCount += (nSlices - 2) * 2;

    // Allocate enough space for the new vertex and triangle data
    mPrepareData.vertexData.resize( mPrepareData.vertexCount * nVertexStride );
    mPrepareData.vertexFlags.resize( mPrepareData.vertexCount );
    mPrepareData.triangleData.resize( mPrepareData.triangleCount );

    // For each stack
    cgByte * pCurrentVertex = &mPrepareData.vertexData[0];
    cgByte * pCurrentFlags  = &mPrepareData.vertexFlags[0];
    for ( cgUInt32 nStack = 0; nStack <= nStacks; ++nStack )
    {
        // Generate a ring of vertices which describe the top edges of that stack's geometry
        // The last vertex is a duplicate of the first to ensure we have correct texturing
        for ( cgUInt32 nSlice = 0; nSlice <= nSlices; ++nSlice )
        {
            // Precompute any reusable values
            cgFloat a = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;

            // Compuse vertex normal at this location around the cylinder.
            vNormal.x   = sinf(a);
            vNormal.y   = 0;
            vNormal.z   = cosf(a);
            
            // Position is simply a scaled version of the normal
            // with the correctly computed height.
            vCurrentPos.x = vNormal.x * fRadius;
            vCurrentPos.y = (cgFloat)nStack * (fHeight / (cgFloat)nStacks);
            vCurrentPos.z = vNormal.z * fRadius;

            // Compute the texture coordinate.
            vCurrentTex.x = (1.0f / (cgFloat)nSlices) * (cgFloat)nSlice;
            vCurrentTex.y = (1.0f / (cgFloat)nStacks) * (cgFloat)nStack;

            // Position in center or at base/tip?
            if ( Origin == cgMeshCreateOrigin::Center )
                vCurrentPos.y -= fHeight * 0.5f;
            else if ( Origin == cgMeshCreateOrigin::Top )
                vCurrentPos.y -= fHeight;

            // Should we invert the vertex normal
            if ( bInverted )
                vNormal = -vNormal;
            
            // Store!
            if ( nPositionOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vCurrentPos;
            if ( nNormalOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vNormal;
            if ( nTexCoordOffset >= 0 )
                *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vCurrentTex;

            // Set flags for this vertex (we want to generate tangents 
            // and binormals if we need them).
            *pCurrentFlags++ = PreparationData::SourceContainsNormal;

            // Grow the object space bounding box for this mesh
            // by including the computed position.
            mBoundingBox.addPoint( vCurrentPos );
            
            // Move on to next vertex
            pCurrentVertex += nVertexStride;

        } // Next Slice

    } // Next Stack

    // Now cmpute the vertices for the base cylinder cap geometry.
    for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
    {
        // Precompute any reusable values
        cgFloat a = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;
        
        // Compute the vertex components.
        vCurrentPos    = cgVector3( sinf(a), 0, cosf(a) );
        vNormal        = (fHeight >= 0 ) ? cgVector3( 0, -1, 0 ) : cgVector3( 0, 1, 0 );
        vCurrentTex.x  = (vCurrentPos.x * 0.5f) + 0.5f;
        vCurrentTex.y  = (vCurrentPos.z * 0.5f) + 0.5f;
        vCurrentPos.x *= fRadius;
        vCurrentPos.z *= fRadius;

        // Position in center or at base/tip?
        if ( Origin == cgMeshCreateOrigin::Center )
            vCurrentPos.y -= fHeight * 0.5f;
        else if ( Origin == cgMeshCreateOrigin::Top )
            vCurrentPos.y -= fHeight;

        // Should we invert the vertex normal
        if ( bInverted )
            vNormal = -vNormal;

        // Store!
        if ( nPositionOffset >= 0 )
            *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vCurrentPos;
        if ( nNormalOffset >= 0 )
            *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vNormal;
        if ( nTexCoordOffset >= 0 )
            *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vCurrentTex;

        // Set flags for this vertex (we want to generate tangents 
        // and binormals if we need them).
        *pCurrentFlags++ = PreparationData::SourceContainsNormal;

        // Grow the object space bounding box for this mesh
        // by including the computed position.
        mBoundingBox.addPoint( vCurrentPos );
        
        // Move on to next vertex
        pCurrentVertex += nVertexStride;

    } // Next Slice

    // And the vertices for the end cylinder cap geometry.
    for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
    {
        // Precompute any reusable values
        cgFloat a = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;

        // Compute the vertex components.
        vCurrentPos    = cgVector3( sinf(a), fHeight, cosf(a) );
        vNormal        = (fHeight >= 0 ) ? cgVector3( 0, 1, 0 ) : cgVector3( 0, -1, 0 );
        vCurrentTex.x  = (vCurrentPos.x * -0.5f) + 0.5f;
        vCurrentTex.y  = (vCurrentPos.z * -0.5f) + 0.5f;
        vCurrentPos.x *= fRadius;
        vCurrentPos.z *= fRadius;

        // Position in center or at base/tip?
        if ( Origin == cgMeshCreateOrigin::Center )
            vCurrentPos.y -= fHeight * 0.5f;
        else if ( Origin == cgMeshCreateOrigin::Top )
            vCurrentPos.y -= fHeight;

        // Should we invert the vertex normal
        if ( bInverted )
            vNormal = -vNormal;

        // Store!
        if ( nPositionOffset >= 0 )
            *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vCurrentPos;
        if ( nNormalOffset >= 0 )
            *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vNormal;
        if ( nTexCoordOffset >= 0 )
            *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vCurrentTex;

        // Set flags for this vertex (we want to generate tangents 
        // and binormals if we need them).
        *pCurrentFlags++ = PreparationData::SourceContainsNormal;

        // Grow the object space bounding box for this mesh
        // by including the computed position.
        mBoundingBox.addPoint( vCurrentPos );
        
        // Move on to next vertex
        pCurrentVertex += nVertexStride;

    } // Next Slice

    // Generate the 'default' material if necessary.
    generateDefaultMaterial( );

    // Now compute the indices. For each stack (except the top and bottom)
    Triangle * pCurrentTriangle = &mPrepareData.triangleData[0];
    for ( cgUInt32 nStack = 0; nStack < nStacks; ++nStack )
    {
        // Generate two triangles for the quad on each slice
        for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
        {
            // If height was negative (i.e. faces are inverted)
            // we need to flip the order of the indices
            if ( ((!bInverted) && fHeight < 0) || (bInverted && fHeight > 0) )
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle++;

            } // End if inverted
            else
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice;
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[2] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle++;
            
            } // End if not inverted

        } // Next Slice

    } // Next Stack

    // Add cylinder cap geometry
    for ( cgUInt32 nSlice = 0; nSlice < nSlices - 2; ++nSlice )
    {
        // If height was negative (i.e. faces are inverted)
        // we need to flip the order of the indices
        if ( ((!bInverted) && fHeight < 0) || (bInverted && fHeight > 0) )
        {
            // Base Cap
            pCurrentTriangle->dataGroupId = 0;
            pCurrentTriangle->material  = mDefaultMaterial;
            pCurrentTriangle->indices[0] = nCapsStart;
            pCurrentTriangle->indices[1] = nCapsStart + nSlice + 1;
            pCurrentTriangle->indices[2] = nCapsStart + nSlice + 2;
            pCurrentTriangle++;
            
            // End Cap
            pCurrentTriangle->dataGroupId = 0;
            pCurrentTriangle->material  = mDefaultMaterial;
            pCurrentTriangle->indices[0] = nCapsStart + nSlices + nSlice + 2;
            pCurrentTriangle->indices[1] = nCapsStart + nSlices + nSlice + 1;
            pCurrentTriangle->indices[2] = nCapsStart + nSlices;
            pCurrentTriangle++;

        } // End if inverted
        else
        {
            // Base Cap
            pCurrentTriangle->dataGroupId = 0;
            pCurrentTriangle->material  = mDefaultMaterial;
            pCurrentTriangle->indices[0] = nCapsStart + nSlice + 2;
            pCurrentTriangle->indices[1] = nCapsStart + nSlice + 1;
            pCurrentTriangle->indices[2] = nCapsStart;
            pCurrentTriangle++;
            
            // End Cap
            pCurrentTriangle->dataGroupId = 0;
            pCurrentTriangle->material  = mDefaultMaterial;
            pCurrentTriangle->indices[0] = nCapsStart + nSlices;
            pCurrentTriangle->indices[1] = nCapsStart + nSlices + nSlice + 1;
            pCurrentTriangle->indices[2] = nCapsStart + nSlices + nSlice + 2;
            pCurrentTriangle++;
            
        } // End if not inverted

    } // Next Slice
    
    // We need to generate binormals / tangents?
    mPrepareData.computeBinormals = ( mVertexFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
    mPrepareData.computeTangents  = ( mVertexFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );

    // Finish up
    return endPrepare( bHardwareCopy, false, false );
}

//-----------------------------------------------------------------------------
//  Name : createCapsule ()
/// <summary>
/// Create capsule geometry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::createCapsule( cgVertexFormat * pFormat, cgFloat fRadius, cgFloat fHeight, cgUInt32 nStacks, cgUInt32 nSlices, bool bInverted, cgMeshCreateOrigin::Base Origin, bool bHardwareCopy /* = true */, cgResourceManager * pManager /* = NULL */ )
{
    cgVector3 vCurrentPos, vNormal;
    cgVector2 vCurrentTex;

    // Clear out old data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // We are in the process of preparing.
    mPrepareStatus = cgMeshStatus::Preparing;
    mVertexFormat = cgVertexFormat::formatFromDeclarator( pFormat->getDeclarator() );
    
    // Determine the correct offset to any relevant elements in the vertex
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nTexCoordOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_TEXCOORD );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

    // Compute the number of 'stacks' required for the hemisphere caps.
    // This must be the closest multiple of 2 to ensure a valid center division.
    cgUInt32 nSphereStacks = ((nSlices / 2) + 1) & 0xFFFFFFFE;
    if ( nSphereStacks < 2 ) nSphereStacks = 2;

    // Height must be at least equal to radius * 2 (to account for the hemispheres)
    bool bNegateY = (fHeight < 0);
    fRadius = fabsf( fRadius );
    fHeight = fabsf( fHeight );
    fHeight = max( fRadius * 2.0f, fHeight );
    cgFloat fCylinderHeight = fHeight - (fRadius * 2.0f);
    if ( bNegateY )
    {
        fHeight = -fHeight;
        fCylinderHeight = -fCylinderHeight;
    
    } // End if negated.

    // Add vertices for the first hemisphere. The cap shares a common row of vertices with the capsule sides.
    mPrepareData.triangleCount = (nSlices * (nSphereStacks/2)) * 2;
    mPrepareData.vertexCount   = (nSlices + 1) * ((nSphereStacks/2) + 1);

    // Cylinder geometry starts at the last row of the first hemisphere.
    cgUInt32 nCylinderStart = mPrepareData.vertexCount - (nSlices + 1);

    // Compute the number of faces and vertices that will be required by the
    // cylinder shape that exists between the two hemispheres.
    cgUInt32 nCylinderVerts = (nSlices + 1) * (nStacks - 1);
    mPrepareData.triangleCount += (nSlices * nStacks) * 2;
    mPrepareData.vertexCount   += nCylinderVerts;

    // Add vertices for the bottom hemisphere. The cap shares a common row of vertices with the capsule sides.
    mPrepareData.triangleCount += (nSlices * (nSphereStacks/2)) * 2;
    mPrepareData.vertexCount   += (nSlices + 1) * ((nSphereStacks/2) + 1);

    // Allocate enough space for the new vertex and triangle data
    mPrepareData.vertexData.resize( mPrepareData.vertexCount * nVertexStride );
    mPrepareData.vertexFlags.resize( mPrepareData.vertexCount );
    mPrepareData.triangleData.resize( mPrepareData.triangleCount );

    // First add the top hemisphere
    cgByte * pCurrentVertex = &mPrepareData.vertexData[0];
    cgByte * pCurrentFlags  = &mPrepareData.vertexFlags[0];
    cgInt32 nVerticesAdded = 0;
    for ( cgUInt32 nStack = 0; nStack <= nSphereStacks / 2; ++nStack )
    {
        // Generate a ring of vertices which describe the top edges of that stack's geometry
        // The last vertex is a duplicate of the first to ensure we have correct texturing
        for ( cgUInt32 nSlice = 0; nSlice <= nSlices; ++nSlice )
        {
            // Precompute any reusable values
            cgFloat  a = (CGE_PI / (cgFloat)nSphereStacks) * (cgFloat)nStack;
            cgFloat  b = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;
            cgFloat xz = sinf(a);

            // Compute the normal & position of the vertex
            vNormal.x = xz * sinf(b);
            vNormal.y = cosf((bNegateY) ? CGE_PI - a : a);
            vNormal.z = xz * cosf(b);
            vCurrentPos = vNormal * fRadius;

            // Offset so that it sits at the top of the central cylinder
            vCurrentPos.y += fCylinderHeight * 0.5f;

            // Compute the texture coordinate.
            vCurrentTex.x = (1.0f / (cgFloat)nSlices) * (cgFloat)nSlice;
            vCurrentTex.y = (vCurrentPos.y + (fHeight * 0.5f)) / fHeight;

            // Invert normal if required
            if ( bInverted )
                vNormal = -vNormal;

            // Position in center or at base/tip?
            if ( Origin == cgMeshCreateOrigin::Bottom )
                vCurrentPos.y += fHeight * 0.5f;
            else if ( Origin == cgMeshCreateOrigin::Top )
                vCurrentPos.y -= fHeight * 0.5f;

            // Store vertex components
            if ( nPositionOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vCurrentPos;
            if ( nNormalOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vNormal;
            if ( nTexCoordOffset >= 0 )
                *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vCurrentTex;

            // Set flags for this vertex (we want to generate tangents 
            // and binormals if we need them).
            *pCurrentFlags++ = PreparationData::SourceContainsNormal;

            // Grow the object space bounding box for this mesh
            // by including the computed position.
            mBoundingBox.addPoint( vCurrentPos );

            // Move on to next vertex
            pCurrentVertex += nVertexStride;
            nVerticesAdded++;

        } // Next Slice

    } // Next Stack

    // Create cylinder side vertices. We don't generate
    // vertices for the top/bottom row -- these were added by the 
    // hemispheres.
    for ( cgUInt32 nStack = 1; nStack < nStacks; ++nStack )
    {
        // Generate a ring of vertices which describe the top edges of that stack's geometry
        // The last vertex is a duplicate of the first to ensure we have correct texturing
        for ( cgUInt32 nSlice = 0; nSlice <= nSlices; ++nSlice )
        {
            // Precompute any reusable values
            cgFloat a = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;

            // Compuse vertex normal at this location around the cylinder.
            vNormal.x   = sinf(a);
            vNormal.y   = 0;
            vNormal.z   = cosf(a);
            
            // Position is simply a scaled version of the normal
            // with the correctly computed height.
            vCurrentPos.x  = vNormal.x * fRadius;
            vCurrentPos.y  = (fCylinderHeight - ((cgFloat)nStack * (fHeight / (cgFloat)nStacks)));
            vCurrentPos.y -= (fCylinderHeight * 0.5f);
            vCurrentPos.z  = vNormal.z * fRadius;

            // Compute the texture coordinate.
            vCurrentTex.x = (1.0f / (cgFloat)nSlices) * (cgFloat)nSlice;
            vCurrentTex.y = (vCurrentPos.y + (fHeight * 0.5f)) / fHeight;

            // Position in center or at base/tip?
            if ( Origin == cgMeshCreateOrigin::Bottom )
                vCurrentPos.y += fHeight * 0.5f;
            else if ( Origin == cgMeshCreateOrigin::Top )
                vCurrentPos.y -= fHeight * 0.5f;

            // Should we invert the vertex normal
            if ( bInverted )
                vNormal = -vNormal;
            
            // Store!
            if ( nPositionOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vCurrentPos;
            if ( nNormalOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vNormal;
            if ( nTexCoordOffset >= 0 )
                *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vCurrentTex;

            // Set flags for this vertex (we want to generate tangents 
            // and binormals if we need them).
            *pCurrentFlags++ = PreparationData::SourceContainsNormal;

            // Grow the object space bounding box for this mesh
            // by including the computed position.
            mBoundingBox.addPoint( vCurrentPos );
            
            // Move on to next vertex
            pCurrentVertex += nVertexStride;
            nVerticesAdded++;

        } // Next Slice

    } // Next Stack

    // Now the bottom hemisphere
    for ( cgUInt32 nStack = nSphereStacks / 2; nStack <= nSphereStacks; ++nStack )
    {
        // Generate a ring of vertices which describe the top edges of that stack's geometry
        // The last vertex is a duplicate of the first to ensure we have correct texturing
        for ( cgUInt32 nSlice = 0; nSlice <= nSlices; ++nSlice )
        {
            // Precompute any reusable values
            cgFloat  a = (CGE_PI / (cgFloat)nSphereStacks) * (cgFloat)nStack;
            cgFloat  b = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;
            cgFloat xz = sinf(a);

            // Compute the normal & position of the vertex
            vNormal.x = xz * sinf(b);
            vNormal.y = cosf((bNegateY) ? CGE_PI - a : a);
            vNormal.z = xz * cosf(b);
            vCurrentPos = vNormal * fRadius;

            // Offset so that it sits at the bottom of the central cylinder
            vCurrentPos.y -= fCylinderHeight * 0.5f;

            // Compute the texture coordinate.
            vCurrentTex.x = (1.0f / (cgFloat)nSlices) * (cgFloat)nSlice;
            vCurrentTex.y = (vCurrentPos.y + (fHeight * 0.5f)) / fHeight;

            // Invert normal if required
            if ( bInverted )
                vNormal = -vNormal;

            // Position in center or at base/tip?
            if ( Origin == cgMeshCreateOrigin::Bottom )
                vCurrentPos.y += fHeight * 0.5f;
            else if ( Origin == cgMeshCreateOrigin::Top )
                vCurrentPos.y -= fHeight * 0.5f;

            // Store vertex components
            if ( nPositionOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vCurrentPos;
            if ( nNormalOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vNormal;
            if ( nTexCoordOffset >= 0 )
                *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vCurrentTex;

            // Set flags for this vertex (we want to generate tangents 
            // and binormals if we need them).
            *pCurrentFlags++ = PreparationData::SourceContainsNormal;

            // Grow the object space bounding box for this mesh
            // by including the computed position.
            mBoundingBox.addPoint( vCurrentPos );

            // Move on to next vertex
            pCurrentVertex += nVertexStride;
            nVerticesAdded++;

        } // Next Slice

    } // Next Stack

    // Generate the 'default' material if necessary.
    generateDefaultMaterial( );

    // Now generate indices for the top hemisphere first.
    cgInt32 nTrianglesAdded = 0;
    Triangle * pCurrentTriangle = &mPrepareData.triangleData[0];
    for ( cgUInt32 nStack = 0; nStack < nSphereStacks / 2; ++nStack )
    {
        // Generate two triangles for the quad on each slice
        for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
        {
            // If height was negative (i.e. faces are inverted)
            // we need to flip the order of the indices
            if ( ((!bInverted) && fHeight < 0) || (bInverted && fHeight > 0) )
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice;
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[2] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle++;

                nTrianglesAdded += 2;

            } // End if inverted
            else
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle++;

                nTrianglesAdded += 2;
            
            } // End if !inverted
            
        } // Next Slice

    } // Next Stack

    // Cylinder stacks.
    for ( cgUInt32 nStack = 0; nStack < nStacks; ++nStack )
    {
        // Generate two triangles for the quad on each slice
        for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
        {
            // If height was negative (i.e. faces are inverted)
            // we need to flip the order of the indices
            if ( ((!bInverted) && fHeight < 0) || (bInverted && fHeight > 0) )
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCylinderStart + ((nStack*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle->indices[1] = nCylinderStart + (((nStack+1)*(nSlices+1)) + nSlice);
                pCurrentTriangle->indices[2] = nCylinderStart + ((nStack*(nSlices+1)) + nSlice);
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCylinderStart + ((nStack*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle->indices[1] = nCylinderStart + (((nStack+1)*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle->indices[2] = nCylinderStart + (((nStack+1)*(nSlices+1)) + nSlice);
                pCurrentTriangle++;

                nTrianglesAdded += 2;

            } // End if inverted
            else
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCylinderStart + ((nStack*(nSlices+1)) + nSlice);
                pCurrentTriangle->indices[1] = nCylinderStart + (((nStack+1)*(nSlices+1)) + nSlice);
                pCurrentTriangle->indices[2] = nCylinderStart + ((nStack*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCylinderStart + (((nStack+1)*(nSlices+1)) + nSlice);
                pCurrentTriangle->indices[1] = nCylinderStart + (((nStack+1)*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle->indices[2] = nCylinderStart + ((nStack*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle++;

                nTrianglesAdded += 2;
            
            } // End if not inverted

        } // Next Slice

    } // Next Stack

    // Finally, the indices for the bottom hemisphere.
    for ( cgUInt32 nStack = nSphereStacks / 2; nStack < nSphereStacks; ++nStack )
    {
        // Generate two triangles for the quad on each slice
        for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
        {
            // If height was negative (i.e. faces are inverted)
            // we need to flip the order of the indices
            if ( ((!bInverted) && fHeight < 0) || (bInverted && fHeight > 0) )
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCylinderVerts + (((nStack+1)*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle->indices[1] = nCylinderVerts + (((nStack+2)*(nSlices+1)) + nSlice);
                pCurrentTriangle->indices[2] = nCylinderVerts + (((nStack+1)*(nSlices+1)) + nSlice);
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCylinderVerts + (((nStack+1)*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle->indices[1] = nCylinderVerts + (((nStack+2)*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle->indices[2] = nCylinderVerts + (((nStack+2)*(nSlices+1)) + nSlice);
                pCurrentTriangle++;

                nTrianglesAdded += 2;
            
            } // End if inverted
            else
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCylinderVerts + (((nStack+1)*(nSlices+1)) + nSlice);
                pCurrentTriangle->indices[1] = nCylinderVerts + (((nStack+2)*(nSlices+1)) + nSlice);
                pCurrentTriangle->indices[2] = nCylinderVerts + (((nStack+1)*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCylinderVerts + (((nStack+2)*(nSlices+1)) + nSlice);
                pCurrentTriangle->indices[1] = nCylinderVerts + (((nStack+2)*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle->indices[2] = nCylinderVerts + (((nStack+1)*(nSlices+1)) + nSlice + 1);
                pCurrentTriangle++;

                nTrianglesAdded += 2;

            } // End if !inverted
            
        } // Next Slice

    } // Next Stack

    // We need to generate binormals / tangents?
    mPrepareData.computeBinormals = ( mVertexFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
    mPrepareData.computeTangents  = ( mVertexFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );

    // Finish up
    return endPrepare( bHardwareCopy, false, false );
}

//-----------------------------------------------------------------------------
//  Name : createSphere ()
/// <summary>
/// Create sphere geometry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::createSphere( cgVertexFormat * pFormat, cgFloat fRadius, cgUInt32 nStacks, cgUInt32 nSlices, bool bInverted, cgMeshCreateOrigin::Base Origin, bool bHardwareCopy /* = true */, cgResourceManager * pManager /* = NULL */ )
{
    cgVector3 vecPosition, vecNormal;

    // Clear out old data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // Inverting?
    if ( bInverted )
        fRadius = -fRadius;

    // We are in the process of preparing.
    mPrepareStatus = cgMeshStatus::Preparing;
    mVertexFormat = cgVertexFormat::formatFromDeclarator( pFormat->getDeclarator() );

    // Determine the correct offset to any relevant elements in the vertex
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nTexCoordOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_TEXCOORD );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt )mVertexFormat->getStride();
    
    // Compute the number of faces and vertices that will be required for this sphere
    mPrepareData.triangleCount = (nSlices * nStacks) * 2;
    mPrepareData.vertexCount   = (nSlices + 1) * (nStacks + 1);

    // Allocate enough space for the new vertex and triangle data
    mPrepareData.vertexData.resize( mPrepareData.vertexCount * nVertexStride );
    mPrepareData.vertexFlags.resize( mPrepareData.vertexCount );
    mPrepareData.triangleData.resize( mPrepareData.triangleCount );
        
    // For each stack
    cgByte * pCurrentVertex = &mPrepareData.vertexData[0];
    cgByte * pCurrentFlags  = &mPrepareData.vertexFlags[0];
    for ( cgUInt32 nStack = 0; nStack <= nStacks; ++nStack )
    {
        // Generate a ring of vertices which describe the top edges of that stack's geometry
        // The last vertex is a duplicate of the first to ensure we have correct texturing
        for ( cgUInt32 nSlice = 0; nSlice <= nSlices; ++nSlice )
        {
            // Precompute any reusable values
            cgFloat  a = (CGE_PI / (cgFloat)nStacks) * (cgFloat)nStack;
            cgFloat  b = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;
            cgFloat xz = sinf(a);

            // Compute the normal & position of the vertex
            vecNormal.x = xz * sinf(b);
            vecNormal.y = cosf(a);
            vecNormal.z = xz * cosf(b);
            vecPosition = vecNormal * fRadius;

            // Position in center or at base/tip?
            if ( Origin == cgMeshCreateOrigin::Bottom )
                vecPosition.y += fabsf(fRadius);
            else if ( Origin == cgMeshCreateOrigin::Top )
                vecPosition.y -= fabsf(fRadius);

            // Store vertex components
            if ( nPositionOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vecPosition;
            if ( nNormalOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vecNormal;
            if ( nTexCoordOffset >= 0 )
                *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = cgVector2((1 / (cgFloat)nSlices) * (cgFloat)nSlice, (1 / (cgFloat)nStacks) * (cgFloat)nStack);

            // Set flags for this vertex (we want to generate tangents 
            // and binormals if we need them).
            *pCurrentFlags++ = PreparationData::SourceContainsNormal;

            // Grow the object space bounding box for this mesh
            // by including the computed position.
            mBoundingBox.addPoint( vecPosition );

            // Move on to next vertex
            pCurrentVertex += nVertexStride;

        } // Next Slice

    } // Next Stack

    // Generate the 'default' material if necessary.
    generateDefaultMaterial( );

    // Now generate indices. Process each stack (except the top and bottom)
    Triangle * pCurrentTriangle = &mPrepareData.triangleData[0];
    for ( cgUInt32 nStack = 0; nStack < nStacks; ++nStack )
    {
        // Generate two triangles for the quad on each slice
        for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
        {
            pCurrentTriangle->dataGroupId = 0;
            pCurrentTriangle->material  = mDefaultMaterial;
            pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice;
            pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice;
            pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice + 1;
            pCurrentTriangle++;
            
            pCurrentTriangle->dataGroupId = 0;
            pCurrentTriangle->material  = mDefaultMaterial;
            pCurrentTriangle->indices[0] = ((nStack+1)*(nSlices+1)) + nSlice;
            pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice + 1;
            pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice + 1;
            pCurrentTriangle++;
            
        } // Next Slice

    } // Next Stack

    // We need to generate binormals / tangents?
    mPrepareData.computeBinormals = ( mVertexFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
    mPrepareData.computeTangents  = ( mVertexFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );

    // Finish up
    return endPrepare( bHardwareCopy, false, false );
}

//-----------------------------------------------------------------------------
//  Name : createTorus ()
/// <summary>
/// Create torus geometry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::createTorus( cgVertexFormat * pFormat, cgFloat fOuterRadius, cgFloat fInnerRadius, cgUInt32 nBands, cgUInt32 nSides, bool bInverted, cgMeshCreateOrigin::Base Origin, bool bHardwareCopy /* = true */, cgResourceManager * pManager /* = NULL */ )
{
    cgVector3 vPosition, vNormal, vCenter;
    cgVector2 vTexCoord;

    // Clear out old data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // We are in the process of preparing.
    mPrepareStatus = cgMeshStatus::Preparing;
    mVertexFormat = cgVertexFormat::formatFromDeclarator( pFormat->getDeclarator() );
    
    // Determine the correct offset to any relevant elements in the vertex
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nTexCoordOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_TEXCOORD );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

    // Compute the number of faces and vertices that will be required for this torus
    mPrepareData.triangleCount = (nBands * nSides) * 2;
    mPrepareData.vertexCount   = (nBands + 1) * (nSides + 1);

    // Allocate enough space for the new vertex and triangle data
    mPrepareData.vertexData.resize( mPrepareData.vertexCount * nVertexStride );
    mPrepareData.vertexFlags.resize( mPrepareData.vertexCount );
    mPrepareData.triangleData.resize( mPrepareData.triangleCount );
    
    // The radius of a circle running through the core of the torus interior
    cgFloat fCoreRadius = (fInnerRadius + fOuterRadius) / 2.0f;
    cgFloat fBandRadius = (fOuterRadius - fInnerRadius ) / 2.0f;

    // Generate vertex data. For each band (around the outside)
    cgByte * pCurrentVertex = &mPrepareData.vertexData[0];
    cgByte * pCurrentFlags  = &mPrepareData.vertexFlags[0];
    for ( cgUInt32 nBand = 0; nBand <= nBands; ++nBand )
    {
        // Precompute any re-usable values
        cgFloat a       = (CGE_TWO_PI / (cgFloat)nBands) * nBand;
        cgFloat sinBand = sinf( a );
        cgFloat cosBand = cosf( a );

        // Compute the center point for this band (imagine drawing a circle through the torus interior.)
        vCenter = cgVector3( sinBand * fCoreRadius, 0, cosBand * fCoreRadius );

        // Position in center or at base/tip?
        if ( Origin == cgMeshCreateOrigin::Bottom )
            vCenter.y += fabsf(fBandRadius);
        else if ( Origin == cgMeshCreateOrigin::Top )
            vCenter.y -= fabsf(fBandRadius);

        // Generate a ring of vertices that wrap around this core point.
        // The last vertex is a duplicate of the first to ensure we have correct texturing
        for ( cgUInt32 nSide = 0; nSide <= nSides; ++nSide )
        {
            // Precompute any re-usable values
            cgFloat b = (CGE_TWO_PI / (cgFloat)nSides) * nSide;
            cgFloat c = sinf(b) * fBandRadius;

            // Compute the vertex components
            vPosition.x   = vCenter.x + (c * sinBand);
            vPosition.y   = vCenter.y + (cosf(b) * fBandRadius);
            vPosition.z   = vCenter.z + (c * cosBand);
            cgVector3::normalize( vNormal, vPosition - vCenter );
            vTexCoord = cgVector2((1 / (cgFloat)nBands) * (cgFloat)nBand, (1 / (cgFloat)nSides) * (cgFloat)nSide);

            // Inverting?
            if ( bInverted )
                vNormal = -vNormal;

            // Store!
            if ( nPositionOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vPosition;
            if ( nNormalOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vNormal;
            if ( nTexCoordOffset >= 0 )
                *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vTexCoord;

            // Set flags for this vertex (we want to generate tangents 
            // and binormals if we need them).
            *pCurrentFlags++ = PreparationData::SourceContainsNormal;

            // Grow the object space bounding box for this mesh
            // by including the computed position.
            mBoundingBox.addPoint( vPosition );
            
            // Move on to next vertex
            pCurrentVertex += nVertexStride;

        } // Next Slice

    } // Next Stack

    // Generate the 'default' material if necessary.
    generateDefaultMaterial( );

    // Now generate indices. For each band.
    Triangle * pCurrentTriangle = &mPrepareData.triangleData[0];
    for ( cgUInt32 nBand = 0; nBand < nBands; ++nBand )
    {
        // Generate two triangles for the quad on each side
        for ( cgUInt32 nSide = 0; nSide < nSides; ++nSide )
        {
            if ( !bInverted )
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nBand*(nSides+1)) + nSide + 1;
                pCurrentTriangle->indices[1] = ((nBand+1)*(nSides+1)) + nSide;
                pCurrentTriangle->indices[2] = (nBand*(nSides+1)) + nSide;
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nBand*(nSides+1)) + nSide + 1;
                pCurrentTriangle->indices[1] = ((nBand+1)*(nSides+1)) + nSide + 1;
                pCurrentTriangle->indices[2] = ((nBand+1)*(nSides+1)) + nSide;
                pCurrentTriangle++;

            } // End if !inverted
            else
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nBand*(nSides+1)) + nSide;
                pCurrentTriangle->indices[1] = ((nBand+1)*(nSides+1)) + nSide;
                pCurrentTriangle->indices[2] = (nBand*(nSides+1)) + nSide + 1;
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = ((nBand+1)*(nSides+1)) + nSide;
                pCurrentTriangle->indices[1] = ((nBand+1)*(nSides+1)) + nSide + 1;
                pCurrentTriangle->indices[2] = (nBand*(nSides+1)) + nSide + 1;
                pCurrentTriangle++;
            
            } // End if inverted

        } // Next Side

    } // Next Band

    // We need to generate binormals / tangents?
    mPrepareData.computeBinormals = ( mVertexFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
    mPrepareData.computeTangents  = ( mVertexFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );

    // Finish up
    return endPrepare( bHardwareCopy, false, false );
}

//-----------------------------------------------------------------------------
//  Name : createBox ()
/// <summary>
/// Create box geometry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::createBox( cgVertexFormat * pFormat, cgFloat fWidth, cgFloat fHeight, cgFloat fDepth, cgUInt32 nWidthSegs, cgUInt32 nHeightSegs, cgUInt32 nDepthSegs, bool bInverted, cgMeshCreateOrigin::Base Origin, bool bHardwareCopy /* = true */, cgResourceManager * pManager /* = NULL */ )
{
    return createBox( pFormat, fWidth, fHeight, fDepth, nWidthSegs, nHeightSegs, nDepthSegs, 1.0f, 1.0f, bInverted, Origin, bHardwareCopy, pManager );
}

//-----------------------------------------------------------------------------
//  Name : createBox ()
/// <summary>
/// Create box geometry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::createBox( cgVertexFormat * pFormat, cgFloat fWidth, cgFloat fHeight, cgFloat fDepth, cgUInt32 nWidthSegs, cgUInt32 nHeightSegs, cgUInt32 nDepthSegs, cgFloat fTexUScale, cgFloat fTexVScale, bool bInverted, cgMeshCreateOrigin::Base Origin, bool bHardwareCopy /* = true */, cgResourceManager * pManager /* = NULL */ )
{
    cgUInt32  nXCount, nYCount, nCounter;
    cgVector3 vCurrentPos, vDeltaPosX, vDeltaPosY, vNormal;
    cgVector2 vCurrentTex, vDeltaTex;

    // Clear out old data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // We are in the process of preparing.
    mPrepareStatus = cgMeshStatus::Preparing;
    mVertexFormat = cgVertexFormat::formatFromDeclarator( pFormat->getDeclarator() );

    // Determine the correct offset to any relevant elements in the vertex
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nTexCoordOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_TEXCOORD );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt )mVertexFormat->getStride();

    // Compute the number of faces and vertices that will be required for this box
    mPrepareData.triangleCount = (4 * (nWidthSegs * nDepthSegs)) + (4 * (nWidthSegs * nHeightSegs)) + (4 * (nHeightSegs * nDepthSegs));
    mPrepareData.vertexCount   = (2*((nWidthSegs+1) * (nDepthSegs+1))) + (2*((nWidthSegs+1) * (nHeightSegs+1))) + (2*((nHeightSegs+1) * (nDepthSegs+1)));

    // Allocate enough space for the new vertex and triangle data
    mPrepareData.vertexData.resize( mPrepareData.vertexCount * nVertexStride );
    mPrepareData.vertexFlags.resize( mPrepareData.vertexCount );
    mPrepareData.triangleData.resize( mPrepareData.triangleCount );
    
    // Ensure width and depth are absolute (prevent inverting on those axes)
    fWidth  = fabsf(fWidth);
    fDepth  = fabsf(fDepth);

    // Generate faces
    cgByte * pCurrentVertex = &mPrepareData.vertexData[0];
    cgByte * pCurrentFlags  = &mPrepareData.vertexFlags[0];
    cgFloat fHalfWidth = fWidth / 2, fHalfDepth = fDepth / 2;
    for ( cgUInt32 i = 0; i < 6; ++i )
    {
        switch ( i )
        {
            case 0: // +X
                nXCount     = nDepthSegs + 1;
                nYCount     = nHeightSegs + 1;
                vDeltaPosX  = cgVector3( 0, 0, fDepth / (cgFloat)nDepthSegs );
                vDeltaPosY  = cgVector3( 0, -fHeight / (cgFloat)nHeightSegs, 0 );
                vCurrentPos = cgVector3( fHalfWidth, fHeight, -fHalfDepth );
                vNormal     = cgVector3( 1, 0, 0 );
                break;

            case 1: // +Y
                nXCount     = nWidthSegs + 1;
                nYCount     = nDepthSegs + 1;
                vDeltaPosX  = cgVector3( fWidth / (cgFloat)nWidthSegs, 0, 0 );
                vDeltaPosY  = cgVector3( 0, 0, -fDepth / (cgFloat)nDepthSegs );
                vCurrentPos = cgVector3( -fHalfWidth, fHeight, fHalfDepth );
                vNormal     = (fHeight > 0.0f) ? cgVector3( 0, 1, 0 ) : cgVector3( 0, -1, 0 );
                break;

            case 2: // +Z
                nXCount     = nWidthSegs + 1;
                nYCount     = nHeightSegs + 1;
                vDeltaPosX  = cgVector3( -fWidth / (cgFloat)nWidthSegs, 0, 0 );
                vDeltaPosY  = cgVector3( 0, -fHeight / (cgFloat)nHeightSegs, 0 );
                vCurrentPos = cgVector3( fHalfWidth, fHeight, fHalfDepth );
                vNormal     = cgVector3( 0, 0, 1 );
                break;

            case 3: // -X
                nXCount     = nDepthSegs + 1;
                nYCount     = nHeightSegs + 1;
                vDeltaPosX  = cgVector3( 0, 0, -fDepth / (cgFloat)nDepthSegs );
                vDeltaPosY  = cgVector3( 0, -fHeight / (cgFloat)nHeightSegs, 0 );
                vCurrentPos = cgVector3( -fHalfWidth, fHeight, fHalfDepth );
                vNormal     = cgVector3( -1, 0, 0 );
                break;

            case 4: // -Y
                nXCount     = nWidthSegs + 1;
                nYCount     = nDepthSegs + 1;
                vDeltaPosX  = cgVector3( fWidth / (cgFloat)nWidthSegs, 0, 0 );
                vDeltaPosY  = cgVector3( 0, 0, fDepth / (cgFloat)nDepthSegs );
                vCurrentPos = cgVector3( -fHalfWidth, 0, -fHalfDepth );
                vNormal     = (fHeight > 0.0f) ? cgVector3( 0, -1, 0 ) : cgVector3( 0, 1, 0 );
                break;

            case 5: // -Z
                nXCount     = nWidthSegs + 1;
                nYCount     = nHeightSegs + 1;
                vDeltaPosX  = cgVector3( fWidth / (cgFloat)nWidthSegs, 0, 0 );
                vDeltaPosY  = cgVector3( 0, -fHeight / (cgFloat)nHeightSegs, 0 );
                vCurrentPos = cgVector3( -fHalfWidth, fHeight, -fHalfDepth );
                vNormal     = cgVector3( 0, 0, -1 );
                break;

        } // End Face Switch

        // Should we invert the vertex normal
        if ( bInverted == true )
            vNormal = -vNormal;

        // Add faces
        vCurrentTex = cgVector2( 0, 0 );
        vDeltaTex   = cgVector2( 1.0f / (float)(nXCount - 1), 1.0f / (float)(nYCount - 1) );
        for ( cgUInt32 y = 0; y < nYCount; ++y )
        {
            for ( cgUInt32 x = 0; x < nXCount; ++x )
            {
                cgVector3 vOutputPos = vCurrentPos;
                if ( Origin == cgMeshCreateOrigin::Center )
                    vOutputPos.y -= fHeight * 0.5f;
                else if ( Origin == cgMeshCreateOrigin::Top )
                    vOutputPos.y -= fHeight;

                // Store vertex components
                if ( nPositionOffset >= 0 )
                    *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vOutputPos;
                if ( nNormalOffset >= 0 )
                    *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vNormal;
                if ( nTexCoordOffset >= 0 )
                    *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = cgVector2(vCurrentTex.x * fTexUScale, vCurrentTex.y * fTexVScale );

                // Set flags for this vertex (we want to generate tangents 
                // and binormals if we need them).
                *pCurrentFlags++ = PreparationData::SourceContainsNormal;

                // Grow the object space bounding box for this mesh
                // by including the computed position.
                mBoundingBox.addPoint( vOutputPos );

                // Move to next vertex position
                vCurrentPos    += vDeltaPosX;
                vCurrentTex.x  += vDeltaTex.x;
                pCurrentVertex += nVertexStride;

            } // Next Column

            // Move to next row
            vCurrentPos   += vDeltaPosY;
            vCurrentPos   -= vDeltaPosX * (float)nXCount;
            vCurrentTex.x  = 0.0f;
            vCurrentTex.y += vDeltaTex.y;
        
        } // Next Row

    } // Next Face

    // Generate the 'default' material if necessary.
    generateDefaultMaterial( );

    // Now generate indices. For each box face.
    nCounter = 0;
    Triangle * pCurrentTriangle = &mPrepareData.triangleData[0];
    for ( cgUInt32 i = 0; i < 6; ++i )
    {
        switch ( i )
        {
            case 0: // +X
            case 3: // -X
                nXCount = nDepthSegs + 1;
                nYCount = nHeightSegs + 1;
                break;

            case 1: // +Y
            case 4: // -Y
                nXCount = nWidthSegs + 1;
                nYCount = nDepthSegs + 1;
                break;

            case 2: // +Z
            case 5: // -Z
                nXCount = nWidthSegs + 1;
                nYCount = nHeightSegs + 1;
                break;

        } // End Face Switch

        for ( cgUInt32 y = 0; y < nYCount - 1; ++y )
        {
            for ( cgUInt32 x = 0; x < nXCount - 1; ++x )
            {
                // If height was negative (i.e. faces are inverted)
                // we need to flip the order of the indices
                if ( (bInverted == false && fHeight < 0) || (bInverted == true && fHeight > 0) )
                {
                    pCurrentTriangle->dataGroupId = 0;
                    pCurrentTriangle->material  = mDefaultMaterial;
                    pCurrentTriangle->indices[0] = x + 1 + ((y+1) * nXCount) + nCounter;
                    pCurrentTriangle->indices[1] = x + 1 + (y * nXCount) + nCounter;
                    pCurrentTriangle->indices[2] = x + (y * nXCount) + nCounter;
                    pCurrentTriangle++;
                    
                    pCurrentTriangle->dataGroupId = 0;
                    pCurrentTriangle->material  = mDefaultMaterial;
                    pCurrentTriangle->indices[0] = x + ((y+1) * nXCount) + nCounter;
                    pCurrentTriangle->indices[1] = x + 1 + ((y+1) * nXCount) + nCounter;
                    pCurrentTriangle->indices[2] = x + (y * nXCount) + nCounter;
                    pCurrentTriangle++;

                } // End if inverted
                else
                {
                    pCurrentTriangle->dataGroupId = 0;
                    pCurrentTriangle->material  = mDefaultMaterial;
                    pCurrentTriangle->indices[0] = x + (y * nXCount) + nCounter;
                    pCurrentTriangle->indices[1] = x + 1 + (y * nXCount) + nCounter;
                    pCurrentTriangle->indices[2] = x + 1 + ((y+1) * nXCount) + nCounter;
                    pCurrentTriangle++;

                    pCurrentTriangle->dataGroupId = 0;
                    pCurrentTriangle->material  = mDefaultMaterial;
                    pCurrentTriangle->indices[0] = x + (y * nXCount) + nCounter;
                    pCurrentTriangle->indices[1] = x + 1 + ((y+1) * nXCount) + nCounter;
                    pCurrentTriangle->indices[2] = x + ((y+1) * nXCount) + nCounter;
                    pCurrentTriangle++;
                
                } // End if normal
            
            } // Next Column
        
        } // Next Row

        // Compute vertex start for next face.
        nCounter += nXCount * nYCount;

    } // Next Face
        
    // We need to generate binormals / tangents?
    mPrepareData.computeBinormals = ( mVertexFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
    mPrepareData.computeTangents  = ( mVertexFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );

    // Finish up
    return endPrepare( bHardwareCopy, false, false );
}

//-----------------------------------------------------------------------------
//  Name : createCone ()
/// <summary>
/// Create cone geometry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::createCone( cgVertexFormat * pFormat, cgFloat fRadius, cgFloat fRadiusTip, cgFloat fHeight, cgUInt32 nStacks, cgUInt32 nSlices, bool bInverted, cgMeshCreateOrigin::Base Origin, bool bHardwareCopy /* = true */, cgResourceManager * pManager /* = NULL */ )
{
    cgVector3 vecPosition, vecNormal;
    cgVector2 vecTexCoords;

    // Clear out old data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // We are in the process of preparing.
    mPrepareStatus = cgMeshStatus::Preparing;
    mVertexFormat = cgVertexFormat::formatFromDeclarator( pFormat->getDeclarator() );

    // Determine the correct offset to any relevant elements in the vertex
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nTexCoordOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_TEXCOORD );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();
    
    // Compute the number of faces and vertices that will be required for this cone
    mPrepareData.triangleCount = (nSlices * nStacks) * 2;
    mPrepareData.vertexCount   = (nSlices + 1) * (nStacks + 1);

    // Add vertices and faces for caps
    if ( fRadiusTip < CGE_EPSILON_1MM ) fRadiusTip = 0.0f;
    cgUInt32 nNumCaps = ( fRadiusTip > 0.0f ) ? 2 : 1;
    cgUInt32 nCapsStart = mPrepareData.vertexCount;
    mPrepareData.vertexCount   += nSlices * nNumCaps;
    mPrepareData.triangleCount += (nSlices - 2) * nNumCaps;
    
    // Allocate enough space for the new vertex and triangle data
    mPrepareData.vertexData.resize( mPrepareData.vertexCount * nVertexStride );
    mPrepareData.vertexFlags.resize( mPrepareData.vertexCount );
    mPrepareData.triangleData.resize( mPrepareData.triangleCount );

    // Generate vertex data. For each stack
    cgByte * pCurrentVertex = &mPrepareData.vertexData[0];
    cgByte * pCurrentFlags  = &mPrepareData.vertexFlags[0];
    for ( cgUInt32 nStack = 0; nStack <= nStacks; ++nStack )
    {
        // Generate a ring of vertices which describe the top edges of that stack's geometry
        // The last vertex is a duplicate of the first to ensure we have correct texturing
        for ( cgUInt32 nSlice = 0; nSlice <= nSlices; ++nSlice )
        {
            // Precompute any reusable values
            cgFloat a = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;
            cgFloat b = fRadius + ((fRadiusTip - fRadius) * ((cgFloat)nStack / (cgFloat)nStacks));
            
            // Compute the vertex components
            vecPosition.x   = sinf(a);
            vecPosition.y   = (cgFloat)nStack * (fHeight / (cgFloat)nStacks);
            vecPosition.z   = cosf(a);
            vecNormal       = cgVector3( vecPosition.x, 0.0f, vecPosition.z );
            vecPosition.x  *= b;
            vecPosition.z  *= b;

            // Position in center or at base/tip?
            if ( Origin == cgMeshCreateOrigin::Center )
                vecPosition.y -= fHeight * 0.5f;
            else if ( Origin == cgMeshCreateOrigin::Top )
                vecPosition.y -= fHeight;

            // Inverting the normal?
            if ( bInverted )
                vecNormal = -vecNormal;
            
            // Store vertex components
            if ( nPositionOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vecPosition;
            if ( nNormalOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vecNormal;
            if ( nTexCoordOffset >= 0 )
                *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = cgVector2((1 / (cgFloat)nSlices) * (cgFloat)nSlice, (1 / (cgFloat)nStacks) * (cgFloat)nStack);

            // Set flags for this vertex (we want to generate tangents 
            // and binormals if we need them).
            *pCurrentFlags++ = PreparationData::SourceContainsNormal;

            // Grow the object space bounding box for this mesh
            // by including the computed position.
            mBoundingBox.addPoint( vecPosition );
            
            // Move on to next vertex
            pCurrentVertex += nVertexStride;

        } // Next Slice

    } // Next Stack

    // Now cmpute the vertices for the base cylinder cap geometry.
    for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
    {
        // Precompute any reusable values
        cgFloat a = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;
        
        // Compute the vertex components
        vecPosition     = cgVector3( sinf(a), 0, cosf(a) );
        vecNormal       = (fHeight >= 0 ) ? cgVector3( 0, -1, 0 ) : cgVector3( 0, 1, 0 );
        vecTexCoords    = cgVector2( (vecPosition.x * 0.5f) + 0.5f, (vecPosition.z * 0.5f) + 0.5f );
        vecPosition.x  *= fRadius;
        vecPosition.z  *= fRadius;

        // Position in center or at base/tip?
        if ( Origin == cgMeshCreateOrigin::Center )
            vecPosition.y -= fHeight * 0.5f;
        else if ( Origin == cgMeshCreateOrigin::Top )
            vecPosition.y -= fHeight;

        // Inverting the normal?
        if ( bInverted )
            vecNormal = -vecNormal;
        
        // Store vertex components
        if ( nPositionOffset >= 0 )
            *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vecPosition;
        if ( nNormalOffset >= 0 )
            *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vecNormal;
        if ( nTexCoordOffset >= 0 )
            *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vecTexCoords;

        // Set flags for this vertex (we want to generate tangents 
        // and binormals if we need them).
        *pCurrentFlags++ = PreparationData::SourceContainsNormal;

        // Grow the object space bounding box for this mesh
        // by including the computed position.
        mBoundingBox.addPoint( vecPosition );
        
        // Move on to next vertex
        pCurrentVertex += nVertexStride;

    } // Next Slice

    // And the vertices for the end cylinder cap geometry.
    if ( nNumCaps > 1 )
    {
        for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
        {
            // Precompute any reusable values
            cgFloat a = (CGE_TWO_PI / (cgFloat)nSlices) * (cgFloat)nSlice;
            
            // Compute the vertex components
            vecPosition     = cgVector3( sinf(a), fHeight, cosf(a) );
            vecNormal       = (fHeight >= 0 ) ? cgVector3( 0, 1, 0 ) : cgVector3( 0, -1, 0 );
            vecTexCoords    = cgVector2( (vecPosition.x * -0.5f) + 0.5f, (vecPosition.z * -0.5f) + 0.5f );
            vecPosition.x  *= fRadiusTip;
            vecPosition.z  *= fRadiusTip;

            // Position in center or at base/tip?
            if ( Origin == cgMeshCreateOrigin::Center )
                vecPosition.y -= fHeight * 0.5f;
            else if ( Origin == cgMeshCreateOrigin::Top )
                vecPosition.y -= fHeight;

            // Inverting the normal?
            if ( bInverted )
                vecNormal = -vecNormal;

            // Store vertex components
            if ( nPositionOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nPositionOffset)) = vecPosition;
            if ( nNormalOffset >= 0 )
                *((cgVector3*)(pCurrentVertex + nNormalOffset))   = vecNormal;
            if ( nTexCoordOffset >= 0 )
                *((cgVector2*)(pCurrentVertex + nTexCoordOffset)) = vecTexCoords;
            
            // Set flags for this vertex (we want to generate tangents 
            // and binormals if we need them).
            *pCurrentFlags++ = PreparationData::SourceContainsNormal;

            // Grow the object space bounding box for this mesh
            // by including the computed position.
            mBoundingBox.addPoint( vecPosition );
            
            // Move on to next vertex
            pCurrentVertex += nVertexStride;

        } // Next Slice

    } // End if add end cap

    // Generate the 'default' material if necessary.
    generateDefaultMaterial( );

    // Now generate indices. Process each stack (except the top and bottom)
    Triangle * pCurrentTriangle = &mPrepareData.triangleData[0];
    for ( cgUInt32 nStack = 0; nStack < nStacks; ++nStack )
    {
        // Generate two triangles for the quad on each slice
        for ( cgUInt32 nSlice = 0; nSlice < nSlices; ++nSlice )
        {
            // If height was negative (i.e. faces are inverted)
            // we need to flip the order of the indices
            if ( ((!bInverted) && fHeight < 0) || (bInverted && fHeight > 0) )
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle++;
                
            } // End if inverted
            else
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle->indices[2] = (nStack*(nSlices+1)) + nSlice;
                pCurrentTriangle++;
                
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = (nStack*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[1] = ((nStack+1)*(nSlices+1)) + nSlice + 1;
                pCurrentTriangle->indices[2] = ((nStack+1)*(nSlices+1)) + nSlice;
                pCurrentTriangle++;
                
            } // End if not inverted

        } // Next Slice

    } // Next Stack

    // Add cylinder cap geometry
    for ( cgUInt32 nSlice = 0; nSlice < nSlices - 2; ++nSlice )
    {
        // If height was negative (i.e. faces are inverted)
        // we need to flip the order of the indices
        if ( ((!bInverted) && fHeight < 0) || (bInverted && fHeight > 0) )
        {
            // Base Cap
            pCurrentTriangle->dataGroupId = 0;
            pCurrentTriangle->material  = mDefaultMaterial;
            pCurrentTriangle->indices[0] = nCapsStart;
            pCurrentTriangle->indices[1] = nCapsStart + nSlice + 1;
            pCurrentTriangle->indices[2] = nCapsStart + nSlice + 2;
            pCurrentTriangle++;
            
            // End Cap
            if ( nNumCaps > 1 )
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCapsStart + nSlices + nSlice + 2;
                pCurrentTriangle->indices[1] = nCapsStart + nSlices + nSlice + 1;
                pCurrentTriangle->indices[2] = nCapsStart + nSlices;
                pCurrentTriangle++;
            
            } // End if add second cap

        } // End if inverted
        else
        {
            // Base Cap
            pCurrentTriangle->dataGroupId = 0;
            pCurrentTriangle->material  = mDefaultMaterial;
            pCurrentTriangle->indices[0] = nCapsStart + nSlice + 2;
            pCurrentTriangle->indices[1] = nCapsStart + nSlice + 1;
            pCurrentTriangle->indices[2] = nCapsStart;
            pCurrentTriangle++;
            
            // End Cap
            if ( nNumCaps > 1 )
            {
                pCurrentTriangle->dataGroupId = 0;
                pCurrentTriangle->material  = mDefaultMaterial;
                pCurrentTriangle->indices[0] = nCapsStart + nSlices;
                pCurrentTriangle->indices[1] = nCapsStart + nSlices + nSlice + 1;
                pCurrentTriangle->indices[2] = nCapsStart + nSlices + nSlice + 2;
                pCurrentTriangle++;
                
            } // End if add second cap
            
        } // End if not inverted

    } // Next Slice

    // We need to generate binormals / tangents?
    mPrepareData.computeBinormals = ( mVertexFormat->getElement( D3DDECLUSAGE_BINORMAL ) != CG_NULL );
    mPrepareData.computeTangents  = ( mVertexFormat->getElement( D3DDECLUSAGE_TANGENT ) != CG_NULL );

    // Finish up
    return endPrepare( bHardwareCopy, false, false );
}

//-----------------------------------------------------------------------------
//  Name : subsetSortPredicate () (Private, Static)
/// <summary>
/// Predicate function that allows us to sort mesh subset lists by
/// material and datagroup.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::subsetSortPredicate( const MeshSubset * lhs, const MeshSubset * rhs )
{
    if ( lhs->material < rhs->material )
        return true;
    else if ( (lhs->material == rhs->material) && (lhs->dataGroupId < rhs->dataGroupId) )
        return true;
    return false;
}

//-----------------------------------------------------------------------------
//  Name : endPrepare ()
/// <summary>
/// All data has been added to the mesh and we should now build the
/// renderable data for the mesh.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::endPrepare( bool bHardwareCopy /* = true */, bool bWeld /* = true */, bool bOptimize /* = true */ )
{
    // Were we previously preparing?
    if ( mPrepareStatus != cgMeshStatus::Preparing )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Attempting to call 'endPrepare' on a mesh without first calling 'prepareMesh' is not allowed.\n") );
        return false;
    
    } // End if previously preparing

    // Clear out old data that is no longer necessary in order to preserve memory
    setVertexSource( CG_NULL, 0, CG_NULL );

    // Process the vertex data in order to generate any additional components that may be necessary
    // (i.e. Normal, Binormal and Tangent)
    if ( generateVertexComponents( bWeld ) == false )
        return false;

    // Allocate the system memory vertex buffer ready for population.
    mVertexCount = (cgUInt32)mPrepareData.vertexCount;
    mSystemVB    = new cgByte[ mVertexCount * mVertexFormat->getStride() ];
    
    // Copy vertex data into the new buffer and dispose of the temporary data.
    memcpy( mSystemVB, &mPrepareData.vertexData[0], mVertexCount * mVertexFormat->getStride() );
    mPrepareData.vertexData.clear();
    mPrepareData.vertexFlags.clear();
    mPrepareData.vertexCount = 0;

    // Vertex data has been updated and potentially needs to be serialized.
    mDBVertexStreamDirty = true;

    // Allocate the memory for our system memory index buffer
    mFaceCount = mPrepareData.triangleCount;
    mSystemIB  = new cgUInt32[ mFaceCount * 3 ];
    
    // Transform triangle indices, material and data group information
    // to the final triangle data arrays. We keep the latter two handy so
    // that we know precisely which subset each triangle belongs to.
    mTriangleData.resize( mFaceCount );
    cgUInt32 * pDstIndices = mSystemIB;
    for ( cgUInt32 i = 0; i < mFaceCount; ++i )
    {
        // Copy indices.
        const Triangle & TriIn = mPrepareData.triangleData[i];
        *pDstIndices++ = TriIn.indices[0];
        *pDstIndices++ = TriIn.indices[1];
        *pDstIndices++ = TriIn.indices[2];

        // Copy triangle subset information.
        MeshSubsetKey & TriOut = mTriangleData[i];
        TriOut.material       = TriIn.material;
        TriOut.dataGroupId    = TriIn.dataGroupId;

    } // Next triangle
    mPrepareData.triangleCount = 0;
    mPrepareData.triangleData.clear();

    // Index data has been updated and potentially needs to be serialized.
    mDBIndicesDirty = true;

    // A video memory copy of the mesh was requested?
    if ( bHardwareCopy )
    {
        // Calculate the required size of the vertex buffer
        cgUInt32 nBufferSize = mVertexCount * mVertexFormat->getStride();
        
        // Create the vertex buffer for this format
        if ( !mManager->createVertexBuffer( &mHardwareVB, nBufferSize, cgBufferUsage::WriteOnly, 
                                               mVertexFormat, cgMemoryPool::Default, cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create vertex buffer of size %i bytes during 'endPrepare' call.\n"), nBufferSize );
            return false;

        } // End if failed

        // Populate the buffer.
        cgVertexBuffer * pBuffer = mHardwareVB.getResource(true);
        if ( !pBuffer || !pBuffer->updateBuffer( 0, 0, mSystemVB ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to populate vertex buffer during 'endPrepare' call.\n") );
            return false;

        } // End if failed

    } // End if video memory vertex buffer required

    // Skin binding data has potentially been updated and needs to be serialized.
    mDBSkinDataDirty      = true;
    mDBBonePalettesDirty  = true;

    // Finally perform the final sort of the mesh data in order
    // to build the index buffer and subset tables.
    if ( !sortMeshData( bOptimize, bHardwareCopy ) )
        return false;

    // The mesh is now prepared
    mPrepareStatus   = cgMeshStatus::Prepared;
    mResourceLoaded = true;
    mHardwareMesh   = bHardwareCopy;
    mOptimizedMesh  = bOptimize;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : sortMeshData() (Protected)
/// <summary>
/// Sort the data in the mesh into material & datagroup order.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::sortMeshData( bool bOptimize, bool bBuildHardwareBuffers )
{
    std::map<MeshSubsetKey,cgUInt32> SubsetSizes;
    std::map<MeshSubsetKey,cgUInt32>::iterator itSubsetSize;
    DataGroupSubsetMap::iterator itDataGroup;
    MaterialSubsetMap::iterator itMaterial;
    cgUInt32 i, j;

    // Clear out any old data.
    for ( i = 0; i < mMeshSubsets.size(); ++i )
        delete mMeshSubsets[i];
    mMeshSubsets.clear();
    mDataGroups.clear();
    mMaterials.clear();
    mSubsetLookup.clear();

    // Our first job is to collate all the various subsets and also
    // to determine how many triangles should exist in each.
    for ( i = 0; i < mFaceCount; ++i )
    {
        const MeshSubsetKey & SubsetKey = mTriangleData[i];

        // Already contains this material / data group combination?
        itSubsetSize = SubsetSizes.find( SubsetKey );
        if ( itSubsetSize == SubsetSizes.end() )
        {
            // Add a new entry for this subset
            SubsetSizes[ SubsetKey ] = 1;
        
        } // End if !exists
        else
        {
            // Update the existing subset
            itSubsetSize->second++;
        
        } // End if already encountered
    
    } // Next Triangle

    // We should now have a complete list of subsets and the number of triangles
    // which should exist in each. Populate final mesh subset table and update start / count 
    // values so that we can correctly generate the new sorted index buffer.
    cgUInt32 nCounter = 0;
    for ( itSubsetSize = SubsetSizes.begin(); itSubsetSize != SubsetSizes.end(); ++itSubsetSize )
    {
        // Construct a new subset and populate with initial construction 
        // values including the expected starting face location.
        const MeshSubsetKey & Key = itSubsetSize->first;
        MeshSubset * pSubset      = new MeshSubset();
        pSubset->material        = Key.material;
        pSubset->dataGroupId     = Key.dataGroupId;
        pSubset->faceStart       = nCounter;
        nCounter                 += itSubsetSize->second;

        // Ensure that "FaceCount" defaults to zero at this point
        // so that we can keep a running total during the final buffer 
        // construction.
        pSubset->faceCount     = 0;

        // Also reset vertex values as appropriate (will grow
        // using standard 'bounding' value insert).
        pSubset->vertexStart   = 0x7FFFFFFF;
        pSubset->vertexCount   = 0;

        // Add to list for fast linear access, and lookup table
        // for sorted search.
        mMeshSubsets.push_back( pSubset );
        mSubsetLookup[ Key ] = pSubset;

        // Add to data group lookup table
        itDataGroup = mDataGroups.find( pSubset->dataGroupId );
        if ( itDataGroup == mDataGroups.end() )
            mDataGroups[ pSubset->dataGroupId ].push_back( pSubset );
        else
            itDataGroup->second.push_back( pSubset );

        // Add to the material lookup table.
        mMaterials[ pSubset->material ].push_back( pSubset );

    } // Next Subset

    // Allocate space for new sorted index buffer and face re-map information
    cgUInt32 * pSrcIndices = mSystemIB;
    cgUInt32 * pDstIndices = new cgUInt32[ mFaceCount * 3 ];
    cgUInt32 * pFaceRemap  = new cgUInt32[ mFaceCount ];

    // Start building new indices
    cgInt32  nIndex;
    cgUInt32 nIndexStart = 0;
    for ( i = 0; i < mFaceCount; ++i )
    {
        // Find a matching subset for this triangle
        MeshSubset * pSubset = mSubsetLookup[ mTriangleData[i] ];
        
        // Copy index data over to new buffer, taking care to record the correct
        // vertex values as required. We'll temporarily use VertexStart and VertexCount
        // as a min/max record that we'll come round and correct later.
        nIndexStart = (pSubset->faceStart + pSubset->faceCount) * 3;
        
        // Index[0]
        nIndex = (cgInt32)(*pSrcIndices++);
        if ( nIndex < pSubset->vertexStart )
            pSubset->vertexStart = nIndex;
        if ( nIndex > pSubset->vertexCount )
            pSubset->vertexCount = nIndex;
        pDstIndices[nIndexStart++] = nIndex;
        
        // Index[1]
        nIndex = (cgInt32)(*pSrcIndices++);
        if ( nIndex < pSubset->vertexStart )
            pSubset->vertexStart = nIndex;
        if ( nIndex > pSubset->vertexCount )
            pSubset->vertexCount = nIndex;
        pDstIndices[nIndexStart++] = nIndex;

        // Index[2]
        nIndex = (cgInt32)(*pSrcIndices++);
        if ( nIndex < pSubset->vertexStart )
            pSubset->vertexStart = nIndex;
        if ( nIndex > pSubset->vertexCount )
            pSubset->vertexCount = nIndex;
        pDstIndices[nIndexStart++] = nIndex;

        // Store face re-map information so that we can remap data as required
        pFaceRemap[ i ] = pSubset->faceStart + pSubset->faceCount;

        // We have now recorded a triangle in this subset
        pSubset->faceCount++;

    } // Next Triangle

    // Sort the subset list in order to ensure that all subsets with the same 
    // materials and data groups are added next to one another in the final
    // index buffer. This ensures that we can batch draw all subsets that share 
    // common properties.
    std::sort( mMeshSubsets.begin(), mMeshSubsets.end(), subsetSortPredicate );

    // Perform the same sort on the data group and material mapped lists.
    // Also take the time to build the final list of materials used by this mesh
    // (render control batching system requires that we cache this information in a 
    // specific format).
    mObjectMaterials.clear();
    mObjectMaterials.reserve( mMaterials.size() );
    for ( itDataGroup = mDataGroups.begin(); itDataGroup != mDataGroups.end(); ++itDataGroup )
        std::sort( itDataGroup->second.begin(), itDataGroup->second.end(), subsetSortPredicate );
    for ( itMaterial = mMaterials.begin(); itMaterial != mMaterials.end(); ++itMaterial )
    {
        std::sort( itMaterial->second.begin(), itMaterial->second.end(), subsetSortPredicate );

        // ToDo: 9999 - This will likely be removed and replaced with the new object visibility
        // push system.        
        mObjectMaterials.push_back( itMaterial->first );

    } // Next Material

    // Optimize the faces as we transfer to the final destination index buffer
    // if requested. Otherwise, just copy them over directly.
    pSrcIndices = pDstIndices;
    pDstIndices = mSystemIB;
    for ( nCounter = 0, i = 0; i < (size_t)mMeshSubsets.size(); ++i )
    {
        MeshSubset * pSubset = mMeshSubsets[i];
        
        // Note: Remember that at this stage, the subset's 'vertexCount' member still describes
        // a 'max' vertex (not a count)... We're correcting this later.
        if ( bOptimize == true )
            buildOptimizedIndexBuffer( pSubset, pSrcIndices + (pSubset->faceStart * 3), pDstIndices, pSubset->vertexStart, pSubset->vertexCount );
        else
            memcpy( pDstIndices, pSrcIndices + (pSubset->faceStart * 3), pSubset->faceCount * 3 * sizeof(cgUInt32) );
        
        // This subset's starting face now refers to its location 
        // in the final destination buffer rather than the temporary one.
        pSubset->faceStart = nCounter;
        nCounter += pSubset->faceCount;

        // Move on to output next sorted subset.
        pDstIndices += pSubset->faceCount * 3;

    } // Next Subset

    // Clean up.
    delete []pSrcIndices;
    pSrcIndices = CG_NULL;

    // Rebuild the additional triangle data based on the newly sorted
    // subset data, and also convert the previously recorded maximum
    // vertex value (stored in "VertexCount") into its final form
    for ( i = 0; i < (cgUInt32)mMeshSubsets.size(); ++i )
    {
        // Convert vertex "Max" to "Count"
        MeshSubset * pSubset = mMeshSubsets[i];
        pSubset->vertexCount = (pSubset->vertexCount - pSubset->vertexStart) + 1;
        
        // Update additional triangle data array.
        for ( j = pSubset->faceStart; j < ((cgUInt32)pSubset->faceStart + (cgUInt32)pSubset->faceCount); ++j )
        {
            mTriangleData[j].material    = pSubset->material;
            mTriangleData[j].dataGroupId = pSubset->dataGroupId;

        } // Next Triangle

    } // Next Subset

    // We're done with the remap data.
    // ToDo: Note - we don't actually use the face remap information at
    // the moment, but it could be useful?
    delete []pFaceRemap;
    pFaceRemap = CG_NULL;

    // Hardware versions of the final buffer were required?
    if ( bBuildHardwareBuffers )
    {
        // Calculate the required size of the index buffer
        cgUInt32 nBufferSize = mFaceCount * 3 * sizeof(cgUInt32);

        // Allocate hardware buffer if required (i.e. it does not already exist).
        if ( !mHardwareIB.isValid() )
        {
            // Create the index buffer for this format
            if ( !mManager->createIndexBuffer( &mHardwareIB, nBufferSize, cgBufferUsage::WriteOnly, 
                                                 cgBufferFormat::Index32, cgMemoryPool::Default, cgDebugSource() ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to create index buffer of size %i bytes during 'sortMeshData' call.\n"), nBufferSize );
                return false;

            } // End if failed

        } // End if not allocated

        // Populate the hardware index buffer
        cgIndexBuffer * pBuffer = mHardwareIB.getResource(true);
        if ( !pBuffer || !pBuffer->updateBuffer( 0, 0, mSystemIB ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to populate index buffer during 'sortMeshData' call.\n") );
            return false;

        } // End if failed

    } // End if hardware buffer required

    // Index data and subsets have been updated and potentially need to be serialized.
    mDBIndicesDirty = true;
    mDBSubsetsDirty = true;

    // Update database data as necessary.
    if ( !serializeMesh( ) )
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : generateDefaultMaterial ()
/// <summary>
/// Generate the internal 'default' material that will be used when no valid
/// application defined material is available for a given primitive or batch
/// of primitives.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::generateDefaultMaterial( )
{
    // ToDo: 9999 - Can we just share with a 'common' default mesh material?
    if ( mDefaultMaterial.isValid() == false && mManager != CG_NULL )
    {
        // ToDo: Error Handling
        // Construct a default material.
        mManager->createMaterial( &mDefaultMaterial, cgMaterialType::Standard, cgResourceFlags::ForceNew, cgDebugSource() );
        cgMaterial * pDefaultMaterial = mDefaultMaterial.getResource(true);
        pDefaultMaterial->beginPopulate( mManager );
        pDefaultMaterial->setName( _T("Mesh::DefaultMaterial") );
        pDefaultMaterial->endPopulate( );
        
    } // End if need default
}


//-----------------------------------------------------------------------------
//  Name : generateVertexComponents () (Private)
/// <summary>
/// Some vertex components potentially need to be generated. This may
/// include vertex normals, binormals or tangents. This function will
/// generate any such components which were not provided.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::generateVertexComponents( bool bWeld )
{
    // Vertex normals were requested (and at least some were not yet provided?)
    if ( mForceNormalGen || mPrepareData.computeNormals )
    {
        // Generate the adjacency information for vertex normal computation
        cgUInt32Array adjacency;
        if ( !generateAdjacency( adjacency ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to generate adjacency buffer for mesh containing %i faces.\n"), mPrepareData.triangleCount );
            return false;
        
        } // End if failed to generate

        // Generate any vertex normals that have not been provided
        if ( !generateVertexNormals( &adjacency.front() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to generate vertex normals for mesh containing %i faces.\n"), mPrepareData.triangleCount );
            return false;

        } // End if failed to generate

    } // End if compute

    // Weld vertices at this point
    if ( bWeld )
    {
        if ( !weldVertices( ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to weld vertices for mesh containing %i faces.\n"), mPrepareData.triangleCount );
            return false;

        } // End if failed to weld
    
    } // End if optional weld

    // Binormals and / or tangents were requested (and at least some where not yet provided?)
    if ( mForceTangentGen || mPrepareData.computeBinormals || mPrepareData.computeTangents )
    {
        // Requires normals
        if ( mVertexFormat->getElement( D3DDECLUSAGE_NORMAL ) != CG_NULL )
        {
            // Generate any vertex tangents that have not been provided
            if ( !generateVertexTangents( ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to generate vertex tangents for mesh containing %i faces.\n"), mPrepareData.triangleCount );
                return false;

            } // End if failed to generate

        } // End if has normals

    } // End if compute

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : generateVertexNormals () (Private)
/// <summary>
/// Generates any vertex normals that may have been requested but not
/// provided when adding vertex data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::generateVertexNormals( cgUInt32 * pAdjacency, cgUInt32Array * pRemapArray /* = CG_NULL */ )
{
    cgUInt32        nStartTri, nPreviousTri, nCurrentTri;
    cgVector3       vecEdge1, vecEdge2, vecNormal;
    cgUInt32        i, j, k, nIndex;

    // Get access to useful data offset information.
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

    // Final format requests vertex normals?
    if ( nNormalOffset < 0 )
        return true;

    // Size the remap array accordingly and populate it with the default mapping.
    cgUInt32 nOriginalVertexCount = mPrepareData.vertexCount;
    if ( pRemapArray )
    {
        pRemapArray->resize( mPrepareData.vertexCount );
        for ( i = 0; i < mPrepareData.vertexCount; ++i )
            (*pRemapArray)[i] = i;
    
    } // End if supplied

    // Pre-compute surface normals for each triangle
    cgByte * pSrcVertices = &mPrepareData.vertexData[0];
    cgVector3 * pNormals = new cgVector3[ mPrepareData.triangleCount ];
    memset( pNormals, 0, mPrepareData.triangleCount * sizeof(cgVector3) );
    for ( i = 0; i < mPrepareData.triangleCount; ++i )
    {
        // Retrieve positions of each referenced vertex.
        const Triangle & Tri = mPrepareData.triangleData[i];
        const cgVector3 * v1 = (cgVector3*)(pSrcVertices + (Tri.indices[0] * nVertexStride) + nPositionOffset);
        const cgVector3 * v2 = (cgVector3*)(pSrcVertices + (Tri.indices[1] * nVertexStride) + nPositionOffset);
        const cgVector3 * v3 = (cgVector3*)(pSrcVertices + (Tri.indices[2] * nVertexStride) + nPositionOffset);

        // Compute the two edge vectors required for generating our normal
        // We normalize here to prevent problems when the triangles are very small.
        cgVector3::normalize( vecEdge1, *v2 - *v1 );
        cgVector3::normalize( vecEdge2, *v3 - *v1 );

        // Generate the normal
        cgVector3::cross( vecNormal, vecEdge1, vecEdge2 );
        cgVector3::normalize( pNormals[i], vecNormal );

    } // Next Face

    // Now compute the actual VERTEX normals using face adjacency information
    for ( i = 0; i < mPrepareData.triangleCount; ++i )
    {
        Triangle & Tri = mPrepareData.triangleData[i];

        // Process each vertex in the face
        for ( j = 0; j < 3; ++j )
        {
            // Retrieve the index for this vertex.
            nIndex = Tri.indices[ j ];

            // Skip this vertex if normal information was already provided.
            if ( !mForceNormalGen && (mPrepareData.vertexFlags[nIndex] & PreparationData::SourceContainsNormal) )
                continue;

            // To generate vertex normals using the adjacency information we first need to walk backwards
            // through the list to find the first triangle that references this vertex (using entrance/exit edge strategy).
            // Once we have the first triangle, step forwards and sum the normals of each of the faces
            // for each triangle we touch. This is essentially a flood fill through all of the triangles
            // that touch this vertex, without ever having to test the entire set for shared vertices.
            // The initial backwards traversal prevents us from having to store (and test) a 'visited' flag for
            // every triangle in the buffer.
            
            // First walk backwards...
            nStartTri    = i;
            nPreviousTri = i;
            nCurrentTri  = pAdjacency[ (i*3) + ((j + 2) % 3) ];
            for ( ; ; )
            {
                // Stop walking if we reach the starting triangle again, or if there
                // is no connectivity out of this edge
                if ( nCurrentTri == nStartTri || nCurrentTri == 0xFFFFFFFF )
                    break;

                // Find the edge in the adjacency list that we came in through
                for ( k = 0; k < 3; ++k )
                {
                    if ( pAdjacency[ (nCurrentTri*3) + k ] == nPreviousTri )
                        break;
                
                } // Next item in adjacency list

                // If we found the edge we entered through, the exit edge will
                // be the edge counter-clockwise from this one when walking backwards
                if ( k < 3 )
                {
                    nPreviousTri = nCurrentTri;
                    nCurrentTri  = pAdjacency[ (nCurrentTri*3) + ((k + 2) % 3) ];
                
                } // End if found entrance edge
                else
                {
                    break;
                
                } // End if failed to find entrance edge
            
            } // Next Test

            // We should now be at the starting triangle, we can start to walk forwards
            // collecting the face normals. First find the exit edge so we can start walking.
            for ( k = 0; k < 3; ++k )
            {
                if ( pAdjacency[ (nCurrentTri*3) + k ] == nPreviousTri )
                    break;
            
            } // Next item in adjacency list

            if ( k < 3 )
            {
                nStartTri    = nCurrentTri;
                nPreviousTri = nCurrentTri;
                nCurrentTri  = pAdjacency[ (nCurrentTri*3) + k ];
                vecNormal    = pNormals[ nStartTri ];
                for ( ; ; )
                {
                    // Stop walking if we reach the starting triangle again, or if there
                    // is no connectivity out of this edge
                    if ( nCurrentTri == nStartTri || nCurrentTri == 0xFFFFFFFF )
                        break;

                    // Add this normal.
                    vecNormal += pNormals[ nCurrentTri ];

                    // Find the edge in the adjacency list that we came in through
                    for ( k = 0; k < 3; ++k )
                    {
                        if ( pAdjacency[ (nCurrentTri*3) + k ] == nPreviousTri )
                            break;
                    
                    } // Next item in adjacency list

                    // If we found the edge we came entered through, the exit edge will
                    // be the edge clockwise from this one when walking forwards
                    if ( k < 3 )
                    {
                        nPreviousTri = nCurrentTri;
                        nCurrentTri  = pAdjacency[ (nCurrentTri*3) + ((k + 1) % 3) ];
                    
                    } // End if found entrance edge
                    else
                    {
                        break;
                    
                    } // End if failed to find entrance edge
                
                } // Next Test

            } // End if found entrance edge
            
            // Normalize the new vertex normal
            cgVector3::normalize( vecNormal, vecNormal );

            // If the normal we are about to store is significantly different from any normal
            // already stored in this vertex (excepting the case where it is <0,0,0>), we need
            // to split the vertex into two.
            cgVector3 * pRefNormal = (cgVector3*)(pSrcVertices + (nIndex * nVertexStride) + nNormalOffset);
            if ( pRefNormal->x == 0.0f && pRefNormal->y == 0.0f && pRefNormal->z == 0.0f )
            {
                *pRefNormal = vecNormal;
            
            } // End if no normal stored here yet
            else
            {
                // Split and store in a new vertex if it is different (enough)
                if ( fabsf( pRefNormal->x - vecNormal.x ) >= 1e-3f || 
                     fabsf( pRefNormal->y - vecNormal.y ) >= 1e-3f ||
                     fabsf( pRefNormal->z - vecNormal.z ) >= 1e-3f)
                {
                    // Make room for new vertex data.
                    mPrepareData.vertexData.resize( mPrepareData.vertexData.size() + nVertexStride );

                    // Ensure that we update the 'pSrcVertices' pointer (used throughout the
                    // loop). The internal buffer wrapped by the resized vertex data vector
                    // may have been re-allocated.
                    pSrcVertices = &mPrepareData.vertexData[0];
                    
                    // Duplicate the vertex at the end of the buffer
                    memcpy( pSrcVertices + (mPrepareData.vertexCount * nVertexStride),
                            pSrcVertices + (nIndex * nVertexStride),
                            nVertexStride );

                    // Duplicate any other remaining information.
                    mPrepareData.vertexFlags.push_back( mPrepareData.vertexFlags[nIndex] );
                    
                    // Record the split
                    if ( pRemapArray )
                        (*pRemapArray)[ nIndex ] = mPrepareData.vertexCount;

                    // Store the new normal and finally record the fact that we have
                    // added a new vertex.
                    nIndex = mPrepareData.vertexCount++;
                    pRefNormal = (cgVector3*)(pSrcVertices + (nIndex * nVertexStride) + nNormalOffset);
                    *pRefNormal = vecNormal;
                    
                    // Update the index
                    Tri.indices[ j ] = nIndex;

                } // End if normal is different

            } // End if normal already stored here

        } // Next Vertex

    } // Next Face

    // We're done with the surface normals
    delete []pNormals;

    // If no new vertices were introduced, then it is not necessary
    // for the caller to remap anything.
    if ( pRemapArray && nOriginalVertexCount == mPrepareData.vertexCount )
        pRemapArray->clear();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : generateVertexTangents () 
/// <summary>
/// Builds the tangent space vectors for this polygon. 
/// Credit to Terathon Software - http://www.terathon.com/code/tangent.html 
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::generateVertexTangents( ) 
{
    cgVector3     * pTangents = CG_NULL, * pBinormals = CG_NULL;
    cgUInt32        i, i1, i2, i3, nNumFaces, nNumVerts;
    cgVector3       P, Q, T, B, vCross, vNormal;
    cgFloat         s1, t1, s2, t2, r;
    cgPlane         Plane;

    // Get access to useful data offset information.
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nNormalOffset   = mVertexFormat->getElementOffset( D3DDECLUSAGE_NORMAL );
    cgInt nBinormalOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_BINORMAL );
    cgInt nTangentOffset  = mVertexFormat->getElementOffset( D3DDECLUSAGE_TANGENT );
    cgInt nTexCoordOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_TEXCOORD, 0 );
    cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

    // Final format requests tangents?
    bool bRequiresBinormal = (nBinormalOffset >= 0);
    bool bRequiresTangent  = (nTangentOffset >= 0);
    if ( !mForceTangentGen && !bRequiresBinormal && !bRequiresTangent )
        return true;

    // This will fail if we don't already have normals however.
    if ( nNormalOffset < 0 )
        return false;
    
    // Allocate storage space for the tangent and binormal vectors
    // that we will effectively need to average for shared vertices.
    nNumFaces  = mPrepareData.triangleCount;
    nNumVerts  = mPrepareData.vertexCount;
    pTangents  = new cgVector3[nNumVerts];
    pBinormals = new cgVector3[nNumVerts];
    memset( pTangents, 0, sizeof(cgVector3) * nNumVerts );
    memset( pBinormals, 0, sizeof(cgVector3) * nNumVerts );

    // Iterate through each triangle in the mesh
    cgByte * pSrcVertices = &mPrepareData.vertexData[0];
    for ( i = 0; i < nNumFaces; ++i ) 
    {
        Triangle & Tri = mPrepareData.triangleData[i];

        // Compute the three indices for the triangle
        i1 = Tri.indices[0];
        i2 = Tri.indices[1];
        i3 = Tri.indices[2];
        
        // Retrieve references to the positions of the three vertices in the triangle.
        const cgVector3 & E = *(cgVector3*)(pSrcVertices + (i1 * nVertexStride) + nPositionOffset);
        const cgVector3 & F = *(cgVector3*)(pSrcVertices + (i2 * nVertexStride) + nPositionOffset);
        const cgVector3 & G = *(cgVector3*)(pSrcVertices + (i3 * nVertexStride) + nPositionOffset);

        // Retrieve references to the base texture coordinates of the three vertices in the triangle.
        // ToDo: Allow customization of which tex coordinates to generate from.
        const cgVector2 & Et = *(cgVector2*)(pSrcVertices + (i1 * nVertexStride) + nTexCoordOffset);
        const cgVector2 & Ft = *(cgVector2*)(pSrcVertices + (i2 * nVertexStride) + nTexCoordOffset);
        const cgVector2 & Gt = *(cgVector2*)(pSrcVertices + (i3 * nVertexStride) + nTexCoordOffset);

        // Compute the known variables P & Q, where "P = F-E" and "Q = G-E"
        // based on our original discussion of the tangent vector
        // calculation.
        P = F - E;
        Q = G - E;

        // Also compute the know variables <s1,t1> and <s2,t2>. Recall that
        // these are the texture coordinate deltas similarly for "F-E"
        // and "G-E".
        s1 = Ft.x - Et.x;
        t1 = Ft.y - Et.y;
        s2 = Gt.x - Et.x;
        t2 = Gt.y - Et.y;

        // Next we can pre-compute part of the equation we developed
        // earlier: "1/(s1 * t2 - s2 * t1)". We do this in two separate
        // stages here in order to ensure that the texture coordinates
        // are not invalid.
        r = (s1 * t2 - s2 * t1);
        if ( fabs( r ) < CGE_EPSILON )
            continue;
        r = 1.0f / r;

        // All that's left for us to do now is to run the matrix
        // multiplication and multiply the result by the scalar portion
        // we precomputed earlier.
        T.x = r * (t2 * P.x - t1 * Q.x);
        T.y = r * (t2 * P.y - t1 * Q.y);
        T.z = r * (t2 * P.z - t1 * Q.z);
        B.x = r * (s1 * Q.x - s2 * P.x);
        B.y = r * (s1 * Q.y - s2 * P.y);
        B.z = r * (s1 * Q.z - s2 * P.z);

        // Add the tangent and binormal vectors (summed average) to
        // any previous values computed for each vertex.
        pTangents[i1] += T;
        pTangents[i2] += T;
        pTangents[i3] += T;
        pBinormals[i1] += B;
        pBinormals[i2] += B;
        pBinormals[i3] += B; 

    } // Next Triangle 

    // Generate final tangent vectors 
    for ( i = 0; i < nNumVerts; i++, pSrcVertices += nVertexStride ) 
    { 
        // Skip if the original imported data already provided a binormal / tangent.
        bool bHasBinormal = ((mPrepareData.vertexFlags[i] & PreparationData::SourceContainsBinormal) != 0);
        bool bHasTangent  = ((mPrepareData.vertexFlags[i] & PreparationData::SourceContainsTangent) != 0);
        if ( !mForceTangentGen && bHasBinormal && bHasTangent )
            continue;

        // Retrieve the normal vector from the vertex and the computed
        // tangent vector.
        vNormal = *(cgVector3*)(pSrcVertices + nNormalOffset);
        T = pTangents[ i ];

        // GramSchmidt orthogonalize
        T = T - (vNormal * cgVector3::dot( vNormal, T ));
        cgVector3::normalize( T, T );

        // Store tangent if required
        if ( mForceTangentGen || (!bHasTangent && bRequiresTangent) )
            *(cgVector3*)(pSrcVertices + nTangentOffset) = T;

        // Compute and store binormal if required
        if ( mForceTangentGen || (!bHasBinormal && bRequiresBinormal) )
        {
            // Calculate the new orthogonal binormal
            cgVector3::cross( B, vNormal, T );
            cgVector3::normalize( B, B );

            // Compute the "handedness" of the tangent and binormal. This
            // ensures the inverted / mirrored texture coordinates still have
            // an accurate matrix.
            cgVector3::cross(vCross,vNormal,T);
            if ( cgVector3::dot( vCross, pBinormals[i] ) < 0.0f )
            {
                // Flip the binormal
                B = -B;

            } // End if coordinates inverted

            // Store.
            *(cgVector3*)(pSrcVertices + nBinormalOffset) = B;

        } // End if requires binormal   

    } // Next vertex

    // Cleanup 
    delete []pTangents;
    delete []pBinormals;

    // Return success 
    return true; 
}

//-----------------------------------------------------------------------------
//  Name : generateAdjacency ()
/// <summary>
/// Generates edge-triangle adjacency information for the mesh data either
/// prior to, or after building the hardware buffers. Input array will be
/// automatically sized, and will contain 3 values per triangle contained
/// in the mesh representing the indices to adjacent faces for each edge in 
/// the triangle (or 0xFFFFFFFF if there is no adjacent face).
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::generateAdjacency( cgUInt32Array & adjacency )
{
    std::map< AdjacentEdgeKey, cgUInt32 > EdgeTree;
    std::map< AdjacentEdgeKey, cgUInt32 >::iterator itEdge;
    
    // What is the status of the mesh?
    if ( mPrepareStatus != cgMeshStatus::Prepared )
    {
        // Validate requirements
        if ( mPrepareData.triangleCount == 0 )
            return false;

        // Retrieve useful data offset information.
        cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
        cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

        // Insert all edges into the edge tree
        cgByte * pSrcVertices = &mPrepareData.vertexData[0] + nPositionOffset;
        for ( cgUInt32 i = 0; i < mPrepareData.triangleCount; ++i )
        {
            AdjacentEdgeKey Edge;
            
            // Retrieve positions of each referenced vertex.
            const Triangle & Tri = mPrepareData.triangleData[i];
            const cgVector3 * v1 = (cgVector3*)(pSrcVertices + (Tri.indices[0] * nVertexStride));
            const cgVector3 * v2 = (cgVector3*)(pSrcVertices + (Tri.indices[1] * nVertexStride));
            const cgVector3 * v3 = (cgVector3*)(pSrcVertices + (Tri.indices[2] * nVertexStride));
            
            // Edge 1
            Edge.vertex1     = v1;
            Edge.vertex2     = v2;
            EdgeTree[ Edge ] = i;

            // Edge 2
            Edge.vertex1     = v2;
            Edge.vertex2     = v3;
            EdgeTree[ Edge ] = i;

            // Edge 3
            Edge.vertex1     = v3;
            Edge.vertex2     = v1;
            EdgeTree[ Edge ] = i;

        } // Next Face

        // Size the output array.
        adjacency.resize( mPrepareData.triangleCount * 3, 0xFFFFFFFF );

        // Now, find any adjacent edges for each triangle edge
        for ( cgUInt32 i = 0; i < mPrepareData.triangleCount; ++i )
        {
            AdjacentEdgeKey Edge;

            // Retrieve positions of each referenced vertex.
            const Triangle & Tri = mPrepareData.triangleData[i];
            const cgVector3 * v1 = (cgVector3*)(pSrcVertices + (Tri.indices[0] * nVertexStride));
            const cgVector3 * v2 = (cgVector3*)(pSrcVertices + (Tri.indices[1] * nVertexStride));
            const cgVector3 * v3 = (cgVector3*)(pSrcVertices + (Tri.indices[2] * nVertexStride));

            // Note: Notice below that the order of the edge vertices
            //       is swapped. This is because we want to find the 
            //       matching ADJACENT edge, rather than simply finding
            //       the same edge that we're currently processing.
            
            // Edge 1
            Edge.vertex2 = v1;
            Edge.vertex1 = v2;
            
            // Find the matching adjacent edge
            itEdge = EdgeTree.find( Edge );
            if ( itEdge != EdgeTree.end() )
                adjacency[ (i * 3) ] = itEdge->second;

            // Edge 2
            Edge.vertex2 = v2;
            Edge.vertex1 = v3;
            
            // Find the matching adjacent edge
            itEdge = EdgeTree.find( Edge );
            if ( itEdge != EdgeTree.end() )
                adjacency[ (i * 3) + 1 ] = itEdge->second;

            // Edge 3
            Edge.vertex2 = v3;
            Edge.vertex1 = v1;
            
            // Find the matching adjacent edge
            itEdge = EdgeTree.find( Edge );
            if ( itEdge != EdgeTree.end() )
                adjacency[ (i * 3) + 2 ] = itEdge->second;

        } // Next Face

    } // End if not prepared
    else
    {
        // Validate requirements
        if ( mFaceCount == 0 )
            return false;

        // Retrieve useful data offset information.
        cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
        cgInt nVertexStride   = (cgInt)mVertexFormat->getStride();

        // Insert all edges into the edge tree
        cgByte * pSrcVertices = mSystemVB + nPositionOffset;
        cgUInt32 * pSrcIndices  = mSystemIB;
        for ( cgUInt32 i = 0; i < mFaceCount; ++i, pSrcIndices+=3 )
        {
            AdjacentEdgeKey Edge;
            
            // Retrieve positions of each referenced vertex.
            const cgVector3 * v1 = (cgVector3*)(pSrcVertices + (pSrcIndices[0] * nVertexStride));
            const cgVector3 * v2 = (cgVector3*)(pSrcVertices + (pSrcIndices[1] * nVertexStride));
            const cgVector3 * v3 = (cgVector3*)(pSrcVertices + (pSrcIndices[2] * nVertexStride));
            
            // Edge 1
            Edge.vertex1     = v1;
            Edge.vertex2     = v2;
            EdgeTree[ Edge ] = i;

            // Edge 2
            Edge.vertex1     = v2;
            Edge.vertex2     = v3;
            EdgeTree[ Edge ] = i;

            // Edge 3
            Edge.vertex1     = v3;
            Edge.vertex2     = v1;
            EdgeTree[ Edge ] = i;

        } // Next Face

        // Size the output array.
        adjacency.resize( mFaceCount * 3, 0xFFFFFFFF );

        // Now, find any adjacent edges for each triangle edge
        pSrcIndices  = mSystemIB;
        for ( cgUInt32 i = 0; i < mFaceCount; ++i, pSrcIndices+=3 )
        {
            AdjacentEdgeKey Edge;

            // Retrieve positions of each referenced vertex.
            const cgVector3 * v1 = (cgVector3*)(pSrcVertices + (pSrcIndices[0] * nVertexStride));
            const cgVector3 * v2 = (cgVector3*)(pSrcVertices + (pSrcIndices[1] * nVertexStride));
            const cgVector3 * v3 = (cgVector3*)(pSrcVertices + (pSrcIndices[2] * nVertexStride));

            // Note: Notice below that the order of the edge vertices
            //       is swapped. This is because we want to find the 
            //       matching ADJACENT edge, rather than simply finding
            //       the same edge that we're currently processing.
            
            // Edge 1
            Edge.vertex2 = v1;
            Edge.vertex1 = v2;
            
            // Find the matching adjacent edge
            itEdge = EdgeTree.find( Edge );
            if ( itEdge != EdgeTree.end() )
                adjacency[ (i * 3) ] = itEdge->second;

            // Edge 2
            Edge.vertex2 = v2;
            Edge.vertex1 = v3;
            
            // Find the matching adjacent edge
            itEdge = EdgeTree.find( Edge );
            if ( itEdge != EdgeTree.end() )
                adjacency[ (i * 3) + 1 ] = itEdge->second;

            // Edge 3
            Edge.vertex2 = v3;
            Edge.vertex1 = v1;
            
            // Find the matching adjacent edge
            itEdge = EdgeTree.find( Edge );
            if ( itEdge != EdgeTree.end() )
                adjacency[ (i * 3) + 2 ] = itEdge->second;

        } // Next Face

    } // End if prepared

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : weldVertices ()
/// <summary>
/// Weld all of the vertices together that can be combined.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::weldVertices( cgUInt32Array * pVertexRemap /* = CG_NULL */ )
{
    WeldKey Key;
    std::map< WeldKey, cgUInt32 > VertexTree;
    std::map< WeldKey, cgUInt32 >::const_iterator itKey;
    cgByteArray NewVertexData, NewVertexFlags;
    cgUInt32 nNewVertexCount = 0;

    // Allocate enough space to build the remap array for the existing vertices
    if ( pVertexRemap )
        pVertexRemap->resize( mPrepareData.vertexCount );
    cgUInt32 * pCollapseMap = new cgUInt32[ mPrepareData.vertexCount ];

    // Retrieve useful data offset information.
    cgInt nVertexStride = (cgInt)mVertexFormat->getStride();

    // For each vertex to be welded.
    for ( cgUInt32 i = 0; i < mPrepareData.vertexCount; ++i )
    {
        // Build a new key structure for inserting
        Key.vertex  = (&mPrepareData.vertexData[0]) + (i * nVertexStride);
        Key.format  = mVertexFormat;
        Key.tolerance = CGE_EPSILON_1MM;

        // Does a vertex with matching details already exist in the tree.
        itKey = VertexTree.find( Key );
        if ( itKey == VertexTree.end() )
        {
            // No matching vertex. Insert into the tree (value = NEW index of vertex).
            VertexTree[Key] = nNewVertexCount;
            pCollapseMap[i] = nNewVertexCount;
            if ( pVertexRemap )
                (*pVertexRemap)[i] = nNewVertexCount;
            
            // Store the vertex in the new buffer
            NewVertexData.resize( (nNewVertexCount + 1) * nVertexStride );
            memcpy( &NewVertexData[ nNewVertexCount * nVertexStride], Key.vertex, nVertexStride );
            NewVertexFlags.push_back( mPrepareData.vertexFlags[i] );
            nNewVertexCount++;
            
        } // End if no matching vertex
        else
        {
            // A vertex already existed at this location.
            // Just mark the 'collapsed' index for this vertex in the remap array.
            pCollapseMap[i] = itKey->second;
            if ( pVertexRemap )
                (*pVertexRemap)[i] = 0xFFFFFFFF;

        } // End if vertex already existed

    } // Next Vertex

    // If nothing was welded, just bail
    if ( mPrepareData.vertexCount == nNewVertexCount )
    {
        delete []pCollapseMap;
        if ( pVertexRemap )
            pVertexRemap->clear();
        return true;
    
    } // End if nothing to do

    // Otherwise, replace the old preparation vertices and remap
    mPrepareData.vertexData.clear();
    mPrepareData.vertexData.resize( NewVertexData.size() );
    memcpy( &mPrepareData.vertexData[0], &NewVertexData[0], NewVertexData.size() );
    mPrepareData.vertexFlags.clear();
    mPrepareData.vertexFlags.resize( NewVertexFlags.size() );
    memcpy( &mPrepareData.vertexFlags[0], &NewVertexFlags[0], NewVertexFlags.size() );
    mPrepareData.vertexCount = nNewVertexCount;
    
    // Now remap all the triangle indices
    for ( cgUInt32 i = 0; i < mPrepareData.triangleCount; ++i )
    {
        Triangle & Tri = mPrepareData.triangleData[i];
        Tri.indices[0] = pCollapseMap[ Tri.indices[0] ];
        Tri.indices[1] = pCollapseMap[ Tri.indices[1] ];
        Tri.indices[2] = pCollapseMap[ Tri.indices[2] ];
    
    } // Next Triangle

    // Clean up
    delete []pCollapseMap;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : draw ()
/// <summary>
/// Draw the mesh in its entirety.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::draw( cgMeshDrawMode::Base Mode /* = cgMeshDrawMode::Automatic */ )
{
    // Set the vertex format (required in order to set stream source)
    cgRenderDriver * pDriver = getRenderDriver();
    pDriver->setVertexFormat( mVertexFormat );

    // Is this a hardware mesh?
    if ( mHardwareMesh == true )
    {
        // Restore the hardware vertex / index buffers if they are lost.
        restoreBuffers( );

        // Set vertex and index buffer source streams
        pDriver->setStreamSource( 0, mHardwareVB );
        pDriver->setIndices( mHardwareIB );
    
    } // End if hardware mesh

    // Should we get involved in the rendering process?
    cgInt32 nFaceStart = 0, nFaceCount = 0, nVertexStart = 0, nVertexCount = 0;
    cgMaterialHandle  hMaterial;
    if ( Mode == cgMeshDrawMode::Automatic )
    {
        // Because we're going to attempt to stitch together as many subsets
        // as we can based on their material, we must now collect together as
        // large a batch of triangles as possible. First, iterate through each 
        // subset in this format.
        SubsetArray::iterator itSubset;
        for ( itSubset = mMeshSubsets.begin(); itSubset != mMeshSubsets.end(); ++itSubset )
        {
            const MeshSubset * pSubset = *itSubset;

            // We'll only start rendering if and when the material changes
            if ( pSubset->material != hMaterial )
            {
                // Render previous collected batch
                if ( hMaterial.isValid() == true && nFaceCount > 0 )
                    renderMeshData( pDriver, cgMeshDrawMode::Automatic, &hMaterial, nFaceStart, nFaceCount, nVertexStart, nVertexCount );

                // Select new material and reset other values ready for next batch
                hMaterial    = pSubset->material;
                nFaceStart   = pSubset->faceStart;
                nFaceCount   = pSubset->faceCount;
                nVertexStart = pSubset->vertexStart;
                nVertexCount = pSubset->vertexCount;
                
            } // End if material changed
            else
            {
                int nSubsetVertStart, nSubsetVertEnd, nVertexEnd;

                // Collect render data from this subset. They should already 
                // be in order, so we can simply grow our face count.
                nFaceCount += pSubset->faceCount;

                // Vertex start/end is a little more complex, but can be computed
                // using a containment style test. First precompute some values to
                // make the tests a little simpler.
                nSubsetVertStart = pSubset->vertexStart;
                nSubsetVertEnd   = nSubsetVertStart + pSubset->vertexCount - 1;
                nVertexEnd       = nVertexStart + nVertexCount - 1;

                // Perform the containment tests
                if ( nSubsetVertStart < nVertexStart ) nVertexStart = nSubsetVertStart;
                if ( nSubsetVertStart > nVertexEnd   ) nVertexEnd   = nSubsetVertStart;
                if ( nSubsetVertEnd < nVertexStart   ) nVertexStart = nSubsetVertEnd;
                if ( nSubsetVertEnd > nVertexEnd     ) nVertexEnd   = nSubsetVertEnd;

                // Finally, compute the new vertex count (vertex start will already have been adjusted)
                nVertexCount = (nVertexEnd - nVertexStart) + 1;

            } // End if material did not change

        } // Next subset

        // Render any remaining data
        if ( hMaterial.isValid() == true && nFaceCount > 0 )
            renderMeshData( pDriver, cgMeshDrawMode::Automatic, &hMaterial, (cgUInt32)nFaceStart, (cgUInt32)nFaceCount, (cgUInt32)nVertexStart, (cgUInt32)nVertexCount );

    } // End if Automatic
    else
    {
        // Just render the entire mesh as requested.
        renderMeshData( pDriver, Mode, NULL, 0, getFaceCount(), 0, getVertexCount() );
    
    } // End if Simple
}

//-----------------------------------------------------------------------------
//  Name : drawSubset ()
/// <summary>
/// Draw an individual subset of the mesh based on the material 
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::drawSubset( const cgMaterialHandle & hMaterial, cgMeshDrawMode::Base Mode /* = cgMeshDrawMode::Automatic */ )
{
    // Do we have any data for the specified material?
    MaterialSubsetMap::const_iterator itMaterial = mMaterials.find( hMaterial );
    if ( itMaterial == mMaterials.end() )
        return;
    
    // Set the vertex format (required in order to set stream source)
    cgRenderDriver * pDriver = getRenderDriver();
    pDriver->setVertexFormat( mVertexFormat );

    // Is this a hardware mesh?
    if ( mHardwareMesh )
    {
        // Restore the hardware vertex / index buffers if they are lost.
        restoreBuffers( );
        
        // Set vertex and index buffer source streams
        pDriver->setStreamSource( 0, mHardwareVB );
        pDriver->setIndices( mHardwareIB );
        
    } // End if hardware mesh

    // Process and draw all subsets of the mesh that use the specified material.
    cgInt32 nSubsetVertStart, nSubsetVertEnd;
    cgInt32 nFaceStart = 0, nFaceCount = 0, nVertexStart = 0, nVertexEnd = 0, nVertexCount = 0;
    const SubsetArray & Subsets = itMaterial->second;
    if ( !Subsets.empty() )
    {
        // Process first subset (simpler path).
        const MeshSubset * pSubset = Subsets[0];
        nFaceStart   = pSubset->faceStart;
        nFaceCount   = pSubset->faceCount;
        nVertexStart = pSubset->vertexStart;
        nVertexEnd   = nVertexStart + pSubset->vertexCount - 1;

        // Process remaining subsets.
        for ( size_t i = 1; i < Subsets.size(); ++i )
        {
            // Collect render data from this subset. They should already 
            // be in order, so we can simply grow our face count.
            pSubset = Subsets[i];
            nFaceCount += pSubset->faceCount;

            // Vertex start/end is a little more complex, but can be computed
            // using a containment style test. First precompute some values to
            // make the tests a little simpler.
            nSubsetVertStart = pSubset->vertexStart;
            nSubsetVertEnd   = nSubsetVertStart + pSubset->vertexCount - 1;

            // Perform the containment tests
            if ( nSubsetVertStart < nVertexStart ) nVertexStart = nSubsetVertStart;
            if ( nSubsetVertStart > nVertexEnd   ) nVertexEnd   = nSubsetVertStart;
            if ( nSubsetVertEnd < nVertexStart   ) nVertexStart = nSubsetVertEnd;
            if ( nSubsetVertEnd > nVertexEnd     ) nVertexEnd   = nSubsetVertEnd;

        } // Next subset

        // Compute the final vertex count.
        nVertexCount = (nVertexEnd - nVertexStart) + 1;

    } // End if any subsets
    
    // Render any batched data.
    if ( nFaceCount > 0 && (Mode != cgMeshDrawMode::Automatic || hMaterial.isValid() ) )
        renderMeshData( pDriver, Mode, &hMaterial, (cgUInt32)nFaceStart, (cgUInt32)nFaceCount, (cgUInt32)nVertexStart, (cgUInt32)nVertexCount );
}

//-----------------------------------------------------------------------------
//  Name : drawSubset ()
/// <summary>
/// Draw an individual subset of the mesh based on the material AND
/// data group specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::drawSubset( const cgMaterialHandle & hMaterial, cgUInt32 nDataGroupId, cgMeshDrawMode::Base Mode /* = cgMeshDrawMode::Automatic */ )
{
    // Attempt to find a matching subset.
    SubsetKeyMap::iterator itSubset = mSubsetLookup.find( MeshSubsetKey( hMaterial, nDataGroupId ) );
    if ( itSubset == mSubsetLookup.end() )
        return;

    // Set the vertex format (required in order to set stream source)
    cgRenderDriver * pDriver = getRenderDriver();
    pDriver->setVertexFormat( mVertexFormat );

    // Is this a hardware mesh?
    if ( mHardwareMesh == true )
    {
        // Restore the hardware vertex / index buffers if they are lost.
        restoreBuffers( );

        // Set vertex and index buffer source streams
        pDriver->setStreamSource( 0, mHardwareVB );
        pDriver->setIndices( mHardwareIB );
    
    } // End if hardware mesh

    // Render this subset directly.
    MeshSubset * pSubset = itSubset->second;
    renderMeshData( pDriver, Mode, &hMaterial, (cgUInt32)pSubset->faceStart, (cgUInt32)pSubset->faceCount, 
                    (cgUInt32)pSubset->vertexStart, (cgUInt32)pSubset->vertexCount );
}

//-----------------------------------------------------------------------------
//  Name : drawSubset ()
/// <summary>
/// Draw an individual subset of the mesh based on the single data group
/// identifier specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::drawSubset( cgUInt32 nDataGroupId, cgMeshDrawMode::Base Mode /* = cgMeshDrawMode:::Automatic */  )
{
    // Do we have any data for the specified data group?
    DataGroupSubsetMap::iterator itDataGroup = mDataGroups.find( nDataGroupId );
    if ( itDataGroup == mDataGroups.end() ) return;

    // Set the vertex format (required in order to set stream source)
    cgRenderDriver * pDriver = getRenderDriver();
    pDriver->setVertexFormat( mVertexFormat );

    // Is this a hardware mesh?
    if ( mHardwareMesh == true )
    {
        // Restore the hardware vertex / index buffers if they are lost.
        restoreBuffers( );

        // Set vertex and index buffer source streams
        pDriver->setStreamSource( 0, mHardwareVB );
        pDriver->setIndices( mHardwareIB );
    
    } // End if hardware mesh

    // Process and draw all subsets of the mesh that belong to the selected data group. 
    SubsetArray::const_iterator itSubset;
    const SubsetArray & Subsets = itDataGroup->second;
    cgInt nFaceStart = 0, nFaceCount = 0, nVertexStart = 0, nVertexCount = 0;
    const cgMaterialHandle * pRenderAttrib = CG_NULL;
    for ( itSubset = Subsets.begin(); itSubset != Subsets.end(); ++itSubset )
    {
        const MeshSubset * pSubset        = *itSubset;
        bool               bContinueBatch = false;

        // What method are we using to render?
        if ( Mode == cgMeshDrawMode::Simple )
        {
            // If we're not setting materials we can simply collect together as 
            // many triangles as we can that belong to this data group and render
            // them all in as few batches as possible. Continue building batch
            // while the triangle data is contiguous.
            if ( pSubset->faceStart == (nFaceStart + nFaceCount) )
                bContinueBatch = (nFaceCount != 0);

            // Do not pay attention to materials in these modes.
            pRenderAttrib = CG_NULL;

        } // End if simple draw
        else
        {
            // Otherwise, in full automatic mode we must stop and draw for each
            // new material that we encounter. Continue building batch while
            // the material matches.
            if ( (pRenderAttrib != CG_NULL) && (pSubset->material == *pRenderAttrib) )
                bContinueBatch = (nFaceCount != 0);

            // We should render using materials.
            pRenderAttrib = &pSubset->material;

        } // End if automatic draw

        // Continuing the batch we're building?
        if ( bContinueBatch == true )
        {
            int nSubsetVertStart, nSubsetVertEnd, nVertexEnd;

            // Collect render data from this subset. They should already 
            // be in order, so we can simply grow our face count.
            nFaceCount += pSubset->faceCount;

            // Vertex start/end is a little more complex, but can be computed
            // using a containment style test. First precompute some values to
            // make the tests a little simpler.
            nSubsetVertStart = pSubset->vertexStart;
            nSubsetVertEnd   = nSubsetVertStart + pSubset->vertexCount - 1;
            nVertexEnd       = nVertexStart + nVertexCount - 1;

            // Perform the containment tests
            if ( nSubsetVertStart < nVertexStart ) nVertexStart = nSubsetVertStart;
            if ( nSubsetVertStart > nVertexEnd   ) nVertexEnd   = nSubsetVertStart;
            if ( nSubsetVertEnd < nVertexStart   ) nVertexStart = nSubsetVertEnd;
            if ( nSubsetVertEnd > nVertexEnd     ) nVertexEnd   = nSubsetVertEnd;

            // Finally, compute the new vertex count (vertex start will already have been adjusted)
            nVertexCount = (nVertexEnd - nVertexStart) + 1;

        } // End if consecutive primitives
        else
        {
            // This is a new batch of data so draw anything previously batched.
            if ( nFaceCount > 0 && (pRenderAttrib == CG_NULL || pRenderAttrib->isValid() == true ) )
                renderMeshData( pDriver, Mode, pRenderAttrib, (cgUInt32)nFaceStart, (cgUInt32)nFaceCount, (cgUInt32)nVertexStart, (cgUInt32)nVertexCount );

            // Start a new batch
            nFaceStart   = pSubset->faceStart;
            nFaceCount   = pSubset->faceCount;
            nVertexStart = pSubset->vertexStart;
            nVertexCount = pSubset->vertexCount;

        } // End if new batch

    } // Next subset

    // Render any remaining data
    if ( nFaceCount > 0 && (pRenderAttrib == CG_NULL || pRenderAttrib->isValid() == true ) )
        renderMeshData( pDriver, Mode, pRenderAttrib, (cgUInt32)nFaceStart, (cgUInt32)nFaceCount, (cgUInt32)nVertexStart, (cgUInt32)nVertexCount );
}

//-----------------------------------------------------------------------------
//  Name : drawSubset ()
/// <summary>
/// Draw an individual subset of the mesh based on the material 
/// specified and the list of data groups supplied.
/// Note : Supplied data group list must be pre-sorted in ascending numeric 
/// order.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::drawSubset( const cgMaterialHandle & hMaterial, cgUInt32Array & DataGroups, cgMeshDrawMode::Base Mode /* = cgMeshDrawMode:::Automatic */  )
{
    // Do we have any data for the specified material?
    MaterialSubsetMap::iterator itMaterial = mMaterials.find( hMaterial );
    if ( itMaterial == mMaterials.end() ) return;

    // Set the vertex format (required in order to set stream source)
    cgRenderDriver * pDriver = getRenderDriver();
    pDriver->setVertexFormat( mVertexFormat );

    // Is this a hardware mesh?
    if ( mHardwareMesh == true )
    {
        // Restore the hardware vertex / index buffers if they are lost.
        restoreBuffers( );

        // Set vertex and index buffer source streams
        pDriver->setStreamSource( 0, mHardwareVB );
        pDriver->setIndices( mHardwareIB );
    
    } // End if hardware mesh

    // Process and draw all subsets of the mesh that use the specified material.
    bool bMaterialSet = false;
    SubsetArray::const_iterator itSubset;
    const SubsetArray & Subsets = itMaterial->second;
    cgInt32 nCurrentGroup = 0, nFaceStart = 0, nFaceCount = 0, nVertexStart = 0, nVertexCount = 0;
    for ( itSubset = Subsets.begin(); itSubset != Subsets.end() && nCurrentGroup < (int)DataGroups.size(); )
    {
        const MeshSubset * pSubset        = *itSubset;
        bool               bContinueBatch = false;

        // Is this a subset we should pay attention to?
        if ( pSubset->dataGroupId < DataGroups[nCurrentGroup] )
        {
            // Skip this subset.
            ++itSubset;
            continue;

        } // End if catch up
        else if ( pSubset->dataGroupId > DataGroups[nCurrentGroup] )
        {
            // Skip this data group.
            nCurrentGroup++;
            continue;

        } // End if out of groups
        else
        {
            // Group matches, process.
            nCurrentGroup++;
        
        } // End if match

        // Is this subset contiguous with data collected so far?
        if ( pSubset->faceStart == (nFaceStart + nFaceCount) )
            bContinueBatch = (nFaceCount != 0);

        // Continuing the batch we're building?
        if ( bContinueBatch == true )
        {
            int nSubsetVertStart, nSubsetVertEnd, nVertexEnd;

            // Collect render data from this subset. They should already 
            // be in order, so we can simply grow our face count.
            nFaceCount += pSubset->faceCount;

            // Vertex start/end is a little more complex, but can be computed
            // using a containment style test. First precompute some values to
            // make the tests a little simpler.
            nSubsetVertStart = pSubset->vertexStart;
            nSubsetVertEnd   = nSubsetVertStart + pSubset->vertexCount - 1;
            nVertexEnd       = nVertexStart + nVertexCount - 1;

            // Perform the containment tests
            if ( nSubsetVertStart < nVertexStart ) nVertexStart = nSubsetVertStart;
            if ( nSubsetVertStart > nVertexEnd   ) nVertexEnd   = nSubsetVertStart;
            if ( nSubsetVertEnd < nVertexStart   ) nVertexStart = nSubsetVertEnd;
            if ( nSubsetVertEnd > nVertexEnd     ) nVertexEnd   = nSubsetVertEnd;

            // Finally, compute the new vertex count (vertex start will already have been adjusted)
            nVertexCount = (nVertexEnd - nVertexStart) + 1;

        } // End if consecutive primitives
        else
        {
            // This is a new batch of data so draw anything previously batched.
            if ( nFaceCount > 0 )
            {
                // Set material (once) if we're in automatic rendering mode.
                if ( bMaterialSet == false && Mode == cgMeshDrawMode::Automatic )
                {
                    pDriver->setMaterial( hMaterial );
                    bMaterialSet = true;
                
                } // End if set material

                // Draw collected data
                renderMeshData( pDriver, Mode, CG_NULL, (cgUInt32)nFaceStart, (cgUInt32)nFaceCount, (cgUInt32)nVertexStart, (cgUInt32)nVertexCount );

            } // End if something to draw

            // Start a new batch
            nFaceStart   = pSubset->faceStart;
            nFaceCount   = pSubset->faceCount;
            nVertexStart = pSubset->vertexStart;
            nVertexCount = pSubset->vertexCount;

        } // End if new batch

    } // Next subset

    // Render any remaining data
    if ( nFaceCount > 0 )
    {
        // Set material (once) if we're in automatic rendering mode.
        if ( bMaterialSet == false && Mode == cgMeshDrawMode::Automatic )
            pDriver->setMaterial( hMaterial );
        
        // Draw collected data
        renderMeshData( pDriver, Mode, CG_NULL, (cgUInt32)nFaceStart, (cgUInt32)nFaceCount, (cgUInt32)nVertexStart, (cgUInt32)nVertexCount );

    } // End if something to draw
}

//-----------------------------------------------------------------------------
//  Name : drawSubset ()
/// <summary>
/// Draw an individual subset of the mesh based on the list of data 
/// groups supplied.
/// Note : Supplied data group list must be pre-sorted in ascending numeric 
/// order.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::drawSubset( cgUInt32Array & DataGroups, cgMeshDrawMode::Base Mode /* = cgMeshDrawMode::Automatic */  )
{
    // ToDo:
}

// ToDo: Do we still need this?
//-----------------------------------------------------------------------------
//  Name : draw ()
/// <summary>
/// Draw the mesh (face count = nNumFaces).
/// Note : Used during volume rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::draw( cgUInt32 nNumFaces )
{
    // Get access to required systems
    cgRenderDriver * pDriver = getRenderDriver();

    // Set the vertex format (required in order to set stream source)
    pDriver->setVertexFormat( mVertexFormat );

    // Is this a hardware mesh?
    if ( mHardwareMesh == true )
    {
        // Restore the hardware vertex / index buffers if they are lost.
        restoreBuffers( );

        // Set vertex and index buffer source streams
        pDriver->setStreamSource( 0, mHardwareVB );
        pDriver->setIndices( mHardwareIB );
    
    } // End if hardware mesh

	// Draw collected data
    renderMeshData( pDriver, cgMeshDrawMode::Simple, CG_NULL, 0, nNumFaces, 0, mVertexCount );
}

//-----------------------------------------------------------------------------
//  Name : restoreBuffers () (Private)
/// <summary>
/// If any hardware buffer resources have been lost, restore them.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::restoreBuffers( )
{
    // Do we need to re-upload the vertex data?
    if ( mHardwareVB.isResourceLost() )
    {
        // Copy the vertex data over.
        cgVertexBuffer * pBuffer = mHardwareVB.getResource(true);
        if ( !pBuffer || !pBuffer->updateBuffer( 0, 0, mSystemVB ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to re-populate vertex buffer after the data was classified as lost.\n") );
            return false;

        } // End if failed

        // Resource has been restored.
        mHardwareVB.setResourceLost( false );

    } // End if VB resource lost

    // Do we need to re-upload the index data?
    if ( mHardwareIB.isResourceLost() )
    {
        // Copy the index data over.
        cgIndexBuffer * pBuffer = mHardwareIB.getResource(true);
        if ( !pBuffer || !pBuffer->updateBuffer( 0, 0, mSystemIB ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to re-populate index buffer after the data was classified as lost.\n") );
            return false;

        } // End if failed

        // Resource has been restored.
        mHardwareIB.setResourceLost( false );

    } // End if IB resource lost

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : renderMeshData () (Private)
/// <summary>
/// Given the specified subset values, simply render the selected batch
/// of primitives the required number of times based on their material.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::renderMeshData( cgRenderDriver * pDriver, cgMeshDrawMode::Base Mode, const cgMaterialHandle * pMaterialHandle, cgUInt32 nFaceStart, cgUInt32 nFaceCount, cgUInt32 nVertexStart, cgUInt32 nVertexCount )
{
    // Set the material data to the driver?
    if ( pMaterialHandle )
    {
        if ( Mode == cgMeshDrawMode::Automatic )
        {
            // If this is the 'default' material for this mesh, override its properties
            // such that we render using the supplied default color.
            if ( mDefaultMaterial.isValid() && *pMaterialHandle == mDefaultMaterial )
            {
                cgStandardMaterial * pMaterial = (cgStandardMaterial*)mDefaultMaterial.getResource( true );
                pMaterial->setDiffuse( mDefaultColor );
                
            } // End if default material

            // Set material and bail if this material was disallowed.
            if ( !pDriver->setMaterial( *pMaterialHandle ) )
                return;
        
        } // End if set materials
        else
        {
            // If this is the 'default' material for this mesh, override its properties
            // such that we render using the supplied default color.
            if ( mDefaultMaterial.isValid() && *pMaterialHandle == mDefaultMaterial &&
                 pDriver->getMaterial() == mDefaultMaterial )
            {
                cgStandardMaterial * pMaterial = (cgStandardMaterial*)mDefaultMaterial.getResource( true );
                cgMaterialTerms Terms = pMaterial->getMaterialTerms();
                Terms.diffuse = mDefaultColor;
                pDriver->setMaterialTerms( Terms );
                
            } // End if default material

        } // End if !set materials
    
    } // End if attrib supplied
    
    // Automatic handling of effect rendering?
    if ( Mode != cgMeshDrawMode::Simple )
    {
        // Begin. Applies device states
        if ( pDriver->beginMaterialRender( ) )
        {
            // Begin rendering for the required number of passes.
            while ( pDriver->executeMaterialPass( ) == cgTechniqueResult::Continue )
            {
                // Hardware or software rendering?
                if ( mHardwareMesh == true )
                {
                    // Render using hardware streams
                    pDriver->drawIndexedPrimitive( cgPrimitiveType::TriangleList, 0, nVertexStart, nVertexCount, nFaceStart * 3, nFaceCount );
                    
                } // End if has hardware copy
                else
                {
                    // ToDo: FIXME : Specifying the correct values for pSubset->nVertexStart and pSubset->nVertexCount causes mesh to render incorrectly! Doesn't fail if using SoftwareVP.. Pretty much confirmed as a driver bug?
                    pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, mVertexCount, nFaceCount, &mSystemIB[ nFaceStart * 3 ],
                                                     cgBufferFormat::Index32, mSystemVB );
                    /*pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, pSubset->nVertexStart, pSubset->nVertexCount, pSubset->nFaceCount, 
                                                     &mSystemIB[ pSubset->nFaceStart * 3 ], cgBufferFormat::Index32, mSystemVB );*/

                } // End if software only copy

            } // Next Render pass

            // Finish up
            pDriver->endMaterialRender( );

        } // End if begun
        
    } // End if handle effects
    else
    {
        // Just render the geometry. Hardware or software rendering?
        if ( mHardwareMesh )
        {
            // Render using hardware streams
            pDriver->drawIndexedPrimitive( cgPrimitiveType::TriangleList, 0, nVertexStart, nVertexCount, nFaceStart * 3, nFaceCount );
        
        } // End if has hardware copy
        else
        {
            // ToDo: FIXME : Specifying the correct values for pSubset->vertexStart and pSubset->nVertexCount causes mesh to render incorrectly! Doesn't fail if using SoftwareVP.. Pretty much confirmed as a 6800 driver bug?
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, mVertexCount, nFaceCount, 
                                             &mSystemIB[ nFaceStart * 3 ], cgBufferFormat::Index32, mSystemVB );
            /*pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, pSubset->vertexStart, pSubset->nVertexCount, pSubset->faceCount, 
                                             &mSystemIB[ pSubset->nFaceStart * 3 ], cgBufferFormat::Index32, mSystemVB );*/

        } // End if software only copy

    } // End if simple
}

//-----------------------------------------------------------------------------
//  Name : findVertexOptimizerScore () (Private, Static)
/// <summary>
/// During optimization, this method will generate scores used to
/// identify important vertices when ordering triangle data.
/// Note : Thanks to Tom Forsyth for the fantastic implementation on which
/// this is based.
/// URL  : http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgMesh::findVertexOptimizerScore( const OptimizerVertexInfo * pVertexInfo )
{
    cgFloat Score = 0.0f;

    // Do any remaining triangles use this vertex?
    if ( pVertexInfo->unusedTriangleReferences == 0 )
        return -1.0f;
    
    cgInt32 CachePosition = pVertexInfo->cachePosition;
    if ( CachePosition < 0 )
    {

        // Vertex is not in FIFO cache - no score.
    }
    else
    {
        if ( CachePosition < 3 )
        {
            // This vertex was used in the last triangle,
            // so it has a fixed score, whichever of the three
            // it's in. Otherwise, you can get very different
            // answers depending on whether you add
            // the triangle 1,2,3 or 3,1,2 - which is silly.
            Score = MeshOptimizer::LastTriScore;
        }
        else
        {
            // Points for being high in the cache.
            const cgFloat Scaler = 1.0f / ( MeshOptimizer::MaxVertexCacheSize - 3 );
            Score = 1.0f - ( CachePosition - 3 ) * Scaler;
            Score = powf ( Score, MeshOptimizer::CacheDecayPower );
        }
    
    } // End if already in vertex cache

    // Bonus points for having a low number of tris still to
    // use the vert, so we get rid of lone verts quickly.
    cgFloat ValenceBoost = powf ( (cgFloat)pVertexInfo->unusedTriangleReferences, -MeshOptimizer::ValenceBoostPower );
    Score += MeshOptimizer::ValenceBoostScale * ValenceBoost;

    // Return the final score
    return Score;
}

//-----------------------------------------------------------------------------
//  Name : buildOptimizedIndexBuffer () (Private, Static)
/// <summary>
/// Calculate the best order for triangle data, optimizing for efficient
/// use of the hardware vertex cache, given an unkown vertex cache size
/// and implementation.
/// Note : Thanks to Tom Forsyth for the fantastic implementation on which
/// this is based.
/// URL  : http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::buildOptimizedIndexBuffer( const MeshSubset * pSubset, cgUInt32 * pSrcBuffer, cgUInt32 * pDestBuffer, cgUInt32 nMinVertex, cgUInt32 nMaxVertex )
{
    OptimizerVertexInfo   * pVertexInfo   = CG_NULL, * pVert;
    OptimizerTriangleInfo * pTriangleInfo = CG_NULL, * pTri;
    cgUInt32                i, j, k, nVertexCount;
    cgFloat                 fBestScore    = 0.0f, fScore;
    cgInt32                 nBestTriangle = -1;
    cgUInt32                nVertexCacheSize = 0;
    cgUInt32                nIndex, nTriangleIndex, nTemp;

    // Declare vertex cache storage (plus one to allow them to drop "off the end")
    cgUInt32 pVertexCache[ MeshOptimizer::MaxVertexCacheSize + 1 ];

    // First allocate enough room for the optimization information for each vertex and triangle
    nVertexCount  = (nMaxVertex - nMinVertex) + 1;
    pVertexInfo   = new OptimizerVertexInfo[ nVertexCount ];
    pTriangleInfo = new OptimizerTriangleInfo[ pSubset->faceCount ];

    // The first pass is to initialize the vertex information with information about the
    // faces which reference them.
    for ( i = 0; i < (unsigned)pSubset->faceCount; ++i )
    {
        nIndex = pSrcBuffer[i*3] - nMinVertex;    
        pVertexInfo[ nIndex ].unusedTriangleReferences++;
        pVertexInfo[ nIndex ].triangleReferences.push_back( i );
        nIndex = pSrcBuffer[ (i*3) + 1 ] - nMinVertex;
        pVertexInfo[ nIndex ].unusedTriangleReferences++;
        pVertexInfo[ nIndex ].triangleReferences.push_back( i );
        nIndex = pSrcBuffer[ (i*3) + 2 ] - nMinVertex;
        pVertexInfo[ nIndex ].unusedTriangleReferences++;
        pVertexInfo[ nIndex ].triangleReferences.push_back( i );

    } // Next Triangle

    // Initialize vertex scores
    for ( i = 0; i < nVertexCount; ++i )
        pVertexInfo[i].vertexScore = findVertexOptimizerScore( &pVertexInfo[i] );

    // Compute the score for each triangle, and record the triangle with the best score
    for ( i = 0; i < (unsigned)pSubset->faceCount; ++i )
    {
        // The triangle score is the sum of the scores of each of
        // its three vertices.
        nIndex = pSrcBuffer[i*3] - nMinVertex;
        fScore = pVertexInfo[ nIndex ].vertexScore;
        nIndex = pSrcBuffer[ (i*3) + 1 ] - nMinVertex;
        fScore += pVertexInfo[ nIndex ].vertexScore;
        nIndex = pSrcBuffer[ (i*3) + 2 ] - nMinVertex;
        fScore += pVertexInfo[ nIndex ].vertexScore;
        pTriangleInfo[i].triangleScore = fScore;

        // Record the triangle with the highest score
        if ( fScore > fBestScore )
        {
            fBestScore    = fScore;
            nBestTriangle = (signed)i;
        
        } // End if better than previous score

    } // Next Triangle

    // Now we can start adding triangles, beginning with the previous highest scoring triangle.
    for ( i = 0; i < (unsigned)pSubset->faceCount; ++i )
    {
        // If we don't know the best triangle, for whatever reason, find it
        if ( nBestTriangle < 0 )
        {
            nBestTriangle  = -1;
            fBestScore     = 0.0f;
        
            // Iterate through the entire list of un-added faces
            for ( j = 0; j < (unsigned)pSubset->faceCount; ++j )
            {
                if ( pTriangleInfo[j].added == false )
                {
                    fScore = pTriangleInfo[j].triangleScore;

                    // Record the triangle with the highest score
                    if ( fScore > fBestScore )
                    {
                        fBestScore    = fScore;
                        nBestTriangle = (signed)j;
                    
                    } // End if better than previous score

                } // End if not added

            } // Next Triangle

        } // End if best triangle is not known

        // Use the best scoring triangle from last pass and reset score keeping
        nTriangleIndex = (unsigned)nBestTriangle;
        pTri           = &pTriangleInfo[ nTriangleIndex ];
        nBestTriangle  = -1;
        fBestScore     = 0.0f;

        // This triangle can be added to the 'draw' list, and each
        // of the vertices it references should be updated.
        pTri->added   = true;
        for ( j = 0; j < 3; ++j )
        {
            // Extract the vertex index and store in the index buffer
            nIndex         = pSrcBuffer[ (nTriangleIndex * 3) + j ];
            *pDestBuffer++ = nIndex;

            // Adjust the index so that it points into our info buffer
            // rather than the actual source vertex itself.
            nIndex = nIndex - nMinVertex;

            // Retrieve the referenced vertex information
            pVert = &pVertexInfo[ nIndex ];

            // Reduce the 'valence' of this vertex (one less triangle is now referencing)
            pVert->unusedTriangleReferences--;

            // Remove this triangle from the list of references in the vertex
            cgUInt32Array::iterator itReference = std::find( pVert->triangleReferences.begin(), pVert->triangleReferences.end(), nTriangleIndex );
            if ( itReference != pVert->triangleReferences.end() )
                pVert->triangleReferences.erase( itReference );

            // Now we must update the vertex cache to include this vertex. If it was
            // already in the cache, it should be moved to the head, otherwise it should
            // be inserted (pushing one off the end).
            if ( pVert->cachePosition == -1 )
            {
                // Not in the vertex cache, insert it at the head.
                if ( nVertexCacheSize > 0 )
                {
                    // First shuffle EVERYONE up by one position in the cache.
                    memmove( &pVertexCache[1], &pVertexCache[0], nVertexCacheSize * sizeof(cgUInt32) );

                } // End if any vertices exist in the cache

                // Grow the cache if applicable
                if ( nVertexCacheSize < MeshOptimizer::MaxVertexCacheSize )
                    nVertexCacheSize++;
                else
                {
                    // Set the associated index of the vertex which dropped "off the end" of the cache.
                    pVertexInfo[ pVertexCache[ nVertexCacheSize ] ].cachePosition = -1;

                } // End if no more room

                // Overwrite the first entry
                pVertexCache[0] = nIndex;

            } // End if not in cache
            else if ( pVert->cachePosition > 0 )
            {
                // Already in the vertex cache, move it to the head.
                // Note : If the cache position is already 0, we just ignore
                // it... hence the above 'else if' rather than just 'else'.
                if ( pVert->cachePosition == 1 )
                {
                    // We were in the second slot, just swap the two
                    nTemp = pVertexCache[0];
                    pVertexCache[0] = nIndex;
                    pVertexCache[1] = nTemp;

                } // End if simple swap
                else
                {
                    // Shuffle EVERYONE up who came before us.
                    memmove( &pVertexCache[1], &pVertexCache[0], pVert->cachePosition * sizeof(cgUInt32) );

                    // Insert this vertex at the head
                    pVertexCache[0] = nIndex;

                } // End if memory move required

            } // End if already in cache

            // Update the cache position records for all vertices in the cache
            for ( k = 0; k < nVertexCacheSize; ++k )
                pVertexInfo[pVertexCache[k]].cachePosition = k;

        } // Next Index

        // Recalculate the of all vertices contained in the cache
        for ( j = 0; j < nVertexCacheSize; ++j )
        {
            pVert = &pVertexInfo[pVertexCache[j]];
            pVert->vertexScore = findVertexOptimizerScore( pVert );

        } // Next entry in the vertex cache

        // Update the score of the triangles which reference this vertex
        // and record the highest scoring.
        for ( j = 0; j < nVertexCacheSize; ++j )
        {
            pVert = &pVertexInfo[pVertexCache[j]];

            // For each triangle referenced
            for ( k = 0; k < pVert->unusedTriangleReferences; ++k )
            {
                nTriangleIndex       = pVert->triangleReferences[k];
                pTri                 = &pTriangleInfo[ nTriangleIndex ];
                fScore               = pVertexInfo[ pSrcBuffer[ (nTriangleIndex * 3) ] - nMinVertex ].vertexScore;
                fScore              += pVertexInfo[ pSrcBuffer[ (nTriangleIndex * 3) + 1 ] - nMinVertex ].vertexScore;
                fScore              += pVertexInfo[ pSrcBuffer[ (nTriangleIndex * 3) + 2 ] - nMinVertex ].vertexScore;
                pTri->triangleScore = fScore;

                // Highest scoring so far?
                if ( fScore > fBestScore )
                {
                    fBestScore    = fScore;
                    nBestTriangle = (signed)nTriangleIndex;
                
                } // End if better than previous score

            } // Next Triangle

        } // Next entry in the vertex cache

    } // Next Triangle to Add

    // Destroy the temporary arrays
    delete []pVertexInfo;
    delete []pTriangleInfo;
}

//-----------------------------------------------------------------------------
// Name : pick ( )
/// <summary>
/// Given the specified object space ray, determine if the mesh data is 
/// intersected at all and also compute the intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::pick( const cgVector3 & vOrigin, const cgVector3 & vDir, cgFloat & fDistance )
{
    // Mesh must be prepared for picking to be supported.
    if ( mPrepareStatus != cgMeshStatus::Prepared )
        return false;

    // Reset parameters as required
    fDistance = FLT_MAX;
    
    // First, test for broadphase intersection against our bounding box
    if ( mBoundingBox.isPopulated() == true )
    {
        if ( !mBoundingBox.intersect( vOrigin, vDir, fDistance, false ) )
            return false;
        
    } // End if bounds generated

    // Now test the physical mesh data.
    cgUInt32 nFace;
    cgMaterialHandle hMaterial;
    cgFloat fLocalDistance = FLT_MAX;
    if ( pickMeshSubset( 0xFFFFFFFF, CG_NULL, cgSize(), cgTransform::Identity, vOrigin, vDir, false, 0, fLocalDistance, nFace, hMaterial ) == false )
        return false;

    // Intersected!
    fDistance = fLocalDistance;
    return true;
}

//-----------------------------------------------------------------------------
// Name : pick ( )
/// <summary>
/// Given the specified object space ray, determine if the mesh data is 
/// intersected at all and also compute the intersection distance. This 
/// overload supports both wireframe and solid picking.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::pick( cgCameraNode * pCamera, const cgSize & ViewportSize, const cgTransform & ObjectTransform, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Mesh must be prepared for picking to be supported.
    if ( mPrepareStatus != cgMeshStatus::Prepared )
        return false;

    // Reset parameters as required
    fDistance = FLT_MAX;
    
    // First, test for broadphase intersection against our bounding box
    if ( mBoundingBox.isPopulated() == true )
    {
        // If we're in wireframe mode, this must be expanded to include the wireframe tolerance.
        if ( !bWireframe )
        {
            if ( !mBoundingBox.intersect( vOrigin, vDir, fDistance, false ) )
                return false;

        } // End if solid
        else
        {
            cgBoundingBox WireBounds = mBoundingBox;
            WireBounds.inflate( pCamera->estimatePickTolerance( ViewportSize, fWireTolerance, WireBounds.getCenter(), ObjectTransform ) );
            if ( !WireBounds.intersect( vOrigin, vDir, fDistance, false ) )
                return false;
        
        } // End if wireframe
        
    } // End if bounds generated

    // Now test the physical mesh data.
    cgUInt32 nFace;
    cgMaterialHandle hMaterial;
    cgFloat fLocalDistance = FLT_MAX;
    if ( pickMeshSubset( 0xFFFFFFFF, pCamera, ViewportSize, ObjectTransform, vOrigin, vDir, bWireframe, fWireTolerance, fLocalDistance, nFace, hMaterial ) == false )
        return false;

    // Intersected!
    fDistance = fLocalDistance;
    return true;
}

//-----------------------------------------------------------------------------
// Name : pickFace ( ) (Virtual)
/// <summary>
/// Similar to pick() but is used to simply determine which face was 
/// intersected and which material is assigned to it.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::pickFace( const cgVector3 & vOrigin, const cgVector3 & vDir, cgVector3 & vIntersection, cgUInt32 & nFace, cgMaterialHandle & hMaterial )
{
    // Mesh must be prepared for picking to be supported.
    if ( mPrepareStatus != cgMeshStatus::Prepared )
        return false;

	// First, test for broadphase intersection against our bounding box
    cgFloat fLocalDistance = FLT_MAX;
    if ( mBoundingBox.isPopulated() == true )
    {
        if ( mBoundingBox.intersect( vOrigin, vDir, fLocalDistance, false ) == false )
            return false;
        
    } // End if bounds generated

    // Now test the physical mesh data.
    if ( pickMeshSubset( 0xFFFFFFFF, CG_NULL, cgSize(), cgTransform::Identity, vOrigin, vDir, false, 0, fLocalDistance, nFace, hMaterial ) == false )
        return false;

    // Compute the final intersection point and return.
    vIntersection = vOrigin + (vDir * fLocalDistance);
    return true;
}

//-----------------------------------------------------------------------------
// Name : pickMeshSubset ( ) (Protected)
/// <summary>
/// Internal mesh utility class designed for testing the mesh data against
/// a ray for picking etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::pickMeshSubset( cgUInt32 nDataGroupId, cgCameraNode * pCamera, const cgSize & ViewportSize, const cgTransform & ObjectTransform, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance, cgUInt32 & nFace, cgMaterialHandle & hMaterial )
{
    cgFloat          t, fClosestDistance = FLT_MAX;
    cgUInt32         nClosestFace = 0;
    cgMaterialHandle hClosestMaterial;
    bool             bHit = false;

    // Mesh must be prepared for picking to be supported.
    if ( mPrepareStatus != cgMeshStatus::Prepared )
        return false;

    // Retrieve the vertex position offset based on the selected
    // vertex format. If no 3D position is available, we can't pick.
    cgInt32 nVertexStride   = (cgInt32)mVertexFormat->getStride();
    cgInt32 nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    if ( nPositionOffset < 0 )
        return false;

    // Wireframe or solid picking?
    if ( bWireframe == false )
    {
        // Solid picking. Test for intersection against our internal mesh representation
        if ( nDataGroupId == 0xFFFFFFFF )
        {
            // Test every triangle for intersection.
            cgUInt32 nFaceCount = getFaceCount();
            for ( cgUInt32 nTestFace = 0; nTestFace < nFaceCount; ++nTestFace )
            {
                // Get vertex position data.
                const cgVector3 & v1 = *(cgVector3*)(mSystemVB + (mSystemIB[ nTestFace * 3 ] * nVertexStride) + nPositionOffset);
                const cgVector3 & v2 = *(cgVector3*)(mSystemVB + (mSystemIB[ (nTestFace * 3) + 1 ] * nVertexStride) + nPositionOffset);
                const cgVector3 & v3 = *(cgVector3*)(mSystemVB + (mSystemIB[ (nTestFace * 3) + 2 ] * nVertexStride) + nPositionOffset);

                // Ray intersects this triangle? (Skip if the triangle faces away from the ray direction.)
                if ( cgCollision::rayIntersectTriangle( vOrigin, vDir, v1, v2, v3, t, CGE_EPSILON_1MM, false, false ) == true )
                {
                    // Closest hit so far?
                    if ( t < fClosestDistance )
                    {
                        fClosestDistance = t;
                        nClosestFace     = nTestFace;
                        bHit             = true;
                    
                    } // End if closest

                } // End if intersected

            } // Next Face

            // Find the material assigned to the closest intersected face.
            if ( bHit == true )
                hClosestMaterial = mTriangleData[nClosestFace].material;
        
        } // End if no subset
        else
        {
            // Get the list of all subsets associated with this data group
            for ( size_t i = 0; i < mMeshSubsets.size(); ++i )
            {
                MeshSubset * pSubset = mMeshSubsets[i];
                if ( pSubset->dataGroupId != nDataGroupId )
                    continue;

                // Test subset triangles for intersection.
                cgUInt32 nFaceStart = pSubset->faceStart;
                cgUInt32 nFaceEnd   = (pSubset->faceStart + pSubset->faceCount) - 1;
                for ( cgUInt32 nTestFace = pSubset->faceStart; nTestFace <= nFaceEnd; ++nTestFace )
                {
                    // Get vertex position data.
                    const cgVector3 & v1 = *(cgVector3*)(mSystemVB + (mSystemIB[ nTestFace * 3 ] * nVertexStride) + nPositionOffset);
                    const cgVector3 & v2 = *(cgVector3*)(mSystemVB + (mSystemIB[ (nTestFace * 3) + 1 ] * nVertexStride) + nPositionOffset);
                    const cgVector3 & v3 = *(cgVector3*)(mSystemVB + (mSystemIB[ (nTestFace * 3) + 2 ] * nVertexStride) + nPositionOffset);

                    // Ray intersects this triangle? (Skip if the triangle faces away from the ray direction.)
                    if ( cgCollision::rayIntersectTriangle( vOrigin, vDir, v1, v2, v3, t, CGE_EPSILON_1MM, false, false ) == true )
                    {
                        // Closest hit so far?
                        if ( t < fClosestDistance )
                        {
                            fClosestDistance = t;
                            nClosestFace     = nTestFace;
                            hClosestMaterial = pSubset->material;
                            bHit             = true;
                        
                        } // End if closest

                    } // End if intersected

                } // Next Face

            } // Next Subset

        } // End if subset test
            
        // Any front-face intersection found?
        if ( bHit == false )
            return false;
            
        // Return relevant information.
        fDistance = fClosestDistance;
        nFace     = nClosestFace;
        hMaterial = hClosestMaterial;
	    return true;

    } // End if solid picking
    else
    {
        // If there is no camera, we can't test.
        if ( !pCamera )
            return false;

        // In wireframe mode, we must test the geometry edges.
        cgPlane TrianglePlane;
        cgVector3 vWireTolerance;
        cgUInt32 nFaceCount = getFaceCount();
        for ( cgUInt32 nTestFace = 0; nTestFace < nFaceCount; ++nTestFace )
        {
            // A subset we're interested in?
            if ( nDataGroupId != 0xFFFFFFFF )
            {
                if ( mTriangleData[nTestFace].dataGroupId != nDataGroupId )
                    continue;

            } // End if filtering

            // Retrieve the three vertices that form this triangle
            const cgVector3 & v1 = *(cgVector3*)(mSystemVB + (mSystemIB[ nTestFace * 3 ] * nVertexStride) + nPositionOffset);
            const cgVector3 & v2 = *(cgVector3*)(mSystemVB + (mSystemIB[ (nTestFace * 3) + 1 ] * nVertexStride) + nPositionOffset);
            const cgVector3 & v3 = *(cgVector3*)(mSystemVB + (mSystemIB[ (nTestFace * 3) + 2 ] * nVertexStride) + nPositionOffset);

            // Compute a plane for this triangle
            cgPlane::fromPoints( TrianglePlane, v1, v2, v3 );

            // Determine if the ray intersects the plane formed by this triangle
            if ( cgCollision::rayIntersectPlane( vOrigin, vDir, TrianglePlane, t, true, false ) )
            {
                // Compute the plane intersection point
                const cgVector3 vIntersect = vOrigin + (vDir * t);

                // Test each edge in the triangle in order to determine if the ray
                // passes close to the edge (could optimize this by skipping any
                // edges which have already been tested in adjacent triangles).
                for ( size_t nEdge = 0; nEdge < 3; ++nEdge )
                {
                    // Retrieve the two vertices that form this edge
                    const cgVector3 & e1 = (nEdge == 0) ? v1 : (nEdge==1) ? v2 : v3;
                    const cgVector3 & e2 = (nEdge == 0) ? v2 : (nEdge==1) ? v3 : v1;

                    // Test a ray, formed by the edge itself, to see if it intersects
                    // a plane passing through the cursor position. This intersection
                    // point can then be tested to see if it came close to the original
                    // /plane/ intersection point.
                    cgVector3 vNormal;
                    cgVector3::normalize( vNormal, (e1-e2) );
                    if ( cgCollision::rayIntersectPlane( e1, e2-e1, vNormal, vIntersect, t ) )
                    {
                        // Compute intersection point
                        const cgVector3 vEdgeIntersect = e1 + ((e2-e1) * t);

                        // Compute the correct wireframe tolerance to use at the edge.
                        vWireTolerance = pCamera->estimatePickTolerance( ViewportSize, fWireTolerance, vEdgeIntersect, ObjectTransform );

                        // Test to see if the cursor hit location (on the plane) is close to the edge.
                        if ( fabsf( vIntersect.x - vEdgeIntersect.x ) <= vWireTolerance.x &&
                             fabsf( vIntersect.y - vEdgeIntersect.y ) <= vWireTolerance.y &&
                             fabsf( vIntersect.z - vEdgeIntersect.z ) <= vWireTolerance.z )
                        {
                            t = cgVector3::length( vEdgeIntersect - vOrigin );

                            // Is this the closest intersection so far?
                            if ( t < fClosestDistance )
                            {
                                fClosestDistance = t;
                                nClosestFace     = nTestFace;
                                bHit             = true;

                            } // End if closest so far

                        } // End if vectors match

                    } // End if edge hit cursor plane

                } // Next Edge

            } // End if intersects triangle plane

        } // Next Face

        // Find the material assigned to the closest intersected face
        // or bail if no hit was detected.
        if ( bHit == true )
            hClosestMaterial = mTriangleData[nClosestFace].material;
        else
            return false;

        // Return relevant information.
        fDistance = fClosestDistance;
        nFace     = nClosestFace;
        hMaterial = hClosestMaterial;
        return true;

    } // End if wireframe picking

    // No intersection
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getMaterials ()
/// <summary>
/// Retrieve a list of all of the materials used by this object for use
/// during rendering.
/// </summary>
//-----------------------------------------------------------------------------
const cgMaterialHandleArray & cgMesh::getMaterials( ) const
{
    return mObjectMaterials;
}

//-----------------------------------------------------------------------------
//  Name : getFaceCount ()
/// <summary>
/// Determine the number of faces stored here. If the mesh has already
/// been finalized with a call to endPrepare(), this will be the total
/// number of triangles stored. Otherwise, this will be the number of
/// windings maintained within the mesh ready for preparation.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgMesh::getFaceCount( ) const
{
    if ( mPrepareStatus == cgMeshStatus::Prepared )
        return mFaceCount;
    else if ( mPrepareStatus == cgMeshStatus::Preparing )
        return (cgUInt32)mPrepareData.triangleData.size();
    else
        return 0;
}

//-----------------------------------------------------------------------------
//  Name : getVertexCount ()
/// <summary>
/// Determine the number of vertices stored here. If the mesh has already
/// been finalized with a call to endPrepare(), this will be the total
/// number of vertices stored. Otherwise, this will be the number of
/// vertices currently maintained within the mesh ready for preparation.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgMesh::getVertexCount( ) const
{
    if ( mPrepareStatus == cgMeshStatus::Prepared )
        return mVertexCount;
    else if ( mPrepareStatus == cgMeshStatus::Preparing )
        return (cgUInt32)mPrepareData.vertexCount;
    else
        return 0;
}

//-----------------------------------------------------------------------------
//  Name : getSystemVB ()
/// <summary>
/// Retrieve the underlying vertex data from the mesh.
/// </summary>
//-----------------------------------------------------------------------------
cgByte * cgMesh::getSystemVB( )
{
    return mSystemVB;
}

//-----------------------------------------------------------------------------
//  Name : getSystemIB ()
/// <summary>
/// Retrieve the underlying index data from the mesh.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 * cgMesh::getSystemIB( )
{
    return mSystemIB;
}

//-----------------------------------------------------------------------------
//  Name : getVertexFormat ()
/// <summary>
/// Retrieve the format of the underlying mesh vertex data.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat * cgMesh::getVertexFormat( )
{
    return mVertexFormat;
}

//-----------------------------------------------------------------------------
//  Name : getSkinBindData ()
/// <summary>
/// If this mesh has been bound as a skin, this method can be called to
/// retrieve the original data that was used to bind it.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinBindData * cgMesh::getSkinBindData( )
{
    return mSkinBindData;
}

//-----------------------------------------------------------------------------
//  Name : getBonePalettes ()
/// <summary>
/// If this mesh has been bound as a skin, this method can be called to
/// retrieve the compiled bone combination palette data.
/// </summary>
//-----------------------------------------------------------------------------
const cgMesh::BonePaletteArray & cgMesh::getBonePalettes( ) const
{
    return mBonePalettes;
}

//-----------------------------------------------------------------------------
//  Name : getSubset ()
/// <summary>
/// Retrieve information about the subset of the mesh that is associated with
/// the specified material and data group identifier.
/// </summary>
//-----------------------------------------------------------------------------
const cgMesh::MeshSubset * cgMesh::getSubset( const cgMaterialHandle & material, cgUInt32 dataGroupId /* = 0 */ ) const
{
    SubsetKeyMap::const_iterator itSubset = mSubsetLookup.find( MeshSubsetKey(material, dataGroupId) );
    if ( itSubset == mSubsetLookup.end() )
        return CG_NULL;
    return itSubset->second;
}

//-----------------------------------------------------------------------------
//  Name : setDefaultColor ()
/// <summary>
/// Set the 'default' color to use whenever the default material is invoked
/// during rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgMesh::setDefaultColor( cgUInt32 nColor )
{
    mDefaultColor = nColor;
}

//-----------------------------------------------------------------------------
// Name : setMeshMaterial()
/// <summary>
/// Apply the specified material to every face in the mesh, replacing that
/// previously assigned.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::setMeshMaterial( const cgMaterialHandle & hMaterial )
{
    // Mesh requires standard material types.
    const cgMaterial * pMaterial = hMaterial.getResourceSilent();
    if ( pMaterial != CG_NULL && pMaterial->queryReferenceType( RTID_StandardMaterial ) == false )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to assign material to mesh because the specified material was not of a valid type.\n") );
        return false;
    
    } // End if invalid type

    // Replace all triangle materials in the mesh with that specified.
    if ( mPrepareStatus == cgMeshStatus::Preparing )
    {
        for ( cgUInt32 i = 0; i < mPrepareData.triangleCount; ++i )
            mPrepareData.triangleData[i].material = hMaterial;

        // Success!
        return true;

    } // End if Preparing
    else if ( mPrepareStatus == cgMeshStatus::Prepared )
    {
        for ( cgUInt32 i = 0; i < mFaceCount; ++i )
            mTriangleData[i].material = hMaterial;

        // Re-sort the mesh data if required.
        if ( mDisableFinalSort == true )
            return true;
        else
            return sortMeshData( mOptimizedMesh, mHardwareMesh );

    } // End if Prepared

    // Nothing to do.
    return false;
}

//-----------------------------------------------------------------------------
// Name : setFaceMaterial()
/// <summary>
/// Apply the specified material to the relevant face, replacing that
/// previously assigned.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::setFaceMaterial( cgUInt32 nFace, const cgMaterialHandle & hMaterial )
{
    // Mesh requires standard material types.
    const cgMaterial * pMaterial = hMaterial.getResourceSilent();
    if ( pMaterial != CG_NULL && pMaterial->queryReferenceType( RTID_StandardMaterial ) == false )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to assign material to mesh face because the specified material was not of a valid type.\n") );
        return false;
    
    } // End if invalid type

    // Different approaches are required depending on whether or not
    // the mesh is currently in the process of being prepared or not.
    if ( mPrepareStatus == cgMeshStatus::Preparing )
    {
        // Ensure specified face is valid
        if ( nFace >= mPrepareData.triangleCount )
            return false;

        // No-op?
        if ( mPrepareData.triangleData[nFace].material == hMaterial )
            return false;

        // Replace the material reference for the specified face.
        mPrepareData.triangleData[nFace].material = hMaterial;

        // Success!
        return true;

    } // End if Preparing
    else if ( mPrepareStatus == cgMeshStatus::Prepared )
    {
        // Ensure specified face is valid
        if ( nFace >= mFaceCount )
            return false;

        // No-op?
        if ( mTriangleData[nFace].material == hMaterial )
            return false;

        // Replace the material reference for the specified face.
        mTriangleData[nFace].material = hMaterial;

        // Re-sort the mesh data if required.
        if ( mDisableFinalSort == true )
            return true;
        else
            return sortMeshData( mOptimizedMesh, mHardwareMesh );
        
    } // End if Prepared

    // Nothing to do.
    return false;
}

//-----------------------------------------------------------------------------
// Name : replaceMaterial()
/// <summary>
/// Replace the specified material, with a new one.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::replaceMaterial( const cgMaterialHandle & oldMaterial, const cgMaterialHandle & newMaterial )
{
    // Mesh requires standard material types.
    const cgMaterial * newMaterialRes = newMaterial.getResourceSilent();
    if ( newMaterialRes != CG_NULL && newMaterialRes->queryReferenceType( RTID_StandardMaterial ) == false )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to assign new material to mesh because the specified material was not of a valid type.\n") );
        return false;
    
    } // End if invalid type

    // Different approaches are required depending on whether or not
    // the mesh is currently in the process of being prepared or not.
    if ( mPrepareStatus == cgMeshStatus::Preparing )
    {
        // Process all unprepared triangles.
        for ( cgUInt32 i = 0; i < mPrepareData.triangleCount; ++i )
        {
            // Replace the material reference for the specified face.
            if ( mPrepareData.triangleData[i].material == oldMaterial )
                mPrepareData.triangleData[i].material = newMaterial;
        
        } // Next triangle

        // Success!
        return true;

    } // End if Preparing
    else if ( mPrepareStatus == cgMeshStatus::Prepared )
    {
        // Swap materials in triangle data.
        for ( cgUInt32 i = 0; i < mFaceCount; ++i )
        {
            // Replace the material reference for the specified face.
            if ( mTriangleData[i].material == oldMaterial )
                mTriangleData[i].material = newMaterial;
        
        } // Next triangle

        // Re-sort the mesh data if required.
        if ( mDisableFinalSort == true )
            return true;
        else
            return sortMeshData( mOptimizedMesh, mHardwareMesh );
        
    } // End if Prepared

    // Nothing to do.
    return false;
}

//-----------------------------------------------------------------------------
// Name : scaleMeshData()
/// <summary>
/// Apply the specified scale to all local mesh data (baked scale).
/// </summary>
//-----------------------------------------------------------------------------
bool cgMesh::scaleMeshData( cgFloat fScale )
{
    // Determine the location of the 'position' component of the vertex
    // in the system vertex buffer.
    cgInt nPositionOffset = mVertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgInt nVertexStride   = mVertexFormat->getStride();

    // Rescale the position of every vertex in the system memory vertex buffer.
    if ( mSystemVB )
    {
        cgByte * pBuffer = mSystemVB + nPositionOffset;
        for ( cgUInt32 i = 0; i < mVertexCount; ++i, pBuffer += nVertexStride )
            *((cgVector3*)pBuffer) *= fScale;

    } // End if has buffer

    // Scale the object space axis aligned bounding box accordingly.
    mBoundingBox *= fScale;

    // Apply scale to skin binding pose matrices (translation)
    if ( mSkinBindData )
    {
        cgSkinBindData::BoneArray & aBones = mSkinBindData->getBones();
        for ( size_t i = 0; i < aBones.size(); ++i )
        {
            cgSkinBindData::BoneInfluence * pBone = aBones[i];
            pBone->bindPoseTransform.position() *= fScale;
        
        } // Next Bone

    } // End if has skin data

    // Repopulate any hardware vertex buffer.
    if ( mHardwareVB.isValid() )
    {
        mHardwareVB.setResourceLost( true );
        restoreBuffers();
    
    } // End if has VB

    // Vertex data and skin binding data has been updated and 
    // potentially needs to be serialized.
    mDBVertexStreamDirty = true;
    mDBSkinDataDirty     = true;
    return serializeMesh( );
}

///////////////////////////////////////////////////////////////////////////////
// cgSkinBindData Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSkinBindData() (Constructor)
/// <summary>
/// Class constructor.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinBindData::cgSkinBindData( )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgSkinBindData() (Copy Constructor)
/// <summary>
/// Class Copy constructor.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinBindData::cgSkinBindData( const cgSkinBindData & Init )
{
    // Duplicate bone data
    mBones.resize( Init.mBones.size() );
    for ( size_t i = 0; i < mBones.size(); ++i )
    {
        const BoneInfluence * pSrcBone = Init.mBones[i];

        // Allocate new bone influence data and copy simple properties.
        BoneInfluence * pBone    = new BoneInfluence();
        pBone->boneIdentifier    = pSrcBone->boneIdentifier;
        pBone->bindPoseTransform = pSrcBone->bindPoseTransform;
        pBone->influences        = pSrcBone->influences;

        // Store
        mBones[i] = pBone;

    } // Next Bone
}

//-----------------------------------------------------------------------------
//  Name : ~cgSkinBindData() (Destructor)
/// <summary>
/// Clean up any resources being used.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinBindData::~cgSkinBindData()
{
    // Clean up any allocated resources
    for ( size_t i = 0; i < mBones.size(); ++i )
        delete mBones[i];
    mBones.clear();
}

//-----------------------------------------------------------------------------
//  Name : addBone()
/// <summary>
/// Add influence information for a specific bone.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinBindData::addBone( BoneInfluence * pBone )
{
    mBones.push_back( pBone );
}

//-----------------------------------------------------------------------------
//  Name : removeEmptyBones()
/// <summary>
/// Strip out any bones that did not contain any influences.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinBindData::removeEmptyBones( )
{
    for ( size_t i = 0; i < mBones.size(); )
    {
        if ( !mBones[i] || mBones[i]->influences.empty() )
        {
            delete mBones[i];
            mBones.erase( mBones.begin() + i );
        
        } // End if empty
        else
            ++i;

    } // Next Bone
}

//-----------------------------------------------------------------------------
//  Name : clearVertexInfluences()
/// <summary>
/// Release memory allocated for vertex influences in each stored bone.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinBindData::clearVertexInfluences( )
{
    for ( size_t i = 0; i < mBones.size(); ++i )
        mBones[i]->influences.clear();
}

//-----------------------------------------------------------------------------
//  Name : remapVertices()
/// <summary>
/// Remap the vertex references stored in the binding based on the supplied
/// remap array (Array[old vertex index] = new vertex index). Store an index
/// of 0xFFFFFFFF to indicate that the vertex was removed, or use an index
/// greater than the remap array size to indicate the new location of a split
/// vertex.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinBindData::remapVertices( const cgUInt32Array & Remap )
{
    // Iterate through all bone information and remap vertex indices.
    for ( size_t i = 0; i < mBones.size(); ++i )
    {
        VertexInfluenceArray NewInfluences;
        VertexInfluenceArray &Influences = mBones[i]->influences;
        NewInfluences.reserve( Influences.size() );
        for ( size_t j = 0; j < Influences.size(); ++j )
        {
            cgUInt32 nNewIndex = Remap[Influences[j].vertexIndex];
            if ( nNewIndex != 0xFFFFFFFF )
            {
                // Insert an influence at the new index
                NewInfluences.push_back( VertexInfluence( nNewIndex, Influences[j].weight ) );

                // If the vertex was split into two, we want to retain an
                // influence to the original index too.
                if ( nNewIndex >= Remap.size() )
                    NewInfluences.push_back( VertexInfluence( Influences[j].vertexIndex, Influences[j].weight ) );

            } // End if !removed

        } // Next source influence
        mBones[i]->influences = NewInfluences;

    } // Next bone
}

//-----------------------------------------------------------------------------
//  Name : buildVertexTable()
/// <summary>
/// Construct a list of bone influences and weights for each vertex based 
/// on the binding data provided.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkinBindData::buildVertexTable( cgUInt32 nVertexCount, const cgUInt32Array & VertexRemap, BindVertexArray & Table )
{
    cgUInt32 nVertex;

    // Initialize the vertex table with the required number of vertices.
    Table.reserve( nVertexCount );
    for ( nVertex = 0; nVertex < nVertexCount; ++nVertex )
    {
        VertexData * pData      = new VertexData();
        pData->palette         = -1;
        pData->originalVertex  = nVertex;
        Table.push_back( pData );

    } // Next Vertex

    // Iterate through all bone information and populate the above array.
    for ( size_t i = 0; i < mBones.size(); ++i )
    {
        VertexInfluenceArray &Influences = mBones[i]->influences;
        for ( size_t j = 0; j < Influences.size(); ++j )
        {
            VertexData * pData = CG_NULL;
                        
            // Vertex data has been remapped?
            if ( VertexRemap.size() > 0 )
            {
                nVertex = VertexRemap[Influences[j].vertexIndex];
                if ( nVertex == 0xFFFFFFFF ) continue;
                pData = Table[nVertex];
            
            } // End if remap
            else
                pData = Table[Influences[j].vertexIndex];

            // Push influence data.
            pData->influences.push_back( (cgInt32)i );
            pData->weights.push_back( Influences[j].weight );

        } // Next Influence

    } // Next Bone
}

//-----------------------------------------------------------------------------
//  Name : getBones ()
/// <summary>
/// Retrieve a list of all bones that influence the skin in some way.
/// </summary>
//-----------------------------------------------------------------------------
const cgSkinBindData::BoneArray & cgSkinBindData::getBones( ) const
{
    return mBones;
}

//-----------------------------------------------------------------------------
//  Name : getBones ()
/// <summary>
/// Retrieve a list of all bones that influence the skin in some way.
/// </summary>
//-----------------------------------------------------------------------------
cgSkinBindData::BoneArray & cgSkinBindData::getBones( )
{
    return mBones;
}

///////////////////////////////////////////////////////////////////////////////
// cgBonePalette Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBonePalette() (Constructor)
/// <summary>
/// Class constructor.
/// </summary>
//-----------------------------------------------------------------------------
cgBonePalette::cgBonePalette( cgUInt32 nPaletteSize )
{
    // Initialize variables to sensible defaults
    mDataGroupId   = 0;
    mMaximumSize   = nPaletteSize;
    mMaximumBlendIndex = -1;
}

//-----------------------------------------------------------------------------
//  Name : cgBonePalette() (Copy Constructor)
/// <summary>
/// Class copy constructor.
/// </summary>
//-----------------------------------------------------------------------------
cgBonePalette::cgBonePalette( const cgBonePalette & Init )
{
    mBoneLUT           = Init.mBoneLUT;
    mBones             = Init.mBones;
    mFaces             = Init.mFaces;
    mMaterial          = Init.mMaterial;
    mDataGroupId       = Init.mDataGroupId;
    mMaximumSize       = Init.mMaximumSize;
    mMaximumBlendIndex = Init.mMaximumBlendIndex;
}

//-----------------------------------------------------------------------------
//  Name : ~cgBonePalette() (Destructor)
/// <summary>
/// Clean up any resources being used.
/// </summary>
//-----------------------------------------------------------------------------
cgBonePalette::~cgBonePalette()
{
}

//-----------------------------------------------------------------------------
//  Name : apply()
/// <summary>
/// Apply the bone / palette information to the render driver ready for
/// drawing the skinned mesh.
/// </summary>
//-----------------------------------------------------------------------------
void cgBonePalette::apply( cgObjectNodeArray & BoneObjects, cgSkinBindData * pBindData, bool bComputeInverseTranspose, cgRenderDriver * pDriver )
{
    // Retrieve the main list of bones from the skin bind data that will
    // be referenced by the palette's bone index list.
    const cgSkinBindData::BoneArray & BindList = pBindData->getBones();

    // Compute transformation matrix for each bone in the palette
    if ( bComputeInverseTranspose )
    {
        cgToDo( "Carbon General", "Take size from render driver's 'getMaxBlendTransforms()'?" )
        cgMatrix Transforms[50];
        cgMatrix ITTransforms[50];

        for ( size_t i = 0; i < mBones.size(); ++i )
        {
            cgObjectNode * pBone = BoneObjects[mBones[i]];
            const cgSkinBindData::BoneInfluence* pBoneData = BindList[mBones[i]];

            if ( !pBone )
                Transforms[i] = pBoneData->bindPoseTransform;
            else
                Transforms[i] = pBoneData->bindPoseTransform * pBone->getWorldTransform();

            // Compute inverse transpose
            ITTransforms[i] = Transforms[i];
            (cgVector4&)ITTransforms[i]._41 = cgVector4( 0, 0, 0, 1 );
            cgMatrix::inverse( ITTransforms[i], ITTransforms[i] );
            cgMatrix::transpose( ITTransforms[i], ITTransforms[i] );
            
        } // Next Bone

        // Set to the device
        pDriver->setVertexBlendData( Transforms, ITTransforms, (cgUInt32)mBones.size(), mMaximumBlendIndex );

    } // End if bComputeInverseTranspose
    else
    {
        cgToDo( "Carbon General", "Take size from render driver's 'getMaxBlendTransforms()'?" )
        cgMatrix Transforms[50];

        for ( size_t i = 0; i < mBones.size(); ++i )
        {
            cgObjectNode * pBone = BoneObjects[mBones[i]];
            const cgSkinBindData::BoneInfluence* pBoneData = BindList[mBones[i]];

            if ( !pBone )
                Transforms[i] = pBoneData->bindPoseTransform;
            else
                Transforms[i] = pBoneData->bindPoseTransform * pBone->getWorldTransform();
            
        } // Next Bone

        // Set to the device
        pDriver->setVertexBlendData( Transforms, CG_NULL, (cgUInt32)mBones.size(), mMaximumBlendIndex );

    } // End if !bComputeInverseTranspose

}

//-----------------------------------------------------------------------------
//  Name : assignBones()
/// <summary>
/// Assign the specified bones (and faces) to this bone palette.
/// </summary>
//-----------------------------------------------------------------------------
void cgBonePalette::assignBones( BoneIndexMap & Bones, cgUInt32Array & Faces )
{
    BoneIndexMap::iterator itBone, itBone2;

    // Iterate through newly specified input bones and add any unique ones to the palette.
    for ( itBone = Bones.begin(); itBone != Bones.end(); ++itBone )
    {
        itBone2 = mBoneLUT.find( itBone->first );
        if ( itBone2 == mBoneLUT.end() )
        {
            mBoneLUT[ itBone->first ] = (cgUInt32)mBones.size();
            mBones.push_back( itBone->first );
        
        } // End if not already added

    } // Next Bone

    // Merge the new face list with ours.
    mFaces.insert( mFaces.end(), Faces.begin(), Faces.end() );
}

//-----------------------------------------------------------------------------
//  Name : assignBones()
/// <summary>
/// Assign the specified bones to this bone palette.
/// </summary>
//-----------------------------------------------------------------------------
void cgBonePalette::assignBones( const cgUInt32Array & Bones )
{
    BoneIndexMap::iterator itBone;

    // Clear out prior data.
    mBones.clear();
    mBoneLUT.clear();
    
    // Iterate through newly specified input bones and add any unique ones to the palette.
    for ( size_t i = 0; i < Bones.size(); ++i )
    {
        itBone = mBoneLUT.find( Bones[i] );
        if ( itBone == mBoneLUT.end() )
        {
            mBoneLUT[ Bones[i] ] = (cgUInt32)mBones.size();
            mBones.push_back( Bones[i] );
        
        } // End if not already added

    } // Next Bone
}

//-----------------------------------------------------------------------------
//  Name : computePaletteFit()
/// <summary>
/// Determine the relevant "fit" information that can be used to 
/// discover if and how the specified combination of bones will fit into 
/// this palette.
/// </summary>
//-----------------------------------------------------------------------------
void cgBonePalette::computePaletteFit( BoneIndexMap & Input, cgInt32 & nCurrentSpace, cgInt32 & nCommonBones, cgInt32 & nAdditionalBones )
{
    // Reset values
    nCurrentSpace    = mMaximumSize - (cgInt32)mBones.size();
    nCommonBones     = 0;
    nAdditionalBones = 0;

    // Early out if possible
    if ( mBones.size() == 0 )
    {
        nAdditionalBones = (int)Input.size();
        return;
    
    } // End if no bones stored
    else if ( Input.size() == 0 )
    {
        return;
    
    } // End if no bones input

    // Iterate through newly specified input bones and see how many
    // indices it has in common with our existing set.
    BoneIndexMap::iterator itBone, itBone2;
    for ( itBone = Input.begin(); itBone != Input.end(); ++itBone )
    {
        itBone2 = mBoneLUT.find( itBone->first );
        if ( itBone2 != mBoneLUT.end() )
            nCommonBones++;
        else
            nAdditionalBones++;

    } // Next Bone
}

//-----------------------------------------------------------------------------
//  Name : translateBoneToPalette ()
/// <summary>
/// Translate the specified bone index into its associated position in 
/// the palette. If it does not exist, a value of -1 will be returned.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBonePalette::translateBoneToPalette( cgUInt32 nBoneIndex )
{
    BoneIndexMap::iterator itBone = mBoneLUT.find( nBoneIndex );
    if ( itBone == mBoneLUT.end() )
        return 0xFFFFFFFF;
    return itBone->second;
}

//-----------------------------------------------------------------------------
//  Name : getMaterial ()
/// <summary>
/// Retrieve the material assigned to the subset of the mesh reserved
/// for this bone palette.
/// </summary>
//-----------------------------------------------------------------------------
cgMaterialHandle & cgBonePalette::getMaterial()
{
    return mMaterial;
}

//-----------------------------------------------------------------------------
//  Name : setMaterial ()
/// <summary>
/// Set the material assigned to the subset of the mesh reserved for 
/// this bone palette.
/// </summary>
//-----------------------------------------------------------------------------
void cgBonePalette::setMaterial( const cgMaterialHandle & hMaterial )
{
    mMaterial = hMaterial;
}

//-----------------------------------------------------------------------------
//  Name : getDataGroup ()
/// <summary>
/// Retrieve the identifier of the data group assigned to the subset of the
/// mesh reserved for this bone palette.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBonePalette::getDataGroup() const
{
    return mDataGroupId;
}

//-----------------------------------------------------------------------------
//  Name : setDataGroup ()
/// <summary>
/// Set the identifier of the data group assigned to the subset of the mesh 
/// reserved for this bone palette.
/// </summary>
//-----------------------------------------------------------------------------
void cgBonePalette::setDataGroup( cgUInt32 nGroup )
{
    mDataGroupId = nGroup;
}

//-----------------------------------------------------------------------------
//  Name : getMaximumBlendIndex ()
/// <summary>
/// Retrieve the maximum vertex blend index for this palette (i.e. if 
/// every vertex was only influenced by one bone, this variable would 
/// contain a value of 0).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgBonePalette::getMaximumBlendIndex() const
{
    return mMaximumBlendIndex;
}

//-----------------------------------------------------------------------------
//  Name : setMaximumBlendIndex ()
/// <summary>
/// Set the maximum vertex blend index for this palette (i.e. if every 
/// vertex was only influenced by one bone, this variable would contain a
/// value of 0).
/// </summary>
//-----------------------------------------------------------------------------
void cgBonePalette::setMaximumBlendIndex( int nIndex )
{
    mMaximumBlendIndex = nIndex;
}

//-----------------------------------------------------------------------------
//  Name : getMaximumSize ()
/// <summary>
/// Retrieve the maximum size of the palette -- the maximum number of bones
/// capable of being referenced.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBonePalette::getMaximumSize() const
{
    return mMaximumSize;
}

//-----------------------------------------------------------------------------
//  Name : getInfluencedFaces ()
/// <summary>
/// Retrieve the list of faces assigned to this palette.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32Array & cgBonePalette::getInfluencedFaces()
{
    return mFaces;
}

//-----------------------------------------------------------------------------
//  Name : clearInfluencedFaces ()
/// <summary>
/// Clear out the temporary face influences array.
/// </summary>
//-----------------------------------------------------------------------------
void cgBonePalette::clearInfluencedFaces()
{
    mFaces.clear();
}

//-----------------------------------------------------------------------------
//  Name : getBones ()
/// <summary>
/// Retrieve the indices of the bones referenced by this palette.
/// </summary>
//-----------------------------------------------------------------------------
const cgUInt32Array & cgBonePalette::getBones( ) const
{
    return mBones;
}

///////////////////////////////////////////////////////////////////////////////
// Global Operator Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : operator < () (AdjacentEdgeKey&, AdjacentEdgeKey&)
/// <summary>
/// Perform less than comparison on the AdjacentEdgeKey structure.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgMesh::AdjacentEdgeKey& Key1, const cgMesh::AdjacentEdgeKey& Key2)
{
    cgFloat fDifference = 0;

    // Test vertex positions.
    static const cgInt32 maxUlps = 10;
    if ( !cgMathUtility::dynamicEpsilonTest( Key1.vertex1->x, Key2.vertex1->x, maxUlps ) )
        return ( Key2.vertex1->x < Key1.vertex1->x );
    if ( !cgMathUtility::dynamicEpsilonTest( Key1.vertex1->y, Key2.vertex1->y, maxUlps ) )
        return ( Key2.vertex1->y < Key1.vertex1->y );
    if ( !cgMathUtility::dynamicEpsilonTest( Key1.vertex1->z, Key2.vertex1->z, maxUlps ) )
        return ( Key2.vertex1->z < Key1.vertex1->z );

    if ( !cgMathUtility::dynamicEpsilonTest( Key1.vertex2->x, Key2.vertex2->x, maxUlps ) )
        return ( Key2.vertex2->x < Key1.vertex2->x );
    if ( !cgMathUtility::dynamicEpsilonTest( Key1.vertex2->y, Key2.vertex2->y, maxUlps ) )
        return ( Key2.vertex2->y < Key1.vertex2->y );
    if ( !cgMathUtility::dynamicEpsilonTest( Key1.vertex2->z, Key2.vertex2->z, maxUlps ) )
        return ( Key2.vertex2->z < Key1.vertex2->z );
    
    // Exactly equal
    return false;
}

//-----------------------------------------------------------------------------
//  Name : operator < () (MeshSubsetKey&, MeshSubsetKey&)
/// <summary>
/// Perform less than comparison on the MeshSubsetKey structure.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgMesh::MeshSubsetKey& Key1, const cgMesh::MeshSubsetKey& Key2)
{
    cgInt nDifference = 0;

    nDifference = (Key1.material == Key2.material) ? 0 : (Key1.material < Key2.material) ? -1 : 1;
    if ( nDifference != 0 ) return (nDifference < 0);
    nDifference = (cgInt)Key1.dataGroupId - (cgInt)Key2.dataGroupId;
    if ( nDifference != 0 ) return (nDifference < 0);

    // Exactly equal
    return false;
}

//-----------------------------------------------------------------------------
//  Name : operator < () (WeldKey&, WeldKey&)
/// <summary>
/// Perform less than comparison on the WeldKey structure.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgMesh::WeldKey& Key1, const cgMesh::WeldKey& Key2)
{
    int nDifference = cgVertexFormat::compareVertices( Key1.vertex, Key2.vertex, Key1.format, Key1.tolerance );
    if ( nDifference != 0 ) return (nDifference < 0);

    // Exactly equal
    return false;
}

//-----------------------------------------------------------------------------
//  Name : operator < () (BoneCombinationKey&, BoneCombinationKey&)
/// <summary>
/// Perform less than comparison on the BoneCombinationKey structure.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgMesh::BoneCombinationKey& Key1, const cgMesh::BoneCombinationKey& Key2)
{
    cgInt nDifference;
    const cgMesh::FaceInfluences * p1 = Key1.influences;
    const cgMesh::FaceInfluences * p2 = Key2.influences;

    // The material must match
    nDifference = (p1->material == p2->material) ? 0 : ((p1->material < p2->material) ? -1 : 1);
    if ( nDifference != 0 ) return (nDifference < 0);
    
    // The bone count must match.
    nDifference = (cgInt)p1->bones.size() - (cgInt)p2->bones.size();
    if ( nDifference != 0 ) return (nDifference < 0);
    
    // Compare the bone indices in each list
    cgBonePalette::BoneIndexMap::const_iterator itBone1 = p1->bones.begin();
    cgBonePalette::BoneIndexMap::const_iterator itBone2 = p2->bones.begin();
    for ( ; itBone1 != p1->bones.end() && itBone2 != p2->bones.end(); ++itBone1, ++itBone2 )
    {
        nDifference = (cgInt)itBone1->first - (cgInt)itBone2->first;
        if ( nDifference != 0 ) return (nDifference < 0);

    } // Next Bone
    
    // Exact match (for the purposes of combining influences)
    return false;
}