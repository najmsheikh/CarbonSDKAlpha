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
// Name : cgBoneObject.cpp                                                   //
//                                                                           //
// Desc : Contains classes that allow for rigging of skinned meshes with     //
//        bone information. These bones can then be animated independantly   //
//        using the animation controller, in order to affect the skinned     //
//        mesh to which they are assigned.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBoneObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgBoneObject.h>
#include <World/cgScene.h>
#include <Math/cgMathUtility.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgMesh.h>
#include <Resources/cgResourceManager.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgBoneObject::mInsertBone;
cgWorldQuery cgBoneObject::mUpdateProperties;
cgWorldQuery cgBoneObject::mLoadBone;

//-----------------------------------------------------------------------------
//  Name : cgBoneObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneObject::cgBoneObject( cgUInt32 referenceId, cgWorld * world ) : cgWorldObject( referenceId, world )
{
    // Initialize members to sensible defaults
    mWidth    = 0.15f;
    mHeight   = 0.15f;
    mLength   = 0.0f; // Should be 0.0 for 'NewLength - OldLength' child update process to function on first time creation.
    mTaper    = 0.9f;
}

//-----------------------------------------------------------------------------
//  Name : cgBoneObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneObject::cgBoneObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgWorldObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    cgBoneObject * pObject = (cgBoneObject*)init;
    mWidth    = pObject->mWidth;
    mHeight   = pObject->mHeight;
    mLength   = pObject->mLength;
    mTaper    = pObject->mTaper;
}

//-----------------------------------------------------------------------------
// Name : ~cgBoneObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneObject::~cgBoneObject()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgBoneObject::allocateNew( const cgUID & type, cgUInt32 referenceId, cgWorld * world )
{
    return new cgBoneObject( referenceId, world );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgBoneObject::allocateClone( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod )
{
    // Valid clone?
    return new cgBoneObject( referenceId, world, init, initMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgBoneObject::getLocalBoundingBox( )
{
    // Compute bounding box for bone
    return cgBoundingBox( cgVector3( 0.0f, -mHeight * 0.5f, -mWidth * 0.5f ),
                          cgVector3( mLength, mHeight * 0.5f, mWidth * 0.5f ) );
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::pick( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Generate the sandbox mesh as necessary.
    if ( !mSandboxMesh.isValid() )
    {
        if ( !createSandboxMesh() )
            return false;
    
    } // End if no mesh

    // Retrieve the underlying mesh resource and pick if available
    cgMesh * mesh = mSandboxMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
        return false;

    // Pass through
    return mesh->pick( rayOrigin, rayDirection, wireframe, wireTolerance, distance );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::sandboxRender( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer )
{
    // Get access to required systems.
    cgRenderDriver * driver = mWorld->getRenderDriver();

    // Generate the sandbox mesh as necessary.
    if ( !mSandboxMesh.isValid() )
    {
        if ( !createSandboxMesh() )
            return;
    
    } // End if no mesh

    // Retrieve the underlying rendering resources if available
    cgMesh * mesh = mSandboxMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
    {
        cgWorldObject::sandboxRender( camera, visibilityData, wireframe, gridPlane, issuer );
        return;
    
    } // End if no mesh

    // Setup constants.
    cgColorValue interiorColor = (!issuer->isSelected()) ? issuer->getNodeColor() : cgColorValue( 0.7f, 0.2f, 0.1f, 0.4f );
    interiorColor.a = 0.4f;
    cgColorValue wireColor = (!issuer->isSelected()) ? issuer->getNodeColor() : cgColorValue( 0xFFFFFFFF );
    cgConstantBuffer * constants = driver->getSandboxConstantBuffer().getResource( true );
    constants->setVector( _T("shapeInteriorColor"), (cgVector4&)interiorColor );
    constants->setVector( _T("shapeWireColor"), (cgVector4&)wireColor );
    driver->setConstantBufferAuto( driver->getSandboxConstantBuffer() );

    // Setup world transform (include scale from meters to world space).
    driver->setWorldTransform( issuer->getWorldTransform(false) );

    // Execute technique.
    cgSurfaceShader * shader = driver->getSandboxSurfaceShader().getResource(true);
    if ( shader->beginTechnique( _T("drawGhostedShapeMesh") ) )
    {
        while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
            mesh->draw( cgMeshDrawMode::Simple );
        shader->endTechnique();
    
    } // End if success

    // Call base class implementation last.
    cgWorldObject::sandboxRender( camera, visibilityData, wireframe, gridPlane, issuer );
}

//-----------------------------------------------------------------------------
// Name : createSandboxMesh() (Protected)
/// <summary>
/// Generate the physical representation of this bone for rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::createSandboxMesh( )
{
    // Compute a shape identifier that can be used to find duplicate meshes.
    cgString referenceName = cgString::format( _T("Core::SandboxShapes::Bone(w:%.4f,h:%.4f,l:%.4f,t:%.4f)"), mWidth, mHeight, mLength, mTaper );

    // Does a mesh with this identifier already exist?
    cgResourceManager * resources = mWorld->getResourceManager();
    if ( !resources->getMesh( &mSandboxMesh, referenceName ) )
    {
        // No mesh was found, construct a new one.
        cgVertex  vertices[32];
        cgUInt32  indices[42];
        cgVector3 points[4];
        cgPlane   plane;
        
        // Generate the unique vertex positions for the bone (in object space).
        // These are generated as separate triangles to account for the differing
        // vertex normals required.
        // Top near tip triangle
        cgVector3 offset   = cgVector3( (mWidth * 0.25f) + (mHeight * 0.25f), mHeight * 0.5f, -mWidth * 0.5f );
        points[0] = cgVector3( 0, 0, 0 );
        points[1] = cgVector3( offset.x, offset.y,-offset.z);
        points[2] = cgVector3( offset.x, offset.y, offset.z);
        cgPlane::fromPoints( plane, points[0], points[1], points[2] );

        // Store final vertices.
        cgUInt32 vertex = 0;
        for ( cgUInt32 i = 0; i < 3; ++i )
            vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );
        
        // Right near tip triangle
        points[1] = cgVector3( offset.x, offset.y, offset.z);
        points[2] = cgVector3( offset.x,-offset.y, offset.z);
        cgPlane::fromPoints( plane, points[0], points[1], points[2] );

        // Store geometry
        for ( cgUInt32 i = 0; i < 3; ++i )
            vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );

        // Bottom near tip triangle
        points[1] = cgVector3( offset.x, -offset.y, offset.z);
        points[2] = cgVector3( offset.x, -offset.y,-offset.z);
        cgPlane::fromPoints( plane, points[0], points[1], points[2] );

        // Store geometry
        for ( cgUInt32 i = 0; i < 3; ++i )
            vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );

        // Left near tip triangle
        points[1] = cgVector3( offset.x,-offset.y,-offset.z);
        points[2] = cgVector3( offset.x, offset.y,-offset.z);
        cgPlane::fromPoints( plane, points[0], points[1], points[2] );
        
        // Store geometry
        for ( cgUInt32 i = 0; i < 3; ++i )
            vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );

        // Generate indices for these 4 tip triangles
        cgUInt32 triangle = 0;
        for ( cgUInt32 i = 0; i < 4; ++i, triangle++ )
        {
            indices[(triangle*3)  ] = (triangle*3);
            indices[(triangle*3)+1] = (triangle*3)+1;
            indices[(triangle*3)+2] = (triangle*3)+2;
        
        } // Next Triangle
        
        // Now the long quads of the bone. First the top quad.
        offset    = cgVector3( mLength, (mHeight * 0.5f) * (1.0f - mTaper), 
                                 -((mWidth  * 0.5f) * (1.0f - mTaper)) );
        points[0] = vertices[2].position;
        points[1] = vertices[1].position;
        points[2] = cgVector3( offset.x, offset.y,-offset.z);
        points[3] = cgVector3( offset.x, offset.y, offset.z);
        cgPlane::fromPoints( plane, points[0], points[1], points[2] );

        // Generate indices
        indices[(triangle*3)]   = vertex;
        indices[(triangle*3)+1] = vertex+1;
        indices[(triangle*3)+2] = vertex+2;
        triangle++;
        indices[(triangle*3)]   = vertex;
        indices[(triangle*3)+1] = vertex+2;
        indices[(triangle*3)+2] = vertex+3;
        triangle++;

        // Store geometry
        for ( cgUInt32 i = 0; i < 4; ++i )
            vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );

        // Right quad
        points[0] = vertices[5].position;
        points[1] = vertices[4].position;
        points[2] = cgVector3( offset.x,  offset.y, offset.z);
        points[3] = cgVector3( offset.x, -offset.y, offset.z);
        cgPlane::fromPoints( plane, points[0], points[1], points[2] );

        // Generate indices
        indices[(triangle*3)]   = vertex;
        indices[(triangle*3)+1] = vertex+1;
        indices[(triangle*3)+2] = vertex+2;
        triangle++;
        indices[(triangle*3)]   = vertex;
        indices[(triangle*3)+1] = vertex+2;
        indices[(triangle*3)+2] = vertex+3;
        triangle++;

        // Store geometry
        for ( cgUInt32 i = 0; i < 4; ++i )
            vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );

        // Bottom quad
        points[0] = vertices[8].position;
        points[1] = vertices[7].position;
        points[2] = cgVector3( offset.x, -offset.y, offset.z);
        points[3] = cgVector3( offset.x, -offset.y,-offset.z);
        cgPlane::fromPoints( plane, points[0], points[1], points[2] );

        // Generate indices
        indices[(triangle*3)]   = vertex;
        indices[(triangle*3)+1] = vertex+1;
        indices[(triangle*3)+2] = vertex+2;
        triangle++;
        indices[(triangle*3)]   = vertex;
        indices[(triangle*3)+1] = vertex+2;
        indices[(triangle*3)+2] = vertex+3;
        triangle++;

        // Store geometry
        for ( cgUInt32 i = 0; i < 4; ++i )
            vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );

        // Left quad
        points[0] = vertices[11].position;
        points[1] = vertices[10].position;
        points[2] = cgVector3( offset.x, -offset.y,-offset.z);
        points[3] = cgVector3( offset.x,  offset.y,-offset.z);
        cgPlane::fromPoints( plane, points[0], points[1], points[2] );

        // Generate indices
        indices[(triangle*3)]   = vertex;
        indices[(triangle*3)+1] = vertex+1;
        indices[(triangle*3)+2] = vertex+2;
        triangle++;
        indices[(triangle*3)]   = vertex;
        indices[(triangle*3)+1] = vertex+2;
        indices[(triangle*3)+2] = vertex+3;
        triangle++;

        // Store geometry
        for ( cgUInt32 i = 0; i < 4; ++i )
            vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );

        // Finally, generate the end quad (if taper allows)
        if ( mTaper < 1.0f )
        {
            // Far quad
            points[0] = vertices[23].position;
            points[1] = vertices[22].position;
            points[2] = vertices[15].position;
            points[3] = vertices[14].position;
            cgPlane::fromPoints( plane, points[0], points[1], points[2] );

            // Generate indices
            indices[(triangle*3)]   = vertex;
            indices[(triangle*3)+1] = vertex+1;
            indices[(triangle*3)+2] = vertex+2;
            triangle++;
            indices[(triangle*3)]   = vertex;
            indices[(triangle*3)+1] = vertex+2;
            indices[(triangle*3)+2] = vertex+3;
            triangle++;

            // Store geometry
            for ( cgUInt32 i = 0; i < 4; ++i )
                vertices[vertex++] = cgVertex( points[i], (cgVector3&)plane );

        } // End if not fully tapered

        // Populate default triangle data
        cgMesh::TriangleArray triangleData( triangle );
        for ( cgUInt32 i = 0; i < triangle; ++i )
        {
            triangleData[i].indices[0] = indices[i*3];
            triangleData[i].indices[1] = indices[(i*3)+1];
            triangleData[i].indices[2] = indices[(i*3)+2];
            triangleData[i].dataGroupId = 0;
        
        } // Next Triangle

        // Dispose of any previous mesh generated and
        // allocate a new one.
        mSandboxMesh.close( true );
        cgMesh * newMesh = new cgMesh( 0, CG_NULL );
        
        // Set the mesh data format and populate it.
        cgResourceManager * resources = mWorld->getResourceManager();
        cgVertexFormat * vertexFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( !newMesh->prepareMesh( vertexFormat, vertices, vertex, triangleData, true, true, true, resources ) )
        {
            delete newMesh;
            return false;
        
        } // End if failed

        // Add to the resource manager
        if ( !resources->addMesh( &mSandboxMesh, newMesh, 0, referenceName, cgDebugSource() ) )
        {
            delete newMesh;
            return false;
        
        } // End if failed

    } // End if no existing mesh
    
    // Success?
    return mSandboxMesh.isValid();
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::applyObjectRescale( cgFloat scale )
{
    // Apply the scale to object-space data
    const cgFloat newWidth  = mWidth  * scale;
    const cgFloat newHeight = mHeight * scale;
    const cgFloat newLength = mLength * scale;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, newWidth );
        mUpdateProperties.bindParameter( 2, newHeight );
        mUpdateProperties.bindParameter( 3, newLength );
        mUpdateProperties.bindParameter( 4, mTaper );
        mUpdateProperties.bindParameter( 5, mReferenceId );
        
        // Execute
        if ( !mUpdateProperties.step( true ) )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties of bone object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local value.
    mWidth  = newWidth;
    mHeight = newHeight;
    mLength = newLength;

    // Sandbox mesh must be re-generated.
    mSandboxMesh.close();

    // Notify listeners that property was altered
    static const cgString modificationContext = _T("ApplyRescale");
    onComponentModified( &cgComponentModifiedEventArgs( modificationContext ) );

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( scale );
}

//-----------------------------------------------------------------------------
//  Name : getWidth()
/// <summary>
/// Retrieve the local width of the bone object (pre-scale) for the purposes
/// of sandbox visualization.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBoneObject::getWidth( ) const
{
    return mWidth;
}

//-----------------------------------------------------------------------------
//  Name : getHeight()
/// <summary>
/// Retrieve the local height of the bone object (pre-scale) for the purposes
/// of sandbox visualization.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBoneObject::getHeight( ) const
{
    return mHeight;
}

//-----------------------------------------------------------------------------
//  Name : getLength()
/// <summary>
/// Retrieve the local length of the bone object (pre-scale) for the purposes
/// of sandbox visualization. This can also be modified to more accurately set
/// the distance between this bone and any attached child bone(s).
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBoneObject::getLength( ) const
{
    return mLength;
}

//-----------------------------------------------------------------------------
//  Name : getTaper()
/// <summary>
/// Retrieve the amount that the sandbox representation of the bone tapers from
/// its origin to its tip (0-1).
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBoneObject::getTaper( ) const
{
    return mTaper;
}

//-----------------------------------------------------------------------------
//  Name : setWidth()
/// <summary>
/// Set the local width of the bone object (pre-scale) for the purposes
/// of sandbox visualization.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::setWidth( cgFloat width )
{
    // Is this a no-op?
    if ( mWidth == width )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, width );
        mUpdateProperties.bindParameter( 2, mHeight );
        mUpdateProperties.bindParameter( 3, mLength );
        mUpdateProperties.bindParameter( 4, mTaper );
        mUpdateProperties.bindParameter( 5, mReferenceId );
        
        // Execute
        if ( !mUpdateProperties.step( true ) )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties of bone object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mWidth = width;

    // Sandbox mesh must be re-generated.
    mSandboxMesh.close();

    // Notify listeners that property was altered
    static const cgString modificationContext = _T("Width");
    onComponentModified( &cgComponentModifiedEventArgs( modificationContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setHeight()
/// <summary>
/// Set the local height of the bone object (pre-scale) for the purposes
/// of sandbox visualization.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::setHeight( cgFloat height )
{
    // Is this a no-op?
    if ( mHeight == height )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, mWidth );
        mUpdateProperties.bindParameter( 2, height );
        mUpdateProperties.bindParameter( 3, mLength );
        mUpdateProperties.bindParameter( 4, mTaper );
        mUpdateProperties.bindParameter( 5, mReferenceId );
        
        // Execute
        if ( !mUpdateProperties.step( true ) )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties of bone object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mHeight = height;

    // Sandbox mesh must be re-generated.
    mSandboxMesh.close();

    // Notify listeners that property was altered
    static const cgString modificationContext = _T("Height");
    onComponentModified( &cgComponentModifiedEventArgs( modificationContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setLength()
/// <summary>
/// Set the local length of the bone object (pre-scale) for the purposes
/// of sandbox visualization. This can also be modified to more accurately set
/// the distance between this bone and any attached child bone(s).
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::setLength( cgFloat length )
{
    // Is this a no-op?
    if ( mLength == length )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, mWidth );
        mUpdateProperties.bindParameter( 2, mHeight );
        mUpdateProperties.bindParameter( 3, length );
        mUpdateProperties.bindParameter( 4, mTaper );
        mUpdateProperties.bindParameter( 5, mReferenceId );
        
        // Execute
        if ( !mUpdateProperties.step( true ) )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties of bone object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    cgFloat oldLength = mLength;
    mLength = length;

    // Sandbox mesh must be re-generated.
    mSandboxMesh.close();

    // Notify listeners that property was altered
    static const cgString modificationContext = _T("Length");
    onComponentModified( &cgComponentModifiedEventArgs( modificationContext, oldLength ) );
}

//-----------------------------------------------------------------------------
//  Name : setTaper()
/// <summary>
/// Set the amount that the sandbox representation of the bone tapers from
/// its origin to its tip (0-1).
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::setTaper( cgFloat taper )
{
    // clamp taper from 0 - 1.
    taper = cgMathUtility::clamp( taper, -99.9f, 1.0f );

    // Is this a no-op?
    if ( mTaper == taper )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, mWidth );
        mUpdateProperties.bindParameter( 2, mHeight );
        mUpdateProperties.bindParameter( 3, mLength );
        mUpdateProperties.bindParameter( 4, taper );
        mUpdateProperties.bindParameter( 5, mReferenceId );
        
        // Execute
        if ( !mUpdateProperties.step( true ) )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties of bone object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mTaper = taper;

    // Sandbox mesh must be re-generated.
    mSandboxMesh.close();

    // Notify listeners that property was altered
    static const cgString modificationContext = _T("Taper");
    onComponentModified( &cgComponentModifiedEventArgs( modificationContext ) );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BoneObject )
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
cgString cgBoneObject::getDatabaseTable( ) const
{
    return _T("Objects::Bone");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgBoneObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("BoneObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertBone.bindParameter( 1, mReferenceId );
        mInsertBone.bindParameter( 2, mWidth );
        mInsertBone.bindParameter( 3, mHeight );
        mInsertBone.bindParameter( 4, mLength );
        mInsertBone.bindParameter( 5, mTaper );
        mInsertBone.bindParameter( 6, mSoftRefCount );

        // Execute
        if ( !mInsertBone.step( true ) )
        {
            cgString error;
            mInsertBone.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for bone object '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("BoneObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("BoneObject::insertComponentData") );

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
bool cgBoneObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the bone data.
    prepareQueries();
    mLoadBone.bindParameter( 1, e->sourceRefId );
    if ( !mLoadBone.step( ) || !mLoadBone.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadBone.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for bone object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for bone object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadBone.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadBone;

    // Update our local members
    mLoadBone.getColumn( _T("Width"), mWidth );
    mLoadBone.getColumn( _T("Height"), mHeight );
    mLoadBone.getColumn( _T("Length"), mLength );
    mLoadBone.getColumn( _T("Taper"), mTaper );

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
void cgBoneObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertBone.isPrepared() == false )
            mInsertBone.prepare( mWorld, _T("INSERT INTO 'Objects::Bone' VALUES(?1,?2,?3,?4,?5,?6)"), true );
        if ( mUpdateProperties.isPrepared() == false )
            mUpdateProperties.prepare( mWorld, _T("UPDATE 'Objects::Bone' SET Width=?1,Height=?2,Length=?3,Taper=?4 WHERE RefId=?5"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadBone.isPrepared() == false )
        mLoadBone.prepare( mWorld, _T("SELECT * FROM 'Objects::Bone' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgBoneNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBoneNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneNode::cgBoneNode( cgUInt32 referenceId, cgScene * scene ) : cgObjectNode( referenceId, scene )
{
    // Default the node color
    mColor                = 0xFFAEBACB;
    mDisableChildUpdates  = false;
}

//-----------------------------------------------------------------------------
//  Name : cgBoneNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneNode::cgBoneNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgObjectNode( referenceId, scene, init, initMethod, initTransform )
{
    mDisableChildUpdates  = false;

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Bone%X"), referenceId );
}

//-----------------------------------------------------------------------------
//  Name : ~cgBoneNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneNode::~cgBoneNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgBoneNode::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgBoneNode( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgBoneNode::allocateClone( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform )
{
    return new cgBoneNode( referenceId, scene, init, initMethod, initTransform );
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // What was modified?
    if ( e->context == _T("Length") )
    {
        // Note: Child updates should not be applied in response to an 'ApplyRescale'

        // Update child bones?
        if ( !mDisableChildUpdates )
        {
            // In order to modify the position of the child bones we will need to know
            // the old length (in world space). cgBoneObject::setLength() passes old length 
            // to this method via the variable argument within the event args structure. 
            cgVector3 scale = getScale();
            cgFloat oldLength = (cgFloat)e->argument * scale.x;

            // Adjust positions of the child bones.
            cgObjectNodeList::iterator itNode;
            for ( itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
            {
                cgObjectNode * node = *itNode;
                if ( node->queryObjectType( RTID_BoneObject ) )
                    node->setPosition( node->getPosition() + getDirection() * (getLength() - oldLength) );
                
            } // Next Child

        } // End if update children

    } // End if Length

    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : setBoneOrientation() (Virtual)
/// <summary>
/// Set the orientation of the bone based on the specified source and
/// destination world space positions.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::setBoneOrientation( const cgVector3 & source, const cgVector3 & destination, const cgVector3 & up )
{
    // Compute new "look at" (Z direction) between the source and destination points.
    cgVector3 newDirection = destination - source;

    // Normalize (if possible)
    cgFloat length = cgVector3::length( newDirection );
    if ( length < CGE_EPSILON )
        return;
    newDirection /= length;

    // Generate the new node transform
    cgTransform t;
    cgVector3 currentPosition = getPosition();
    t.lookAt( currentPosition, currentPosition + newDirection, up );

    // Rotate the transform to ensure our bone is aligned correctly
    // (bone is aligned to the X axis, not the Z as with the lookAt method).
    t.rotateLocal( 0, CGEToRadian(-90.0f), 0 );
    
    // Maintain the original X/Y/Z scale of the bone.
    t.setLocalScale( getScale() );
    
    // Adjust bone transform.
    this->setWorldTransform( t );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BoneNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : setWidth()
/// <summary>
/// Set the world space width of the bone object for the purposes of sandbox 
/// visualization.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::setWidth( cgFloat width )
{
    // Remove scale.
    cgVector3 scale = getScale();
    width /= scale.z;

    // Set to the object.
    ((cgBoneObject*)mReferencedObject)->setWidth( width );
}

//-----------------------------------------------------------------------------
//  Name : setHeight()
/// <summary>
/// Set the world space height of the bone object for the purposes of sandbox 
/// visualization.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::setHeight( cgFloat height )
{
    // Remove scale.
    cgVector3 scale = getScale();
    height /= scale.y;

    // Set to the object.
    ((cgBoneObject*)mReferencedObject)->setHeight( height );
}

//-----------------------------------------------------------------------------
//  Name : setLength()
/// <summary>
/// Set the world space length of the bone object for the purposes of sandbox 
/// visualization. This can also be modified to more accurately set the 
/// distance between this bone and any attached child bone(s).
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::setLength( cgFloat length )
{
    // Remove scale.
    cgVector3 scale = getScale();
    length /= scale.x;

    // Set to the object.
    ((cgBoneObject*)mReferencedObject)->setLength( length );
}

//-----------------------------------------------------------------------------
//  Name : getWidth()
/// <summary>
/// Retrieve the world space width of the bone object for the purposes of 
/// sandbox visualization.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBoneNode::getWidth( )
{
    // Get object local size.
    cgFloat width = ((cgBoneObject*)mReferencedObject)->getWidth( );

    // Apply scale
    cgVector3 scale = getScale();
    return width * scale.z;
}

//-----------------------------------------------------------------------------
//  Name : getHeight()
/// <summary>
/// Retrieve the world space height of the bone object for the purposes of 
/// sandbox visualization.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBoneNode::getHeight( )
{
    // Get object local size.
    cgFloat height = ((cgBoneObject*)mReferencedObject)->getHeight( );

    // Apply scale
    cgVector3 scale = getScale();
    return height * scale.y;
}

//-----------------------------------------------------------------------------
//  Name : getLength()
/// <summary>
/// Retrieve the world space length of the bone object for the purposes of 
/// sandbox visualization. This can also be modified to more accurately set the
/// distance between this bone and any attached child bone(s).
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBoneNode::getLength( )
{
    // Get object local size.
    cgFloat length = ((cgBoneObject*)mReferencedObject)->getLength( );

    // Apply scale
    cgVector3 scale = getScale();
    return length * scale.x;
}

//-----------------------------------------------------------------------------
//  Name : getDirection ()
/// <summary>
/// Get the primary direction of the bone (defaults to X axis).
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgBoneNode::getDirection( )
{
    return getXAxis();
}

//-----------------------------------------------------------------------------
// Name : move() (Virtual)
/// <summary>Move the object by the specified amount in "parent" space.</summary>
//-----------------------------------------------------------------------------
void cgBoneNode::move( const cgVector3 & amount )
{
    // If this object has a parent bone, the parent should be
    // adjusted instead of moving the child. Otherwise, move as normal.
    if ( mParentNode && mParentNode->queryObjectType( RTID_BoneObject ) )
    {
        cgBoneNode * parentBone = (cgBoneNode*)mParentNode;
        
        // Compute new "look at" direction for parent to this bone's newly proposed position.
        cgVector3 tipPosition = parentBone->getPosition();
        tipPosition += parentBone->getDirection() * parentBone->getLength();
        tipPosition += amount;
        parentBone->setBoneOrientation( parentBone->getPosition(), tipPosition, parentBone->getYAxis() );
        return;
    
    } // End if parent bone

    // Call base class implementation if parent is not a bone.
    cgObjectNode::move( amount );
}

//-----------------------------------------------------------------------------
// Name : moveLocal() (Virtual)
/// <summary>Move the object by the specified amount in "local" space.</summary>
//-----------------------------------------------------------------------------
void cgBoneNode::moveLocal( const cgVector3 & amount )
{
    // If this object has a parent bone, the parent should be
    // adjusted instead of moving the child. Otherwise, move as normal.
    if ( mParentNode && mParentNode->queryObjectType( RTID_BoneObject ) )
    {
        cgBoneNode * parentBone = (cgBoneNode*)mParentNode;

        // Compute new position for the end of the parent bone
        cgVector3 tipPosition = parentBone->getPosition() + (parentBone->getDirection() * parentBone->getLength());
        tipPosition += getXAxis() * amount.x;
        tipPosition += getYAxis() * amount.y;
        tipPosition += getZAxis() * amount.z;
        
        // Adjust parent.
        parentBone->setBoneOrientation( parentBone->getPosition(), tipPosition, parentBone->getYAxis() );
        return;
    
    } // End if parent bone
    
    // Call base class implementation if parent is not a bone.
    cgObjectNode::moveLocal( amount );
}

//-----------------------------------------------------------------------------
// Name : recomputeLength()
/// <summary>
/// Compute the average position of all child bones and then recompute
/// our length (without adjusting the child positions).
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::recomputeLength( )
{
    cgVector3 averageChildPosition( 0, 0, 0 );
    cgInt     childBoneCount = 0;

    // Compute a plane that describes the origin of the bone
    cgPlane plane;
    cgPlane::fromPointNormal( plane, getPosition(), getDirection() );

    // Compute average position of any child bones.
    cgObjectNodeList::const_iterator itChild;
    for ( itChild = mChildren.begin(); itChild != mChildren.end(); ++itChild )
    {
        cgObjectNode * childNode = (*itChild);
        if ( childNode->queryObjectType( RTID_BoneObject ) )
        {
            // Skip if this child bone is actually behind the bone's plane
            const cgVector3 & position = childNode->getPosition();
            if ( cgPlane::dotCoord( plane, position ) < -CGE_EPSILON_1MM )
                continue;

            // Record bone position
            averageChildPosition   += position;
            childBoneCount += 1;

        } // End if bone

    } // Next Child

    // Prevent children from being repositioned when length is adjusted.
    mDisableChildUpdates = true;

    // Compute new length
    if ( childBoneCount )
    {
        // Compute distance to a plane parallel with the child bone(s).
        // (It's theoretically possible for a child bone's origin not to
        //  be aligned exactly with the end tip of the parent bone).
        cgPlane::fromPointNormal( plane, (averageChildPosition / (cgFloat)childBoneCount), getDirection() );
        setLength( fabsf( cgPlane::dotCoord( plane, getPosition() ) ) );
    
    } // End if has children

    // Allow child updates to take place once more.
    mDisableChildUpdates = false;
}