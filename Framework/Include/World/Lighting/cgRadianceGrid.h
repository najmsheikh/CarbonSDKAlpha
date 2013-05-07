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
// Name : cgRadianceGrid.h                                                   //
//                                                                           //
// Desc : Classes that manage indirect lighting radiance over a grid.        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRADIANCEGRID_H_ )
#define _CGE_CGRADIANCEGRID_H_

//-----------------------------------------------------------------------------
// cgRadianceGrid Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Math/cgMathTypes.h>
#include <World/Lighting/cgLightingTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;
class cgLightNode;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgRadianceBuffer (Class)
/// <summary>
/// Maintains indirect lighting radiance data.
/// </summary>
//-----------------------------------------------------------------------------
class cgRadianceBuffer
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgLightingManager;
    friend class cgRadianceGrid;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgRadianceBuffer();	
    ~cgRadianceBuffer();

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                initialize      ( cgRenderDriver * pDriver, const cgVector3 & vDimensions );
    // ToDo: 6767 - Make sure these have implementations!
    const cgVector3 & getTextureSize3D( ) const; //  ( ) const { return mDimensions; }
    const cgVector2 & getTextureSize2D( ) const; //  ( ){ return cgVector2( mDimensions.x * mDimensions.z, mDimensions.y ); }

private:
    //-------------------------------------------------------------------------
    // Private Member Variables
    //-------------------------------------------------------------------------
    cgRenderTargetHandleArray mTargets;     // The buffers for storing radiance values.
    cgVector3                 mDimensions;  // The texture dimensions
    cgBoundingBox             mBounds;      // The axis aligned box in world space for this buffer
    cgFloat                   mTime;        // The time this buffer was last updated.
    bool                      mUnrolled;    // Is the texture 2D or 3D?
};


//-----------------------------------------------------------------------------
//  Name : cgRadianceGrid (Class)
/// <summary>
/// Class responsible for managing and generating an indirect lighting radiance
/// grid.
/// </summary>
//-----------------------------------------------------------------------------
class cgRadianceGrid
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgLightingManager;

public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    CGE_LIST_DECLARE    ( cgIndirectLightingTask, TaskList );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgRadianceGrid ( cgLightingManager * lightingManager, cgInt32 cascadeId );
    ~cgRadianceGrid ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        initialize          ( cgRenderDriver * driver, cgInt32 method, const cgVector3 dimensions, float cellSize, cgUInt32 paddingCells, float lightStrength, float blendRegion );
    bool                        update              ( cgCameraNode * camera, float closestIntersectionDistance, float snapSize = 0.0f );
    void			            addLights           ( const cgObjectNodeArray & staticLights, const cgObjectNodeArray & dynamicLights );
    bool                        processTasks        ( );
    bool			            composite           ( bool lowResolution );
    void			            debug               ( );
    TaskList                  & getTasks            ( );
    const TaskList            & getTasks            ( ) const;
    const cgObjectNodeArray   & getLights           ( bool staticLights ) const;


protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    // Constant buffer mapped structures
    struct _cbGrid
    {
        cgVector3 gridMin;
        cgVector3 gridMax;
        cgVector3 gridDimensions;
        cgVector3 gridMinPrev;
        cgVector3 gridMaxPrev;
        cgVector3 gridDimensionsPrev;
        cgVector3 gridShift;
        cgVector4 gridColor;
        cgFloat   blendRegion;
        cgFloat   blendRegionPrev;

    }; // End Struct : _cbGrid

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool processTask            ( const cgIndirectLightingTask & task );
    void addTask                ( cgIndirectLightingTaskType::Base taskType, cgUInt32 frame, bool staticLights, cgInt32 lightStart = 0, cgInt32 lightEnd = 999 );
    bool taskExists             ( cgIndirectLightingTaskType::Base taskType, cgInt32 frame, bool staticLights, cgInt32 lightStart, cgInt32 lightEnd );
    bool affectsGrid            ( cgLightNode * light );
    bool reprojectGrid          ( cgRadianceBuffer * destination, cgRadianceBuffer * source, const cgRect & bounds, cgInt32 minimumZ, cgInt32 maximumZ );
    bool interpolateDynamicGrids( );

    bool gather                 ( bool staticLights );
    bool gatherLightsStatic     ( bool updateAll, const cgRect & bounds, cgInt32 minimumZ, cgInt32 maximumZ );
    bool gatherLightsDynamic    ( bool updateAll, const cgRect & bounds, cgInt32 minimumZ, cgInt32 maximumZ );
    bool gatherLights           ( cgRadianceBuffer * destination, cgObjectNodeArray & lights, bool updateAll, const cgRect & bounds, cgInt32 minimumZ, cgInt32 maximumZ );

    bool inject                 ( bool staticLights );
    bool injectLights           ( const cgObjectNodeArray & lights );
    void propagate              ( bool staticLights );

    //-------------------------------------------------------------------------
    // Protected Member Variables
    //-------------------------------------------------------------------------
    cgLightingManager         * mManager;               // The lighting manager that owns this grid
    cgRenderDriver			  * mDriver;                // The current render driver.
    cgMeshHandle                mGridMesh;              // A unit box for drawing the grid
    cgSurfaceShaderHandle       mShader;                // Internal surface shader file used to correctly handle lighting features.
    cgConstantBufferHandle      mGridConstants;         // Constants for grid.
    cgScriptArgument::Array     mStencilVSArgs;         // Preset argument script list for use during stenciling  (vertex shader)
    cgScriptArgument::Array     mCompositeVSArgs;       // Preset argument script list for use during compositing (vertex shader)
    cgScriptArgument::Array     mCompositePSArgs;       // Preset argument script list for use during compositing (pixel shader)
    cgScriptArgument::Array     mInjectionPSArgs;       // Preset argument script list for use during injection (pixel shader)
    TaskList                    mTasks;                 // The list of tasks to perform for indirect lighting
    cgObjectNodeArray           mStaticLights;          // The list of static lights for this grid
    cgObjectNodeArray           mDynamicLights;         // The list of dynamic lights for this grid

    cgRadianceBuffer          * mStaticRadiance;        // The buffer storing radiance for static light sources
    cgRadianceBuffer          * mDynamicRadiance0;      // The buffer storing radiance for dynamic light sources (kf0)
    cgRadianceBuffer          * mDynamicRadiance1;      // The buffer storing radiance for dynamic light sources (kf1)
    cgFloat                     mDynamicUpdateRate;     // The rate at which the grid should update
    cgUInt32                    mDynamicUpdateFrames;   // The number of frames between each dynamic grid update attempt

    cgVertexBufferHandle        mGridLineVertexBuffer;

    cgInt32					    mCascadeId;             // The index of this grid in the cascade (0 = highest res)
    cgInt32                     mMethod;                // The indirect lighting method
    cgInt32                     mDynamicMethod;         // The method used for dynamic lights
    cgBoundingBox               mBounds;                // The axis aligned box that represents this grid
    cgUInt32				    mPaddingCells;          // Number of cells 'behind' the camera to capture lighting
    cgFloat                     mCellSize;              // The size of a cell in meters (world space)
    cgFloat                     mBlendRegion;           // At what percent of the grid size should we fade to another grid?
    cgVector3                   mGridDimensions;        // Number of cells in x,y,z
    cgVector3                   mGridSize;              // Grid size in meters
    cgVector4                   mGridColor;             // A color for the grid (e.g., for debugging, etc.)
    cgVector3                   mCameraDelta;           // Change in grid location from prior due to camera position/orientation
    cgFloat                     mLightStrength;         // A multiplier for lighting for this grid
    bool                        mGridDirty;
    bool                        mStaticDirty;
    bool                        mDynamicDirty;
    bool                        mDynamicInitialized;
    bool					    mUnrolled;              // Are we using an unrolled radiance buffer (i.e., 2D)?
};

#endif // !_CGE_CGRADIANCEGRID_H_