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
// Name : cgResourceHandles.h                                                //
//                                                                           //
// Desc : System file that provides support for resource handles. Handles    //
//        provide automatic lifetime management and easy integration with    //
//        the resource manager.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRESOURCEHANDLES_H_ )
#define _CGE_CGRESOURCEHANDLES_H_

//-----------------------------------------------------------------------------
// cgResourceHandles Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Scripting/cgScriptInterop.h>
#include <System/cgReference.h>
#include <Resources/cgResource.h>
#include <Resources/cgResourceTypes.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : ResourceHandleBase (Base Class)
/// <summary>
/// This class is similar to a smart pointer which manages a single reference 
/// to an individual resource and handles all reference counting. This is the
/// base class version from which the specific 'ResourceHandle' type is derived
/// (see templates directly following this).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API ResourceHandleBase
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgResourceManager;

public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    typedef cgResource _resType;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : ResourceHandleBase () (Constructor)
    /// <summary>
    /// Constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    ResourceHandleBase( ) : 
        mResource(CG_NULL), mDatabaseUpdate(false), mOwner(CG_NULL) {}
    
    //-------------------------------------------------------------------------
    //  Name : ResourceHandleBase () (Constructor)
    /// <summary>
    /// Constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    ResourceHandleBase( bool databaseUpdate, cgReference * owner = CG_NULL ) : 
        mResource(CG_NULL), mDatabaseUpdate(databaseUpdate), mOwner(owner) {}
    
    //-------------------------------------------------------------------------
    //  Name : ResourceHandleBase () (Constructor)
    /// <summary>
    /// Constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    ResourceHandleBase( const ResourceHandleBase & resource ) : 
        mResource(resource.mResource), mDatabaseUpdate(false), mOwner(CG_NULL)
    {
        // Add reference if applicable
        if ( mResource )
            ((cgReference*)mResource)->addReference( CG_NULL, true );
    }

    //-------------------------------------------------------------------------
    //  Name : ResourceHandleBase () (Constructor)
    /// <summary>
    /// Constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    ResourceHandleBase( const ResourceHandleBase & resource, bool databaseUpdate, cgReference * owner = CG_NULL ) :
        mResource(resource.mResource), mDatabaseUpdate(databaseUpdate), mOwner(owner)
    {
        // Add reference if applicable
        if ( mResource )
            ((cgReference*)mResource)->addReference( owner, ( (!databaseUpdate) || ((cgReference*)mResource)->isInternalReference()) );
    }

    //-------------------------------------------------------------------------
    //  Name : ResourceHandleBase () (Constructor)
    /// <summary>
    /// Constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    ResourceHandleBase( cgResource * resource ) :
        mResource(resource), mDatabaseUpdate(false), mOwner(CG_NULL)
    {
        // Add reference if applicable
        if ( mResource )
            ((cgReference*)mResource)->addReference( CG_NULL, true );
    }

    //-------------------------------------------------------------------------
    //  Name : ResourceHandleBase () (Constructor)
    /// <summary>
    /// Constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    ResourceHandleBase( cgResource * resource, bool databaseUpdate, cgReference * owner = CG_NULL ) :
        mResource(resource), mDatabaseUpdate(databaseUpdate), mOwner(owner)
    {
        // Add reference if applicable
        if ( mResource )
            ((cgReference*)mResource)->addReference( owner, ( (!databaseUpdate) || ((cgReference*)mResource)->isInternalReference()) );
    }

    //-------------------------------------------------------------------------
    //  Name : ~ResourceHandleBase () (Destructor)
    /// <summary>
    /// Destructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    ~ResourceHandleBase( )
    {
        // Remove our reference if applicable.
        if ( mResource ) 
            ((cgReference*)mResource)->removeReference( mOwner, ((!mDatabaseUpdate) || ((cgReference*)mResource)->isInternalReference()) );
        
        // Clear variables
        mResource         = CG_NULL;
        mDatabaseUpdate   = false;
        mOwner            = CG_NULL;
    }

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : getResourceType ()
    /// <summary>
    /// Retrieve the identifier informing the caller of the type of the
    /// resource being managed here.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgResourceType::Base getResourceType( ) const
    {
        return (mResource) ? mResource->getResourceType() : cgResourceType::None;
    }

    //-------------------------------------------------------------------------
    //  Name : getReferenceId ()
    /// <summary>
    /// Retrieve the unique reference identifier of the underlying resource,
    /// or 0 if the resource isn't valid.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32 getReferenceId( ) const
    {
        return (mResource) ? ((cgReference*)mResource)->getReferenceId() : 0;
    }

    //-------------------------------------------------------------------------
    //  Name : setOwnerDetails ()
    /// <summary>
    /// Set the owner pointer that will be supplied whenever the reference 
    /// count for the underlying resource is incremented or decremented. 
    /// Optionally, this information can be immediately applied and any 
    /// reference currently held will be swapped based on the requested options.
    /// </summary>
    //-------------------------------------------------------------------------
    void setOwnerDetails( cgReference * owner, bool applyChanges = false )
    {
        // Is this a no-op?
        if ( owner == mOwner )
            return;

        // Apply the changes to the owner details to an existing resource?
        if ( applyChanges && mResource )
        {
            // If we are applying changes, add the new reference first
            // in order to ensure that we don't inadvertantly trigger
            // the destruction of the resource.
            ((cgReference*)mResource)->addReference( owner, mResource->isInternalReference() );
            
            // Now we can release our old reference.
            ((cgReference*)mResource)->removeReference( mOwner, mResource->isInternalReference() );
            
        } // End if apply changes

        // Record new options.
        mOwner = owner;
    }

    //-------------------------------------------------------------------------
    //  Name : enableDatabaseUpdate ()
    /// <summary>
    /// Enable or disable updates to the database when adding / releasing 
    /// references to this resource. This will ensure that soft (database) 
    /// reference counting will be considered.  Optionally, this information 
    /// can be immediately applied and any reference currently held will be 
    /// swapped based on the requested options.
    /// </summary>
    //-------------------------------------------------------------------------
    void enableDatabaseUpdate( bool enable, bool applyChanges = false )
    {
        // Is this a no-op?
        if ( mDatabaseUpdate == enable )
            return;

        // Apply the changes to an existing resource?
        if ( applyChanges && mResource )
        {
            // If we are applying changes, add the new reference first
            // in order to ensure that we don't inadvertantly trigger
            // the destruction of the resource.
            ((cgReference*)mResource)->addReference( mOwner, ((!enable) || mResource->isInternalReference()) );
            
            // Now we can release our old reference.
            ((cgReference*)mResource)->removeReference( mOwner, ((!mDatabaseUpdate) || mResource->isInternalReference()) );
            
        } // End if apply changes

        // Record new options
        mDatabaseUpdate = enable;
    }

    //-------------------------------------------------------------------------
    //  Name : isResourceLost ()
    /// <summary>
    /// Determine if the resource that this handle is pointing is currently
    /// in a lost state.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isResourceLost( ) const
    {
        return (mResource) ? mResource->isResourceLost() : false;
    }

    //-------------------------------------------------------------------------
    //  Name : setResourceLost ()
    /// <summary>
    /// Update the "lost" state of the underlying resource.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setResourceLost( bool lost )
    {
        if ( mResource )
            mResource->setResourceLost( lost );
    }

    //-------------------------------------------------------------------------
    //  Name : loadResource ()
    /// <summary>
    /// Request that the underlying resource loads its data if it has not 
    /// already been loaded.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool loadResource( )
    {
        return (mResource) ? mResource->loadResource() : false;
    }

    //-------------------------------------------------------------------------
    //  Name : unloadResource ()
    /// <summary>
    /// Request that the underlying resource unloads its data, but the handle
    /// itself remains valud.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool unloadResource( )
    {
        // Returns 'true' even if no resource is actually assigned to
        // this handle. Requesting that a null resource is unloaded should 
        // not result in a fatal error situation.
        return (mResource) ? mResource->unloadResource() : true;
    }

    //-------------------------------------------------------------------------
    //  Name : close ()
    /// <summary>
    /// "Close" this handle and ensure that any underlying resources are
    /// released.
    /// </summary>
    //-------------------------------------------------------------------------
    void close( )
    {
        if ( mResource )
            mResource->removeReference( mOwner, ((!mDatabaseUpdate) || ((cgReference*)mResource)->isInternalReference()), false );
        mResource = CG_NULL;
    }

    //-------------------------------------------------------------------------
    //  Name : close ()
    /// <summary>
    /// "Close" this handle and ensure that any underlying resources are
    /// released. Any destruction delay specified by the underlying resource 
    /// can optionally be overriden to release immediately should this be the 
    /// last remaining open handle.
    /// </summary>
    //-------------------------------------------------------------------------
    void close( bool ignoreDestructDelay )
    {
        if ( mResource )
            mResource->removeReference( mOwner, ((!mDatabaseUpdate) || ((cgReference*)mResource)->isInternalReference()), ignoreDestructDelay );
        mResource = CG_NULL;
    }

    //-------------------------------------------------------------------------
    // Name : isValid ()
    /// <summary>
    /// Determine if the specified handle manages a valid resource.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isValid( ) const
    {
        return (mResource != CG_NULL);
    }

    //-------------------------------------------------------------------------
    // Name : isLoaded ()
    /// <summary>
    /// Determine if the specified handle manages a resource that is both
    /// valid, and currently loaded.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isLoaded( ) const
    {
        return (mResource) ? mResource->isLoaded() : false;
    }

    //-------------------------------------------------------------------------
    //  Name : getDatabaseUpdate ()
    /// <summary>
    /// Determine if database update mode is enabled.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool getDatabaseUpdate( ) const
    {
        return mDatabaseUpdate;
    }

    //-------------------------------------------------------------------------
    //  Name : getOwner ()
    /// <summary>
    /// Retrieve the registered owner used when adding or removing references
    /// to the underlying resource.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgReference * getOwner( ) const
    {
        return mOwner;
    }
    
protected:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgResource    * mResource;        // The resource managed by this handle
    cgReference   * mOwner;           // Who should we mark as owner?
    bool            mDatabaseUpdate;  // Should reference counting update database soft reference count?
};

//-----------------------------------------------------------------------------
//  Name : ResourceHandle<Resource Type, Base Class> (Template Class)
/// <summary>
/// This class is similar to a smart pointer which manages a single reference 
/// to an individual resource and handles all reference counting. This class is
/// derived from the base 'ResourceHandleBase' which provides much of the
/// handle's basic functionality.
/// </summary>
//-----------------------------------------------------------------------------
template <class _RT, class _BaseClass = ResourceHandleBase>
class CGE_API ResourceHandle : public _BaseClass
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgResourceManager;

public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    typedef _RT _resType;

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    static const ResourceHandle<_RT,_BaseClass> Null;

    //-------------------------------------------------------------------------
    // Constructors & Destructors (Pass throughs)
    //-------------------------------------------------------------------------
    ResourceHandle( ) : _BaseClass( ) {}

    ResourceHandle( bool databaseUpdate, cgReference * owner = CG_NULL ) : 
        _BaseClass( databaseUpdate, owner ) {}
    
    ResourceHandle( const ResourceHandle & resource ) : 
        _BaseClass( resource ) {}

    ResourceHandle( const ResourceHandle & resource, bool databaseUpdate, cgReference * owner = CG_NULL ) :
        _BaseClass( resource, databaseUpdate, owner ) {}
        
    ResourceHandle( _RT * resource ) :
        _BaseClass( (_BaseClass::_resType*)resource ) {}
        
    ResourceHandle( _RT * resource, bool databaseUpdate, cgReference * owner = CG_NULL ) :
        _BaseClass( (_BaseClass::_resType*)resource, databaseUpdate, owner )  {}

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : getResource ()
    /// <summary>
    /// Called to retrieve a pointer to the underlying resource data.
    /// </summary>
    //-------------------------------------------------------------------------
    inline _RT * getResource( bool guaranteeLoaded = true )
    {
        // Actually managing a valid resource?
        if ( mResource )
        {
            // Notify manager that we touched this resource (for LMRU processing)
            mResource->resourceTouched();

            // If the caller required that the resource be immediately available
            // then load (if applicable).
            if ( guaranteeLoaded && !isLoaded() )
                mResource->loadResource();

        } // End if resource available

        // Return the resource pointer
        return (_RT*)mResource;
    }

    //-------------------------------------------------------------------------
    //  Name : getResourceSilent ()
    /// <summary>
    /// Simply retrieve the underlying resource pointer without doing
    /// anything or informing anyone.
    /// </summary>
    //-------------------------------------------------------------------------
    inline _RT * getResourceSilent( )
    {
        return (_RT*)mResource;
    }

    //-------------------------------------------------------------------------
    //  Name : getResourceSilent () (const)
    /// <summary>
    /// Simply retrieve the underlying resource pointer without doing
    /// anything or informing anyone.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const _RT * getResourceSilent( ) const
    {
        return (const _RT*)mResource;
    }

    //-------------------------------------------------------------------------
    //  Name : exchangeResource ()
    /// <summary>
    /// Swap internal resource data with that maintained by the specified 
    /// resource. Use with extreme care.
    /// </summary>
    //-------------------------------------------------------------------------
    bool exchangeResource( ResourceHandle & resource )
    {
        // Ensure this is a valid exchange
        if ( getResourceType() != resource.getResourceType() )
            return false;
        if ( !isValid() || !resource.isValid() )
            return false;

        // Swap internal data.
        return mResource->exchangeResource( resource.mResource );
    }

    //-------------------------------------------------------------------------
    // Operator Overloads
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : operator= () (ResourceHandle<_RT>&)
    /// <summary>
    /// Overloaded assignment operator.
    /// </summary>
    //-------------------------------------------------------------------------
    ResourceHandle & operator=( const ResourceHandle & handle )
    {
        // It's important that we increment the reference count
        // of the input stream first in case it is a self reference
        // or referencing the same data.
        if ( handle.mResource )
            ((cgReference*)handle.mResource)->addReference( mOwner, ((!mDatabaseUpdate) || ((cgReference*)handle.mResource)->isInternalReference()) );
        
        // Clean up our own internal data
        if ( mResource )
            ((cgReference*)mResource)->removeReference( mOwner, ((!mDatabaseUpdate) || ((cgReference*)mResource)->isInternalReference()) );
            
        // Share the specified handle's data pointer
        mResource = handle.mResource;

        // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
        return *this;
    }

    //-------------------------------------------------------------------------
    //  Name : operator== () (ResourceHandle<_RT>&)
    /// <summary>
    /// Overloaded equality operator.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool operator==( const ResourceHandle & handle ) const
    {
        return ( mResource == handle.mResource );
    }

    //-------------------------------------------------------------------------
    //  Name : operator!= () (ResourceHandle<_RT>&)
    /// <summary>
    /// Overloaded != equality operator.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool operator!=( const ResourceHandle & handle ) const
    {
        return ( mResource != handle.mResource );
    }

    //-------------------------------------------------------------------------
    //  Name : operator< () (ResourceHandleBase<_RT>&)
    /// <summary>
    /// Overloaded less-than operator.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool operator<( const ResourceHandle & handle ) const
    {
        return ( mResource < handle.mResource );
    }

    //-------------------------------------------------------------------------
    //  Name : operator> () (ResourceHandleBase<_RT>&)
    /// <summary>
    /// Overloaded greater-than operator.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool operator>( const ResourceHandle & handle ) const
    {
        return ( mResource > handle.mResource );
    }

    // Dereferencing operators. Use with caution!
    inline _RT * operator*( )
    {
        cgAssert( mResource != CG_NULL );
        return (_RT*)mResource;
    }
    inline _RT const * operator*( ) const
    {
        cgAssert( mResource != CG_NULL );
        return (_RT const*)mResource;
    }
    inline _RT* operator->( )
    {
        cgAssert( mResource != CG_NULL );
        return (_RT*)mResource;
    }
    inline _RT const* operator->( ) const
    {
        cgAssert( mResource != CG_NULL );
        return (_RT const*)mResource;
    }

    // Allows 'ResourceHandleBase' to function with 'unordered_map' and 'unordered_set'.
    // ToDo: Consider removing this and only supporting STL port (thus moving this
    // operator into the hash template struct below). As it is, this allows the handle
    // to be implicitly cast to a numeric type and can potentially cause function / method
    // selection ambiguities when passed as an argument (weakens the type).
    operator size_t( ) const
    {
        union __vp {
            size_t s;
            void  *p;
        };
        __vp vp;
        vp.p = (void*)getResourceSilent();
        return vp.s;
    }
        
};
template <class _RT, class _BaseClass> const ResourceHandle<_RT,_BaseClass> ResourceHandle<_RT,_BaseClass>::Null;

#if defined(__SGI_STL_PORT)
// This allows us to use ResourceHandle as a key in a unordered_map or unordered_set
namespace std
{
    template<class _RT,class _BaseClass> struct hash<ResourceHandle<_RT,_BaseClass>>
    {
        inline size_t operator()(const ResourceHandle<_RT,_BaseClass>& x) const { return (size_t)x; }
    };
};
#endif // __SGI_STL_PORT

//-----------------------------------------------------------------------------
// Common Global Typedefs
//-----------------------------------------------------------------------------
class cgAnimationSet;
typedef ResourceHandle<cgAnimationSet>                          cgAnimationSetHandle;
class cgAudioBuffer;
typedef ResourceHandle<cgAudioBuffer>                           cgAudioBufferHandle;
class cgTexture;
typedef ResourceHandle<cgTexture>                               cgTextureHandle;
class cgRenderTarget;
typedef ResourceHandle<cgRenderTarget,cgTextureHandle>          cgRenderTargetHandle;
class cgDepthStencilTarget;
typedef ResourceHandle<cgDepthStencilTarget,cgTextureHandle>    cgDepthStencilTargetHandle;
class cgMesh;
typedef ResourceHandle<cgMesh>                                  cgMeshHandle;
class cgVertexBuffer;
typedef ResourceHandle<cgVertexBuffer>                          cgVertexBufferHandle;
class cgIndexBuffer;
typedef ResourceHandle<cgIndexBuffer>                           cgIndexBufferHandle;
class cgConstantBuffer;
typedef ResourceHandle<cgConstantBuffer>                        cgConstantBufferHandle;
class cgVertexShader;
typedef ResourceHandle<cgVertexShader>                          cgVertexShaderHandle;
class cgPixelShader;
typedef ResourceHandle<cgPixelShader>                           cgPixelShaderHandle;
class cgSurfaceShader;
typedef ResourceHandle<cgSurfaceShader>                         cgSurfaceShaderHandle;
class cgMaterial;
typedef ResourceHandle<cgMaterial>                              cgMaterialHandle;
class cgScript;
typedef ResourceHandle<cgScript>                                cgScriptHandle;
class cgSamplerState;
typedef ResourceHandle<cgSamplerState>                          cgSamplerStateHandle;
class cgDepthStencilState;
typedef ResourceHandle<cgDepthStencilState>                     cgDepthStencilStateHandle;
class cgRasterizerState;
typedef ResourceHandle<cgRasterizerState>                       cgRasterizerStateHandle;
class cgBlendState;
typedef ResourceHandle<cgBlendState>                            cgBlendStateHandle;

//-----------------------------------------------------------------------------
// Common Global Containers
//-----------------------------------------------------------------------------
CGE_VECTOR_DECLARE( cgRenderTargetHandle, cgRenderTargetHandleArray )
CGE_VECTOR_DECLARE( cgAnimationSetHandle, cgAnimationSetHandleArray )
CGE_VECTOR_DECLARE( cgMaterialHandle, cgMaterialHandleArray )
CGE_VECTOR_DECLARE( cgSamplerStateHandle, cgSamplerStateHandleArray )

#endif // !_CGE_CGRESOURCEHANDLES_H_