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
// Name : cgConvexHullShape.cpp                                              //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for an          //
//        automatically generated convex hull shape constructed from the     //
//        supplied vertex data.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgConvexHullShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgConvexHullShape.h>
#include <Physics/cgPhysicsWorld.h>
#include <Math/cgTransform.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgMesh.h>
#include <Resources/cgResourceManager.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgConvexHullShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgConvexHullShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexHullShape::cgConvexHullShape( cgPhysicsWorld * world, void * serializedBuffer, cgUInt32 dataSize ) : cgConvexShape( world )
{
    // Open the serialized data as an input stream.
    cgInputStream stream( serializedBuffer, dataSize );
    stream.open();

    // Deserialize the shape
    mShape = NewtonCreateCollisionFromSerialization( world->getInternalWorld(), deserialize, &stream );

    // We're done with the stream
    stream.close();
}

//-----------------------------------------------------------------------------
//  Name : cgConvexHullShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexHullShape::cgConvexHullShape( cgPhysicsWorld * world, const cgByteArray & serializedBuffer ) : cgConvexShape( world )
{
    // Open the serialized data as an input stream.
    std::istringstream stream;
    stream.rdbuf()->pubsetbuf( (cgChar*)&serializedBuffer[0], serializedBuffer.size() );

    // Deserialize the shape
    mShape = NewtonCreateCollisionFromSerialization( world->getInternalWorld(), deserialize, &stream );
}

//-----------------------------------------------------------------------------
//  Name : cgConvexHullShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexHullShape::cgConvexHullShape( cgPhysicsWorld * world, void * vertexData, cgUInt32 vertexCount, cgUInt32 stride ) : cgConvexShape( world )
{
    // Transform to new scale if warranted.
    cgFloat factor = mWorld->toPhysicsScale();
    if ( factor != 1.0f )
    {
        // Build a new point cloud in the scale of our physics system
        std::vector<cgVector3> points( vertexCount );
        cgByte    * input  = (cgByte*)vertexData;
        cgVector3 * output = &points[0];
        for ( size_t i = 0; i < vertexCount; ++i, input += stride, ++output )
            *output = *((cgVector3*)input) * factor;

        // Construct the wrapped Newton shape.
        mShape = NewtonCreateConvexHull( world->getInternalWorld(), vertexCount, (const cgFloat*)&points.front(), sizeof(cgVector3), 0.0f, getReferenceId(), CG_NULL );
        
    } // End if convert
    else
    {
        // Construct the wrapped Newton shape.
        mShape = NewtonCreateConvexHull( world->getInternalWorld(), vertexCount, (const cgFloat*)vertexData, stride, 0.0f, getReferenceId(), CG_NULL );

    } // End if !convert
}

//-----------------------------------------------------------------------------
//  Name : cgConvexHullShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexHullShape::cgConvexHullShape( cgPhysicsWorld * world, void * vertexData, cgUInt32 vertexCount, cgUInt32 stride, const cgTransform & offset ) : cgConvexShape( world )
{
    // Transform to new scale if warranted.
    cgFloat factor = mWorld->toPhysicsScale();
    if ( factor != 1.0f )
    {
        // Build a new point cloud in the scale of our physics system.
        std::vector<cgVector3> points( vertexCount );
        cgByte    * input  = (cgByte*)vertexData;
        cgVector3 * output = &points[0];
        for ( size_t i = 0; i < vertexCount; ++i, input += stride, ++output )
            *output = *((cgVector3*)input) * factor;

        // Construct the wrapped Newton shape.
        mShape = NewtonCreateConvexHull( world->getInternalWorld(), vertexCount, (const cgFloat*)&points.front(), sizeof(cgVector3), 0.0f, getReferenceId(), (cgMatrix)offset );
        
    } // End if convert
    else
    {
        // Construct the wrapped Newton shape.
        mShape = NewtonCreateConvexHull( world->getInternalWorld(), vertexCount, (const cgFloat*)vertexData, stride, 0.0f, getReferenceId(), (cgMatrix)offset );

    } // End if !convert
}

//-----------------------------------------------------------------------------
//  Name : cgConvexHullShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexHullShape::cgConvexHullShape( cgPhysicsWorld * world, void * vertexData, cgUInt32 vertexCount, cgUInt32 stride, cgFloat collapseTolerance ) : cgConvexShape( world )
{
    // Transform to new scale if warranted.
    cgFloat factor = mWorld->toPhysicsScale();
    if ( factor != 1.0f )
    {
        // Build a new point cloud in the scale of our physics system.
        std::vector<cgVector3> points( vertexCount );
        cgByte    * input  = (cgByte*)vertexData;
        cgVector3 * output = &points[0];
        for ( size_t i = 0; i < vertexCount; ++i, input += stride, ++output )
            *output = *((cgVector3*)input) * factor;

        // Construct the wrapped Newton shape.
        mShape = NewtonCreateConvexHull( world->getInternalWorld(), vertexCount, (const cgFloat*)&points.front(), sizeof(cgVector3), collapseTolerance, getReferenceId(), CG_NULL );
        
    } // End if convert
    else
    {
        // Construct the wrapped Newton shape.
        mShape = NewtonCreateConvexHull( world->getInternalWorld(), vertexCount, (const cgFloat*)vertexData, stride, collapseTolerance, getReferenceId(), CG_NULL );

    } // End if !convert
}

//-----------------------------------------------------------------------------
//  Name : cgConvexHullShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexHullShape::cgConvexHullShape( cgPhysicsWorld * world, void * vertexData, cgUInt32 vertexCount, cgUInt32 stride, cgFloat collapseTolerance, const cgTransform & offset ) : cgConvexShape( world )
{
    // Transform to new scale if warranted.
    cgFloat factor = mWorld->toPhysicsScale();
    if ( factor != 1.0f )
    {
        // Build a new point cloud in the scale of our physics system.
        std::vector<cgVector3> points( vertexCount );
        cgByte    * input  = (cgByte*)vertexData;
        cgVector3 * output = &points[0];
        for ( size_t i = 0; i < vertexCount; ++i, input += stride, ++output )
            *output = *((cgVector3*)input) * factor;

        // Construct the wrapped Newton shape.
        mShape = NewtonCreateConvexHull( world->getInternalWorld(), vertexCount, (const cgFloat*)&points.front(), sizeof(cgVector3), collapseTolerance, getReferenceId(), (cgMatrix)offset );
        
    } // End if convert
    else
    {
        // Construct the wrapped Newton shape.
        mShape = NewtonCreateConvexHull( world->getInternalWorld(), vertexCount, (const cgFloat*)vertexData, stride, collapseTolerance, getReferenceId(), (cgMatrix)offset );

    } // End if !convert
}

//-----------------------------------------------------------------------------
//  Name : ~cgConvexHullShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexHullShape::~cgConvexHullShape()
{

}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConvexHullShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ConvexHullShape )
        return true;

    // Supported by base?
    return cgConvexShape::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : buildRenderMesh()
/// <summary>
/// Build a renderable version of the convex hull mesh for debugging and
/// display purposes.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshHandle cgConvexHullShape::buildRenderMesh( cgResourceManager * resourceManager )
{
    // Validate requirements
    if ( !mShape )
        return cgMeshHandle::Null;

    // Retrieve the shape collision info.
    NewtonCollisionInfoRecord info;
    NewtonCollisionGetInfo( mShape, &info );
    if ( info.m_convexHull.m_faceCount == 0 )
        return cgMeshHandle::Null;
    
    // Create a new mesh ready for population.
    cgMesh * newMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );
    newMesh->prepareMesh( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), false, resourceManager );

    /*// Build a custom vertex format compatible with the 4 component
    // vectors used by Newton.
    cgVertexFormat sourceFormat;
    sourceFormat.beginPrepare( );
    sourceFormat.addVertexElement( D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION );
    sourceFormat.endPrepare( );*/
    
    // Convert mesh data back into game scale (from physics scale)
    cgFloat factor = mWorld->fromPhysicsScale();
    cgVector3 * outputVertices = new cgVector3[ info.m_convexHull.m_vertexCount ];
    cgByte    * inputVertices  = (cgByte*)info.m_convexHull.m_vertex;
    for ( cgInt i = 0; i < info.m_convexHull.m_vertexCount; ++i, inputVertices += info.m_convexHull.m_vertexStrideInBytes )
        outputVertices[i] = *((cgVector3*)inputVertices) * factor;
    
    // Set as the source for the mesh.
    newMesh->setVertexSource( outputVertices, info.m_convexHull.m_vertexCount, newMesh->getVertexFormat() );
    
    // Start building the mesh.
    cgUInt32 * indices = new cgUInt32[ info.m_convexHull.m_vertexCount ]; // Enough room for all vertices (just in case)
    for ( cgInt i = 0; i < info.m_convexHull.m_faceCount; ++i )
    {
        // Note: Internally newton uses Int32, but the interface expects an int. It is
        // (currently) safe to cast to 'cgInt*' but we should take care with this.
        cgInt faceVertexCount = NewtonConvexHullGetFaceIndices( mShape, i, (cgInt*)indices );

        // Add to the mesh.
        newMesh->addPrimitives( cgPrimitiveType::TriangleFan, indices, 0, faceVertexCount, cgMaterialHandle::Null );
    
    } // Next face
    delete []indices;
    delete []outputVertices;

    // Finish preparing the mesh.
    if ( !newMesh->endPrepare( ) )
    {
        delete newMesh;
        return cgMeshHandle::Null;
    
    } // End if failed

    // We're done, add to the resource manager.
    cgMeshHandle meshHandle;
    cgString referenceName = cgString::format( _T("Core::CollisionShapes::ConvexHull(0x%x)"), getReferenceId() );
    resourceManager->addMesh( &meshHandle, newMesh, cgResourceFlags::ForceNew, referenceName, cgDebugSource() );

    // Return the new mesh.
    return meshHandle;
}

//-----------------------------------------------------------------------------
//  Name : serializeShape () (Virtual)
/// <summary>
/// Populate a buffer with data that can later be used to deserialize the shape
/// rather than re-generate it at load time.
/// </summary>
//-----------------------------------------------------------------------------
void cgConvexHullShape::serializeShape( cgByteArray & serializedBuffer )
{
    // Clear out the buffer initially.
    serializedBuffer.clear();
    
    // Perform the serialization
    if ( mShape )
        NewtonCollisionSerialize( mWorld->getInternalWorld(), mShape, serialize, &serializedBuffer );
}

//-----------------------------------------------------------------------------
//  Name : deserialize () (Protected, Static)
/// <summary>
/// Callback function that allows the convex hull data to be deserialized from
/// a specified binary input stream. This avoids the need to re-generate the
/// convex hull from the source data set at load time.
/// </summary>
//-----------------------------------------------------------------------------
void cgConvexHullShape::deserialize( void * serializeHandle, void * buffer, cgInt size )
{
    // Read the next requested section of the stream.
    ((cgInputStream*)serializeHandle)->read( buffer, size );
}

//-----------------------------------------------------------------------------
//  Name : serialize () (Protected, Static)
/// <summary>
/// Callback function that allows the convex hull data to be serialized to
/// a specified binary output stream. This allows the application to avoid 
/// re-generating the convex hull from the source data set at load time.
/// </summary>
//-----------------------------------------------------------------------------
void cgConvexHullShape::serialize( void * serializeHandle, const void * buffer, cgInt size )
{
    cgByteArray & destination = *((cgByteArray*)serializeHandle);

    // Resize the buffer ready to hold the new data.
    destination.resize( destination.size() + size );

    // Copy the new data in
    memcpy( &destination[ destination.size() - size ], buffer, size );
}

//-----------------------------------------------------------------------------
//  Name : compare () (Virtual)
/// <summary>
/// Compare the physics shapes to see if they describe the same data.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::Dispose()" />
//-----------------------------------------------------------------------------
cgInt cgConvexHullShape::compare( cgPhysicsShape * shape ) const
{
    // Compare base properties first.
    cgInt result = cgPhysicsShape::compare( shape );
    if ( result != 0 )
        return result;

    // Now compare custom properties.
    cgConvexHullShape * convexHullShape = (cgConvexHullShape*)shape;
    cgToDo( "Physics", "Support shape cache when necessary." )
    
    // Equivalent
    return 0;
}