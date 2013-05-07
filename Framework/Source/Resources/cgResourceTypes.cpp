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
// Name : cgResourceTypes.cpp                                                //
//                                                                           //
// Desc : Common system file that defines various resource types and common  //
//        enumerations.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgResourceTypes Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgResourceTypes.h>
#include <Math/cgChecksum.h>

cgToDo( "Effect Overhaul", "For consistency, it may be a good idea to rename 'cgResourceHandles.h' back to 'cgResourceTypes.h' and instead rename the current cgResourceTypes.h to something else." )

/*//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
const cgAnimationSetHandle        cgAnimationSetHandle::Null;
cgAudioBufferHandle         cgAudioBufferHandle::Null;
cgTextureHandle             cgTextureHandle::Null;
cgRenderTargetHandle        cgRenderTargetHandle::Null;
cgDepthStencilTargetHandle  cgDepthStencilTargetHandle::Null;
cgMeshHandle                cgMeshHandle::Null;
cgVertexBufferHandle        cgVertexBufferHandle::Null;
cgIndexBufferHandle         cgIndexBufferHandle::Null;
cgConstantBufferHandle      cgConstantBufferHandle::Null;
cgVertexShaderHandle        cgVertexShaderHandle::Null;
cgPixelShaderHandle         cgPixelShaderHandle::Null;
cgSurfaceShaderHandle       cgSurfaceShaderHandle::Null;
cgMaterialHandle            cgMaterialHandle::Null;
cgScriptHandle              cgScriptHandle::Null;
cgSamplerStateHandle        cgSamplerStateHandle::Null;
cgDepthStencilStateHandle   cgDepthStencilStateHandle::Null;
cgRasterizerStateHandle     cgRasterizerStateHandle::Null;
cgBlendStateHandle          cgBlendStateHandle::Null;*/

// ToDo: Remove when complete.
void foo()
{
    
}
///////////////////////////////////////////////////////////////////////////////
// ResourceHandle Member Functions
///////////////////////////////////////////////////////////////////////////////
/*//-----------------------------------------------------------------------------
//  Name : ResourceHandle () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::ResourceHandle( )
{
    // Initialize variables to sensible defaults
    m_pResource         = CG_NULL;
    m_bDatabaseUpdate   = false;
    m_pOwner            = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ResourceHandle () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::ResourceHandle( bool bDatabaseUpdate, cgReference * pOwner )
{
    // Initialize variables to sensible defaults
    m_pResource         = CG_NULL;
    m_bDatabaseUpdate   = bDatabaseUpdate;
    m_pOwner            = pOwner;
}

//-----------------------------------------------------------------------------
//  Name : ResourceHandle () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::ResourceHandle( cgResource * pResource )
{
    // Initialize variables to sensible defaults
    m_pResource         = pResource;
    m_bDatabaseUpdate   = false;
    m_pOwner            = CG_NULL;

    // Add reference if applicable
    if ( m_pResource != CG_NULL )
        m_pResource->AddReference( CG_NULL, true );
}

//-----------------------------------------------------------------------------
//  Name : ResourceHandle () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::ResourceHandle( cgResource * pResource, bool bDatabaseUpdate, cgReference * pOwner )
{
    // Initialize variables to sensible defaults
    m_pResource         = pResource;
    m_bDatabaseUpdate   = bDatabaseUpdate;
    m_pOwner            = pOwner;

    // Add reference if applicable
    if ( m_pResource != CG_NULL )
        m_pResource->AddReference( pOwner, ((bDatabaseUpdate==false) || m_pResource->IsInternalReference()) );
}

//-----------------------------------------------------------------------------
//  Name : ResourceHandle () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::ResourceHandle( const ResourceHandle & hResource )
{
    // Share the specified handle's data pointer
    m_pResource         = hResource.m_pResource;
    m_bDatabaseUpdate   = false;
    m_pOwner            = CG_NULL;

    // Add reference if applicable
    if ( m_pResource != CG_NULL )
        m_pResource->AddReference( CG_NULL, true );

}

//-----------------------------------------------------------------------------
//  Name : ResourceHandle () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::ResourceHandle( const ResourceHandle & hResource, bool bDatabaseUpdate, cgReference * pOwner )
{
    // Share the specified handle's data pointer
    m_pResource         = hResource.m_pResource;
    m_bDatabaseUpdate   = bDatabaseUpdate;
    m_pOwner            = pOwner;

    // Add reference if applicable
    if ( m_pResource != CG_NULL )
        m_pResource->AddReference( pOwner, ((bDatabaseUpdate == false) || m_pResource->IsInternalReference()) );
    
}

//-----------------------------------------------------------------------------
//  Name : ~ResourceHandle () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::~ResourceHandle()
{
    // Remove our reference if applicable.
    if ( m_pResource != CG_NULL )
        m_pResource->RemoveReference( m_pOwner, ((m_bDatabaseUpdate == false) || m_pResource->IsInternalReference()) );
    
    // Clear variables
    m_pResource         = CG_NULL;
    m_bDatabaseUpdate   = false;
    m_pOwner            = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : Close ()
/// <summary>
/// "Close" this handle and ensure that any underlying resources are
/// released.
/// </summary>
//-----------------------------------------------------------------------------
void ResourceHandle::Close( )
{
    Close( false );
}
void ResourceHandle::Close( bool bIgnoreDestructDelay )
{
    if ( m_pResource != CG_NULL )
        m_pResource->RemoveReference( m_pOwner, ((m_bDatabaseUpdate == false) || m_pResource->IsInternalReference()), bIgnoreDestructDelay );
    m_pResource = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (ResourceHandle&)
/// <summary>
/// Overloaded assignment operator.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle & ResourceHandle::operator=( const ResourceHandle & hResource )
{
    // It's important that we increment the reference count
    // of the input stream first in case it is a self reference
    // or referencing the same data.
    if ( hResource.m_pResource != CG_NULL )
        hResource.m_pResource->AddReference( m_pOwner, ((m_bDatabaseUpdate == false) || hResource.m_pResource->IsInternalReference()) );
    
    // Clean up our own internal data
    if ( m_pResource != CG_NULL )
        m_pResource->RemoveReference( m_pOwner, ((m_bDatabaseUpdate == false) || m_pResource->IsInternalReference()) );
        
    // Share the specified handle's data pointer
    m_pResource = hResource.m_pResource;

    // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator== () (ResourceHandle&)
/// <summary>
/// Overloaded equality operator.
/// </summary>
//-----------------------------------------------------------------------------
bool ResourceHandle::operator==( const ResourceHandle & hResource ) const
{
    return ( m_pResource == hResource.m_pResource );
}

//-----------------------------------------------------------------------------
//  Name : operator!= () (ResourceHandle&)
/// <summary>
/// Overloaded != equality operator.
/// </summary>
//-----------------------------------------------------------------------------
bool ResourceHandle::operator!=( const ResourceHandle & hResource ) const
{
    return ( m_pResource != hResource.m_pResource );
}

//-----------------------------------------------------------------------------
//  Name : operator< () (ResourceHandle&)
/// <summary>
/// Overloaded less-than operator.
/// </summary>
//-----------------------------------------------------------------------------
bool ResourceHandle::operator<( const ResourceHandle & hResource ) const
{
    return ( m_pResource < hResource.m_pResource );
}

//-----------------------------------------------------------------------------
//  Name : operator> () (ResourceHandle&)
/// <summary>
/// Overloaded greater-than operator.
/// </summary>
//-----------------------------------------------------------------------------
bool ResourceHandle::operator>( const ResourceHandle & hResource ) const
{
    return ( m_pResource > hResource.m_pResource );
}

//-----------------------------------------------------------------------------
//  Name : GetResource ()
/// <summary>
/// Called to retrieve a pointer to the underlying resource data.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * ResourceHandle::GetResource( bool bGuaranteeLoad )
{
    // Actually managing a valid resource?
    if ( m_pResource != CG_NULL )
    {
        // Notify manager that we touched this resource (for LMRU processing)
        m_pResource->ResourceTouched();

        // If the caller required that the resource be immediately available
        // then load (if applicable).
        if ( bGuaranteeLoad == true )
            m_pResource->GuaranteedLoad();

    } // End if resource available

    // Return the resource pointer
    return m_pResource;
}

//-----------------------------------------------------------------------------
//  Name : GetResourceSilent () (const)
/// <summary>
/// Simply retrieve the underlying resource pointer without doing
/// anything or informing anyone.
/// </summary>
//-----------------------------------------------------------------------------
const cgResource * ResourceHandle::GetResourceSilent( ) const
{
    return m_pResource;
}

//-----------------------------------------------------------------------------
//  Name : GetResourceSilent ()
/// <summary>
/// Simply retrieve the underlying resource pointer without doing
/// anything or informing anyone.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * ResourceHandle::GetResourceSilent( )
{
    return m_pResource;
}

//-----------------------------------------------------------------------------
//  Name : GetManagedData ()
/// <summary>
/// Can be called to retrieve any additional internal data that this
/// resource type is wrapping (if applicable). For instance, a texture
/// resource type might store a Direct3D IDirect3DTexture9 resource
/// internally.
/// </summary>
//-----------------------------------------------------------------------------
void * ResourceHandle::GetManagedData( bool bGuaranteeLoad )
{
    // Validate requirements
    if ( m_pResource == CG_NULL )
        return CG_NULL;

    // Notify manager that we touched this resource (for LMRU processing)
    m_pResource->ResourceTouched();

    // If the caller required that the resource be immediately available
    // then load (if applicable).
    if ( bGuaranteeLoad == true )
        m_pResource->GuaranteedLoad();

    // Return the resource's managed data
    return m_pResource->GetManagedData();
}

//-----------------------------------------------------------------------------
//  Name : GetResourceType ()
/// <summary>
/// Retrieve the unique id informing the caller of the type of the
/// resource being managed here.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceType::Base ResourceHandle::GetResourceType( ) const
{
    if ( !m_pResource )
        return cgResourceType::None;
    return m_pResource->GetResourceType();
}

//-----------------------------------------------------------------------------
//  Name : GetReferenceId ()
/// <summary>
/// Retrieve the unique reference identifier of the underlying resource, or 0
/// if the resource isn't valid.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 ResourceHandle::GetReferenceId( ) const
{
    if ( !m_pResource )
        return 0;
    return m_pResource->GetReferenceId();
}

//-----------------------------------------------------------------------------
//  Name : SetOwnerDetails ()
/// <summary>
/// Set the owner pointer that will be supplied whenever the reference count
/// for the underlying resource is incremented or decremented. Optionally, this 
/// information can be immediately applied and any reference currently held 
/// will be swapped based on the requested options.
/// </summary>
//-----------------------------------------------------------------------------
void ResourceHandle::SetOwnerDetails( cgReference * pOwner, bool bApplyChanges )
{
    // Is this a no-op?
    if ( pOwner == m_pOwner )
        return;

    // Apply the changes to the owner details to an existing resource?
    if ( bApplyChanges == true && m_pResource != CG_NULL )
    {
        // If we are applying changes, add the new reference first
        // in order to ensure that we don't inadvertantly trigger
        // the destruction of the resource.
        m_pResource->AddReference( pOwner, m_pResource->IsInternalReference() );
        
        // Now we can release our old reference.
        m_pResource->RemoveReference( m_pOwner, m_pResource->IsInternalReference() );
        
    } // End if apply changes

    // Record new options.
    m_pOwner = pOwner;
}

//-----------------------------------------------------------------------------
//  Name : EnableDatabaseUpdate ()
/// <summary>
/// Enable or disable updates to the database when adding / releasing 
/// references to this resource. This will ensure that soft (database) 
/// reference counting will be considered.  Optionally, this information can be 
/// immediately applied and any reference currently held will be swapped based 
/// on the requested options.
/// </summary>
//-----------------------------------------------------------------------------
void ResourceHandle::EnableDatabaseUpdate( bool bEnable, bool bApplyChanges )
{
    // Is this a no-op?
    if ( m_bDatabaseUpdate == bEnable )
        return;

    // Apply the changes to an existing resource?
    if ( bApplyChanges == true && m_pResource != CG_NULL )
    {
        // If we are applying changes, add the new reference first
        // in order to ensure that we don't inadvertantly trigger
        // the destruction of the resource.
        m_pResource->AddReference( m_pOwner, ((bEnable == false) || m_pResource->IsInternalReference()) );
        
        // Now we can release our old reference.
        m_pResource->RemoveReference( m_pOwner, ((m_bDatabaseUpdate == false) || m_pResource->IsInternalReference()) );
        
    } // End if apply changes

    // Record new options
    m_bDatabaseUpdate = bEnable;
}

//-----------------------------------------------------------------------------
//  Name : ExchangeResource ()
/// <summary>
/// Swap internal resource data with that maintained by the specified 
/// resource. Use with extreme care.
/// </summary>
//-----------------------------------------------------------------------------
bool ResourceHandle::ExchangeResource( ResourceHandle & hResource )
{
    // Ensure this is a valid exchange
    if ( GetResourceType() != hResource.GetResourceType() )
        return false;
    if ( IsValid() == false || hResource.IsValid() == false )
        return false;

    // Swap internal data.
    return m_pResource->ExchangeResource( hResource.m_pResource );
}

//-----------------------------------------------------------------------------
//  Name : IsResourceLost ()
/// <summary>
/// Determine if the resource that this handle is pointing is currently
/// in a lost state.
/// </summary>
//-----------------------------------------------------------------------------
bool ResourceHandle::IsResourceLost( ) const
{
    if ( m_pResource == CG_NULL )
        return false;
    return m_pResource->IsResourceLost();
}

//-----------------------------------------------------------------------------
//  Name : SetResourceLost ()
/// <summary>
/// Update the "lost" state of the underlying resource.
/// </summary>
//-----------------------------------------------------------------------------
void ResourceHandle::SetResourceLost( bool bLost )
{
    if ( m_pResource == CG_NULL ) return;
    m_pResource->SetResourceLost( bLost );
}

//-----------------------------------------------------------------------------
//  Name : operator TYPE* ()
/// <summary>
/// The various automatic casting operators to cast a handle to the
/// required resource type. Internally calls 'GetResource(true)' which
/// marks the resource as touched.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::operator cgSurface*( )
{
    if ( GetResourceType() == cgResourceType::Surface )
        return (cgSurface*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgVertexBuffer*( )
{
    if ( GetResourceType() == cgResourceType::VertexBuffer )
        return (cgVertexBuffer*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgIndexBuffer*( )
{
    if ( GetResourceType() == cgResourceType::IndexBuffer )
        return (cgIndexBuffer*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgRenderTarget*( )
{
    if ( GetResourceType() == cgResourceType::RenderTarget )
        return (cgRenderTarget*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgAudioBuffer*( )
{
    if ( GetResourceType() == cgResourceType::AudioBuffer )
        return (cgAudioBuffer*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgScript*( )
{
    if ( GetResourceType() == cgResourceType::Script )
        return (cgScript*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgTexture*( )
{
    if ( GetResourceType() == cgResourceType::Texture )
        return (cgTexture*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgMesh*( )
{
    if ( GetResourceType() == cgResourceType::Mesh )
        return (cgMesh*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgEffectFile*( )
{
    if ( GetResourceType() == cgResourceType::EffectFile )
        return (cgEffectFile*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgMaterial*( )
{
    if ( GetResourceType() == cgResourceType::Material )
        return (cgMaterial*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgAnimationSet*( )
{
    if ( GetResourceType() == cgResourceType::AnimationSet )
        return (cgAnimationSet*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgSurfaceShader*( )
{
    if ( GetResourceType() == cgResourceType::SurfaceShader )
        return (cgSurfaceShader*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgVertexShader*( )
{
    if ( GetResourceType() == cgResourceType::VertexShader )
        return (cgVertexShader*)GetResource();
    return CG_NULL;
}

ResourceHandle::operator cgPixelShader*( )
{
    if ( GetResourceType() == cgResourceType::PixelShader )
        return (cgPixelShader*)GetResource();
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : operator const TYPE* () const
/// <summary>
/// The various automatic casting operators to cast a handle to the
/// required resource type. Internally calls 'GetResourceSilent()'.
/// </summary>
//-----------------------------------------------------------------------------
ResourceHandle::operator const cgSurface*( ) const
{
    if ( GetResourceType() == cgResourceType::Surface )
        return (const cgSurface*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgVertexBuffer*( ) const
{
    if ( GetResourceType() == cgResourceType::VertexBuffer )
        return (const cgVertexBuffer*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgIndexBuffer*( ) const
{
    if ( GetResourceType() == cgResourceType::IndexBuffer )
        return (const cgIndexBuffer*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgRenderTarget*( ) const
{
    if ( GetResourceType() == cgResourceType::RenderTarget )
        return (const cgRenderTarget*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgAudioBuffer*( ) const
{
    if ( GetResourceType() == cgResourceType::AudioBuffer )
        return (const cgAudioBuffer*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgScript*( ) const
{
    if ( GetResourceType() == cgResourceType::Script )
        return (const cgScript*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgTexture*( ) const
{
    if ( GetResourceType() == cgResourceType::Texture )
        return (const cgTexture*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgMesh*( ) const
{
    if ( GetResourceType() == cgResourceType::Mesh )
        return (const cgMesh*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgEffectFile*( ) const
{
    if ( GetResourceType() == cgResourceType::EffectFile )
        return (const cgEffectFile*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgMaterial*( ) const
{
    if ( GetResourceType() == cgResourceType::Material )
        return (const cgMaterial*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgAnimationSet*( ) const
{
    if ( GetResourceType() == cgResourceType::AnimationSet )
        return (const cgAnimationSet*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgSurfaceShader*( ) const
{
    if ( GetResourceType() == cgResourceType::SurfaceShader )
        return (const cgSurfaceShader*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgVertexShader*( ) const
{
    if ( GetResourceType() == cgResourceType::VertexShader )
        return (const cgVertexShader*)GetResourceSilent();
    return CG_NULL;
}

ResourceHandle::operator const cgPixelShader*( ) const
{
    if ( GetResourceType() == cgResourceType::PixelShader )
        return (const cgPixelShader*)GetResourceSilent();
    return CG_NULL;
}*/

///////////////////////////////////////////////////////////////////////////////
// cgShaderIdentifier Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : generateHashName ()
/// <summary>
/// Generate the unique single hash for this identifier. This can be used as
/// a filename for on-disk caching.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgShaderIdentifier::generateHashName() const
{
    // First build a unique SHA1 hash (20 byte) for this shader permutation based on 
    // the source files, the parameter data and the shader identifier / name.
    cgChecksum::SHA1 Checksum;
    Checksum.beginMessage( );
    Checksum.messageData( shaderIdentifier.c_str(), shaderIdentifier.size() * sizeof(cgTChar) );
    if ( !parameterData.empty() )
        Checksum.messageData( &parameterData[0], parameterData.size() * sizeof(cgUInt32) );
    for ( size_t i = 0; i < sourceFiles.size(); ++i )
        Checksum.messageData( sourceFiles[i].hash, 20 );
    Checksum.endMessage( );

    // Retrieve the hash and turn it into a string.
    cgUInt32 PermutationHash[5];
    Checksum.getHash( PermutationHash );
    return cgString::format( _T("%x%x%x%x%x"), PermutationHash[0], PermutationHash[1], PermutationHash[2], PermutationHash[3], PermutationHash[4] );
}