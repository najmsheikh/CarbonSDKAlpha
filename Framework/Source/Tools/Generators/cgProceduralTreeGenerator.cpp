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
// Name : cgProceduralTreeGenerator.cpp                                      //
//                                                                           //
// Desc : Utility classes capable of generating tree meshes and associated   //
//        materials procedurally, based on supplied parameters.              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgProceduralTreeGenerator Module Includes
//-----------------------------------------------------------------------------
#include <Tools/Generators/cgProceduralTreeGenerator.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgMesh.h>
#include <Math/cgRandom.h>
#include <System/cgStringUtility.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgProceduralTreeGenerator::mInsertGenerator;
cgWorldQuery cgProceduralTreeGenerator::mUpdateGenerator;
cgWorldQuery cgProceduralTreeGenerator::mInsertLevel;
cgWorldQuery cgProceduralTreeGenerator::mUpdateLevel;
cgWorldQuery cgProceduralTreeGenerator::mDeleteLevel;
cgWorldQuery cgProceduralTreeGenerator::mDeleteLevels;
cgWorldQuery cgProceduralTreeGenerator::mInsertFrond;
cgWorldQuery cgProceduralTreeGenerator::mUpdateFrond;
cgWorldQuery cgProceduralTreeGenerator::mLoadGenerator;
cgWorldQuery cgProceduralTreeGenerator::mLoadLevels;
cgWorldQuery cgProceduralTreeGenerator::mLoadFronds;

///////////////////////////////////////////////////////////////////////////////
// cgProceduralTreeGenerator Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeGenerator () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeGenerator::cgProceduralTreeGenerator( ) : cgWorldComponent( cgReferenceManager::generateInternalRefId(), CG_NULL )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgNavigationMesh () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeGenerator::cgProceduralTreeGenerator( cgUInt32 referenceId, cgWorld * world ) : cgWorldComponent( referenceId, world )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgProceduralTreeGenerator () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeGenerator::~cgProceduralTreeGenerator( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgProceduralTreeGenerator::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // We should simply 'disconnect' from the materials, never
    // physically remove the reference from the database.
    mParams.trunkData.material.close();
    mParams.frondData.material.close();
    for ( size_t i = 0; i < mParams.branchLevels.size(); ++i )
        mParams.branchLevels[i].material.close();

    // Clear out old data.
    clear();

    // Dispose base(s).
    if ( disposeBase )
        cgWorldComponent::dispose( true );
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
bool cgProceduralTreeGenerator::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ProceduralTreeGenerator )
        return true;

    // Supported by base?
    return cgWorldComponent::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgProceduralTreeGenerator::getDatabaseTable( ) const
{
    return _T("DataSources::ProceduralTree");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeGenerator::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new navigation mesh data.
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
bool cgProceduralTreeGenerator::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Create type tables as necessary.
        if ( !cgWorldComponent::createTypeTables( getReferenceType() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create world database type tables for defined object type '%s'.\n"), cgStringUtility::toString( getReferenceType(), _T("B") ).c_str() );
            return false;
        
        } // End if failed

        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("ProceduralTreeGenerator::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertGenerator.bindParameter( 1, mReferenceId );
        mInsertGenerator.bindParameter( 2, (cgUInt32)0 ); // Flags
        mInsertGenerator.bindParameter( 3, mParams.mainSeed );
        mInsertGenerator.bindParameter( 4, mParams.flareSeed );
        mInsertGenerator.bindParameter( 5, mParams.globalScale );
        mInsertGenerator.bindParameter( 6, mParams.globalScaleVariance );
        mInsertGenerator.bindParameter( 7, mParams.globalTextureUScale );
        mInsertGenerator.bindParameter( 8, mParams.globalTextureVScale );
        mInsertGenerator.bindParameter( 9, mSoftRefCount );
        
        // Execute
        if ( !mInsertGenerator.step( true ) )
        {
            cgString error;
            mInsertGenerator.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for procedural tree data source '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::insertComponentData") );
            return false;
        
        } // End if failed

        // Serialize levels. Trunk first.
        if ( !serializeLevel( mParams.trunkData, mReferenceId, mWorld, 0, 0 ) )
        {
            mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::insertComponentData") );
            return false;
        
        } // End if failed
            
        // Now branches.
        for ( size_t i = 0; i < mParams.branchLevels.size(); ++i )
        {
            if ( !serializeLevel( mParams.branchLevels[i], mReferenceId, mWorld, 3, i ) )
            {
                mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::insertComponentData") );
                return false;
            
            } // End if failed

        } // Next level.

        // Serialize frond data.
        if ( !serializeFrond( mParams.frondData, mReferenceId, mWorld ) )
        {
            mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("ProceduralTreeGenerator::insertComponentData") );

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
bool cgProceduralTreeGenerator::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the mesh data.
    prepareQueries();
    mLoadGenerator.bindParameter( 1, e->sourceRefId );
    if ( !mLoadGenerator.step( ) || !mLoadGenerator.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadGenerator.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for procedural tree data source '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for procedural tree data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // mLoadGenerator any pending read operation.
        mLoadGenerator.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadGenerator;

    // Clear out old parameter data.
    mParams.branchLevels.clear();

    // Grab parameters.
    cgUInt32 flags;
    mLoadGenerator.getColumn( _T("Flags"), flags );
    mLoadGenerator.getColumn( _T("MainSeed"), mParams.mainSeed );
    mLoadGenerator.getColumn( _T("FlareSeed"), mParams.flareSeed );
    mLoadGenerator.getColumn( _T("GlobalScale"), mParams.globalScale );
    mLoadGenerator.getColumn( _T("GlobalScaleVariance"), mParams.globalScaleVariance );
    mLoadGenerator.getColumn( _T("GlobalTextureUScale"), mParams.globalTextureUScale );
    mLoadGenerator.getColumn( _T("GlobalTextureVScale"), mParams.globalTextureVScale );

    // Attempt to load levels.
    mLoadLevels.bindParameter( 1, e->sourceRefId );
    if ( !mLoadLevels.step( ) )
    {
        // Log any error.
        cgString error;
        if ( !mLoadLevels.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve child level data for procedural tree data source '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve child level data for procedural tree data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadLevels.reset();
        mLoadGenerator.reset();
        return false;
    
    } // End if failed

    // Process each level.
    cgProceduralTreeGrowthLevel level;
    while ( mLoadLevels.nextRow() )
    {
        // Get the level type so we know what we're loading.
        cgUInt32 levelType;
        mLoadLevels.getColumn( _T("LevelType"), levelType );

        // Load the level.
        if ( !deserializeLevel( mLoadLevels, (mReferenceId != e->sourceRefId), level ) )
        {
            // Clean up.
            mLoadLevels.reset();
            mLoadGenerator.reset();
            return false;
        
        } // End if failed

        // Add to the list.
        if ( levelType == 0 )
        {
            // This is the trunk
            mParams.trunkData = level;

        } // End if trunk
        else if ( levelType == 3 )
        {
            // This is a branch
            mParams.branchLevels.push_back( level );

        } // End if branch

    } // Next level

    // Close pending reads.
    mLoadLevels.reset();

    // Attempt to load frond data.
    mLoadFronds.bindParameter( 1, e->sourceRefId );
    if ( !mLoadFronds.step( ) )
    {
        // Log any error.
        cgString error;
        if ( !mLoadFronds.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve frond data for procedural tree data source '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve frond data for procedural tree data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadFronds.reset();
        mLoadGenerator.reset();
        return false;
    
    } // End if failed

    // Deserialize any frond data.
    if ( mLoadFronds.nextRow() )
    {
        if ( !deserializeFrond( mLoadFronds, (mReferenceId != e->sourceRefId), mParams.frondData ) )
        {
            // Clean up.
            mLoadFronds.reset();
            mLoadGenerator.reset();
            return false;
        
        } // End if failed
    
    } // End if has frond data

    // Close pending reads.
    mLoadFronds.reset();

    // If we're cloning, take ownership of the materials.
    if ( mReferenceId != e->sourceRefId && !isInternalReference() )
    {
        mParams.trunkData.material.enableDatabaseUpdate( true, true );
        mParams.trunkData.material.enableDatabaseUpdate( false );
        mParams.frondData.material.enableDatabaseUpdate( true, true );
        mParams.frondData.material.enableDatabaseUpdate( false );
        for ( size_t i = 0; i < mParams.branchLevels.size(); ++i )
        {
            mParams.branchLevels[i].material.enableDatabaseUpdate( true, true );
            mParams.branchLevels[i].material.enableDatabaseUpdate( false );
        
        } // Next branch
    
    } // End if cloning

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
void cgProceduralTreeGenerator::onComponentDeleted( )
{
    // Remove our physical reference to any materials. Full database 
    // update and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    mParams.trunkData.material.enableDatabaseUpdate( !isInternalReference() );
    mParams.trunkData.material.close();
    mParams.frondData.material.enableDatabaseUpdate( !isInternalReference() );
    mParams.frondData.material.close();
    for ( size_t i = 0; i < mParams.branchLevels.size(); ++i )
    {
        mParams.branchLevels[i].material.enableDatabaseUpdate( !isInternalReference() );
        mParams.branchLevels[i].material.close();
    }

    // Call base class implementation last.
    cgWorldComponent::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeGenerator::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( !mInsertGenerator.isPrepared( mWorld ) )
            mInsertGenerator.prepare( mWorld, _T("INSERT INTO 'DataSources::ProceduralTree' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9)"), true );
        if ( !mUpdateGenerator.isPrepared( mWorld ) )
            mUpdateGenerator.prepare( mWorld, _T("UPDATE 'DataSources::ProceduralTree' SET Flags=?1, MainSeed=?2, FlareSeed=?3, GlobalScale=?4, GlobalScaleVariance=?5, GlobalTextureUScale=?6, GlobalTextureVScale=?7 WHERE RefId=?8"), true );
        if ( !mInsertLevel.isPrepared( mWorld ) )
            mInsertLevel.prepare( mWorld, _T("INSERT INTO 'DataSources::ProceduralTree::Levels' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,")
                                          _T("?21,?22,?23,?24,?25,?26,?27,?28,?29,?30,?31,?32,?33,?34,?35,?36,?37,?38,?39,?40,?41,?42,?43,?44,?45,?46,?47,?48,?49,?50,")
                                          _T("?51,?52,?53,?54,?55,?56,?57,?58,?59,?60,?61,?62,?63,?64,?65,?66,?67,?68,?69,?70,?71,?72,?73,?74,?75,?76,?77,?78,?79,?80,")
                                          _T("?81,?82,?83,?84,?85,?86,?87,?88,?89,?90,?91,?92,?93,?94,?95,?96,?97,?98,?99,?100,?101,?102,?103,?104,?105,?106,?107,?108,?109,?110,")
                                          _T("?111,?112,?113,?114)"), true );
        if ( !mUpdateLevel.isPrepared( mWorld ) )
            mUpdateLevel.prepare( mWorld, _T("UPDATE 'DataSources::ProceduralTree::Levels' SET ")
                                          _T("[LevelType]=?1, [LevelOrder]=?2, [AzimuthInitialType]=?3, [AzimuthInitialCurveSize]=?4, [AzimuthInitialCurve]=?5, [AzimuthInitialMin]=?6, [AzimuthInitialMax]=?7, [AzimuthInitialVar]=?8, [PolarInitialType]=?9, [PolarInitialCurveSize]=?10,")
                                          _T("[PolarInitialCurve]=?11, [PolarInitialMin]=?12, [PolarInitialMax]=?13, [PolarInitialVar]=?14, [AzimuthTwistType]=?15, [AzimuthTwistCurveSize]=?16, [AzimuthTwistCurve]=?17, [AzimuthTwistMin]=?18, [AzimuthTwistMax]=?19, [AzimuthTwistVar]=?20,")
                                          _T("[PolarTwistType]=?21, [PolarTwistCurveSize]=?22, [PolarTwistCurve]=?23, [PolarTwistMin]=?24, [PolarTwistMax]=?25, [PolarTwistVar]=?26, [GravityType]=?27, [GravityCurveSize]=?28, [GravityCurve]=?29, [GravityMin]=?30, [GravityMax]=?31, [GravityVar]=?32,")
                                          _T("[GravityProfileType]=?33, [GravityProfileCurveSize]=?34, [GravityProfileCurve]=?35, [GravityProfileMin]=?36, [GravityProfileMax]=?37, [GravityProfileVar]=?38, [FlexibilityType]=?39, [FlexibilityCurveSize]=?40, [FlexibilityCurve]=?41, [FlexibilityMin]=?42,")
                                          _T("[FlexibilityMax]=?43, [FlexibilityVar]=?44, [FlexibilityProfileType]=?45, [FlexibilityProfileCurveSize]=?46, [FlexibilityProfileCurve]=?47, [FlexibilityProfileMin]=?48, [FlexibilityProfileMax]=?49, [FlexibilityProfileVar]=?50, [LengthType]=?51,")
                                          _T("[LengthCurveSize]=?52, [LengthCurve]=?53, [LengthMin]=?54, [LengthMax]=?55, [LengthVar]=?56, [RadiusType]=?57, [RadiusCurveSize]=?58, [RadiusCurve]=?59, [RadiusMin]=?60, [RadiusMax]=?61, [RadiusVar]=?62, [RadiusProfileType]=?63, [RadiusProfileCurveSize]=?64,")
                                          _T("[RadiusProfileCurve]=?65, [RadiusProfileMin]=?66, [RadiusProfileMax]=?67, [RadiusProfileVar]=?68, [SegmentVerticesProfileType]=?69, [SegmentVerticesProfileCurveSize]=?70, [SegmentVerticesProfileCurve]=?71, [SegmentVerticesProfileMin]=?72,")
                                          _T("[SegmentVerticesProfileMax]=?73, [SegmentVerticesProfileVar]=?74, [RoughnessProfileType]=?75, [RoughnessProfileCurveSize]=?76, [RoughnessProfileCurve]=?77, [RoughnessProfileMin]=?78, [RoughnessProfileMax]=?79, [RoughnessProfileVar]=?80,")
                                          _T("[RoughnessGnarlProfileType]=?81, [RoughnessGnarlProfileCurveSize]=?82, [RoughnessGnarlProfileCurve]=?83, [RoughnessGnarlProfileMin]=?84, [RoughnessGnarlProfileMax]=?85, [RoughnessGnarlProfileVar]=?86, [FrequencyProfileType]=?87,")
                                          _T("[FrequencyProfileCurveSize]=?88, [FrequencyProfileCurve]=?89, [FrequencyProfileMin]=?90, [FrequencyProfileMax]=?91, [FrequencyProfileVar]=?92, [Segments]=?93, [SegmentVertices]=?94, [SegmentPack]=?95, [RoughnessValue]=?96, [RoughnessVariance]=?97,")
                                          _T("[RoughnessFrequencyU]=?98, [RoughnessFrequencyV]=?99, [RoughnessGnarl]=?100, [LevelBegin]=?101, [LevelEnd]=?102, [PruneDistance]=?103, [Frequency]=?104, [RoughnessGnarlUnison]=?105, [PruneDepth]=?106, [EnableForkPrune]=?107, [ForkBias]=?108, [ForkAngle]=?109,")
                                          _T("[ForkLimit]=?110, [SegmentLengthKeep]=?111, [SegmentVerticesKeep]=?112, [MaterialId]=?113 WHERE LevelId=?114"), true );
        if ( !mDeleteLevel.isPrepared( mWorld ) )
            mDeleteLevel.prepare( mWorld, _T("DELETE FROM 'DataSources::ProceduralTree::Levels' WHERE LevelId=?1"), true );
        if ( !mDeleteLevels.isPrepared( mWorld ) )
            mDeleteLevels.prepare( mWorld, _T("DELETE FROM 'DataSources::ProceduralTree::Levels' WHERE DataSourceId=?1"), true );
        if ( !mInsertFrond.isPrepared( mWorld ) )
            mInsertFrond.prepare( mWorld, _T("INSERT INTO 'DataSources::ProceduralTree::Fronds' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21,?22)"), true );
        if ( !mUpdateFrond.isPrepared( mWorld ) )
            mUpdateFrond.prepare( mWorld, _T("UPDATE 'DataSources::ProceduralTree::Fronds' SET ")
                                          _T("[Enabled]=?1, [InitialLevel]=?2, [MinimumDistance]=?3, [Depth]=?4, [AboveCondition]=?5, [BelowCondition]=?6, [SizeFactor]=?7, [MinimumOffsetAngle]=?8, [MaximumOffsetAngle]=?9, [FrondType]=?10, [ExtrusionProfileType]=?11, [ExtrusionProfileCurveSize]=?12,")
                                          _T("[ExtrusionProfileCurve]=?13, [ExtrusionProfileMin]=?14, [ExtrusionProfileMax]=?15, [ExtrusionProfileVar]=?16, [ExtrusionSegments]=?17, [Blades]=?18, [LengthSegmentOverride]=?19, [LengthSegments]=?20, [MaterialId]=?21 ")
                                          _T("WHERE [FrondId]=?22"), true );
    } // End if sandbox

    // Read queries
    if ( !mLoadGenerator.isPrepared( mWorld ) )
        mLoadGenerator.prepare( mWorld, _T("SELECT * FROM 'DataSources::ProceduralTree' WHERE RefId=?1"), true );
    if ( !mLoadLevels.isPrepared( mWorld ) )
        mLoadLevels.prepare( mWorld, _T("SELECT * FROM 'DataSources::ProceduralTree::Levels' WHERE DataSourceId=?1 ORDER BY LevelType ASC, LevelOrder ASC"), true );
    if ( !mLoadFronds.isPrepared( mWorld ) )
        mLoadFronds.prepare( mWorld, _T("SELECT * FROM 'DataSources::ProceduralTree::Fronds' WHERE DataSourceId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : deserializeLevel ( ) (Protected)
/// <summary>
/// Load the growth level data referenced by the specified query, and place
/// the results into the provided data structure.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeGenerator::deserializeLevel( cgWorldQuery & levelQuery, bool cloning, cgProceduralTreeGrowthLevel & levelOut )
{
    cgUInt32 databaseId;
    levelQuery.getColumn( _T("LevelId"), databaseId );

    // Grab splines
    deserializeSpline( levelQuery, levelOut.azimuthInitial, _T("AzimuthInitial") );
    deserializeSpline( levelQuery, levelOut.polarInitial, _T("PolarInitial") );
    deserializeSpline( levelQuery, levelOut.azimuthTwist, _T("AzimuthTwist") );
    deserializeSpline( levelQuery, levelOut.polarTwist, _T("PolarTwist") );
    deserializeSpline( levelQuery, levelOut.gravity, _T("Gravity") );
    deserializeSpline( levelQuery, levelOut.gravityProfile, _T("GravityProfile") );
    deserializeSpline( levelQuery, levelOut.flexibility, _T("Flexibility") );
    deserializeSpline( levelQuery, levelOut.flexibilityProfile, _T("FlexibilityProfile") );
    deserializeSpline( levelQuery, levelOut.length, _T("Length") );
    deserializeSpline( levelQuery, levelOut.radius, _T("Radius") );
    deserializeSpline( levelQuery, levelOut.radiusProfile, _T("RadiusProfile") );
    deserializeSpline( levelQuery, levelOut.segmentVerticesProfile, _T("SegmentVerticesProfile") );
    deserializeSpline( levelQuery, levelOut.roughnessProfile, _T("RoughnessProfile") );
    deserializeSpline( levelQuery, levelOut.roughnessGnarlProfile, _T("RoughnessGnarlProfile") );
    deserializeSpline( levelQuery, levelOut.frequencyProfile, _T("FrequencyProfile") );

    // Grab basic parameters
    cgUInt32 materialId;
    levelQuery.getColumn( _T("Segments"), levelOut.segments );
    levelQuery.getColumn( _T("SegmentVertices"), levelOut.segmentVertices );
    levelQuery.getColumn( _T("SegmentPack"), levelOut.segmentPack );
    levelQuery.getColumn( _T("RoughnessValue"), levelOut.roughnessValue );
    levelQuery.getColumn( _T("RoughnessVariance"), levelOut.roughnessVariance );
    levelQuery.getColumn( _T("RoughnessFrequencyU"), levelOut.roughnessFrequencyU );
    levelQuery.getColumn( _T("RoughnessFrequencyV"), levelOut.roughnessFrequencyV );
    levelQuery.getColumn( _T("RoughnessGnarl"), levelOut.roughnessGnarl );
    levelQuery.getColumn( _T("LevelBegin"), levelOut.levelBegin );
    levelQuery.getColumn( _T("LevelEnd"), levelOut.levelEnd );
    levelQuery.getColumn( _T("PruneDistance"), levelOut.pruneDistance );
    levelQuery.getColumn( _T("Frequency"), levelOut.frequency );
    levelQuery.getColumn( _T("RoughnessGnarlUnison"), levelOut.roughnessGnarlUnison );
    levelQuery.getColumn( _T("PruneDepth"), levelOut.pruneDepth );
    levelQuery.getColumn( _T("EnableForkPrune"), levelOut.enableForkPrune );
    levelQuery.getColumn( _T("ForkBias"), levelOut.forkBias );
    levelQuery.getColumn( _T("ForkAngle"), levelOut.forkAngle );
    levelQuery.getColumn( _T("ForkLimit"), levelOut.forkLimit );
    levelQuery.getColumn( _T("SegmentLengthKeep"), levelOut.segmentLengthKeep );
    levelQuery.getColumn( _T("SegmentVerticesKeep"), levelOut.segmentVerticesKeep );
    levelQuery.getColumn( _T("MaterialId"), materialId );

    // Load material as necessary.
    if ( materialId )
        mWorld->getResourceManager()->loadMaterial( &levelOut.material, mWorld, cgMaterialType::Standard, materialId, false, 0, cgDebugSource() );

    // Only associate with the original database row identifier if we were
    // not instructed to clone data. By ensuring that the 'databaseId'
    // is '0', this will force the insertion of a new entry on next serialize.
    if ( cloning )
        levelOut.mDatabaseId = 0;
    else
        levelOut.mDatabaseId = databaseId;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : serializeLevel ( ) (Protected)
/// <summary>
/// Insert (or update) the provided growth level data into the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeGenerator::serializeLevel( cgProceduralTreeGrowthLevel & level, cgUInt32 parentId, cgWorld * world, cgUInt32 levelType, cgUInt32 levelOrder )
{
    // Update or insert?
    if ( level.mDatabaseId )
    {
        // Level descriptor.
        mUpdateLevel.bindParameter( 1, levelType );
        mUpdateLevel.bindParameter( 2, levelOrder );

        // Splines
        serializeSpline( mUpdateLevel, level.azimuthInitial, 3 );
        serializeSpline( mUpdateLevel, level.polarInitial, 9 );
        serializeSpline( mUpdateLevel, level.azimuthTwist, 15 );
        serializeSpline( mUpdateLevel, level.polarTwist, 21 );
        serializeSpline( mUpdateLevel, level.gravity, 27 );
        serializeSpline( mUpdateLevel, level.gravityProfile, 33 );
        serializeSpline( mUpdateLevel, level.flexibility, 39 );
        serializeSpline( mUpdateLevel, level.flexibilityProfile, 45 );
        serializeSpline( mUpdateLevel, level.length, 51 );
        serializeSpline( mUpdateLevel, level.radius, 57 );
        serializeSpline( mUpdateLevel, level.radiusProfile, 63 );
        serializeSpline( mUpdateLevel, level.segmentVerticesProfile, 69 );
        serializeSpline( mUpdateLevel, level.roughnessProfile, 75 );
        serializeSpline( mUpdateLevel, level.roughnessGnarlProfile, 81 );
        serializeSpline( mUpdateLevel, level.frequencyProfile, 87 );

        // Standard data.
        mUpdateLevel.bindParameter( 93, level.segments );
        mUpdateLevel.bindParameter( 94, level.segmentVertices );
        mUpdateLevel.bindParameter( 95, level.segmentPack );
        mUpdateLevel.bindParameter( 96, level.roughnessValue );
        mUpdateLevel.bindParameter( 97, level.roughnessVariance );
        mUpdateLevel.bindParameter( 98, level.roughnessFrequencyU );
        mUpdateLevel.bindParameter( 99, level.roughnessFrequencyV );
        mUpdateLevel.bindParameter( 100, level.roughnessGnarl );
        mUpdateLevel.bindParameter( 101, level.levelBegin );
        mUpdateLevel.bindParameter( 102, level.levelEnd );
        mUpdateLevel.bindParameter( 103, level.pruneDistance );
        mUpdateLevel.bindParameter( 104, level.frequency );
        mUpdateLevel.bindParameter( 105, level.roughnessGnarlUnison );
        mUpdateLevel.bindParameter( 106, level.pruneDepth );
        mUpdateLevel.bindParameter( 107, level.enableForkPrune );
        mUpdateLevel.bindParameter( 108, level.forkBias );
        mUpdateLevel.bindParameter( 109, level.forkAngle );
        mUpdateLevel.bindParameter( 110, level.forkLimit );
        mUpdateLevel.bindParameter( 111, level.segmentLengthKeep );
        mUpdateLevel.bindParameter( 112, level.segmentVerticesKeep );
        mUpdateLevel.bindParameter( 113, level.material.getReferenceId() );

        // Where
        mUpdateLevel.bindParameter( 114, level.mDatabaseId );

        // Execute
        if ( !mUpdateLevel.step( true ) )
        {
            cgString error;
            mUpdateLevel.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update parameter data for procedural tree growth level '0x%x'. Error: %s\n"), level.mDatabaseId, error.c_str() );
            return false;
        
        } // End if failed

    } // End if update
    else
    {
        // Level descriptor.
        mInsertLevel.bindParameter( 1, parentId );
        mInsertLevel.bindParameter( 2, levelType );
        mInsertLevel.bindParameter( 3, levelOrder );

        // Splines
        serializeSpline( mInsertLevel, level.azimuthInitial, 4 );
        serializeSpline( mInsertLevel, level.polarInitial, 10 );
        serializeSpline( mInsertLevel, level.azimuthTwist, 16 );
        serializeSpline( mInsertLevel, level.polarTwist, 22 );
        serializeSpline( mInsertLevel, level.gravity, 28 );
        serializeSpline( mInsertLevel, level.gravityProfile, 34 );
        serializeSpline( mInsertLevel, level.flexibility, 40 );
        serializeSpline( mInsertLevel, level.flexibilityProfile, 46 );
        serializeSpline( mInsertLevel, level.length, 52 );
        serializeSpline( mInsertLevel, level.radius, 58 );
        serializeSpline( mInsertLevel, level.radiusProfile, 64 );
        serializeSpline( mInsertLevel, level.segmentVerticesProfile, 70 );
        serializeSpline( mInsertLevel, level.roughnessProfile, 76 );
        serializeSpline( mInsertLevel, level.roughnessGnarlProfile, 82 );
        serializeSpline( mInsertLevel, level.frequencyProfile, 88 );

        // Standard data.
        mInsertLevel.bindParameter( 94, level.segments );
        mInsertLevel.bindParameter( 95, level.segmentVertices );
        mInsertLevel.bindParameter( 96, level.segmentPack );
        mInsertLevel.bindParameter( 97, level.roughnessValue );
        mInsertLevel.bindParameter( 98, level.roughnessVariance );
        mInsertLevel.bindParameter( 99, level.roughnessFrequencyU );
        mInsertLevel.bindParameter( 100, level.roughnessFrequencyV );
        mInsertLevel.bindParameter( 101, level.roughnessGnarl );
        mInsertLevel.bindParameter( 102, level.levelBegin );
        mInsertLevel.bindParameter( 103, level.levelEnd );
        mInsertLevel.bindParameter( 104, level.pruneDistance );
        mInsertLevel.bindParameter( 105, level.frequency );
        mInsertLevel.bindParameter( 106, level.roughnessGnarlUnison );
        mInsertLevel.bindParameter( 107, level.pruneDepth );
        mInsertLevel.bindParameter( 108, level.enableForkPrune );
        mInsertLevel.bindParameter( 109, level.forkBias );
        mInsertLevel.bindParameter( 110, level.forkAngle );
        mInsertLevel.bindParameter( 111, level.forkLimit );
        mInsertLevel.bindParameter( 112, level.segmentLengthKeep );
        mInsertLevel.bindParameter( 113, level.segmentVerticesKeep );
        mInsertLevel.bindParameter( 114, level.material.getReferenceId() );

        // Execute
        if ( !mInsertLevel.step( true ) )
        {
            cgString error;
            mInsertLevel.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new procedural tree growth level data for data source '0x%x'. Error: %s\n"), parentId, error.c_str() );
            return false;
        
        } // End if failed

        // Retrieve the new database record identifier for
        // later alterations.
        level.mDatabaseId = mInsertLevel.getLastInsertId();

    } // End if insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : deleteLevel ( ) (Protected)
/// <summary>
/// Delete the specified growth level data from the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeGenerator::deleteLevel( cgProceduralTreeGrowthLevel & level )
{
    // Do nothing if not in the database.
    if ( !level.mDatabaseId )
        return true;

    // Delete the level from the database.
    mDeleteLevel.bindParameter( 1, level.mDatabaseId );
    if ( !mDeleteLevel.step( true ) )
    {
        cgString error;
        mDeleteLevel.getLastError( error );
        cgAppLog::write( cgAppLog::Error, _T("Failed to delete parameter data for procedural tree growth level '0x%x'. Error: %s\n"), level.mDatabaseId, error.c_str() );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : deserializeSpline ( ) (Protected)
/// <summary>
/// Retrieve the spline data from the specified query object that is identified 
/// by the supplied prefix.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeGenerator::deserializeSpline( cgWorldQuery & levelQuery, cgBezierSpline2 & splineOut, const cgString & prefix )
{
    cgUInt32 desc;
    levelQuery.getColumn( prefix + _T("Type"), desc );
    if ( desc == cgBezierSpline2::Custom )
    {
        cgUInt32 pointCount, size;
        cgBezierSpline2::SplinePoint * pointData = CG_NULL;
        levelQuery.getColumn( prefix + _T("CurveSize"), pointCount );
        levelQuery.getColumn( prefix + _T("Curve"), (void**)&pointData, size );
        if ( pointData )
        {
            splineOut.clear();
            for ( cgUInt32 point = 0; point < pointCount; ++point )
                splineOut.addPoint( pointData[point] );

        } // End if valid point data

    } // End if custom
    else
    {
        splineOut.setDescription( (cgBezierSpline2::SplineDescription)desc );

    } // End if described

    // Get range / variance data.
    cgRangeF range;
    cgFloat  variance;
    levelQuery.getColumn( prefix + _T("Min"), range.min );
    levelQuery.getColumn( prefix + _T("Max"), range.max );
    levelQuery.getColumn( prefix + _T("Var"), variance );
    splineOut.setRange( range );
    splineOut.setVariance( variance );
}

//-----------------------------------------------------------------------------
// Name : serializeSpline ( ) (Protected)
/// <summary>
/// Bind the data from the specified spline to the referenced query object
/// starting at the indicated parameter offset.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeGenerator::serializeSpline( cgWorldQuery & levelQuery, const cgBezierSpline2 & spline, cgUInt16 paramOffset )
{
    // Get spline data.
    cgUInt32 desc     = spline.getDescription();
    cgRangeF range    = spline.getRange();
    cgFloat  variance = spline.getVariance();

    // Output spline data.
    levelQuery.bindParameter( paramOffset, (cgUInt32)desc );
    if ( desc == cgBezierSpline2::Custom )
    {
        levelQuery.bindParameter( paramOffset + 1, spline.getPointCount() );
        levelQuery.bindParameter( paramOffset + 2, &spline.getPoints()[0], spline.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
    
    } // End if custom
    else
    {
        levelQuery.bindParameter( paramOffset + 1, 0 );
        levelQuery.bindParameter( paramOffset + 2, CG_NULL, 0 );
    
    } // End if described
    levelQuery.bindParameter( paramOffset + 3, range.min );
    levelQuery.bindParameter( paramOffset + 4, range.max );
    levelQuery.bindParameter( paramOffset + 5, variance );
}

//-----------------------------------------------------------------------------
// Name : deserializeFrond ( ) (Protected)
/// <summary>
/// Load the frond data referenced by the specified query, and place
/// the results into the provided data structure.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeGenerator::deserializeFrond( cgWorldQuery & frondQuery, bool cloning, cgProceduralTreeFrondDesc & frondOut )
{
    cgUInt32 databaseId;
    frondQuery.getColumn( _T("FrondId"), databaseId );

    cgUInt32 materialId, aboveCondition, belowCondition, frondType;
    frondQuery.getColumn( _T("Enabled"), frondOut.enabled );
    frondQuery.getColumn( _T("InitialLevel"), frondOut.initialLevel );
    frondQuery.getColumn( _T("MinimumDistance"), frondOut.minimumDistance );
    frondQuery.getColumn( _T("Depth"), frondOut.depth );
    frondQuery.getColumn( _T("AboveCondition"), aboveCondition );
    frondQuery.getColumn( _T("BelowCondition"), belowCondition );
    frondQuery.getColumn( _T("SizeFactor"), frondOut.sizeFactor );
    frondQuery.getColumn( _T("MinimumOffsetAngle"), frondOut.minimumOffsetAngle );
    frondQuery.getColumn( _T("MaximumOffsetAngle"), frondOut.maximumOffsetAngle );
    frondQuery.getColumn( _T("FrondType"), frondType );
    deserializeSpline( frondQuery, frondOut.extrusionProfile, _T("ExtrusionProfile") );
    frondQuery.getColumn( _T("ExtrusionSegments"), frondOut.extrusionSegments );
    frondQuery.getColumn( _T("Blades"), frondOut.blades );
    frondQuery.getColumn( _T("LengthSegmentOverride"), frondOut.lengthSegmentOverride );
    frondQuery.getColumn( _T("LengthSegments"), frondOut.lengthSegments );
    frondQuery.getColumn( _T("MaterialId"), materialId );

    // Set enumerations.
    frondOut.aboveCondition = (cgProceduralTreeGrowthCondition::Base)aboveCondition;
    frondOut.belowCondition = (cgProceduralTreeGrowthCondition::Base)belowCondition;
    frondOut.type           = (cgProceduralTreeFrondType::Base)frondType;
    
    // Load material as necessary.
    if ( materialId )
        mWorld->getResourceManager()->loadMaterial( &frondOut.material, mWorld, cgMaterialType::Standard, materialId, false, 0, cgDebugSource() );

    // Only associate with the original database row identifier if we were
    // not instructed to clone data. By ensuring that the 'databaseId'
    // is '0', this will force the insertion of a new entry on next serialize.
    if ( cloning )
        frondOut.mDatabaseId = 0;
    else
        frondOut.mDatabaseId = databaseId;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : serializeFrond ( ) (Protected)
/// <summary>
/// Insert (or update) the provided frond data into the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeGenerator::serializeFrond( cgProceduralTreeFrondDesc & frond, cgUInt32 parentId, cgWorld * world )
{
    // Update or insert?
    if ( frond.mDatabaseId )
    {
        // Level descriptor.
        mUpdateFrond.bindParameter( 1, frond.enabled );
        mUpdateFrond.bindParameter( 2, frond.initialLevel );
        mUpdateFrond.bindParameter( 3, frond.minimumDistance );
        mUpdateFrond.bindParameter( 4, frond.depth );
        mUpdateFrond.bindParameter( 5, (cgUInt16)frond.aboveCondition );
        mUpdateFrond.bindParameter( 6, (cgUInt16)frond.belowCondition );
        mUpdateFrond.bindParameter( 7, frond.sizeFactor );
        mUpdateFrond.bindParameter( 8, frond.minimumOffsetAngle );
        mUpdateFrond.bindParameter( 9, frond.maximumOffsetAngle );
        mUpdateFrond.bindParameter( 10, (cgUInt16)frond.type );
        serializeSpline( mUpdateFrond, frond.extrusionProfile, 11 );
        mUpdateFrond.bindParameter( 17, frond.extrusionSegments );
        mUpdateFrond.bindParameter( 18, frond.blades );
        mUpdateFrond.bindParameter( 19, frond.lengthSegmentOverride );
        mUpdateFrond.bindParameter( 20, frond.lengthSegments );
        mUpdateFrond.bindParameter( 21, frond.material.getReferenceId() );

        // Where
        mUpdateFrond.bindParameter( 22, frond.mDatabaseId );

        // Execute
        if ( !mUpdateFrond.step( true ) )
        {
            cgString error;
            mUpdateFrond.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update parameter data for procedural tree frond description '0x%x'. Error: %s\n"), frond.mDatabaseId, error.c_str() );
            return false;
        
        } // End if failed

    } // End if update
    else
    {
        // Level descriptor.
        mInsertFrond.bindParameter( 1, parentId );
        mInsertFrond.bindParameter( 2, frond.enabled );
        mInsertFrond.bindParameter( 3, frond.initialLevel );
        mInsertFrond.bindParameter( 4, frond.minimumDistance );
        mInsertFrond.bindParameter( 5, frond.depth );
        mInsertFrond.bindParameter( 6, (cgUInt16)frond.aboveCondition );
        mInsertFrond.bindParameter( 7, (cgUInt16)frond.belowCondition );
        mInsertFrond.bindParameter( 8, frond.sizeFactor );
        mInsertFrond.bindParameter( 9, frond.minimumOffsetAngle );
        mInsertFrond.bindParameter( 10, frond.maximumOffsetAngle );
        mInsertFrond.bindParameter( 11, (cgUInt16)frond.type );
        serializeSpline( mInsertFrond, frond.extrusionProfile, 12 );
        mInsertFrond.bindParameter( 18, frond.extrusionSegments );
        mInsertFrond.bindParameter( 19, frond.blades );
        mInsertFrond.bindParameter( 20, frond.lengthSegmentOverride );
        mInsertFrond.bindParameter( 21, frond.lengthSegments );
        mInsertFrond.bindParameter( 22, frond.material.getReferenceId() );

        // Execute
        if ( !mInsertFrond.step( true ) )
        {
            cgString error;
            mInsertFrond.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new procedural tree frond description for data source '0x%x'. Error: %s\n"), parentId, error.c_str() );
            return false;
        
        } // End if failed

        // Retrieve the new database record identifier for
        // later alterations.
        frond.mDatabaseId = mInsertFrond.getLastInsertId();

    } // End if insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : getParameters ( )
/// <summary>
/// Retrieve the growth properties that will be used in any subsequent call
/// to the 'generate()' method.
/// </summary>
//-----------------------------------------------------------------------------
const cgProceduralTreeGrowthProperties & cgProceduralTreeGenerator::getParameters() const
{
    return mParams;
}

//-----------------------------------------------------------------------------
// Name : setParameters ( )
/// <summary>
/// Set the growth properties that will be used in any subsequent call
/// to the 'generate()' method.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeGenerator::setParameters( const cgProceduralTreeGrowthProperties & parameters )
{
    bool transactionBegun = false;
    if ( shouldSerialize() )
    {
        cgProceduralTreeGrowthProperties newParams = parameters;

        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("ProceduralTreeGenerator::setParameters") );
        transactionBegun = true;

        prepareQueries();
        mUpdateGenerator.bindParameter( 1, (cgUInt32)0 );   // Flags
        mUpdateGenerator.bindParameter( 2, newParams.mainSeed );
        mUpdateGenerator.bindParameter( 3, newParams.flareSeed );
        mUpdateGenerator.bindParameter( 4, newParams.globalScale );
        mUpdateGenerator.bindParameter( 5, newParams.globalScaleVariance );
        mUpdateGenerator.bindParameter( 6, newParams.globalTextureUScale );
        mUpdateGenerator.bindParameter( 7, newParams.globalTextureVScale );
        mUpdateGenerator.bindParameter( 8, mReferenceId );

        // Execute
        if ( !mUpdateGenerator.step( true ) )
        {
            cgString error;
            mUpdateGenerator.getLastError( error );
            mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::setParameters") );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update parameter data for procedural tree data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed

        // Update trunk data.
        newParams.trunkData.mDatabaseId = mParams.trunkData.mDatabaseId;
        if ( !serializeLevel( newParams.trunkData, mReferenceId, mWorld, 0, 0 ) )
        {
            mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::setParameters") );
            return;
        
        } // End if failed

        // Update as many branch levels as we can.
        size_t updateCount = min( newParams.branchLevels.size(), mParams.branchLevels.size() );
        for ( size_t i = 0; i < updateCount; ++i )
        {
            newParams.branchLevels[i].mDatabaseId = mParams.branchLevels[i].mDatabaseId;
            if ( !serializeLevel( newParams.branchLevels[i], mReferenceId, mWorld, 3, i ) )
            {
                mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::setParameters") );
                return;
            
            } // End if failed
        
        } // Next branch level

        // Delete or insert the remaining levels.
        if ( newParams.branchLevels.size() < mParams.branchLevels.size() )
        {
            // Delete remaining levels
            for ( size_t i = newParams.branchLevels.size(); i < mParams.branchLevels.size(); ++i )
            {
                if ( !deleteLevel( mParams.branchLevels[i] ) )
                {
                    mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::setParameters") );
                    return;
                
                } // End if failed
            
            } // Next destroyed layer
        
        } // End if fewer
        else
        {
            // Insert remaining levels.
            for ( size_t i = mParams.branchLevels.size(); i < newParams.branchLevels.size(); ++i )
            {
                newParams.branchLevels[i].mDatabaseId = 0;
                if ( !serializeLevel( newParams.branchLevels[i], mReferenceId, mWorld, 3, i ) )
                {
                    mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::setParameters") );
                    return;
                
                } // End if failed
            
            } // Next destroyed layer

        } // End if more

        // Serialize frond data.
        if ( !serializeFrond( newParams.frondData, mReferenceId, mWorld ) )
        {
            mWorld->rollbackTransaction( _T("ProceduralTreeGenerator::setParameters") );
            return;
        
        } // End if failed

        // Grab a list of the old parameter materials so we can detach later.
        cgMaterialHandleArray oldMaterials;
        oldMaterials.push_back( mParams.trunkData.material );
        oldMaterials.push_back( mParams.frondData.material );
        for ( size_t i = 0; i < mParams.branchLevels.size(); ++i )
            oldMaterials.push_back( mParams.branchLevels[i].material );

        // Replace local parameters.
        mParams = newParams;

        // Attach to new materials.
        mParams.trunkData.material.enableDatabaseUpdate( true, true );
        mParams.trunkData.material.enableDatabaseUpdate( false );
        mParams.frondData.material.enableDatabaseUpdate( true, true );
        mParams.frondData.material.enableDatabaseUpdate( false );
        for ( size_t i = 0; i < mParams.branchLevels.size(); ++i )
        {
            mParams.branchLevels[i].material.enableDatabaseUpdate( true, true );
            mParams.branchLevels[i].material.enableDatabaseUpdate( false );
        
        } // Next branch

        // Release old materials.
        for ( size_t i = 0; i < oldMaterials.size(); ++i )
        {
            oldMaterials[i].enableDatabaseUpdate( true );
            oldMaterials[i].close();
        
        } // Next material
            
        // Commit changes
        mWorld->commitTransaction( _T("ProceduralTreeGenerator::setParameters") );

    } // End if serialize
    else
    {
        // Just update local parameters.
        mParams = parameters;
    
    } // End if !serialize

    // Notify listeners that an update occurred.
    static const cgString strContext = _T("CreationParameters");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
// Name : generate ( )
/// <summary>
/// Compute the tree mesh / data based on the supplied growth parameters.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshHandle cgProceduralTreeGenerator::generate( )
{
    // Clear out any old data
    clear();

    // Enable double precision.
    bool switched = cgFPUDoublePrecision();
    
    // Grow the tree (starting with the trunk)
    growTrunk();

    // Return FPU to original precision.
    if ( switched )
        cgFPURestorePrecision();

    // Build the mesh data
    if ( !buildMesh() )
        clear();

    // Grab result and then release temporary memory.
    cgMeshHandle result = mMesh;
    clear();

    // Return new mesh result (if any).
    return result;
}

//-----------------------------------------------------------------------------
// Name : growTrunk ( ) (Protected)
/// <summary>
/// Compute the segments for trunk level and start the growth for child
/// branches.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeGenerator::growTrunk( )
{
    // Use ParkMiller random number generation (with state table)
    cgRandom::ParkMiller prng( true );

    // Seed the random number generator
    prng.setSeed( mParams.mainSeed );

    // Populating level[0] (trunk)
    cgProceduralTreeGrowthLevel & trunkLevel = mParams.trunkData;
    mLevels.resize( mParams.branchLevels.empty() ? 1 : 2 );
    mLevels[0].branches.resize(1);
    Branch & trunkBranch = mLevels[0].branches[0];

    // Compute the overall global scale of the tree. (Iteration[0])
    cgDouble rv = prng.next();
    mGlobalScale = (mParams.globalScale - mParams.globalScaleVariance) + (rv * (mParams.globalScaleVariance * 2.0f));

    // Does this branch meet our criteria to be a frond branch.
    if ( mParams.frondData.enabled && mParams.frondData.initialLevel == 0 )
    {
        if ( mParams.frondData.aboveCondition == cgProceduralTreeGrowthCondition::Enabled )
            trunkBranch.frondBranch = true;
        else if ( mParams.frondData.aboveCondition == cgProceduralTreeGrowthCondition::Pruned )
            return;
    
    } // End if fronds enabled
    
    // Compute the length of the trunk.
    trunkBranch.length  = eval( trunkLevel.length, cgBezierSpline2::NormalizePlusVariance, 0, rv );
    trunkBranch.length *= mGlobalScale;
    
    // Compute the initial azimuth angle
    cgDouble azimuthInitialAngle = eval( trunkLevel.azimuthInitial, cgBezierSpline2::NormalizePlusVariance, 0, prng.next() );

    // Compute the initial polar angle. (Iteration[2])
    cgDouble polarInitialAngle = eval( trunkLevel.polarInitial, cgBezierSpline2::NormalizePlusVariance, 0, prng.next() );

    // Compute the base gravity scalar. (Iteration[3])
    cgDouble baseGravity = eval( trunkLevel.gravity, cgBezierSpline2::NormalizePlusVariance, 0, prng.next() );

    // Next, compute the base radius for the tree. (Iteration[4])
    cgDouble baseRadius = eval( trunkLevel.radius, cgBezierSpline2::NormalizePlusVariance, 0, prng.next() );
    baseRadius = min( (cgDouble)trunkLevel.radius.getRange().max, baseRadius ) * mGlobalScale;

    // Iteration[5]
    prng.next();
    // Iteration[6]
    prng.next();

    // Generate the correct starting matrix for the trunk (-X = direction of growth)
    cgTransform currentTransform = cgTransform::Identity, rotationTransform;
    currentTransform.rotateAxis( CGEToRadian((cgFloat)azimuthInitialAngle), currentTransform.zAxis() );
    currentTransform.rotateAxis( -CGEToRadian((cgFloat)polarInitialAngle), currentTransform.yAxis() );

    // Select the correct number of segments
    cgUInt32 segmentCount = trunkLevel.segments;
    if ( trunkBranch.frondBranch && mParams.frondData.lengthSegmentOverride )
        segmentCount = mParams.frondData.lengthSegments;
    trunkBranch.segments.resize( segmentCount + 1 );

    // Generate the geometry for each segment in the trunk
    for ( cgUInt32 segment = 0; segment <= segmentCount; ++segment )
    {
        // Record the current growth axis at the start of this segment.
        // This will allow us to determine the total amount of "deviation"
        // that occurs at each segment (for branch fork support).
        cgVector3 previousGrowthAxis = -currentTransform.xAxis();

        // Compute the distance to the segment start
        cgDouble segmentBegin = pow( (cgDouble)segment / (cgDouble)segmentCount, trunkLevel.segmentPack );
        cgDouble segmentEnd   = pow( (cgDouble)(segment + 1) / (cgDouble)segmentCount, trunkLevel.segmentPack );
        cgDouble segmentEval  = segmentBegin;

        // Add the current segment start to the spline
        Segment & currentSegment = trunkBranch.segments[segment];
        if ( segment < segmentCount )
            currentSegment.length = ((segmentEnd - segmentBegin) * trunkBranch.length);
        else
            currentSegment.length = 0;

        // Store basic details
        currentSegment.segmentLocation = segmentBegin;

        // Compute the number of points in the cross section of this segment
        cgDouble verticesProfile    = eval( trunkLevel.segmentVerticesProfile, cgBezierSpline2::NormalizeOnly, segmentEval, 0 );
        verticesProfile             = min( 1.0, verticesProfile );
        verticesProfile             = max( 0.0, verticesProfile );
        cgDouble vertices           = 3.0 + ((cgDouble)(trunkLevel.segmentVertices - 3) * verticesProfile );
        currentSegment.vertices     = (cgUInt32)vertices;

        // Azimuth Twisting (Iteration[7])
        cgDouble azimuthTwist = eval( trunkLevel.azimuthTwist, cgBezierSpline2::NormalizeScaleVariance, segmentEval, prng.next() );
        
        // Polar Twisting (Iteration[8])
        cgDouble polarTwist = eval( trunkLevel.polarTwist, cgBezierSpline2::NormalizeScaleVariance, segmentEval, prng.next() );
        
        // Adjustment for segment matrix twisting comes prior to gravity for the first segment only
        if ( segment == 0 )
        {
            currentTransform.rotateAxis( CGEToRadian((cgFloat)azimuthTwist), cgVector3(0,0,1), currentTransform.position() );
            currentTransform.rotateLocal( 0, -CGEToRadian((cgFloat)polarTwist), 0 );
        
        } // End if first segment

        // Compute the final radius of this segment (Iteration[9])
        currentSegment.radius = baseRadius * eval( trunkLevel.radiusProfile, cgBezierSpline2::NormalizePlusVariance, segmentEval, prng.next() );
        if ( currentSegment.radius > baseRadius )
            currentSegment.radius = baseRadius;
        if ( currentSegment.radius < 0.0 )
            currentSegment.radius = 0.0;
        
        // Iteration[10]
        prng.next();

        // Compute the final gravity twisting for this segment (Iteration[11])
        // Twisting due to gravity (angle of difference) = (90 - (pow(SegmentAngle,2) / 90));
        cgVector3 gravity( 0, 0, -1 );
        cgDouble gravityAdjust;
        cgDouble angleForGravity = CGEToDegree( asin( (cgDouble)cgVector3::dot(gravity, -currentTransform.xAxis() ) ) );
        cgDouble gravityProfile = (eval( trunkLevel.gravityProfile, cgBezierSpline2::NormalizePlusVariance, segmentEval, prng.next() ) - 0.5) * 2.0;
        if ( angleForGravity < 0.0 )
            gravityAdjust = (90 - (pow(angleForGravity,2) / 90)) * baseGravity * gravityProfile;
        else
            gravityAdjust = (90 + (-2 * angleForGravity) + (pow(angleForGravity,2) / 90)) * baseGravity * gravityProfile;

        // Generate axis about which to rotate in order to apply gravity
        if ( abs(currentTransform.xAxis().z) < 0.9990 )
        {
            cgVector3 rotateAxis;
            cgVector3::normalize( rotateAxis, *cgVector3::cross( rotateAxis, gravity, -currentTransform.xAxis() ) );
            currentTransform.rotateAxis( CGEToRadian((cgFloat)gravityAdjust), rotateAxis, currentTransform.position() );
        
        } // End if apply gravity adjustment

        // Adjustment for segment matrix twisting comes after gravity for
        // all segments except the first.
        if ( segment != 0 )
        {
            currentTransform.rotateLocal( 0, 0, CGEToRadian((cgFloat)azimuthTwist) );
            currentTransform.rotateLocal( 0, -CGEToRadian((cgFloat)polarTwist), 0 );
        
        } // End if not first

        // Record the total amount of deviation that occured between the
        // prior and current segments.
        currentSegment.deviation = CGEToDegree( acos( (cgDouble)cgVector3::dot(previousGrowthAxis, -currentTransform.xAxis()) ) );

        // Store the current growth matrix in this segment. This will allow us
        // to understand how the tree was grown at a later time in order to
        // correctly attach child branches.
        currentSegment.transform = currentTransform;
        
        // Push transform origin for start of next segment.
        currentTransform.setPosition( currentTransform.position() - (currentTransform.xAxis() * (cgFloat)currentSegment.length) );

        // First segment has three additional RNG queries
        if ( segment == 0 )
        {
            prng.next();
            prng.next();
            prng.next();
        
        } // End if first segment
        
    } // Next trunk segment

    // Reset the random number generator for growth probability (frequency) determination.
    prng.setSeed( mParams.mainSeed );

    // Grow branches from the trunk
    if ( !mParams.branchLevels.empty() )
    {
        // How many branches should we attempt to generate?
        cgProceduralTreeGrowthLevel & branchLevel = mParams.branchLevels[0];
        cgInt32 branchCount = (cgInt32)(branchLevel.frequency * (trunkBranch.length / mGlobalScale));
        if ( branchCount < 0 )
            branchCount = 0;

        // Generate branches
        for ( cgInt32 branch = 0; branch < branchCount; ++branch )
        {
            Branch & trunkBranch = mLevels[0].branches[0];
            Level & branchOutLevel = mLevels[1];
            cgInt32 branchIndex = branchOutLevel.branches.size();
            trunkBranch.childBranches.push_back( branchIndex );
            branchOutLevel.branches.push_back( Branch() );

            // Grow the next branch (WARNING, Existing references to 'mLevels' elements may become invalid here)
            if ( !growBranch( prng.next(), branchLevel, 1, branchIndex, branch, 0, 0 ) )
            {
                // Branch rejected
                Branch & trunkBranch = mLevels[0].branches[0];
                Level & branchOutLevel = mLevels[1];
                trunkBranch.childBranches.pop_back();
                branchOutLevel.branches.pop_back();

            } // End if rejected

        } // Next Branch
        
    } // End if branches
}

//-----------------------------------------------------------------------------
// Name : growBranch ( ) (Protected)
/// <summary>
/// Compute the segments for specified branch level and start the growth for
/// child branches.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeGenerator::growBranch( cgDouble growthRand, cgProceduralTreeGrowthLevel & branchLevel, cgInt32 outBranchLevel, cgInt32 outBranchIndex, cgInt32 branchAttempt, cgInt32 parentBranchIndex, cgInt32 parentAttempt )
{
    // Use ParkMiller random number generation (with state table)
    cgRandom::ParkMiller prng( true );
    
    // Seed the random number generator
    cgInt32 branchSeed = mParams.mainSeed + (3 * (branchAttempt+outBranchLevel+parentAttempt));
    prng.setSeed( (cgUInt32)branchSeed );

    // Setup output data container.
    Branch & branchOut      = mLevels[outBranchLevel].branches[ outBranchIndex ];
    branchOut.level         = outBranchLevel;
    branchOut.parentBranch  = parentBranchIndex;
    
    // Compute the starting position along the parent branch
    Branch & parentBranch = mLevels[ outBranchLevel - 1 ].branches[ parentBranchIndex ];
    if ( branchAttempt == 0 )
    {
        // First branch is always positioned at 85%-95% of the length of the
        // parent branch (adjusted by the level start and end percentages).
        branchOut.branchLocation = prng.next( 0.85, 0.95 );

    } // End if first branch
    else
    {
        // Other branches can begin anywhere along the parent.
        branchOut.branchLocation = prng.next();

    } // End if other branch

    // The branch location prior to scaling into level population
    // begin / end ranges is used as the spline evaluation value.
    cgDouble segmentEval = branchOut.branchLocation;

    // Scale branch location into appropriate range to describe its
    // true location along the parent branch.
    branchOut.branchLocation = branchLevel.levelBegin + ((branchLevel.levelEnd - branchLevel.levelBegin) * branchOut.branchLocation );

    // Reject the branch if the probability falls below the provided random value.
    cgDouble growthProbability = branchLevel.frequencyProfile.evaluateForX( (cgFloat)segmentEval, false, 4 );
    if ( growthProbability < growthRand )
        return false;

    // If fork pruning is enabled, check to see if this is a branch to which
    // the fork tests should be applied. Note, this second random number
    // is also used as the random variance value for the length calculation.
    branchOut.forkBranch = false;
    cgDouble rv = prng.next();
    if ( rv < branchLevel.forkBias )
        branchOut.forkBranch = branchLevel.enableForkPrune;

    // Compute the starting matrix at the correct location along the parent branch.
    cgTransform currentTransform = cgTransform::Identity;
    cgDouble branchBeginLength = parentBranch.length * branchOut.branchLocation;
    cgDouble maxRadius = 0.0, delta, currentLength = 0.0;
    cgDouble segmentDeviation;
    for ( cgInt32 segment = 0; segment < (cgInt32)parentBranch.segments.size() - 1; ++segment )
    {
        Segment & prevSegment    = parentBranch.segments[ max(0, segment-1) ];
        Segment & currentSegment = parentBranch.segments[ segment ];
        Segment & nextSegment    = parentBranch.segments[ segment + 1 ];
        if ( branchBeginLength <= currentLength + currentSegment.length )
        {
            // Determine position along segment.
            delta = (branchBeginLength - currentLength) / currentSegment.length;

            // Fork branches are snapped to the closest segment.
            if ( branchOut.forkBranch == true )
            {
                if ( delta > 0.5 )
                {
                    // If this means it will be pushed to the tip, discard the branch.
                    if ( segment == (cgInt32)parentBranch.segments.size() - 2 )
                        return false;

                    // If we have reached our fork branch limit at this segment,
                    // reject this branch.
                    if ( nextSegment.forkBranches == branchLevel.forkLimit )
                        return false;

                    // Snap to end of this segment
                    currentTransform = nextSegment.transform;
                    delta            = 1.0;
                    segmentDeviation = nextSegment.deviation;
                    branchOut.branchLocation = currentLength + currentSegment.length;
                    nextSegment.forkBranches++;
                
                } // End if snap to end
                else
                {
                    // If we have reached our fork branch limit at this segment,
                    // reject this branch.
                    if ( currentSegment.forkBranches == branchLevel.forkLimit )
                        return false;

                    // Snap to begining of this segment
                    currentTransform = currentSegment.transform;
                    delta            = 0;
                    segmentDeviation = currentSegment.deviation;
                    branchOut.branchLocation = currentLength;
                    currentSegment.forkBranches++;

                } // End if snap to start

            } // End if fork
            else
            {
                // Standard branch
                currentTransform = currentSegment.transform;
                currentTransform.setPosition( currentTransform.position() - (currentTransform.xAxis() * (cgFloat)(branchBeginLength - currentLength)) );
            
            } // End if !fork

            // Compute the maximum allowed radius of this branch
            maxRadius = currentSegment.radius + ((nextSegment.radius - currentSegment.radius) * delta);
            break;

        } // End if within this segment
        currentLength += currentSegment.length;

    } // Next Segment

    // Pruned based on distance along specified parent branch. First select the 
    // relevant branch, at the specified depth, that we will use for testing.
    cgUInt32 testLevel   = outBranchLevel;
    cgUInt32 testBranch  = outBranchIndex;
    for ( cgUInt32 i = 0; i < branchLevel.pruneDepth; ++i )
    {
        // Select next level up
        testBranch = mLevels[testLevel].branches[testBranch].parentBranch;
        testLevel  = testLevel - 1;

    } // Next Parent Level

    // If the starting location of the branch to be tested falls below the 
    // specified distance threshold, then cull this branch.
    if ( branchLevel.pruneDistance > 1e-4 )
    {
        if ( mLevels[ testLevel ].branches[ testBranch ].branchLocation < branchLevel.pruneDistance )
            return false;
    
    } // End if positive
    else if ( branchLevel.pruneDistance < -1e-4 )
    {
        // Negative distance culls above the threshold.
        if ( mLevels[ testLevel ].branches[ testBranch ].branchLocation > abs(branchLevel.pruneDistance) )
            return false;

    } // End if negative

    // Does this branch meet our criteria to be a frond branch.
    if ( mParams.frondData.enabled )
    {
        // Must be above or equal to the frond initial level.
        if ( outBranchLevel >= (cgInt32)mParams.frondData.initialLevel )
        {
            bool below = false;

            // Selected based on distance along specified parent branch. First select 
            // the relevant branch, at the specified depth, that we will use for testing.
            cgUInt32 testLevel   = outBranchLevel;
            cgUInt32 testBranch  = outBranchIndex;
            for ( cgUInt32 i = 0; i < mParams.frondData.depth; ++i )
            {
                // Select next level up
                testBranch = mLevels[testLevel].branches[testBranch].parentBranch;
                testLevel  = testLevel - 1;

            } // Next Parent Level

            // If the starting location of the branch to be tested falls below the 
            // specified distance threshold, then we're below.
            if ( mParams.frondData.minimumDistance > 1e-4 )
            {
                // Positive distance culls below the threshold
                below = ( mLevels[ testLevel ].branches[ testBranch ].branchLocation < mParams.frondData.minimumDistance );
            
            } // End if positive
            else if ( mParams.frondData.minimumDistance < -1e-4 )
            {
                // Negative distance culls above the threshold.
                below = ( mLevels[ testLevel ].branches[ testBranch ].branchLocation > abs(mParams.frondData.minimumDistance) );

            } // End if negative

            // Above or below the distance threshold?
            if ( below && mParams.frondData.belowCondition == cgProceduralTreeGrowthCondition::Enabled )
                branchOut.frondBranch = true;
            else if ( below && mParams.frondData.belowCondition == cgProceduralTreeGrowthCondition::Pruned )
                return false;
            if ( !below && mParams.frondData.aboveCondition == cgProceduralTreeGrowthCondition::Enabled )
                branchOut.frondBranch = true;
            else if ( !below && mParams.frondData.aboveCondition == cgProceduralTreeGrowthCondition::Pruned )
                return false;

        } // End if >= initialLevel
    
    } // End if fronds enabled

    // Compute the length of the branch (Note: we reuse the fork bias random number).
    branchOut.length  = eval( branchLevel.length, cgBezierSpline2::NormalizePlusVariance, segmentEval, rv );
    branchOut.length *= mGlobalScale;
    
    // Compute the initial azimuth angle
    cgDouble azimuthInitialAngle = eval( branchLevel.azimuthInitial, cgBezierSpline2::NormalizePlusVariance, segmentEval, prng.next( ) );

    // Compute the initial polar angle. (Iteration[2])
    cgDouble polarInitialAngle = eval( branchLevel.polarInitial, cgBezierSpline2::NormalizePlusVariance, segmentEval, prng.next( ) );

    // Compute the base gravity scalar. (Iteration[3])
    cgDouble baseGravity = eval( branchLevel.gravity, cgBezierSpline2::NormalizePlusVariance, segmentEval, prng.next( ) );

    // Next, compute the base radius for the branch. (Iteration[4])
    cgDouble baseRadius = eval( branchLevel.radius, cgBezierSpline2::NormalizePlusVariance, segmentEval, prng.next( ) );

    // Clamp to 75% of parent radius
    baseRadius = (baseRadius * mGlobalScale) * 0.75f; 
    baseRadius = min(maxRadius, baseRadius); 

    // Iteration[5]
    prng.next();
    // Iteration[6]
    prng.next();

    // Record the direction of growth of the parent branch so that
    // we can appropriately cull based on angle of difference should this 
    // be a fork branch.
    cgVector3 parentGrowthAxis = -currentTransform.xAxis();

    // Generate the correct starting matrix for the trunk (-X = direction of growth)
    currentTransform.rotateAxis( CGEToRadian((cgFloat)azimuthInitialAngle), -currentTransform.xAxis(), currentTransform.position() );
    currentTransform.rotateAxis( -CGEToRadian((cgFloat)polarInitialAngle), currentTransform.yAxis(), currentTransform.position() );

    // Select the correct number of segments
    cgUInt32 segmentCount = branchLevel.segments;
    if ( branchOut.frondBranch && mParams.frondData.lengthSegmentOverride )
        segmentCount = mParams.frondData.lengthSegments;
    branchOut.segments.resize( segmentCount + 1 );

    // Generate the geometry for each segment in the trunk
    for ( cgUInt32 segment = 0; segment <= segmentCount; ++segment )
    {
        // Record the current growth axis at the start of this segment.
        // This will allow us to determine the total amount of "deviation"
        // that occurs at each segment (for branch fork support).
        cgVector3 previousGrowthAxis = -currentTransform.xAxis();

        // Compute the distance to the segment start
        cgDouble segmentBegin = pow( (cgDouble)segment / (cgDouble)segmentCount, branchLevel.segmentPack );
        cgDouble segmentEnd   = pow( (cgDouble)(segment + 1) / (cgDouble)segmentCount, branchLevel.segmentPack );
        cgDouble segmentEval  = segmentBegin;

        // Add the current segment start to the spline
        Segment & currentSegment = branchOut.segments[segment];
        if ( segment < segmentCount )
            currentSegment.length = ((segmentEnd - segmentBegin) * branchOut.length);
        else
            currentSegment.length = 0;

        // Store basic details
        currentSegment.segmentLocation = segmentBegin;

        // Compute the number of points in the cross section of this segment
        cgDouble verticesProfile = eval( branchLevel.segmentVerticesProfile, cgBezierSpline2::NormalizeOnly, segmentEval, 0 );
        verticesProfile          = min( 1.0, verticesProfile );
        verticesProfile          = max( 0.0, verticesProfile );
        cgDouble vertices        = 3.0 + ((cgDouble)(branchLevel.segmentVertices - 3) * verticesProfile );
        currentSegment.vertices  = (cgUInt32)vertices;

        // Azimuth Twisting (Iteration[7])
        cgDouble azimuthTwist = eval( branchLevel.azimuthTwist, cgBezierSpline2::NormalizeScaleVariance, segmentEval, prng.next() );
        
        // Polar Twisting (Iteration[8])
        cgDouble polarTwist   = eval( branchLevel.polarTwist, cgBezierSpline2::NormalizeScaleVariance, segmentEval, prng.next() );
        
        // Adjustment for segment matrix twisting comes prior to gravity for the first segment only
        if ( segment == 0 )
        {
            currentTransform.rotateAxis( CGEToRadian( (cgFloat)azimuthTwist ), cgVector3(0,0,1), currentTransform.position() );
            currentTransform.rotateLocal( 0, -CGEToRadian( (cgFloat)polarTwist ), 0 );

        } // End if first segment

        // Compute the final radius of this segment (Iteration[9])
        currentSegment.radius = baseRadius * eval( branchLevel.radiusProfile, cgBezierSpline2::NormalizePlusVariance, segmentEval, prng.next() );
        if ( currentSegment.radius > baseRadius )
            currentSegment.radius = baseRadius;
        if ( currentSegment.radius < 0.0 )
            currentSegment.radius = 0.0;
        
        /// Iteration[10]
        prng.next();

        // Compute the final gravity twisting for this segment (Iteration[11])
        // Twisting due to gravity (angle of difference) = (90 - (pow(SegmentAngle,2) / 90));
        cgVector3 gravity( 0, 0, -1 );
        cgDouble gravityAdjust;
        cgDouble angleForGravity = CGEToDegree( asin((cgDouble)cgVector3::dot( gravity, -currentTransform.xAxis() ) ) );
        cgDouble gravityProfile  = (eval( branchLevel.gravityProfile, cgBezierSpline2::NormalizePlusVariance, segmentEval, prng.next() ) - 0.5) * 2.0;
        if ( angleForGravity < 0.0 )
            gravityAdjust = (90 - (pow(angleForGravity,2) / 90)) * baseGravity * gravityProfile;
        else
            gravityAdjust = (90 + (-2 * angleForGravity) + (pow(angleForGravity,2) / 90)) * baseGravity * gravityProfile;

        // Generate axis about which to rotate in order to apply gravity
        if ( abs(currentTransform.xAxis().z) < 0.9990 )
        {
            cgVector3 rotateAxis;
            cgVector3::normalize( rotateAxis, *cgVector3::cross( rotateAxis, gravity, -currentTransform.xAxis() ) );
            currentTransform.rotateAxis( CGEToRadian((cgFloat)gravityAdjust), rotateAxis, currentTransform.position() );
        
        } // End if apply gravity adjustment

        // Adjustment for segment matrix twisting comes after gravity for
        // all segments except the first.
        if ( segment != 0 )
        {
            currentTransform.rotateLocal( 0, 0, CGEToRadian( (cgFloat)azimuthTwist ) );
            currentTransform.rotateLocal( 0, -CGEToRadian( (cgFloat)polarTwist ), 0 );
        
        } // End if not first

        // Record the total amount of deviation that occured between the
        // prior and current segments.
        currentSegment.deviation = CGEToDegree( acos( (cgDouble)cgVector3::dot( previousGrowthAxis, -currentTransform.xAxis() ) ) );

        // Store the current growth matrix in this segment. This will allow us
        // to understand how the tree was grown at a later time in order to
        // correctly attach child branches.
        currentSegment.transform = currentTransform;
        
        // Shift matrix to start of next segment.
        currentTransform.setPosition( currentTransform.position() - (currentTransform.xAxis() * (cgFloat)currentSegment.length) );

        // Final branch initialization prep
        if ( segment == 0 )
        {
            // If this branch and the parent form a "fork" then keep it, otherwise reject.
            if ( branchOut.forkBranch && branchLevel.forkAngle > 1e-4 )
            {
                // Parent segment's deviation is within tolerance?
                if ( segmentDeviation >= branchLevel.forkAngle )
                {
                    // Parent and branch are far enough apart?
                    cgDouble angle = acos( (cgDouble)cgVector3::dot( parentGrowthAxis, -currentTransform.xAxis() ) );
                    if ( CGEToDegree(angle) < branchLevel.forkAngle )
                        return false;

                } // End if parent deviates
                else
                {
                    // Parent doesn't deviate so we cannot create a fork.
                    return false;

                } // End if no parent deviation

            } // End if fork

            // First segment has three additional RNG queries
            prng.next();
            prng.next();
            prng.next();
        
        } // End if first segment
        
    } // Next trunk segment

    // Reset the random number generator for growth probability (frequency) determination.
    prng.setSeed( branchSeed );

    // Grow branches from this branch
    cgInt32 nextLevelIn  = outBranchLevel; // ToDo: This may change when we add roots?
    cgInt32 nextLevelOut = outBranchLevel + 1;
    if ( (cgInt32)mParams.branchLevels.size() > nextLevelIn )
    {
        // How many branches should we attempt to generate?
        cgProceduralTreeGrowthLevel & branchLevel = mParams.branchLevels[nextLevelIn];
        cgInt32 branchCount = (cgInt32)(branchLevel.frequency * (branchOut.length / mGlobalScale));
        if ( branchCount < 0 )
            branchCount = 0;

        // Add new generated level if it doesn't already exist.
        if ( (cgInt32)mLevels.size() <= nextLevelOut )
            mLevels.resize( nextLevelOut + 1 );
        
        // Generate branches
        for ( cgInt32 branch = 0; branch < branchCount; ++branch )
        {
            Branch & branchOut     = mLevels[outBranchLevel].branches[ outBranchIndex ];
            Level & branchOutLevel = mLevels[ nextLevelOut ];
            cgInt32 newBranchIndex = (cgInt32)branchOutLevel.branches.size();
            branchOut.childBranches.push_back( newBranchIndex );
            branchOutLevel.branches.push_back( Branch() );

            // Grow the next branch (WARNING, Existing references to 'mLevels' elements may become invalid here)
            if ( !growBranch( prng.next(), branchLevel, nextLevelOut, newBranchIndex, branch, outBranchIndex, branchAttempt ) )
            {
                // Branch rejected
                Branch & branchOut      = mLevels[outBranchLevel].branches[ outBranchIndex ];
                Level & branchOutLevel = mLevels[ nextLevelOut ];
                branchOut.childBranches.pop_back();
                branchOutLevel.branches.pop_back();

            } // End if rejected

        } // Next Branch
        
    } // End if branches

    // Branch generated
    return true;
}

//-----------------------------------------------------------------------------
// Name : buildMesh ( ) (Protected)
/// <summary>
/// Construct the mesh data based on the grown tree segments.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeGenerator::buildMesh( )
{
    // Build Z to Y transformation matrix (tree is grown with Z as up).
    cgTransform transform = cgTransform::Identity;
    transform.rotate( CGEToRadian(-90), 0, 0 );

    // Count the number of vertices / triangles required and store buffer offsets.
    size_t vertexCount = 0, triangleCount = 0;
    for ( size_t levelIndex = 0; levelIndex < mLevels.size(); ++levelIndex )
    {
        Level & level = mLevels[levelIndex];

        // Process each of the branches referenced at this level.
        for ( size_t branchIndex = 0; branchIndex < level.branches.size(); ++branchIndex )
        {
            // Is this a frond branch?
            Branch & branch = level.branches[branchIndex];
            if ( !branch.frondBranch )
            {
                // Iterate through each segment and count vertices first of all.
                for ( size_t segmentIndex = 0; segmentIndex < branch.segments.size(); ++segmentIndex )
                {
                    Segment & currentSegment = branch.segments[segmentIndex];
                    
                    // This segment's ring of vertices (note: we add one more vertex to correctly
                    // wrap the texture coordinates).
                    currentSegment.startVertex = (cgUInt32)vertexCount;
                    vertexCount += currentSegment.vertices + 1;

                } // Next Segment

                // Iterate and count triangles for same.
                for ( size_t segmentIndex = 0; segmentIndex < branch.segments.size() - 1; ++segmentIndex )
                {
                    Segment & currentSegment = branch.segments[segmentIndex];
                    Segment & nextSegment    = branch.segments[segmentIndex+1];

                    // Depending on whether this segment transitions to a higher
                    // or lower resolution, we need to take appropriate action
                    if ( currentSegment.vertices > nextSegment.vertices )
                    {
                        // Iterate through the segment with the largest number of edges first
                        // and count triangles required for connecting to the closest points on 
                        // the opposing side.
                        cgUInt32 prevConnect = 0xFFFFFFFF;
                        for ( cgUInt32 i = 0; i < currentSegment.vertices; ++i )
                        {
                            ++triangleCount;

                            // Determine the closest vertex on the next segment to connect to.
                            cgFloat  delta   = ((cgFloat)i+1.0f) / (cgFloat)currentSegment.vertices;
                            cgUInt32 connect = nextSegment.startVertex + ((cgUInt32)( delta * nextSegment.vertices ) % (nextSegment.vertices + 1));

                            // If the connection vertex (on the opposite edge) has "jumped",
                            // we will need to fill in the gap with another triangle.
                            if ( prevConnect != 0xFFFFFFFF && connect != prevConnect )
                                ++triangleCount;
                            prevConnect = connect;

                        } // Next Edge

                    } // End if current > next
                    else if ( currentSegment.vertices < nextSegment.vertices )
                    {
                        // Iterate through the segment with the largest number of edges first
                        // and count triangles required for connecting to the closest points on 
                        // the opposing side.
                        cgUInt32 prevConnect = 0xFFFFFFFF;
                        for ( cgUInt32 i = 0; i < nextSegment.vertices; ++i )
                        {
                            ++triangleCount;

                            // Determine the closest vertex on the next segment to connect to.
                            cgFloat  delta   = ((cgFloat)i+1.0f) / (cgFloat)nextSegment.vertices;
                            cgUInt32 connect = currentSegment.startVertex + ((cgUInt32)( delta * currentSegment.vertices ) % (currentSegment.vertices + 1));

                            // If the connection vertex (on the opposint edge) has "jumped",
                            // we will need to fill in the gap with another triangle.
                            if ( prevConnect != 0xFFFFFFFF && connect != prevConnect )
                                ++triangleCount;
                            prevConnect = connect;

                        } // Next Edge

                    } // End if next > current
                    else
                    {
                        // Add all quads.
                        triangleCount += currentSegment.vertices * 2;

                    } // End if next == current

                } // Next Segment

            } // End if !frond
            else
            {
                // Iterate through each segment and count vertices first of all.
                for ( size_t segmentIndex = 0; segmentIndex < branch.segments.size(); ++segmentIndex )
                {
                    Segment & currentSegment = branch.segments[segmentIndex];
                    
                    // This segment's ring of vertices (note: we add one more vertex to correctly
                    // wrap the texture coordinates).
                    currentSegment.startVertex = (cgUInt32)vertexCount;
                    vertexCount += mParams.frondData.blades * 2;

                } // Next Segment

                // Count triangles for same.
                triangleCount += (branch.segments.size() - 1) * mParams.frondData.blades * 4;

            } // End if frond

        } // Next Branch

    } // Next Level

    // Anything to do?
    if ( !vertexCount || !triangleCount )
        return false;

    // Allocate memory.
    cgArray<cgVertex> vertices( vertexCount );
    cgMesh::TriangleArray faces( triangleCount );

    // Material to assign.
    cgMaterialHandle meshMaterial = mParams.trunkData.material;

    // Process each of the generated levels (trunk and branches)
    cgVertex * verts = &vertices[0];
    cgMesh::Triangle * tris = &faces[0];
    for ( size_t levelIndex = 0; levelIndex < mLevels.size(); ++levelIndex )
    {
        Level & level = mLevels[levelIndex];

        // Process each of the branches referenced at this level.
        for ( size_t branchIndex = 0; branchIndex < level.branches.size(); ++branchIndex )
        {
            // Is this a frond branch?
            Branch & branch = level.branches[branchIndex];
            if ( !branch.frondBranch )
            {
                // Iterate through each segment and generate vertices first of all.
                for ( size_t segmentIndex = 0; segmentIndex < branch.segments.size(); ++segmentIndex )
                {
                    Segment & currentSegment = branch.segments[segmentIndex];
                    cgTransform segmentTransform = currentSegment.transform * transform;

                    // Generate the ring of vertices (note: we add one more vertex to correctly
                    // wrap the texture coordinates).
                    cgFloat deltaV = (cgFloat)currentSegment.segmentLocation; //(cgFloat)segmentIndex / (cgFloat)(branch.segments.size() - 1);
                    for ( cgUInt32 i = 0; i <= currentSegment.vertices; ++i )
                    {
                        // Compute normal (will be turned into a position later)
                        cgFloat deltaU = (cgFloat)i / (cgFloat)currentSegment.vertices;
                        cgVector3 normal( 0,
                                          cosf( deltaU * (cgFloat)CGE_TWO_PI ),
                                          sinf( deltaU * (cgFloat)CGE_TWO_PI )
                                        );

                        // Transform to final location
                        cgVector3 pos;
                        segmentTransform.transformCoord( pos, normal * (cgFloat)currentSegment.radius );
                        segmentTransform.transformNormal( normal, normal );
                        cgVector3::normalize( normal, normal );

                        // Generate binormal and tangent.
                        cgVector3 binormal;
                        cgVector3 tangent = -segmentTransform.xAxis();
                        cgVector3::normalize( tangent, tangent );
                        cgVector3::cross( binormal, normal, tangent );
                        cgVector3::normalize( binormal, binormal );

                        // Mirror Z
                        normal.z = -normal.z;
                        binormal.z = -binormal.z;
                        tangent.z = -tangent.z;
                        pos.z = -pos.z;
                        
                        // Build vertex.
                        cgVertex v;
                        v.position = pos;
                        v.normal   = normal;
                        v.binormal = binormal;
                        v.tangent  = tangent;
                        v.textureCoords[0].x = deltaU * (cgFloat)mParams.globalTextureUScale;
                        v.textureCoords[0].y = deltaV * (cgFloat)mParams.globalTextureVScale;

                        // Store
                        *verts++ = v;

                    } // Next vertex
                    
                } // Next Segment

                // Iterate and generate indices for same.
                for ( size_t segmentIndex = 0; segmentIndex < branch.segments.size() - 1; ++segmentIndex )
                {
                    Segment & currentSegment = branch.segments[segmentIndex];
                    Segment & nextSegment    = branch.segments[segmentIndex+1];

                    // Depending on whether this segment transitions to a higher
                    // or lower resolution, we need to take appropriate action
                    if ( currentSegment.vertices > nextSegment.vertices )
                    {
                        // Iterate through the segment with the largest number of edges first
                        // and connect to the closest points on the opposing side.
                        cgUInt32 prevConnect = 0xFFFFFFFF;
                        for ( cgUInt32 i = 0; i < currentSegment.vertices; ++i )
                        {
                            cgUInt32 current = currentSegment.startVertex + i;
                            cgUInt32 next    = currentSegment.startVertex + i + 1;

                            // Determine the closest vertex on the next segment to connect to.
                            cgFloat  delta   = ((cgFloat)i+1.0f) / (cgFloat)currentSegment.vertices;
                            cgUInt32 connect = nextSegment.startVertex + ((cgUInt32)( delta * nextSegment.vertices ) % (nextSegment.vertices + 1));

                            // If the connection vertex (on the opposint edge) has "jumped",
                            // fill in the gap with another triangle.
                            if ( prevConnect != 0xFFFFFFFF && connect != prevConnect )
                            {
                                tris->indices[0] = connect;
                                tris->indices[1] = connect - 1;
                                tris->indices[2] = current;
                                tris->material   = meshMaterial;
                                ++tris;
                            
                            } // End if inject
                            prevConnect = connect;
                            
                            // Build indices for the required face
                            tris->indices[0] = connect;
                            tris->indices[1] = current;
                            tris->indices[2] = next;
                            tris->material   = meshMaterial;
                            ++tris;

                        } // Next Edge

                    } // End if current > next
                    else if ( currentSegment.vertices < nextSegment.vertices )
                    {
                        // Iterate through the segment with the largest number of edges first
                        // and connect to the closest points on the opposing side.
                        cgUInt32 prevConnect = 0xFFFFFFFF;
                        for ( cgUInt32 i = 0; i < nextSegment.vertices; ++i )
                        {
                            cgUInt32 current = nextSegment.startVertex + i;
                            cgUInt32 next    = nextSegment.startVertex + i + 1;

                            // Determine the closest vertex on the next segment to connect to.
                            cgFloat  delta   = ((cgFloat)i+1.0f) / (cgFloat)nextSegment.vertices;
                            cgUInt32 connect = currentSegment.startVertex + ((cgUInt32)( delta * currentSegment.vertices ) % (currentSegment.vertices + 1));

                            // If the connection vertex (on the opposint edge) has "jumped",
                            // fill in the gap with another triangle.
                            if ( prevConnect != 0xFFFFFFFF && connect != prevConnect )
                            {
                                tris->indices[0] = connect - 1;
                                tris->indices[1] = connect;
                                tris->indices[2] = current;
                                tris->material   = meshMaterial;
                                ++tris;
                            
                            } // End if inject
                            prevConnect = connect;
                            
                            // Build indices for the required face
                            tris->indices[0] = current;
                            tris->indices[1] = connect;
                            tris->indices[2] = next;
                            tris->material   = meshMaterial;
                            ++tris;

                        } // Next Edge

                    } // End if next > current
                    else
                    {
                        // Add all quads.
                        for ( cgUInt32 i = 0; i < currentSegment.vertices; ++i )
                        {
                            cgUInt32 current     = currentSegment.startVertex + i;
                            cgUInt32 next        = currentSegment.startVertex + i + 1;
                            cgUInt32 connect     = nextSegment.startVertex + i;
                            cgUInt32 connectNext = nextSegment.startVertex + i + 1;

                            // Build indices for the two required faces
                            tris->indices[0] = connect;
                            tris->indices[1] = current;
                            tris->indices[2] = next;
                            tris->material   = meshMaterial;
                            ++tris;
                            tris->indices[0] = connectNext;
                            tris->indices[1] = connect;
                            tris->indices[2] = next;
                            tris->material   = meshMaterial;
                            ++tris;

                        } // Next Edge

                    } // End if next == current

                } // Next Segment

            } // End if !frond
            else
            {
                cgFloat radius = ((cgFloat)branch.length / 5.0f) * (cgFloat)mParams.frondData.sizeFactor;

                // Iterate through each segment and generate vertices first of all.
                for ( size_t segmentIndex = 0; segmentIndex < branch.segments.size(); ++segmentIndex )
                {
                    Segment & currentSegment = branch.segments[segmentIndex];
                    cgTransform segmentTransform = currentSegment.transform * transform;
                    if ( segmentIndex == branch.segments.size() - 1 )
                    {
                        segmentTransform = branch.segments[segmentIndex-1].transform;
                        segmentTransform.position() = currentSegment.transform.position();
                        segmentTransform *= transform;
                    }

                    // Generate the ring of vertices (note: we add one more vertex to correctly
                    // wrap the texture coordinates).
                    cgFloat deltaV = (cgFloat)currentSegment.segmentLocation; //(cgFloat)segmentIndex / (cgFloat)(branch.segments.size() - 1);
                    for ( cgUInt32 i = 0; i < mParams.frondData.blades; ++i )
                    {
                        // Compute normal (will be turned into a position later)
                        cgFloat deltaU = (cgFloat)i / (cgFloat)mParams.frondData.blades;
                        cgVector3 normal( 0,
                                          cosf( deltaU * (cgFloat)CGE_TWO_PI * 0.5f ),
                                          sinf( deltaU * (cgFloat)CGE_TWO_PI * 0.5f )
                                        );

                        // Transform to final location
                        cgVector3 pos1, pos2;
                        segmentTransform.transformCoord( pos1, normal * radius );
                        segmentTransform.transformCoord( pos2, -normal * radius );
                        
                        // Generate tangent frame.
                        cgVector3 binormal = segmentTransform.yAxis();
                        cgVector3 tangent = segmentTransform.zAxis();
                        normal = -segmentTransform.xAxis();
                        cgVector3::normalize( tangent, tangent );
                        cgVector3::normalize( binormal, binormal );
                        cgVector3::normalize( normal, normal );

                        // Mirror Z
                        normal.z = -normal.z;
                        binormal.z = -binormal.z;
                        tangent.z = -tangent.z;
                        pos1.z = -pos1.z;
                        pos2.z = -pos2.z;
                        
                        // Build vertex.
                        cgVertex v;
                        v.position = pos1;
                        v.normal   = normal;
                        v.binormal = binormal;
                        v.tangent  = tangent;
                        v.textureCoords[0].x = 0;
                        v.textureCoords[0].y = 1.0f - deltaV;

                        // Store
                        *verts++ = v;

                        // And another.
                        v.position = pos2;
                        v.textureCoords[0].x = 1.0f;

                        // Store
                        *verts++ = v;

                    } // Next vertex
                    
                } // Next Segment

                // Iterate and generate indices for same.
                for ( size_t segmentIndex = 0; segmentIndex < branch.segments.size() - 1; ++segmentIndex )
                {
                    Segment & currentSegment = branch.segments[segmentIndex];
                    Segment & nextSegment    = branch.segments[segmentIndex+1];

                    // Add quads.
                    for ( cgUInt32 i = 0; i < mParams.frondData.blades; ++i )
                    {
                        tris->indices[0] = currentSegment.startVertex + (i*2);
                        tris->indices[1] = currentSegment.startVertex + (i*2)+1;
                        tris->indices[2] = nextSegment.startVertex + (i*2);
                        tris->material = mParams.frondData.material;
                        ++tris;

                        tris->indices[0] = currentSegment.startVertex + (i*2)+1;
                        tris->indices[1] = nextSegment.startVertex + (i*2)+1;
                        tris->indices[2] = nextSegment.startVertex + (i*2);
                        tris->material = mParams.frondData.material;
                        ++tris;

                        tris->indices[2] = currentSegment.startVertex + (i*2);
                        tris->indices[1] = currentSegment.startVertex + (i*2)+1;
                        tris->indices[0] = nextSegment.startVertex + (i*2);
                        tris->material = mParams.frondData.material;
                        ++tris;

                        tris->indices[2] = currentSegment.startVertex + (i*2)+1;
                        tris->indices[1] = nextSegment.startVertex + (i*2)+1;
                        tris->indices[0] = nextSegment.startVertex + (i*2);
                        tris->material = mParams.frondData.material;
                        ++tris;
                    
                    } // Next quad

                } // Next Segment

            } // End if frond

        } // Next Branch

    } // Next Level

    // Allocate a new *internal* mesh data item to house our tree data. 
    cgMesh * mesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

    // Prepare mesh data for rendering.
    if ( !mesh->prepareMesh( cgVertexFormat::formatFromDeclarator(cgVertex::Declarator), &vertices.front(), vertices.size(), faces, true, true, true, cgResourceManager::getInstance() ) )
    {
        mesh->deleteReference();
        return false;
    
    } // End if failed

    // Create mesh resource
    cgResourceManager * resources = cgResourceManager::getInstance();
    resources->addMesh( &mMesh, mesh, cgResourceFlags::ForceNew, cgString::Empty, cgDebugSource() );
    
    // Did the creation succeed?
    if ( !mMesh.isValid() )
    {
        mesh->deleteReference();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : Clear ( ) (Protected)
/// <summary>
/// Destroy any resident prior growth / rendering data.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeGenerator::clear( )
{
    // Destroy any resident data.
    mMesh.close();
    mLevels.clear();
}

///////////////////////////////////////////////////////////////////////////////
// cgProceduralTreeGrowthProperties Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeGrowthProperties () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeGrowthProperties::cgProceduralTreeGrowthProperties( )
{
    cgBezierSpline2::SplinePoint pt;
    cgRandom::ParkMiller prng( false );

    // Initialize global variables to sensible defaults
    mainSeed            = (cgInt32)prng.next( (cgDouble)0, (cgDouble)999999 );
    flareSeed           = (cgInt32)prng.next( (cgDouble)0, (cgDouble)999999 );
    globalScale         = 1.0;
    globalScaleVariance = 0.1;
    globalTextureUScale = 4.0f;
    globalTextureVScale = 8.0f;

    // Setup default frond properties.
    frondData.enabled               = true;
    frondData.initialLevel          = 2;
    frondData.minimumDistance       = 0.25;
    frondData.depth                 = 0;
    frondData.aboveCondition        = cgProceduralTreeGrowthCondition::Enabled;
    frondData.belowCondition        = cgProceduralTreeGrowthCondition::Disabled;
    frondData.sizeFactor            = 0.7;
    frondData.minimumOffsetAngle    = 45;
    frondData.maximumOffsetAngle    = 75;
    frondData.type                  = cgProceduralTreeFrondType::Blades;
    frondData.blades                = 2;
    frondData.extrusionSegments     = 1;
    frondData.lengthSegmentOverride = true;
    frondData.lengthSegments        = 2;

    // Setup default trunk properties
    // Azimuth initial angle
    trunkData.azimuthInitial.setRange( 0, 0 );
    trunkData.azimuthInitial.setVariance( 180 );
    trunkData.azimuthInitial.setDescription( cgBezierSpline2::Maximum );
    
    // Polar initial angle
    trunkData.polarInitial.setRange( -90, -90 );
    trunkData.polarInitial.setVariance( 0 );
    trunkData.polarInitial.setDescription( cgBezierSpline2::Maximum );
    
    // Azimuth twisting
    trunkData.azimuthTwist.setRange( 0, 1 );
    trunkData.azimuthTwist.setVariance( 20 );
    trunkData.azimuthTwist.setDescription( cgBezierSpline2::LinearGrowth );
    
    // Polar Twisting
    trunkData.polarTwist.setRange( 0, 1 );
    trunkData.polarTwist.setVariance( 20 );
    trunkData.polarTwist.setDescription( cgBezierSpline2::LinearGrowth );
    
    // Gravity
    trunkData.gravity.setRange( 0.95, 0.95 );
    trunkData.gravity.setVariance( 0.2 );
    trunkData.gravity.setDescription( cgBezierSpline2::Maximum );
    
    // Gravity Profile
    trunkData.gravityProfile.setRange( 0, 1 );
    trunkData.gravityProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0.0f, 0.5f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.894427191f * 0.1f, 0.4472135955f * 0.1f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.894427191f * 0.1f, 0.4472135955f * 0.1f );
    trunkData.gravityProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 1.0f, 1.0f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.894427191f * 0.1f, 0.4472135955f * 0.1f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.894427191f * 0.1f, 0.4472135955f * 0.1f );
    trunkData.gravityProfile.setPoint( 1, pt );

    // Flexibility
    trunkData.flexibility.setRange( 20, 20 );
    trunkData.flexibility.setVariance( 5 );
    trunkData.flexibility.setDescription( cgBezierSpline2::Maximum );

    // Flexibility Profile
    trunkData.flexibilityProfile.setRange( 0, 1 );
    trunkData.flexibilityProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0.0f, 0.0f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.7071068f * 0.079604f, 0.7071068f * 0.079604f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.7071068f * 0.079604f, 0.7071068f * 0.079604f );
    trunkData.flexibilityProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 1.0f, 0.882136f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.999355f * 0.799201f, 0.0359068f * 0.799201f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.999355f * 0.799201f, 0.0359068f * 0.799201f );
    trunkData.flexibilityProfile.setPoint( 1, pt );

    // Length
    trunkData.length.setRange( 4.7, 4.7 );
    trunkData.length.setVariance( 1.5 );
    trunkData.length.setDescription( cgBezierSpline2::Maximum );

    // Radius
    trunkData.radius.setRange( 0.25, 0.25 );
    trunkData.radius.setVariance( 0.01 );
    trunkData.radius.setDescription( cgBezierSpline2::Maximum );

    // Radius Profile
    trunkData.radiusProfile.setRange( 0, 1 );
    trunkData.radiusProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0.0f, 1.0f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.546261f * 0.695738f, -0.837615f * 0.6957384f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.546261f * 0.695738f, -0.837615f * 0.6957384f );
    trunkData.radiusProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 1.0f, 0.01122f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.110917f * 0.3815f, -0.99383f * 0.3815f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.110917f * 0.3815f, -0.99383f * 0.3815f );
    trunkData.radiusProfile.setPoint( 1, pt );

    // Segment Data
    trunkData.segments                              = 16;
    trunkData.segmentVertices                       = 16;
    trunkData.segmentPack                           = 2.0;
    trunkData.segmentVerticesProfile.setRange( 0, 1 );
    trunkData.segmentVerticesProfile.setVariance( 0.0f );
    pt.point                                        = cgVector2( 0, 1.07522f );
    pt.controlPointOut                              = pt.point + cgVector2( 1 * 0.1f, 0 * 0.1f );
    pt.controlPointIn                               = pt.point - cgVector2( 1 * 0.1f, 0 * 0.1f );
    trunkData.segmentVerticesProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 0.254187f, 0.5f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.0447598f * 0.352272f, -0.998998f * 0.352272f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.0447598f * 0.352272f, -0.998998f * 0.352272f );
    trunkData.segmentVerticesProfile.insertPoint( 1, pt );
    pt.point                                        = cgVector2( 1.0f, 0.0f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.693813f * 0.300332f, -0.720155f * 0.300332f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.693813f * 0.300332f, -0.720155f * 0.300332f );
    trunkData.segmentVerticesProfile.setPoint( 2, pt );

    // Roughness Data
    trunkData.roughnessValue                        = 0.8;
    trunkData.roughnessVariance                     = -0.25;
    trunkData.roughnessFrequencyU                   = 6.5;
    trunkData.roughnessFrequencyV                   = 28;
    trunkData.roughnessGnarl                        = 0;
    trunkData.roughnessGnarlProfile.setRange( 0, 1 );
    trunkData.roughnessGnarlProfile.setVariance( 0 );
    trunkData.roughnessGnarlProfile.setDescription( cgBezierSpline2::LinearGrowth );
    
    trunkData.roughnessProfile.setRange( 0, 1 );
    trunkData.roughnessProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0, 0 );
    pt.controlPointOut                              = pt.point + cgVector2( 0.110432f * 0.1f, 0.993884f * 0.1f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.110432f * 0.1f, 0.993884f * 0.1f );
    trunkData.roughnessProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 0.0674281f, 0.951112f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.99872f * 0.0874774f, -0.0505817f * 0.0874774f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.99872f * 0.0874774f, -0.0505817f * 0.0874774f );
    trunkData.roughnessProfile.insertPoint( 1, pt );
    pt.point                                        = cgVector2( 0.264072f, 0.123699f );
    pt.controlPointOut                              = pt.point + cgVector2( 1.0f * 0.05f, 0.0f * 0.05f );
    pt.controlPointIn                               = pt.point - cgVector2( 1.0f * 0.05f, 0.0f * 0.05f );
    trunkData.roughnessProfile.insertPoint( 2, pt );
    pt.point                                        = cgVector2( 1.0f, 0.0f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.898545f * 0.100624f, -0.438882f * 0.100624f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.898545f * 0.100624f, -0.438882f * 0.100624f );
    trunkData.roughnessProfile.setPoint( 3, pt );

    // Add two branch levels
    branchLevels.resize(2);

    // Populate first branch level
    cgProceduralTreeGrowthLevel & branch1           = branchLevels[0];
    branch1.levelBegin                              = 0.25;
    branch1.levelEnd                                = 0.93;
    branch1.pruneDepth                              = 0;
    branch1.pruneDistance                           = 0;

    // Frequency
    branch1.frequency                               = 3;
    branch1.frequencyProfile.setRange( 0, 1 );
    branch1.frequencyProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0, 1 );
    pt.controlPointOut                              = pt.point + cgVector2( 1 * 0.1f, 0 * 0.1f );
    pt.controlPointIn                               = pt.point - cgVector2( 1 * 0.1f, 0 * 0.1f );
    branch1.frequencyProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 1, 0.694233f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.923393f * 0.278798f, -0.383857f * 0.278798f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.923393f * 0.278798f, -0.383857f * 0.278798f );
    branch1.frequencyProfile.setPoint( 1, pt );

    // Forking
    branch1.enableForkPrune                         = true;
    branch1.forkBias                                = 1;
    branch1.forkAngle                               = 0;
    branch1.forkLimit                               = 2;
    
    // Azimuth initial angle
    branch1.azimuthInitial.setRange( 0, 0 );
    branch1.azimuthInitial.setVariance( 180 );
    pt.point                                      = cgVector2( 0.0f, 0.946903f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.999372f * 0.197421f, -0.035428f * 0.197421f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.999372f * 0.197421f, -0.035428f * 0.197421f );
    branch1.azimuthInitial.setPoint( 0, pt );
    pt.point                                      = cgVector2( 0.315271f, 0.513274f );
    pt.controlPointOut                            = pt.point + cgVector2( 1.0f * 0.05f, 0.0f * 0.05f );
    pt.controlPointIn                             = pt.point - cgVector2( 1.0f * 0.05f, 0.0f * 0.05f );
    branch1.azimuthInitial.insertPoint( 1, pt );
    pt.point                                      = cgVector2( 1.0f, 0.000782381f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.0795023f * 0.72477f, -0.996835f * 0.72477f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.0795023f * 0.72477f, -0.996835f * 0.72477f );
    branch1.azimuthInitial.setPoint( 2, pt );
    
    // Polar initial angle
    branch1.polarInitial.setRange( -10, 60 );
    branch1.polarInitial.setVariance( 25 );
    pt.point                                      = cgVector2( 0.0f, 0.946903f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.999372f * 0.197421f, -0.035428f * 0.197421f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.999372f * 0.197421f, -0.035428f * 0.197421f );
    branch1.polarInitial.setPoint( 0, pt );
    pt.point                                      = cgVector2( 0.315271f, 0.513274f );
    pt.controlPointOut                            = pt.point + cgVector2( 1.0f * 0.05f, 0.0f * 0.05f );
    pt.controlPointIn                             = pt.point - cgVector2( 1.0f * 0.05f, 0.0f * 0.05f );
    branch1.polarInitial.insertPoint( 1, pt );
    pt.point                                      = cgVector2( 1.0f, 0.000782381f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.0795023f * 0.72477f, -0.996835f * 0.72477f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.0795023f * 0.72477f, -0.996835f * 0.72477f );
    branch1.polarInitial.setPoint( 2, pt );
    
    // Azimuth Twisting
    branch1.azimuthTwist.setRange( 0, 1 );
    branch1.azimuthTwist.setVariance( 50 );
    branch1.azimuthTwist.setDescription( cgBezierSpline2::LinearGrowth );
    
    // Polar Twisting
    branch1.polarTwist.setRange( 0, 1 );
    branch1.polarTwist.setVariance( 50 );
    branch1.polarTwist.setDescription( cgBezierSpline2::LinearGrowth );
    
    // Gravity
    branch1.gravity.setRange( 0.2, 1.0 );
    branch1.gravity.setVariance( 0.1 );
    pt.point                                      = cgVector2( 0.0f, 1.0f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.714831f * 0.079604f, -0.699297f * 0.079604f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.714831f * 0.079604f, -0.699297f * 0.079604f );
    branch1.gravity.setPoint( 0, pt );
    pt.point                                      = cgVector2( 1.0f, 0.501783f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.699609f * 0.107006f, -0.714526f * 0.107006f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.699609f * 0.107006f, -0.714526f * 0.107006f );
    branch1.gravity.setPoint( 1, pt );
    
    // Gravity Profile
    branch1.gravityProfile.setRange( 0, 1 );
    branch1.gravityProfile.setVariance( 0 );
    pt.point                                      = cgVector2( 0.0f, 0.5f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.89466f * 0.113537f, 0.446747f * 0.113537f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.89466f * 0.113537f, 0.446747f * 0.113537f );
    branch1.gravityProfile.setPoint( 0, pt );
    pt.point                                      = cgVector2( 1.0f, 0.958251f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.253012f * 0.714723f, 0.967463f * 0.714723f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.253012f * 0.714723f, 0.967463f * 0.714723f );
    branch1.gravityProfile.setPoint( 1, pt );

    // Flexibility
    branch1.flexibility.setRange( 10, 20 );
    branch1.flexibility.setVariance( 3 );
    branch1.flexibility.setDescription( cgBezierSpline2::LinearDecay );

    // Flexibility Profile
    branch1.flexibilityProfile.setRange( 0, 1 );
    branch1.flexibilityProfile.setVariance( 0 );
    pt.point                                      = cgVector2( 0.0f, 0.0f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.7071068f * 0.079604f, 0.7071068f * 0.079604f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.7071068f * 0.079604f, 0.7071068f * 0.079604f );
    branch1.flexibilityProfile.setPoint( 0, pt );
    pt.point                                      = cgVector2( 1.0f, 0.73884f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.999698f * 0.743114f, -0.0245894f * 0.743114f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.999698f * 0.743114f, -0.0245894f * 0.743114f );
    branch1.flexibilityProfile.setPoint( 1, pt );

    // Length
    branch1.length.setRange( 1.8, 2.5 );
    branch1.length.setVariance( 0.5 );
    pt.point                                      = cgVector2( 0.0f, 1.0f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.93842f * 0.870629f, -0.345496f * 0.870629f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.93842f * 0.870629f, -0.345496f * 0.870629f );
    branch1.length.setPoint( 0, pt );
    pt.point                                      = cgVector2( 1.0f, 0.0f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.25536f * 0.498751f, -0.966846f * 0.498751f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.25536f * 0.498751f, -0.966846f * 0.498751f );
    branch1.length.setPoint( 1, pt );

    // Radius
    branch1.radius.setRange( 0.1, 0.2 );
    branch1.radius.setVariance( 0 );
    pt.point                                      = cgVector2( 0.0f, 1.0f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.890265f * 0.139823f, -0.455443f * 0.139823f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.890265f * 0.139823f, -0.455443f * 0.139823f );
    branch1.radius.setPoint( 0, pt );
    pt.point                                      = cgVector2( 1.0f, 0.214182f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.759983f * 0.175014f, -0.649943f * 0.175014f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.759983f * 0.175014f, -0.649943f * 0.175014f );
    branch1.radius.setPoint( 1, pt );

    // Radius Profile
    branch1.radiusProfile.setRange( 0, 1 );
    branch1.radiusProfile.setVariance( 0 );
    pt.point                                      = cgVector2( 0.0f, 0.792036f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.924693f * 0.39713f, -0.380713f * 0.39713f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.924693f * 0.39713f, -0.380713f * 0.39713f );
    branch1.radiusProfile.setPoint( 0, pt );
    pt.point                                      = cgVector2( 1.0f, 0.000782381f );
    pt.controlPointOut                            = pt.point + cgVector2( 0.48165f * 0.545668f, -0.876364f * 0.545668f );
    pt.controlPointIn                             = pt.point - cgVector2( 0.48165f * 0.545668f, -0.876364f * 0.545668f );
    branch1.radiusProfile.setPoint( 1, pt );

    // Segment Data
    branch1.segments                              = 6;
    branch1.segmentLengthKeep                     = 0.45;
    branch1.segmentVertices                       = 7;
    branch1.segmentVerticesKeep                   = 0.80;
    branch1.segmentPack                           = 0.75;
    branch1.segmentVerticesProfile.setRange( 0, 1 );
    branch1.segmentVerticesProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0, 1.07522f );
    pt.controlPointOut                              = pt.point + cgVector2( 1 * 0.1f, 0 * 0.1f );
    pt.controlPointIn                               = pt.point - cgVector2( 1 * 0.1f, 0 * 0.1f );
    branch1.segmentVerticesProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 0.254187f, 0.5f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.0447598f * 0.352272f, -0.998998f * 0.352272f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.0447598f * 0.352272f, -0.998998f * 0.352272f );
    branch1.segmentVerticesProfile.insertPoint( 1, pt );
    pt.point                                        = cgVector2( 1.0f, 0.0f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.693813f * 0.300332f, -0.720155f * 0.300332f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.693813f * 0.300332f, -0.720155f * 0.300332f );
    branch1.segmentVerticesProfile.setPoint( 2, pt );

    // Roughness Data
    branch1.roughnessValue                        = 0.0;
    branch1.roughnessVariance                     = 0.0;
    branch1.roughnessFrequencyU                   = 5;
    branch1.roughnessFrequencyV                   = 30;
    branch1.roughnessGnarl                        = 0;
    branch1.roughnessGnarlUnison                  = false;
    branch1.roughnessGnarlProfile.setRange( 0, 1 );
    branch1.roughnessGnarlProfile.setVariance( 0 );
    branch1.roughnessGnarlProfile.setDescription( cgBezierSpline2::LinearGrowth );
    branch1.roughnessProfile.setRange( 0, 1 );
    branch1.roughnessProfile.setVariance( 0 );
    branch1.roughnessProfile.setDescription( cgBezierSpline2::Maximum );

    // Populate second branch level
    cgProceduralTreeGrowthLevel & branch2           = branchLevels[1];
    branch2.levelBegin                              = 0.25;
    branch2.levelEnd                                = 0.75;
    branch2.pruneDepth                              = 0;
    branch2.pruneDistance                           = 0;

    // Frequency
    branch2.frequency                               = 30;
    branch2.frequencyProfile.setRange( 0, 1 );
    branch2.frequencyProfile.setVariance( 0 );
    branch2.frequencyProfile.setDescription( cgBezierSpline2::Maximum );
    
    // Forking
    branch2.enableForkPrune                         = true;
    branch2.forkBias                                = 1;
    branch2.forkAngle                               = 0;
    branch2.forkLimit                               = 1;
    
    // Azimuth initial angle
    branch2.azimuthInitial.setRange( 0, 0 );
    branch2.azimuthInitial.setVariance( 180 );
    branch2.azimuthInitial.setDescription( cgBezierSpline2::LinearDecay );
    
    // Polar initial angle
    branch2.polarInitial.setRange( -10, 75 );
    branch2.polarInitial.setVariance( 15 );
    branch2.polarInitial.setDescription( cgBezierSpline2::LinearDecay );
    
    // Azimuth Twisting
    branch2.azimuthTwist.setRange( 0, 1 );
    branch2.azimuthTwist.setVariance( 20 );
    branch2.azimuthTwist.setDescription( cgBezierSpline2::LinearGrowth );
    
    // Polar Twisting
    branch2.polarTwist.setRange( 0, 1 );
    branch2.polarTwist.setVariance( 20 );
    branch2.polarTwist.setDescription( cgBezierSpline2::LinearGrowth );
    
    // Gravity
    branch2.gravity.setRange( 0.1, 1 );
    branch2.gravity.setVariance( 0.1 );
    branch2.gravity.setDescription( cgBezierSpline2::LinearDecay );
    
    // Gravity Profile
    branch2.gravityProfile.setRange( 0, 1 );
    branch2.gravityProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0.0f, 0.557522f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.955763f * 0.142357f, 0.294139f * 0.142357f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.955763f * 0.142357f, 0.294139f * 0.142357f );
    branch2.gravityProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 1.0f, 0.787611f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.966125f * 0.122151f, 0.258076f * 0.122151f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.966125f * 0.122151f, 0.258076f * 0.122151f );
    branch2.gravityProfile.setPoint( 1, pt );

    // Flexibility
    branch2.flexibility.setRange( 2, 8 );
    branch2.flexibility.setVariance( 2 );
    branch2.flexibility.setDescription( cgBezierSpline2::LinearDecay );

    // Flexibility Profile
    branch2.flexibilityProfile.setRange( 0, 1 );
    branch2.flexibilityProfile.setVariance( 0 );
    branch2.flexibilityProfile.setDescription( cgBezierSpline2::LinearGrowth );
    
    // Length
    branch2.length.setRange( 1.2, 1.5 );
    branch2.length.setVariance( 0.3 );
    branch2.length.setDescription( cgBezierSpline2::LinearDecay );

    // Radius
    branch2.radius.setRange( 0.02, 0.2 );
    branch2.radius.setVariance( 0 );
    branch2.radius.setDescription( cgBezierSpline2::LinearDecay );

    // Radius Profile
    branch2.radiusProfile.setRange( 0, 1 );
    branch2.radiusProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0.0f, 0.637168f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.995237f * 0.888723f, -0.0974885f * 0.888723f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.995237f * 0.888723f, -0.0974885f * 0.888723f );
    branch2.radiusProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 1.0f, 0.000782381f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.699609f * 0.107006f, -0.714526f * 0.107006f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.699609f * 0.107006f, -0.714526f * 0.107006f );
    branch2.radiusProfile.setPoint( 1, pt );

    // Segment Data
    branch2.segments                                = 3;
    branch2.segmentLengthKeep                       = 1.0;
    branch2.segmentVertices                         = 3;
    branch2.segmentVerticesKeep                     = 1.0;
    branch2.segmentPack                             = 2.0;
    branch2.segmentVerticesProfile.setRange( 0, 1 );
    branch2.segmentVerticesProfile.setVariance( 0 );
    pt.point                                        = cgVector2( 0.0f, 1.12832f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.707106f * 0.1f, -0.707108f * 0.1f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.707106f * 0.1f, -0.707108f * 0.1f );
    branch2.segmentVerticesProfile.setPoint( 0, pt );
    pt.point                                        = cgVector2( 1.0f, 0.0f );
    pt.controlPointOut                              = pt.point + cgVector2( 0.949826f * 0.896478f, -0.312778f * 0.896478f );
    pt.controlPointIn                               = pt.point - cgVector2( 0.949826f * 0.896478f, -0.312778f * 0.896478f );
    branch2.segmentVerticesProfile.setPoint( 1, pt );
    
    // Roughness Data
    branch2.roughnessValue                          = 0.0;
    branch2.roughnessVariance                       = 0.0;
    branch2.roughnessFrequencyU                     = 5;
    branch2.roughnessFrequencyV                     = 30;
    branch2.roughnessGnarl                          = 0;
    branch2.roughnessGnarlUnison                    = false;
    branch2.roughnessGnarlProfile.setRange( 0, 1 );
    branch2.roughnessGnarlProfile.setVariance( 0 );
    branch2.roughnessGnarlProfile.setDescription( cgBezierSpline2::LinearGrowth );
    branch2.roughnessProfile.setRange( 0, 1 );
    branch2.roughnessProfile.setVariance( 0 );
    branch2.roughnessProfile.setDescription( cgBezierSpline2::Maximum );
}
