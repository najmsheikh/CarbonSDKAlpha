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
// Name : cgClutterMaterial.cpp                                              //
//                                                                           //
// Desc : Provides classes which outline properties for scene clutter        //
//        (foliage, rocks, trees, etc.) that can be applied to landscapes    //
//        and other scene elements.                                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgClutterMaterial Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgClutterMaterial.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgVertexBuffer.h>
#include <Resources/cgIndexBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgSampler.h>
#include <Rendering/cgVertexFormats.h>
#include <World/Objects/cgCameraObject.h>
#include <System/cgExceptions.h>
#include <Math/cgRandom.h>

// Preview rendering
#include <System/cgImage.h>
#include <Resources/cgRenderTarget.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgClutterMaterial::mInsertMaterial;
cgWorldQuery cgClutterMaterial::mUpdateName;
cgWorldQuery cgClutterMaterial::mUpdatePreview;
cgWorldQuery cgClutterMaterial::mLoadMaterial;
cgWorldQuery cgClutterMaterial::mLoadFoliageLayers;
cgWorldQuery cgFoliageClutterLayer::mInsertLayer;
cgWorldQuery cgFoliageClutterLayer::mUpdateName;
cgWorldQuery cgFoliageClutterLayer::mUpdateSamplers;
cgWorldQuery cgFoliageClutterLayer::mUpdateRenderProperties;
cgWorldQuery cgFoliageClutterLayer::mUpdateElementProperties;
cgWorldQuery cgFoliageClutterLayer::mUpdateGrowthProperties;
cgWorldQuery cgFoliageClutterLayer::mUpdateMaterialProperties;
cgWorldQuery cgFoliageClutterLayer::mUpdateLODProperties;
cgWorldQuery cgFoliageClutterLayer::mLoadLayer;
cgWorldQuery cgFoliageClutterLayer::mDeleteLayer;

///////////////////////////////////////////////////////////////////////////////
// cgFoliageVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgTerrainVertex::declarator definition
const D3DVERTEXELEMENT9 cgFoliageVertex::declarator[5] =
{
    // cgVector3 position;
    {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
    // cgVector3 normal;
    {0,12,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_NORMAL,0},
    // cgUInt32 color;
    {0,24,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0},
    // cgVector2 coords;
    {0,28,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},
    D3DDECL_END()
};

///////////////////////////////////////////////////////////////////////////////
// cgClutterMaterial Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgClutterMaterial () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterMaterial::cgClutterMaterial( cgUInt32 referenceId, cgWorld * world ) : cgMaterial( referenceId, world )
{
    // Initialize variables
    
    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::Clutter'.
    mComponentTypeId = (cgUInt32)cgMaterialType::Clutter;
}

//-----------------------------------------------------------------------------
//  Name : cgClutterMaterial () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterMaterial::cgClutterMaterial( cgUInt32 referenceId, cgWorld * world, cgUInt32 sourceRefId ) : cgMaterial( referenceId, world, sourceRefId )
{
    // Initialize variables
    
    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::Clutter'.
    mComponentTypeId  = (cgUInt32)cgMaterialType::Clutter;
}

//-----------------------------------------------------------------------------
//  Name : cgClutterMaterial () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterMaterial::cgClutterMaterial( cgUInt32 referenceId, cgWorld * world, cgClutterMaterial * init ) : cgMaterial( referenceId, world, init )
{
    // Process layers.
    if ( !init->mLayers.empty() )
    {
        mLayers.resize( init->mLayers.size() );
        for ( size_t i = 0; i < init->mLayers.size(); ++i )
        {
            cgClutterLayer * layer = init->mLayers[i];
            switch ( layer->getType() )
            {
                case cgClutterType::Foliage:
                    mLayers[i] = new cgFoliageClutterLayer( this, static_cast<cgFoliageClutterLayer*>(init->mLayers[i]) );
                    break;
            
            } // End switch type
        
        } // Next layer

    } // End if has layers
    
    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::Clutter'.
    mComponentTypeId = (cgUInt32)cgMaterialType::Clutter;
}

//-----------------------------------------------------------------------------
//  Name : ~cgClutterMaterial () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterMaterial::~cgClutterMaterial( )
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
void cgClutterMaterial::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base.
    if ( disposeBase == true )
        cgMaterial::dispose( true );
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
bool cgClutterMaterial::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ClutterMaterial )
        return true;

    // Supported by base?
    return cgMaterial::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgClutterMaterial::getDatabaseTable( ) const
{
    return _T("Materials::Clutter");
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterMaterial::prepareQueries()
{
    // Any database supplied?
    if ( mWorld == CG_NULL )
        return;

    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertMaterial.isPrepared() == false )
            mInsertMaterial.prepare( mWorld, _T("INSERT INTO 'Materials::Clutter' VALUES(?1,?2,?3,?4)"), true );  
        if ( mUpdateName.isPrepared() == false )
            mUpdateName.prepare( mWorld, _T("UPDATE 'Materials::Clutter' SET Name=?1 WHERE RefId=?2"), true );  
        if ( mUpdatePreview.isPrepared() == false )
            mUpdatePreview.prepare( mWorld, _T("UPDATE 'Materials::Clutter' SET PreviewImage=?1 WHERE RefId=?2"), true ); 
    
    } // End if sandbox

    // Read queries
    if ( mLoadMaterial.isPrepared() == false )
        mLoadMaterial.prepare( mWorld, _T("SELECT * FROM 'Materials::Clutter' WHERE RefId=?1"), true );
    if ( mLoadFoliageLayers.isPrepared() == false )
        mLoadFoliageLayers.prepare( mWorld, _T("SELECT LayerId FROM 'Materials::Clutter::FoliageLayers' WHERE MaterialId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : serializeMaterial() (Protected)
/// <summary>
/// Write or update the material data stored in the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgClutterMaterial::serializeMaterial( )
{
    // Bail if we are not allowed to serialize any data
    if ( shouldSerialize() == false )
        return true;

    // Bail if there is nothing to do.
    if ( mDBDirtyFlags == 0 && mMaterialSerialized == true )
        return true;

    // Catch exceptions
    try
    {
        // Start a new transaction
        mWorld->beginTransaction( _T("serializeMaterial") );

        // If material data has yet been inserted, serialize everything.
        // Note: 'Materials' table is part of the core specification
        // and as a result does not need to be created.
        prepareQueries();
        if ( !mMaterialSerialized )
        {
            // Material entry does not exist at all at this stage so insert it.
            mInsertMaterial.bindParameter( 1, mReferenceId );
            mInsertMaterial.bindParameter( 2, mName );

            // Preview Image
            mInsertMaterial.bindParameter( 3, CG_NULL, 0 );

            // Database ref count (just in case it has already been adjusted)
            mInsertMaterial.bindParameter( 4, mSoftRefCount );

            // Process!
            if ( !mInsertMaterial.step( true )  )
            {
                cgString error;
                mInsertMaterial.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to insert data for clutter material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Layers should now be serialized
            mDBDirtyFlags = LayersDirty;
            
            // All other material properties have been serialized
            mMaterialSerialized = true;
        
        } // End if not yet serialized
        
        // Serialize name
        if ( mDBDirtyFlags & NameDirty )
        {
            // Update name.
            mUpdateName.bindParameter( 1, mName );
            mUpdateName.bindParameter( 2, mReferenceId );
            
            // Process!
            if ( mUpdateName.step( true ) == false )
            {
                cgString error;
                mUpdateName.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update name data for clutter material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~NameDirty;

        } // End if name dirty

        // Serialize layers
        if ( mDBDirtyFlags & LayersDirty )
        {
            // Give layers an opportunity to serialize.
            for ( size_t i = 0; i < mLayers.size(); ++i )
                mLayers[i]->serializeLayer();
            
            // Property has been serialized
            mDBDirtyFlags &= ~LayersDirty;

        } // End if name dirty

        // Commit recorded changes.
        mWorld->commitTransaction( _T("serializeMaterial") );

        // Success!
        return true;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        // Roll back any transaction
        mWorld->rollbackTransaction( _T("serializeMaterial") );
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch

}

//-----------------------------------------------------------------------------
//  Name : loadMaterial ()
/// <summary>
/// Load the specified material from the supplied database file.
/// Note : This function requires you to specify the resource manager into which
/// you would like all additional resources to be loaded (such as
/// textures, etc.) if you are not loading this material directly via the 
/// cgResourceManager::loadMaterial() methods.
/// </summary>
//-----------------------------------------------------------------------------
bool cgClutterMaterial::loadMaterial( cgUInt32 sourceRefId, cgResourceManager * resourceManager /* = CG_NULL */ )
{
    // Call base class implementation first.
    if ( cgMaterial::loadMaterial( sourceRefId, resourceManager ) == false )
        return false;

    // Handle exceptions
    try
    {
        /*// Utilize the landscape rendering shader.
        if ( !mSurfaceShader.isValid() )
        {
            if ( !mManager->createSurfaceShader( &mSurfaceShader, _T("sys://Shaders/Landscape.sh"), 0, cgDebugSource() ) )
                throw cgExceptions::ResultException( _T("Failed to instantiate landscape material surface shader. See previous output for further details."), cgDebugSource() );
        
        } // End if no shader loaded*/

        // Load the primary material data entry.
        prepareQueries();
        mLoadMaterial.bindParameter( 1, sourceRefId );
        if ( mLoadMaterial.step() == false || mLoadMaterial.nextRow() == false )
            throw cgExceptions::ResultException( _T("Failed to retrieve material data. World database has potentially become corrupt."), cgDebugSource() );

        // Retrieve the current 'soft' reference count for this component
        // if we are literally wrapping the database entry.
        if ( sourceRefId == mReferenceId )
            mLoadMaterial.getColumn( _T("RefCount"), mSoftRefCount );

        // Populate name and register it with the resource manager (do not use
        // the 'setName()' method as this will trigger premature serialization).
        mLoadMaterial.getColumn( _T("Name"), mName );
        if ( mName.empty() == false )
        {
            cgResourceManager::NameUsageMap & usage = mManager->getMaterialNameUsage();
            cgString nameKey = cgString::toLower( mName );
            cgResourceManager::NameUsageMap::iterator itName = usage.find( nameKey );
            if ( itName != usage.end() )
                itName->second = itName->second + 1;
            else
                usage[nameKey] = 1;

        } // End if has name
       
        // Preview Image
        // mLoadMaterial.getColumn( 4, CG_NULL, 0 ); // ToDo: 9999

        // We're done with the data from the material query
        mLoadMaterial.reset();

        // Load layer data.
        // Now load the sampler data.
        mLoadFoliageLayers.reset();
        mLoadFoliageLayers.bindParameter( 1, sourceRefId );
        if ( !mLoadFoliageLayers.step() )
            throw cgExceptions::ResultException( _T("Material data contained invalid or corrupt foliage layer information."), cgDebugSource() );

        // Iterate through each row returned and load each layer.
        for ( ; mLoadFoliageLayers.nextRow(); )
        {
            cgUInt32 layerId = 0;
            if ( mLoadFoliageLayers.getColumn( _T("LayerId"), layerId ) && layerId > 0 )
            {
                cgClutterLayer * layer = new cgFoliageClutterLayer(this);
                if ( !layer->loadLayer( layerId, (sourceRefId != mReferenceId) ) )
                {
                    delete layer;
                    continue;
                
                } // End if failed

                // Add the list of layers.
                mLayers.push_back( layer );

            } // End if valid
            
        } // Next sampler

        // We're done with the layer data.
        mLoadFoliageLayers.reset();

        // If we are not simply wrapping the original database entry (i.e. the reference
        // identifier of this material resource doesn't match the source identifier) 
        // then  we potentially need to serialize this data to the database next time 
        // 'serializeMaterial()' is called (assuming we are not an internal resource). 
        // See 'loadResource()' for more information.
        if ( isInternalReference() == false && sourceRefId != mReferenceId )
        {
            // We are not wrapping. Everything should be serialized.
            mMaterialSerialized   = false;
            mDBDirtyFlags         = AllDirty;

        } // End if not wrapping
        else
        {
            // We are wrapping. Everything is already serialized.
            mMaterialSerialized   = true;
            mDBDirtyFlags         = 0;
        
        } // End if wrapping
            
        // The material is now loaded
        mResourceLoaded = true;
        
        // Success!
        return true;

    } // End Try Block

    catch ( const cgExceptions::ResultException & e )
    {
        // Release any pending read operations.
        mLoadMaterial.reset();
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;

    } // End Catch Block
    return true;
}

//-----------------------------------------------------------------------------
//  Name : layerUpdated ()
/// <summary>
/// This method is triggered by child material layers whenever they have been
/// updated / one of their properties is modified.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterMaterial::layerUpdated( )
{
    // Note: Layers should automatically serialize themselves whenever they 
    // are modified, but in order to allow us to forcibly check that this is
    // the case at appropriate times, we flag as dirty here.
    mDBDirtyFlags |= LayersDirty;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// Unload the underlying resource data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgClutterMaterial::unloadResource( )
{
    // Dispose of referenced layers (disconnect, do not delete).
    for ( size_t i = 0; i < mLayers.size(); ++i )
        delete mLayers[i];
    mLayers.clear();
    
    // Call base class implementation last.
    return cgMaterial::unloadResource( );
}

//-----------------------------------------------------------------------------
//  Name : buildPreviewImage ()
/// <summary>
/// Construct the preview image used by the sandbox material editor to display 
/// a representation of this material. An internal scene is provided which is
/// set up with an appropriate render control script should it be required
/// (for instance, object rendering).
/// </summary>
//-----------------------------------------------------------------------------
bool cgClutterMaterial::buildPreviewImage( cgScene * scene, cgRenderView * view )
{
    // Find an appropriate layer.
    for ( size_t i = 0; i < mLayers.size(); ++i )
    {
        if ( mLayers[i]->getType() == cgClutterType::Foliage )
        {
            cgFoliageClutterLayer * layer = (cgFoliageClutterLayer*)mLayers[i];
            if ( !layer->getColorSampler() || !layer->getColorSampler()->isTextureValid() )
                continue;

            // Create the image ready to store data.
            cgSize size = view->getSize();
            if ( mPreviewImage == CG_NULL )
                mPreviewImage = cgImage::createInstance();
            cgToDo( "DX11", "Texture format needs considering more given lack of ARGB in DX10+ (Use GetBestFormat()?)" );
            if ( mPreviewImage->createImage( size.width, size.height, cgBufferFormat::B8G8R8A8, true ) == false )
                return false;

            // Copy the texture into the preview image.
            cgTexture * texture = layer->getColorSampler()->getTexture().getResource(true);
            if ( !texture->getImageData( *mPreviewImage ) )
            {
                delete mPreviewImage;
                mPreviewImage = CG_NULL;
                return false;
            
            } // End if failed
            
            // Success
            return true;
            
        } // End if foliage
    
    } // Next layer
    
    // No available preview
    return false;
}

//-----------------------------------------------------------------------------
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterMaterial::onComponentDeleted( )
{
    // Delete referenced layers.
    for ( size_t i = 0; i < mLayers.size(); ++i )
    {
        mLayers[i]->deleteLayer();
        delete mLayers[i];
    
    } // End if deleted.
    mLayers.clear();
    
    // Call base class implementation last.
    cgMaterial::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
//  Name : compare ()
/// <summary>
/// Performs a deep compare allowing us to perform rapid binary tree 
/// style searches (such as the map in our resource manager).
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgClutterMaterial::compare( const cgMaterial & material ) const
{
    // Clutter materials cannot be batched. Returns 'false' from cgMaterial::canBatch()
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : setSurfaceShader ()
/// <summary>
/// Load the surface shader file specified for rendering with this material.
/// </summary>
//-----------------------------------------------------------------------------
bool cgClutterMaterial::setSurfaceShader( const cgSurfaceShaderHandle & shader )
{
    // No shader can ever be set for this material.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// Apply the necessary material data to the device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgClutterMaterial::apply( cgRenderDriver * driver )
{
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : reset ()
/// <summary>
/// Reset / restore any applicable data when the material is no longer set to 
/// the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterMaterial::reset( cgRenderDriver * driver )
{
    // Nothing in this implementation
}

//-----------------------------------------------------------------------------
//  Name : createLayer ()
/// <summary>
/// Create a new clutter layer of the specified type to be applied with this 
/// material.
/// </summary>
//-----------------------------------------------------------------------------
size_t cgClutterMaterial::createLayer( cgClutterType::Base type )
{
    // Allocate a new clutter layer.
    cgClutterLayer * newLayer = CG_NULL;
    switch ( type )
    {
        case cgClutterType::Foliage:
            newLayer = new cgFoliageClutterLayer( this );
            break;
    
    } // End switch type

    // Serialize the layer
    if ( !newLayer->serializeLayer() )
    {
        delete newLayer;
        return (size_t)-1;
    
    } // End if failed

    // Add to the list of layers maintained by this material.
    mLayers.push_back( newLayer );

    // Notify listeners that material data has changed.
    static const cgString context = _T("LayerCreated");
    onComponentModified( &cgComponentModifiedEventArgs( context, mLayers.size() - 1 ) );

    // Return the index to the new layer.
    return mLayers.size() - 1;
}

//-----------------------------------------------------------------------------
//  Name : removeLayer ()
/// <summary>
/// Remove the clutter layer that exists at the specified index.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterMaterial::removeLayer( size_t index )
{
    if ( index >= mLayers.size() )
        return;

    // Delete the layer from the database.
    mLayers[index]->deleteLayer();

    // Remove from the actual array.
    delete mLayers[index];
    mLayers.erase( mLayers.begin() + index );

    // Notify listeners that material data has changed.
    static const cgString context = _T("LayerRemoved");
    onComponentModified( &cgComponentModifiedEventArgs( context, index ) );
}

//-----------------------------------------------------------------------------
//  Name : removeLayer ()
/// <summary>
/// Remove the specified clutter layer from this material.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterMaterial::removeLayer( cgClutterLayer * layer )
{
    for ( size_t i = 0; i < mLayers.size(); ++i )
    {
        if ( mLayers[i] == layer )
        {
            removeLayer(i);
            return;
        
        } // End if match
    
    } // Next layer
}

//-----------------------------------------------------------------------------
//  Name : getLayerCount ()
/// <summary>
/// Retrieve the total number of clutter layers defined within this material.
/// </summary>
//-----------------------------------------------------------------------------
size_t cgClutterMaterial::getLayerCount( ) const
{
    return mLayers.size();
}

//-----------------------------------------------------------------------------
//  Name : getLayer ()
/// <summary>
/// Retrieve the specified clutter layer by index (0 <= index < getLayerCount()).
/// </summary>
//-----------------------------------------------------------------------------
cgClutterLayer * cgClutterMaterial::getLayer( size_t index ) const
{
    return mLayers[index];
}

///////////////////////////////////////////////////////////////////////////////
// cgClutterLayer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgClutterLayer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterLayer::cgClutterLayer( cgClutterMaterial * material ) :
    mMaterial( material )
{
    // Initialize variables
    mLayerSerialized    = false;
    mDBDirtyFlags       = 0;
    mLastModifiedFrame  = cgTimer::getInstance()->getFrameCounter();
}

//-----------------------------------------------------------------------------
//  Name : cgClutterLayer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterLayer::cgClutterLayer( cgClutterMaterial * material, cgClutterLayer * initLayer ) :
    mMaterial( material )
{
    // Initialize variables
    mLayerSerialized    = false;
    mDBDirtyFlags       = AllDirty;
    mLastModifiedFrame  = cgTimer::getInstance()->getFrameCounter();
}

//-----------------------------------------------------------------------------
//  Name : ~cgClutterLayer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterLayer::~cgClutterLayer( )
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
void cgClutterLayer::dispose( bool disposeBase )
{
}

//-----------------------------------------------------------------------------
//  Name : getMaterial ()
/// <summary>
/// Retrieve the material that owns this layer.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterMaterial * cgClutterLayer::getMaterial( )
{
    return mMaterial;
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Retrieve the name of this layer for identification purposes.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgClutterLayer::getName( ) const
{
    return mName;
}

//-----------------------------------------------------------------------------
//  Name : setName () (Virtual)
/// <summary>
/// Set the name of this layer for identification purposes. Derived class 
/// should override this method in order to provide serialization behaviors.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterLayer::setName( const cgString & name )
{
    mName = name;
}

//-----------------------------------------------------------------------------
//  Name : getPerlinGenerator () (Virtual)
/// <summary>
/// Retrieve the shared perlin noise generator, configured with the appropriate
/// seed and properties for this clutter layer.
/// </summary>
//-----------------------------------------------------------------------------
cgRandom::NoiseGenerator & cgClutterLayer::getPerlinGenerator( )
{
    return mPerlin;
}

//-----------------------------------------------------------------------------
//  Name : isDirtySince () (Virtual)
/// <summary>
/// Determine if the layer has been modified at some point after the specified
/// frame.
/// </summary>
//-----------------------------------------------------------------------------
bool cgClutterLayer::isDirtySince( cgUInt32 frame ) const
{
    return ( mLastModifiedFrame >= frame );
}

///////////////////////////////////////////////////////////////////////////////
// cgClutterCell Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgClutterCell () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterCell::cgClutterCell( cgClutterLayer * layer ) :
    mLayer( layer )
{
    // Initialize variables
    mGridLayout         = cgSize( 0, 0 );
    mLastRefreshFrame   = 0;
}

//-----------------------------------------------------------------------------
//  Name : ~cgClutterCell () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterCell::~cgClutterCell( )
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
void cgClutterCell::dispose( bool disposeBase )
{
    // Clean up.
    mGrid.clear();

    // Reset variables
    mGridLayout         = cgSize( 0, 0 );
    mLastRefreshFrame   = 0;
}

//-----------------------------------------------------------------------------
//  Name : setCellLayout ()
/// <summary>
/// Clutter cells can house more than one displacement / layer map in a grid 
/// like pattern. For instance, one clutter cell might house data for a 
/// collection of 2x2 terrain blocks, or any other subdivided section of a 
/// larger world region. This method can be called to alter the internal grid
/// grid from its default of 1x1, to any larger grid layout, as well as to
/// supply cell offset and size data. This method must be called prior to 
/// applying displacement or layer maps.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterCell::setCellLayout( const cgPointF & cellOffset, const cgSizeF & cellSize, const cgSize & cellGridLayout )
{
    // Store layout data.
    mCellOffset = cellOffset;
    mCellSize   = cellSize;
    mGridLayout = cellGridLayout;

    // Resize the grid.
    mGrid.clear();
    mGrid.resize( mGridLayout.width * mGridLayout.height );
}

//-----------------------------------------------------------------------------
//  Name : setGridDisplacement ()
/// <summary>
/// Supply the displacement map for a specific grid square within this cell.
/// This is used to compute the correct Y axis offset for each clutter element.
/// Note: no copy will be made of the displacement data, and thus the lifetime
/// of this information must be guaranteed.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterCell::setGridDisplacement( cgInt32 x, cgInt32 y, const cgInt16 * displacement, const cgSize & mapPitch, const cgRect & sourceRegion, cgFloat displacementScale )
{
    GridData & data = mGrid[ x + y * mGridLayout.width ];
    data.displacement       = displacement;
    data.displacementPitch  = mapPitch;
    data.displacementRegion = sourceRegion;
    data.displacementScale  = displacementScale;
}

//-----------------------------------------------------------------------------
//  Name : setGridGrowthMask ()
/// <summary>
/// Supply a 'mask' / layer map for a specific grid square with this cell.
/// This can be used to mask off areas within the cell where the clutter may
/// or may not exist. Note: no copy will be made of the mask data, and thus 
/// the lifetime of this information must be guaranteed.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterCell::setGridGrowthMask( cgInt32 x, cgInt32 y, const cgByte * mapData, const cgSize & mapPitch, const cgRect & sourceRegion )
{
    GridData & data = mGrid[ x + y * mGridLayout.width ];
    data.growthMask       = mapData;
    data.growthMaskPitch  = mapPitch;
    data.growthMaskRegion = sourceRegion;
}

//-----------------------------------------------------------------------------
//  Name : noise2D ()
/// <summary>
/// Generate a random noise value based on the specified 2d position.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgClutterCell::noise2D( cgUInt32 seed, cgFloat x, cgFloat y )
{
    const cgDouble scale = 2398.23;
    seed = cgUInt32(seed)*1087;
    seed ^= 0xE56FAA12;
    seed += cgUInt32(x*scale)*2749;
    seed ^= 0x69628a2d;
    seed += cgUInt32(y*scale)*3433;
    seed ^= 0xa7b2c49a;
    return cgFloat(seed%2000)/2000.0f;
}

//-----------------------------------------------------------------------------
//  Name : getDisplacement ()
/// <summary>
/// Compute the displacement at the specified position within the cell based on
/// the provided displacement data. Coordinates should be expressed in world
/// space.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgClutterCell::getDisplacement( cgFloat x, cgFloat y )
{
    // Transform coordinates into the space of this cell.
    x -= mCellOffset.x;
    y -= mCellOffset.y;

    // Select the correct grid element into which this point falls.
    const cgFloat gridSizeX = (mCellSize.width / (cgFloat)mGridLayout.width);
    const cgFloat gridSizeY = (mCellSize.height / (cgFloat)mGridLayout.height);
    cgFloat gfx = x / gridSizeX;
    cgFloat gfy = -y / gridSizeY;

    // Integer grid array indices.
    cgInt32 gx = (cgInt32)floorf(gfx);
    cgInt32 gy = (cgInt32)floorf(gfy);

    // Fractional component (grid array element delta).
    cgFloat gdx = gfx - gx;
    cgFloat gdy = gfy - gy;

    // Clamp grid indices.
    if ( gx < 0 )
    {
        gx = 0;
        gdx = 0;
    }
    if ( gx >= mGridLayout.width )
    {
        gx = mGridLayout.width - 1;
        gdx = 1.0f;
    }
    if ( gy < 0 )
    {
        gy = 0;
        gdy = 0;
    }
    if ( gy >= mGridLayout.height )
    {
        gy = mGridLayout.height - 1;
        gdy = 1.0f;
    }

    // Select the correct grid element.
    GridData & data = mGrid[gx + gy * mGridLayout.width];
    if ( !data.displacement )
        return 0.0f;

    // Compute the correct location in the displacement map.
    cgInt width = data.displacementRegion.width(), height = data.displacementRegion.height();
    x = gdx * cgFloat(width - 1);
    y = gdy * cgFloat(height - 1);
    if ( x < 0 )
        x = 0;
    if ( y < 0 )
        y = 0;
    
    // First retrieve the indices for the top left displacement samples
    cgInt ix = (cgInt)x;
    cgInt iy = (cgInt)y;
    if ( ix > width - 2 )
        ix = width - 2;
    if ( iy > height - 2 )
        iy = height - 2;
	
    // Calculate the remainder (percent across quad)
    cgFloat dx = x - (cgFloat)ix;
    cgFloat dy = y - (cgFloat)iy;

    // Offset starting location to correct location in the displacement map.
    ix += data.displacementRegion.left;
    iy += data.displacementRegion.top;

    // First retrieve the height of each point in the dividing edge
    cgFloat topLeft     = (cgFloat)data.displacement[ ix + iy * data.displacementPitch.width ];
    cgFloat bottomRight = (cgFloat)data.displacement[ (ix + 1) + (iy + 1) * data.displacementPitch.height ];

    // Which triangle of the quad are we in ?
    cgFloat bottomLeft, topRight;
    if ( dx < dy )
    {
        bottomLeft = (cgFloat)data.displacement[ ix + (iy + 1) * data.displacementPitch.width ];
        topRight = topLeft + (bottomRight - bottomLeft);
    
    } // End if left Triangle
    else
    {
        topRight   = (cgFloat)data.displacement[ (ix + 1) + iy * data.displacementPitch.width ];
        bottomLeft = topLeft + (bottomRight - topRight);

    } // End if Right Triangle
    
    // Calculate the height interpolated across the top and bottom edges
    cgFloat topHeight    = topLeft    + ((topRight - topLeft) * dx );
    cgFloat bottomHeight = bottomLeft + ((bottomRight - bottomLeft) * dx );

    // Calculate the resulting height interpolated between the two heights
    return (topHeight + ((bottomHeight - topHeight) * dy )) * data.displacementScale;
}

//-----------------------------------------------------------------------------
//  Name : getGrowthMask ()
/// <summary>
/// Compute the growth mask value at the specified position within the cell 
/// based on the provided layer mask data. Coordinates should be expressed in 
/// world space.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgClutterCell::getGrowthMask( cgFloat x, cgFloat y )
{
    // Transform coordinates into the space of this cell.
    x -= mCellOffset.x;
    y -= mCellOffset.y;

    // Select the correct grid element into which this point falls.
    const cgFloat gridSizeX = (mCellSize.width / (cgFloat)mGridLayout.width);
    const cgFloat gridSizeY = (mCellSize.height / (cgFloat)mGridLayout.height);
    cgFloat gfx = x / gridSizeX;
    cgFloat gfy = -y / gridSizeY;

    // Integer grid array indices.
    cgInt32 gx = (cgInt32)floorf(gfx);
    cgInt32 gy = (cgInt32)floorf(gfy);

    // Fractional component (grid array element delta).
    cgFloat gdx = gfx - gx;
    cgFloat gdy = gfy - gy;

    // Clamp grid indices.
    if ( gx < 0 )
    {
        gx = 0;
        gdx = 0;
    }
    if ( gx >= mGridLayout.width )
    {
        gx = mGridLayout.width - 1;
        gdx = 1.0f;
    }
    if ( gy < 0 )
    {
        gy = 0;
        gdy = 0;
    }
    if ( gy >= mGridLayout.height )
    {
        gy = mGridLayout.height - 1;
        gdy = 1.0f;
    }

    // Select the correct grid element.
    GridData & data = mGrid[gx + gy * mGridLayout.width];
    if ( !data.growthMask )
        return 0.0f;

    // Compute the correct location in the growth mask.
    cgInt width = data.growthMaskRegion.width(), height = data.growthMaskRegion.height();
    x = gdx * (cgFloat)width;
    y = gdy * (cgFloat)height;
    x -= 0.5f;
    y -= 0.5f;
    if ( x < 0 )
        x = 0;
    if ( y < 0 )
        y = 0;
    
    // Retrieve the indices for the current growth mask sample
    cgInt ix = (cgInt)x;
    cgInt iy = (cgInt)y;

    // Calculate the remainder (percent across sample)
    cgFloat dx = x - (cgFloat)ix;
    cgFloat dy = y - (cgFloat)iy;

    // Clamp
    if ( ix >= width )
    {
        ix = width - 1;
        dx = 1.0f;
    }
    if ( iy >= height )
    {
        iy = height - 1;
        dy = 1.0f;
    }

    // Offset starting location to correct location in the growth mask.
    ix += data.growthMaskRegion.left;
    iy += data.growthMaskRegion.top;

    // Retrieve all four samples
    const cgInt ox = (ix < width-1) ? 1 : 0, oy = (iy < height-1) ? 1 : 0;
    cgFloat c00 = data.growthMask[ ix + iy * data.growthMaskPitch.width ] / 255.0f;
    cgFloat c10 = data.growthMask[ (ix+ox) + iy * data.growthMaskPitch.width ] / 255.0f;
    cgFloat c01 = data.growthMask[ ix + (iy+oy) * data.growthMaskPitch.width ] / 255.0f;
    cgFloat c11 = data.growthMask[ (ix+ox) + (iy+oy) * data.growthMaskPitch.width ] / 255.0f;
    
    // Interpolate horizontal
    cgFloat h1 = c00 + (c10-c00) * dx;
    cgFloat h2 = c01 + (c11-c01) * dx;

    // Interpolate vertical.
    return h1 + (h2-h1) * dy;
}

//-----------------------------------------------------------------------------
//  Name : getNormal ()
/// <summary>
/// Compute the surface normal at the specified position within the cell based 
/// on the provided displacement data. Coordinates should be expressed in world
/// space.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgClutterCell::getNormal( cgFloat x, cgFloat y )
{
    // Transform coordinates into the space of this cell.
    x -= mCellOffset.x;
    y -= mCellOffset.y;

    // Select the correct grid element into which this point falls.
    const cgFloat gridSizeX = (mCellSize.width / (cgFloat)mGridLayout.width);
    const cgFloat gridSizeY = (mCellSize.height / (cgFloat)mGridLayout.height);
    cgFloat gfx = x / gridSizeX;
    cgFloat gfy = -y / gridSizeY;

    // Integer grid array indices.
    cgInt32 gx = (cgInt32)floorf(gfx);
    cgInt32 gy = (cgInt32)floorf(gfy);

    // Fractional component (grid array element delta).
    cgFloat gdx = gfx - gx;
    cgFloat gdy = gfy - gy;

    // Clamp grid indices.
    if ( gx < 0 )
    {
        gx = 0;
        gdx = 0;
    }
    if ( gx >= mGridLayout.width )
    {
        gx = mGridLayout.width - 1;
        gdx = 1.0f;
    }
    if ( gy < 0 )
    {
        gy = 0;
        gdy = 0;
    }
    if ( gy >= mGridLayout.height )
    {
        gy = mGridLayout.height - 1;
        gdy = 1.0f;
    }

    // Select the correct grid element.
    GridData & data = mGrid[gx + gy * mGridLayout.width];
    if ( !data.displacement )
        return cgVector3(0,1,0);

    // Compute the correct location in the displacement map.
    cgInt width = data.displacementRegion.width(), height = data.displacementRegion.height();
    x = gdx * cgFloat(width - 1);
    y = gdy * cgFloat(height - 1);
    if ( x < 0 )
        x = 0;
    if ( y < 0 )
        y = 0;
    
    // First retrieve the indices for the top left displacement samples
    cgInt ix = (cgInt)x;
    cgInt iy = (cgInt)y;

    // Calculate the remainder (percent across quad)
    cgFloat dx = x - (cgFloat)ix;
    cgFloat dy = y - (cgFloat)iy;

    // Clamp
    if ( ix > width - 2 )
    {
        ix = width - 2;
        dx = 1.0f;
    }
    if ( iy > height - 2 )
    {
        iy = height - 2;
        dy = 1.0f;
    }
	
    // Offset starting location to correct location in the displacement map.
    ix += data.displacementRegion.left;
    iy += data.displacementRegion.top;

    // Retrieve the 4x4 height samples we need from the displacement map.
    cgFloat samples[4][4] = {0};
    for ( cgInt sy = iy - 1, oy = 0; sy <= iy + 2; ++sy, ++oy )
    {
        const cgInt cy = (sy < 0) ? 0 : (sy >= data.displacementPitch.height) ? data.displacementPitch.height-1 : sy;
        for ( cgInt sx = ix - 1, ox = 0; sx <= ix + 2; ++sx, ++ox )
        {
            const cgInt cx = (sx < 0) ? 0 : (sx >= data.displacementPitch.width) ? data.displacementPitch.width-1 : sx;
            samples[ox][oy] = (cgFloat)data.displacement[ cx + cy * data.displacementPitch.width ] * data.displacementScale;
        
        } // Next column
    
    } // Next row

    // Compute averaged normals for the four corner points that
    // make up this quad.
    cgFloat count = 0.0f;
    cgVector3 normals[4], edge1, edge2, normal;
    memset( normals, 0, 4 * sizeof(cgVector3) );
    const cgFloat scaleX = gridSizeX / width, scaleY = gridSizeY / height;
    static const cgPoint offsets[4] = { cgPoint(1,1), cgPoint(2,1), cgPoint(2,2), cgPoint(1,2) };
    for ( cgInt i = 0; i < 4; ++i )
    {
        const cgInt sx = offsets[i].x, sy = offsets[i].y;
        const cgFloat center = samples[sx][sy];
        
        // Top right quadrant
        edge1 = cgVector3( 0.0f, samples[sx][sy-1] - center, scaleY );
        edge2 = cgVector3( scaleX, samples[sx+1][sy] - center, 0.0f );
        cgVector3::normalize( normal, *cgVector3::cross( normal, edge1, edge2 ) );
        normals[i] += normal;
        count += 1.0f;
        
        // Bottom right quadrant
        edge1 = cgVector3( scaleX, samples[sx+1][sy] - center, 0.0f );
        edge2 = cgVector3( 0.0f, samples[sx][sy+1] - center, -scaleY );
        cgVector3::normalize( normal, *cgVector3::cross( normal, edge1, edge2 ) );
        normals[i] += normal;
        count += 1.0f;

        // Bottom left quadrant
        edge1 = cgVector3( 0.0f, samples[sx][sy+1] - center, -scaleY );
        edge2 = cgVector3( -scaleX, samples[sx-1][sy] - center, 0.0f );
        cgVector3::normalize( normal, *cgVector3::cross( normal, edge1, edge2 ) );
        normals[i] += normal;
        count += 1.0f;

        // Top left quadrant
        edge1 = cgVector3( -scaleX, samples[sx-1][sy] - center, 0.0f );
        edge2 = cgVector3( 0.0f, samples[sx][sy-1] - center, scaleY );
        cgVector3::normalize( normal, *cgVector3::cross( normal, edge1, edge2 ) );
        normals[i] += normal;
        count += 1.0f;

        // Valid?
        if ( count ) 
            normals[i] /= count;
        else
            normals[i] = cgVector3(0,1,0);

    } // Next corner

    // Now we need to interpolate the normals.
    cgVector3 topLeft     = normals[0];
    cgVector3 bottomRight = normals[2];

    // Which triangle of the quad are we in ?
    cgVector3 bottomLeft, topRight;
    if ( dx < dy )
    {
        bottomLeft = normals[3];
        topRight = topLeft + (bottomRight - bottomLeft);
            
    } // End if left Triangle
    else
    {
        topRight   = normals[1];
        bottomLeft = topLeft + (bottomRight - topRight);

    } // End if Right Triangle

     // Calculate the height interpolated across the top and bottom edges
    cgVector3 top    = topLeft    + ((topRight - topLeft) * dx );
    cgVector3 bottom = bottomLeft + ((bottomRight - bottomLeft) * dx );

    // Calculate the resulting height interpolated between the two heights
    cgVector3::normalize( normal, (top + ((bottom - top) * dy )) );
    return normal;
}

//-----------------------------------------------------------------------------
//  Name : invalidate ()
/// <summary>
/// Force the cell to regenerate its contents as necessary next time it is
/// rendered.
/// </summary>
//-----------------------------------------------------------------------------
void cgClutterCell::invalidate( )
{
    mLastRefreshFrame = 0;
}

//-----------------------------------------------------------------------------
//  Name : getLayer ()
/// <summary>
/// Retrieve the layer that is providing the clutter definition for this cell.
/// </summary>
//-----------------------------------------------------------------------------
cgClutterLayer * cgClutterCell::getLayer( )
{
    return mLayer;
}

//-----------------------------------------------------------------------------
//  Name : getCellGridLayout ()
/// <summary>
/// Retrieve the size/layout of the displacement grid for this cell.
/// </summary>
//-----------------------------------------------------------------------------
const cgSize & cgClutterCell::getCellGridLayout( ) const
{
    return mGridLayout;
}

///////////////////////////////////////////////////////////////////////////////
// cgFoliageClutterLayer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgFoliageClutterLayer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFoliageClutterLayer::cgFoliageClutterLayer( cgClutterMaterial * material ) :
    cgClutterLayer( material )
{
    // Initialize variables
    mRenderMethod       = cgFoliageRenderMethod::Billboard;
    mAnimationMethod    = cgFoliageAnimationMethod::None;
    mWidth              = cgRangeF(1.0f,1.0f);
    mHeight             = cgRangeF(1.0f,1.0f);
    mFadeDistances      = cgRangeF(40, 50);
    mGrowthChance       = 1.0f;
    mClusterElements    = 3;
    mClusterRadius      = 0.0f;
    mClusterEmbed       = 0.0f;
    mClusterSeparation  = 1.0f;
    mColorSampler       = CG_NULL;
    mNormalSampler      = CG_NULL;
    mScreenDoorSampler  = CG_NULL;

    // Select an initial seed.
    mSeed               = cgRandom::ParkMiller().getSeed();

    // Initialise the random number generator.
    mPerlin.setSeed( mSeed );

    // Construct required resources.
    buildResources();
}

//-----------------------------------------------------------------------------
//  Name : cgFoliageClutterLayer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFoliageClutterLayer::cgFoliageClutterLayer( cgClutterMaterial * material, cgFoliageClutterLayer * initLayer ) :
    cgClutterLayer( material, initLayer )
{
    // Initialize variables
    mColorSampler       = CG_NULL;
    mNormalSampler      = CG_NULL;
    mScreenDoorSampler  = CG_NULL;

    // Material data.
    mRenderMethod       = initLayer->mRenderMethod;
    mAnimationMethod    = initLayer->mAnimationMethod;
    mWidth              = initLayer->mWidth;
    mHeight             = initLayer->mHeight;
    mFadeDistances      = initLayer->mFadeDistances;
    mGrowthChance       = initLayer->mGrowthChance;
    mClusterElements    = initLayer->mClusterElements;
    mClusterRadius      = initLayer->mClusterRadius;
    mClusterEmbed       = initLayer->mClusterEmbed;
    mClusterSeparation  = initLayer->mClusterSeparation;
    mSeed               = initLayer->mSeed;

    // Initialise the random number generator.
    mPerlin.setSeed( mSeed );

    // Construct required resources.
    buildResources();

    // Duplicate sampler data
    mColorSampler       = material->getManager()->cloneSampler( material->getParentWorld(), material->isInternalReference(), initLayer->mColorSampler );
    mNormalSampler      = material->getManager()->cloneSampler( material->getParentWorld(), material->isInternalReference(), initLayer->mNormalSampler );
}

//-----------------------------------------------------------------------------
//  Name : ~cgFoliageClutterLayer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFoliageClutterLayer::~cgFoliageClutterLayer( )
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
void cgFoliageClutterLayer::dispose( bool disposeBase )
{
    // Release all samplers we own (disconnect).
    if ( mColorSampler )
        mColorSampler->removeReference(CG_NULL,true);
    if ( mNormalSampler )
        mNormalSampler->removeReference(CG_NULL,true);
    if ( mScreenDoorSampler )
        mScreenDoorSampler->removeReference(CG_NULL);
    mColorSampler       = CG_NULL;
    mNormalSampler      = CG_NULL;
    mScreenDoorSampler  = CG_NULL;

    // Release resources
    mShader.close();
    mRasterizerState.close(true);
    mDepthFillDepthState.close(true);
    mGeometryFillDepthState.close(true);

    // Dispose base class.
    if ( disposeBase )
        cgClutterLayer::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : setName () (Virtual)
/// <summary>
/// Set the name of this layer for identification purposes.
/// </summary>
//-----------------------------------------------------------------------------
void cgFoliageClutterLayer::setName( const cgString & name )
{
    // Is this a no-op?
    if ( mName == name )
        return;

    // Call base class implementation first.
    cgClutterLayer::setName( name );

    // Name is now dirty
    mDBDirtyFlags |= NameDirty;
    serializeLayer();
}

//-----------------------------------------------------------------------------
// Name : loadColorSampler()
/// <summary>Load the specified texture into the color sampler.</summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterLayer::loadColorSampler( cgInputStream stream )
{
    // Create a new sampler of the required type (internal or serialized 
    // depending on the properties of this parent material).
    cgSampler * sampler = CG_NULL;
    if ( !mMaterial->isInternalReference() )
        sampler = mMaterial->getManager()->createSampler( mMaterial->getParentWorld(), _T("Diffuse"), mShader );
    else
        sampler = mMaterial->getManager()->createSampler( _T("Diffuse"), mShader );

    // Do not tile!
    sampler->setAddressU( cgAddressingMode::Border );
    sampler->setAddressV( cgAddressingMode::Border );
    sampler->setBorderColor( 0 );
    
    // Load the referenced texture. We'll ignore the actual loading
    // if the source was invalid. This allows the caller to specify
    // an empty stream simply to create a sampler.
    if ( stream.sourceExists() && !sampler->loadTexture( stream, 0, cgDebugSource() ) )
        return false;

    // Set the sampler
    setColorSampler( sampler );
    return true;
}

//-----------------------------------------------------------------------------
// Name : loadNormalSampler()
/// <summary>Load the specified texture into the normal sampler.</summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterLayer::loadNormalSampler( cgInputStream stream )
{
    // Create a new sampler of the required type (internal or serialized 
    // depending on the properties of this parent material).
    cgSampler * sampler = CG_NULL;
    if ( !mMaterial->isInternalReference() )
        sampler = mMaterial->getManager()->createSampler( mMaterial->getParentWorld(), _T("Normal"), mShader );
    else
        sampler = mMaterial->getManager()->createSampler( _T("Normal"), mShader );
    
    // Load the referenced texture. We'll ignore the actual loading
    // if the source was invalid. This allows the caller to specify
    // an empty stream simply to create a sampler.
    if ( stream.sourceExists() && !sampler->loadTexture( stream, 0, cgDebugSource() ) )
        return false;

    // Set the sampler
    setNormalSampler( sampler );
    return true;
}

//-----------------------------------------------------------------------------
// Name : getColorSampler ( )
/// <summary>
/// Get the current color sampler to apply when rendering layers that
/// utilize this type.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgFoliageClutterLayer::getColorSampler( ) const
{
    return mColorSampler;
}

//-----------------------------------------------------------------------------
// Name : setColorSampler ( )
/// <summary>
/// Set the current color sampler to apply when rendering layers that
/// utilize this type.
/// </summary>
//-----------------------------------------------------------------------------
void cgFoliageClutterLayer::setColorSampler( cgSampler * sampler )
{
    // Is this a no-op?
    if ( sampler == mColorSampler )
        return;

    // Clean up any existing sampler.
    if ( mColorSampler )
        mColorSampler->removeReference( CG_NULL, mMaterial->isInternalReference() );

    // Add this material as a reference holder to the new sampler.
    if ( sampler )
        sampler->addReference( CG_NULL, mMaterial->isInternalReference() );

    // Replace current sampler.
    mColorSampler = sampler;

    // Samplers are now dirty
    mDBDirtyFlags |= SamplersDirty;
    serializeLayer();
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgFoliageClutterLayer::prepareQueries()
{
    // Any database supplied?
    if ( !mMaterial )
        return;

    // Prepare the SQL statements as necessary.
    cgWorld * world = mMaterial->getParentWorld();
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertLayer.isPrepared() == false )
            mInsertLayer.prepare( world, _T("INSERT INTO 'Materials::Clutter::FoliageLayers' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21)"), true );  
        if ( mUpdateName.isPrepared() == false )
            mUpdateName.prepare( world, _T("UPDATE 'Materials::Clutter::FoliageLayers' SET Name=?1 WHERE LayerId=?2"), true );  
        if ( mUpdateSamplers.isPrepared() == false )
            mUpdateSamplers.prepare( world, _T("UPDATE 'Materials::Clutter::FoliageLayers' SET ColorSamplerId=?1, NormalSamplerId=?2 WHERE LayerId=?3"), true );  
        if ( mUpdateRenderProperties.isPrepared() == false )
            mUpdateRenderProperties.prepare( world, _T("UPDATE 'Materials::Clutter::FoliageLayers' SET RenderMethod=?1, AnimationMethod=?2 WHERE LayerId=?3"), true );  
        if ( mUpdateElementProperties.isPrepared() == false )
            mUpdateElementProperties.prepare( world, _T("UPDATE 'Materials::Clutter::FoliageLayers' SET MinWidth=?1, MaxWidth=?2, MinHeight=?3, MaxHeight=?4, ClusterElements=?5, ClusterEmbed=?6, ClusterRadius=?7 WHERE LayerId=?8"), true );  
        if ( mUpdateGrowthProperties.isPrepared() == false )
            mUpdateGrowthProperties.prepare( world, _T("UPDATE 'Materials::Clutter::FoliageLayers' SET GrowthChance=?1, ClusterSeparation=?2, Seed=?3 WHERE LayerId=?4"), true );  
        if ( mUpdateMaterialProperties.isPrepared() == false )
            mUpdateMaterialProperties.prepare( world, _T("UPDATE 'Materials::Clutter::FoliageLayers' SET Color0=?1, Color1=?2 WHERE LayerId=?3"), true );  
        if ( mUpdateLODProperties.isPrepared() == false )
            mUpdateLODProperties.prepare( world, _T("UPDATE 'Materials::Clutter::FoliageLayers' SET FadeBeginDistance=?1, FadeEndDistance=?2 WHERE LayerId=?3"), true );  
        if ( mDeleteLayer.isPrepared() == false )
            mDeleteLayer.prepare( world, _T("DELETE FROM 'Materials::Clutter::FoliageLayers' WHERE LayerId=?1"), true );  
    
    } // End if sandbox

    // Read queries
    if ( mLoadLayer.isPrepared() == false )
        mLoadLayer.prepare( world, _T("SELECT * FROM 'Materials::Clutter::FoliageLayers' WHERE LayerId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : serializeLayer()
/// <summary>
/// Write or update the layer data stored in the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterLayer::serializeLayer( )
{
    // Bail if we are not allowed to serialize any data
    if ( !mMaterial->shouldSerialize() )
        return true;

    // Bail if there is nothing to do.
    if ( mDBDirtyFlags == 0 && mLayerSerialized == true )
        return true;

    // Catch exceptions
    cgWorld * world = mMaterial->getParentWorld();
    try
    {
        // Start a new transaction
        world->beginTransaction( _T("serializeLayer") );

        // If layer data has yet been inserted, serialize everything.
        // Note: 'Materials::Clutter::FoliageLayers' table is part 
        // of the core specification and as a result does not need to 
        // be created.
        prepareQueries();
        if ( !mLayerSerialized )
        {
            // Layer entry does not exist at all at this stage so insert it.
            mInsertLayer.bindParameter( 1, mMaterial->getReferenceId() );
            mInsertLayer.bindParameter( 2, mName );

            // Rendering / animation methods
            mInsertLayer.bindParameter( 3, (cgUInt32)mRenderMethod );
            mInsertLayer.bindParameter( 4, (cgUInt32)mAnimationMethod );

            // Element properties
            mInsertLayer.bindParameter( 5, mWidth.min );
            mInsertLayer.bindParameter( 6, mWidth.max );
            mInsertLayer.bindParameter( 7, mHeight.min );
            mInsertLayer.bindParameter( 8, mHeight.max );
            mInsertLayer.bindParameter( 9, mGrowthChance );
            mInsertLayer.bindParameter( 10, mClusterElements );
            mInsertLayer.bindParameter( 11, mClusterEmbed );
            mInsertLayer.bindParameter( 12, mClusterSeparation );
            mInsertLayer.bindParameter( 13, mClusterRadius );
            mInsertLayer.bindParameter( 14, mSeed );

            // Layer samplers
            mInsertLayer.bindParameter( 15, (mColorSampler) ? mColorSampler->getReferenceId() : 0 );
            mInsertLayer.bindParameter( 16, (mNormalSampler) ? mNormalSampler->getReferenceId() : 0 );
            mInsertLayer.bindParameter( 17, (cgUInt32)0 ); // Specular Sampler
            mInsertLayer.bindParameter( 18, (cgUInt32)0xFFFFFFFF ); // Color0
            mInsertLayer.bindParameter( 19, (cgUInt32)0xFFFFFFFF ); // Color1

            // LOD properties
            mInsertLayer.bindParameter( 20, mFadeDistances.min );
            mInsertLayer.bindParameter( 21, mFadeDistances.max );

            // Process!
            if ( !mInsertLayer.step( true )  )
            {
                cgString error;
                mInsertLayer.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to insert data for foliage clutter layer '%s' for material resource '0x%x'. Error: %s"), mName.c_str(), mMaterial->getReferenceId(), error.c_str()), cgDebugSource() );

            } // End if failed

            // Get the new database id.
            mDatabaseId = mInsertLayer.getLastInsertId();

            // All other material properties have been serialized
            mDBDirtyFlags = 0;
            mLayerSerialized = true;
        
        } // End if not yet serialized
        
        // Serialize name
        if ( mDBDirtyFlags & NameDirty )
        {
            // Update name.
            mUpdateName.bindParameter( 1, mName );
            mUpdateName.bindParameter( 2, mDatabaseId );
            
            // Process!
            if ( mUpdateName.step( true ) == false )
            {
                cgString error;
                mUpdateName.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update name data for foliage clutter layer '%s' for material resource '0x%x'. Error: %s"), mName.c_str(), mMaterial->getReferenceId(), error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~NameDirty;

        } // End if name dirty

        // Serialize rendering properties
        if ( mDBDirtyFlags & RenderPropertiesDirty )
        {
            // Update name.
            mUpdateRenderProperties.bindParameter( 1, (cgUInt32)mRenderMethod );
            mUpdateRenderProperties.bindParameter( 2, (cgUInt32)mAnimationMethod );
            mUpdateRenderProperties.bindParameter( 3, mDatabaseId );
            
            // Process!
            if ( mUpdateRenderProperties.step( true ) == false )
            {
                cgString error;
                mUpdateRenderProperties.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update rendering properties for foliage clutter layer '%s' for material resource '0x%x'. Error: %s"), mName.c_str(), mMaterial->getReferenceId(), error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~RenderPropertiesDirty;

        } // End if render properties dirty

        // Serialize element properties
        if ( mDBDirtyFlags & ElementPropertiesDirty )
        {
            // Update name.
            mUpdateElementProperties.bindParameter( 1, mWidth.min );
            mUpdateElementProperties.bindParameter( 2, mWidth.max );
            mUpdateElementProperties.bindParameter( 3, mHeight.min );
            mUpdateElementProperties.bindParameter( 4, mHeight.max );
            mUpdateElementProperties.bindParameter( 5, mClusterElements );
            mUpdateElementProperties.bindParameter( 6, mClusterEmbed );
            mUpdateElementProperties.bindParameter( 7, mClusterRadius );
            mUpdateElementProperties.bindParameter( 8, mDatabaseId );
            
            // Process!
            if ( mUpdateElementProperties.step( true ) == false )
            {
                cgString error;
                mUpdateElementProperties.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update element properties for foliage clutter layer '%s' for material resource '0x%x'. Error: %s"), mName.c_str(), mMaterial->getReferenceId(), error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~ElementPropertiesDirty;

        } // End if element properties dirty

        // Serialize growth properties
        if ( mDBDirtyFlags & GrowthPropertiesDirty )
        {
            // Update name.
            mUpdateGrowthProperties.bindParameter( 1, mGrowthChance );
            mUpdateGrowthProperties.bindParameter( 2, mClusterSeparation );
            mUpdateGrowthProperties.bindParameter( 3, mSeed );
            mUpdateGrowthProperties.bindParameter( 4, mDatabaseId );
            
            // Process!
            if ( mUpdateGrowthProperties.step( true ) == false )
            {
                cgString error;
                mUpdateGrowthProperties.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update growth properties for foliage clutter layer '%s' for material resource '0x%x'. Error: %s"), mName.c_str(), mMaterial->getReferenceId(), error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~GrowthPropertiesDirty;

        } // End if growth properties dirty

        // TODO: Material properties

        // Serialize LOD properties
        if ( mDBDirtyFlags & LODPropertiesDirty )
        {
            // Update name.
            mUpdateLODProperties.bindParameter( 1, mFadeDistances.min );
            mUpdateLODProperties.bindParameter( 2, mFadeDistances.max );
            mUpdateLODProperties.bindParameter( 3, mDatabaseId );
            
            // Process!
            if ( mUpdateLODProperties.step( true ) == false )
            {
                cgString error;
                mUpdateLODProperties.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update LOD properties for foliage clutter layer '%s' for material resource '0x%x'. Error: %s"), mName.c_str(), mMaterial->getReferenceId(), error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~LODPropertiesDirty;

        } // End if LOD properties dirty

        // Serialize samplers
        if ( mDBDirtyFlags & SamplersDirty )
        {
            // Update samplers.
            mUpdateSamplers.bindParameter( 1, (mColorSampler) ? mColorSampler->getReferenceId() : 0 );
            mUpdateSamplers.bindParameter( 2, (mNormalSampler) ? mNormalSampler->getReferenceId() : 0 );
            mUpdateSamplers.bindParameter( 3, mDatabaseId );
            
            // Process!
            if ( mUpdateSamplers.step( true ) == false )
            {
                cgString error;
                mUpdateSamplers.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update sampler data for foliage clutter layer '%s' for material resource '0x%x'. Error: %s"), mName.c_str(), mMaterial->getReferenceId(), error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~SamplersDirty;

        } // End if samplers dirty

        // Commit recorded changes.
        world->commitTransaction( _T("serializeLayer") );

        // Success!
        return true;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        // Roll back any transaction
        world->rollbackTransaction( _T("serializeLayer") );
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch

}

//-----------------------------------------------------------------------------
//  Name : loadLayer ()
/// <summary>
/// Load the specified layer from the world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterLayer::loadLayer( cgUInt32 sourceLayerId, bool cloning )
{
    // Handle exceptions
    try
    {
        // Load the primary layer data entry.
        prepareQueries();
        mLoadLayer.bindParameter( 1, sourceLayerId );
        if ( mLoadLayer.step() == false || mLoadLayer.nextRow() == false )
            throw cgExceptions::ResultException( _T("Failed to retrieve vegeteation clutter layer data. World database has potentially become corrupt."), cgDebugSource() );

        // Get base properties.
        mLoadLayer.getColumn( _T("Name"), mName );

        // Get layer properties.
        cgUInt32 renderMethod, animationMethod;
        mLoadLayer.getColumn( _T("LayerId"), mDatabaseId );
        mLoadLayer.getColumn( _T("RenderMethod"), renderMethod );
        mLoadLayer.getColumn( _T("AnimationMethod"), animationMethod );
        mLoadLayer.getColumn( _T("MinWidth"), mWidth.min );
        mLoadLayer.getColumn( _T("MaxWidth"), mWidth.max );
        mLoadLayer.getColumn( _T("MinHeight"), mHeight.min );
        mLoadLayer.getColumn( _T("MaxHeight"), mHeight.max );
        mLoadLayer.getColumn( _T("GrowthChance"), mGrowthChance );
        mLoadLayer.getColumn( _T("ClusterElements"), mClusterElements );
        mLoadLayer.getColumn( _T("ClusterRadius"), mClusterRadius );
        mLoadLayer.getColumn( _T("ClusterEmbed"), mClusterEmbed );
        mLoadLayer.getColumn( _T("ClusterSeparation"), mClusterSeparation );
        mLoadLayer.getColumn( _T("Seed"), mSeed );
        mLoadLayer.getColumn( _T("FadeBeginDistance"), mFadeDistances.min );
        mLoadLayer.getColumn( _T("FadeEndDistance"), mFadeDistances.max );

        // Update enumerations
        mRenderMethod = (cgFoliageRenderMethod::Base)renderMethod;
        mAnimationMethod = (cgFoliageAnimationMethod::Base)animationMethod;
        
        // Sampler data
        cgUInt32 colorSamplerId = 0, normalSamplerId = 0;
        mLoadLayer.getColumn( _T("ColorSamplerId"), colorSamplerId );
        mLoadLayer.getColumn( _T("NormalSamplerId"), normalSamplerId );
        
        // ToDo: what resource loading flags should be specified?
        // Load the referenced samplers.
        mColorSampler = mMaterial->getManager()->loadSampler( mMaterial->getParentWorld(), mShader, colorSamplerId, mMaterial->isInternalReference() );
        if ( mColorSampler )
        {
            // Add the layer as a reference holder (just reconnect,
            // do not adjust the database ref count).
            mColorSampler->addReference( CG_NULL, true );
        
        } // End if valid color
        mNormalSampler = mMaterial->getManager()->loadSampler( mMaterial->getParentWorld(), mShader, normalSamplerId, mMaterial->isInternalReference() );
        if ( mNormalSampler )
        {
            // Add the material as a reference holder (just reconnect,
            // do not adjust the database ref count).
            mNormalSampler->addReference( CG_NULL, true );
        
        } // End if valid color

        // Update constant buffer.
        if ( mFoliageConstants.isValid() )
        {
            _cbFoliageData constants;
            constants.fadeDistance.x = mFadeDistances.min;
            constants.fadeDistance.y = max( mFadeDistances.min + CGE_EPSILON_1CM, mFadeDistances.max );
            constants.fadeDistance.z = -constants.fadeDistance.x;
            constants.fadeDistance.w = 1.0f / (constants.fadeDistance.y - constants.fadeDistance.x);
	        mFoliageConstants->updateBuffer( 0, 0, &constants );
        
        } // End if buffer valid
       
        // We're done with the data from the layer query
        mLoadLayer.reset();

        // If we are not simply wrapping the original database entry then we potentially 
        // need to serialize this data to the database next time 'serializeLayer()' is 
        // called (assuming we are not an internal resource). 
        if ( !mMaterial->isInternalReference() && cloning )
        {
            // We are not wrapping. Everything should be serialized.
            mLayerSerialized = false;
            mDBDirtyFlags    = AllDirty;

        } // End if not wrapping
        else
        {
            // We are wrapping. Everything is already serialized.
            mLayerSerialized = true;
            mDBDirtyFlags    = 0;
        
        } // End if wrapping
            
        // Serialize necessary data.
        if ( mMaterial->shouldSerialize() )
            serializeLayer();
        
        // Success!
        return true;

    } // End Try Block

    catch ( const cgExceptions::ResultException & e )
    {
        // Release any pending read operations.
        mLoadLayer.reset();
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;

    } // End Catch Block
}

//-----------------------------------------------------------------------------
// Name : deleteLayer()
/// <summary>
/// Call in order to remove the layer entirely from the database.
/// </summary>
//-----------------------------------------------------------------------------
void cgFoliageClutterLayer::deleteLayer( )
{
    // Remove our physical references to any child samplers. Full database update 
    // and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    if ( mColorSampler )
        mColorSampler->removeReference(CG_NULL,mMaterial->isInternalReference());
    if ( mNormalSampler )
        mNormalSampler->removeReference(CG_NULL,mMaterial->isInternalReference());
    mColorSampler  = CG_NULL;
    mNormalSampler = CG_NULL;

    // Remove layer from the database.
    if ( mDatabaseId != 0 && mMaterial->shouldSerialize() )
    {
        prepareQueries();
        mDeleteLayer.bindParameter( 1, mDatabaseId );
    
    } // End if should remove
}

//-----------------------------------------------------------------------------
// Name : getNormalSampler ( )
/// <summary>
/// Get the current normal sampler to apply when rendering layer that
/// utilize this type.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgFoliageClutterLayer::getNormalSampler( ) const
{
    return mNormalSampler;
}

//-----------------------------------------------------------------------------
// Name : setNormalSampler ( )
/// <summary>
/// Set the current normal sampler to apply when rendering layer that
/// utilize this type.
/// </summary>
//-----------------------------------------------------------------------------
void cgFoliageClutterLayer::setNormalSampler( cgSampler * sampler )
{
    // Is this a no-op?
    if ( sampler == mNormalSampler )
        return;

    // Clean up any existing sampler.
    if ( mNormalSampler )
        mNormalSampler->removeReference( CG_NULL, mMaterial->isInternalReference() );

    // Add this material as a reference holder to the new sampler.
    if ( sampler )
        sampler->addReference( CG_NULL, mMaterial->isInternalReference() );

    // Replace current sampler.
    mNormalSampler = sampler;

    // Samplers are now dirty
    mDBDirtyFlags |= SamplersDirty;
    serializeLayer();
}

//-----------------------------------------------------------------------------
// Name : buildResources ( ) (Protected)
/// <summary>
/// Build the resources necessary for rendering with this clutter layer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterLayer::buildResources( )
{
    cgResourceManager * resources = mMaterial->getManager();

    // Rasterizer states.
    cgRasterizerStateDesc rs;
    rs.cullMode = cgCullMode::None;
    if ( !resources->createRasterizerState( &mRasterizerState, rs, 0, cgDebugSource() ) )
        return false;

    // Depth stencil states
    cgDepthStencilStateDesc ds;
    ds.stencilEnable                      = true;
    ds.frontFace.stencilFunction          = cgComparisonFunction::Always;
    ds.frontFace.stencilPassOperation     = cgStencilOperation::Replace;
    ds.backFace.stencilFunction           = cgComparisonFunction::Always;
    ds.backFace.stencilPassOperation      = cgStencilOperation::Replace;
    if ( !resources->createDepthStencilState( &mDepthFillDepthState, ds, 0, cgDebugSource() ) )
        return false;

    // Depth stencil states
    ds                  = cgDepthStencilStateDesc(); // Defaults
    ds.depthEnable      = true;
    ds.depthWriteEnable = false;
    ds.depthFunction    = cgComparisonFunction::Equal;
    ds.stencilEnable    = true;
    ds.frontFace.stencilFunction    = cgComparisonFunction::NotEqual;
    ds.backFace.stencilFunction     = cgComparisonFunction::NotEqual;
    if ( !resources->createDepthStencilState( &mGeometryFillDepthState, ds, 0, cgDebugSource() ) )
        return false;

    // Load the internal surface shader ready for drawing
    if ( !resources->createSurfaceShader( &mShader, _T("sys://Shaders/Foliage.sh"), 0, cgDebugSource() ) )
        return false;

    // Load the screen door noise texture.
    if ( !(mScreenDoorSampler = resources->createSampler( _T("ScreenDoorNoise"), mShader )) )
        return false;
    if ( !mScreenDoorSampler->loadTexture( _T("sys://Textures/ScreenDoor.dds"), 0, cgDebugSource() ) )
        return false;

    // Cache texture registers.
    mColorSamplerRegister = mShader->findSamplerRegister( _T("Diffuse") );
    mNormalSamplerRegister = mShader->findSamplerRegister( _T("Normal") );

    // Create constant buffers that maps layer parameter data
    // to the physical processing shader.
    if ( !resources->createConstantBuffer( &mFoliageConstants, mShader, _T("cbFoliageData"), cgDebugSource() ) )
        return false;
    cgAssert( mFoliageConstants->getDesc().length == sizeof(_cbFoliageData) );

    // Initialize the constant buffer
    _cbFoliageData constants;
    constants.fadeDistance.x = mFadeDistances.min;
    constants.fadeDistance.y = max( mFadeDistances.min + CGE_EPSILON_1CM, mFadeDistances.max );
    constants.fadeDistance.z = -constants.fadeDistance.x;
    constants.fadeDistance.w = 1.0f / (constants.fadeDistance.y - constants.fadeDistance.x);
	mFoliageConstants->updateBuffer( 0, 0, &constants );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : begin ( ) (Virtual)
/// <summary>
/// Apply states necessary for rendering clutter of this type.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterLayer::begin( cgRenderDriver * driver, bool depthFill )
{
    // Valid?
    if ( !mShader.isValid() )
        return false;

    // Apply color sampler (fall back to default if unavailable)
    cgResourceManager * resources = mMaterial->getManager();
    if ( mColorSampler )
    {
        if ( !mColorSampler->isTextureValid() )
            mColorSampler->apply(resources->getDefaultSampler(cgResourceManager::DefaultDiffuseSampler)->getTexture());
        else
            mColorSampler->apply();
    
    } // End if valid
    else
        resources->getDefaultSampler(cgResourceManager::DefaultDiffuseSampler)->apply(mColorSamplerRegister);

    // Apply color sampler (fall back to default if unavailable)
    if ( mNormalSampler )
    {
        if ( !mNormalSampler->isTextureValid() )
            mNormalSampler->apply(resources->getDefaultSampler(cgResourceManager::DefaultNormalSampler)->getTexture());
        else
            mNormalSampler->apply();
    
    } // End if valid
    else
        resources->getDefaultSampler(cgResourceManager::DefaultNormalSampler)->apply(mNormalSamplerRegister);

    // Apply screen door noise sampler
    if ( mScreenDoorSampler )
        mScreenDoorSampler->apply();

    // Custom rendering depending on pass.
    if ( depthFill )
    {
        // Build vertex / pixel shader permutation arguments.
        // ToDo: This can likely be pre-created in postInit (essentially binding address 
        // of system export 'depthType' variable to the shader permutation selector).
        static cgScriptArgument::Array args(3);
        cgInt32 depthOutputType = driver->getSystemState( cgSystemState::DepthType );
        cgInt32 normalOutputType = driver->getSystemState( cgSystemState::SurfaceNormalType );
        bool orthographicCamera = (driver->getSystemState( cgSystemState::OrthographicCamera ) != 0);
        args[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &depthOutputType );
        args[1] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &normalOutputType );
        args[2] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &orthographicCamera );

        // Select the vertex and pixel shader.
        cgSurfaceShader * shader = mShader.getResource(true);
        if ( !shader->selectVertexShader( _T("transformDepth"), args ) ||
             !shader->selectPixelShader( _T("drawDepth"), args ) )
             return false;

        // Apply pass specific states
        driver->pushDepthStencilState( mDepthFillDepthState );
    
    } // End if depth fill
    else
    {
        // Build vertex / pixel shader permutation arguments.
        // ToDo: This can likely be pre-created in postInit
        static cgScriptArgument::Array vertexArgs(1), pixelArgs(0);
        bool viewSpaceNormals = (driver->getSystemState( cgSystemState::ViewSpaceLighting ) != 0);
        vertexArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &viewSpaceNormals );

        // Select the vertex and pixel shader.
        cgSurfaceShader * shader = mShader.getResource(true);
        if ( !shader->selectVertexShader( _T("transformGeometry"), vertexArgs ) ||
             !shader->selectPixelShader( _T("drawGeometry"), pixelArgs ) )
             return false;

        // Apply pass specific states
        driver->pushDepthStencilState( mGeometryFillDepthState, cgStencilId::Terrain );

    } // End if geometry fill

    // Apply common states
    driver->pushBlendState( cgBlendStateHandle::Null );
    driver->pushRasterizerState( mRasterizerState );

    // Set constants
    driver->setConstantBufferAuto( mFoliageConstants );

    // Proceed with rendering.
    return true;
}

//-----------------------------------------------------------------------------
// Name : end ( ) (Virtual)
/// <summary>
/// Finish rendering clutter of this type.
/// </summary>
//-----------------------------------------------------------------------------
void cgFoliageClutterLayer::end( cgRenderDriver * driver, bool depthFill )
{
    driver->popDepthStencilState();
    driver->popBlendState();
    driver->popRasterizerState();
}

void cgFoliageClutterLayer::setRenderMethod( cgFoliageRenderMethod::Base method )
{
    // Is this a no-op?
    if ( method == mRenderMethod )
        return;

    // Update local value.
    mRenderMethod = method;

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= RenderPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setAnimationMethod( cgFoliageAnimationMethod::Base method )
{
    // Is this a no-op?
    if ( method == mAnimationMethod )
        return;

    // Update local value.
    mAnimationMethod = method;

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= RenderPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setElementSize( const cgRangeF & width, const cgRangeF & height )
{
    // Is this a no-op?
    if ( width == mWidth && height == mHeight )
        return;

    // Update local values.
    mWidth = width;
    mHeight = height;

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= ElementPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setElementSize( cgFloat minWidth, cgFloat maxWidth, cgFloat minHeight, cgFloat maxHeight )
{
    // Is this a no-op?
    if ( minWidth == mWidth.min && maxWidth == mWidth.max &&
         minHeight == mHeight.min && maxHeight == mHeight.max )
        return;

    // Update local values.
    mWidth = cgRangeF( minWidth, maxWidth );
    mHeight = cgRangeF( minHeight, maxHeight );

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= ElementPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setGrowthChance( cgFloat chance )
{
    // Is this a no-op?
    if ( chance == mGrowthChance )
        return;

    // Update local value.
    mGrowthChance = chance;

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= GrowthPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setClusterElements( cgUInt32 elements )
{
    // Clamp
    if ( elements < 1 )
        elements = 1;

    // Is this a no-op?
    if ( elements == mClusterElements )
        return;

    // Update local value.
    mClusterElements = elements;

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= ElementPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setClusterEmbed( cgFloat embed )
{
    // Is this a no-op?
    if ( embed == mClusterEmbed )
        return;

    // Update local value.
    mClusterEmbed = embed;

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= ElementPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setClusterRadius( cgFloat radius )
{
    // Is this a no-op?
    if ( radius == mClusterRadius )
        return;

    // Update local value.
    mClusterRadius = radius;

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= ElementPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setClusterSeparation( cgFloat separation )
{
    // Is this a no-op?
    if ( separation == mClusterSeparation )
        return;

    // Update local value.
    mClusterSeparation = separation;

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= GrowthPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setSeed( cgUInt32 seed )
{
    // Is this a no-op?
    if ( seed == mSeed )
        return;

    // Update local value.
    mSeed = seed;

    // Reinitialize the random number generator.
    mPerlin.setSeed( mSeed );

    // Properties are now dirty
    mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
    mDBDirtyFlags |= GrowthPropertiesDirty;
    serializeLayer();
}

void cgFoliageClutterLayer::setFadeDistance( const cgRangeF & distance )
{
    setFadeDistance( distance.min, distance.max );
}

void cgFoliageClutterLayer::setFadeDistance( cgFloat minimum, cgFloat maximum )
{
    // Clamp
    if ( minimum < 0 )
        minimum = 0;
    if ( maximum < minimum )
        maximum = minimum;

    // Is this a no-op?
    if ( minimum == mFadeDistances.min && maximum == mFadeDistances.max )
        return;

    // Update local value.
    mFadeDistances.min = minimum;
    mFadeDistances.max = maximum;

    // Update constant buffer.
    if ( mFoliageConstants.isValid() )
    {
        _cbFoliageData constants;
        constants.fadeDistance.x = mFadeDistances.min;
        constants.fadeDistance.y = max( mFadeDistances.min + CGE_EPSILON_1CM, mFadeDistances.max );
        constants.fadeDistance.z = -constants.fadeDistance.x;
        constants.fadeDistance.w = 1.0f / (constants.fadeDistance.y - constants.fadeDistance.x);
	    mFoliageConstants->updateBuffer( 0, 0, &constants );
    
    } // End if buffer valid

    // Properties are now dirty
    mDBDirtyFlags |= LODPropertiesDirty;
    serializeLayer();
}

const cgRangeF & cgFoliageClutterLayer::getFadeDistance( ) const
{
    return mFadeDistances;
}

void cgFoliageClutterLayer::getElementSize( cgRangeF & width, cgRangeF & height ) const
{
    width = mWidth;
    height = mHeight;
}

cgFloat cgFoliageClutterLayer::getGrowthChance( ) const
{
    return mGrowthChance;
}

cgUInt32 cgFoliageClutterLayer::getClusterElements( ) const
{
    return mClusterElements;
}

cgFloat cgFoliageClutterLayer::getClusterEmbed( ) const
{
    return mClusterEmbed;
}

cgFloat cgFoliageClutterLayer::getClusterRadius( ) const
{
    return mClusterRadius;
}

cgFloat cgFoliageClutterLayer::getClusterSeparation( ) const
{
    return mClusterSeparation;
}

cgUInt32 cgFoliageClutterLayer::getSeed( ) const
{
    return mSeed;
}

cgFoliageRenderMethod::Base cgFoliageClutterLayer::getRenderMethod( ) const
{
    return mRenderMethod;
}

cgFoliageAnimationMethod::Base cgFoliageClutterLayer::getAnimationMethod( ) const
{
    return mAnimationMethod;
}

///////////////////////////////////////////////////////////////////////////////
// cgFoliageClutterCell Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgFoliageClutterCell () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFoliageClutterCell::cgFoliageClutterCell( cgClutterLayer * layer ) :
    cgClutterCell( layer )
{
    // Initialize variables
    mVertexFormat       = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgFoliageClutterCell () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFoliageClutterCell::~cgFoliageClutterCell( )
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
void cgFoliageClutterCell::dispose( bool disposeBase )
{
    // Release resources.
    mVertices.close();
    mIndices.close();
    mClusterOffsets.clear();

    // Clear variables.
    mVertexFormat       = CG_NULL;

    // Dispose base class.
    if ( disposeBase )
        cgClutterCell::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : updateClusterOffets () (Protected)
/// <summary>
/// Generate the position for each foliage cluster in this cell.
/// </summary>
//-----------------------------------------------------------------------------
void cgFoliageClutterCell::updateClusterOffsets( )
{
    cgFoliageClutterLayer * layer = (cgFoliageClutterLayer*)mLayer;

    // Do nothing if there is no chance of growth
    cgFloat growthChance = layer->getGrowthChance();
    if ( growthChance <= 0 )
    {
        mClusterOffsets.clear();
        return;
    
    } // End if no chance

    // Clear offsets initially.
    mClusterOffsets.clear();

    // Get the properties that we need.
    cgRangeF width, height;
    layer->getElementSize( width, height );
    cgFloat separation = layer->getClusterSeparation();
    cgUInt32 seed = layer->getSeed();
    
    // Separation cannot fall below 5%.
    if ( separation < 0.05f )
        separation = 0.05f;

    // Cell will be subdivided into a regular grid. The size of each grid element
    // is equivalent to the amount of separation requested by the layer in multiples
    // of the maximum cluster element width.
    cgFloat gridSizeX = mCellSize.width / (separation * width.max);
    cgFloat gridSizeY = mCellSize.height / (separation * width.max);
    cgFloat gridDeltaX = mCellSize.width / (floorf(gridSizeX)+1);
    cgFloat gridDeltaY = mCellSize.height / (floorf(gridSizeY)+1);

    // Must be able to fit at least one element in the cell.
    if ( gridDeltaX > mCellSize.width )
        gridDeltaX = mCellSize.width;
    if ( gridDeltaY > mCellSize.height )
        gridDeltaY = mCellSize.height;
    
    // Iterate over the grid
    for ( cgFloat y = mCellOffset.y, endy = mCellOffset.y - (mCellSize.height - CGE_EPSILON_1CM); y > endy; y -= gridDeltaY )
    {
        for ( cgFloat x = mCellOffset.x, endx = mCellOffset.x + (mCellSize.width - CGE_EPSILON_1CM); x < endx; x += gridDeltaX )
        {
            // Compute the random position offsets
            cgFloat ox = noise2D( seed, x, y ) * gridDeltaX;
            cgFloat oy = noise2D( seed, x + (0.25f * gridDeltaX), y ) * -gridDeltaY;

            // Can grow at this location?
            cgFloat layerMask = getGrowthMask(x+ox,y+oy);
            cgFloat growthChanceMasked = growthChance * layerMask;
            if ( layerMask < 0.5f )
                continue;
            if ( growthChanceMasked < 1 )
            {
                if ( noise2D( seed, x + ox, y + oy ) >= growthChanceMasked )
                    continue;
            
            } // End if < 100%

            // We should grow here.
            mClusterOffsets.push_back( cgVector3(x+ox, getDisplacement(x+ox,y+oy), y+oy) );

        } // Next column

    } // Next row
}

//-----------------------------------------------------------------------------
//  Name : updateVertices () (Protected)
/// <summary>
/// Generate vertex buffer ready for the rendering of this foliage cell.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterCell::updateVertices( )
{
    cgFoliageClutterLayer * layer = (cgFoliageClutterLayer*)mLayer;
    cgResourceManager * resources = layer->getMaterial()->getManager();

    // Retrieve random value generators
    cgRandom::NoiseGenerator & perlin = layer->getPerlinGenerator( );
    cgRandom::ParkMiller random;
    cgUInt32 seed = layer->getSeed();
    random.setSeed( seed );

    // Get properties we need.
    cgRangeF width, height;
    layer->getElementSize( width, height );
    cgFloat embed = layer->getClusterEmbed();
    cgFloat radius = layer->getClusterRadius();
    cgUInt32 elements = layer->getClusterElements();

    // Destroy old buffers.
    mVertices.close();
    mIndices.close(); 

    // Retrieve the correct vertex format used for the clutter geometry representation.
    mVertexFormat = cgVertexFormat::formatFromDeclarator(cgFoliageVertex::declarator);

    // Determine new vertex buffer usage flags and length
    cgUInt32 length = (cgUInt32)mClusterOffsets.size() * 4 * elements * mVertexFormat->getStride();
    cgUInt32 usage  = cgBufferUsage::WriteOnly | cgBufferUsage::Dynamic;

    // Create vertex buffer
    if ( !resources->createVertexBuffer( &mVertices, length, usage, mVertexFormat, cgMemoryPool::Default, cgDebugSource() ) )
        return false;

    // Compute element angle delta.
    cgFloat angleDelta = CGEToRadian( ((radius > CGE_EPSILON_1MM) ? 360.0f : 180.0f) / (cgFloat)elements );

    // Update vertex data.
    cgArray<cgFoliageVertex> vertices( (cgUInt32)mClusterOffsets.size() * 4 * elements );
    cgFoliageVertex * vOut = &vertices.front();
    for ( cgUInt32 i = 0; i < mClusterOffsets.size(); ++i )
    {
        cgVector3 offset = mClusterOffsets[i];

        // Compute the overall intensity of this cluster.
        cgFloat intensity = perlin.getValue( offset.x * 2.0f, offset.z * 2.0f ) + 0.6f;
        if ( intensity > 1.0f )
            intensity = 1.0f;
        cgFloat intensity2 = noise2D(seed + 1867, offset.x, offset.z) + 0.8f;
        if ( intensity2 > 1.0f )
            intensity2 = 1.0f;
        intensity *= intensity2;

        // Compute the final width and height for this cluster.
        cgFloat finalWidth = width.min + (width.max-width.min) * noise2D(seed + 1934, offset.x, offset.z);
        cgFloat finalHeight = height.min + (height.max-height.min) * noise2D(seed + 2319, offset.x, offset.z);

        // Generate the cluster elements.
        const cgColorValue c( intensity, intensity, intensity, 1.0f );
        const cgVector3 normal = getNormal( offset.x, offset.z );
        cgFloat angle = CGEToRadian(noise2D(seed + 3124, offset.x, offset.z) * 360.0f);
        for ( cgUInt32 j = 0; j < elements; ++j, angle += angleDelta )
        {
            const cgFloat ac = cosf( angle );
            const cgFloat as = sinf( angle );
            const cgFloat ox = ac * finalWidth * 0.5f;
            const cgFloat oz = as * finalWidth * 0.5f;
            const cgFloat rx = -as * radius;
            const cgFloat rz = ac * radius;
            offset.y = getDisplacement((offset.x + rx) - ox,(offset.z + rz) - oz);
            *vOut++ = cgFoliageVertex( (offset.x + rx) - ox, offset.y - embed, (offset.z + rz) - oz, normal, cgVector2(0,1), c );
            *vOut++ = cgFoliageVertex( (offset.x + rx) - ox, (offset.y - embed) + finalHeight, (offset.z + rz) - oz, normal, cgVector2(0,0), c );
            offset.y = getDisplacement((offset.x + rx) + ox,(offset.z + rz) + oz);
            *vOut++ = cgFoliageVertex( (offset.x + rx) + ox, (offset.y - embed) + finalHeight, (offset.z + rz) + oz, normal, cgVector2(1,0), c );
            *vOut++ = cgFoliageVertex( (offset.x + rx) + ox, offset.y - embed, (offset.z + rz) + oz, normal, cgVector2(1,1), c );
        
        } // Next element
        
    } // Next cluster

    // Populate the hardware index buffer.
    cgVertexBuffer * vertexBuffer = mVertices.getResource(true);
    if ( !vertexBuffer || !vertexBuffer->updateBuffer( 0, 0, &vertices.front() ) )
    {
        mVertices.close();
        return false;

    } // End if update failed
    vertices.clear();

    // Determine index buffer usage flags and length
    length = (cgUInt32)mClusterOffsets.size() * 6 * elements * sizeof(cgUInt32);
    usage  = cgBufferUsage::WriteOnly;

    // Create index buffer
    if ( !resources->createIndexBuffer( &mIndices, length, usage, cgBufferFormat::Index32, cgMemoryPool::Managed, cgDebugSource() ) )
        return false;
    
    // Build the data
    cgUInt32Array indices( (cgUInt32)mClusterOffsets.size() * 6 * elements );
    cgUInt32 * iOut = &indices.front();
    for ( cgUInt32 i = 0, c = 0; i < mClusterOffsets.size() * elements; ++i )
    {
        // Build indices for two triangles
        *iOut++ = c;
        *iOut++ = c + 1;
        *iOut++ = c + 2;
        *iOut++ = c;
        *iOut++ = c + 2;
        *iOut++ = c + 3;

        // Move along by 4 vertices (quad)
        c += 4;
        
    } // Next polygon

    // Populate the hardware index buffer.
    cgIndexBuffer * indexBuffer = mIndices.getResource(true);
    if ( !indexBuffer || !indexBuffer->updateBuffer( 0, 0, &indices.front() ) )
    {
        mIndices.close();
        return false;

    } // End if update failed
    indices.clear();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : shouldRender () (Virtual)
/// <summary>
/// Determine if the contents of this cell should be rendered.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterCell::shouldRender( cgCameraNode * camera ) const
{
    cgFoliageClutterLayer * layer = (cgFoliageClutterLayer*)mLayer;
    const cgVector3 cameraPos = camera->getPosition();

    // Check to see if we need to render this cell.
    cgBoundingBox bounds( cgVector3( mCellOffset.x, cameraPos.y - CGE_EPSILON_1M, mCellOffset.y - mCellSize.height ),
                          cgVector3( mCellOffset.x + mCellSize.width, cameraPos.y + CGE_EPSILON_1M, mCellOffset.y ) );
    if ( !bounds.containsPoint( cameraPos, CGE_EPSILON_1CM ) )
    {
        // Closest point on bounds is out of range?
        if ( cgVector3::length(bounds.closestPoint(cameraPos) - cameraPos) > layer->getFadeDistance().max )
            return false;
    
    } // End if !contained

    // We should render
    return true;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Render the contents of the clutter cell.
/// </summary>
//-----------------------------------------------------------------------------
void cgFoliageClutterCell::render( cgRenderDriver * driver, bool depthFill )
{
    // Does the layer need refreshing
    cgFoliageClutterLayer * layer = (cgFoliageClutterLayer*)mLayer;
    if ( layer->isDirtySince( mLastRefreshFrame ) )
    {
        if ( !refresh() )
            return;
    
    } // End if properties dirty

    // Update vertices if the data is lost.
    if ( mVertices.isResourceLost() || mIndices.isResourceLost() )
        updateVertices();

    // Render
    driver->setStreamSource( 0, mVertices );
    driver->setIndices( mIndices );
    driver->setVertexFormat( mVertexFormat );
    driver->drawIndexedPrimitive( cgPrimitiveType::TriangleList, 0, 0, mClusterOffsets.size() * 4 * layer->getClusterElements(), 0, mClusterOffsets.size() * 2 * layer->getClusterElements() );
}

//-----------------------------------------------------------------------------
//  Name : refresh ()
/// <summary>
/// Refresh the renderable data buffers.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFoliageClutterCell::refresh( )
{
    updateClusterOffsets();
    if ( updateVertices() )
    {
        mLastRefreshFrame = cgTimer::getInstance()->getFrameCounter();
        return true;
    
    } // End if success
    return false;
}