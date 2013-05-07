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
// Name : cgTargetObject.cpp                                                 //
//                                                                           //
// Desc : Contains simple classes responsible for providing a 'target' point //
//        with which objects can be reorientated. This is essentially a      //
//        'look at' point that will, for instance, manipulate a parent spot  //
//        light or camera object's matrices to face it.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgTargetObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgTargetObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgScene.h>
#include <World/cgSceneCell.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Interface/cgUIManager.h>
#include <Math/cgCollision.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgTargetObject::mInsertTarget;
cgWorldQuery cgTargetObject::mUpdateAlignment;
cgWorldQuery cgTargetObject::mLoadTarget;

///////////////////////////////////////////////////////////////////////////////
// cgTargetObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTargetObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTargetObject::cgTargetObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize members to sensible defaults
    mTargetingMode = cgNodeTargetMethod::NoTarget;
}

//-----------------------------------------------------------------------------
//  Name : cgTargetObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTargetObject::cgTargetObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgTargetObject * pObject = (cgTargetObject*)pInit;
    mTargetingMode = pObject->mTargetingMode;
}

//-----------------------------------------------------------------------------
//  Name : ~cgTargetObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTargetObject::~cgTargetObject()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgTargetObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release managed resources.

    // Dispose base.
    if ( bDisposeBase )
        cgWorldObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgTargetObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgTargetObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgTargetObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgTargetObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgTargetObject::getLocalBoundingBox( )
{
    // Set to current (object space) position by default
    return cgBoundingBox::Empty;
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Retrieve useful values
    cgFloat fZoomFactor  = pCamera->estimateZoomFactor( ViewportSize, pIssuer->getPosition(), 2.5f );
    cgFloat fSize = fZoomFactor * 5.0f;

    // Targets are picked as if they were solid boxes.
    cgBoundingBox Bounds( -fSize, -fSize, -fSize, fSize, fSize, fSize );
    Bounds.inflate( pCamera->estimatePickTolerance( ViewportSize, fWireTolerance, Bounds.getCenter(), pIssuer->getWorldTransform(false) ) );
    return Bounds.intersect( vOrigin, vDir, fDistance, false );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgTargetObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // Local Variables
    const cgUInt32      ColorX = 0xFFC00000, ColorY = 0xFF00B000, ColorZ = 0xFF0000C0;
    cgShadedVertex      Points[8];
    cgUInt32            Indices[48];
    cgUInt32            i;
    bool                bCull;

    // Get access to required systems.
    cgRenderDriver  * pDriver = cgRenderDriver::getInstance();
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);

    // Set the object's transformation matrix to the device 
    // (representation will be constructed in object space)
    cgTransform InverseObjectTransform;
    const cgTransform & ObjectTransform = pIssuer->getWorldTransform();
    cgTransform::inverse( InverseObjectTransform, ObjectTransform );
    pDriver->setWorldTransform( ObjectTransform );

    // Retrieve useful values
    const cgViewport & Viewport = pDriver->getViewport();
    cgFloat     fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, pIssuer->getPosition(), 2.5f );
    cgFloat     fSize       = fZoomFactor * 5.0f;
    cgFloat     fGuideSize  = fZoomFactor * 20.0f;
    cgUInt32    nColor      = (pIssuer->isSelected()) ? 0xFFFFFFFF : 0xFFFFF600;
    bool        bOrtho      = (pCamera->getProjectionMode() == cgProjectionMode::Orthographic);

    // We'll render each of the cube faces as line lists with manual back face culling.
    // Because we construct in object space, transform camera culling details into object space too.
    cgVector3 vCameraLook, vCameraPos;
    InverseObjectTransform.transformNormal( vCameraLook, pCamera->getZAxis( false ) );
    InverseObjectTransform.transformCoord( vCameraPos, pCamera->getPosition( false ) );
    cgVector3::normalize( vCameraLook, vCameraLook );

    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    for ( i = 0; i < 8; ++i )
        Points[i].color = nColor;
    
    // Construct the target object (cube) vertices.
    Points[0].position = cgVector3( -fSize,  fSize, -fSize );
    Points[1].position = cgVector3( -fSize,  fSize,  fSize );
    Points[2].position = cgVector3(  fSize,  fSize,  fSize );
    Points[3].position = cgVector3(  fSize,  fSize, -fSize );
    Points[4].position = cgVector3( -fSize, -fSize, -fSize );
    Points[5].position = cgVector3( -fSize, -fSize,  fSize );
    Points[6].position = cgVector3(  fSize, -fSize,  fSize );
    Points[7].position = cgVector3(  fSize, -fSize, -fSize );
    
    // +X Face
    if ( bOrtho )
        bCull = (cgVector3::dot( cgVector3( 1,0,0 ), vCameraLook ) >= 0.0f );
    else
        bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 1,0,0 ), -fSize ) != cgPlaneQuery::Front);
    
    // Build indices
    cgUInt32 nCount = 0;
    if ( !bCull )
    {
        Indices[nCount++] = 3; Indices[nCount++] = 2;
        Indices[nCount++] = 2; Indices[nCount++] = 6;
        Indices[nCount++] = 6; Indices[nCount++] = 7;
        Indices[nCount++] = 7; Indices[nCount++] = 3;

    } // End if !cull

    // -X Face
    if ( bOrtho )
        bCull = (cgVector3::dot( cgVector3( -1,0,0 ), vCameraLook ) >= 0.0f );
    else
        bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( -1,0,0 ), fSize ) != cgPlaneQuery::Front);
    
    // Build indices
    if ( !bCull )
    {
        Indices[nCount++] = 1; Indices[nCount++] = 0;
        Indices[nCount++] = 0; Indices[nCount++] = 4;
        Indices[nCount++] = 4; Indices[nCount++] = 5;
        Indices[nCount++] = 5; Indices[nCount++] = 1;

    } // End if !cull

    // +Z Face
    if ( bOrtho )
        bCull = (cgVector3::dot( cgVector3( 0,0,1 ), vCameraLook ) >= 0.0f );
    else
        bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,1 ), -fSize ) != cgPlaneQuery::Front);
    
    // Build indices
    if ( !bCull )
    {
        Indices[nCount++] = 2; Indices[nCount++] = 1;
        Indices[nCount++] = 1; Indices[nCount++] = 5;
        Indices[nCount++] = 5; Indices[nCount++] = 6;
        Indices[nCount++] = 6; Indices[nCount++] = 2;

    } // End if !cull

    // -Z Face
    if ( bOrtho )
        bCull = (cgVector3::dot( cgVector3( 0,0,-1 ), vCameraLook ) >= 0.0f );
    else
        bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,-1 ), fSize ) != cgPlaneQuery::Front);
    
    // Build indices
    if ( !bCull )
    {
        Indices[nCount++] = 0; Indices[nCount++] = 3;
        Indices[nCount++] = 3; Indices[nCount++] = 7;
        Indices[nCount++] = 7; Indices[nCount++] = 4;
        Indices[nCount++] = 4; Indices[nCount++] = 0;

    } // End if !cull

    // +Y Face
    if ( bOrtho )
        bCull = (cgVector3::dot( cgVector3( 0,1,0 ), vCameraLook ) >= 0.0f );
    else
        bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,1,0 ), -fSize ) != cgPlaneQuery::Front);
    
    // Build indices
    if ( !bCull )
    {
        Indices[nCount++] = 0; Indices[nCount++] = 1;
        Indices[nCount++] = 1; Indices[nCount++] = 2;
        Indices[nCount++] = 2; Indices[nCount++] = 3;
        Indices[nCount++] = 3; Indices[nCount++] = 0;

    } // End if !cull

    // -Y Face
    if ( bOrtho )
        bCull = (cgVector3::dot( cgVector3( 0,-1,0 ), vCameraLook ) >= 0.0f );
    else
        bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,-1,0 ), fSize ) != cgPlaneQuery::Front);
    
    // Build indices
    if ( !bCull )
    {
        Indices[nCount++] = 7; Indices[nCount++] = 6;
        Indices[nCount++] = 6; Indices[nCount++] = 5;
        Indices[nCount++] = 5; Indices[nCount++] = 4;
        Indices[nCount++] = 4; Indices[nCount++] = 7;

    } // End if !cull

    // Begin rendering
    cgObjectNode * pTargetingNode = CG_NULL;
    bool bWireframe = (flags & cgSandboxRenderFlags::Wireframe);
    pDriver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    pShader->setBool( _T("wireViewport"), bWireframe );
    if ( pShader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( pShader->executeTechniquePass() != cgTechniqueResult::Abort )
        {
            // Draw the cube.
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineList, 0, 8, nCount / 2, Indices, cgBufferFormat::Index32, Points );
    
            // Also draw a line to the center of the targeting node.
            pTargetingNode = ((cgTargetNode*)pIssuer)->getTargetingNode();
            if ( pTargetingNode )
            {
                nColor             = (pIssuer->isSelected() || pTargetingNode->isSelected()) ? 0xFF4C6672 : 0xFF99CCE5;
                Points[0].position = cgVector3(0,0,0);
                Points[0].color    = nColor;
                InverseObjectTransform.transformCoord( Points[1].position, pTargetingNode->getPosition() );
                Points[1].color    = nColor;
                pDriver->drawPrimitiveUP( cgPrimitiveType::LineList, 1, Points );

                // Draw axis guide lines if we are selected. These axes represent those
                // of the targeting node, not the target node itself.
                if ( pIssuer->isSelected() || pTargetingNode->isSelected() )
                {
                    const cgUInt32 ColorX = 0xFFC00000, ColorY = 0xFF00B000, ColorZ = 0xFF0000C0;
                    
                    // X axis
                    cgVector3 vAxis, vPos = pIssuer->getPosition();
                    cgVector3::normalize( vAxis, pTargetingNode->getXAxis() );
                    Points[0].position = vPos;
                    Points[0].color    = ColorX;
                    Points[1].position = vPos + (vAxis * fGuideSize);
                    Points[1].color    = ColorX;
                    
                    // Y axis
                    cgVector3::normalize( vAxis, pTargetingNode->getYAxis() );
                    Points[2].position = vPos;
                    Points[2].color    = ColorY;
                    Points[3].position = vPos + (vAxis * fGuideSize);
                    Points[3].color    = ColorY;
                    
                    // Z axis
                    cgVector3::normalize( vAxis, pTargetingNode->getZAxis() );
                    Points[4].position = vPos;
                    Points[4].color    = ColorZ;
                    Points[5].position = vPos + (vAxis * fGuideSize);
                    Points[5].color    = ColorZ;

                    // Issue draw
                    pDriver->setWorldTransform( NULL );
                    pDriver->drawPrimitiveUP( cgPrimitiveType::LineList, 3, Points );
                    
                } // End if selected
            
            } // End if has parent light

        } // End if execute success

        // We have finished rendering
        pShader->endTechnique();

    } // End if beginTechnique()

    // We also need to draw the axis labels
    if ( pTargetingNode && (pIssuer->isSelected() || pTargetingNode->isSelected()) )
    {
        const cgViewport & Viewport = pDriver->getViewport();
        
        // Select the correct system font before we start printing in case 
        // another part of the system has modified it.
        cgUIManager * pUIManager = cgUIManager::getInstance();
        pUIManager->selectFont( _T("fixed_v01_white_plain") );

        // Draw labels for each axis at the end point.
        cgVector3 vTip, vAxis, vPos = pIssuer->getPosition();
        cgVector3::normalize( vAxis, pTargetingNode->getXAxis() );
        if ( pCamera->worldToViewport( Viewport.size, vPos + (vAxis * fGuideSize), vTip ) )
            pUIManager->printText( (cgInt32)(vTip.x + Viewport.x), (cgInt32)(vTip.y + Viewport.y) - 9, _T("x"), 0, ColorX );
        cgVector3::normalize( vAxis, pTargetingNode->getYAxis() );
        if ( pCamera->worldToViewport( Viewport.size, vPos + (vAxis * fGuideSize), vTip ) )
            pUIManager->printText( (cgInt32)(vTip.x + Viewport.x), (cgInt32)(vTip.y + Viewport.y) - 9, _T("y"), 0, ColorY );
        cgVector3::normalize( vAxis, pTargetingNode->getZAxis() );
        if ( pCamera->worldToViewport( Viewport.size, vPos + (vAxis * fGuideSize), vTip ) )
            pUIManager->printText( (cgInt32)(vTip.x + Viewport.x), (cgInt32)(vTip.y + Viewport.y) - 9, _T("z"), 0, ColorZ );
            
    } // End if selected

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
void cgTargetObject::applyObjectRescale( cgFloat fScale )
{
    // Nothing to re-scale in this implementation

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
bool cgTargetObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_TargetObject )
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
cgString cgTargetObject::getDatabaseTable( ) const
{
    return _T("Objects::Target");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetObject::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgTargetObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("TargetObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertTarget.bindParameter( 1, mReferenceId );
        mInsertTarget.bindParameter( 2, (cgInt32)mTargetingMode );
        mInsertTarget.bindParameter( 3, mSoftRefCount );

        // Execute
        if ( !mInsertTarget.step( true ) )
        {
            cgString strError;
            mInsertTarget.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for target object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("TargetObject::insertComponentData") );
            return false;

        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("TargetObject::insertComponentData") );

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
bool cgTargetObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the dummy data.
    prepareQueries();
    mLoadTarget.bindParameter( 1, e->sourceRefId );
    if ( !mLoadTarget.step( ) || !mLoadTarget.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( mLoadTarget.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for target object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for target object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadTarget.reset();
        return false;

    } // End if failed

    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadTarget;

    // Update our local members
    cgInt32 alignment = 0;
    mLoadTarget.getColumn( _T("Alignment"), alignment );
    mTargetingMode = (cgNodeTargetMethod::Base)alignment;

    // Call base class implementation to read remaining data.
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
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgTargetObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertTarget.isPrepared() == false )
            mInsertTarget.prepare( mWorld, _T("INSERT INTO 'Objects::Target' VALUES(?1,?2,?3)"), true );
        if ( mUpdateAlignment.isPrepared() == false )
            mUpdateAlignment.prepare( mWorld, _T("UPDATE 'Objects::Target' SET Alignment=?1 WHERE RefId=?2"), true );

    } // End if sandbox

    // Read queries
    if ( mLoadTarget.isPrepared() == false )
        mLoadTarget.prepare( mWorld, _T("SELECT * FROM 'Objects::Target' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : getMode ( )
/// <summary>
/// Get the method that will be used to reorientate the targeting object.
/// </summary>
//-----------------------------------------------------------------------------
cgNodeTargetMethod::Base cgTargetObject::getMode( ) const
{
    return mTargetingMode;
}

//-----------------------------------------------------------------------------
// Name : setMode ( )
/// <summary>
/// Set the method that will be used to reorientate the targeting object.
/// </summary>
//-----------------------------------------------------------------------------
void cgTargetObject::setMode( cgNodeTargetMethod::Base Mode )
{
    // Is this a no-op?
    if ( mTargetingMode == Mode )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateAlignment.bindParameter( 1, (cgInt32)Mode );
        mUpdateAlignment.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( !mUpdateAlignment.step( true ) )
        {
            cgString strError;
            mUpdateAlignment.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update alignment of target object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mTargetingMode = Mode;

    // Notify listeners that property was altered
    static const cgString strContext = _T("Mode");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

///////////////////////////////////////////////////////////////////////////////
// cgTargetNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTargetNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTargetNode::cgTargetNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults.
    mTargetingNode    = CG_NULL;
    mTargetUpdated    = false;
    mUpdating         = false;

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Target%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgTargetNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTargetNode::cgTargetNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables to sensible defaults.
    mTargetingNode    = CG_NULL;
    mTargetUpdated    = false;
    mUpdating         = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgTargetNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTargetNode::~cgTargetNode()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgTargetNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear variables
    mTargetingNode    = CG_NULL;
    mTargetUpdated    = false;
    mUpdating         = false;

    // Dispose base.
    if ( bDisposeBase )
        cgObjectNode::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgTargetNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgTargetNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgTargetNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgTargetNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_TargetNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : canSetName ( )
/// <summary>Determine if name setting is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::canSetName( ) const
{
    // Cannot set the name of target nodes.
    return false;
}

//-----------------------------------------------------------------------------
// Name : canDelete ( )
/// <summary>Determine if deletion is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::canDelete( ) const
{
    // Target nodes cannot be deleted explicitly unless they 
    // are no longer attached to a targeting node.
    return (!mTargetingNode);
}

//-----------------------------------------------------------------------------
// Name : canClone ( )
/// <summary>
/// Determine if node's of this type can be cloned.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::canClone( ) const
{
    // Target nodes cannot be cloned explicitly unless they
    // are no longer attached to a targeting node.
    return (!mTargetingNode);
}

//-----------------------------------------------------------------------------
//  Name : getName() (Virtual)
/// <summary>
/// Retrieve the name of this instance of the specified object node.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgTargetNode::getName( ) const
{
    if ( !mTargetingNode )
        return cgString::Empty;
    return mTargetingNode->getName() + _T(".Target");
}

//-----------------------------------------------------------------------------
// Name : validateAttachment ( ) (Virtual)
/// <summary>
/// Allows nodes to describe whether or not a particular attachment of
/// another node as either a child or parent of this node is valid.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::validateAttachment( cgObjectNode * pNode, bool bNodeAsChild )
{
    return true;
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::canScale( ) const
{
    // Targets cannot be scaled.
    return false;
}

//-----------------------------------------------------------------------------
// Name : canAdjustPivot ( )
/// <summary>
/// Determine if separation of this node's pivot and object space is allowed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::canAdjustPivot( ) const
{
    // Targets cannot have their pivot adjusted.
    return false;
}

//-----------------------------------------------------------------------------
// Name : getTargetingNode ( )
/// <summary>
/// Retrieve the node that this target will be reorientating whenever its
/// position is manipulated.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgTargetNode::getTargetingNode( ) const
{
    return mTargetingNode;
}

//-----------------------------------------------------------------------------
// Name : setTargetingNode ( )
/// <summary>
/// Set the node that this target will be reorientating whenever its
/// position is manipulated. Note: Passing a value of CG_NULL to detach the
/// target will also automatically destroy the target node. Take care not to
/// call any further methods past this point.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::setTargetingNode( cgObjectNode * pNode )
{
    // Is this a no-op?
    if ( mTargetingNode == pNode )
        return true;

    // Cannot attach to other target nodes (including self).
    if ( pNode && pNode->queryReferenceType( RTID_TargetNode ) )
        return false;

    // Update value.
    mTargetingNode = pNode;

    // If we no longer have a targeting node, this target should be 
    // deleted from the scene. Otherwise, trigger the scene's node
    // name change notification (name of target is based on targeting
    // node's name).
    if ( !mTargetingNode && mParentScene )
        mParentScene->deleteObjectNode( this );
    else if ( mParentScene )
        mParentScene->onNodeNameChange( &cgNodeUpdatedEventArgs( mParentScene, this ) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : setTargetMethod ( ) (Virtual)
/// <summary>
/// Creates a target node that the application can use to manipulate the
/// orientation of this node by adjusting the position of that node. Useful
/// for light sources, cameras, bones, character's head / eyes, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgTargetNode::setTargetMethod( cgNodeTargetMethod::Base Mode )
{
    // DISABLE BEHAVIOR. Targets cannot themselves have a target.
    return;
}

//-----------------------------------------------------------------------------
// Name : adjustTargetingTransform ( )
/// <summary>
/// Perform the necessary adjustments to the input transform in order to have
/// it correctly orientate toward this node.
/// </summary>
//-----------------------------------------------------------------------------
void cgTargetNode::adjustTargetingTransform( cgTransform & TargetingTransform )
{
    // If our targeting node is also our parent then we only need to
    // update if the transform modification originated at the parent.
    if ( mTargetingNode == mParentNode && !mUpdating )
        return;

    // First we need to make sure that this target node is orientated 
    // correctly based on the current or new targeting node location.
    cgTransform TargetTransform = ( mTargetUpdated ) ? mNewTargetTransform : mCellTransform;
    adjustTargetTransform( TargetTransform, TargetingTransform );

    // No-op?
    cgNodeTargetMethod::Base Mode = getMode();
    if ( Mode == cgNodeTargetMethod::Unlocked )
        return;

    // Generate correct orientation vectors based on the target's
    // new orientation and the chosen method.
    cgVector3 vRight = TargetTransform.xUnitAxis();
    cgVector3 vUp    = TargetTransform.yUnitAxis();
    cgVector3 vLook  = TargetTransform.zUnitAxis();
    switch ( Mode )
    {
        case cgNodeTargetMethod::XAxis:
        {
            cgVector3 vTemp = vRight;
            vRight = vLook;
            vLook  = -vTemp;
            break;
        
        } // End Case X
        case cgNodeTargetMethod::YAxis:
        {
            cgVector3 vTemp = vUp;
            vUp = vLook;
            vLook  = -vTemp;
            break;
        
        } // End Case Y
        case cgNodeTargetMethod::Relative:
        {
            break;

        } // End Case Relative
    
    } // End Switch Method

    // Store new orientation vectors
    TargetingTransform.setOrientation( vRight, vUp, vLook );
}

//-----------------------------------------------------------------------------
// Name : adjustTargetTransform ( ) (Protected)
/// <summary>
/// Perform the necessary adjustments of this node's transform in order to have
/// it correctly orientate away from the targeting node.
/// </summary>
//-----------------------------------------------------------------------------
void cgTargetNode::adjustTargetTransform( cgTransform & TargetTransform, const cgTransform & TargetingTransform )
{
    // When we are attached to a targeting node only 'rolls' and 
    // position changes are allowed. Enforce these rules accordingly.
    // First get the relevant components.
    cgVector3 vTargetPos    = TargetTransform.position();
    cgVector3 vTargetingPos = TargetingTransform.position();
    cgVector3 vUpAlign      = TargetTransform.yUnitAxis();

    // Transform targeting position such that it is relative to this node's
    // parent cell rather than its own.
    if ( mParentCell && mTargetingNode->getCell() )
        vTargetingPos += (mTargetingNode->getCell()->getWorldOrigin() - mParentCell->getWorldOrigin());
    else if ( !mParentCell && mTargetingNode->getCell() )
        vTargetingPos += mTargetingNode->getCell()->getWorldOrigin();
    else if ( mParentCell )
        vTargetingPos -= mParentCell->getWorldOrigin();

    // Generate the new transform for the target.
    TargetTransform.lookAt( vTargetPos, vTargetPos + (vTargetPos - vTargetingPos), vUpAlign );
    
    // Update the target node's transformation matrix.
    mTargetUpdated = true;
    setCellTransform( TargetTransform );
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform() (Override)
/// <summary>
/// Update our internal cell matrix with that specified here.
/// Note : Hooked into base class so that we can updatate our own properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    if ( Source == cgTransformSource::TransformResolve )
    {
        // Ignore updates from parent if this was the node ultimately
        // responsible for triggering its transformation.
        if ( mParentNode != mTargetingNode )
            return cgObjectNode::setCellTransform( Transform, Source );
        return true;

    } // End if TransformResolve
    else
    {
        if ( mTargetUpdated || !mTargetingNode )
        {
            // Target update has been comitted.
            mTargetUpdated = false;

            // Targeting node is requesting the update via 'adjustTargetingTransform()'.
            // Just call  base class implementation with the specified transform.
            return cgObjectNode::setCellTransform( Transform, Source );

        } // End if targeting node update
        else
        {
            
            // If we are the parent of the targeting node then we only need to
            // update if the transform modification originated at the child.
            if ( mTargetingNode->getParent() == this )
                return cgObjectNode::setCellTransform( Transform, Source );
            
            // The target (this node) has been updated and needs to have its 
            // information recomputed based on this specified transform when
            // the targeting node requests that we adjust its transform. If the
            // update did not originate here (i.e. the targeting node was
            // modified directly) then the current transform will be used.
            mTargetUpdated = true;
            mNewTargetTransform = Transform;

            // Trigger the transformation method of the targeting node.
            // This method automatically queries the target for an updated 
            // transform via 'adjustTargetingTransform()' which will automatically
            // trigger our own reorientation as appropriate.
            mUpdating = true;
            mTargetingNode->setCellTransform( mTargetingNode->getCellTransform() );
            mUpdating = false;

            // Success!
            return true;
        
        } // End if target (this) node update

    } // End if !TransformResolve
}

//-----------------------------------------------------------------------------
//  Name : isUpdating()
/// <summary>
/// Determine if the target node is in the process of updating its
/// transformation or that of its parent.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTargetNode::isUpdating( ) const
{
    return mUpdating;
}