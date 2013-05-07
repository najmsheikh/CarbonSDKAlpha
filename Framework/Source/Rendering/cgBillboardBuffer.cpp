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
// Name : cgBillboardBuffer.cpp                                              //
//                                                                           //
// Desc : Contains classes which provide support for both three dimensional  //
//        and two dimensional (pre-transformed), screen oriented billboards. //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBillboardBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgBillboardBuffer.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgSampler.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgTexture.h>
#include <Resources/cgVertexBuffer.h>
#include <Resources/cgIndexBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <World/Objects/cgCameraObject.h>
#include <System/cgXML.h>

///////////////////////////////////////////////////////////////////////////////
// cgBillboardBuffer Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBillboardBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardBuffer::cgBillboardBuffer( )
{
    // Initialize variables to sensible defaults
    mFlags            = 0;
    mDriver           = CG_NULL;
    mResources        = CG_NULL;
    mBufferDirty      = false;
    mSampler          = CG_NULL;
    mVertexFormat     = CG_NULL;
    
    // Clear structures
    memset( &mTechniques, 0, sizeof(Techniques) );
}

//-----------------------------------------------------------------------------
//  Name : ~cgBillboardBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboardBuffer::~cgBillboardBuffer( )
{
    // Release allocated resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgBillboardBuffer::dispose( bool disposeBase )
{
    clear( true );
}

//-----------------------------------------------------------------------------
//  Name : clear ()
/// <summary>
/// Empty the billboard buffer, optionally destroying the assigned billboard
/// objects. After clearning, the buffer can be re-initialized once again with 
/// a call to prepareBuffer(), etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardBuffer::clear( bool destroyBillboards )
{
    // Release allocated resources.
    mRenderShader.close(true);
    mRenderVertices.close(true);
    mRenderIndices.close(true);
    mSortedIndices.close(true);
    mTexture.close();

    // Iterate through billboards and release them if requested
    if ( destroyBillboards )
    {
        BillboardArray::iterator itBillboard;
        for ( itBillboard = mBillboards.begin(); itBillboard != mBillboards.end(); ++itBillboard )
            (*itBillboard)->scriptSafeDispose();
    
    } // End if destroy
    mBillboards.clear();

    // Release allocated memory
    mSysVertices.clear();
    if ( mSampler )
        mSampler->scriptSafeDispose();

    // Clear variables
    mFlags            = 0;
    mDriver           = CG_NULL;
    mResources        = CG_NULL;
    mBufferDirty      = false;
    mSampler          = CG_NULL;
    mVertexFormat     = CG_NULL;

    // Clear structures
    memset( &mTechniques, 0, sizeof(Techniques) );
    
    // Clear containers
    mFrameGroups.clear();
    mFrameGroupNames.clear();

}

//-----------------------------------------------------------------------------
//  Name : prepareBuffer ()
/// <summary>
/// Initialize the buffer ready for population.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::prepareBuffer( cgUInt32 flags, cgRenderDriver * driver, cgInputStream textureFile )
{
    return prepareBuffer( flags, driver, textureFile, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : prepareBuffer ()
/// <summary>
/// Initialize the buffer ready for population.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::prepareBuffer( cgUInt32 flags, cgRenderDriver * driver, cgInputStream textureFile, cgInputStream shaderFile )
{
    // Cannot initialize more than once
    if ( mDriver != CG_NULL )
        return false;

    // Validate requirements
    if ( driver == CG_NULL )
        return false;

    // Store the specified values
    mDriver        = driver;
    mResources     = driver->getResourceManager();
    mFlags         = flags;

    // First, attempt to load the texture (also retrieves the texture information).
    if ( mResources->loadTexture( &mTexture, textureFile, 0, cgDebugSource() ) == false )
        return false;
    mTextureInfo = (mTexture.getResource(false))->getInfo();

    // No surface shader referenced?
    if ( shaderFile.getName().empty() == true )
        shaderFile.setStreamSource( _T("sys://Shaders/Billboards.sh") );

    // Also, attempt to load the surface shader.
    if ( mResources->createSurfaceShader( &mRenderShader, shaderFile, 0, cgDebugSource() ) == false )
        return false;
        
    // Create a sampler to bind the texture / sampler data to the shader
    if ( (mSampler = mResources->createSampler( _T("BillboardInput"), mRenderShader )) == CG_NULL )
        return false;
    if ( mSampler->setTexture( mTexture ) == false )
        return false;
    
    // Select the correct vertex format
    if ( flags & ScreenSpace )
        mVertexFormat = cgVertexFormat::formatFromDeclarator( cgBillboard2DVertex::Declarator );
    else
        mVertexFormat = cgVertexFormat::formatFromDeclarator( cgBillboard3DVertex::Declarator );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : prepareBufferFromAtlas ()
/// <summary>
/// Initialize the buffer ready for population based upon the image
/// atlas definition file specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::prepareBufferFromAtlas( cgUInt32 flags, cgRenderDriver * driver, cgInputStream atlasFile )
{
    return prepareBufferFromAtlas( flags, driver, atlasFile, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : prepareBufferFromAtlas ()
/// <summary>
/// Initialize the buffer ready for population based upon the image
/// atlas definition file specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::prepareBufferFromAtlas( cgUInt32 flags, cgRenderDriver * driver, cgInputStream atlasFile, cgInputStream shaderFile )
{
    // Open and parse the XML file
    cgXMLDocument document;
    cgXMLError::Result result = document.open( atlasFile, _T("ImageAtlas") );

    // Any error parsing XML file?
    if ( result != cgXMLError::Success )
    {
        if ( result == cgXMLError::NoDocumentTag )
        {
            // Write to the registered output streams
            cgAppLog::write( cgAppLog::Error, _T("%s : First tag 'ImageAtlas' not found when parsing XML.\n"), atlasFile.getName().c_str() );
        
        } // End if first tag not found
        else
        {
            // Write to the registered output streams
            // ToDo: Support line / column
            cgAppLog::write( cgAppLog::Error, _T("%s(%d,%d) : %s\n"), atlasFile.getName().c_str(),
                             0, 0, cgXMLDocument::getErrorDescription( result ).c_str() );
        
        } // End if parse error

        return false;
    
    } // End if failed to parse

    // Get the texture source item
    cgXMLNode documentNode = document.getDocumentNode();
    cgXMLNode childNode = documentNode.getChildNode( _T("Source") );
    if ( childNode.isEmpty() == true )
    {
        // Write to the registered output streams
        cgAppLog::write( cgAppLog::Error, _T("%s : Failed to parse image atlas XML data. Missing 'Source' node detected.\n"), atlasFile.getName().c_str() );
        return false;

    } // End if failed

    // Make relative to atlas file.
    // ToDo: Test this for reliability
    cgString textureFile = cgFileSystem::getDirectoryName(atlasFile.getName()) + _T("/") + childNode.getText();

    // Pass through to standard preparation method before we parse groups etc.
    if ( prepareBuffer( flags, driver, textureFile, shaderFile ) == false )
        return false;

    // Attempt to parse the loaded data
    if ( parseAtlas( documentNode ) == false )
    {
        // Write to the registered output streams
        cgAppLog::write( cgAppLog::Error, _T("%s : Failed to parse image atlas XML data, possibly contains invalid or missing nodes.\n"), atlasFile.getName().c_str() );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseAtlas () (Private)
/// <summary>
/// Parse the data loaded from the XML file relating to the image atlas.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::parseAtlas( const cgXMLNode & mainNode )
{
    // Find all child group nodes
    for ( cgUInt32 i = 0; ; )
    {
        // Retrieve the group node
        cgXMLNode groupNode = mainNode.getNextChildNode( _T("Group"), i );
        if ( groupNode.isEmpty() == true )
            break;

        // Add this image group to the billboard buffer
        cgInt16 groupIndex;
        cgString groupName;
        if ( groupNode.getAttributeText( _T("name"), groupName ) == true )
            groupIndex = addFrameGroup( groupName );
        else
            groupIndex = addFrameGroup( );

        // Find all child image nodes
        for ( cgUInt32 j = 0; ; )
        {
            // Retrieve the image node
            cgXMLNode imageNode = groupNode.getNextChildNode( _T("Image"), j );
            if ( imageNode.isEmpty() == true )
                break;

            // Check to ensure all required properties exist
            if ( imageNode.definesAttribute( _T("x") ) == false ||
                 imageNode.definesAttribute( _T("y") ) == false ||
                 imageNode.definesAttribute( _T("width") ) == false ||
                 imageNode.definesAttribute( _T("height") ) == false )
                 continue;

            // Retrieve rectangle
            cgRect imageBounds;
            cgStringParser( imageNode.getAttributeText( _T("x") ) ) >> imageBounds.left;
            cgStringParser( imageNode.getAttributeText( _T("y") ) ) >> imageBounds.top;
            cgStringParser( imageNode.getAttributeText( _T("width") ) ) >> imageBounds.right;
            cgStringParser( imageNode.getAttributeText( _T("height") ) ) >> imageBounds.bottom;
            imageBounds.right  += imageBounds.left;
            imageBounds.bottom += imageBounds.top;

            // Add this frame to the billboard buffer
            cgString frameName;
            if ( imageNode.getAttributeText( _T("name"), frameName ) == true )
                addFrame( groupIndex, imageBounds, frameName );
            else
                addFrame( groupIndex, imageBounds );

        } // Next child image node
        
    } // Next child group node

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addBillboard ()
/// <summary>
/// Adds a billboard to this effects buffer ready for processing and
/// rendering.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgBillboardBuffer::addBillboard( cgBillboard * billboard )
{
    // Validate requirements
    if ( billboard == CG_NULL )
        return -1;
    
    // Set the billboard's internal properties
    billboard->mBuffer      = this;
    billboard->mBillboardId = (cgInt32)mBillboards.size();

    // Add the billboard to our internal array
    mBillboards.push_back( billboard );

    // Allocate space in the system memory vertex container (in case the billboard
    // wants to update immediately).
    if ( mFlags & ScreenSpace )
        mSysVertices.resize( mSysVertices.size() + 4 * sizeof(cgBillboard2DVertex) );
    else
        mSysVertices.resize( mSysVertices.size() + 4 * sizeof(cgBillboard3DVertex) );

    // The billboard buffer is now dirty and will need to be
    // updated when drawing takes place.
    mBufferDirty = true;

    // Return the index for this billboard
    return (cgInt32)mBillboards.size() - 1;
}

//-----------------------------------------------------------------------------
//  Name : buildUniformFrames ()
/// <summary>
/// A utility class which can be used to build a simple uniform grid
/// of frames in the first frame group. This is useful if the billboard
/// simply contains 1 or more frames of a consistant size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::buildUniformFrames( cgInt16 frameCount, cgInt16 framePitch )
{
    const cgUInt32 framesWide  = (cgUInt32)framePitch;
    const cgUInt32 framesHigh  = (cgUInt32)frameCount / framesWide;
    const cgUInt32 frameWidth  = mTextureInfo.width / framesWide;
    const cgUInt32 frameHeight = mTextureInfo.height / framesHigh;
    
    // For each row
    for ( cgUInt32 y = 0; y < framesHigh; ++y )
    {
        // For each column
        for ( cgUInt32 x = 0; x < framesWide; ++x )
        {
            // Build the rectangle
            cgRect frameBounds;
            frameBounds.left   = x * frameWidth;
            frameBounds.right  = frameBounds.left + frameWidth;
            frameBounds.top    = y * frameHeight;
            frameBounds.bottom = frameBounds.top + frameHeight;

            // Add the frame
            addFrame( 0, frameBounds );

        } // Next Column

    } // Next Row

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addFrameGroup ()
/// <summary>
/// Different frame data can be added to separate frame groups. This
/// allows you to for instance pack different animations into the same
/// texture and reference them correctly.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgBillboardBuffer::addFrameGroup()
{
    // Ensure we don't go out of bounds
    if ( mFrameGroups.size() == 0x7FFF )
        return -1;
    
    // Resize and return the index to the new group
    mFrameGroups.resize( mFrameGroups.size() + 1 );
    return (cgInt16)mFrameGroups.size() - 1;
}

//-----------------------------------------------------------------------------
//  Name : addFrameGroup () (Overload)
/// <summary>
/// Overload of the primary 'addFrameGroup' method, that allows a name 
/// to be assigned to the group.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgBillboardBuffer::addFrameGroup( const cgString & groupName )
{
    cgInt16 groupIndex = -1;

    // Group with this name already exists?
    if ( mFrameGroupNames.find( groupName ) != mFrameGroupNames.end() )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("A billboard frame group with the name '%s' already exists in this billboard buffer.\n"), groupName.c_str() );
        return -1;
    
    } // End if already exists

    // Add the group
    if ( (groupIndex = addFrameGroup()) < 0 )
        return -1;

    // Add to the name map and return the group index
    mFrameGroupNames[ groupName ] = groupIndex;
    return groupIndex;
}

//-----------------------------------------------------------------------------
//  Name : addFrame ()
/// <summary>
/// Add an individual frame of animation to the specified group.
/// Note : The rectangle specified must be in pixels.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgBillboardBuffer::addFrame( cgInt16 groupIndex, const cgRect & pixelBounds )
{
    FrameDesc frame;

    // No main frame group added?
    if ( mFrameGroups.size() == 0 )
    {        
        // Always automatically add one main frame group if none already exists
        mFrameGroups.resize( 1 );

    } // End if no default group

    // Is the group index or frame out of bounds?
    if ( groupIndex < 0 || groupIndex >= (cgInt16)mFrameGroups.size() )
        return -1;
    if ( mFrameGroups[groupIndex].frames.size() == 0x7FFF )
        return -1;

    // Compute the texture space rectangle
    frame.textureBounds.left   = ((cgFloat)pixelBounds.left + 0.0f) / (cgFloat)mTextureInfo.width;
    frame.textureBounds.top    = ((cgFloat)pixelBounds.top + 0.0f) / (cgFloat)mTextureInfo.height;
    frame.textureBounds.right  = ((cgFloat)pixelBounds.right + 0.0f) / (cgFloat)mTextureInfo.width;
    frame.textureBounds.bottom = ((cgFloat)pixelBounds.bottom + 0.0f) / (cgFloat)mTextureInfo.height;

    // Store remaining details
    frame.bounds = pixelBounds;
    
    // Add to the group and return the new index
    mFrameGroups[groupIndex].frames.push_back( frame );
    return (cgInt16)mFrameGroups[groupIndex].frames.size() - 1;
}

//-----------------------------------------------------------------------------
//  Name : addFrame () (Overload)
/// <summary>
/// Overload of the primary 'addFrame' method, that allows a name to be 
/// assigned to the frame.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgBillboardBuffer::addFrame( cgInt16 groupIndex, const cgRect & pixelBounds, const cgString & frameName )
{
    cgInt16 frameIndex = -1;

    // If the group index is in range, see if a frame with the 
    // specified name already exists in the group
    if ( groupIndex >= 0 && groupIndex < (cgInt16)mFrameGroups.size() )
    {
        // Retrieve the group
        FrameGroup & group = mFrameGroups[ groupIndex ];

        // Frame with this name already exists?
        if ( group.frameNames.find( frameName ) != group.frameNames.end() )
        {
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("A billboard frame with the name '%s' already exists in this billboard group.\n"), frameName.c_str() );
            return -1;
        
        } // End if already exists

    } // End if not out of range

    // Add to this group
    frameIndex = addFrame( groupIndex, pixelBounds );
    if ( frameIndex < 0 ) return -1;

    // Add to the name map and return the frame index
    mFrameGroups[ groupIndex ].frameNames[ frameName ] = frameIndex;
    return frameIndex;
}

//-----------------------------------------------------------------------------
//  Name : getSurfaceShader ()
/// <summary>
/// Retrieve the surface shader associated with this billboard buffer and used
/// to supply rendering states and shader code.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderHandle cgBillboardBuffer::getSurfaceShader( ) const
{
    return mRenderShader;
}

//-----------------------------------------------------------------------------
//  Name : getTExture ()
/// <summary>
/// Retrieve the texture associated with this billboard buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgTextureHandle cgBillboardBuffer::getTexture( ) const
{
    return mTexture;
}

//-----------------------------------------------------------------------------
//  Name : getFrameGroupIndex ()
/// <summary>
/// Get the index of the frame group based on the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgBillboardBuffer::getFrameGroupIndex( const cgString & groupName ) const
{
    // Find a group with this name?
    IndexMap::const_iterator itName = mFrameGroupNames.find( groupName );
    if ( itName == mFrameGroupNames.end() ) return -1;

    // Return the index
    return itName->second;
}

//-----------------------------------------------------------------------------
//  Name : getFrameIndex ()
/// <summary>
/// Get the index of the frame based on the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgBillboardBuffer::getFrameIndex( cgInt16 groupIndex, const cgString & frameName ) const
{
    // Is the group index out of bounds?
    if ( groupIndex < 0 || groupIndex >= (cgInt16)mFrameGroups.size() )
        return -1;

    // Retrieve the group
    const FrameGroup & group = mFrameGroups[ groupIndex ];

    // Find a frame with this name?
    IndexMap::const_iterator itName = group.frameNames.find( frameName );
    if ( itName == group.frameNames.end() ) return -1;

    // Return the index
    return itName->second;
}

//-----------------------------------------------------------------------------
//  Name : getFrameData ()
/// <summary>
/// Retrieve the frame description structure for an individual frame of
/// animation in the billboard.
/// </summary>
//-----------------------------------------------------------------------------
const cgBillboardBuffer::FrameDesc * cgBillboardBuffer::getFrameData( cgInt16 groupIndex, cgInt16 frameIndex ) const
{
    // Is the group index or frame out of bounds?
    if ( groupIndex < 0 || groupIndex >= (cgInt16)mFrameGroups.size() )
        return CG_NULL;
    
    // Get the group descriptor
    const FrameGroup & group = mFrameGroups[groupIndex];

    // Is the frame out of bounds?
    if ( frameIndex < 0 || frameIndex > (cgInt16)group.frames.size() )
        return CG_NULL;

    // Return the frame
    return &group.frames[frameIndex];
}

//-----------------------------------------------------------------------------
//  Name : endPrepare ()
/// <summary>
/// Call this function once the buffer has been entirely prepared and
/// the render data should be constructed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::endPrepare( )
{
    // If no texture frames were added, add one that fills the entire 
    // texture.
    if ( mFrameGroups.size() == 0 )
    {
        cgRect rcFrame( 0, 0, mTextureInfo.width, mTextureInfo.height );
        addFrame( 0, rcFrame );
    
    } // End if no frames added

    // Perform initial update for all billboards
    for ( size_t i = 0; i < mBillboards.size(); ++i )
        mBillboards[i]->update();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Actually perform the render of the buffer data.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardBuffer::render( cgInt32 billboardBegin /* = 0 */, cgInt32 billboardCount /* = -1 */, bool precise /* = true */ )
{
    // Validate requirements
    if ( mDriver == CG_NULL || billboardCount == 0 )
        return;

    // Validate starting index
    if ( billboardBegin < 0 )
        billboardBegin = 0;
    if ( billboardBegin >= (signed)mBillboards.size() )
        return;

    // If we're specifying some subset of the billboard array, check it doesn't overflow
    if ( billboardCount < 0 ||
        (billboardCount >= 0 && billboardBegin + billboardCount > (signed)mBillboards.size() ) )
    {
        // Set to full buffer size from starting point
        billboardCount = (cgInt32)mBillboards.size() - billboardBegin;
    
    } // End if overflow, or full buffer request

    // If vertex / index buffers have not yet been created, or 
    // they are lost or dirty reconstruct them.
    if ( restoreBuffers() == false )
        return;

    // Now build the information that allow us to render only the billboards requested.
    const cgUInt32 startIndex    = billboardBegin * 6;
    const cgUInt32 triangleCount = billboardCount * 2;
    const cgUInt32 minVertex     = billboardBegin * 4;
    const cgUInt32 maxVertex     = minVertex + ((billboardCount * 4) - 1);

    // Apply the texture to the effect.
    mSampler->apply();

    // Select the correct technique for rendering
    cgSurfaceShader * pShader = mRenderShader.getResource(true);
    if ( !selectTechnique( pShader, precise ) )
        return;

    // Set the streams
    mDriver->setVertexFormat( mVertexFormat );
    mDriver->setIndices( mRenderIndices );
    mDriver->setStreamSource( 0, mRenderVertices );

    // Begin rendering the billboards.
    if ( pShader->beginTechnique() )
    {
        // Loop through and render for each pass
        while ( pShader->executeTechniquePass( ) == cgTechniqueResult::Continue )
            mDriver->drawIndexedPrimitive( cgPrimitiveType::TriangleList, 0, minVertex, 
                                            (maxVertex - minVertex) + 1, startIndex, triangleCount );

        // We're done.
        pShader->endTechnique();

    } // Next Pass
}

//-----------------------------------------------------------------------------
//  Name : billboardDepthCompare () (Private, Static)
/// <summary>
/// qsort compatible comparison function to dest the depth
/// </summary>
//-----------------------------------------------------------------------------
int cgBillboardBuffer::billboardDepthCompare( const void *arg1, const void *arg2 )
{
   // (Note: swapped compare order because we want those furthest away first)
   if ( ((DepthSortInfo*)arg1)->depth > ((DepthSortInfo*)arg2)->depth )
       return -1;
   if ( ((DepthSortInfo*)arg1)->depth < ((DepthSortInfo*)arg2)->depth )
       return 1;
   return 0;
}

//-----------------------------------------------------------------------------
//  Name : renderSorted ()
/// <summary>
/// Sort the billboards based on distance and render them in a back to
/// front order.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardBuffer::renderSorted( cgCameraNode * camera, cgMatrix * localTransform, cgInt32 billboardBegin /* = 0 */, cgInt32 billboardCount /* = -1 */, bool precise /* = true */ )
{
    // Validate requirements
    if ( camera == CG_NULL )
        return;

    // Did the user request that sorted rendering be supported?
    if ( (mFlags & SupportSorting) != SupportSorting )
    {
        // Sorting not supported. Pass through to standard render method.
        render( billboardBegin, billboardCount );
        return;
    
    } // End if no sorting

    // Validate requirements
    if ( !mDriver || !camera || !billboardCount )
        return;

    // Validate starting index
    if ( billboardBegin < 0 ) billboardBegin = 0;
    if ( billboardBegin >= (signed)mBillboards.size() ) return;

    // If we're specifying some subset of the billboard array, check it doesn't overflow
    if ( billboardCount < 0 ||
        (billboardCount >= 0 && billboardBegin + billboardCount > (signed)mBillboards.size() ) )
    {
        // Set to full buffer size from starting point
        billboardCount = (cgInt32)mBillboards.size() - billboardBegin;
    
    } // End if overflow, or full buffer request

    // If there were no billboards to render, return (paranoia)
    if ( billboardCount <= 0 )
        return;

    // If vertex / index buffers have not yet been created, or 
    // they are lost or dirty reconstruct them.
    if ( !restoreBuffers() )
        return;

    // Compute the sort origin (usually camera position). For local space billboards, we 
    // need to back transform the camera position into the space of the emitter.
    cgVector3 sortOrigin = camera->getPosition();
    if ( localTransform )
    {
        cgMatrix inverseTransform;
        cgMatrix::inverse( inverseTransform, *localTransform );
        cgVector3::transformCoord( sortOrigin, sortOrigin, inverseTransform );
        
    } // End if LocalBillboards

    // Build list of billboard depth info to be sorted.
    cgInt32 finalCount = 0;
    DepthSortInfo * depthInfo = new DepthSortInfo[ billboardCount ];
    for ( cgInt32 i = billboardBegin; i < billboardBegin + billboardCount; ++i )
    {
        cgBillboard * billboard = mBillboards[i];
        
        // Skip invalid or invisible billboards
        if ( !billboard || !billboard->getVisible() )
            continue;

        // Valid billboard!
        depthInfo[ finalCount ].billboardId = i;
        depthInfo[ finalCount ].depth = cgVector3::lengthSq( billboard->getPosition() - sortOrigin );
        finalCount++;

    } // Next Billboard

    // If nothing made it this far, return (paranoia)
    if ( !finalCount )
    {
        delete []depthInfo;
        return;
    
    } // End if no data

    // Sort the depth list
    qsort( (void*)depthInfo, (size_t)finalCount, sizeof(DepthSortInfo), billboardDepthCompare );

    // Lock the index buffer
    cgUInt32 * indices;
    if ( !(indices = (cgUInt32*)mResources->lockIndexBuffer( mSortedIndices, 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard )) )
    {
        delete []depthInfo;
        return;

    } // End if failed to lock

    // Populate the index buffer
    cgUInt32 minVertex  = INT_MAX, maxVertex = 0;
    for ( cgInt32 i = 0; i < finalCount; ++i )
    {
        cgInt32  billboardId = depthInfo[i].billboardId;
        cgUInt32 vertexStart = billboardId * 4;
        
        // Record minimum and maximum vertices
        // (Note: We use vertexStart + 3 here, even though there are four vertices
        //  because this is the maximum vertex INDEX not a total for the number of vertices
        //  i.e. 0,1,2,3 would be 4 vertices with a max vertex index of 3, or 0+3).
        if ( vertexStart < minVertex ) minVertex = vertexStart;
        if ( vertexStart + 3 > maxVertex ) maxVertex = vertexStart + 3;

        // Build indices for two triangles
        *indices++ = vertexStart;
        *indices++ = vertexStart + 1;
        *indices++ = vertexStart + 2;
        *indices++ = vertexStart;
        *indices++ = vertexStart + 2;
        *indices++ = vertexStart + 3;
        
    } // Next Billboard

    // Unlock buffer and clean up
    mResources->unlockIndexBuffer( mSortedIndices );
    delete []depthInfo;

    // Apply the texture to the effect.
    mSampler->apply();

    // Select the correct technique for rendering
    cgSurfaceShader * shader = mRenderShader.getResource(true);
    if ( !selectTechnique( shader, precise ) )
        return;

    // Set the streams
    mDriver->setVertexFormat( mVertexFormat );
    mDriver->setIndices( mSortedIndices );
    mDriver->setStreamSource( 0, mRenderVertices );

    // Begin rendering the billboards.
    if ( shader->beginTechnique() )
    {
        // Loop through and render for each pass
        while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
            mDriver->drawIndexedPrimitive( cgPrimitiveType::TriangleList, 0, minVertex, 
                                           (maxVertex - minVertex) + 1, 0, finalCount * 2 );

        // We're done.
        shader->endTechnique();

    } // Next Pass
}

//-----------------------------------------------------------------------------
//  Name : restoreBuffers ()
/// <summary>
/// If vertex / index buffers have not yet been created, or they are lost
/// or dirty, this method will reconstruct them.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::restoreBuffers( )
{
    // Any vertex buffer yet (or resource was "lost" / reset)?
    if ( !mRenderVertices.isValid() )
    {
        // Determine vertex buffer usage flags and length
        cgUInt32 length = ((cgUInt32)mBillboards.size() * 4) * mVertexFormat->getStride();
        cgUInt32 usage  = cgBufferUsage::WriteOnly | cgBufferUsage::Dynamic;

        // Create vertex buffer
        if ( mResources->createVertexBuffer( &mRenderVertices, length, usage, mVertexFormat, cgMemoryPool::Default, cgDebugSource() ) == false )
            return false;
        
        // Buffer is automatically dirty
        mBufferDirty = true;
    
    } // End if no vertex buffer yet

    // Any index buffer yet?
    if ( !mRenderIndices.isValid() )
    {
        // Determine index buffer usage flags and length
        cgUInt32 length = ((cgUInt32)mBillboards.size() * 2) * 3 * sizeof(cgUInt32);
        cgUInt32 usage  = cgBufferUsage::WriteOnly;

        // Create index buffer
        if ( mResources->createIndexBuffer( &mRenderIndices, length, usage, cgBufferFormat::Index32, cgMemoryPool::Managed, cgDebugSource() ) == false )
            return false;

        // Build the data
        cgUInt32Array indices( ((cgUInt32)mBillboards.size() * 2) * 3 );
        cgUInt32 * output = &indices.front();
        for ( cgUInt32 i = 0, nCounter = 0; i < mBillboards.size(); ++i )
        {
            // Build indices for two triangles
            *output++ = nCounter;
            *output++ = nCounter + 1;
            *output++ = nCounter + 2;
            *output++ = nCounter;
            *output++ = nCounter + 2;
            *output++ = nCounter + 3;

            // Move along by 4 vertices (quad)
            nCounter += 4;
            
        } // Next billboard

        // Populate the hardware index buffer.
        cgIndexBuffer * indexBuffer = mRenderIndices.getResource(true);
        if ( !indexBuffer || !indexBuffer->updateBuffer( 0, 0, &indices.front() ) )
        {
            mRenderIndices.close();
            return false;

        } // End if update failed

    } // End if no index buffer yet

    // Any sortable index buffer yet or resource was "lost" / reset (if required)?
    if ( ((mFlags & SupportSorting) == SupportSorting)  && (mSortedIndices.isValid() == false) )
    {
        // Determine vertex buffer usage flags and length
        cgUInt32 length = ((cgUInt32)mBillboards.size() * 2) * 3 * sizeof(cgUInt32);
        cgUInt32 usage  = cgBufferUsage::WriteOnly | cgBufferUsage::Dynamic;

        // Create index buffer
        if ( mResources->createIndexBuffer( &mSortedIndices, length, usage, cgBufferFormat::Index32, cgMemoryPool::Default, cgDebugSource() ) == false )
            return false;
        
    } // End if no sortable index buffer yet

    // Populate vertex buffer
    if ( mBufferDirty || mRenderVertices.isResourceLost() )
    {
        cgVertexBuffer * vertexBuffer = mRenderVertices.getResource(true);
        if ( !vertexBuffer || !vertexBuffer->updateBuffer( 0, 0, &mSysVertices.front() ) )
            return false;
        
        // Resource has been restored.
        mRenderVertices.setResourceLost( false );

        // Buffer is no longer dirty
        mBufferDirty = false;
    
    } // End if buffer is dirty

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : selectTechnique () (Protected)
/// <summary>
/// Select the correct rendering technique for this billboard buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboardBuffer::selectTechnique( cgSurfaceShader * shader, bool precise /* = true */ )
{
    cgToDo( "Carbon General", "Technique caching mechanism fails after 'cgResourceManager::ReloadShaders()', disabled for now" );

    if ( mFlags & ScreenSpace )
    {
        if ( precise )
        {
            //if ( !mTechniques.precise2D )
            {
                static const cgString techniqueName = _T("billboard2D");
                mTechniques.precise2D = shader->getTechnique( techniqueName );
            } // End if no handle
            return shader->selectTechnique( mTechniques.precise2D );
        
        } // End if precise
        else
        {
            //if ( !mTechniques.standard2D )
            {
                static const cgString techniqueName = _T("billboard2DFiltered");
                mTechniques.standard2D = shader->getTechnique( techniqueName );
            } // End if no handle
            return shader->selectTechnique( mTechniques.standard2D );

        } // End if !precise

    } // End if ScreenSpace
    else if ( mFlags & OrientationY )
    {
        //if ( !mTechniques.upAxisLocked3D )
        {
            static const cgString techniqueName = _T("billboardYLocked3D");
            mTechniques.upAxisLocked3D = shader->getTechnique( techniqueName );
        } // End if no handle
        return shader->selectTechnique( mTechniques.upAxisLocked3D );

    } // End if OrientationY
    else if ( mFlags & OrientationAxis )
    {
        //if ( !mTechniques.customAxisLocked3D )
        {
            static const cgString techniqueName = _T("billboardAxisLocked3D");
            mTechniques.customAxisLocked3D = shader->getTechnique( techniqueName );
        } // End if no handle
        return shader->selectTechnique( mTechniques.customAxisLocked3D );

    } // End if OrientationY
    else
    {
        //if ( !mTechniques.standard3D )
        {
            static const cgString techniqueName = _T("billboard3D");
            mTechniques.standard3D = shader->getTechnique( techniqueName );
        } // End if no handle
        return shader->selectTechnique( mTechniques.standard3D );

    } // End if other
}

//-----------------------------------------------------------------------------
//  Name : getBillboard ()
/// <summary>
/// Retrieve the physical billboard object at the specified location.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboard * cgBillboardBuffer::getBillboard( cgInt32 billboardId )
{
    // Bounds check
    if ( billboardId < 0 || billboardId >= (cgInt32)mBillboards.size() )
        return CG_NULL;

    // Return the billboard
    return mBillboards[ billboardId ];
}

//-----------------------------------------------------------------------------
//  Name : getBillboardVertices ()
/// <summary>
/// Retrieve a pointer to the first vertex in the system memory vertex
/// buffer, for the specified billboard.
/// </summary>
//-----------------------------------------------------------------------------
void * cgBillboardBuffer::getBillboardVertices( cgInt32 billboardId )
{
    // Bounds check
    if ( billboardId < 0 || billboardId >= (cgInt32)mBillboards.size() )
        return CG_NULL;

    // Return the first vertex in the billboard (always multiples of 4 vertices)
    if ( mFlags & ScreenSpace )
        return ((cgBillboard2DVertex*)&mSysVertices[0]) + (billboardId * 4);
    else
        return ((cgBillboard3DVertex*)&mSysVertices[0]) + (billboardId * 4);
}

//-----------------------------------------------------------------------------
//  Name : billboardUpdated ()
/// <summary>
/// This function should be called by the application whenever a
/// billboard has been updated (i.e. some of it's properties altered).
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboardBuffer::billboardUpdated( cgInt32 billboardId )
{
    // Bounds check
    if ( billboardId < 0 || billboardId >= (cgInt32)mBillboards.size() )
        return;

    // Retrieve the specified billboard
    cgBillboard * pBillboard = mBillboards[ billboardId ];
    if ( pBillboard == CG_NULL )
        return;

    // Trigger the billboard update routine
    pBillboard->update( );

    // The vertex buffer will now need to be rebuilt
    mBufferDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
// cgBillboard Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBillboard () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboard::cgBillboard( )
{
    mBuffer       = CG_NULL;
    mPosition     = cgVector3( 0, 0, 0 );
    mSize         = cgSizeF( 0, 0 );
    mBillboardId  = -1;
    mVisible      = true;
    mColor        = 0xFFFFFFFF;
    mCurrentGroup = 0;
    mCurrentFrame = 0;
}

//-----------------------------------------------------------------------------
//  Name : ~cgBillboard () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboard::~cgBillboard( )
{
    // Clear variables
}

//-----------------------------------------------------------------------------
//  Name : setSize ()
/// <summary>
/// Set the size of the billboard.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard::setSize( const cgSizeF & size )
{
    mSize = size;
}

//-----------------------------------------------------------------------------
//  Name : setSize ()
/// <summary>
/// Set the size of the billboard.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard::setSize( cgFloat width, cgFloat height )
{
    mSize.width  = width;
    mSize.height = height;
}

//-----------------------------------------------------------------------------
//  Name : setVisible ()
/// <summary>
/// Update the billboard visibility status.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard::setVisible( bool visible )
{
    // Store new visibility status
    mVisible = visible;

}

//-----------------------------------------------------------------------------
//  Name : setColor ()
/// <summary>
/// Update the color of the billboard.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard::setColor( cgUInt32 color )
{
    // Store new color value
    mColor = color;
}

//-----------------------------------------------------------------------------
//  Name : setPosition ()
/// <summary>
/// Set the position of the billboard in world space.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard::setPosition( const cgVector3 & position )
{
    mPosition = position;
}

//-----------------------------------------------------------------------------
//  Name : setPosition ()
/// <summary>
/// Set the position of the billboard in world space.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard::setPosition( cgFloat x, cgFloat y, cgFloat z )
{
    mPosition.x = x;
    mPosition.y = y;
    mPosition.z = z;
}

//-----------------------------------------------------------------------------
//  Name : setFrameGroup ()
/// <summary>
/// Set the 'animation' frame group that should be used when generating
/// texture coordinates for the billboard.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard::setFrameGroup( cgInt16 groupIndex )
{
    mCurrentGroup = groupIndex;
    mCurrentFrame = 0;
}

//-----------------------------------------------------------------------------
//  Name : setFrame ()
/// <summary>
/// Set the 'animation' frame that should be used when generating texture
/// coordinates for the billboard.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard::setFrame( cgInt16 frameIndex, bool autoSetSize /* = false */ )
{
    mCurrentFrame = frameIndex;

    // Update the size?
    if ( autoSetSize )
    {
        const cgBillboardBuffer::FrameDesc * frame = mBuffer->getFrameData( mCurrentGroup, mCurrentFrame );
        if ( frame != CG_NULL )
        {
            setSize( (cgFloat)(frame->bounds.right - frame->bounds.left), 
                     (cgFloat)(frame->bounds.bottom - frame->bounds.top) );
        
        } // End if valid frame

    } // End if automatically set size
}

///////////////////////////////////////////////////////////////////////////////
// cgBillboard3D Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBillboard3D () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboard3D::cgBillboard3D( )
{
    mScale      = cgVector2( 1, 1 );
    mRotation   = 0;
    mDirection  = cgVector3( 0, 0, 0 );
    mHDRScale   = 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : ~cgBillboard3D () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboard3D::~cgBillboard3D( )
{
    // Clear variables
}

//-----------------------------------------------------------------------------
//  Name : setScale ()
/// <summary>
/// Set the scaling factor for the billboard.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard3D::setScale( const cgVector2 & scale )
{
    mScale = scale;
}

//-----------------------------------------------------------------------------
//  Name : setScale ()
/// <summary>
/// Set the scaling factor for the billboard.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard3D::setScale( cgFloat x, cgFloat y )
{
    mScale.x = x;
    mScale.y = y;
}

//-----------------------------------------------------------------------------
//  Name : setRotation ()
/// <summary>
/// Set the amount in degrees that the billboard will be rotated.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard3D::setRotation( cgFloat degrees )
{
    mRotation = degrees;
}

//-----------------------------------------------------------------------------
//  Name : setHDRScale ()
/// <summary>
/// Set the intensity scalar to apply when HDR rendering is enabled.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard3D::setHDRScale( cgFloat scale )
{
    mHDRScale = scale;
}

//-----------------------------------------------------------------------------
//  Name : setDirection ()
/// <summary>
/// Set the direction of the billboard in world space.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard3D::setDirection( const cgVector3 & direction )
{
    mDirection = direction;
}

//-----------------------------------------------------------------------------
//  Name : setDirection ()
/// <summary>
/// Set the direction of the billboard in world space.
/// </summary>
//-----------------------------------------------------------------------------
void cgBillboard3D::setDirection( cgFloat x, cgFloat y, cgFloat z )
{
    mDirection.x = x;
    mDirection.y = y;
    mDirection.z = z;
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Called in order to signify that certain properties of this billboard
/// have been updated which potentially requires our buffer to be updated.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboard3D::update( )
{
    cgBillboard3DVertex * vertices = (cgBillboard3DVertex*)mBuffer->getBillboardVertices( mBillboardId );
    if ( mVisible )
    {
        // Get frame data (optimization = cache this when the frame changes)
        const cgBillboardBuffer::FrameDesc * frame = mBuffer->getFrameData( mCurrentGroup, mCurrentFrame );
        if ( !frame ) return false;

        // Cache references to coordinate data
        const cgRectF & coords = frame->textureBounds;
        
        // Update dynamic data for this billboard
        for ( size_t i = 0; i < 4; ++i )
        {
            // * position - All vertices set to billboard center initially
            vertices[i].position = mPosition;
            // * color - The tint color for the billboard
            vertices[i].color = mColor;
            // * angle - The amount to rotate the billboard relative to the billboard's final orientation
            vertices[i].angle = CGEToRadian( mRotation );
            // * scale - The amount to scale the billboard relative to the billboard's final orientation
            vertices[i].scale = mScale;
            // * hdrScale - The intesity scalar to apply when HDR rendering is enabled.
            vertices[i].hdrScale = mHDRScale;
            // * direction - The optional axis to which a billboard can be locked
            vertices[i].direction = mDirection;

            // Dependant on the vertex being processed
            switch ( i )
            {
                case 0: // Bottom left
                    // * textureCoords - The texture coordinates for this vertex
                    vertices[i].textureCoords = cgVector2( coords.left, coords.bottom );
                    // * offset - The amount to offset the vertex relative to the billboard's final orientation
                    vertices[i].offset = cgVector2( -mSize.width * 0.5f, -mSize.height * 0.5f );
                    break;
                case 1: // Top left
                    // * textureCoords - The texture coordinates for this vertex
                    vertices[i].textureCoords = cgVector2( coords.left, coords.top );
                    // * offset - The amount to offset the vertex relative to the billboard's final orientation
                    vertices[i].offset = cgVector2( -mSize.width * 0.5f, mSize.height * 0.5f );
                    break;
                case 2: // Top Right
                    // * textureCoords - The texture coordinates for this vertex
                    vertices[i].textureCoords = cgVector2( coords.right, coords.top );
                    // * offset - The amount to offset the vertex relative to the billboard's final orientation
                    vertices[i].offset = cgVector2( mSize.width * 0.5f, mSize.height * 0.5f );
                    break;
                case 3: // Bottom Right
                    // * textureCoords - The texture coordinates for this vertex
                    vertices[i].textureCoords = cgVector2( coords.right, coords.bottom );
                    // * offset - The amount to offset the vertex relative to the billboard's final orientation
                    vertices[i].offset = cgVector2( mSize.width * 0.5f, -mSize.height * 0.5f );
                    break;

            } // End switch i
            
        } // Next Vertex

    } // End if visible
    else
    {
        // Set triangles as degenerate
        for ( size_t i = 0; i < 4; ++i )
        {
            vertices[i].position = cgVector3( 0, 0, 0 );
            vertices[i].offset   = cgVector2( 0, 0 );
        
        } // Next Vertex
    
    } // End if invisible

    // Notify parent that we're dirty
    if ( mBuffer )
        mBuffer->setDirty( true );

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgBillboard2D Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBillboard2D () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboard2D::cgBillboard2D( )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgBillboard2D () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBillboard2D::~cgBillboard2D( )
{
    // Clear variables
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Called in order to signify that certain properties of this billboard
/// have been updated which potentially requires our buffer to be updated.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBillboard2D::update( )
{
    cgBillboard2DVertex * vertices = (cgBillboard2DVertex*)mBuffer->getBillboardVertices( mBillboardId );
    if ( mVisible )
    {
        // Get frame data (optimization = cache this when the frame changes)
        const cgBillboardBuffer::FrameDesc * frame = mBuffer->getFrameData( mCurrentGroup, mCurrentFrame );
        if ( !frame ) return false;

        // Cache references to coordinate data
        const cgRectF & coords = frame->textureBounds;
        
        // Update dynamic data for this billboard
        for ( size_t i = 0; i < 4; ++i )
        {
            // * color - The tint color for the billboard
            vertices[i].color = mColor;
            
            // Dependant on the vertex being processed
            switch ( i )
            {
                case 0: // Bottom left
                    // * position - The final screen space position in pixels
                    vertices[i].position = cgVector4( mPosition.x, mPosition.y + mSize.height, mPosition.z, 1.0f );
                    // * textureCoords - The texture coordinates for this vertex
                    vertices[i].textureCoords = cgVector2( coords.left, coords.bottom );
                    break;
                case 1: // Top left
                    // * position - The final screen space position in pixels
                    vertices[i].position = cgVector4( mPosition.x, mPosition.y, mPosition.z, 1.0f );
                    // * textureCoords - The texture coordinates for this vertex
                    vertices[i].textureCoords = cgVector2( coords.left, coords.top );
                    break;
                case 2: // Top Right
                    // * position - The final screen space position in pixels
                    vertices[i].position = cgVector4( mPosition.x + mSize.width, mPosition.y, mPosition.z, 1.0f );
                    // * textureCoords - The texture coordinates for this vertex
                    vertices[i].textureCoords = cgVector2( coords.right, coords.top );
                    break;
                case 3: // Bottom Right
                    // * position - The final screen space position in pixels
                    vertices[i].position = cgVector4( mPosition.x + mSize.width, mPosition.y + mSize.height, mPosition.z, 1.0f );
                    // * textureCoords - The texture coordinates for this vertex
                    vertices[i].textureCoords = cgVector2( coords.right, coords.bottom );
                    break;

            } // End switch i
            
            // Texel -> Pixel mapping correction (Important if you want to avoid "fuzzy" or disjointed characters)
            vertices[i].position.x -= 0.5f;
            vertices[i].position.y -= 0.5f;
            
        } // Next Vertex

    } // End if visible
    else
    {
        // Set triangles as degenerate
        for ( size_t i = 0; i < 4; ++i )
            vertices[i].position = cgVector4( 0, 0, 0, 0 );
    
    } // End if invisible

    // Notify parent that we're dirty
    if ( mBuffer )
        mBuffer->setDirty( true );

    // Success!
    return true;
}