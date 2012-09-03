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
// Name : cgRadianceGrid.cpp                                                 //
//                                                                           //
// Desc : Provides classes responsible for the management of the various     //
//        lighting systems used by the engine.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgRadianceGrid Module Includes
//-----------------------------------------------------------------------------
#include <World/Lighting/cgRadianceGrid.h>
#include <World/Lighting/cgLightingManager.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgLightObject.h>
#include <World/Objects/cgPointLight.h>
#include <World/Objects/cgSpotLight.h>
#include <World/Objects/cgProjectorLight.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgMesh.h>
#include <Resources/cgVertexBuffer.h>
#include <Resources/cgScript.h>
#include <Resources/cgSurfaceShader.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgSampler.h>

///////////////////////////////////////////////////////////////////////////////
// cgRadianceGrid Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRadianceGrid ()
/// <summary>
/// Class constructor
/// </summary>
//-----------------------------------------------------------------------------
cgRadianceGrid::cgRadianceGrid( cgLightingManager * pManager, cgInt32 nCascadeId )
{
    mManager                = pManager;
    mDriver                 = CG_NULL;
    mStaticRadiance         = CG_NULL;
    mDynamicRadiance0       = CG_NULL;     
    mDynamicRadiance1       = CG_NULL;     
    mPaddingCells           = 1;
    mCellSize               = 0.0f;
    mBlendRegion            = 0.0f;
    mLightStrength          = 1.0f;
    mBounds.min             = cgVector3(0, 0, 0);
    mBounds.max             = cgVector3(0, 0, 0);
    mGridDimensions         = cgVector3(0, 0, 0);
    mGridSize               = cgVector3(0, 0, 0);
    mUnrolled               = true;
    mGridDirty              = false;
    mStaticDirty            = false;
    mDynamicDirty           = false;
    mCameraDelta            = cgVector3(0, 0, 0);
    mGridColor              = cgVector4(1, 1, 1, 1);
    mCascadeId              = nCascadeId;
    mDynamicUpdateFrames    = 15;
    mDynamicInitialized     = false;
    mMethod                 = pManager->getIndirectMethod();
}

//-----------------------------------------------------------------------------
//  Name : ~cgRadianceGrid ()
/// <summary>
/// Class destructor
/// </summary>
//-----------------------------------------------------------------------------
cgRadianceGrid::~cgRadianceGrid()
{
    mGridMesh.close();
    mGridConstants.close();

    if ( mStaticRadiance != CG_NULL )
        delete mStaticRadiance;
    mStaticRadiance = CG_NULL;

    if ( mDynamicRadiance0 != CG_NULL )
        delete mDynamicRadiance0;
    mDynamicRadiance0 = CG_NULL;

    if ( mDynamicRadiance1 != CG_NULL )
        delete mDynamicRadiance1;
    mDynamicRadiance1 = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initializes the grid
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::initialize( cgRenderDriver * pDriver, cgInt32 nMethod, const cgVector3 vDimensions, float fCellSize, cgUInt32 nPaddingCells, float fLightStrength, float fBlendRegion )
{	
    // Cache the driver
    mDriver = pDriver;

    // Copy initial values
    mMethod         = nMethod;
    mPaddingCells   = nPaddingCells;
    mCellSize       = fCellSize;
    mGridDimensions = vDimensions;
    mBlendRegion    = fBlendRegion;
    mGridSize       = mGridDimensions * mCellSize;
    mLightStrength  = fLightStrength;

    // Create the constant buffer for grid data
    cgResourceManager * pResources = pDriver->getResourceManager();
    const cgLightingSystemStates & States = mManager->mStates;
    if ( !pResources->createConstantBuffer( &mGridConstants, States.lightingShader, _T("_cbGrid"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create a required constant buffer during radiance grid initialization.\n") );
        return false;

    } // End if failed
    cgAssert( mGridConstants->getDesc().length == sizeof(_cbGrid) );

    // Vertex shader permutations for compositing passes.
    static const bool bNoStencilPass = false;
    mCompositeVSArgs.clear();
    mCompositeVSArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bNoStencilPass ) );

    static const bool bStencilPass = true;
    mStencilVSArgs.clear();
    mStencilVSArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bStencilPass ) );

    // Pixel shader permutations for compositing passes.
    mCompositePSArgs.clear();
    cgScriptObject * pSystemExports = pDriver->getShaderInterface();
    mCompositePSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("renderFlags") ) ) );
    mCompositePSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("shadingQuality") ) ) );

    // Pixel shader permutations for injection passes.
    mInjectionPSArgs.clear();
    mInjectionPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("shadowMethod") ) ) );
    mInjectionPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("primaryTaps") ) ) );
    mInjectionPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("secondaryTaps") ) ) );
    pSystemExports->release();

    // Create our radiance buffer(s)
    mStaticRadiance = new cgRadianceBuffer();
    if ( !mStaticRadiance->initialize( pDriver, mGridDimensions ) )
        return false;

    mDynamicRadiance0 = new cgRadianceBuffer();
    if ( !mDynamicRadiance0->initialize( pDriver, mGridDimensions ) )
        return false;

    mDynamicRadiance1 = new cgRadianceBuffer();
    if ( !mDynamicRadiance1->initialize( pDriver, mGridDimensions ) )
        return false;

    // Create a unit-size box (if it doesn't already exist) for grid drawing.
    if ( !pResources->getMesh( &mGridMesh, _T("Core::LightShapes::UnitBox") ) )
    {
        // Construct a new unit box mesh ( i.e., -1 to 1 on x,y,z) and add to resource manager
        cgMesh * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );
        pMesh->createBox( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), 2, 2, 2, 1, 1, 1, true, cgMeshCreateOrigin::Center, true, pResources );
        pResources->addMesh( &mGridMesh, pMesh, 0, _T("Core::LightShapes::UnitBox"), cgDebugSource() );

    } // End if no existing mesh

    // On the first frame, populate grids in full
    mMethod = mManager->getIndirectMethod();
    if ( mMethod == cgIndirectLightingMethod::PropagationVolumes )
    {
        // Only dynamic grids for lpv
        addTask( cgIndirectLightingTaskType::ReassignRSMs, 1, false );
        addTask( cgIndirectLightingTaskType::FillRSMs,     1, false );
        addTask( cgIndirectLightingTaskType::InjectRSMs,   1, false );
        addTask( cgIndirectLightingTaskType::Propagate,    1, false );

        // Setup the dynamic grid for amortized updates (push to next frame initially with identifier offset) 
        addTask( cgIndirectLightingTaskType::ReassignRSMs, 1 + mCascadeId + 1, false );
        addTask( cgIndirectLightingTaskType::FillRSMs,     1 + mCascadeId + 1, false );
        addTask( cgIndirectLightingTaskType::InjectRSMs,   1 + mCascadeId + 1, false );
        addTask( cgIndirectLightingTaskType::Propagate,    1 + mCascadeId + 1, false );
    }
    else
    {
        addTask( cgIndirectLightingTaskType::ReassignRSMs, 1, true );
        addTask( cgIndirectLightingTaskType::FillRSMs,     1, true );
        addTask( cgIndirectLightingTaskType::GatherRSMs,   1, true );

        addTask( cgIndirectLightingTaskType::ReassignRSMs, 1, false );
        addTask( cgIndirectLightingTaskType::FillRSMs,     1, false );
        addTask( cgIndirectLightingTaskType::GatherRSMs,   1, false );

        // Setup the dynamic grid for amortized updates (push to next frame initially with identifier offset) 
        addTask( cgIndirectLightingTaskType::ReassignRSMs, 1 + mCascadeId + 1, false );
        addTask( cgIndirectLightingTaskType::FillRSMs,     1 + mCascadeId + 1, false );
        addTask( cgIndirectLightingTaskType::GatherRSMs,   1 + mCascadeId + 1, false );
    }

    // Create a vertex buffer for grid drawing (cubic grids only for now)
    cgVertexFormat * pPointVertexFormat = cgVertexFormat::formatFromDeclarator( cgPointVertex::Declarator );
    cgUInt32 nStride  = (cgUInt32)pPointVertexFormat->getStride();
    cgUInt32 numCells = (cgUInt32)mGridDimensions.x;
    cgUInt32 vbLength = (numCells + 1) * (numCells + 1) * 6 * nStride;
    cgUInt32 rowWidth = (numCells + 1) * 6;
    if ( pResources->createVertexBuffer( &mGridLineVertexBuffer, vbLength, cgBufferUsage::WriteOnly, pPointVertexFormat, cgMemoryPool::Managed ) )
    {
        cgVertexBuffer *pVertexBuffer = (cgVertexBuffer*)mGridLineVertexBuffer.getResource();
        if ( pVertexBuffer != NULL )
        {
            // Create a temporary vertex array for loading
            cgPointVertex * pVertices = new cgPointVertex[ 6 * (numCells + 1) * (numCells + 1) ];
            for ( cgUInt32 i = 0; i <= numCells; i++ )
            {
                for ( cgUInt32 j = 0; j <= numCells; j++ )
                {
                    pVertices[ i * (rowWidth) + (j * 6) + 0 ].position = cgVector3( 0.0f, (float)i / (float)numCells, (float)j / (float)numCells );
                    pVertices[ i * (rowWidth) + (j * 6) + 1 ].position = cgVector3( 1.0f, (float)i / (float)numCells, (float)j / (float)numCells );
                    pVertices[ i * (rowWidth) + (j * 6) + 2 ].position = cgVector3( (float)i / (float)numCells, 0.0f, (float)j / (float)numCells );
                    pVertices[ i * (rowWidth) + (j * 6) + 3 ].position = cgVector3( (float)i / (float)numCells, 1.0f, (float)j / (float)numCells );
                    pVertices[ i * (rowWidth) + (j * 6) + 4 ].position = cgVector3( (float)i / (float)numCells, (float)j / (float)numCells, 0.0f );
                    pVertices[ i * (rowWidth) + (j * 6) + 5 ].position = cgVector3( (float)i / (float)numCells, (float)j / (float)numCells, 1.0f );
                }
            }

            // Upload our vertices to the buffer
            if ( !pVertexBuffer->updateBuffer( 0, vbLength, pVertices ) )
                cgAppLog::write( cgAppLog::Warning, _T("Lighting manager could not populate a grid line debug vertex buffer (%d vertices requested).\n"), (numCells+1) * (numCells+1) );

            // Cleanup temporary memory
            delete []pVertices;

        } // End valid buffer

    } // End buffer created

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getTasks ()
/// <summary>
/// Retrieve the list of tasks required in order to generate this radiance
/// grid.
/// </summary>
//-----------------------------------------------------------------------------
cgRadianceGrid::TaskList & cgRadianceGrid::getTasks( )
{
    return mTasks;
}

//-----------------------------------------------------------------------------
//  Name : getTasks () (const overload)
/// <summary>
/// Retrieve the list of tasks required in order to generate this radiance
/// grid.
/// </summary>
//-----------------------------------------------------------------------------
const cgRadianceGrid::TaskList & cgRadianceGrid::getTasks( ) const
{
    return mTasks;
}

//-----------------------------------------------------------------------------
//  Name : getLights ()
/// <summary>
/// Retrieve the list of static or dynamic light sources that are used to
/// populate this radiance grid.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectNodeArray & cgRadianceGrid::getLights( bool bStatic ) const
{
    return bStatic ? mStaticLights : mDynamicLights;
}

// ToDo: 6767 -- Make static member or check to see if is even used.
//-----------------------------------------------------------------------------
//  Name : ComputeGridExtents () (Utility Function)
/// <summary>
/// Computes the (component-wise) extents for the grid based on the camera's look
/// direction and the minimum number of buffer cells required behind the camera.
/// </summary>
//-----------------------------------------------------------------------------
void ComputeGridExtents( cgFloat & fMinOut, cgFloat & fMaxOut, cgFloat fPosition, cgFloat fDirection, cgFloat fGridSize, cgInt32 nTotalCells, cgInt32 nBufferCells )
{
    cgInt32 nHalfCells = nTotalCells / 2;
    cgInt32 nMax = nHalfCells - (cgInt32)(fDirection * (cgFloat)(nBufferCells - nHalfCells) ) ;
    nMax = min( nMax, nTotalCells - nBufferCells );
    nMax = max( nMax, nBufferCells );

    cgFloat fCellSize = fGridSize / (cgFloat)nTotalCells;
    fMaxOut = fPosition + ((cgFloat)nMax * fCellSize);
    fMinOut = fMaxOut - fGridSize;
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Updates the grid data based on the provided camera information.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::update( cgCameraNode * pCamera, float fClosestIntersectionDistance, float fSnapSize )
{
    // Get current camera position and look vector
    cgVector3 vCameraPosition = pCamera->getPosition(false);
    cgVector3 vCameraLook     = pCamera->getZAxis(false);

    // We may wish to snap to a another grid's spatial layout (helpful during cascading)
    if ( fSnapSize <= 0.0f ) fSnapSize = mCellSize;

    // Reset grid dirty flags
    mGridDirty    = false;
    mDynamicDirty = false;
    mStaticDirty  = false;

    // Cache current values
    cgBoundingBox prevAABB = mBounds;

    // Snap the camera position to the grid based on the cell size
    cgVector3 snappedPos;
    snappedPos.x = floor( vCameraPosition.x / fSnapSize ) * fSnapSize;
    snappedPos.y = floor( vCameraPosition.y / fSnapSize ) * fSnapSize;
    snappedPos.z = floor( vCameraPosition.z / fSnapSize ) * fSnapSize;

    ComputeGridExtents( mBounds.min.x, mBounds.max.x, snappedPos.x, vCameraLook.x, mGridSize.x, (cgInt32)mGridDimensions.x, mPaddingCells );
    ComputeGridExtents( mBounds.min.y, mBounds.max.y, snappedPos.y, vCameraLook.y, mGridSize.y, (cgInt32)mGridDimensions.y, mPaddingCells );
    ComputeGridExtents( mBounds.min.z, mBounds.max.z, snappedPos.z, vCameraLook.z, mGridSize.z, (cgInt32)mGridDimensions.z, mPaddingCells );

    // If there was a change in the grid location, we'll need to update
    mCameraDelta = mBounds.min - prevAABB.min;
    if ( fabs( mCameraDelta.x ) > 1e-5f || fabs( mCameraDelta.y ) > 1e-5f || fabs( mCameraDelta.z ) > 1e-5f )
    {
        // Compute number of cells to shift
        mCameraDelta.x /= (mGridSize.x / mGridDimensions.x);
        mCameraDelta.y /= (mGridSize.y / mGridDimensions.y);
        mCameraDelta.z /= (mGridSize.z / mGridDimensions.z);
        mGridDirty = true;
    }

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : affectsGrid ()
// Desc : Does the input light influence this grid?
//-----------------------------------------------------------------------------
bool cgRadianceGrid::affectsGrid( cgLightNode * pLight )
{
    // Are we using HDR lighting?
    bool bHDR = mDriver->getSystemState( cgSystemState::HDRLighting ) > 0;

    // ToDo: Figure out how this should be set...
    float m_fIntensityThrehsold = bHDR ? 0.01f : 0.001f;

    // If light is directional, automatically add it
    if( pLight->getLightType() == cgLightObject::Light_Directional )
    {
        return true;
    
    } // End if directional
    else
    {
        // For non-directionals, we will use a rough light falloff test to determine eligbility
        cgVector3 vLightPosition = pLight->getPosition(false);

        // If light is inside the grid, automatically add it
        if ( mBounds.containsPoint( vLightPosition ) )
        {
            return true;
        }
        else
        {
            cgVector3 vClosestPoint = mBounds.closestPoint( vLightPosition );
            cgVector3 vRay = vClosestPoint - vLightPosition;
            cgFloat fDistance = cgVector3::length( vRay );

            // We will assume that the closest object affected by the light 
            // is at the end of the light range to be overly conservative
            cgFloat fRange = 0;
            switch( pLight->getLightType() )
            {
                case cgLightObject::Light_Point:
                    fRange = ((cgPointLightNode*)pLight)->getOuterRange();
                    break;
                case cgLightObject::Light_Spot:
                    fRange = ((cgSpotLightNode*)pLight)->getOuterRange();
                    break;
                case cgLightObject::Light_Projector:
                    fRange = ((cgProjectorLightNode*)pLight)->getOuterRange();
                    break;
            
            } // End switch type

            // If the light's range exceeds the distance to grid, add the light
            if ( fDistance < fRange )
            {
                return true;
            
            } // End if within range
            else
            {
                // Get the maximum diffuse spectral intensity
                cgColorValue cDiffuse = pLight->getDiffuseColor();
                float fIntensity = max( cDiffuse.r, cDiffuse.g );
                fIntensity = max( fIntensity, cDiffuse.b );

                // Adjust for HDR
                if ( bHDR )
                    fIntensity *= pLight->getDiffuseHDRScale();

                // Adjust for overall ambient intensity (ToDo: Create dedicated factor for this)
                fIntensity *= pLight->getAmbientFarHDRScale();

                // Adjust for distance attenuation
                fIntensity /= (fDistance * fDistance);

                // Compare against threshold 
                // Note: The threshold can try to accomodate average reflectance
                //       and lambertian terms that cannot be accounted for here.
                //       However a conservative approach is preferred to prevent
                //       temporal artifacts if a light is erroneously skipped.
                if ( fIntensity > m_fIntensityThrehsold )
                    return true;

            } // End if outside range

        } // End point not contained

    } // End if non-directional light

    // No influence
    return false;
}

//-----------------------------------------------------------------------------
// Name : addLights ()
// Desc : Builds the list of indirect lights to be added into our radiance grids
//-----------------------------------------------------------------------------
void cgRadianceGrid::addLights( const cgObjectNodeArray & aStaticLights, const cgObjectNodeArray & aDynamicLights )
{
    // Get the current frame number
    cgUInt32 nCurrentFrame = cgTimer::getInstance()->getFrameCounter();

    // Create lookup tables for the current static and dynamic lights
    cgObjectNodeSet aPreviousStaticLights, aPreviousDynamicLights;
    for ( size_t i = 0; i < mStaticLights.size(); ++i )
        aPreviousStaticLights.insert( mStaticLights[ i ] );
    for ( size_t i = 0; i < mDynamicLights.size(); ++i )
        aPreviousDynamicLights.insert( mDynamicLights[ i ] );

    // Clear the previous lists
    mStaticLights.clear();
    mDynamicLights.clear();

    // Keep track of how many lights match from the last time
    cgUInt32 nStaticFailed  = 0;
    cgUInt32 nDynamicFailed = 0;

    // Add static lights
    for ( size_t i = 0; i < aStaticLights.size(); ++i )
    {
        if ( affectsGrid( (cgLightNode*)aStaticLights[ i ] ) )
        {
            mStaticLights.push_back( aStaticLights[ i ] );
            if ( aPreviousStaticLights.find( aStaticLights[ i ] ) == aPreviousStaticLights.end() )
                ++nStaticFailed;
        
        } // End if affects
    
    } // Next light

    // Add dynamic lights
    for ( size_t i = 0; i < aDynamicLights.size(); ++i )
    {
        if ( affectsGrid( (cgLightNode*)aDynamicLights[ i ] ) )
        {
            mDynamicLights.push_back( aDynamicLights[ i ] );
            if ( aPreviousDynamicLights.find( aDynamicLights[ i ] ) != aPreviousDynamicLights.end() )
                ++nDynamicFailed;
        
        } // End if affects
    
    } // Next light

    // If using LPV
    if ( mMethod == cgIndirectLightingMethod::PropagationVolumes )
    {
        // All lights will be considered dynamic
        for ( size_t i = 0; i < mStaticLights.size(); ++i )
            mDynamicLights.push_back( mStaticLights[ i ] );
        mStaticLights.clear();

    } // End LPV
    else
    {
        // If we didn't get back exactly the same static lights, we'll need a full grid update
        mStaticDirty = nStaticFailed > 0 || (mStaticLights.size() != aPreviousStaticLights.size());
        if ( mStaticDirty )
        {
            addTask( cgIndirectLightingTaskType::ReassignRSMs, nCurrentFrame, true );
            addTask( cgIndirectLightingTaskType::FillRSMs,     nCurrentFrame, true );
            addTask( cgIndirectLightingTaskType::GatherRSMs,   nCurrentFrame, true );
        
        } // End if dirty

        // If the grid has moved and we are beyond frame 1 (which is automatic)
        // do reprojection (assuming a full update is not already needed)
        if ( mGridDirty && nCurrentFrame > 1 )
        {
            if ( !mStaticLights.empty() && !mStaticDirty )
            {
                addTask( cgIndirectLightingTaskType::ReassignRSMs,  nCurrentFrame, true );
                addTask( cgIndirectLightingTaskType::FillRSMs,      nCurrentFrame, true );
                addTask( cgIndirectLightingTaskType::ReprojectGrid, nCurrentFrame, true );
            
            } // End if static !dirty

            if ( !mDynamicLights.empty() )
            {
                addTask( cgIndirectLightingTaskType::ReassignRSMs,  nCurrentFrame, false );
                addTask( cgIndirectLightingTaskType::FillRSMs,      nCurrentFrame, false );
                addTask( cgIndirectLightingTaskType::ReprojectGrid, nCurrentFrame, false );
            
            } // End if process dynamics
        
        } // End if dirty and > frame 1

    } // End radiance hints

}

//-----------------------------------------------------------------------------
//  Name : processTask ()
/// <summary>
/// Builds a list of items that need to be done to update indirect lighting
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::processTask( const cgIndirectLightingTask & Task )
{
    // Which light list are we processing?
    const cgObjectNodeArray & Lights = Task.staticUpdate ? mStaticLights : mDynamicLights;

    // Are we using propagation volumes?
    bool bLPV = mMethod == cgIndirectLightingMethod::PropagationVolumes;

    // Fill grids with RSM data (light, occlusion, etc.)
    if ( ( Task.type == cgIndirectLightingTaskType::GatherRSMs ) || ( Task.type == cgIndirectLightingTaskType::InjectRSMs ) )
    {
        // Do we have light sources to process?
        if ( !Lights.empty() )
        {
            // Is this a dynamic grid update?
            if ( !Task.staticUpdate )
                mDynamicDirty = true;

            // gather the RSMs
            if ( bLPV )
                inject( Task.staticUpdate );
            else
                gather( Task.staticUpdate );
        
        } // End if has lights

        // If this was a dynamic grid injection, add another refresh for N frames from now
        if ( !Task.staticUpdate )
        {
            // If we've already done our first pass initialization, we can start doing frame management 
            if ( mDynamicInitialized )
            {
                // To reduce popping, for the first N frames, update the dynamic grid frequently, 
                // then switch to the desired rate.
                cgUInt32 nRate = mDynamicUpdateFrames;
                if ( cgTimer::getInstance()->getFrameCounter() < 30 )
                    nRate = 1;

                addTask( cgIndirectLightingTaskType::ReassignRSMs, Task.frame + nRate - 1, false );
                addTask( cgIndirectLightingTaskType::FillRSMs,     Task.frame + nRate - 1, false );
                if ( bLPV )
                {
                    addTask( cgIndirectLightingTaskType::InjectRSMs, Task.frame + nRate, false );
                    addTask( cgIndirectLightingTaskType::Propagate,  Task.frame + nRate, false );
                
                } // End if LPV
                else
                {
                    addTask( cgIndirectLightingTaskType::GatherRSMs, Task.frame + nRate, false );
                
                } // End if !LPV
            
            } // End if initialized

            // At this point, the dynamic grid is initialized
            mDynamicInitialized = true;
        
        } // End if dynamics

    } // End if injecting RSMs

    // Fill grids with reprojected data
    if ( Task.type == cgIndirectLightingTaskType::ReprojectGrid )
        gather( Task.staticUpdate );

    // Fill grids with reprojected data
    if ( Task.type == cgIndirectLightingTaskType::Propagate )
        propagate( Task.staticUpdate );

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : gather ()
/// <summary>
/// Gathers RSM lighting into grids 
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::gather( bool bStatic )
{
    // Get the integer grid extents and camera deltas
    int sx = int( mGridDimensions.x );
    int sy = int( mGridDimensions.y );
    int sz = int( mGridDimensions.z );
    int dx = int( mCameraDelta.x );
    int dy = int( mCameraDelta.y );
    int dz = int( mCameraDelta.z );

    // Compute extents for reprojection
    cgRect copyRect;
    copyRect.left   = 0;
    copyRect.top    = 0;
    copyRect.right  = sx;
    copyRect.bottom = sy;
    if ( dx > 0 )
        copyRect.right -= dx;
    else if ( dx < 0 )
        copyRect.left  += abs( dx );

    if ( dy > 0 )
        copyRect.top += dy;
    else if ( dy < 0 )
        copyRect.bottom -= abs( dy );

    int maxZ = dz > 0 ? sz - dz : sz;
    int minZ = dz < 0 ? abs(dz) : 0;

    // If we need to update the full width/height, no reprojection is needed
    bool bUpdateAll = (bStatic && mStaticDirty) || (!bStatic && mDynamicDirty);
    if ( (abs( dx ) >= sx) || (abs( dy ) >= sy) )
        bUpdateAll = true;

    // On the first frame, force a full lighting update (i.e., no copying)
    if ( cgTimer::getInstance()->getFrameCounter() == 1 )
        bUpdateAll = true;

    // Setup constant buffer
    cgConstantBuffer * pGridBuffer = mGridConstants.getResource( true );
    _cbGrid * pGridData = (_cbGrid*)pGridBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
    pGridData->gridMin            = mBounds.min;
    pGridData->gridMax            = mBounds.max;
    pGridData->gridDimensions     = mGridDimensions;
    pGridData->gridShift          = mCameraDelta; 
    pGridData->gridShift.y       *= -1.0f; // Y axis shift reversed as texture directions are reversed
    pGridData->gridColor          = mGridColor;
    pGridData->gridColor.w        = mLightStrength;

    // Set previous grid constants
    if ( mCascadeId == 0 )
    {
        pGridData->gridMinPrev        = cgVector3(0,0,0);
        pGridData->gridMaxPrev        = cgVector3(0,0,0);
        pGridData->gridDimensionsPrev = cgVector3(0,0,0);
        pGridData->blendRegionPrev    = 0.0f;
    }
    else
    {
        cgRadianceGrid * pSibling     = mManager->getRadianceGrid( mCascadeId - 1 );
        pGridData->gridMinPrev        = pSibling->mBounds.min;
        pGridData->gridMaxPrev        = pSibling->mBounds.max;
        pGridData->gridDimensionsPrev = pSibling->mGridDimensions;
        pGridData->blendRegionPrev    = pSibling->mBlendRegion;
    }

    // Unlock the buffer. 
    pGridBuffer->unlock();

    // Apply the constant buffer during the next draw
    mDriver->setConstantBufferAuto( mGridConstants );

    // If we are updating the whole grid...
    if ( bUpdateAll )
    {
        minZ = 0;
        maxZ = sz;
        memset( &copyRect, 0, sizeof( cgRect ) );
    }

    // Run injection
    if ( bStatic )
    {
        if ( !gatherLightsStatic( bUpdateAll, copyRect, minZ, maxZ ) )
            return false;

        mStaticDirty = false;
    }
    else
    {
        if ( !gatherLightsDynamic( bUpdateAll, copyRect, minZ, maxZ ) )
            return false;

        mDynamicDirty = false;
    }

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : gatherLightsStatic ()
/// <summary>
/// Gathers light from RSM samples for the static grid
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::gatherLightsStatic( bool bUpdateAll, const cgRect & rcCopy, cgInt32 nMinZ, cgInt32 nMaxZ )
{
    // Get a scratch buffer from the manager
    cgRadianceBuffer * pDestBuffer = mManager->getScratchBuffer( mGridDimensions );
    cgAssert( pDestBuffer != CG_NULL );
    cgRenderTargetHandleArray aTargets;
    aTargets.push_back( pDestBuffer->mTargets[ 0 ] );
    aTargets.push_back( pDestBuffer->mTargets[ 1 ] );
    aTargets.push_back( pDestBuffer->mTargets[ 2 ] );

    // Set the radiance buffer as render target
    if ( mDriver->beginTargetRender( aTargets ) )
    {
        // Clear the buffer
        mDriver->clear( cgClearFlags::Target, 0, 0, 0 );

        // If we are not doing a full update, reproject instead of recomputing lighting (where possible)
        if ( !bUpdateAll )
            reprojectGrid( CG_NULL, mStaticRadiance, rcCopy, nMinZ, nMaxZ );

        // Gather light
        gatherLights( CG_NULL, mStaticLights, bUpdateAll, rcCopy, nMinZ, nMaxZ );

        // Done rendering
        mDriver->endTargetRender( );

    } // End begin rendering

    // Exchange textures with manager (we are swapping ownership of the pointers here)
    cgRadianceBuffer * pPrevBuffer = mStaticRadiance;
    mStaticRadiance = pDestBuffer;
    mManager->replaceScratchBuffer( pDestBuffer, pPrevBuffer );

    // Update box and time information for this buffer
    mStaticRadiance->mBounds  = mBounds;
    mStaticRadiance->mTime = (cgFloat)cgTimer::getInstance()->getTime();

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : gatherLightsDynamic ()
/// <summary>
/// Gathers light from RSM samples for the dynamic grid
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::gatherLightsDynamic( bool bUpdateAll, const cgRect & rcCopy, cgInt32 nMinZ, cgInt32 nMaxZ )
{
    // Get a scratch buffer from the manager
    cgRadianceBuffer * pDestBuffer = mManager->getScratchBuffer( mGridDimensions );
    cgAssert( pDestBuffer != CG_NULL );
    cgRenderTargetHandleArray aTargets;
    aTargets.push_back( pDestBuffer->mTargets[ 0 ] );
    aTargets.push_back( pDestBuffer->mTargets[ 1 ] );
    aTargets.push_back( pDestBuffer->mTargets[ 2 ] );

    // Set the radiance buffer as render target
    if ( mDriver->beginTargetRender( aTargets ) )
    {
        // Clear the targets
        mDriver->clear( cgClearFlags::Target, 0, 0, 0 );

        // If we are not doing a full update, reproject instead of recomputing lighting (where possible)
        if ( !bUpdateAll )
            reprojectGrid( CG_NULL, mDynamicRadiance0, rcCopy, nMinZ, nMaxZ );

        // Gather light
        gatherLights( CG_NULL, mDynamicLights, bUpdateAll, rcCopy, nMinZ, nMaxZ );

        // Done rendering
        mDriver->endTargetRender( );

    } // End begin rendering

    // Exchange textures with manager (we are swapping ownership of the pointers here)
    cgRadianceBuffer * pPrevBuffer = mDynamicRadiance1;
    mDynamicRadiance1 = pDestBuffer;
    mManager->replaceScratchBuffer( pDestBuffer, pPrevBuffer );

    // Update box and time information for this buffer
    mDynamicRadiance1->mBounds  = mBounds;
    mDynamicRadiance1->mTime = (cgFloat)cgTimer::getInstance()->getTime();

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : reprojectGrid()
/// <summary>
/// Reprojects samples in the grid that can be reused (as camera moves)
/// Note: Assumes render targets already setup
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::reprojectGrid( cgRadianceBuffer *pDestBuffer, cgRadianceBuffer *pSrcBuffer, const cgRect & rcCopy, cgInt32 nMinZ, cgInt32 nMaxZ )
{
    // Get access to lighting states
    cgLightingSystemStates & States = mManager->mStates;

    // Get the surface shader
    cgSurfaceShader * pShader = States.lightingShader.getResource(true);

    // Select the grid copying shader
    if ( !pShader->selectPixelShader( _T("copyGrid") ) )
        return false;

    // Set rendering states
    mDriver->setDepthStencilState( States.disabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setBlendState( cgBlendStateHandle::Null );

    // Bind the current radiance buffers as input
    States.harmonicsRedPointSampler->apply( pSrcBuffer->mTargets[ 0 ] );
    States.harmonicsGreenPointSampler->apply( pSrcBuffer->mTargets[ 1 ] );
    States.harmonicsBluePointSampler->apply( pSrcBuffer->mTargets[ 2 ] );

    // Initialize a viewport
    cgViewport Viewport;
    Viewport.x      = 0;
    Viewport.y      = 0;
    Viewport.width  = (cgInt32)mGridDimensions.x;
    Viewport.height = (cgInt32)mGridDimensions.y;
    Viewport.minimumZ   = 0.0f;
    Viewport.maximumZ   = 1.0f;

    // If a destination buffer was provided, begin rendering to it
    if ( pDestBuffer )
    {
        cgRenderTargetHandleArray aTargets;
        aTargets.push_back( pDestBuffer->mTargets[ 0 ] );
        aTargets.push_back( pDestBuffer->mTargets[ 1 ] );
        aTargets.push_back( pDestBuffer->mTargets[ 2 ] );
        if ( !mDriver->beginTargetRender( aTargets ) )
            return false;
    
    } // End if has dest buffer

    // Run the copy loop before we process lights
    for ( cgInt32 i = nMinZ; i < nMaxZ; i++ )
    {
        // Set the viewport according to our copying rectangle
        Viewport.x      = (i * (cgInt32)mGridDimensions.x) + rcCopy.left;
        Viewport.y      = rcCopy.top;
        Viewport.width  = rcCopy.right  - rcCopy.left;
        Viewport.height = rcCopy.bottom - rcCopy.top;

        // Set the viewport and draw
        mDriver->setViewport( &Viewport );
        mDriver->drawScreenQuad( );

    } // Next slice

    // If a destination buffer was provided, end rendering to it
    if ( pDestBuffer )
        mDriver->endTargetRender();

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : gatherLights()
/// <summary>
/// Gathers lighting from RSM samples for the current grid. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::gatherLights( cgRadianceBuffer *pDestBuffer, cgObjectNodeArray & aLights, bool bUpdateAll, const cgRect & rcCopy, cgInt32 nMinZ, cgInt32 nMaxZ )
{
    /*
    cgInt32 i, j, nPass, nNumPasses = 0;
    cgLightNode * pLight;
    cgLightNode::LightingOp Op;
    cgVisibilitySet * pRenderSet;
    _cbGrid * pGridData = CG_NULL;*/

    // Get access to required systems
    cgTexturePool  * pPool = mManager->getShadowMaps();
    cgScriptObject * pScript = mManager->mScriptObject;
    const cgString & strRSMCallback = mManager->mIndirectLightCallback;

    // Get the integer grid extents and camera deltas
    cgInt32 sx = (cgInt32)mGridDimensions.x;
    cgInt32 sy = (cgInt32)mGridDimensions.y;
    cgInt32 sz = (cgInt32)mGridDimensions.z;

    // Initialize a viewport
    cgViewport Viewport;
    Viewport.x      = 0;
    Viewport.y      = 0;
    Viewport.width  = sx;
    Viewport.height = sy;
    Viewport.minimumZ   = 0.0f;
    Viewport.maximumZ   = 1.0f;

    // Get access to lighting states
    cgLightingSystemStates & States = mManager->mStates;

    // Get the surface shader
    cgSurfaceShader * pShader = States.lightingShader.getResource(true);

    // Set rendering states
    mDriver->setDepthStencilState( States.disabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setBlendState( States.additiveBlendState );

    // If a destination buffer was provided, begin rendering to it
    if ( pDestBuffer )
    {
        cgRenderTargetHandleArray aTargets;
        aTargets.push_back( pDestBuffer->mTargets[ 0 ] );
        aTargets.push_back( pDestBuffer->mTargets[ 1 ] );
        aTargets.push_back( pDestBuffer->mTargets[ 2 ] );
        if ( !mDriver->beginTargetRender( aTargets ) )
            return false;
    
    } // End if has dest

    // For each light (frustum/rsm) we wish to process this frame
    for ( size_t j = 0; j < aLights.size(); ++j )
    {
        // Get the current light
        cgLightNode * pLight = (cgLightNode*)aLights[ j ];

        // Begin indirect lighting 
        cgInt32 nNumPasses = pLight->beginIndirectLighting( pPool );

        // For each pass (i.e., RSM)
        for ( cgInt32 nPass = 0; nPass < nNumPasses; ++nPass )
        {
            // Begin the pass (Either prepares for RSM fill or sets up the RSM inputs.)
            cgVisibilitySet * pRenderSet;
            cgLightNode::LightingOp Op = pLight->beginIndirectLightingPass( nPass, pRenderSet );

            // What operation is taking place?
            if ( Op == cgLightNode::Lighting_FillShadowMap )
            {
                // We need to fill a default resource, do so...
                // Notify the script that it should render.
                try
                {
                    cgScriptArgument::Array ScriptArgs;
                    static const cgString strContext = _T("FillReflectiveShadowMap");
                    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"),  &strContext ) );
                    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("LightNode@+"),     pLight ) );
                    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("VisibilitySet@+"), pRenderSet ) );
                    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord,   _T("uint"),           &nPass ) );

                    // Execute the specified script function.
                    pScript->executeMethodVoid( strRSMCallback, ScriptArgs );

                } // End try to execute
                catch ( cgScriptInterop::Exceptions::ExecuteException & e )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strRSMCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );

                } // End catch exception					

            } // End if FillReflectiveShadowMap
            else if ( Op == cgLightNode::Lighting_ProcessLight )
            {
                // Otherwise, inject into our volume
                // Set shaders
                if ( !pShader->selectPixelShader( _T("drawRSM"), mInjectionPSArgs ) )
                    continue;

                // For each volume slice 
                for ( cgInt32 i = 0; i < sz; ++i )
                {
                    // Reset the viewport to cover the current slice
                    Viewport.x      = i * sx;
                    Viewport.y      = 0;
                    Viewport.width  = sx;
                    Viewport.height = sy;

                    // If this slice requires a full update...
                    if ( bUpdateAll || (i < nMinZ) || (i >= nMaxZ) )
                    {
                        // Set the full viewport and draw
                        mDriver->setViewport( &Viewport );
                        mDriver->drawScreenQuad( );
                    
                    } // End if full update
                    else if ( int( mCameraDelta.x ) == 0 && int( mCameraDelta.y ) == 0 )
                    {
                        // Slice requires no update at all (already copied during reprojection)
                        continue;
                    
                    } // End already copied
                    else
                    {
                        // Slice requires x and/or y partial update
                        // Process top (+Y motion)
                        if ( rcCopy.top > 0 )
                        {
                            Viewport.y      = 0;
                            Viewport.height = rcCopy.top;
                            mDriver->setViewport( &Viewport );
                            mDriver->drawScreenQuad( );
                        }

                        // Process left (-X motion)
                        if ( rcCopy.left > 0 )
                        {
                            Viewport.x      = (i * sx) + 0;
                            Viewport.y      = rcCopy.top;
                            Viewport.width  = rcCopy.left;
                            Viewport.height = rcCopy.bottom - rcCopy.top;
                            mDriver->setViewport( &Viewport );
                            mDriver->drawScreenQuad( );
                        }

                        // Process right (+X motion)
                        if ( rcCopy.right < sx )
                        {
                            Viewport.x      = (i * sx) + rcCopy.right;
                            Viewport.y      = rcCopy.top;
                            Viewport.width  = sx - rcCopy.right;
                            Viewport.height = rcCopy.bottom - rcCopy.top;
                            mDriver->setViewport( &Viewport );
                            mDriver->drawScreenQuad( );
                        }

                        // Process bottom (-Y motion)
                        if ( rcCopy.bottom < sy )
                        {
                            Viewport.x      = 0;
                            Viewport.y      = rcCopy.bottom;
                            Viewport.width  = sx;
                            Viewport.height = sy;
                            mDriver->setViewport( &Viewport );
                            mDriver->drawScreenQuad( );
                        }

                    } // End if partial update

                } // Next slice

            } // End if ProcessLight

            // End pass
            pLight->endIndirectLightingPass( );

        } // Next pass

        // End indirect
        pLight->endIndirectLighting( );

    } // Next light

    // If a destination buffer was provided, end rendering to it
    if ( pDestBuffer )
        mDriver->endTargetRender();

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : composite ()
/// <summary>
/// Applies grid's radiance to scene
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::composite( bool bLowRes )
{
    // What lights do we have to work with?
    bool bStaticLights  = !mStaticLights.empty();
    bool bDynamicLights = !mDynamicLights.empty();
    if ( !bStaticLights && !bDynamicLights )
        return true;

    // Get access to lighting states
    cgLightingSystemStates & States = mManager->mStates;

    // Get the surface shader
    cgSurfaceShader * pShader = States.lightingShader.getResource(true);

    // If we are using a dynamic grid, interpolate
    if ( bDynamicLights )
        interpolateDynamicGrids( );

    // Update grid constant buffer
    cgConstantBuffer * pGridBuffer = mGridConstants.getResource( true );
    cgAssert( pGridBuffer != CG_NULL );
    _cbGrid * pGridData = (_cbGrid*)pGridBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
    cgAssert( pGridData != CG_NULL );

    pGridData->gridMin            = mBounds.min;
    pGridData->gridMax            = mBounds.max;
    pGridData->gridDimensions     = mGridDimensions;
    pGridData->gridShift          = mCameraDelta; 
    pGridData->blendRegion        = mBlendRegion;
    pGridData->gridColor          = mGridColor;
    pGridData->gridColor.w        = mLightStrength;

    // Set previous grid constants
    if ( mCascadeId == 0 )
    {
        pGridData->gridMinPrev        = cgVector3(0,0,0);
        pGridData->gridMaxPrev        = cgVector3(0,0,0);
        pGridData->gridDimensionsPrev = cgVector3(0,0,0);
        pGridData->blendRegionPrev    = 0.0f;
    }
    else
    {
        cgRadianceGrid * pSibling     = mManager->getRadianceGrid( mCascadeId - 1 );
        pGridData->gridMinPrev        = pSibling->mBounds.min;
        pGridData->gridMaxPrev        = pSibling->mBounds.max;
        pGridData->gridDimensionsPrev = pSibling->mGridDimensions;
        pGridData->blendRegionPrev    = pSibling->mBlendRegion;
    }

    pGridBuffer->unlock();
    mDriver->setConstantBufferAuto( mGridConstants );

    // Setup radiance buffer based on what sort of lights are contributing
    cgRadianceBuffer * pRadianceBuffer = CG_NULL;

    // If we have static and dynamic buffers, we need to combine them
    if ( bStaticLights && bDynamicLights )
    {
        // Get a new buffer from the manager
        pRadianceBuffer = mManager->getScratchBuffer( mGridDimensions );

        cgAssert( pRadianceBuffer != CG_NULL );
        cgRenderTargetHandleArray aTargets;
        aTargets.push_back( pRadianceBuffer->mTargets[ 0 ] );
        aTargets.push_back( pRadianceBuffer->mTargets[ 1 ] );
        aTargets.push_back( pRadianceBuffer->mTargets[ 2 ] );

        // Set pixel shader 
        if ( !pShader->selectPixelShader( _T("readGrid") ) )
            return false;

        // Set states
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
        mDriver->setDepthStencilState( States.disabledDepthState, 0 );

        if ( mDriver->beginTargetRender( aTargets ) )
        {
            // Copy static light
            States.harmonicsRedSampler->apply( mStaticRadiance->mTargets[ 0 ] );
            States.harmonicsGreenSampler->apply( mStaticRadiance->mTargets[ 1 ] );
            States.harmonicsBlueSampler->apply( mStaticRadiance->mTargets[ 2 ] );
            mDriver->setBlendState( cgBlendStateHandle::Null );
            mDriver->drawScreenQuad();	

            // Add dynamic light
            States.harmonicsRedSampler->apply( mDynamicRadiance0->mTargets[ 0 ] );
            States.harmonicsGreenSampler->apply( mDynamicRadiance0->mTargets[ 1 ] );
            States.harmonicsBlueSampler->apply( mDynamicRadiance0->mTargets[ 2 ] );
            mDriver->setBlendState( States.additiveBlendState );
            mDriver->drawScreenQuad();	

            // End rendering
            mDriver->endTargetRender();

        } // End render

    } // End if both
    // Static only
    else if ( bStaticLights )
    {
        pRadianceBuffer = mStaticRadiance;
    }
    // Dynamic only
    else 
    {
        pRadianceBuffer = mDynamicRadiance0;
    }

    // Bind the radiance textures for input
    States.harmonicsRedSampler->apply( pRadianceBuffer->mTargets[ 0 ] );
    States.harmonicsGreenSampler->apply( pRadianceBuffer->mTargets[ 1 ] );
    States.harmonicsBlueSampler->apply( pRadianceBuffer->mTargets[ 2 ] );

    // Build a new world matrix for this grid 
    cgMatrix mtxWorld;
    cgMatrix::identity( mtxWorld );
    cgVector3 vCenter = mBounds.getCenter();
    mtxWorld._11 = mGridSize.x * 0.5f;
    mtxWorld._22 = mGridSize.y * 0.5f;
    mtxWorld._33 = mGridSize.z * 0.5f;
    mtxWorld._41 = vCenter.x;
    mtxWorld._42 = vCenter.y;
    mtxWorld._43 = vCenter.z;

    // Draw the grid box
    cgMesh * pMesh = mGridMesh.getResource( true );
    if ( pMesh && pMesh->isLoaded() )
    {
        // Setup our shaders
        if ( !pShader->selectVertexShader( _T("transformGrid"), mCompositeVSArgs ) )
            return false;

        if ( bLowRes )
        {
            if ( !pShader->selectPixelShader( _T("computeIrradiance"), mCompositePSArgs ) )
                return false;
        }
        else
        {
            if ( !pShader->selectPixelShader( _T("compositeRadiance"), mCompositePSArgs ) )
                return false;
        }

        // Set rendering states
        mDriver->setDepthStencilState( States.testStencilDepthState, 1 );
        mDriver->setBlendState( States.additiveBlendState );
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

        mDriver->setWorldTransform( &mtxWorld );
        pMesh->drawSubset( 0, cgMeshDrawMode::Simple );

        // If blending, draw the smaller box to mask the stencil buffer
        if ( mBlendRegion > 0.0f )
        {
            if ( !pShader->selectVertexShader( _T("transformGrid"), mStencilVSArgs ) )
                return false;
            if ( !pShader->selectPixelShader( _T("nullOutput") ) )
                return false;

            mDriver->setDepthStencilState( States.stencilClearState, 0 );
            mDriver->setBlendState( States.disabledBlendState );
            mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

            cgMatrix mtxInnerBox = mtxWorld;
            mtxInnerBox._11 *= (1.0f - mBlendRegion);
            mtxInnerBox._22 *= (1.0f - mBlendRegion);
            mtxInnerBox._33 *= (1.0f - mBlendRegion);
            mDriver->setWorldTransform( &mtxInnerBox );
            pMesh->drawSubset( 0, cgMeshDrawMode::Simple );
        }
    }

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : interpolateDynamicGrids ()
/// <summary>
/// Applies grid's radiance to scene
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::interpolateDynamicGrids( )
{
    _cbGrid * pGridData;

    // If we haven't completed initial updates yet, just swap buffers
    if ( mDynamicRadiance0->mTime == 0.0f )
    {
        cgRadianceBuffer * pTmp = mDynamicRadiance0;
        mDynamicRadiance0 = mDynamicRadiance1;
        mDynamicRadiance0->mBounds  = mDynamicRadiance1->mBounds;
        mDynamicRadiance0->mTime = mDynamicRadiance1->mTime;
        mDynamicRadiance1 = pTmp;
        return true;
    }

    // Get access to required systems
    cgLightingSystemStates & States = mManager->mStates;
    cgSurfaceShader  * pShader      = States.lightingShader.getResource(true);
    cgConstantBuffer * pGridBuffer  = mGridConstants.getResource(true);

    // Select the pixel shader
    cgUInt32 nType = 2; //2 = test box
    cgScriptArgument::Array psArgs( 1 );
    psArgs[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nType );
    if ( !pShader->selectPixelShader( _T("reprojectGrid"), psArgs ) )
        return false;

    // Bind the constant buffer
    mDriver->setConstantBufferAuto( mGridConstants );

    // Set states
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setDepthStencilState( States.disabledDepthState );

    // Get a new buffer from the manager
    cgRadianceBuffer * pRadianceBuffer = mManager->getScratchBuffer( mGridDimensions );
    cgRenderTargetHandleArray aTargets;
    aTargets.push_back( pRadianceBuffer->mTargets[ 0 ] );
    aTargets.push_back( pRadianceBuffer->mTargets[ 1 ] );
    aTargets.push_back( pRadianceBuffer->mTargets[ 2 ] );
    if ( mDriver->beginTargetRender( aTargets ) )
    {
        // Compute the blend term
        cgFloat fDeltaT = mDynamicRadiance1->mTime - mDynamicRadiance0->mTime;
        cgFloat fLerpValue = 0.1f;

        // Populate the constant buffer with some initial data we'll need 
        pGridData                  = (_cbGrid*)pGridBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
        pGridData->gridMin         = mBounds.min;
        pGridData->gridMax         = mBounds.max;
        pGridData->gridMinPrev     = mBounds.min;
        pGridData->gridMaxPrev     = mBounds.max;
        pGridData->gridDimensions  = mGridDimensions;
        pGridData->gridShift       = mCameraDelta;
        pGridData->gridShift.y    *= -1.0f;
        pGridData->blendRegion     =  fLerpValue;
        pGridData->blendRegionPrev =  1.0f;
        pGridBuffer->unlock();

        // Place current results (replace existing values)
        // If we cannot find a valid location in the previous AABB,
        // we fill the cell with the full current value (i.e., blend term = t if passed or 1.0f if failed )
        States.harmonicsRedSampler->apply( mDynamicRadiance1->mTargets[ 0 ] );
        States.harmonicsGreenSampler->apply( mDynamicRadiance1->mTargets[ 1 ] );
        States.harmonicsBlueSampler->apply( mDynamicRadiance1->mTargets[ 2 ] );
        mDriver->setBlendState( cgBlendStateHandle::Null );
        mDriver->drawScreenQuad();	

        // Mix in previous lighting results
        // If we cannot find a valid location in the previous AABB,
        // we leave the cell value alone (i.e., blend term = t if passed or 0.0f if failed )
        pGridData                  = (_cbGrid*)pGridBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
        pGridData->blendRegion     =  1.0f - fLerpValue;
        pGridData->blendRegionPrev =  0.0f;
        pGridBuffer->unlock();

        States.harmonicsRedSampler->apply( mDynamicRadiance0->mTargets[ 0 ] );
        States.harmonicsGreenSampler->apply( mDynamicRadiance0->mTargets[ 1 ] );
        States.harmonicsBlueSampler->apply( mDynamicRadiance0->mTargets[ 2 ] );
        mDriver->setBlendState( States.additiveBlendState );
        mDriver->drawScreenQuad();	

        // End rendering
        mDriver->endTargetRender();
    }

    // Preserve the time and box of our "previous" grid
    pRadianceBuffer->mTime = mDynamicRadiance0->mTime;
    pRadianceBuffer->mBounds  = mDynamicRadiance0->mBounds;

    // The new combined results are now the new "previous" grid, so swap with manager
    cgRadianceBuffer * pPrevBuffer = mDynamicRadiance0;
    mDynamicRadiance0 = pRadianceBuffer;
    mManager->replaceScratchBuffer( pRadianceBuffer, pPrevBuffer );

    // Success
    return true;
}


//-----------------------------------------------------------------------------
//  Name : taskExists ()
/// <summary>
/// Checks to see if a task already exists in the list
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::taskExists( cgIndirectLightingTaskType::Base TaskType, cgInt32 nFrame, bool bStatic, cgInt32 nLightStart, cgInt32 nLightEnd )
{
    // See if this task already exists
    TaskList::iterator itTask = mTasks.begin();
    while ( itTask != mTasks.end() )
    {
        // Is this a task for this frame?
        if ( (*itTask).frame == nFrame &&
            (*itTask).type == TaskType &&
            (*itTask).staticUpdate == bStatic &&
            (*itTask).lightIndexStart == nLightStart &&
            (*itTask).lightIndexEnd == nLightEnd )
        {
            return true;
        
        } // End if this frame task

        // Task for next frame
        if ( (*itTask).frame > nFrame )
            break;
        else
            ++itTask;

    } // Next task

    // No match
    return false;
}


//-----------------------------------------------------------------------------
//  Name : addTask ()
/// <summary>
/// Adds a new task to the task list
/// </summary>
//-----------------------------------------------------------------------------
void cgRadianceGrid::addTask( cgIndirectLightingTaskType::Base TaskType, cgUInt32 nFrame, bool bStatic, cgInt32 nLightStart, cgInt32 nLightEnd )
{
    // Get the light array 
    cgObjectNodeArray & aLights = bStatic ? mStaticLights : mDynamicLights;

    // Ensure we have the proper end point
    nLightEnd = max( 0, min( nLightEnd, (cgInt32)aLights.size() - 1 ) );

    // If this task already exists, exit
    if ( taskExists( TaskType, nFrame, bStatic, nLightStart, nLightEnd ) )
        return;

    // Assume all slices get updated
    cgIndirectLightingGridSlice s;
    s.gridIndex  = mCascadeId;
    s.sliceStart = 0;
    s.sliceEnd   = cgInt32( mGridDimensions.z ) - 1;

    // Create a new task
    cgIndirectLightingTask Task;
    Task.frame           = nFrame;
    Task.type             = TaskType;
    Task.staticUpdate          = bStatic;
    Task.lightIndexStart = nLightStart;
    Task.lightIndexEnd   = nLightEnd;
    Task.slices.push_back( s );

    // Add to list
    mTasks.push_back( Task );
}

//-----------------------------------------------------------------------------
//  Name : processTasks ()
/// <summary>
/// Processes all tasks for the current frame
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::processTasks( )
{
    // Get the current frame
    cgUInt32 nCurrentFrame = cgTimer::getInstance()->getFrameCounter();

    // Do any work required for this frame
    TaskList::iterator itTask = mTasks.begin();
    while ( itTask != mTasks.end() )
    {
        // Is this a task for this frame?
        if ( (*itTask).frame == nCurrentFrame )
        {
            // Process the task
            processTask( *itTask );

            // Remove the task since it is now complete
            mTasks.erase( itTask++ );
        }
        else
        {
            ++itTask;
        }

    } // Next task

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : inject ()
/// <summary>
/// Injects RSM lighting into grid
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::inject( bool bStatic )
{
    // Setup constant buffer
    cgConstantBuffer * pGridBuffer = mGridConstants.getResource( true );
    _cbGrid * pGridData = (_cbGrid*)pGridBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
    pGridData->gridMin            = mBounds.min;
    pGridData->gridMax            = mBounds.max;
    pGridData->gridDimensions     = mGridDimensions;
    pGridData->gridShift          = mCameraDelta; 
    pGridData->gridShift.y       *= -1.0f; // Y axis shift reversed as texture directions are reversed
    pGridData->gridColor          = mGridColor;
    pGridData->gridColor.w        = mLightStrength;

    // Set previous grid constants
    if ( mCascadeId == 0 )
    {
        pGridData->gridMinPrev        = cgVector3(0,0,0);
        pGridData->gridMaxPrev        = cgVector3(0,0,0);
        pGridData->gridDimensionsPrev = cgVector3(0,0,0);
        pGridData->blendRegionPrev    = 0.0f;
    }
    else
    {
        cgRadianceGrid * pSibling     = mManager->getRadianceGrid( mCascadeId - 1 );
        pGridData->gridMinPrev        = pSibling->mBounds.min;
        pGridData->gridMaxPrev        = pSibling->mBounds.max;
        pGridData->gridDimensionsPrev = pSibling->mGridDimensions;
        pGridData->blendRegionPrev    = pSibling->mBlendRegion;
    }

    // Unlock the buffer. 
    pGridBuffer->unlock();

    // Apply the constant buffer during the next draw
    mDriver->setConstantBufferAuto( mGridConstants );

    // Get a scratch buffer from the manager
    cgRadianceBuffer * pDestBuffer = mManager->getScratchBuffer( mGridDimensions );
    cgAssert( pDestBuffer != CG_NULL );
    cgRenderTargetHandleArray aTargets;
    aTargets.push_back( pDestBuffer->mTargets[ 0 ] );
    aTargets.push_back( pDestBuffer->mTargets[ 1 ] );
    aTargets.push_back( pDestBuffer->mTargets[ 2 ] );

    // Set the radiance buffer as render target
    if ( mDriver->beginTargetRender( aTargets ) )
    {
        // Clear the targets
        mDriver->clear( cgClearFlags::Target, 0, 0, 0 );

        // Gather light (always dynamic lights)
        injectLights( mDynamicLights );

        // Done rendering
        mDriver->endTargetRender();

    } // End begin rendering

    // Exchange textures with manager (we are swapping ownership of the pointers here)
    cgRadianceBuffer * pPrevBuffer = mDynamicRadiance1;
    mDynamicRadiance1 = pDestBuffer;
    mManager->replaceScratchBuffer( pDestBuffer, pPrevBuffer );

    // Update box and time information for this buffer
    mDynamicRadiance1->mBounds  = mBounds;
    mDynamicRadiance1->mTime = (cgFloat)cgTimer::getInstance()->getTime();

    mStaticDirty = false;
    mDynamicDirty = false;

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : injectLights()
/// <summary>
/// Injects RSM samples into the current grid. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceGrid::injectLights( const cgObjectNodeArray & aLights )
{
    // Get access to required systems
    cgTexturePool  * pPool = mManager->getShadowMaps();
    cgScriptObject * pScript = mManager->mScriptObject;
    const cgString & strRSMCallback = mManager->mIndirectLightCallback;
    cgLightingSystemStates & States = mManager->mStates;

    // Get the integer grid extents and camera deltas
    cgInt32 sx = (cgInt32)mGridDimensions.x;
    cgInt32 sy = (cgInt32)mGridDimensions.y;
    cgInt32 sz = (cgInt32)mGridDimensions.z;

    // Get the surface shader
    cgSurfaceShader * pShader = States.lightingShader.getResource(true);

    // Set rendering states
    mDriver->setDepthStencilState( States.disabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setBlendState( States.additiveBlendState );

    // For each light (frustum/rsm) we wish to process this frame
    for ( size_t j = 0; j < aLights.size(); ++j )
    {
        // Get the current light
        cgLightNode * pLight = (cgLightNode*)aLights[ j ];

        // Begin indirect lighting 
        cgInt32 nNumPasses = pLight->beginIndirectLighting( pPool );

        // For each pass (i.e., RSM)
        for( cgInt32 nPass = 0; nPass < nNumPasses; ++nPass )
        {
            // Begin the pass (Either prepares for RSM fill or sets up the RSM inputs.)
            cgVisibilitySet * pRenderSet;
            cgLightNode::LightingOp Op = pLight->beginIndirectLightingPass( nPass, pRenderSet );

            // What operation are we performing?
            if ( Op == cgLightNode::Lighting_FillShadowMap )
            {
                // We need to fill a default resource, do so...
                // Notify the script that it should render.
                try
                {
                    cgScriptArgument::Array ScriptArgs;
                    static const cgString strContext = _T("FillReflectiveShadowMap");
                    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"),  &strContext ) );
                    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("LightNode@+"),     pLight ) );
                    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("VisibilitySet@+"), pRenderSet ) );
                    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord,   _T("uint"),           &nPass ) );

                    // Execute the specified script function.
                    pScript->executeMethodVoid( strRSMCallback, ScriptArgs );

                } // End try to execute
                catch ( cgScriptInterop::Exceptions::ExecuteException & e )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strRSMCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );

                } // End catch exception					

            } // End if FillReflectiveShadowMap
            // Otherwise, inject into our volume
            else if ( Op == cgLightNode::Lighting_ProcessLight )
            {
                // Set the viewport to cover entire target
                cgViewport Viewport;
                Viewport.x      = 0;
                Viewport.y      = 0;
                Viewport.width  = sx * sx;
                Viewport.height = sy;
                Viewport.minimumZ   = 0.0f;
                Viewport.maximumZ   = 1.0f;
                mDriver->setViewport( &Viewport );

                // Get the size of the current VPL buffer
                cgSize vplDimensions = mDriver->getVPLBufferDimensions();
                if ( vplDimensions.width > 0 && vplDimensions.height > 0 )
                {
                    // Get the matching vertex buffer
                    cgVertexBufferHandle hVertexBuffer = mDriver->getVPLVertexBuffer( vplDimensions );
                    cgVertexBuffer * pVertexBuffer = hVertexBuffer.getResource(true);
                    if ( pVertexBuffer )
                    {
                        // Set shaders
                        if ( !pShader->selectVertexShader( _T("lightInjectionVS") ) )
                            return false;
                        if ( !pShader->selectPixelShader( _T("lightInjectionPS") ) )
                            return false;

                        // Draw the vertex buffer to inject values
                        mDriver->setVertexFormat( pVertexBuffer->getFormat() );
                        mDriver->setStreamSource( 0, hVertexBuffer );
                        mDriver->drawPrimitive( cgPrimitiveType::PointList, 0, vplDimensions.width * vplDimensions.height );
                    }
                }

            } // End if ProcessLight

            // End pass
            pLight->endIndirectLightingPass( );

        } // Next pass

        // End indirect
        pLight->endIndirectLighting( );

    } // Next light

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : propagate ()
// Desc : Pulls indirect lighting through the grid
//-----------------------------------------------------------------------------
void cgRadianceGrid::propagate( bool bStatic )
{
    // Get access to lighting states
    cgLightingSystemStates & States = mManager->mStates;

    // Get the surface shader
    cgSurfaceShader * pShader = States.lightingShader.getResource(true);

    // Get the current source buffer 
    cgRadianceBuffer * pSrcBuffer = bStatic ? mStaticRadiance : mDynamicRadiance1;

    // Get two scratch buffers for propagation
    cgRadianceBuffer * pPropBufferA, * pPropBufferB;
    if ( !mManager->getScratchBuffers( mGridDimensions, pPropBufferA, pPropBufferB ) )
        return;

    // Update grid constant buffer
    cgConstantBuffer * pGridBuffer = mGridConstants.getResource( true );
    _cbGrid * pGridData = (_cbGrid*)pGridBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
    pGridData->gridMin            = mBounds.min;
    pGridData->gridMax            = mBounds.max;
    pGridData->gridDimensions     = mGridDimensions;
    pGridData->gridShift          = mCameraDelta; 
    pGridData->blendRegion        = mBlendRegion;
    pGridData->gridColor          = mGridColor;
    pGridData->gridColor.w        = mLightStrength;
    pGridBuffer->unlock();
    mDriver->setConstantBufferAuto( mGridConstants );

    // Set states
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setDepthStencilState( States.disabledDepthState, 0 );

    // Make sure the shader compiles
    if ( !pShader->selectPixelShader( _T("propagateIndirectLighting") ) )
        return;

    // Propagate the lighting over the desired number of passes
    const cgInt32 nNumPropagationPasses = 8;
    cgRenderTargetHandleArray aTargets(3);
    cgRadianceBuffer * pInput, * pOutput;
    for ( cgInt32 j = 0; j < nNumPropagationPasses; j++ )
    {
        // Setup input/output targets (Note: The first pass initializes with the injected rsm direct lighting.)
        if ( (j % 2) == 0 )
        {
            pInput  = (j == 0) ? pSrcBuffer : pPropBufferB;
            pOutput = pPropBufferA;
        
        } // End if even passes
        else
        {
            pInput  = pPropBufferA;
            pOutput = pPropBufferB;
        
        } // End if odd passes

        // Propagate lighting from input to output
        aTargets[0] = pOutput->mTargets[ 0 ];
        aTargets[1] = pOutput->mTargets[ 1 ];
        aTargets[2] = pOutput->mTargets[ 2 ];
        if ( mDriver->beginTargetRender( aTargets ) )
        {
            // Set the shaders
            if ( !pShader->selectPixelShader( _T("propagateIndirectLighting") ) )
            {
                mDriver->endTargetRender();
                return;
            
            } // End if failed

            // Set the input textures
            States.harmonicsRedSampler->apply( pInput->mTargets[ 0 ] );
            States.harmonicsGreenSampler->apply( pInput->mTargets[ 1 ] );
            States.harmonicsBlueSampler->apply( pInput->mTargets[ 2 ] );

            // Use standard blending (i.e., replace)
            mDriver->setBlendState( cgBlendStateHandle::Null );

            // Draw quad
            mDriver->drawScreenQuad();

            // Restore rendering to previous target.
            mDriver->endTargetRender();

        } // End begin rendering

        // Accumulate current propagation results
        aTargets[0] = pSrcBuffer->mTargets[0];
        aTargets[1] = pSrcBuffer->mTargets[1];
        aTargets[2] = pSrcBuffer->mTargets[2];
        if ( mDriver->beginTargetRender( aTargets ) )
        {
            // Set the shaders
            if ( !pShader->selectPixelShader( _T("readGrid") ) )
            {
                mDriver->endTargetRender();
                return;
            
            } // End if failed

            // Inputs for accumulation are the outputs of the propagation pass
            States.harmonicsRedSampler->apply( pOutput->mTargets[ 0 ] );
            States.harmonicsGreenSampler->apply( pOutput->mTargets[ 1 ] );
            States.harmonicsBlueSampler->apply( pOutput->mTargets[ 2 ] );

            // Use additive blending
            mDriver->setBlendState( States.additiveBlendState );

            // Draw quad
            mDriver->drawScreenQuad();

            // Restore rendering to previous target.
            mDriver->endTargetRender();

        } // End begin rendering

    } // Next propagation pass

}

//-----------------------------------------------------------------------------
//  Name : debug ()
/// <summary>
/// Draws grid
/// </summary>
//-----------------------------------------------------------------------------
void cgRadianceGrid::debug( )
{
    // Get access to lighting states
    cgLightingSystemStates & States = mManager->mStates;

    // Get the surface shader
    cgSurfaceShader * pShader = States.lightingShader.getResource(true);

    // Update grid constant buffer
    cgConstantBuffer * pGridBuffer = mGridConstants.getResource( true );
    _cbGrid * pGridData = (_cbGrid*)pGridBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
    pGridData->gridMin            = mBounds.min;
    pGridData->gridMax            = mBounds.max;
    pGridData->gridDimensions     = mGridDimensions;
    pGridData->gridShift          = mCameraDelta; 
    pGridData->blendRegion        = mBlendRegion;
    pGridData->gridColor          = mGridColor;
    pGridBuffer->unlock();
    mDriver->setConstantBufferAuto( mGridConstants );

    // Set pixel shader 
    if ( !pShader->selectVertexShader( _T("drawGridVS") ) )
        return;
    if ( !pShader->selectPixelShader( _T("drawGridPS") ) )
        return;

    // Set states
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setDepthStencilState( States.lessEqualTestDepthState );
    mDriver->setBlendState( States.alphaBlendState );

    // Draw lines
    cgUInt32 numCells = (cgUInt32)mGridDimensions.x;
    cgVertexFormat * pPointVertexFormat = cgVertexFormat::formatFromDeclarator( cgPointVertex::Declarator );
    mDriver->setVertexFormat( pPointVertexFormat );
    mDriver->setStreamSource( 0, mGridLineVertexBuffer );
    mDriver->drawPrimitive( cgPrimitiveType::LineList, 0,  3 * (numCells + 1) * (numCells + 1) );
}

///////////////////////////////////////////////////////////////////////////////
// cgRadianceBuffer Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRadianceBuffer ()
/// <summary>
/// Class constructor
/// </summary>
//-----------------------------------------------------------------------------
cgRadianceBuffer::cgRadianceBuffer()
{
    mDimensions = cgVector3(0,0,0);
    mUnrolled  = true;
    mTime      = 0;
}

//-----------------------------------------------------------------------------
//  Name : ~cgRadianceBuffer ()
/// <summary>
/// Class destructor
/// </summary>
//-----------------------------------------------------------------------------
cgRadianceBuffer::~cgRadianceBuffer()
{
    for ( cgUInt32 i = 0; i < mTargets.size(); i++ )
        mTargets[ i ].close();

    mDimensions = cgVector3(0,0,0);
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Creates render targets of appropriate size for radiance capture
/// </summary>
//-----------------------------------------------------------------------------
bool cgRadianceBuffer::initialize( cgRenderDriver * pDriver, const cgVector3 & vDimensions )
{
    mDimensions = vDimensions;

    // Close any prior target handles
    for ( cgUInt32 i = 0; i < mTargets.size(); i++ )
        mTargets[ i ].close();

    // Build our radiance buffer(s)
    cgImageInfo Desc;
    Desc.type      = cgBufferType::RenderTarget;
    Desc.pool      = cgMemoryPool::Default;
    Desc.format    = cgBufferFormat::R16G16B16A16_Float;
    Desc.mipLevels = 1;
    if ( mUnrolled )
    {
        Desc.width  = cgUInt32(vDimensions.x * vDimensions.z);
        Desc.height = cgUInt32(vDimensions.y);
        Desc.depth  = 1;
    
    } // End if unrolled
    else
    {
        Desc.width  = cgUInt32(vDimensions.x);
        Desc.height = cgUInt32(vDimensions.y);
        Desc.depth  = cgUInt32(vDimensions.z);
    
    } // End if !unrolled

    mTargets.resize( 4 );
    cgResourceManager * pResources = pDriver->getResourceManager();
    for ( cgInt32 i = 0; i < 4; i++ )
    {
        if ( !pResources->createRenderTarget( &mTargets[ i ], Desc, cgResourceFlags::ForceNew, cgString::format( _T("%s_%d"), _T("RadianceBuffer_Target_"), i ), cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create a radiance buffer render target.") );
            return false;
        
        } // End if failed
    
    } // Next target

    // Success
    return true;
}