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
// Name : cgMaterial.cpp                                                     //
//                                                                           //
// Desc : Provides classes which outline the various properties that should  //
//        be applied to individual surfaces within the world. These include  //
//        properties such as light reflectance, samplers and potentially     //
//        even physical properties (friction etc.)                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgMaterial Module Includes
//-----------------------------------------------------------------------------
#include <World/cgWorldConfiguration.h>
#include <Resources/cgMaterial.h>
#include <Resources/cgResourceManager.h>
#include <System/cgStringUtility.h>
#include <System/cgImage.h>

///////////////////////////////////////////////////////////////////////////////
// cgMaterial Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgMaterial () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMaterial::cgMaterial( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldResourceComponent( nReferenceId, pWorld )
{
    // Initialize variables
    mProperties           = 0;
    
    // Loading & serialization
    mSourceRefId          = 0;
    mMaterialSerialized   = false;
    mDBDirtyFlags         = 0;

    // Sandbox
    mPreviewImage         = CG_NULL;
    
    // Cached responses
    mResourceType          = cgResourceType::Material;
    mCanEvict             = true;
}

//-----------------------------------------------------------------------------
//  Name : cgMaterial () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMaterial::cgMaterial( cgUInt32 nReferenceId, cgWorld * pWorld, cgUInt32 nSourceRefId ) : cgWorldResourceComponent( nReferenceId, pWorld )
{
    // Initialize variables
    mProperties           = 0;
    
    // Loading & serialization
    mSourceRefId          = nSourceRefId;
    mMaterialSerialized   = false;
    mDBDirtyFlags         = 0;

    // Sandbox
    mPreviewImage         = CG_NULL;
    
    // Cached responses
    mResourceType         = cgResourceType::Material;
    mCanEvict             = true;
}

//-----------------------------------------------------------------------------
//  Name : cgMaterial () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMaterial::cgMaterial( cgUInt32 nReferenceId, cgWorld * pWorld, cgMaterial * pInit ) : cgWorldResourceComponent( nReferenceId, pWorld, pInit )
{
    // Initialize variables to sensible defaults.
    mManager              = pInit->mManager;
    
    // Loading and serialization.
    mSourceRefId          = 0;
    mMaterialSerialized   = false; // Material is not yet serialized in the clone case.
    mDBDirtyFlags         = AllDirty;
    
    // Material data.
    mProperties           = pInit->mProperties;

    // resources
    mSurfaceShader        = pInit->mSurfaceShader;

    // Sandbox
    mPreviewImage         = CG_NULL;
    
    // Register the name with the resource manager (do not use the 'setName()' method 
    // as this may trigger premature serialization).
    if ( mName.empty() == false )
    {
        cgResourceManager::NameUsageMap & Usage = mManager->getMaterialNameUsage();
        cgString strKey = cgString::toLower( mName );
        cgResourceManager::NameUsageMap::iterator itName = Usage.find( strKey );
        if ( itName != Usage.end() )
            itName->second = itName->second + 1;
        else
            Usage[strKey] = 1;

    } // End if has name

    // Cached responses
    mResourceType = cgResourceType::Material;
    mCanEvict     = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgMaterial () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMaterial::~cgMaterial( )
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
void cgMaterial::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Remove from name usage map
    setName( cgString::Empty );

    // Unload resource data.
    unloadResource();

    // Unload child resources
    mSurfaceShader.close();

    // Dispose base.
    if ( bDisposeBase == true )
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
bool cgMaterial::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_MaterialResource )
        return true;

    // Supported by base?
    return cgWorldResourceComponent::queryReferenceType( type );
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
bool cgMaterial::loadMaterial( cgUInt32 nSourceRefId, cgResourceManager * pManager /* = CG_NULL */ )
{
    // Dispose of any prior material data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager != CG_NULL && mManager == CG_NULL )
        mManager = pManager;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// Load the underlying resource data based on the details specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMaterial::loadResource( )
{
    bool bSerializeNecessary = false;

    // Is resource already loaded?
    if ( isLoaded() )
        return true;

    // If we should be loading data, do so now.
    bool bResult = true;
    if ( mSourceRefId != 0 )
    {
        if ( (bResult = loadMaterial( mSourceRefId, CG_NULL )) == false )
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
        // application has added this material) needs to be written to
        // the database.
        bSerializeNecessary = true;

    } // End if not loading

    // If we are not an internal resource, write any requested data.
    if ( bSerializeNecessary == true && shouldSerialize() == true )
    {
        if ( serializeMaterial( ) == false )
            return false;

    } // End if serialize data.

    // Mark the material as loaded. We always assume that it is
    // loaded since the material may simply supply its own default 
    // values which will be modified on the fly (i.e. the material
    // can be applied from the moment it is instantiated).
    mResourceLoaded = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// Unload the underlying resource data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMaterial::unloadResource( )
{
    // Loading and serialization
    // Note: Do not modify mMaterialSerialized
    mDBDirtyFlags = 0;

    // Clear preview image.
    if ( mPreviewImage != CG_NULL )
        mPreviewImage->scriptSafeDispose();
    mPreviewImage = CG_NULL;

    // Resource is no longer loaded, but if we are not internal
    // and the data WAS serialized, we can reload.
    if ( isInternalReference() == false && mMaterialSerialized == true )
        mSourceRefId = mReferenceId;
    mResourceLoaded = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginPopulate ()
/// <summary>
/// When building a material manually, this must be called before 
/// doing so.
/// </summary>
//-----------------------------------------------------------------------------
void cgMaterial::beginPopulate( )
{
    beginPopulate( CG_NULL );
}
void cgMaterial::beginPopulate( cgResourceManager * pManager )
{
    if ( pManager != CG_NULL )
        mManager = pManager;
}

//-----------------------------------------------------------------------------
//  Name : endPopulate ()
/// <summary>
/// When building a material manually, this must be called once the
/// construction is complete.
/// </summary>
//-----------------------------------------------------------------------------
void cgMaterial::endPopulate()
{
}

//-----------------------------------------------------------------------------
//  Name : compare ()
/// <summary>
/// Performs a deep compare allowing us to perform rapid binary tree 
/// style searches (such as the map in our resource manager).
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgMaterial::compare( const cgMaterial & Material ) const
{
    cgInt nDifference = 0;

    // Important: Shader should be the primary sorting key such that we 
    // reduce the number of begin / end calls as much as we possibly can.
    nDifference = (mSurfaceShader == Material.mSurfaceShader ) ? 0 : (mSurfaceShader < Material.mSurfaceShader ) ? -1 : 1;
    if ( nDifference != 0 ) return nDifference;

    // Compare material properties / style
    nDifference = (mProperties == Material.mProperties) ? 0 : (mProperties < Material.mProperties) ? -1 : 1;
    if ( nDifference != 0 ) return nDifference;
    
    // Totally equal
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : reset ()
/// <summary>
/// Reset / restore any applicable data when the material is no longer set to 
/// the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgMaterial::reset( cgRenderDriver * pDriver )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : getPreviewImage ()
/// <summary>
/// Retrieve the constructed (or loaded) preview image used by the sandbox
/// material editor to display a representation of this material.
/// </summary>
//-----------------------------------------------------------------------------
cgImage * cgMaterial::getPreviewImage( )
{
    return mPreviewImage;
}

//-----------------------------------------------------------------------------
//  Name : getMaterialProperties ()
/// <summary>
/// Retrieve material property bits used to identify and filter materials based 
/// on user customizable properties.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt64 cgMaterial::getMaterialProperties( ) const
{
    return mProperties;
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Get the editor name of this material.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgMaterial::getName( ) const
{
    return mName;
}

//-----------------------------------------------------------------------------
//  Name : getSurfaceShader ()
/// <summary>
/// Retrieve the loaded surface shader from the material.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderHandle cgMaterial::getSurfaceShader()
{
    return mSurfaceShader;
}

//-----------------------------------------------------------------------------
//  Name : getSurfaceShader ()
/// <summary>
/// Retrieve the loaded surface shader from the material. Specifying a value of
/// 'false' to the 'bAssignedOnly' parameter will instruct the material to 
/// return its 'default' fallback shader if one is not currently assigned. The
/// default value for this parameter is 'true' meaning only the user assigned
/// shader will be returned.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderHandle cgMaterial::getSurfaceShader( bool bAssignedOnly )
{
    return mSurfaceShader;
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
bool cgMaterial::buildPreviewImage( cgScene * pScene, cgRenderView * pView )
{
    // Requires derived class to provide this support.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setName ()
/// <summary>
/// Set the editor name of this material.
/// </summary>
//-----------------------------------------------------------------------------
void cgMaterial::setName( const cgString & strName )
{
    // Skip if this is a no-op.
    if ( mName == strName )
        return;

    // First, remove the currently existing name from the usage map (uses reference counting scheme)
    if ( mManager != CG_NULL && mName.empty() == false )
    {
        cgResourceManager::NameUsageMap & Usage = mManager->getMaterialNameUsage();
        cgString strKey = cgString::toLower( mName );
        cgResourceManager::NameUsageMap::iterator itName = Usage.find( strKey );
        if ( itName != Usage.end() )
        {
            // Update usage map
            itName->second = itName->second - 1;
            if ( itName->second == 0 )
                Usage.erase( strKey );
            
        } // End if exists in map

    } // End if has existing name

    // Store new name
    mName = strName;
    setResourceName( strName );

    // Add a reference to this new name to the usage map
    if ( mManager != CG_NULL && mName.empty() == false )
    {
        cgResourceManager::NameUsageMap & Usage = mManager->getMaterialNameUsage();
        cgString strKey = cgString::toLower( strName );
        cgResourceManager::NameUsageMap::iterator itName = Usage.find( strKey );
        if ( itName != Usage.end() )
            itName->second = itName->second + 1;
        else
            Usage[strKey] = 1;

    } // End if has existing name

    // Relevant terms are now dirty.
    mDBDirtyFlags |= NameDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setManagementData ()
/// <summary>
/// Informs the resource of the manager that is managing it (if any).
/// </summary>
//-----------------------------------------------------------------------------
void cgMaterial::setManagementData( cgResourceManager * pManager, cgUInt32 nFlags )
{
    // First, remove the material name from the usage map of the currently
    // defined resource manager if any (uses reference counting scheme).
    if ( mManager != CG_NULL && mName.empty() == false )
    {
        cgResourceManager::NameUsageMap & Usage = mManager->getMaterialNameUsage();
        cgString strKey = cgString::toLower( mName );
        cgResourceManager::NameUsageMap::iterator itName = Usage.find( strKey );
        if ( itName != Usage.end() )
        {
            // Update usage map
            itName->second = itName->second - 1;
            if ( itName->second == 0 )
                Usage.erase( strKey );
            
        } // End if exists in map

    } // End if has existing name

    // Call base class implementation to update management details.
    cgResource::setManagementData( pManager, nFlags );

    // Re-add name to new manager's usage map if applicable.
    if ( mManager != CG_NULL && mName.empty() == false )
    {
        cgResourceManager::NameUsageMap & Usage = mManager->getMaterialNameUsage();
        cgString strKey = cgString::toLower( mName );
        cgResourceManager::NameUsageMap::iterator itName = Usage.find( strKey );
        if ( itName != Usage.end() )
            itName->second = itName->second + 1;
        else
            Usage[strKey] = 1;

    } // End if has existing name
}

//-----------------------------------------------------------------------------
//  Name : setMaterialProperties ()
/// <summary>
/// Set material property bits used to identify and filter materials based on 
/// user customizable properties.
/// </summary>
//-----------------------------------------------------------------------------
void cgMaterial::setMaterialProperties( cgUInt64 nProperties )
{
    // Skip if this is a no-op.
    if ( mProperties == nProperties )
        return;

    // Store new properties.
    mProperties = nProperties;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= PropertiesDirty;
    
    // Update the database.
    if ( !serializeMaterial() )
        return;    
}

//-----------------------------------------------------------------------------
//  Name : addMaterialProperties ()
/// <summary>
/// Add a new material property used to identify and filter materials based on 
/// user customizable properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMaterial::addMaterialProperty( const cgString & identifier )
{
    if ( !mWorld )
        return false;

    // Find property with the specified identifier.
    cgWorldConfiguration * config = mWorld->getConfiguration();
    const cgFilterExpression::IdentifierArray & identifiers = config->getMaterialPropertyIdentifiers();
    for ( size_t i = 0; i < identifiers.size(); ++i )
    {
        // Does the property have a matching identifier?
        if ( identifiers[i].name == identifier )
        {
            // Identifier matches, is this a no-op? (i.e. already set?)
            if ( !(mProperties & identifiers[i].value) )
            {
                // Property not already set, set it and update the database.
                cgUInt64 oldProperties = mProperties;
                mProperties |= identifiers[i].value;
                mDBDirtyFlags |= PropertiesDirty;
                if ( !serializeMaterial() )
                {
                    mProperties = oldProperties;
                    return false;
                
                } // End if failed

                // Updated, return success.
                return true;

            } // End if not already set

        } // End if found match

    } // Next identifier

    // Matching property not found.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : removeMaterialProperties ()
/// <summary>
/// Remove a material property used to identify and filter materials based on 
/// user customizable properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMaterial::removeMaterialProperty( const cgString & identifier )
{
    if ( !mWorld )
        return false;

    // Find property with the specified identifier.
    cgWorldConfiguration * config = mWorld->getConfiguration();
    const cgFilterExpression::IdentifierArray & identifiers = config->getMaterialPropertyIdentifiers();
    for ( size_t i = 0; i < identifiers.size(); ++i )
    {
        // Does the property have a matching identifier?
        if ( identifiers[i].name == identifier )
        {
            // Identifier matches, is this a no-op? (i.e. is not currently set?)
            if ( mProperties & identifiers[i].value )
            {
                // Property not already set, set it and update the database.
                cgUInt64 oldProperties = mProperties;
                mProperties &= ~identifiers[i].value;
                mDBDirtyFlags |= PropertiesDirty;
                if ( !serializeMaterial() )
                {
                    mProperties = oldProperties;
                    return false;

                } // End if failed

                // Updated, return success.
                return true;

            } // End if not already set

        } // End if found match

    } // Next identifier

    // Matching property not found.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setSurfaceShader ()
/// <summary>
/// Load the surface shader file specified for rendering with this material.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMaterial::setSurfaceShader( const cgSurfaceShaderHandle & hShader )
{
    // ToDo: 9999 - Rebuild samplers and constants to use this new shader.

    // Is this a no-op?
    if ( mSurfaceShader == hShader )
        return true;

    // Store
    mSurfaceShader = hShader;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= ShaderFileDirty;
    
    // Update the database.
    serializeMaterial();

    // Success!!
    return true;
}