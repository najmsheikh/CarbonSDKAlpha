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
// Name : cgBillboardObject.cpp                                              //
//                                                                           //
// Desc : Contains classes responsible for managing a single billboard as a  //
//        scene object that can be created and manipulated both at runtime   //
//        and in advance within the editor.                                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBillboardObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgBillboardObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgScene.h>
#include <Rendering/cgBillboardBuffer.h>
//#include <Rendering/cgRenderDriver.h>
//#include <Math/cgCollision.h>

// ToDo: Create a centralized billboard buffer pool (most likely in the resource manager) so that
//       billboards can be rendered in a combined fashion. The current implementation is really just
//       for testing at the moment.

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
//cgWorldQuery cgBillboardObject::mInsertBillboard;
//cgWorldQuery cgBillboardObject::mLoadBillboard;

///////////////////////////////////////////////////////////////////////////////
// cgBillboardObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBillboardObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardObject::cgBillboardObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize members to sensible defaults
    mSize           = cgSizeF(0,0);
    mScale          = cgVector2(1,1);
    mColor          = 0xFFFFFFFF;
    mFrameGroup     = 0;
    mFrame          = 0;
    mRotation       = 0;
    mHDRScale       = 1;
    mAlignBillboard = false;
}

//-----------------------------------------------------------------------------
//  Name : cgBillboardObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardObject::cgBillboardObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgBillboardObject * pObject = (cgBillboardObject*)pInit;
    mSize           = pObject->mSize;
    mScale          = pObject->mScale;
    mColor          = pObject->mColor;
    mFrameGroup     = pObject->mFrameGroup;
    mFrame          = pObject->mFrame;
    mRotation       = pObject->mRotation;
    mHDRScale       = pObject->mHDRScale;
    mAlignBillboard = pObject->mAlignBillboard;
}

//-----------------------------------------------------------------------------
//  Name : ~cgBillboardObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardObject::~cgBillboardObject()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgBillboardObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgBillboardObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgBillboardObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgBillboardObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgBillboardObject::getLocalBoundingBox( )
{
    // Set to current position by default
    cgVector3 HalfSize = cgVector3( mSize.width * mScale.x, mSize.height * mScale.y, 0) * 0.5f;
    HalfSize.z = min( HalfSize.x, HalfSize.y );
    return cgBoundingBox( -HalfSize.x, -HalfSize.y, -HalfSize.z, HalfSize.x, HalfSize.y, HalfSize.z ); 
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // ToDo:
    
    return false;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // ToDo: 

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::applyObjectRescale( cgFloat fScale )
{
    /*// Apply the scale to object-space data
    cgFloat fNewSize = mSize * fScale;
    
    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateSize.bindParameter( 1, fNewSize );
        mUpdateSize.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdateSize.step( true ) == false )
        {
            cgString strError;
            mUpdateSize.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update size of billboard object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local values.
    mSize = fNewSize;
    
    // Notify listeners that property was altered
    static const cgString strContext = _T("ApplyRescale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );*/

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BillboardObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgBillboardObject::getDatabaseTable( ) const
{
    return _T("Objects::Billboard");
}

//-----------------------------------------------------------------------------
//  Name : getSize()
/// <summary>
/// Retrieve the base size of the billboard (pre-scale).
/// </summary>
//-----------------------------------------------------------------------------
const cgSizeF & cgBillboardObject::getSize( ) const
{
    return mSize;
}

//-----------------------------------------------------------------------------
//  Name : setSize()
/// <summary>
/// Set the local base size of the billboard (pre-scale).
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::setSize( const cgSizeF & size )
{
    // Is this a no-op?
    if ( mSize == size )
        return;

    // ToDo: database
    
    // Update value.
    mSize = size;

    // Notify listeners that property was altered
    static const cgString strContext = _T("Size");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getScale()
/// <summary>
/// Retrieve the scaling factor to apply to the billboard.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector2 & cgBillboardObject::getScale( ) const
{
    return mScale;
}

//-----------------------------------------------------------------------------
//  Name : setScale()
/// <summary>
/// Set the scaling factor to apply to the billboard.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::setScale( const cgVector2 & scale )
{
    // Is this a no-op?
    if ( mScale == scale )
        return;

    // ToDo: database
    
    // Update value.
    mScale = scale;

    // Notify listeners that property was altered
    static const cgString strContext = _T("Scale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getColor()
/// <summary>
/// Retrieve the color (and alpha) of the billboard.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBillboardObject::getColor( ) const
{
    return mColor;
}

//-----------------------------------------------------------------------------
//  Name : setColor()
/// <summary>
/// Set the color (and alpha) of the billboard, used to tint the base texture
/// during rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::setColor( cgUInt32 color )
{
    // Is this a no-op?
    if ( mColor == color )
        return;

    // ToDo: database
    
    // Update value.
    mColor = color;

    // Notify listeners that property was altered
    static const cgString strContext = _T("Color");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getRotation()
/// <summary>
/// Retrieve the rotation angle of the billboard in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBillboardObject::getRotation( ) const
{
    return mRotation;
}

//-----------------------------------------------------------------------------
//  Name : setRotation()
/// <summary>
/// Set the rotation angle of the billboard in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::setRotation( cgFloat degrees )
{
    // Is this a no-op?
    if ( mRotation == degrees )
        return;

    // ToDo: database
    
    // Update value.
    mRotation = degrees;

    // Notify listeners that property was altered
    static const cgString strContext = _T("Rotation");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getHDRScale()
/// <summary>
/// Retrieve the intensity scalar to apply during HDR rendering.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBillboardObject::getHDRScale( ) const
{
    return mHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : setHDRScale()
/// <summary>
/// Set the intensity scalar to apply during HDR rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::setHDRScale( cgFloat scale )
{
    // Is this a no-op?
    if ( mHDRScale == scale )
        return;

    // ToDo: database
    
    // Update value.
    mHDRScale = scale;

    // Notify listeners that property was altered
    static const cgString strContext = _T("HDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : isBillboardAligned()
/// <summary>
/// Determine if this billboard should be aligned to the node's Z axis.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardObject::isBillboardAligned( ) const
{
    return mAlignBillboard;
}

//-----------------------------------------------------------------------------
//  Name : enableBillboardAlignment()
/// <summary>
/// Select whether or not this billboard should be aligned to the node's Z axis.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::enableBillboardAlignment( bool enable )
{
    // Is this a no-op?
    if ( mAlignBillboard == enable )
        return;

    // ToDo: database
    
    // Update value.
    mAlignBillboard = enable;

    // Notify listeners that property was altered
    static const cgString strContext = _T("BillboardAlignment");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : isRenderable ()
/// <summary>
/// Determine if this object is currently renderable or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardObject::isRenderable() const
{
	// All billboards can render by default.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Render the billboard object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardObject::render( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer )
{
    cgRenderDriver * driver = pIssuer->getScene()->getRenderDriver();
    driver->setWorldTransform( CG_NULL );
    cgBillboardBuffer * buffer = static_cast<cgBillboardNode*>(pIssuer)->getBuffer();
    buffer->render( );

    // Drawn
    return true;
}

//-----------------------------------------------------------------------------
// Name : renderSubset ()
/// <summary>
/// Render only the section of this object that relates to the specified 
/// material (used for material batched / sorted rendering).
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardObject::renderSubset( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer, const cgMaterialHandle & hMaterial )
{
    // Route 'null' subset through to standard render.
    if ( !hMaterial.isValid() )
        return render( pCamera, pVisData, pIssuer );

    // Invalid subset
    return false;
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardObject::insertComponentData( )
{
    /*if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("DummyObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertDummy.bindParameter( 1, mReferenceId );
        mInsertDummy.bindParameter( 2, mSize );
        mInsertDummy.bindParameter( 3, mSoftRefCount );

        // Execute
        if ( !mInsertDummy.step( true ) )
        {
            cgString strError;
            mInsertDummy.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for dummy object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("DummyObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("DummyObject::insertComponentData") );

    } // End if !internal*/

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
bool cgBillboardObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    /*// Load the billboard data.
    prepareQueries();
    mLoadDummy.bindParameter( 1, e->sourceRefId );
    if ( !mLoadDummy.step( ) || !mLoadDummy.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( mLoadDummy.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for dummy object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for dummy object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadDummy.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadDummy;

    // Update our local members
    mLoadDummy.getColumn( _T("DisplaySize"), mSize );

    // Call base class implementation to read remaining data.
    if ( !cgWorldObject::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        if ( !insertComponentData() )
            return false;

    } // End if cloned*/

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardObject::prepareQueries()
{
    /*// Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertBillboard.isPrepared() == false )
            mInsertBillboard.prepare( mWorld, _T("INSERT INTO 'Objects::Billboard' VALUES(?1,?2,?3)"), true );
        if ( mUpdateSize.isPrepared() == false )
            mUpdateSize.prepare( mWorld, _T("UPDATE 'Objects::Billboard' SET DisplaySize=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadBillboard.isPrepared() == false )
        mLoadBillboard.prepare( mWorld, _T("SELECT * FROM 'Objects::Billboard' WHERE RefId=?1"), true );*/
}

///////////////////////////////////////////////////////////////////////////////
// cgBillboardNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBillboardNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardNode::cgBillboardNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Initialize variables
    mBillboardBuffer = CG_NULL;
    mBillboard       = CG_NULL;

    // Automaticaly assign to the 'Effects' render class.
    mRenderClassId = pScene->getRenderClassId( _T("Effects") );

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Billboard%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgBillboardNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardNode::cgBillboardNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables
    mBillboardBuffer = CG_NULL;
    mBillboard       = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgBillboardNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardNode::~cgBillboardNode()
{
    // Release allocated memory.
    // Releasing the buffer automatically releases
    // any billboards that were assigned to it.
    if ( mBillboardBuffer )
        mBillboardBuffer->scriptSafeDispose();

    // Clear variables
    mBillboardBuffer = CG_NULL;
    mBillboard = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgBillboardNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgBillboardNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgBillboardNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgBillboardNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BillboardNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    static const cgString BillboardAlignment = _T("BillboardAlignment");
    if ( e->context == BillboardAlignment )
    {
        // If the alignment property has changed, we need to recreate 
        // the buffer entirely. First backup reference to old billboard
        // buffer rather than release it immediately so that any common
        // resources (texture, shader, etc.) are not unloaded and reloaded.
        cgBillboardBuffer * oldBuffer = mBillboardBuffer;
        mBillboardBuffer = CG_NULL;
        mBillboard = CG_NULL;

        // Recreate the billboard with the new properties
        createBillboard();

        // Destroy the old billboard data (releasing the buffer
        // automatically destroys the billboards associated with it).
        if ( oldBuffer )
            oldBuffer->scriptSafeDispose();

    } // End if alignment
    else if ( mBillboard )
    {
        // Set new properties
        mBillboard->setPosition( getPosition(false) );
        mBillboard->setScale( getScale() );
        mBillboard->setSize( getSize() );
        mBillboard->setColor( getColor() );
        mBillboard->setRotation( getRotation() );
        mBillboard->setHDRScale( getHDRScale() );
        mBillboard->update();

        // ToDo: Just mark as dirty and call 'update' on render
        // in case of chained modifications?

    } // End if other changes

    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardNode::onNodeCreated( const cgUID & ObjectType, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation.
    if ( !cgObjectNode::onNodeCreated( ObjectType, CloneMethod ) )
        return false;

    // Create the billboard.
    createBillboard();

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
bool cgBillboardNode::onNodeLoading( const cgUID & ObjectType, cgWorldQuery * pNodeData, cgSceneCell * pParentCell, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeLoading( ObjectType, pNodeData, pParentCell, CloneMethod ) )
        return false;

    // Create the billboard
    createBillboard();

    // Success!
    return true;
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
bool cgBillboardNode::registerVisibility( cgVisibilitySet * pSet )
{
    // Allow base class to perform basic tests against filters and
    // add itself to the list of visible objects where necessary.
    if ( !cgObjectNode::registerVisibility( pSet ) )
        return false;

    // Register with the 'default' (null) material so that this node 
    // can be included in standard material batched / queue based rendering.
    cgUInt32 nFlags = pSet->getSearchFlags();
    if ( nFlags & cgVisibilitySearchFlags::CollectMaterials )
        pSet->addVisibleMaterial( cgMaterialHandle::Null, this );
    
    // We modified the visibility set.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getBuffer ()
/// <summary>
/// Retrieve the underlying billboard buffer housing this object's billboard.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardBuffer * cgBillboardNode::getBuffer( )
{
    return mBillboardBuffer;
}

//-----------------------------------------------------------------------------
//  Name : createBillboard () (Protected)
/// <summary>
/// Create the underlying billboard data necessary to render this billboard
/// object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardNode::createBillboard( )
{
    // Select creation flags.
    cgUInt32 flags = 0;
    if ( isBillboardAligned() )
        flags |= cgBillboardBuffer::OrientationAxis;

    // Create the billboard buffer.
    mBillboardBuffer = new cgBillboardBuffer();
    if ( !mBillboardBuffer->prepareBufferFromAtlas( flags, getScene()->getRenderDriver(), _T("sys://Textures/Fire.xml"), _T("sys://Shaders/Particles.sh") ) )
    {
        delete mBillboardBuffer;
        return false;
    
    } // End if failed

    // Attach a new 3D billboard.
    mBillboard = new cgBillboard3D();
    mBillboardBuffer->addBillboard( mBillboard );

    // Set initial properties
    mBillboard->setPosition( getPosition(false) );
    mBillboard->setDirection( getZAxis(false) );
    mBillboard->setScale( getScale() );
    mBillboard->setSize( getSize() );
    mBillboard->setColor( getColor() );
    mBillboard->setRotation( getRotation() );
    mBillboard->setHDRScale( getHDRScale() );
    mBillboard->update();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform() (Override)
/// <summary>
/// Update our internal cell matrix with that specified here.
/// Note : Hooked into base class so that we can flag our own matrices as dirty.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgObjectNode::setCellTransform( Transform, Source ) )
        return false;

    // Update the position and orientation of our billboard.
    if ( mBillboard )
    {
        mBillboard->setPosition( getPosition(false) );
        mBillboard->setDirection( getZAxis(false) );
        mBillboard->update();

    } // End if valid

    // Success!
    return true;
}