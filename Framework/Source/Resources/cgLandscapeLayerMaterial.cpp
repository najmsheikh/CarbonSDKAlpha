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
// Name : cgLandscapeLayerMaterial.cpp                                       //
//                                                                           //
// Desc : Provides classes which outline layer material data for use with    //
//        the the scene landscape system. Such materials can either be       //
//        manually applied to the terrain by the user with the landscape     //
//        layer painting tools, or applied procedurally by creating a        //
//        procedural layer material.                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgLandscapeLayerMaterial Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgLandscapeLayerMaterial.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgSampler.h>
#include <System/cgExceptions.h>

// Preview rendering
#include <System/cgImage.h>
#include <Resources/cgRenderTarget.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgLandscapeLayerMaterial::mInsertMaterial;
cgWorldQuery cgLandscapeLayerMaterial::mUpdateName;
cgWorldQuery cgLandscapeLayerMaterial::mUpdateSamplers;
cgWorldQuery cgLandscapeLayerMaterial::mUpdateBaseScale;
cgWorldQuery cgLandscapeLayerMaterial::mUpdateScale;
cgWorldQuery cgLandscapeLayerMaterial::mUpdateOffset;
cgWorldQuery cgLandscapeLayerMaterial::mUpdateAngle;
cgWorldQuery cgLandscapeLayerMaterial::mUpdateTilingReduction;
cgWorldQuery cgLandscapeLayerMaterial::mUpdatePreview;
cgWorldQuery cgLandscapeLayerMaterial::mLoadMaterial;

///////////////////////////////////////////////////////////////////////////////
// cgLandscapeLayerMaterial Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLandscapeLayerMaterial () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeLayerMaterial::cgLandscapeLayerMaterial( cgUInt32 referenceId, cgWorld * world ) : cgMaterial( referenceId, world )
{
    // Initialize variables
    mColorSampler    = CG_NULL;
    mNormalSampler   = CG_NULL;
    mScale           = cgVector2(128,128);
    mBaseScale       = cgVector2(0.225f,0.205f);
    mOffset          = cgVector2(0,0);
    mAngle           = 0.0f;
    mTilingReduction = true;

    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::LandscapeLayer'.
    mComponentTypeId = (cgUInt32)cgMaterialType::LandscapeLayer;
}

//-----------------------------------------------------------------------------
//  Name : cgLandscapeLayerMaterial () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeLayerMaterial::cgLandscapeLayerMaterial( cgUInt32 referenceId, cgWorld * world, cgUInt32 sourceRefId ) : cgMaterial( referenceId, world, sourceRefId )
{
    // Initialize variables
    mColorSampler    = CG_NULL;
    mNormalSampler   = CG_NULL;
    mScale           = cgVector2(128,128);
    mBaseScale       = cgVector2(0.225f,0.205f);
    mOffset          = cgVector2(0,0);
    mAngle           = 0.0f;
    mTilingReduction = true;

    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::LandscapeLayer'.
    mComponentTypeId  = (cgUInt32)cgMaterialType::LandscapeLayer;
}

//-----------------------------------------------------------------------------
//  Name : cgLandscapeLayerMaterial () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeLayerMaterial::cgLandscapeLayerMaterial( cgUInt32 referenceId, cgWorld * world, cgLandscapeLayerMaterial * init ) : cgMaterial( referenceId, world, init )
{
    // Initialize variables to sensible defaults.
    mColorSampler    = CG_NULL;
    mNormalSampler   = CG_NULL;
    
    // Material data.
    mScale           = init->mScale;
    mBaseScale       = init->mBaseScale;
    mOffset          = init->mOffset;
    mAngle           = init->mAngle;
    mTilingReduction = init->mTilingReduction;
    mSurfaceShader   = init->mSurfaceShader;

    // Duplicate sampler data
    mColorSampler    = mManager->cloneSampler( world, isInternalReference(), init->mColorSampler );
    mNormalSampler   = mManager->cloneSampler( world, isInternalReference(), init->mNormalSampler );
    
    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::LandscapeLayer'.
    mComponentTypeId = (cgUInt32)cgMaterialType::LandscapeLayer;
}

//-----------------------------------------------------------------------------
//  Name : ~cgLandscapeLayerMaterial () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeLayerMaterial::~cgLandscapeLayerMaterial( )
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
void cgLandscapeLayerMaterial::dispose( bool disposeBase )
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
bool cgLandscapeLayerMaterial::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_LandscapeLayerMaterial )
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
cgString cgLandscapeLayerMaterial::getDatabaseTable( ) const
{
    return _T("Materials::LandscapeLayer");
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeLayerMaterial::prepareQueries()
{
    // Any database supplied?
    if ( mWorld == CG_NULL )
        return;

    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertMaterial.isPrepared() == false )
            mInsertMaterial.prepare( mWorld, _T("INSERT INTO 'Materials::LandscapeLayer' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14)"), true );  
        if ( mUpdateName.isPrepared() == false )
            mUpdateName.prepare( mWorld, _T("UPDATE 'Materials::LandscapeLayer' SET Name=?1 WHERE RefId=?2"), true );  
        if ( mUpdateSamplers.isPrepared() == false )
            mUpdateSamplers.prepare( mWorld, _T("UPDATE 'Materials::LandscapeLayer' SET ColorSamplerId=?1, NormalSamplerId=?2 WHERE RefId=?3"), true );  
        if ( mUpdateBaseScale.isPrepared() == false )
            mUpdateBaseScale.prepare( mWorld, _T("UPDATE 'Materials::LandscapeLayer' SET BaseScaleU=?1, BaseScaleV=?2 WHERE RefId=?3"), true );  
        if ( mUpdateScale.isPrepared() == false )
            mUpdateScale.prepare( mWorld, _T("UPDATE 'Materials::LandscapeLayer' SET ScaleU=?1, ScaleV=?2 WHERE RefId=?3"), true );  
        if ( mUpdateOffset.isPrepared() == false )
            mUpdateOffset.prepare( mWorld, _T("UPDATE 'Materials::LandscapeLayer' SET OffsetU=?1, OffsetV=?2 WHERE RefId=?3"), true );  
        if ( mUpdateAngle.isPrepared() == false )
            mUpdateAngle.prepare( mWorld, _T("UPDATE 'Materials::LandscapeLayer' SET Rotation=?1 WHERE RefId=?2"), true );  
        if ( mUpdateTilingReduction.isPrepared() == false )
            mUpdateTilingReduction.prepare( mWorld, _T("UPDATE 'Materials::LandscapeLayer' SET TilingReduction=?1 WHERE RefId=?2"), true );  
        if ( mUpdatePreview.isPrepared() == false )
            mUpdatePreview.prepare( mWorld, _T("UPDATE 'Materials::LandscapeLayer' SET PreviewImage=?1 WHERE RefId=?2"), true ); 
    
    } // End if sandbox

    // Read queries
    if ( mLoadMaterial.isPrepared() == false )
        mLoadMaterial.prepare( mWorld, _T("SELECT * FROM 'Materials::LandscapeLayer' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : serializeMaterial() (Protected)
/// <summary>
/// Write or update the material data stored in the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeLayerMaterial::serializeMaterial( )
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
        if ( !mMaterialSerialized  )
        {
            // Material entry does not exist at all at this stage so insert it.
            mInsertMaterial.bindParameter( 1, mReferenceId );
            mInsertMaterial.bindParameter( 2, mName );

            // Material samplers
            mInsertMaterial.bindParameter( 3, (mColorSampler) ? mColorSampler->getReferenceId() : 0 );
            mInsertMaterial.bindParameter( 4, (mNormalSampler) ? mNormalSampler->getReferenceId() : 0 );
            
            // Transformations
            mInsertMaterial.bindParameter( 5, mScale.x );
            mInsertMaterial.bindParameter( 6, mScale.y );
            mInsertMaterial.bindParameter( 7, mBaseScale.x );
            mInsertMaterial.bindParameter( 8, mBaseScale.y );
            mInsertMaterial.bindParameter( 9, mOffset.x );
            mInsertMaterial.bindParameter( 10, mOffset.y );
            mInsertMaterial.bindParameter( 11, mAngle );

            // Quality
            mInsertMaterial.bindParameter( 12, mTilingReduction );

            // Preview Image
            mInsertMaterial.bindParameter( 13, CG_NULL, 0 );

            // Database ref count (just in case it has already been adjusted)
            mInsertMaterial.bindParameter( 14, mSoftRefCount );

            // Process!
            if ( !mInsertMaterial.step( true )  )
            {
                cgString error;
                mInsertMaterial.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to insert data for landscape layer material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // All other material properties have been serialized
            mDBDirtyFlags = 0;
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
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update name data for landscape layer material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~NameDirty;

        } // End if name dirty

        // Serialize samplers
        if ( mDBDirtyFlags & SamplersDirty )
        {
            // Update samplers.
            mUpdateSamplers.bindParameter( 1, (mColorSampler) ? mColorSampler->getReferenceId() : 0 );
            mUpdateSamplers.bindParameter( 2, (mNormalSampler) ? mNormalSampler->getReferenceId() : 0 );
            mUpdateSamplers.bindParameter( 3, mReferenceId );
            
            // Process!
            if ( mUpdateSamplers.step( true ) == false )
            {
                cgString error;
                mUpdateSamplers.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update sampler data for landscape layer material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~SamplersDirty;

        } // End if samplers dirty

        // Serialize scale
        if ( mDBDirtyFlags & ScaleDirty )
        {
            // Update samplers.
            mUpdateScale.bindParameter( 1, mScale.x );
            mUpdateScale.bindParameter( 2, mScale.y );
            mUpdateScale.bindParameter( 3, mReferenceId );
            
            // Process!
            if ( mUpdateScale.step( true ) == false )
            {
                cgString error;
                mUpdateScale.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update scale data for landscape layer material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~ScaleDirty;

        } // End if scale dirty

        // Serialize base scale
        if ( mDBDirtyFlags & BaseScaleDirty )
        {
            // Update samplers.
            mUpdateBaseScale.bindParameter( 1, mBaseScale.x );
            mUpdateBaseScale.bindParameter( 2, mBaseScale.y );
            mUpdateBaseScale.bindParameter( 3, mReferenceId );
            
            // Process!
            if ( mUpdateBaseScale.step( true ) == false )
            {
                cgString error;
                mUpdateBaseScale.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update base scale data for landscape layer material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~BaseScaleDirty;

        } // End if base scale dirty

        // Serialize offset
        if ( mDBDirtyFlags & OffsetDirty )
        {
            // Update samplers.
            mUpdateOffset.bindParameter( 1, mOffset.x );
            mUpdateOffset.bindParameter( 2, mOffset.y );
            mUpdateOffset.bindParameter( 3, mReferenceId );
            
            // Process!
            if ( mUpdateOffset.step( true ) == false )
            {
                cgString error;
                mUpdateOffset.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update offset data for landscape layer material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~OffsetDirty;

        } // End if offset dirty

        // Serialize rotation angle
        if ( mDBDirtyFlags & AngleDirty )
        {
            // Update samplers.
            mUpdateAngle.bindParameter( 1, mAngle );
            mUpdateAngle.bindParameter( 2, mReferenceId );
            
            // Process!
            if ( mUpdateAngle.step( true ) == false )
            {
                cgString error;
                mUpdateAngle.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update rotation angle data for landscape layer material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~AngleDirty;

        } // End if angle dirty

        // Serialize tiling reduction
        if ( mDBDirtyFlags & TilingReductionDirty )
        {
            // Update samplers.
            mUpdateTilingReduction.bindParameter( 1, mTilingReduction );
            mUpdateTilingReduction.bindParameter( 2, mReferenceId );
            
            // Process!
            if ( mUpdateTilingReduction.step( true ) == false )
            {
                cgString error;
                mUpdateTilingReduction.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update tiling reduction data for landscape layer material resource '0x%x'. Error: %s"), mReferenceId, error.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~TilingReductionDirty;

        } // End if tiling reduction dirty

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
bool cgLandscapeLayerMaterial::loadMaterial( cgUInt32 sourceRefId, cgResourceManager * resourceManager /* = CG_NULL */ )
{
    // Call base class implementation first.
    if ( cgMaterial::loadMaterial( sourceRefId, resourceManager ) == false )
        return false;

    // Handle exceptions
    try
    {
        // Utilize the landscape rendering shader.
        if ( !mSurfaceShader.isValid() )
        {
            if ( !mManager->createSurfaceShader( &mSurfaceShader, _T("sys://Shaders/Landscape.sh"), 0, cgDebugSource() ) )
                throw cgExceptions::ResultException( _T("Failed to instantiate landscape material surface shader. See previous output for further details."), cgDebugSource() );
        
        } // End if no shader loaded

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
        // the 'SetName()' method as this will trigger premature serialization).
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

        // Sampler data
        cgUInt32 colorSamplerId = 0, normalSamplerId = 0;
        mLoadMaterial.getColumn( _T("ColorSamplerId"), colorSamplerId );
        mLoadMaterial.getColumn( _T("NormalSamplerId"), normalSamplerId );
        
        // ToDo: what resource loading flags should be specified?
        // Load the referenced samplers.
        mColorSampler = mManager->loadSampler( mWorld, mSurfaceShader, colorSamplerId, isInternalReference() );
        if ( mColorSampler != CG_NULL )
        {
            // Add the material as a reference holder (just reconnect,
            // do not adjust the database ref count).
            mColorSampler->addReference( this, true );
        
        } // End if valid color
        mNormalSampler = mManager->loadSampler( mWorld, mSurfaceShader, normalSamplerId, isInternalReference() );
        if ( mNormalSampler != CG_NULL )
        {
            // Add the material as a reference holder (just reconnect,
            // do not adjust the database ref count).
            mNormalSampler->addReference( this, true );
        
        } // End if valid color
       
        // Transformations
        mLoadMaterial.getColumn( _T("ScaleU"), mScale.x );
        mLoadMaterial.getColumn( _T("ScaleV"), mScale.y );
        mLoadMaterial.getColumn( _T("BaseScaleU"), mBaseScale.x );
        mLoadMaterial.getColumn( _T("BaseScaleV"), mBaseScale.y );
        mLoadMaterial.getColumn( _T("OffsetU"), mOffset.x );
        mLoadMaterial.getColumn( _T("OffsetV"), mOffset.y );
        mLoadMaterial.getColumn( _T("Rotation"), mAngle );

        // Quality
        mLoadMaterial.getColumn( _T("TilingReduction"), mTilingReduction );
        
        // Preview Image
        // mLoadMaterial.getColumn( 22, CG_NULL, 0 ); // ToDo: 9999

        // We're done with the data from the material query
        mLoadMaterial.reset();

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
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// Unload the underlying resource data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeLayerMaterial::unloadResource( )
{
    // Release all samplers we own (disconnect).
    if ( mColorSampler )
        mColorSampler->removeReference(this,true);
    if ( mNormalSampler )
        mNormalSampler->removeReference(this,true);
    mColorSampler  = CG_NULL;
    mNormalSampler = CG_NULL;
    
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
bool cgLandscapeLayerMaterial::buildPreviewImage( cgScene * scene, cgRenderView * view )
{
    // Has a valid color sampler?
    if ( !mColorSampler || !mColorSampler->isTextureValid() )
        return false;

    // Create the image ready to store data.
    cgSize size = view->getSize();
    if ( mPreviewImage == CG_NULL )
        mPreviewImage = cgImage::createInstance();
    cgToDo( "DX11", "Texture format needs considering more given lack of ARGB in DX10+ (Use GetBestFormat()?)" );
    if ( mPreviewImage->createImage( size.width, size.height, cgBufferFormat::B8G8R8A8, true ) == false )
        return false;

    // Copy the texture into the preview image.
    cgTexture * texture = mColorSampler->getTexture().getResource(true);
    if ( !texture->getImageData( *mPreviewImage ) )
    {
        delete mPreviewImage;
        mPreviewImage = CG_NULL;
        return false;
    
    } // End if failed
    
    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeLayerMaterial::onComponentDeleted( )
{
    // Remove our physical references to any child samplers. Full database update 
    // and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    if ( mColorSampler != CG_NULL )
        mColorSampler->removeReference(this,isInternalReference());
    if ( mNormalSampler != CG_NULL )
        mNormalSampler->removeReference(this,isInternalReference());
    mColorSampler  = CG_NULL;
    mNormalSampler = CG_NULL;
    
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
cgInt cgLandscapeLayerMaterial::compare( const cgMaterial & material ) const
{
    cgFloat f = 0;
    cgInt   i = 0;

    // Call base class implementation first (checks effect file and material class).
    if ( i = cgMaterial::compare( material ) != 0 )
        return i;

    // Rapid numeric / handle only tests
    const cgLandscapeLayerMaterial & m = (const cgLandscapeLayerMaterial&)material;

    // Compare samplers
    if ( !mColorSampler && m.mColorSampler )
        return -1;
    else if ( mColorSampler && !m.mColorSampler )
        return 1;
    else if ( !mColorSampler && !m.mColorSampler )
        i = 0;
    else
        i = mColorSampler->compare( *m.mColorSampler );
    if ( i != 0 ) return (i < 0) ? -1 : 1;
    
    if ( !mNormalSampler && m.mNormalSampler )
        return -1;
    else if ( mNormalSampler && !m.mNormalSampler )
        return 1;
    else if ( !mNormalSampler && !m.mNormalSampler )
        i = 0;
    else
        i = mNormalSampler->compare( *m.mNormalSampler );
    if ( i != 0 ) return (i < 0) ? -1 : 1;

    // Test material properties / Base Scale first
    f = mBaseScale.x - m.mBaseScale.x;
    if ( fabsf( f ) > CGE_EPSILON ) return (f < 0) ? -1 : 1;
    f = mBaseScale.y - m.mBaseScale.y;
    if ( fabsf( f ) > CGE_EPSILON ) return (f < 0) ? -1 : 1;

    // Scale
    f = mScale.x - m.mScale.x;
    if ( fabsf( f ) > CGE_EPSILON ) return (f < 0) ? -1 : 1;
    f = mScale.y - m.mScale.y;
    if ( fabsf( f ) > CGE_EPSILON ) return (f < 0) ? -1 : 1;

    // Offset
    f = mOffset.x - m.mOffset.x;
    if ( fabsf( f ) > CGE_EPSILON ) return (f < 0) ? -1 : 1;
    f = mOffset.y - m.mOffset.y;
    if ( fabsf( f ) > CGE_EPSILON ) return (f < 0) ? -1 : 1;

    // Rotation Angle
    f = mAngle - m.mAngle;
    if ( fabsf( f ) > CGE_EPSILON ) return (f < 0) ? -1 : 1;    

    // Tiling reduction
    i = (cgInt)mTilingReduction - (cgInt)m.mTilingReduction;
    if ( i != 0 ) return (i < 0) ? -1 : 1;
    
    // Totally equal
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : setSurfaceShader ()
/// <summary>
/// Load the surface shader file specified for rendering with this material.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeLayerMaterial::setSurfaceShader( const cgSurfaceShaderHandle & shader )
{
    // No shader can ever be set for this material, it uses the
    // system provided 'Landscape.sh'.
    return false;
}

//-----------------------------------------------------------------------------
// Name : loadColorSampler()
/// <summary>Load the specified texture into the color sampler.</summary>
//-----------------------------------------------------------------------------
bool cgLandscapeLayerMaterial::loadColorSampler( cgInputStream stream )
{
    // Create a new sampler of the required type (internal or serialized 
    // depending on the properties of this parent material).
    cgSampler * sampler = CG_NULL;
    if ( !isInternalReference() )
        sampler = mManager->createSampler( mWorld, _T("Diffuse") );
    else
        sampler = mManager->createSampler( _T("Diffuse") );
    
    // Load the referenced texture. We'll ignore the actual loading
    // if the source was invalid. This allows the caller to specify
    // an empty stream simply to create a sampler.
    if ( stream.sourceExists() == true && sampler->loadTexture( stream, 0, cgDebugSource() ) == false )
        return false;

    // Set the sampler
    setColorSampler( sampler );
    return true;
}

//-----------------------------------------------------------------------------
// Name : loadNormalSampler()
/// <summary>Load the specified texture into the normal sampler.</summary>
//-----------------------------------------------------------------------------
bool cgLandscapeLayerMaterial::loadNormalSampler( cgInputStream stream )
{
    // Create a new sampler of the required type (internal or serialized 
    // depending on the properties of this parent material).
    cgSampler * sampler = CG_NULL;
    if ( !isInternalReference() )
        sampler = mManager->createSampler( mWorld, _T("Normal") );
    else
        sampler = mManager->createSampler( _T("Normal") );
    
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
cgSampler * cgLandscapeLayerMaterial::getColorSampler( ) const
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
void cgLandscapeLayerMaterial::setColorSampler( cgSampler * sampler )
{
    // Is this a no-op?
    if ( sampler == mColorSampler )
        return;

    // Clean up any existing sampler.
    if ( mColorSampler != CG_NULL )
        mColorSampler->removeReference( this, isInternalReference() );

    // Add this material as a reference holder to the new sampler.
    if ( sampler != CG_NULL )
        sampler->addReference( this, isInternalReference() );

    // Replace current sampler.
    mColorSampler = sampler;

    // Samplers are now dirty
    mDBDirtyFlags |= SamplersDirty;
    serializeMaterial();
}

//-----------------------------------------------------------------------------
// Name : getNormalSampler ( )
/// <summary>
/// Get the current normal sampler to apply when rendering layer that
/// utilize this type.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgLandscapeLayerMaterial::getNormalSampler( ) const
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
void cgLandscapeLayerMaterial::setNormalSampler( cgSampler * sampler )
{
    // Is this a no-op?
    if ( sampler == mNormalSampler )
        return;

    // Clean up any existing sampler.
    if ( mNormalSampler != CG_NULL )
        mNormalSampler->removeReference( this, isInternalReference() );

    // Add this material as a reference holder to the new sampler.
    if ( sampler != CG_NULL )
        sampler->addReference( this, isInternalReference() );

    // Replace current sampler.
    mNormalSampler = sampler;

    // Samplers are now dirty
    mDBDirtyFlags |= SamplersDirty;
    serializeMaterial();
}

//-----------------------------------------------------------------------------
// Name : getScale ( )
/// <summary>
/// Get the planar mapped texture coordinate scale for layers of this type.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector2 & cgLandscapeLayerMaterial::getScale( ) const
{
    return mScale;
}

//-----------------------------------------------------------------------------
// Name : setScale ( )
/// <summary>
/// Set the planar mapped texture coordinate scale for layers of this type.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeLayerMaterial::setScale( const cgVector2 & value )
{
    // Is this a no-op?
    if ( value == mScale )
        return;

    // Update internal member
    mScale = value;

    // Scale is now dirty
    mDBDirtyFlags |= ScaleDirty;
    serializeMaterial();
}

//-----------------------------------------------------------------------------
// Name : getBaseScale ( )
/// <summary>
/// Get the planar mapped texture coordinate scale for any second
/// base application of this layer's texture.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector2 & cgLandscapeLayerMaterial::getBaseScale( ) const
{
    return mBaseScale;
}

//-----------------------------------------------------------------------------
// Name : setBaseScale ( )
/// <summary>
/// Set the planar mapped texture coordinate scale for any second
/// base application of this layer's texture.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeLayerMaterial::setBaseScale( const cgVector2 & value )
{
    // Is this a no-op?
    if ( value == mBaseScale )
        return;

    // Update internal member
    mBaseScale = value;

    // Base scale is now dirty
    mDBDirtyFlags |= BaseScaleDirty;
    serializeMaterial();
}

//-----------------------------------------------------------------------------
// Name : getOffset ( )
/// <summary>
/// Get the planar mapped texture coordinate offset / translation for
/// layers of this type.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector2 & cgLandscapeLayerMaterial::getOffset( ) const
{
    return mOffset;
}

//-----------------------------------------------------------------------------
// Name : setOffset ( )
/// <summary>
/// Set the planar mapped texture coordinate offset / translation for
/// layers of this type.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeLayerMaterial::setOffset( const cgVector2 & value )
{
    // Is this a no-op?
    if ( value == mOffset )
        return;

    // Update internal member
    mOffset = value;

    // Offset is now dirty
    mDBDirtyFlags |= OffsetDirty;
    serializeMaterial();
}

//-----------------------------------------------------------------------------
// Name : getAngle ( )
/// <summary>
/// Get the planar mapped texture coordinate rotation angle (in degrees)
/// for layers of this type.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLandscapeLayerMaterial::getAngle( ) const
{
    return mAngle;
}

//-----------------------------------------------------------------------------
// Name : setAngle ( )
/// <summary>
/// Set the planar mapped texture coordinate rotation angle (in degrees)
/// for layers of this type.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeLayerMaterial::setAngle( cgFloat value )
{
    // Is this a no-op?
    if ( value == mAngle )
        return;

    // Update internal member
    mAngle = value;

    // Angle is now dirty
    mDBDirtyFlags |= AngleDirty;
    serializeMaterial();
}

//-----------------------------------------------------------------------------
// Name : getTilingReduction ( )
/// <summary>
/// Get the flag that indicates whether or not tiling reduction should be
/// applied for layers of this type.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeLayerMaterial::getTilingReduction( ) const
{
    return mTilingReduction;
}

//-----------------------------------------------------------------------------
// Name : enableTilingReduction ( )
/// <summary>
/// Set the flag that indicates whether or not tiling reduction should be
/// applied for layers of this type.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeLayerMaterial::enableTilingReduction( bool value )
{
    // Is this a no-op?
    if ( value == mTilingReduction )
        return;

    // Update internal member
    mTilingReduction = value;

    // Tiling reduction is now dirty
    mDBDirtyFlags |= TilingReductionDirty;
    serializeMaterial();
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// Apply the necessary material data to the device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeLayerMaterial::apply( cgRenderDriver * driver )
{
    /*// Apply material properties
    driver->setMaterialTerms( m_MaterialTerms );
        
    // Apply the material's samplers
    applySamplers();

    // Apply material's shader constants
    applyShaderConstants();*/

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
void cgLandscapeLayerMaterial::reset( cgRenderDriver * driver )
{
    //restoreShaderConstants();
}

/*//-----------------------------------------------------------------------------
//  Name : ApplySamplers ()
/// <summary>
/// Apply the necessary samplers to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeLayerMaterial::applySamplers( )
{
    // Apply each of our managed samplers.
    for ( size_t i = 0; i < m_Samplers->size(); ++i )
        m_Samplers[i]->apply();

    // A default diffuse sampler is always required. If no diffuse sampler / texture 
    // was available, use the default one available from the resource manager.
    if ( m_pExplicitDiffuseSampler == CG_NULL || m_pExplicitDiffuseSampler->isTextureValid() == false )
    {
        // Apply the default diffuse sampler.
        mManager->GetDefaultSampler( cgResourceManager::DefaultDiffuseSampler )->apply();

    } // End if no diffuse
    
    // We also need to do the same for the normal sampler.
    if ( m_pExplicitNormalSampler == CG_NULL || m_pExplicitNormalSampler->isTextureValid() == false )
    {
        // Apply the default diffuse sampler.
        mManager->GetDefaultSampler( cgResourceManager::DefaultNormalSampler )->apply();
        
    } // End if no normal
}*/