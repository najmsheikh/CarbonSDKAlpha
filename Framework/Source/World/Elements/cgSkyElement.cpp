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
// Name : cgSkyElement.cpp                                                   //
//                                                                           //
// Desc : Class that provides configuration and management of scene          //
//        sky rendering data, exposed as a scene element type. This provides //
//        the integration between the application (such as the editing       //
//        environment) and the relevant components of the scene renderer.    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSkyElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Elements/cgSkyElement.h>
#include <Resources/cgTexture.h>
#include <Rendering/cgSampler.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgSkyElement::mInsertSkyElement;
cgWorldQuery cgSkyElement::mDeleteSkyElement;
cgWorldQuery cgSkyElement::mUpdateSkyProperties;
cgWorldQuery cgSkyElement::mLoadSkyElement;

///////////////////////////////////////////////////////////////////////////////
// cgSkyElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSkyElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSkyElement::cgSkyElement( cgUInt32 referenceId, cgScene * scene ) : cgSceneElement( referenceId, scene )
{
    // Initialize variables to sensible defaults
    mType           = cgSkyElementType::SkyBox;
    mBaseSampler    = CG_NULL;
    mBaseHDRScale   = 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSkyElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSkyElement::~cgSkyElement()
{
    // Clean up object resources.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgSkyElement::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release samplers we own (disconnect).
    if ( mBaseSampler )
        mBaseSampler->removeReference(this,true);

    // Clear variables
    mBaseSampler    = CG_NULL;
    mType           = cgSkyElementType::SkyBox;
    mBaseHDRScale   = 1.0f;
    
    // Dispose base class if requested.
    if ( disposeBase == true )
        cgSceneElement::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a scene element of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneElement * cgSkyElement::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgSkyElement( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkyElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SkyElement )
        return true;

    // Supported by base?
    return cgSceneElement::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgSkyElement::getDatabaseTable( ) const
{
    return _T("SceneElements::Sky");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkyElement::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object sub-element.
    if ( !insertComponentData( ) )
        return false;

    // Call base class implementation last.
    return cgSceneElement::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkyElement::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("SkyElement::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertSkyElement.bindParameter( 1, mReferenceId );
        mInsertSkyElement.bindParameter( 2, (cgUInt32)mType );
        mInsertSkyElement.bindParameter( 3, (mBaseSampler) ? mBaseSampler->getReferenceId() : 0 );
        mInsertSkyElement.bindParameter( 4, mBaseHDRScale );
        mInsertSkyElement.bindParameter( 5, mSoftRefCount );
        
        // Execute
        if ( !mInsertSkyElement.step( true ) )
        {
            cgString strError;
            mInsertSkyElement.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for sky scene element '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("SkyElement::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("SkyElement::insertComponentData") );

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
bool cgSkyElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the sky element data
    prepareQueries();
    mLoadSkyElement.bindParameter( 1, e->sourceRefId );
    if ( !mLoadSkyElement.step( ) || !mLoadSkyElement.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadSkyElement.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for sky scene element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for sky scene element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadSkyElement.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadSkyElement;

    // Load parameters
    cgUInt32 skyElementType = 0, baseSamplerId;
    mLoadSkyElement.getColumn( _T("Type"), skyElementType );
    mLoadSkyElement.getColumn( _T("BaseSamplerId"), baseSamplerId );
    mLoadSkyElement.getColumn( _T("BaseHDRScalar"), mBaseHDRScale );
    mType = (cgSkyElementType::Base)skyElementType;

    // Load the referenced sampler if valid.
    if ( baseSamplerId )
    {
        // Load the referenced sampler.
        cgResourceManager * resources = mParentScene->getResourceManager();
        mBaseSampler = resources->loadSampler( mWorld, cgSurfaceShaderHandle::Null, baseSamplerId, isInternalReference() );
        if ( mBaseSampler )
        {
            // Add the element as a reference holder (just reconnect,
            // do not adjust the database ref count).
            mBaseSampler->addReference( this, true );

        } // End if loaded

    } // End if has sampler

    // Call base class implementation to read remaining data.
    if ( !cgSceneElement::onComponentLoading( e ) )
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
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkyElement::onComponentDeleted( )
{
    // Remove our physical references to any child samplers. Full database update 
    // and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    if ( mBaseSampler )
        mBaseSampler->removeReference(this,isInternalReference());
    mBaseSampler = CG_NULL;
    
    // Call base class implementation last.
    cgSceneElement::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkyElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertSkyElement.isPrepared() )
            mInsertSkyElement.prepare( mWorld, _T("INSERT INTO 'SceneElements::Sky' VALUES(?1,?2,?3,?4,?5)"), true );
        if ( !mUpdateSkyProperties.isPrepared() )
            mUpdateSkyProperties.prepare( mWorld, _T("UPDATE 'SceneElements::Sky' SET Type=?1, BaseSamplerId=?2, BaseHDRScalar=?3 WHERE RefId=?4"), true );
        if ( !mDeleteSkyElement.isPrepared() )
            mDeleteSkyElement.prepare( mWorld, _T("DELETE FROM 'SceneElements::Sky' WHERE RefId=?1"), true );
        
    } // End if sandbox

    // Read queries
    if ( !mLoadSkyElement.isPrepared() )
        mLoadSkyElement.prepare( mWorld, _T("SELECT * FROM 'SceneElements::Sky' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : setBaseSampler ( )
/// <summary>
/// Set the base texture and sampling details to be used (if any) during 
/// rendering of this sky element. For instance, for a skybox, a sampler
/// referencing a cube map should be supplied.
/// Note: This method will clone the supplied sampler, and the caller is still
/// responsible for releasing the sampler they supply if necessary.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkyElement::setBaseSampler( cgSampler * sourceSampler )
{
    // Sampler being removed?
    if ( !sourceSampler )
    {
        // Update sampler id (set to 0 -- no sampler).
        if ( shouldSerialize() )
        {
            prepareQueries();
            mUpdateSkyProperties.bindParameter( 1, (cgUInt32)mType );
            mUpdateSkyProperties.bindParameter( 2, 0 );
            mUpdateSkyProperties.bindParameter( 3, mBaseHDRScale );
            mUpdateSkyProperties.bindParameter( 4, mReferenceId );
            
            // Execute
            if ( !mUpdateSkyProperties.step( true ) )
            {
                cgString strError;
                mUpdateSkyProperties.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update properties for sky element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
                return false;
            
            } // End if failed
        
        } // End if serialize

        // Remove our physical references to any child samplers. Full database update 
        // and potentially removal should be allowed to occur (i.e. a full 
        // de-reference rather than a simple disconnect) in this case.
        if ( mBaseSampler )
            mBaseSampler->removeReference(this,isInternalReference());
        mBaseSampler = CG_NULL;
    
    } // End if removed
    else
    {
        cgTextureHandle textureHandle = sourceSampler->getTexture();
        cgTexture * texture = textureHandle.getResource(false);
        return setBaseSampler( (texture) ? texture->getResourceName() : cgString::Empty, sourceSampler->getStates() );

    } // End if update

    // Notify any listeners of this change.
    static const cgString strContext = _T("BaseSampler");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : setBaseSampler ( )
/// <summary>
/// Set the base texture and sampling details to be used (if any) during 
/// rendering of this sky element. For instance, for a skybox, a sampler
/// referencing a cube map should be supplied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSkyElement::setBaseSampler( cgInputStream texture, const cgSamplerStateDesc & states )
{
    bool serializing = shouldSerialize();
    
    // If there is no sampler currently assigned, create one.
    bool samplerCreated = false;
    if ( !mBaseSampler )
    {
        samplerCreated = true;
        cgResourceManager * resources = mParentScene->getResourceManager();
        if ( !isInternalReference() )
            mBaseSampler = resources->createSampler( mWorld, _T("SceneSkyBase") );
        else
            mBaseSampler = resources->createSampler( _T("SceneSkyBase") );
        if ( !mBaseSampler )
            return false;

        // Add the element as a reference holder.
        mBaseSampler->addReference( this, isInternalReference() );

        // Start a transaction so we can roll back on failure.
        mWorld->beginTransaction( _T("SkyElement::setBaseSampler") );

        // Store sampler id
        if ( serializing )
        {
            prepareQueries();
            mUpdateSkyProperties.bindParameter( 1, (cgUInt32)mType );
            mUpdateSkyProperties.bindParameter( 2, mBaseSampler->getReferenceId() );
            mUpdateSkyProperties.bindParameter( 3, mBaseHDRScale );
            mUpdateSkyProperties.bindParameter( 4, mReferenceId );
            
            // Execute
            if ( !mUpdateSkyProperties.step( true ) )
            {
                cgString strError;
                mUpdateSkyProperties.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update properties for sky element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
                mWorld->rollbackTransaction( _T("SkyElement::setBaseSampler") );

                // Remove the created sampler
                mBaseSampler->removeReference(this,isInternalReference());
                mBaseSampler = CG_NULL;
                return false;
            
            } // End if failed
        
        } // End if serializing

    } // End if no assigned sampler

    // Assign texture
    if ( !mBaseSampler->loadTexture( texture, 0, cgDebugSource() ) )
    {
        // Rollback necessary items.
        if ( samplerCreated )
        {
            if ( serializing )
                mWorld->rollbackTransaction( _T("SkyElement::setBaseSampler") );
            mBaseSampler->removeReference(this,isInternalReference());
            mBaseSampler = CG_NULL;
        
        } // End if new sampler

        // Failed
        return false;

    } // End if failed

    // Assign states
    mBaseSampler->setStates( states );

    // Commit changes
    if ( samplerCreated && serializing )
        mWorld->commitTransaction( _T("SkyElement::setBaseSampler") );

    // Notify any listeners of this change.
    static const cgString strContext = _T("BaseSampler");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : setSkyType ( )
/// <summary>
/// Set the enumeration describing the selected mechanism that will be 
/// used to generate and render the sky for this scene.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkyElement::setSkyType( cgSkyElementType::Base type )
{
    // Ignore if this is a no-op.
    if ( mType == type )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateSkyProperties.bindParameter( 1, (cgUInt32)type );
        mUpdateSkyProperties.bindParameter( 2, (mBaseSampler) ? mBaseSampler->getReferenceId() : 0 );
        mUpdateSkyProperties.bindParameter( 3, mBaseHDRScale );
        mUpdateSkyProperties.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( !mUpdateSkyProperties.step( true ) )
        {
            cgString strError;
            mUpdateSkyProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties for sky element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    mType = type;

    // Notify any listeners of this change.
    static const cgString strContext = _T("SkyType");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
// Name : setBaseHDRScale ( )
/// <summary>
/// Set the baseline scalar to use to boost the intensity of the sky when
/// HDR rendering is enabled.
/// </summary>
//-----------------------------------------------------------------------------
void cgSkyElement::setBaseHDRScale( cgFloat value )
{
    // Ignore if this is a no-op.
    if ( mBaseHDRScale == value )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateSkyProperties.bindParameter( 1, (cgUInt32)mType );
        mUpdateSkyProperties.bindParameter( 2, (mBaseSampler) ? mBaseSampler->getReferenceId() : 0 );
        mUpdateSkyProperties.bindParameter( 3, value );
        mUpdateSkyProperties.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( !mUpdateSkyProperties.step( true ) )
        {
            cgString strError;
            mUpdateSkyProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties for sky element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    mBaseHDRScale = value;

    // Notify any listeners of this change.
    static const cgString strContext = _T("BaseHDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
// Name : getBaseSampler ( )
/// <summary>
/// Retrieve the base texture and sampling details to be used (if any) during 
/// rendering of this sky element. For instance, for a skybox, a sampler 
/// referencing a cube map will be returned if assigned.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgSkyElement::getBaseSampler( )
{
    return mBaseSampler;
}

//-----------------------------------------------------------------------------
// Name : getSkyType ( )
/// <summary>
/// Retrieve the enumeration describing the selected mechanism that will be 
/// used to generate and render the sky for this scene.
/// </summary>
//-----------------------------------------------------------------------------
cgSkyElementType::Base cgSkyElement::getSkyType( ) const
{
    return mType;
}

//-----------------------------------------------------------------------------
// Name : getBaseHDRScale ( )
/// <summary>
/// Retrieve the baseline scalar being used to boost the intensity of the sky
/// when HDR rendering is enabled.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSkyElement::getBaseHDRScale( ) const
{
    return mBaseHDRScale;
}