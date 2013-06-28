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
// Name : cgWorldObject.cpp                                                  //
//                                                                           //
// Desc : Base class for all objects that make up a scene. Objects are       //
//        basically the foundation of all scene components such as meshes,   //
//        cameras, light sources, etc. Objects don't specifically maintain   //
//        scene information such as transformation matrices since they       //
//        technically belong to the world and can exist in multiple scenes.  //
//        Objects can however be referenced by object node types to provide  //
//        their final connection to the scene(s) as required (i.e.           //
//        cgObjectNode and its derived types to give them position,          //
//        orientation, etc.)                                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWorldObject Module Includes
//-----------------------------------------------------------------------------
#include <World/cgWorldObject.h>
#include <World/cgWorldConfiguration.h>
#include <World/cgScene.h>
#include <World/cgObjectNode.h>

// Supported sub-element types
#include <World/Objects/Elements/cgBoxCollisionShapeElement.h>
#include <World/Objects/Elements/cgCylinderCollisionShapeElement.h>
#include <World/Objects/Elements/cgConeCollisionShapeElement.h>
#include <World/Objects/Elements/cgSphereCollisionShapeElement.h>
#include <World/Objects/Elements/cgCapsuleCollisionShapeElement.h>
#include <World/Objects/Elements/cgHullCollisionShapeElement.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgWorldObject::mInsertBaseObject;
cgWorldQuery cgWorldObject::mUpdateMassProperties;
cgWorldQuery cgWorldObject::mInsertSubElement;
cgWorldQuery cgWorldObject::mDeleteSubElement;
cgWorldQuery cgWorldObject::mLoadBaseObject;
cgWorldQuery cgWorldObject::mLoadSubElements;

// Registered type containers
cgWorldObjectTypeDesc::Map cgWorldObject::mRegisteredObjectTypes;

///////////////////////////////////////////////////////////////////////////////
// cgWorldObject Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWorldObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject::cgWorldObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldComponent( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
    mMass                 = 10.0f;
    mMassTransformAmount  = 1.0f;
}

//-----------------------------------------------------------------------------
// Name : cgWorldObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject::cgWorldObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldComponent( nReferenceId, pWorld, pInit )
{
    // Initialize variables to sensible defaults
    mMass                 = pInit->mMass;
    mMassTransformAmount  = pInit->mMassTransformAmount;

    cgToDo( "Carbon General", "clone sub-elements" );
}

//-----------------------------------------------------------------------------
//  Name : ~cgWorldObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject::~cgWorldObject()
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
void cgWorldObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Disconnect from each of our sub-elements.
    // We don't do a full live de-reference here which may
    // cause the element to be deleted from the database.
    ElementCategoryMap::iterator itCategory;
    for ( itCategory = mSubElementCategories.begin(); itCategory != mSubElementCategories.end(); ++itCategory )
    {
        cgObjectSubElementArray & aSubElements = itCategory->second;
        for ( size_t i = 0; i < aSubElements.size(); ++i )
            aSubElements[i]->removeReference( this, true );
    
    } // Next category
    mSubElementCategories.clear();

    // Dispose base class as requested.
    if ( bDisposeBase == true )
        cgWorldComponent::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : RegisterObjectType() (Static)
/// <summary>
/// Allows the application to register all of the various object types
/// that are supported by the application. These objects can then be 
/// instantiated and / or referenced directly within the world database.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldObject::registerType( const cgUID & GlobalIdentifier, const cgString & strName, 
                                  cgWorldObjectTypeDesc::ObjectAllocNewFunc pObjectAllocNew,
                                  cgWorldObjectTypeDesc::ObjectAllocCloneFunc pObjectAllocClone, 
                                  cgWorldObjectTypeDesc::NodeAllocNewFunc pNodeAllocNew, 
                                  cgWorldObjectTypeDesc::NodeAllocCloneFunc pNodeAllocClone )
{
    // Build the descriptor
    cgWorldObjectTypeDesc Desc;
    Desc.globalIdentifier = GlobalIdentifier;
    Desc.localIdentifier  = 0; // 0 = Unassigned. This will be filled out in the local duplicate.
    Desc.name             = strName;
    Desc.objectAllocNew   = pObjectAllocNew;
    Desc.objectAllocClone = pObjectAllocClone;
    Desc.nodeAllocNew     = pNodeAllocNew;
    Desc.nodeAllocClone   = pNodeAllocClone;
    
    // Store
    mRegisteredObjectTypes[ GlobalIdentifier ] = Desc;
}

//-----------------------------------------------------------------------------
//  Name : getRegisteredTypes() (Static)
/// <summary>
/// Retrieve the map containing descriptions of all registered world object
/// types that are supported by the application. 
/// </summary>
//-----------------------------------------------------------------------------
const cgWorldObjectTypeDesc::Map & cgWorldObject::getRegisteredTypes( )
{
    return mRegisteredObjectTypes;
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox () (Virtual)
/// <summary>
/// Retrieve the axis aligned bounding box for this node (encompassing
/// its referenced object) as it exists in the object's local space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgWorldObject::getLocalBoundingBox( )
{
    // By default, the local space bounding box is degenerate but positioned 
    // at the object's 'local' origin (always <0,0,0> in object space).
    cgBoundingBox Bounds( cgVector3(0,0,0), cgVector3(0,0,0) );
    return Bounds;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_WorldObject )
        return true;

    // Supported by base?
    return cgWorldComponent::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Base render method is responsible for managing the render queue,
/// checking visibility states etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::render( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer )
{
    // Nothing in base class implementation.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : renderSubset ()
/// <summary>
/// Render the object one material at a time (material batching).
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::renderSubset( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer, const cgMaterialHandle & hMaterial )
{
    // Nothing in base class implementation.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Nothing in base implementation.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    cgScene * pScene = pIssuer->getScene();
    const cgUID & ActiveCategoryId = pScene->getActiveObjectElementType();

    // Iterate through active sub-elements so that they can render their representation.
    // We skip nodes that are currently merged as closed groups since they we do not
    // technically consider them 'selected'.
    bool isSelected = (pIssuer->isSelected() && !pIssuer->isMergedAsGroup());
    if ( isSelected )
    {
        ElementCategoryMap::iterator itCategory = mSubElementCategories.find( ActiveCategoryId );
        if ( itCategory != mSubElementCategories.end() )
        {
            // Allow the sub-elements to render.
            cgObjectSubElementArray & aSubElements = itCategory->second;
            for ( size_t i = 0; i < aSubElements.size(); ++i )
                aSubElements[i]->sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );

        } // End if valid category
    
    } // End if selected

    // If we were instructed to draw collision shapes regardless
    // (and we haven't already drawn them above), do so now.
    bool alwaysShowShapes = ((flags & cgSandboxRenderFlags::ShowCollisionShapes) != 0);
    if ( alwaysShowShapes && (!isSelected || ActiveCategoryId != OSECID_CollisionShapes ) )
    {
        ElementCategoryMap::iterator itCategory = mSubElementCategories.find( OSECID_CollisionShapes );
        if ( itCategory != mSubElementCategories.end() )
        {
            // Allow the sub-elements to render.
            cgObjectSubElementArray & aSubElements = itCategory->second;
            for ( size_t i = 0; i < aSubElements.size(); ++i )
                aSubElements[i]->sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );

        } // End if valid category

    } // End if draw shape
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldObject::applyObjectRescale( cgFloat fScale )
{
    // Iterate through all sub-elements so that they can apply any appropriate re-scale.
    ElementCategoryMap::iterator itCategory;
    for ( itCategory = mSubElementCategories.begin(); itCategory != mSubElementCategories.end(); ++itCategory )
    {
        cgObjectSubElementArray & aSubElements = itCategory->second;
        for ( size_t i = 0; i < aSubElements.size(); ++i )
            aSubElements[i]->applyElementRescale( fScale );
    
    } // Next Category
}

//-----------------------------------------------------------------------------
//  Name : isRenderable ()
/// <summary>
/// Determine if this object is currently renderable or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::isRenderable( ) const
{
    // By default, objects cannot render.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : isShadowCaster () (Virtual)
/// <summary>
/// Is the object capable of casting shadows? (i.e. a camera may not be)
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::isShadowCaster() const
{
    // By default, objects cannot cast shadows.
    return false;
}

//-----------------------------------------------------------------------------
// Name : clone ()
/// <summary>
/// Make a duplicate of this object and optionally its referenced data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::clone( cgCloneMethod::Base Method, cgWorld * pWorld, bool bInternal, cgWorldObject *& pObjectOut )
{
    // Be polite and clear output variables
    pObjectOut = CG_NULL;

    // Clones are always 'internal' when not in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        bInternal = true;

    // Clone the object
    pObjectOut = pWorld->createObject( bInternal, getReferenceType(), Method, this );

    // Success?
    return (pObjectOut != CG_NULL);
}

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::getSubElementCategories( cgObjectSubElementCategory::Map & Categories ) const
{
    // Clear the category list initially. This allows the caller to
    // skip this step.
    Categories.clear();

    // Add supported collision shapes.
    cgObjectSubElementCategory & Category = Categories[ OSECID_CollisionShapes ];
    Category.identifier     = OSECID_CollisionShapes;
    Category.name           = _T("Collision Shapes");
    Category.canCreate      = true;
    Category.canDelete      = true;
    Category.canEnumerate   = false;
    
    // Box Collision Shape
    {
        cgObjectSubElementDesc & Type = Category.supportedTypes[ RTID_BoxCollisionShapeElement ];
        Type.identifier = RTID_BoxCollisionShapeElement;
        Type.name       = _T("Box");
    
    } // End Box

    // Cylinder Collision Shape
    {
        cgObjectSubElementDesc & Type = Category.supportedTypes[ RTID_CylinderCollisionShapeElement ];
        Type.identifier = RTID_CylinderCollisionShapeElement;
        Type.name       = _T("Cylinder");
    
    } // End Cylinder

    // Sphere Collision Shape
    {
        cgObjectSubElementDesc & Type = Category.supportedTypes[ RTID_SphereCollisionShapeElement ];
        Type.identifier = RTID_SphereCollisionShapeElement;
        Type.name       = _T("Sphere");
    
    } // End Sphere

    // Cone Collision Shape
    {
        cgObjectSubElementDesc & Type = Category.supportedTypes[ RTID_ConeCollisionShapeElement ];
        Type.identifier = RTID_ConeCollisionShapeElement;
        Type.name       = _T("Cone");
    
    } // End Cone

    // Capsule Collision Shape
    {
        cgObjectSubElementDesc & Type = Category.supportedTypes[ RTID_CapsuleCollisionShapeElement ];
        Type.identifier = RTID_CapsuleCollisionShapeElement;
        Type.name       = _T("Capsule");
    
    } // End Capsule
        
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createSubElement () (Virtual)
/// <summary>
/// Create a new object sub-element of the specified type and add it to the
/// object's internal list.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgWorldObject::createSubElement( const cgUID & Category, const cgUID & Identifier )
{
    return createSubElement( false, Category, Identifier );
}

//-----------------------------------------------------------------------------
//  Name : supportsSubElement () (Virtual)
/// <summary>
/// Determine if the specified object sub element type is supported by this
/// world object. Derived object types should implement this to extend the
/// allowable sub element types.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::supportsSubElement( const cgUID & Category, const cgUID & Identifier ) const
{
    // Validate supported categories.
    if ( Category == OSECID_CollisionShapes )
    {
        // Validate supported types.
        if ( Identifier == RTID_BoxCollisionShapeElement ||
             Identifier == RTID_CylinderCollisionShapeElement ||
             Identifier == RTID_SphereCollisionShapeElement ||
             Identifier == RTID_ConeCollisionShapeElement ||
             Identifier == RTID_CapsuleCollisionShapeElement ||
             Identifier == RTID_HullCollisionShapeElement )
             return true;

    } // End OSECID_CollisionShapes
    
    // Unsupported
    return false;
}

//-----------------------------------------------------------------------------
//  Name : createSubElement () (Virtual)
/// <summary>
/// Create a new object sub-element of the specified type and add it to the
/// object's internal list. Optionally specify whether the element should
/// be created as an internal, or serialized component.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgWorldObject::createSubElement( bool internalElement, const cgUID & Category, const cgUID & Identifier )
{
    // Elements will always be internal if this object is internal.
    if ( isInternalReference() )
        internalElement = true;

    // Validate support.
    if ( !supportsSubElement( Category, Identifier ) )
        return CG_NULL;

    // Begin a creation transaction (if necessary) so that we can roll back
    bool bShouldSerialize = shouldSerialize() && !internalElement;
    if ( bShouldSerialize )
        mWorld->beginTransaction( _T("WorldObject::createSubElement") );

    // Create the cloned element.
    cgObjectSubElement * pNewElement = mWorld->createObjectSubElement( internalElement, Identifier, this );
    
    // If this was a supported element type, reference it and add it to the list.
    if ( pNewElement )
    {
        // Store a connecting entry in the sub-elements reference table that
        // that allows us to understand which sub-elements belong to which objects.
        if ( bShouldSerialize && !pNewElement->isInternalReference() )
        {
            prepareQueries();
            mInsertSubElement.bindParameter( 1, pNewElement->getLocalTypeId());
            mInsertSubElement.bindParameter( 2, mReferenceId );
            mInsertSubElement.bindParameter( 3, pNewElement->getReferenceId() );
            if ( mInsertSubElement.step( true ) == false )
            {
                // Roll back
                pNewElement->deleteReference( );
                mWorld->rollbackTransaction( _T("WorldObject::createSubElement") );

                // Print error and fail
                cgString strError;
                mInsertSubElement.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert sub-element reference data for world object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
                return CG_NULL;

            } // End if failed

        } // End if serialize
        
        // Full live reference if applicable.
        pNewElement->addReference( this, internalElement );

        // Push into the correct category list
        mSubElementCategories[Category].push_back( pNewElement );

        // Notify subscribed listeners
        if ( Category == OSECID_CollisionShapes )
        {
            // Notify listeners that object data has changed.
            static const cgString strContext = _T("CollisionShapeAdded");
            onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
        
        } // End if OSECID_CollisionShapes
    
    } // End if created

    // Commit all changes made.
    if ( bShouldSerialize )
        mWorld->commitTransaction( _T("WorldObject::createSubElement") );

    // Return to caller (CG_NULL if invalid)
    return pNewElement;
}

//-----------------------------------------------------------------------------
//  Name : cloneSubElement() (Virtual)
/// <summary>
/// Duplicate the specified object sub-element and add it to the object's 
/// internal list.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgWorldObject::cloneSubElement( cgObjectSubElement * pElement )
{
    // Validate requirements.
    if ( !pElement )
        return CG_NULL;

    // Get existing element details
    cgUID Category   = pElement->getElementCategory();
    cgUID Identifier = pElement->getReferenceType();

    // Validate support.
    if ( !supportsSubElement( Category, Identifier ) )
        return CG_NULL;

    // Begin a creation transaction (if necessary) so that we can roll back
    bool bShouldSerialize = shouldSerialize();
    if ( bShouldSerialize )
        mWorld->beginTransaction( _T("WorldObject::cloneSubElement") );

    // Create the cloned element.
    cgObjectSubElement * pNewElement = mWorld->createObjectSubElement( isInternalReference(), Identifier, this, pElement );
    
    // If this was a supported element type, reference it and add it to the list.
    if ( pNewElement )
    {
        // Store a connecting entry in the sub-elements reference table that
        // that allows us to understand which sub-elements belong to which objects.
        if ( bShouldSerialize && !pNewElement->isInternalReference() )
        {
            prepareQueries();
            mInsertSubElement.bindParameter( 1, pNewElement->getLocalTypeId() );
            mInsertSubElement.bindParameter( 2, mReferenceId );
            mInsertSubElement.bindParameter( 3, pNewElement->getReferenceId() );
            if ( mInsertSubElement.step( true ) == false )
            {
                // Roll back
                pNewElement->deleteReference( );
                mWorld->rollbackTransaction( _T("WorldObject::cloneSubElement") );

                // Print error and fail
                cgString strError;
                mInsertSubElement.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert sub-element reference data for world object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str());
                return CG_NULL;

            } // End if failed

        } // End if serialize
        
        // Full live reference if applicable.
        pNewElement->addReference( this, isInternalReference() );

        // Push into the correct category list
        mSubElementCategories[Category].push_back( pNewElement );

        // Notify subscribed listeners
        if ( Category == OSECID_CollisionShapes )
        {
            // Notify listeners that object data has changed.
            static const cgString strContext = _T("CollisionShapeAdded");
            onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
        
        } // End if OSECID_CollisionShapes
    
    } // End if created

    // Commit all changes made.
    if ( bShouldSerialize )
        mWorld->commitTransaction( _T("WorldObject::cloneSubElement") );

    // Return to caller (CG_NULL if invalid)
    return pNewElement;
}

//-----------------------------------------------------------------------------
//  Name : getSubElements () (Virtual)
/// <summary>
/// Retrieve an array containing all object sub-elements in the associated
/// category.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectSubElementArray & cgWorldObject::getSubElements( const cgUID & Category ) const
{
    static const cgObjectSubElementArray NoElements;
    ElementCategoryMap::const_iterator itCategory = mSubElementCategories.find( Category );
    if ( itCategory == mSubElementCategories.end() )
        return NoElements;
    return itCategory->second;
}

//-----------------------------------------------------------------------------
//  Name : deleteSubElement () (Virtual)
/// <summary>
/// Remove the specified sub-element from this object. Note: No validation is
/// performed to determine whether or not the 'delete' operation is even 
/// allowed for this class of sub-element. It is the caller's responsibility to
/// determine this based on the information returned via the
/// getSubElementCategories() method.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldObject::deleteSubElement( cgObjectSubElement * pElement )
{
    if ( !pElement )
        return;

    const cgUID & Category = pElement->getElementCategory();

    // Find the correct category list for this element.
    ElementCategoryMap::iterator itCategory = mSubElementCategories.find( Category );
    if ( itCategory == mSubElementCategories.end() )
        return;

    cgToDo( "Carbon General", "Should we optimize this by storing a cgObjectSubElementMap instead of array?" );
    // Remove it from the list.
    bool bObjectOwner = false;
    for ( size_t i = 0; i < itCategory->second.size(); ++i )
    {
        if ( itCategory->second[i] == pElement )
        {
            itCategory->second.erase( itCategory->second.begin() + i );
            bObjectOwner = true;
            break;
        
        } // End if match

    } // Next entry

    // Release our reference to the element (assuming we owned it to begin with).
    // This should trigger a full database delete if applicable (i.e. not a disconnect).
    if ( bObjectOwner )
    {
        // Remove the connecting entry from the sub-elements reference table that
        // that allowed us to understand which sub-elements belong to which objects.
        if ( shouldSerialize() && !pElement->isInternalReference() )
        {
            prepareQueries();
            mDeleteSubElement.bindParameter( 1, mReferenceId );
            mDeleteSubElement.bindParameter( 2, pElement->getReferenceId() );
            if ( mDeleteSubElement.step( true ) == false )
            {
                // Print error and fail
                cgString strError;
                mDeleteSubElement.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to remove sub-element reference data from world object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str());
                return;

            } // End if failed

        } // End if serialize

        // Allow the element to remove itself from the database.
        pElement->removeReference( this, isInternalReference() );

        // Notify subscribed listeners
        if ( Category == OSECID_CollisionShapes )
        {
            // Notify listeners that object data has changed.
            static const cgString strContext = _T("CollisionShapeRemoved");
            onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
        
        } // End if OSECID_CollisionShapes

    } // End if should delete
}

//-----------------------------------------------------------------------------
//  Name : deleteSubElement () (Virtual)
/// <summary>
/// Remove the specified sub-elements from this object. Note: No validation is
/// performed to determine whether or not the 'delete' operation is even 
/// allowed for this class of sub-element. It is the caller's responsibility to
/// determine this based on the information returned via the
/// getSubElementCategories() method.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldObject::deleteSubElements( const cgObjectSubElementArray & Elements )
{
    for ( size_t i = 0; i < Elements.size(); ++i )
        deleteSubElement( Elements[i] );
}

//-----------------------------------------------------------------------------
//  Name : setBaseMass ()
/// <summary>
/// Set the base mass for this object. The mass may be scaled up / down based
/// on the scale component of any referencing node's transform based on the
/// mass transform amount scalar (see setMassTransformAmount())
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldObject::setBaseMass( cgFloat fMass )
{
    if ( shouldSerialize() )
    {
        // Update database.
        prepareQueries();
        mUpdateMassProperties.bindParameter( 1, fMass );
        mUpdateMassProperties.bindParameter( 2, mMassTransformAmount );
        mUpdateMassProperties.bindParameter( 3, mReferenceId );
        if ( !mUpdateMassProperties.step( true ) )
        {
            cgString strError;
            mUpdateMassProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update mass properties for world object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed

    } // End if serialize

    // Update local member
    mMass = fMass;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("BaseMass");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setMassTransformAmount ()
/// <summary>
/// A [0..1] range value that describes the amount by which the object's base 
/// mass may be scaled up / down based on the scale component of any 
/// referencing node's transform.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldObject::setMassTransformAmount( cgFloat fAmount )
{
    if ( shouldSerialize() )
    {
        // Update database.
        prepareQueries();
        mUpdateMassProperties.bindParameter( 1, mMass );
        mUpdateMassProperties.bindParameter( 2, fAmount );
        mUpdateMassProperties.bindParameter( 3, mReferenceId );
        if ( !mUpdateMassProperties.step( true ) )
        {
            cgString strError;
            mUpdateMassProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update mass properties for world object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed

    } // End if serialize

    // Update local member
    mMassTransformAmount = fAmount;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MassTransformAmount");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getBaseMass ()
/// <summary>
/// Get the base mass for this object. The mass may be scaled up / down based
/// on the scale component of any referencing node's transform based on the
/// mass transform amount scalar (see setMassTransformAmount())
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgWorldObject::getBaseMass( ) const
{
    return mMass;
}

//-----------------------------------------------------------------------------
//  Name : getMassTransformAmount ()
/// <summary>
/// A [0..1] range value that describes the amount by which the object's base 
/// mass may be scaled up / down based on the scale component of any 
/// referencing node's transform.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgWorldObject::getMassTransformAmount( ) const
{
    return mMassTransformAmount;
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object base data.
    if ( !insertComponentData() )
        return false;
    
    // Call base class implementation last.
    return cgWorldComponent::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("WorldObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertBaseObject.bindParameter( 1, mReferenceId );
        mInsertBaseObject.bindParameter( 2, mMass );
        mInsertBaseObject.bindParameter( 3, mMassTransformAmount );
        
        // Execute
        if ( !mInsertBaseObject.step( true ) )
        {
            cgString strError;
            mInsertBaseObject.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert world object '0x%x' base data into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("WorldObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Insert entries into sub-element connection table for existing 
        // serialized sub-elements which may have come from a clone.
        ElementCategoryMap::iterator itCategory;
        for ( itCategory = mSubElementCategories.begin(); itCategory != mSubElementCategories.end(); ++itCategory )
        {
            cgObjectSubElementArray & aSubElements = itCategory->second;
            for ( size_t i = 0; i < aSubElements.size(); ++i )
            {
                // Only create connection data for serialized sub-elements.
                if ( !aSubElements[i]->isInternalReference() )
                {
                    mInsertSubElement.bindParameter( 1, aSubElements[i]->getLocalTypeId());
                    mInsertSubElement.bindParameter( 2, mReferenceId );
                    mInsertSubElement.bindParameter( 3, aSubElements[i]->getReferenceId() );
                    if ( mInsertSubElement.step( true ) == false )
                    {
                        // Print error and fail
                        cgString strError;
                        mInsertSubElement.getLastError( strError );
                        cgAppLog::write( cgAppLog::Error, _T("Failed to insert sub-element reference data for world object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
                        mWorld->rollbackTransaction( _T("WorldObject::insertComponentData") );
                        return false;

                    } // End if failed
                
                } // End if !internal

            } // Next sub-element
        
        } // Next category

        // Commit changes
        mWorld->commitTransaction( _T("WorldObject::insertComponentData") );

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
bool cgWorldObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the base object data.
    prepareQueries();
    mLoadBaseObject.bindParameter( 1, e->sourceRefId );
    if ( !mLoadBaseObject.step( ) || !mLoadBaseObject.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadBaseObject.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for world object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for world object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadBaseObject.reset();
        return false;
    
    } // End if failed
    
    // Update our local members
    mLoadBaseObject.getColumn( _T("Mass"), mMass );
    mLoadBaseObject.getColumn( _T("MassTransformAmount"), mMassTransformAmount );
    mLoadBaseObject.reset();

    // Now load the sub-element data.
    mLoadSubElements.bindParameter( 1, e->sourceRefId );
    if ( mLoadSubElements.step() )
    {
        // Iterate through each row returned and load each sub-element.
        for ( ; mLoadSubElements.nextRow(); )
        {
            cgUInt32 nSubElementId = 0, nSubElementTypeId;
            mLoadSubElements.getColumn( _T("SubElementTypeId"), nSubElementTypeId );
            mLoadSubElements.getColumn( _T("SubElementId"), nSubElementId );

            // Find the 'Global' type identifier that corresponds to the specified 
            // local (world database) integer type identifier for this sub-element type.
            // This reverse lookup can be achieved via the world's configuration object.
            cgWorldConfiguration * pConfig = mWorld->getConfiguration();
            const cgObjectSubElementTypeDesc * pElementTypeDesc = pConfig->getObjectSubElementTypeByLocalId( nSubElementTypeId );
            if ( !pElementTypeDesc )
                continue;
            
            // Load the referenced sub-element.
            cgObjectSubElement * pElement = mWorld->loadObjectSubElement( pElementTypeDesc->globalIdentifier, nSubElementId, this, e->cloneMethod );
            if ( !pElement )
                continue;

            // Add the object as a reference holder (just reconnect,
            // do not adjust the database ref count).
            pElement->addReference( this, true );

            // Push this element to all appropriate lists.
            mSubElementCategories[pElement->getElementCategory()].push_back( pElement );

        } // Next sub-element

    } // End if success

    // We're done with the sub-element data.
    mLoadSubElements.reset();

    // Call base class implementation to read remaining data.
    if ( !cgWorldComponent::onComponentLoading( e ) )
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
void cgWorldObject::onComponentDeleted( )
{
    // Remove our physical reference to any sub-element data. Full database 
    // update and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    ElementCategoryMap::iterator itCategory;
    for ( itCategory = mSubElementCategories.begin(); itCategory != mSubElementCategories.end(); ++itCategory )
    {
        cgObjectSubElementArray & aSubElements = itCategory->second;
        for ( size_t i = 0; i < aSubElements.size(); ++i )
            aSubElements[i]->removeReference( this, isInternalReference() );
    
    } // Next category
    mSubElementCategories.clear();

    // Note: Sub-Element connection information (in Objects::Base::All::SubElements table)
    // is automatically deleted via a database side trigger.
    
    // Call base class implementation last.
    cgWorldComponent::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
//  Name : createTypeTables() (Virtual)
/// <summary>
/// When this type of component is being inserted into the database for the
/// first time, this method will be called in order to allow it to create any 
/// necessary tables.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldObject::createTypeTables( const cgUID & TypeIdentifier )
{
    // Ensure this base class table is created first.
    if ( !cgWorldComponent::createTypeTables( RTID_WorldObject ) )
        return false;

    // Then allow derived type to create as normal.
    return cgWorldComponent::createTypeTables( TypeIdentifier );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertBaseObject.isPrepared() )
        {
            cgString sSQL = _T("INSERT INTO 'Objects::Base::All' VALUES(?1,?2,?3)");
            mInsertBaseObject.prepare( mWorld, sSQL, true );
        
        } // End if !prepared

        if ( !mInsertSubElement.isPrepared() )
        {
            cgString sSQL = _T("INSERT INTO 'Objects::Base::All::SubElements' VALUES(NULL,?1,?2,?3)");
            mInsertSubElement.prepare( mWorld, sSQL, true );
        
        } // End if !prepared

        if ( !mDeleteSubElement.isPrepared() )
        {
            cgString sSQL = _T("DELETE FROM 'Objects::Base::All::SubElements' WHERE ObjectId=?1 AND SubElementId=?2");
            mDeleteSubElement.prepare( mWorld, sSQL, true );
        
        } // End if !prepared

        if ( !mUpdateMassProperties.isPrepared() )
        {
            cgString sSQL = _T("UPDATE 'Objects::Base::All' SET Mass=?1, MassTransformAmount=?2 WHERE RefId=?3");
            mUpdateMassProperties.prepare( mWorld, sSQL, true );
        
        } // End if !prepared
        
    } // End if sandbox

    // Read queries
    if ( mLoadBaseObject.isPrepared() == false )
        mLoadBaseObject.prepare( mWorld, _T("SELECT * FROM 'Objects::Base::All' WHERE RefId=?1"), true );
    if ( mLoadSubElements.isPrepared() == false )
        mLoadSubElements.prepare( mWorld, _T("SELECT * FROM 'Objects::Base::All::SubElements' WHERE ObjectId=?1"), true );
}