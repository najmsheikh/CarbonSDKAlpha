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
// Name : cgSoundEmitterObject.cpp                                           //
//                                                                           //
// Desc : Contains classes responsible for representing 3D positional sound  //
//        emitters within a scene.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSoundEmitterObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgSoundEmitterObject.h>
#include <World/cgScene.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgAudioBuffer.h>
#include <World/Objects/cgCameraObject.h>
#include <Math/cgCollision.h>
#include <Math/cgMathUtility.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgSoundEmitterObject::mInsertSoundEmitter;
cgWorldQuery cgSoundEmitterObject::mUpdateProperties;
cgWorldQuery cgSoundEmitterObject::mUpdateRanges;
cgWorldQuery cgSoundEmitterObject::mLoadSoundEmitter;

///////////////////////////////////////////////////////////////////////////////
// cgSoundEmitterObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSoundEmitterObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSoundEmitterObject::cgSoundEmitterObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize members to sensible defaults
    mDefaultVolume      = 1;
    mStreamingBuffer    = false;
    mAutoPlay           = true;
    mLooping            = true;
    mMuteOutsideRange   = false;
    mInnerRange         = 0.0f;
    mOuterRange         = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgSoundEmitterObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSoundEmitterObject::cgSoundEmitterObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgWorldObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    cgSoundEmitterObject * object = (cgSoundEmitterObject*)init;
    mSourceFileName     = object->mSourceFileName;
    mStreamingBuffer    = object->mStreamingBuffer;
    mDefaultVolume      = object->mDefaultVolume;
    mAutoPlay           = object->mAutoPlay;
    mLooping            = object->mLooping;
    mMuteOutsideRange   = object->mMuteOutsideRange;
    mInnerRange         = object->mInnerRange;
    mOuterRange         = object->mOuterRange;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSoundEmitterObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSoundEmitterObject::~cgSoundEmitterObject()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgSoundEmitterObject::allocateNew( const cgUID & type, cgUInt32 referenceId, cgWorld * world )
{
    return new cgSoundEmitterObject( referenceId, world );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgSoundEmitterObject::allocateClone( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod )
{
    // Valid clone?
    return new cgSoundEmitterObject( referenceId, world, init, initMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgSoundEmitterObject::getLocalBoundingBox( )
{
    // Compute emitter bounding box
    cgBoundingBox Bounds;
    Bounds.min = cgVector3( -mOuterRange, -mOuterRange, -mOuterRange );
    Bounds.max = cgVector3(  mOuterRange,  mOuterRange,  mOuterRange );
    return Bounds;
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterObject::pick( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Retrieve useful values
    cgFloat   zoomFactor  = camera->estimateZoomFactor( viewportSize, issuer->getPosition( false ), 2.5f );
    cgVector3 right       = cgVector3(1,0,0);
    cgVector3 up          = cgVector3(0,1,0);
    cgVector3 look        = cgVector3(0,0,1);

    // Compute vertices for the light source representation (central diamond)
    cgVector3 points[6];
    points[0] = up * (10.0f * zoomFactor);
    points[1] = look * (10.0f * zoomFactor);
    points[2] = right * (10.0f * zoomFactor);
    points[3] = -look * (10.0f * zoomFactor);
    points[4] = -right * (10.0f * zoomFactor);
    points[5] = -up * (10.0f * zoomFactor);

    // Construct indices for the 8 triangles
    cgUInt32 indices[24];
    indices[0]  = 0; indices[1]  = 1; indices[2]  = 2;
    indices[3]  = 0; indices[4]  = 2; indices[5]  = 3;
    indices[6]  = 0; indices[7]  = 3; indices[8]  = 4;
    indices[9]  = 0; indices[10] = 4; indices[11] = 1;
    indices[12] = 5; indices[13] = 2; indices[14] = 1;
    indices[15] = 5; indices[16] = 3; indices[17] = 2;
    indices[18] = 5; indices[19] = 4; indices[20] = 3;
    indices[21] = 5; indices[22] = 1; indices[23] = 4;

    // Determine if the ray intersects any of the triangles formed by the above points
    for ( cgUInt32 i = 0; i < 8; ++i )
    {
        // Compute plane for the current triangle
        cgPlane plane;
        const cgVector3 & v1 = points[indices[(i*3)+0]];
        const cgVector3 & v2 = points[indices[(i*3)+1]];
        const cgVector3 & v3 = points[indices[(i*3)+2]];
        cgPlane::fromPoints( plane, v1, v2, v3 );

        // Determine if (and where) the ray intersects the triangle's plane
        cgFloat t;
        if ( !cgCollision::rayIntersectPlane( rayOrigin, rayDirection, plane, t, false, false ) )
            continue;

        // Compute the intersection point on the plane
        cgVector3 intersection = rayOrigin + (rayDirection * t);

        // Determine if this point is within the triangles edges (within tolerance)
        if ( !cgCollision::pointInTriangle( intersection, v1, v2, v3, (cgVector3&)plane, 2.0f * zoomFactor ) )
            continue;

        // We intersected! Return the intersection distance.
        distance = t;
        return true;

    } // Next Triangle

    // No intersection with us.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::sandboxRender( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // Get access to required systems.
    cgRenderDriver  * driver = cgRenderDriver::getInstance();
    cgSurfaceShader * shader = driver->getSandboxSurfaceShader().getResource(true);

    // Retrieve useful values
    const cgViewport & viewport = driver->getViewport();
    cgFloat  zoomFactor = camera->estimateZoomFactor( viewport.size, issuer->getPosition( false ), 2.5f );
    cgFloat  size       = zoomFactor * 10.0f;
    cgUInt32 color      = issuer->isSelected() ? 0xFFFFFFFF : 0xFFFFF600;

    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    cgShadedVertex points[30];
    for ( size_t i = 0; i < 6; ++i )
        points[i].color = color;

    // Compute vertices for the light source representation (central diamond)
    points[0].position = cgVector3( 0, size, 0 );
    points[1].position = cgVector3( 0, 0, size );
    points[2].position = cgVector3( size, 0, 0 );
    points[3].position = cgVector3( 0, 0, -size );
    points[4].position = cgVector3( -size, 0, 0 );
    points[5].position = cgVector3( 0, -size, 0 );

    // Compute indices that will allow us to draw the diamond using a tri-list (wireframe)
    // so that we can easily take advantage of back-face culling.
    cgUInt32 indices[31];
    indices[0]  = 0; indices[1]  = 1; indices[2]  = 2;
    indices[3]  = 0; indices[4]  = 2; indices[5]  = 3;
    indices[6]  = 0; indices[7]  = 3; indices[8]  = 4;
    indices[9]  = 0; indices[10] = 4; indices[11] = 1;
    indices[12] = 5; indices[13] = 2; indices[14] = 1;
    indices[15] = 5; indices[16] = 3; indices[17] = 2;
    indices[18] = 5; indices[19] = 4; indices[20] = 3;
    indices[21] = 5; indices[22] = 1; indices[23] = 4;

    // Begin rendering
    bool wireframe = (flags & cgSandboxRenderFlags::Wireframe);
    driver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    shader->setBool( _T("wireViewport"), wireframe );
    driver->setWorldTransform( issuer->getWorldTransform( false ) );
    if ( shader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( shader->executeTechniquePass() != cgTechniqueResult::Abort )
        {
            // Draw the central diamond
            driver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, 6, 8, indices, cgBufferFormat::Index32, points );

            // If the emitter is REALLY selected (i.e. not just one of its parent groups), construct and 
            // render circles that depict the outer range of the emitter.
            if ( issuer->isSelected() && !issuer->isMergedAsGroup() )
            {
                driver->setWorldTransform( CG_NULL );

                // A circle for each axis
                cgVector3 point, axis, orthoAxis1, orthoAxis2, position = issuer->getPosition( false );
                for ( size_t i = 0; i < 3; ++i )
                {
                    // Retrieve the three axis vectors necessary for constructing the
                    // circle representation for this axis.
                    switch ( i )
                    {
                        case 0:
                            axis       = cgVector3( 1, 0, 0 );
                            orthoAxis1 = cgVector3( 0, 1, 0 );
                            orthoAxis2 = cgVector3( 0, 0, 1 );
                            break;
                        case 1:
                            axis       = cgVector3( 0, 1, 0 );
                            orthoAxis1 = cgVector3( 0, 0, 1 );
                            orthoAxis2 = cgVector3( 1, 0, 0 );
                            break;
                        case 2:
                            axis       = cgVector3( 0, 0, 1 );
                            orthoAxis1 = cgVector3( 1, 0, 0 );
                            orthoAxis2 = cgVector3( 0, 1, 0 );
                            break;

                    } // End Axis Switch

                    // Skip if this axis is more or less orthogonal to the camera (ortho view only)
                    if ( camera->getProjectionMode() == cgProjectionMode::Orthographic &&
                        fabsf( cgVector3::dot( axis, camera->getZAxis( false ) ) ) < 0.05f )
                        continue;

                    // Generate line strip circle
                    for ( size_t j = 0; j < 30; ++j )
                    {
                        // Build vertex
                        point  = position;
                        point += orthoAxis2 * (sinf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mOuterRange);
                        point += orthoAxis1 * (cosf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mOuterRange);
                        points[j] = cgShadedVertex( point, 0xFF99CCE5 );

                        // Build index
                        indices[j] = j;

                    } // Next Segment

                    // Add last index to connect back to the start of the circle
                    indices[30] = 0;

                    // Render the lines.
                    driver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 30, 30, indices, cgBufferFormat::Index32, points );

                    // Same for inner range (if >0 & <outer)
                    if ( mInnerRange > 0 && mInnerRange < mOuterRange )
                    {
                        // Generate line strip circle
                        for ( cgInt j = 0; j < 30; ++j )
                        {
                            // Build vertex
                            point  = position;
                            point += orthoAxis2 * (sinf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mInnerRange);
                            point += orthoAxis1 * (cosf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mInnerRange);
                            points[j] = cgShadedVertex( point, 0xFF4C6672 );

                        } // Next Segment

                        // Render the lines.
                        driver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 30, 30, indices, cgBufferFormat::Index32, points );

                    } // End if draw inner range

                } // Next Axis

            } // End if emitter selected

        } // End if begun pass

        // We have finished rendering
        shader->endTechnique();

    } // End if begun technique

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::applyObjectRescale( cgFloat scale )
{
    // Apply the scale to object-space data
    cgFloat newInnerRange = mInnerRange * scale;
    cgFloat newOuterRange = mOuterRange * scale;
    
    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRanges.bindParameter( 1, newOuterRange );
        mUpdateRanges.bindParameter( 2, newInnerRange );
        mUpdateRanges.bindParameter( 3, mReferenceId );

        // Execute
        if ( !mUpdateRanges.step( true ) )
        {
            cgString error;
            mUpdateRanges.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update ranges for sound emitter object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update local values.
    mInnerRange = newInnerRange;
    mOuterRange = newOuterRange;
    
    // Notify listeners that property was altered
    static const cgString context = _T("ApplyRescale");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( scale );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SoundEmitterObject )
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
cgString cgSoundEmitterObject::getDatabaseTable( ) const
{
    return _T("Objects::SoundEmitter");
}

//-----------------------------------------------------------------------------
//  Name : getSourceFile()
/// <summary>
/// Get the name of the source audio file from which this emitter should 
/// populate its buffer for playback.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgSoundEmitterObject::getSourceFile( ) const
{
    return mSourceFileName;
}

//-----------------------------------------------------------------------------
//  Name : getDefaultVolume()
/// <summary>
/// Get the volume at which the sound emitter should play back audio 
/// (0-1 range). This volume is shared by all sound emitter instances unless
/// a given instances opts to supply a custom volume to override this.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSoundEmitterObject::getDefaultVolume( ) const
{
    return mDefaultVolume;
}

//-----------------------------------------------------------------------------
//  Name : isStreaming()
/// <summary>
/// Returns a value indicating whether or not the audio file should be streamed
/// into memory during playback, or loaded into memory in its entirety ahead
/// of time. Streaming is most useful for playback of large files, or in cases
/// where you want little to no loading latency.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterObject::isStreaming( ) const
{
    return mStreamingBuffer;
}

//-----------------------------------------------------------------------------
//  Name : isLooping()
/// <summary>
/// Returns a value indicating whether or not the audio buffer should be
/// played in a repeating / looping fashion.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterObject::isLooping( ) const
{
    return mLooping;
}

//-----------------------------------------------------------------------------
//  Name : shouldAutoPlay()
/// <summary>
/// Returns a value indicating whether or not the emitter should automatically
/// begin playback of the buffer as soon as it is loaded. If this is set to
/// false, playback must be started manually through the cgSoundEmitterNode's
/// 'play()' method.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterObject::shouldAutoPlay( ) const
{
    return mAutoPlay;
}

//-----------------------------------------------------------------------------
//  Name : shouldMuteOutsideRange()
/// <summary>
/// Return a value indicating whether or not the audio buffer should be muted
/// once the listener moves outside of the range specified by the outer range
/// property. This can reduce processing overhead.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterObject::shouldMuteOutsideRange( ) const
{
    return mMuteOutsideRange;
}

//-----------------------------------------------------------------------------
//  Name : getOuterRange()
/// <summary>
/// Get the outer range of the sound emitter that represents the distance
/// outside of which the audio can no longer be heard.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSoundEmitterObject::getOuterRange( ) const
{
    return mOuterRange;
}

//-----------------------------------------------------------------------------
//  Name : getInnerRange()
/// <summary>
/// Get the inner range of the sound emitter that represents the distance
/// inside of which the audio can be heard with full effect.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSoundEmitterObject::getInnerRange( ) const
{
    return mInnerRange;
}

//-----------------------------------------------------------------------------
//  Name : setSourceFile()
/// <summary>
/// Set the name of the source audio file from which this emitter should 
/// populate its buffer for playback.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::setSourceFile( const cgString & fileName )
{
    // Is this a no-op?
    if ( mSourceFileName == fileName )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, fileName );
        mUpdateProperties.bindParameter( 2, mStreamingBuffer );
        mUpdateProperties.bindParameter( 3, mDefaultVolume );
        mUpdateProperties.bindParameter( 4, mAutoPlay );
        mUpdateProperties.bindParameter( 5, mLooping );
        mUpdateProperties.bindParameter( 6, mMuteOutsideRange );
        mUpdateProperties.bindParameter( 7, mReferenceId );
        
        // Execute
        if ( mUpdateProperties.step( true ) == false )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update source file property of sound emitter object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mSourceFileName = fileName;

    // Notify listeners that property was altered
    static const cgString context = _T("SourceFile");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );
}

//-----------------------------------------------------------------------------
//  Name : setDefaultVolume()
/// <summary>
/// Set the volume at which the sound emitter should play back audio 
/// (0-1 range). This volume is shared by all sound emitter instances unless
/// a given instances opts to supply a custom volume to override this.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::setDefaultVolume( cgFloat volume )
{
    // Clamp volume
    volume = cgMathUtility::clamp( volume, 0, 1 );

    // Is this a no-op?
    if ( mDefaultVolume == volume )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, mSourceFileName );
        mUpdateProperties.bindParameter( 2, mStreamingBuffer );
        mUpdateProperties.bindParameter( 3, volume );
        mUpdateProperties.bindParameter( 4, mAutoPlay );
        mUpdateProperties.bindParameter( 5, mLooping );
        mUpdateProperties.bindParameter( 6, mMuteOutsideRange );
        mUpdateProperties.bindParameter( 7, mReferenceId );

        // Execute
        if ( mUpdateProperties.step( true ) == false )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update default volume property of sound emitter object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update value.
    mDefaultVolume = volume;

    // Notify listeners that property was altered
    static const cgString context = _T("DefaultVolume");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );
}

//-----------------------------------------------------------------------------
//  Name : enableStreaming()
/// <summary>
/// Set a value indicating whether or not the audio file should be streamed
/// into memory during playback, or loaded into memory in its entirety ahead
/// of time. Streaming is most useful for playback of large files, or in cases
/// where you want little to no loading latency.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::enableStreaming( bool streaming )
{
    // Is this a no-op?
    if ( mStreamingBuffer == streaming )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, mSourceFileName );
        mUpdateProperties.bindParameter( 2, streaming );
        mUpdateProperties.bindParameter( 3, mDefaultVolume );
        mUpdateProperties.bindParameter( 4, mAutoPlay );
        mUpdateProperties.bindParameter( 5, mLooping );
        mUpdateProperties.bindParameter( 6, mMuteOutsideRange );
        mUpdateProperties.bindParameter( 7, mReferenceId );

        // Execute
        if ( mUpdateProperties.step( true ) == false )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update streaming property of sound emitter object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update value.
    mStreamingBuffer = streaming;

    // Notify listeners that property was altered
    static const cgString context = _T("Streaming");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );
}

//-----------------------------------------------------------------------------
//  Name : enableLooping()
/// <summary>
/// Set the value indicating whether or not the audio buffer should be
/// played in a repeating / looping fashion.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::enableLooping( bool looping )
{
    // Is this a no-op?
    if ( mLooping == looping )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, mSourceFileName );
        mUpdateProperties.bindParameter( 2, mStreamingBuffer );
        mUpdateProperties.bindParameter( 3, mDefaultVolume );
        mUpdateProperties.bindParameter( 4, mAutoPlay );
        mUpdateProperties.bindParameter( 5, looping );
        mUpdateProperties.bindParameter( 6, mMuteOutsideRange );
        mUpdateProperties.bindParameter( 7, mReferenceId );

        // Execute
        if ( mUpdateProperties.step( true ) == false )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update looping property of sound emitter object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update value.
    mLooping = looping;

    // Notify listeners that property was altered
    static const cgString context = _T("Looping");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );
}

//-----------------------------------------------------------------------------
//  Name : enableMuteOutsideRange()
/// <summary>
/// Set the value indicating whether or not the audio buffer should be muted
/// once the listener moves outside of the range specified by the outer range
/// property. This can reduce processing overhead.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::enableMuteOutsideRange( bool mute )
{
    // Is this a no-op?
    if ( mMuteOutsideRange == mute )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, mSourceFileName );
        mUpdateProperties.bindParameter( 2, mStreamingBuffer );
        mUpdateProperties.bindParameter( 3, mDefaultVolume );
        mUpdateProperties.bindParameter( 4, mAutoPlay );
        mUpdateProperties.bindParameter( 5, mLooping );
        mUpdateProperties.bindParameter( 6, mute );
        mUpdateProperties.bindParameter( 7, mReferenceId );

        // Execute
        if ( mUpdateProperties.step( true ) == false )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update the mute outside range property of sound emitter object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update value.
    mMuteOutsideRange = mute;

    // Notify listeners that property was altered
    static const cgString context = _T("MuteOutsideRange");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );
}

//-----------------------------------------------------------------------------
//  Name : enableAutoPlay()
/// <summary>
/// Set the value indicating whether or not the emitter should automatically
/// begin playback of the buffer as soon as it is loaded. If this is set to
/// false, playback must be started manually through the cgSoundEmitterNode's
/// 'play()' method.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::enableAutoPlay( bool autoPlay )
{
    // Is this a no-op?
    if ( mAutoPlay == autoPlay )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, mSourceFileName );
        mUpdateProperties.bindParameter( 2, mStreamingBuffer );
        mUpdateProperties.bindParameter( 3, mDefaultVolume );
        mUpdateProperties.bindParameter( 4, autoPlay );
        mUpdateProperties.bindParameter( 5, mLooping );
        mUpdateProperties.bindParameter( 6, mMuteOutsideRange );
        mUpdateProperties.bindParameter( 7, mReferenceId );

        // Execute
        if ( mUpdateProperties.step( true ) == false )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update auto play property of sound emitter object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update value.
    mAutoPlay = autoPlay;

    // Notify listeners that property was altered
    static const cgString context = _T("AutoPlay");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );
}

//-----------------------------------------------------------------------------
//  Name : setOuterRange()
/// <summary>
/// Set the outer range of the sound emitter that represents the distance
/// outside of which the audio will no longer be attenuated. This does not mean
/// that this is the distance at which the emitter becomes silent, simply that
/// it ceases to attenuate at this distance.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::setOuterRange( cgFloat range )
{
    // Is this a no-op?
    if ( mOuterRange == range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRanges.bindParameter( 1, range );
        mUpdateRanges.bindParameter( 2, mInnerRange );
        mUpdateRanges.bindParameter( 3, mReferenceId );

        // Execute
        if ( !mUpdateRanges.step( true ) )
        {
            cgString error;
            mUpdateRanges.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update ranges for sound emitter object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Store the new light source range
    mOuterRange = fabsf(range);

    // Notify listeners that object data has changed.
    static const cgString context = _T("OuterRange");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );

    // Make sure inner range is less than the outer range
    if ( mInnerRange > mOuterRange )
        setInnerRange( mOuterRange );
}

//-----------------------------------------------------------------------------
//  Name : setInnerRange()
/// <summary>
/// Set the inner range of the sound emitter that represents the distance
/// inside of which the audio can be heard with full effect.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterObject::setInnerRange( cgFloat range )
{
    // Is this a no-op?
    if ( mInnerRange == range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRanges.bindParameter( 1, mOuterRange );
        mUpdateRanges.bindParameter( 2, range );
        mUpdateRanges.bindParameter( 3, mReferenceId );

        // Execute
        if ( !mUpdateRanges.step( true ) )
        {
            cgString error;
            mUpdateRanges.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update ranges for light '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Store the new light source range
    mInnerRange = fabsf(range);

    // Notify listeners that object data has changed.
    static const cgString context = _T("InnerRange");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );

    // Make sure outer range is at LEAST as large as the inner range
    if ( mInnerRange > mOuterRange )
        setOuterRange( mInnerRange );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterObject::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgSoundEmitterObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("SoundEmitterObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertSoundEmitter.bindParameter( 1, mReferenceId );
        mInsertSoundEmitter.bindParameter( 2, mSourceFileName );
        mInsertSoundEmitter.bindParameter( 3, mStreamingBuffer );
        mInsertSoundEmitter.bindParameter( 4, mDefaultVolume );
        mInsertSoundEmitter.bindParameter( 5, mAutoPlay );
        mInsertSoundEmitter.bindParameter( 6, mLooping );
        mInsertSoundEmitter.bindParameter( 7, mMuteOutsideRange );
        mInsertSoundEmitter.bindParameter( 8, mOuterRange );
        mInsertSoundEmitter.bindParameter( 9, mInnerRange );
        mInsertSoundEmitter.bindParameter( 10, mSoftRefCount );

        // Execute
        if ( !mInsertSoundEmitter.step( true ) )
        {
            cgString error;
            mInsertSoundEmitter.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for sound emitter object '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("SoundEmitterObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("SoundEmitterObject::insertComponentData") );

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
bool cgSoundEmitterObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the dummy data.
    prepareQueries();
    mLoadSoundEmitter.bindParameter( 1, e->sourceRefId );
    if ( !mLoadSoundEmitter.step( ) || !mLoadSoundEmitter.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( mLoadSoundEmitter.getLastError( error ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for dummy object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for dummy object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadSoundEmitter.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadSoundEmitter;

    // Update our local members
    mLoadSoundEmitter.getColumn( _T("SourceFile"), mSourceFileName );
    mLoadSoundEmitter.getColumn( _T("Streaming"), mStreamingBuffer );
    mLoadSoundEmitter.getColumn( _T("DefaultVolume"), mDefaultVolume );
    mLoadSoundEmitter.getColumn( _T("AutoPlay"), mAutoPlay );
    mLoadSoundEmitter.getColumn( _T("Looping"), mLooping );
    mLoadSoundEmitter.getColumn( _T("MuteOutsideRange"), mMuteOutsideRange );
    mLoadSoundEmitter.getColumn( _T("OuterRange"), mOuterRange );
    mLoadSoundEmitter.getColumn( _T("InnerRange"), mInnerRange );

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
void cgSoundEmitterObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertSoundEmitter.isPrepared() == false )
            mInsertSoundEmitter.prepare( mWorld, _T("INSERT INTO 'Objects::SoundEmitter' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?19)"), true );
        if ( mUpdateProperties.isPrepared() == false )
            mUpdateProperties.prepare( mWorld, _T("UPDATE 'Objects::SoundEmitter' SET SourceFile=?1, Streaming=?2, DefaultVolume=?3, AutoPlay=?4, Looping=?5, MuteOutsideRange=?6 WHERE RefId=?7"), true );
        if ( mUpdateRanges.isPrepared() == false )
            mUpdateRanges.prepare( mWorld, _T("UPDATE 'Objects::SoundEmitter' SET OuterRange=?1, InnerRange=?2 WHERE RefId=?3"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadSoundEmitter.isPrepared() == false )
        mLoadSoundEmitter.prepare( mWorld, _T("SELECT * FROM 'Objects::SoundEmitter' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgSoundEmitterNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSoundEmitterNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSoundEmitterNode::cgSoundEmitterNode( cgUInt32 referenceId, cgScene * scene ) : cgObjectNode( referenceId, scene )
{
    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("SoundEmitter%X"), referenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgSoundEmitterNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSoundEmitterNode::cgSoundEmitterNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgObjectNode( referenceId, scene, init, initMethod, initTransform )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgSoundEmitterNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSoundEmitterNode::~cgSoundEmitterNode()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgSoundEmitterNode::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgSoundEmitterNode( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgSoundEmitterNode::allocateClone( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform )
{
    return new cgSoundEmitterNode( referenceId, scene, init, initMethod, initTransform );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterNode::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    mAudioBuffer.close();

    // Dispose base.
    if ( disposeBase )
        cgObjectNode::dispose( true );
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
bool cgSoundEmitterNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SoundEmitterNode )
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
void cgSoundEmitterNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    cgAudioBuffer * buffer = mAudioBuffer.getResource(true);
    if ( buffer && !buffer->isLoaded() )
        buffer = CG_NULL;

    // What property was modified?
    if ( e->context == _T("OuterRange") || e->context == _T("ApplyRescale") )
    {
        // Update the audio buffer ranges.
        if ( buffer )
            buffer->set3DRangeProperties( getInnerRange(), getOuterRange() );

        // Emitter is now 'dirty' and should trigger a bounding box 
        // recomputation next time it is necessary.
        nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );

    } // End if OuterRange | ApplyRescale
    else if ( e->context == _T("InnerRange") )
    {
        // Update the audio buffer ranges.
        if ( buffer )
            buffer->set3DRangeProperties( getInnerRange(), getOuterRange() );

    } // End if InnerRange
    else if ( e->context == _T("DefaultVolume" ))
    {
        if ( buffer )
            buffer->setVolume( getDefaultVolume() );

    } // End if DefaultVolume
    else if ( e->context == _T("SourceFile") || e->context == _T("Streaming") || e->context == _T("MuteOutsideRange") )
    {
        // We need to reload the audio buffer with a new sound.
        reloadBuffer();

    } // End if SourceFile | Streaming | MuteOutsideRange
    else if ( e->context == _T("Looping") )
    {
        // Stop and restart the buffer with the new properties.
        stop();
        play();

    } // End if SourceFile | Streaming

    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : onNodeInit () (Virtual)
/// <summary>
/// Optional method called both after creation and during loading to allow the
/// node to finally initialize itself with all relevant scene data it may
/// depend upon being available. In cases where reference identifiers pointing 
/// to nodes may have been remapped during loading (i.e. in cases where 
/// performing a cloned load), information about the remapping is provided to 
/// allow the node (or its underlying object) to take appropriate action.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterNode::onNodeInit( const cgUInt32IndexMap & nodeReferenceRemap )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeInit( nodeReferenceRemap ) )
        return false;

    // Load the audio file.
    reloadBuffer();
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : reloadBuffer () (Protected)
/// <summary>
/// Setup the audio buffer based on the specified properties and automatically
/// commence playback where requested.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterNode::reloadBuffer( )
{
    // Stop and close the old buffer
    stop();
    mAudioBuffer.close();

    // Don't do anything when in full sandbox mode. We don't want audio playing in 
    // the editor unless we're previewing the scene.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        return;

    // Load the referenced audio buffer
    if ( !getSourceFile().empty() )
    {
        cgResourceManager * resources = mParentScene->getResourceManager();
        cgUInt32 flags = cgAudioBufferFlags::Complex3D;
        if ( isStreaming() )
            flags |= cgAudioBufferFlags::Streaming;
        if ( shouldMuteOutsideRange() )
            flags |= cgAudioBufferFlags::MuteAtMaxDistance;
        if ( !resources->loadAudioBuffer( &mAudioBuffer, getSourceFile(), flags, 0, cgDebugSource() ) )
            cgAppLog::write( cgAppLog::Warning, _T("Unable to load audio buffer from '%s' when initializing sound emitter node '0x%x'.\n"), getSourceFile().c_str(), mReferenceId );

        // Setup the initial audio buffer properties.
        cgAudioBuffer * audioBuffer = mAudioBuffer.getResource(true);
        if ( audioBuffer )
        {
            audioBuffer->setVolume( getDefaultVolume() );
            audioBuffer->set3DSoundPosition( getPosition(false) );
            audioBuffer->set3DRangeProperties( getInnerRange(), getOuterRange() );

            // Begin playing immediately?
            if ( shouldAutoPlay() )
            {
                audioBuffer->play( isLooping() );

            } // End if auto play

        } // End if valid

    } // End if valid source
}

//-----------------------------------------------------------------------------
// Name : play ()
/// <summary>
/// Begin playback of the audio buffer represented by this sound emitter.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterNode::play( )
{
    cgAudioBuffer * buffer = mAudioBuffer.getResource(true);
    if ( !buffer || !buffer->isLoaded() )
        return;

    // Begin playing.
    buffer->play( isLooping() );
}

//-----------------------------------------------------------------------------
// Name : stop ()
/// <summary>
/// Stop playback of the audio buffer represented by this sound emitter.
/// </summary>
//-----------------------------------------------------------------------------
void cgSoundEmitterNode::stop( )
{
    cgAudioBuffer * buffer = mAudioBuffer.getResource(true);
    if ( !buffer || !buffer->isLoaded() )
        return;

    // Stop playback
    buffer->stop( );
}

//-----------------------------------------------------------------------------
// Name : isPlaying ()
/// <summary>
/// Determine whether or not the audio buffer represented by this sound emitter
/// is currently in the process of playback.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterNode::isPlaying( )
{
    cgAudioBuffer * buffer = mAudioBuffer.getResource(true);
    if ( !buffer || !buffer->isLoaded() )
        return false;
    return buffer->isPlaying( );
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterNode::canScale( ) const
{
    // Sound emitters cannot be scaled
    return false;
}

//-----------------------------------------------------------------------------
// Name : canRotate ( )
/// <summary>Determine if rotation of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterNode::canRotate( ) const
{
    // Sound emitters cannot be rotated
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform() (Override)
/// <summary>
/// Update our internal cell matrix with that specified here.
/// Note : Hooked into base class so that we can updatate our own properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgObjectNode::setCellTransform( Transform, Source ) )
        return false;

    // Reposition the audio buffer
    cgAudioBuffer * buffer = mAudioBuffer.getResource(true);
    if ( buffer && buffer->isLoaded() )
        buffer->set3DSoundPosition( getPosition(false) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSandboxIconInfo ( ) (Virtual)
/// <summary>
/// Retrieve information about the iconic representation of this object as it
/// is to be displayed in the sandbox rendering viewports.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSoundEmitterNode::getSandboxIconInfo( cgCameraNode * camera, const cgSize & viewportSize, cgString & atlasFile, cgString & frameName, cgVector3 & iconOrigin )
{
    atlasFile  = _T("sys://Textures/ObjectBillboardSheet.xml");
    frameName  = _T("Audio");

    // Position icon
    iconOrigin = getPosition(false);
    cgFloat zoomFactor = camera->estimateZoomFactor( viewportSize, iconOrigin, 2.5f );
    iconOrigin += (camera->getXAxis() * 0.0f * zoomFactor) + (camera->getYAxis() * 17.0f * zoomFactor);
    return true;
}
