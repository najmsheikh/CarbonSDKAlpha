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
// Name : cgProceduralTreeGenerator.h                                        //
//                                                                           //
// Desc : Utility classes capable of generating tree meshes and associated   //
//        materials procedurally, based on supplied parameters.              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPROCEDURALTREEGENERATOR_H_ )
#define _CGE_CGPROCEDURALTREEGENERATOR_H_

//-----------------------------------------------------------------------------
// cgProceduralTreeGenerator Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldComponent.h>
#include <Math/cgBezierSpline.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {FD0D4AFF-D4D2-4FEE-956E-5386EC431B99}
const cgUID RTID_ProceduralTreeGenerator = { 0xfd0d4aff, 0xd4d2, 0x4fee, { 0x95, 0x6e, 0x53, 0x86, 0xec, 0x43, 0x1b, 0x99 } };

//-----------------------------------------------------------------------------
// Global Enumerations
//-----------------------------------------------------------------------------
namespace cgProceduralTreeFrondType
{
    enum Base
    {
        Blades      = 0,
        Extrusion   = 1
    };

} // End namespace : cgProceduralTreeFrondType

namespace cgProceduralTreeGrowthCondition
{
    enum Base
    {
        Enabled     = 0,
        Disabled    = 1,
        Pruned      = 2
    };

} // End namespace : cgProceduralTreeGrowthCondition

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeFrondDesc (Class)
/// <summary>
/// Houses all of the information / parameters that describe how tree fronds
/// should be selected / generated.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProceduralTreeFrondDesc
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgProceduralTreeGenerator;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgProceduralTreeFrondDesc( ) :
        mDatabaseId(0), enabled(false), initialLevel(0), minimumDistance(0.25), depth(0), aboveCondition(cgProceduralTreeGrowthCondition::Enabled),
        belowCondition(cgProceduralTreeGrowthCondition::Disabled), sizeFactor(1), minimumOffsetAngle(0), maximumOffsetAngle(0),
        type( cgProceduralTreeFrondType::Blades ), blades(2), extrusionSegments(2), lengthSegmentOverride(false), lengthSegments(2)
        {};

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    bool                                    enabled;
    cgUInt32                                initialLevel;
    cgDouble                                minimumDistance;
    cgUInt32                                depth;
    cgProceduralTreeGrowthCondition::Base   aboveCondition;
    cgProceduralTreeGrowthCondition::Base   belowCondition;
    cgDouble                                sizeFactor;
    cgDouble                                minimumOffsetAngle;
    cgDouble                                maximumOffsetAngle;
    cgProceduralTreeFrondType::Base         type;
    cgBezierSpline2                         extrusionProfile;
    cgUInt32                                extrusionSegments;
    cgUInt32                                blades;
    bool                                    lengthSegmentOverride;
    cgUInt32                                lengthSegments;

    cgMaterialHandle                        material;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32            mDatabaseId;
};

//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeGrowthLevel (Class)
/// <summary>
/// Houses all of the information / parameters that describe how an individual
/// level of a procedural tree (i.e. trunk, branch segments, etc.) should be
/// 'grown' or generated.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProceduralTreeGrowthLevel
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgProceduralTreeGenerator;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgProceduralTreeGrowthLevel( ) :
        mDatabaseId(0) {};

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    cgBezierSpline2     azimuthInitial;
    cgBezierSpline2     polarInitial;
    cgBezierSpline2     azimuthTwist;
    cgBezierSpline2     polarTwist;
    cgBezierSpline2     gravity;
    cgBezierSpline2     gravityProfile;
    cgBezierSpline2     flexibility;
    cgBezierSpline2     flexibilityProfile;
    cgBezierSpline2     length;
    cgBezierSpline2     radius;
    cgBezierSpline2     radiusProfile;
    cgUInt32            segments;
    cgUInt32            segmentVertices;
    cgBezierSpline2     segmentVerticesProfile;
    cgDouble            segmentPack;
    
    cgDouble            roughnessValue;
    cgDouble            roughnessVariance;
    cgDouble            roughnessFrequencyU;
    cgDouble            roughnessFrequencyV;
    cgDouble            roughnessGnarl;
    cgBezierSpline2     roughnessProfile;
    cgBezierSpline2     roughnessGnarlProfile;
    
    // Applies only to branches
    cgDouble            levelBegin;
    cgDouble            levelEnd;
    cgDouble            pruneDistance;
    cgDouble            frequency;
    cgBezierSpline2     frequencyProfile;
    bool                roughnessGnarlUnison;

    cgUInt32            pruneDepth;
    bool                enableForkPrune;
    cgDouble            forkBias;
    cgDouble            forkAngle;
    cgUInt32            forkLimit;
    cgDouble            segmentLengthKeep;
    cgDouble            segmentVerticesKeep;

    cgMaterialHandle    material;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32            mDatabaseId;
};

//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeGrowthProperties (Class)
/// <summary>
/// Houses all of the information / parameters that describe how a procedural
/// tree should be 'grown' or generated.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProceduralTreeGrowthProperties
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgProceduralTreeGenerator;

public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    CGE_ARRAY_DECLARE( cgProceduralTreeGrowthLevel, LevelArray );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgProceduralTreeGrowthProperties( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    /// <summary>Random seed used to select growth properties for the main tree / branches.</summary>
    cgInt32                     mainSeed;
    /// <summary>Random seed used to select growth properties for any trunk / branch flares.</summary>
    cgInt32                     flareSeed;
    /// <summary>Overall uniform scale value for the tree.</summary>
    cgDouble                    globalScale;
    /// <summary>Random variance to apply to the global scale.</summary>
    cgDouble                    globalScaleVariance;
    /// <summary>Base scale value for texture coordinates along the U axis.</summary>
    cgDouble                    globalTextureUScale;
    /// <summary>Base scale value for texture coordinates along the V axis.</summary>
    cgDouble                    globalTextureVScale;
    /// <summary>Growth data for the main trunk.</summary>
    cgProceduralTreeGrowthLevel trunkData;
    /// <summary>Growth data for fronds (if any).</summary>
    cgProceduralTreeFrondDesc   frondData;
    /// <summary>Growth data for the various levels of branches.</summary>
    LevelArray                  branchLevels;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeGenerator (Class)
/// <summary>
/// Utility class capable of generating tree meshes and associated materials 
/// procedurally, based on supplied parameters.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProceduralTreeGenerator : public cgWorldComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgProceduralTreeGenerator, cgWorldComponent, "ProceduralTreeGenerator" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgProceduralTreeGenerator( );
             cgProceduralTreeGenerator( cgUInt32 referenceId, cgWorld * world );
    virtual ~cgProceduralTreeGenerator( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgProceduralTreeGrowthProperties & getParameters  ( ) const;
    void                        setParameters           ( const cgProceduralTreeGrowthProperties & parameters );
    cgMeshHandle                generate                ( );
    /*void                        setMainSeed             ( cgUInt32 seed );
    void                        setFlareSeed            ( cgUInt32 seed );
    void                        setGlobalScale          ( cgDouble scale );
    void                        setGlobalScaleVariance  ( cgDouble scaleVariance );
    void                        setTrunkParameters      ( const cgProceduralTreeGrowthLevel & parameters );
    void                        setBranchLevelParameters( cgUInt32 level, const cgProceduralTreeGrowthLevel & parameters );*/
    //cgUInt32                    addBranchLevel          ( const cgProceduralTreeGrowthLevel & parameters );
    //void                        removeBranchLevel       ( cgUInt32 level );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual cgString            getDatabaseTable        ( ) const;
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual void                onComponentDeleted      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ProceduralTreeGenerator; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct Segment
    {
        Segment() : vertices(0), startVertex(0), forkBranches(0), length(0), radius(0), deviation(0), segmentLocation(0) {}

        cgTransform         transform;
        cgDouble            length;
        cgDouble            radius;
        cgDouble            segmentLocation;
        /// <summary>Records the total angle of deviation between this segment's growth axis and the prior segment.</summary>
        cgDouble            deviation;
        cgUInt32            vertices;
        cgUInt32            startVertex;
        /// <summary>Records the total number of branches that have so far been accepted as a secondary "fork" at this segment.</summary>
        cgUInt32            forkBranches;
    
    }; // End struct: Segment

    struct Branch
    {
        Branch() : level(0), parentBranch(0), forkBranch(false), frondBranch(false), branchLocation(0), length(0) {}

        cgArray<Segment>    segments;
        cgInt32             level;
        cgInt32             parentBranch;
        cgDouble            length;
        cgDouble            branchLocation;
        cgArray<cgInt32>    childBranches;
        bool                forkBranch;
        bool                frondBranch;
    
    }; // End struct: Branch
    
    struct Level
    {
        cgArray<Branch>     branches;
        
    }; // End struct: Level

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );
    void                        growTrunk               ( );
    bool                        growBranch              ( cgDouble growthRand, cgProceduralTreeGrowthLevel & branchLevel, cgInt32 outBranchLevel, cgInt32 outBranchIndex, cgInt32 branchAttempt, cgInt32 parentBranchIndex, cgInt32 parentAttempt );
    bool                        buildMesh               ( );
    void                        clear                   ( );
    bool                        deserializeLevel        ( cgWorldQuery & levelQuery, bool cloning, cgProceduralTreeGrowthLevel & levelOut );
    bool                        serializeLevel          ( cgProceduralTreeGrowthLevel & level, cgUInt32 parentId, cgWorld * world, cgUInt32 levelType, cgUInt32 levelOrder );
    bool                        deleteLevel             ( cgProceduralTreeGrowthLevel & level );
    void                        deserializeSpline       ( cgWorldQuery & levelQuery, cgBezierSpline2 & splineOut, const cgString & prefix );
    void                        serializeSpline         ( cgWorldQuery & levelQuery, const cgBezierSpline2 & spline, cgUInt16 paramOffset );
    bool                        deserializeFrond        ( cgWorldQuery & frondQuery, bool cloning, cgProceduralTreeFrondDesc & levelOut );
    bool                        serializeFrond          ( cgProceduralTreeFrondDesc & frond, cgUInt32 parentId, cgWorld * world );    

    //-------------------------------------------------------------------------
    // Protected Inline Methods
    //-------------------------------------------------------------------------
    inline  cgDouble    eval        ( cgBezierSpline2 & s, cgBezierSpline2::EvaluateMethod m, cgDouble x, cgDouble r )
    {
        return (cgDouble)s.evaluateForX( m, (cgFloat)x, (cgFloat)r, false, 5 );
    }

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgProceduralTreeGrowthProperties    mParams;
    cgMeshHandle                        mMesh;
    cgArray<Level>                      mLevels;
    cgDouble                            mGlobalScale;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertGenerator;
    static cgWorldQuery mUpdateGenerator;
    static cgWorldQuery mInsertLevel;
    static cgWorldQuery mUpdateLevel;
    static cgWorldQuery mDeleteLevel;
    static cgWorldQuery mDeleteLevels;
    static cgWorldQuery mInsertFrond;
    static cgWorldQuery mUpdateFrond;
    static cgWorldQuery mLoadGenerator;
    static cgWorldQuery mLoadLevels;
    static cgWorldQuery mLoadFronds;
};

#endif // !_CGE_CGPROCEDURALTREEGENERATOR_H_