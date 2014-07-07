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
// Name : cgAnimationTypes.cpp                                               //
//                                                                           //
// Desc : Houses declarations and classes for various miscellaneous          //
//        animation related types, defines and macros.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAnimationTypes Module Includes
//-----------------------------------------------------------------------------
#include <Animation/cgAnimationTypes.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgAnimationChannel::mInsertChannelData;
cgWorldQuery cgAnimationChannel::mUpdateChannelData;
cgWorldQuery cgAnimationTargetController::mInsertController;
cgWorldQuery cgAnimationTargetController::mLoadChannelData;

///////////////////////////////////////////////////////////////////////////////
// cgAnimationChannel Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationChannel::prepareQueries( cgWorld * world )
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertChannelData.isPrepared( world ) )
            mInsertChannelData.prepare( world, _T("INSERT INTO 'DataSources::AnimationSet::ControllerChannels' VALUES(NULL,?1,?2,?3,?4,?5,?6)"), true );
        if ( !mUpdateChannelData.isPrepared( world ) )
            mUpdateChannelData.prepare( world, _T("UPDATE 'DataSources::AnimationSet::ControllerChannels' SET DataType=?1, DataContext=?2, EntryCount=?3, data=?4 WHERE ControllerChannelId=?5"), true );
    
    } // End if sandbox
}

///////////////////////////////////////////////////////////////////////////////
// cgFloatCurveAnimationChannel Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : clear()
/// <summary>
/// Clear out all animation data and recover allocated memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgFloatCurveAnimationChannel::clear( )
{
    data.clear();
    dirty = true;
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this animation
/// channel.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFloatCurveAnimationChannel::serialize( cgUInt32 targetControllerId, const cgString & channelIdentifier, cgWorld * world )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // If we do not yet exist in the database, insert. Otherwise,
        // update the existing record if this channel's data is dirty.
        if ( !databaseId )
        {
            // Insert data for this controller.
            prepareQueries( world );
            mInsertChannelData.bindParameter( 1, targetControllerId  );
            mInsertChannelData.bindParameter( 2, channelIdentifier );
            mInsertChannelData.bindParameter( 3, (cgInt32)cgAnimationChannelDataType::BezierSpline );

            // Insert spline data as blob.
            cgUInt32 curveType = data.getDescription();
            mInsertChannelData.bindParameter( 4, curveType );
            if ( curveType == cgBezierSpline2::Custom && data.getPointCount() > 0 )
            {
                mInsertChannelData.bindParameter( 5, data.getPointCount() );
                mInsertChannelData.bindParameter( 6, &data.getPoints().front(), data.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mInsertChannelData.bindParameter( 5, 0 );
                mInsertChannelData.bindParameter( 6, CG_NULL, 0 );
            
            } // End if described
            
            // Process!
            if ( !mInsertChannelData.step( true ) )
            {
                cgString error;
                mInsertChannelData.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert float curve data for animation channel '%s'. Error: %s\n"), channelIdentifier.c_str(), error.c_str() );
                return false;

            } // End if failed

            // Retrieve the new database record identifier for
            // later alterations.
            databaseId = mInsertChannelData.getLastInsertId();

        } // End if !exists
        else if ( dirty )
        {
            // Existing data is dirty and needs to be updated.
            prepareQueries( world );
            mUpdateChannelData.bindParameter( 1, (cgInt32)cgAnimationChannelDataType::BezierSpline );
            mUpdateChannelData.bindParameter( 5, databaseId );

            // Insert spline data as blob.
            cgUInt32 curveType = data.getDescription();
            mUpdateChannelData.bindParameter( 2, curveType );
            if ( curveType == cgBezierSpline2::Custom && data.getPointCount() > 0 )
            {
                mUpdateChannelData.bindParameter( 3, data.getPointCount() );
                mUpdateChannelData.bindParameter( 4, &data.getPoints().front(), data.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mUpdateChannelData.bindParameter( 3, 0 );
                mUpdateChannelData.bindParameter( 4, CG_NULL, 0 );
            
            } // End if described
            
            // Process!
            if ( !mUpdateChannelData.step( true ) )
            {
                cgString error;
                mInsertChannelData.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update float curve data for animation channel '%s'. Error: %s\n"), channelIdentifier.c_str(), error.c_str() );
                return false;

            } // End if failed

        } // End if exists & dirty

    } // End if sandbox

    // Data is no longer dirty.
    dirty = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserialize ()
/// <summary>
/// Import the data associated with this animation target controller based on
/// the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFloatCurveAnimationChannel::deserialize( cgWorldQuery & channelQuery, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut )
{
    cgUInt32 curveType;
    channelQuery.getColumn( _T("DataContext"), curveType );
    if ( curveType == cgBezierSpline2::Custom )
    {
        cgUInt32 pointCount, size;
        cgBezierSpline2::SplinePoint * pointData = CG_NULL;
        channelQuery.getColumn( _T("EntryCount"), pointCount );
        channelQuery.getColumn( _T("Data"), (void**)&pointData, size );
        data.clear();
        if ( pointData && size == (pointCount * sizeof(cgBezierSpline2::SplinePoint)) )
        {
            for ( cgUInt32 i = 0; i < pointCount; ++i )
            {
                data.addPoint( pointData[i] );

                // Update bounding frames.
                cgInt32 frameIndex = integerFrameIndex( pointData[i].point.x );
                if ( frameIndex < minFrameOut )
                    minFrameOut = frameIndex;
                if ( frameIndex > maxFrameOut )
                    maxFrameOut = frameIndex;

            } // Next point

        } // End if valid point data

    } // End if custom curve
    else
    {
        data.setDescription( (cgBezierSpline2::SplineDescription)curveType );

    } // End if described

    // Make sure spline is initialized during load rather than on first use.
    data.isComplex();

    // Only associate with the original database row identifier if we were
    // not instructed to clone data. By ensuring that the 'databaseId'
    // is '0', this will force the insertion of a new channel row.
    if ( cloning )
    {
        databaseId = 0;
        dirty = true;
    
    } // End if cloning
    else
    {
        channelQuery.getColumn( _T("ControllerChannelId"), databaseId );
        dirty = false;
    
    } // End if !cloning

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addLinearKey ()
/// <summary>
/// Insert a new key-frame into the animation channel to produce a linear 
/// interpolation between the previous and new keys.
/// </summary>
//-----------------------------------------------------------------------------
void cgFloatCurveAnimationChannel::addLinearKey( cgInt32 frame, cgFloat value )
{
    // Compute new spline control point position data.
    cgVector2 position = cgVector2( (cgFloat)frame, value );
    cgVector2 inPosition = cgVector2(0,0);

    // Generate a linear curve between last point (if one exists) and new point.
    if ( data.getPointCount() > 0 )
    {
        cgBezierSpline2::SplinePoint pt = data.getPoint( data.getPointCount() - 1 );
        inPosition = position - ((position - pt.point) * 0.33333333333f);
        pt.controlPointOut = pt.point + ((position - pt.point) * 0.33333333333f);
        data.setPoint( data.getPointCount() - 1, pt );
    
    } // End if !empty

    // Insert the new point.
    data.addPoint( cgBezierSpline2::SplinePoint( inPosition, position, cgVector2(0,0) ) );

    // Data is now dirty and may need to be serialized.
    dirty = true;
}

///////////////////////////////////////////////////////////////////////////////
// cgQuaternionAnimationChannel Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : clear()
/// <summary>
/// Clear out all animation data and recover allocated memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgQuaternionAnimationChannel::clear( )
{
    data.clear();
    dirty = true;
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this animation
/// channel.
/// </summary>
//-----------------------------------------------------------------------------
bool cgQuaternionAnimationChannel::serialize( cgUInt32 targetControllerId, const cgString & channelIdentifier, cgWorld * world )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // If we do not yet exist in the database, insert. Otherwise,
        // update the existing record if this channel's data is dirty.
        if ( !databaseId )
        {
            // Insert data for this controller.
            prepareQueries( world );
            mInsertChannelData.bindParameter( 1, targetControllerId  );
            mInsertChannelData.bindParameter( 2, channelIdentifier );
            mInsertChannelData.bindParameter( 3, (cgInt32)cgAnimationChannelDataType::CustomKeys );

            // Insert key data as blob.
            mInsertChannelData.bindParameter( 4, (cgUInt32)0 ); // Context
            if ( !data.empty() )
            {
                mInsertChannelData.bindParameter( 5, (cgUInt32)data.size() );
                mInsertChannelData.bindParameter( 6, &data.front(), data.size() * sizeof(QuaternionKeyFrame) );
            
            } // End if custom
            else
            {
                mInsertChannelData.bindParameter( 5, 0 );
                mInsertChannelData.bindParameter( 6, CG_NULL, 0 );
            
            } // End if described
            
            // Process!
            if ( !mInsertChannelData.step( true ) )
            {
                cgString error;
                mInsertChannelData.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert quaternion key data for animation channel '%s'. Error: %s\n"), channelIdentifier.c_str(), error.c_str() );
                return false;

            } // End if failed

            // Retrieve the new database record identifier for
            // later alterations.
            databaseId = mInsertChannelData.getLastInsertId();

        } // End if !exists
        else if ( dirty )
        {
            // Existing data is dirty and needs to be updated.
            prepareQueries( world );
            mUpdateChannelData.bindParameter( 1, (cgInt32)cgAnimationChannelDataType::CustomKeys );
            mUpdateChannelData.bindParameter( 5, databaseId );

            // Insert key data as blob.
            mUpdateChannelData.bindParameter( 2, 0 ); // Context
            if ( !data.empty() )
            {
                mUpdateChannelData.bindParameter( 3, (cgUInt32)data.size() );
                mUpdateChannelData.bindParameter( 4, &data.front(), data.size() * sizeof(QuaternionKeyFrame) );
            
            } // End if custom
            else
            {
                mUpdateChannelData.bindParameter( 3, 0 );
                mUpdateChannelData.bindParameter( 4, CG_NULL, 0 );
            
            } // End if described
            
            // Process!
            if ( !mUpdateChannelData.step( true ) )
            {
                cgString error;
                mInsertChannelData.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update quaternion key data for animation channel '%s'. Error: %s\n"), channelIdentifier.c_str(), error.c_str() );
                return false;

            } // End if failed

        } // End if exists & dirty

    } // End if sandbox

    // Data is no longer dirty.
    dirty = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addKey ()
/// <summary>
/// Insert a new key-frame into the animation channel to produce a linear 
/// interpolation (spherical) between the previous and new keys.
/// </summary>
//-----------------------------------------------------------------------------
void cgQuaternionAnimationChannel::addKey( cgInt32 frame, const cgQuaternion & q )
{
    data.resize( data.size() + 1 );
    data.back().frame = frame;
    memcpy( data.back().value, &q, sizeof(cgQuaternion) );

    // Data is now dirty and may need to be serialized.
    dirty = true;
}

//-----------------------------------------------------------------------------
//  Name : deserialize ()
/// <summary>
/// Import the data associated with this animation target controller based on
/// the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgQuaternionAnimationChannel::deserialize( cgWorldQuery & channelQuery, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut )
{
    // Clear out previous data
    data.clear();

    // Load key frames
    cgUInt32 keyCount, size;
    QuaternionKeyFrame * keyData = CG_NULL;
    channelQuery.getColumn( _T("EntryCount"), keyCount);
    channelQuery.getColumn( _T("Data"), (void**)&keyData, size );
    if ( keyData && keyCount && ((keyCount * sizeof(QuaternionKeyFrame)) == size) )
    {
        data.resize( keyCount );
        memcpy( &data.front(), keyData, size );

        // Update bounding frame indices
        for ( size_t i = 0; i < data.size(); ++i )
        {
            // Update bounding frames.
            cgInt32 frameIndex = data[i].frame;
            if ( frameIndex < minFrameOut )
                minFrameOut = frameIndex;
            if ( frameIndex > maxFrameOut )
                maxFrameOut = frameIndex;

        } // Next point

    } // End if data exists

    // Only associate with the original database row identifier if we were
    // not instructed to clone data. By ensuring that the 'databaseId'
    // is '0', this will force the insertion of a new channel row.
    if ( cloning )
    {
        databaseId = 0;
        dirty = true;
    
    } // End if cloning
    else
    {
        channelQuery.getColumn( _T("ControllerChannelId"), databaseId );
        dirty = false;
    
    } // End if !cloning

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgAnimationTargetController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAnimationTargetController () (Constructor)
/// <summary>
/// cgAnimationTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationTargetController::cgAnimationTargetController( )
{
    mDatabaseId   = 0;
}

//-----------------------------------------------------------------------------
//  Name : ~cgAnimationTargetController () (Destructor)
/// <summary>
/// cgAnimationTargetController Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationTargetController::~cgAnimationTargetController()
{
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of an animation target controller of the specified
/// type.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationTargetController * cgAnimationTargetController::createInstance( cgAnimationTargetControllerType::Base type )
{
    switch ( type )
    {
        case cgAnimationTargetControllerType::PositionXYZ:
            return new cgPositionXYZTargetController();
        case cgAnimationTargetControllerType::ScaleXYZ:
            return new cgScaleXYZTargetController();
        case cgAnimationTargetControllerType::UniformScale:
            return new cgUniformScaleTargetController();
        case cgAnimationTargetControllerType::EulerAngles:
            return new cgEulerAnglesTargetController();
        case cgAnimationTargetControllerType::Quaternion:
            return new cgQuaternionTargetController();
        default:
            return CG_NULL;

    } // End switch type
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this animation
/// target controller and any associated child animation channels.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationTargetController::serialize( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize )
{
    if ( (cgGetSandboxMode() == cgSandboxMode::Enabled) && !mDatabaseId )
    {
        // Insert data for this controller.
        prepareQueries( world );
        mInsertController.bindParameter( 1, targetDataId  );
        mInsertController.bindParameter( 2, controllerIdentifier );
        mInsertController.bindParameter( 3, (cgInt32)getControllerType() );
        mInsertController.bindParameter( 4, customData, customDataSize );
        
        // Process!
        if ( !mInsertController.step( true ) )
        {
            cgString error;
            mInsertController.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for animation target controller '%s'. Error: %s\n"), controllerIdentifier.c_str(), error.c_str() );
            return false;

        } // End if failed

        // Retrieve the new database record identifier for
        // later alterations.
        mDatabaseId = mInsertController.getLastInsertId();

    } // End if can insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserialize ()
/// <summary>
/// Import the data associated with this animation target controller based on
/// the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationTargetController::deserialize( cgWorldQuery & controllerQuery, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut, void *& customDataOut, cgUInt32 & customDataSizeOut )
{
    // Prepare queries similar to 'serialize()' so that derived class
    // does not have to.
    prepareQueries( controllerQuery.getWorld() );

    // All we need initially is to store the database identifier.
    cgUInt32 databaseId;
    controllerQuery.getColumn( _T("TargetControllerId"), databaseId );

    // Is there any channel data to import?
    const cgStringArray & registeredChannels = getSupportedChannels();
    if ( registeredChannels.empty() )
        return true;

    // Find all associated channels.
    mLoadChannelData.bindParameter( 1, databaseId );
    if ( !mLoadChannelData.step() )
    {
        cgString error, controllerIdentifier;
        mLoadChannelData.getLastError( error );
        mLoadChannelData.reset();
        controllerQuery.getColumn( _T("ControllerIdentifier"), controllerIdentifier );
        cgAppLog::write( cgAppLog::Error, _T("Failed to load animation channel data for target controller '%s'. Error: %s\n"), controllerIdentifier.c_str(), error.c_str() );
        return false;

    } // End if failed

    // Iterate through discovered channels
    bool result = true;
    cgString channelIdentifier;
    for ( ; mLoadChannelData.nextRow() && result; )
    {
        mLoadChannelData.getColumn( _T("ChannelIdentifier"), channelIdentifier );
        for ( size_t i = 0; i < registeredChannels.size(); ++i )
        {
            if ( channelIdentifier == registeredChannels[i] )
            {
                result = deserializeChannel( mLoadChannelData, i, cloning, minFrameOut, maxFrameOut );
                break;
            
            } // End if known channel

        } // Next known channel

    } // Next loaded channel

    // Get custom data area
    controllerQuery.getColumn( _T("ControllerParams"), &customDataOut, customDataSizeOut );

    // Only associate with the original database row identifier if we were
    // not instructed to clone data. By ensuring that the 'databaseId'
    // is '0', this will force the insertion of a new channel row.
    if ( cloning )
        mDatabaseId = 0;
    else
        mDatabaseId = databaseId;
    
    // Success?
    mLoadChannelData.reset();
    return result;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationTargetController::prepareQueries( cgWorld * world )
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertController.isPrepared( world ) )
            mInsertController.prepare( world, _T("INSERT INTO 'DataSources::AnimationSet::TargetControllers' VALUES(NULL,?1,?2,?3,?4)"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadChannelData.isPrepared( world ) )
        mLoadChannelData.prepare( world, _T("SELECT * FROM 'DataSources::AnimationSet::ControllerChannels' WHERE TargetControllerId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgPositionXYZTargetController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPositionXYZTargetController () (Constructor)
/// <summary>
/// cgPositionXYZTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgPositionXYZTargetController::cgPositionXYZTargetController( )
{
}

//-----------------------------------------------------------------------------
//  Name : cgPositionXYZTargetController () (Constructor)
/// <summary>
/// cgPositionXYZTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgPositionXYZTargetController::cgPositionXYZTargetController( const cgPositionXYZTargetController & init, const cgRange & frameRange )
{
    for ( size_t i = 0; i < 3; ++i )
    {
        cgBezierSpline2 & destSpline = mCurves[i].data;
        const cgBezierSpline2::SplinePointArray & srcPoints = init.mCurves[i].data.getPoints();
        for ( size_t j = 0; j < srcPoints.size(); ++j )
        {
            if ( srcPoints[j].point.x >= (cgFloat)frameRange.min && srcPoints[j].point.x <= (cgFloat)frameRange.max )
            {
                // Offset to start of range and add.
                cgBezierSpline2::SplinePoint pt = srcPoints[j];
                pt.point.x -= (cgFloat)frameRange.min;
                pt.controlPointIn.x -= (cgFloat)frameRange.min;
                pt.controlPointOut.x -= (cgFloat)frameRange.min;
                destSpline.addPoint( pt );
            
            } // End if in range

        } // Next point

        // Channel is dirty and needs serializing.
        mCurves[i].dirty = true;
    
    } // Next channel
}

//-----------------------------------------------------------------------------
//  Name : ~cgPositionXYZTargetController () (Destructor)
/// <summary>
/// cgPositionXYZTargetController Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgPositionXYZTargetController::~cgPositionXYZTargetController()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgPositionXYZTargetController::dispose( bool disposeBase )
{
    // Clean up.
    mCurves[0].clear();
    mCurves[1].clear();
    mCurves[2].clear();

    // Dispose of base classes
    if ( disposeBase )
        cgAnimationTargetController::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this animation
/// target controller and any associated child animation channels.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPositionXYZTargetController::serialize( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Call base class implementation.
        if ( !cgAnimationTargetController::serialize( targetDataId, controllerIdentifier, world, CG_NULL, 0 ) )
            return false;

        // Allow each of the animation channels to serialize.
        mCurves[0].serialize( mDatabaseId, _T("x"), world );
        mCurves[1].serialize( mDatabaseId, _T("y"), world );
        mCurves[2].serialize( mDatabaseId, _T("z"), world );

    } // End if can insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserializeChannel ()
/// <summary>
/// Import the data for the specified animation channel (referenced by index
/// matching the order from 'getSupportedChannels()') associated with this 
/// controller from the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPositionXYZTargetController::deserializeChannel( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut )
{
    return mCurves[channelIndex].deserialize( channelQuery, cloning, minFrameOut, maxFrameOut );
}

//-----------------------------------------------------------------------------
//  Name : evaluate ()
/// <summary>
/// Evaluate and retrieve the control value at the specified position / time.
/// </summary>
//-----------------------------------------------------------------------------
void cgPositionXYZTargetController::evaluate( cgDouble position, cgVector3 & p, const cgVector3 & default )
{
    if ( !mCurves[0].isEmpty() )
        p.x = mCurves[0].data.evaluateForX( (cgFloat)position, true );
    else
        p.x = default.x;
    if ( !mCurves[1].isEmpty() )
        p.y = mCurves[1].data.evaluateForX( (cgFloat)position, true );
    else
        p.y = default.y;
    if ( !mCurves[2].isEmpty() )
        p.z = mCurves[2].data.evaluateForX( (cgFloat)position, true );
    else
        p.z = default.z;
}

//-----------------------------------------------------------------------------
//  Name : addLinearKey ()
/// <summary>
/// Insert a new key-frame into the controller's animation channels to produce 
/// a linear interpolation between the previous and new keys.
/// </summary>
//-----------------------------------------------------------------------------
void cgPositionXYZTargetController::addLinearKey( cgInt32 frame, const cgVector3 & value )
{
    for ( cgInt i = 0; i < 3; ++i )
        mCurves[i].addLinearKey( frame, value[i] );
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the data associated with a specific animation channel by index.
/// Valid indices are 0=X, 1=Y, 2=Z
/// </summary>
//-----------------------------------------------------------------------------
const cgFloatCurveAnimationChannel & cgPositionXYZTargetController::getAnimationChannel( cgUInt32 index ) const
{
    static const cgFloatCurveAnimationChannel Empty;
    if ( index >= 3 )
        return Empty;
    return mCurves[index];
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the data associated with a specific animation channel by index.
/// Valid indices are 0=X, 1=Y, 2=Z
/// </summary>
//-----------------------------------------------------------------------------
cgFloatCurveAnimationChannel & cgPositionXYZTargetController::getAnimationChannel( cgUInt32 index )
{
    return mCurves[index];
}

//-----------------------------------------------------------------------------
// Name : getSupportedChannels ( )
/// <summary>
/// Retrieve an array containing the string identifiers of the animation
/// channels that this controller supports (if any) in the order that they
/// will be identified when referenced by index.
/// </summary>
//-----------------------------------------------------------------------------
const cgStringArray & cgPositionXYZTargetController::getSupportedChannels( ) const
{
    static const cgString channels[] = { _T("x"), _T("y"), _T("z"), cgString::Empty };
    static const cgStringArray channelVector( channels, channels + 3 );
    return channelVector;
}

///////////////////////////////////////////////////////////////////////////////
// cgScaleXYZTargetController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgScaleXYZTargetController () (Constructor)
/// <summary>
/// cgScaleXYZTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgScaleXYZTargetController::cgScaleXYZTargetController( )
{
}

//-----------------------------------------------------------------------------
//  Name : cgScaleXYZTargetController () (Constructor)
/// <summary>
/// cgScaleXYZTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgScaleXYZTargetController::cgScaleXYZTargetController( const cgScaleXYZTargetController & init, const cgRange & frameRange )
{
    for ( size_t i = 0; i < 3; ++i )
    {
        cgBezierSpline2 & destSpline = mCurves[i].data;
        const cgBezierSpline2::SplinePointArray & srcPoints = init.mCurves[i].data.getPoints();
        for ( size_t j = 0; j < srcPoints.size(); ++j )
        {
            if ( srcPoints[j].point.x >= (cgFloat)frameRange.min && srcPoints[j].point.x <= (cgFloat)frameRange.max )
            {
                // Offset to start of range and add.
                cgBezierSpline2::SplinePoint pt = srcPoints[j];
                pt.point.x -= (cgFloat)frameRange.min;
                pt.controlPointIn.x -= (cgFloat)frameRange.min;
                pt.controlPointOut.x -= (cgFloat)frameRange.min;
                destSpline.addPoint( pt );
            
            } // End if in range

        } // Next point

        // Channel is dirty and needs serializing.
        mCurves[i].dirty = true;
    
    } // Next channel
}

//-----------------------------------------------------------------------------
//  Name : ~cgScaleXYZTargetController () (Destructor)
/// <summary>
/// cgScaleXYZTargetController Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgScaleXYZTargetController::~cgScaleXYZTargetController()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgScaleXYZTargetController::dispose( bool disposeBase )
{
    // Clean up.
    mCurves[0].clear();
    mCurves[1].clear();
    mCurves[2].clear();

    // Dispose of base classes
    if ( disposeBase )
        cgAnimationTargetController::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this animation
/// target controller and any associated child animation channels.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScaleXYZTargetController::serialize( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Call base class implementation.
        if ( !cgAnimationTargetController::serialize( targetDataId, controllerIdentifier, world, CG_NULL, 0 ) )
            return false;

        // Allow each of the animation channels to serialize.
        mCurves[0].serialize( mDatabaseId, _T("x"), world );
        mCurves[1].serialize( mDatabaseId, _T("y"), world );
        mCurves[2].serialize( mDatabaseId, _T("z"), world );

    } // End if can insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserializeChannel ()
/// <summary>
/// Import the data for the specified animation channel (referenced by index
/// matching the order from 'getSupportedChannels()') associated with this 
/// controller from the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScaleXYZTargetController::deserializeChannel( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut )
{
    return mCurves[channelIndex].deserialize( channelQuery, cloning, minFrameOut, maxFrameOut );
}

//-----------------------------------------------------------------------------
//  Name : evaluate ()
/// <summary>
/// Evaluate and retrieve the control value at the specified position / time.
/// </summary>
//-----------------------------------------------------------------------------
void cgScaleXYZTargetController::evaluate( cgDouble position, cgVector3 & s, const cgVector3 & default )
{
    if ( !mCurves[0].isEmpty() )
        s.x = mCurves[0].data.evaluateForX( (cgFloat)position, true );
    else
        s.x = default.x;
    if ( !mCurves[1].isEmpty() )
        s.y = mCurves[1].data.evaluateForX( (cgFloat)position, true );
    else
        s.y = default.y;
    if ( !mCurves[2].isEmpty() )
        s.z = mCurves[2].data.evaluateForX( (cgFloat)position, true );
    else
        s.z = default.z;
}

//-----------------------------------------------------------------------------
//  Name : addLinearKey ()
/// <summary>
/// Insert a new key-frame into the controller's animation channels to produce 
/// a linear interpolation between the previous and new keys.
/// </summary>
//-----------------------------------------------------------------------------
void cgScaleXYZTargetController::addLinearKey( cgInt32 frame, const cgVector3 & value )
{
    for ( cgInt i = 0; i < 3; ++i )
        mCurves[i].addLinearKey( frame, value[i] );
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the data associated with a specific animation channel by index.
/// Valid indices are 0=X, 1=Y, 2=Z
/// </summary>
//-----------------------------------------------------------------------------
const cgFloatCurveAnimationChannel & cgScaleXYZTargetController::getAnimationChannel( cgUInt32 index ) const
{
    static const cgFloatCurveAnimationChannel Empty;
    if ( index >= 3 )
        return Empty;
    return mCurves[index];
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the data associated with a specific animation channel by index.
/// Valid indices are 0=X, 1=Y, 2=Z
/// </summary>
//-----------------------------------------------------------------------------
cgFloatCurveAnimationChannel & cgScaleXYZTargetController::getAnimationChannel( cgUInt32 index )
{
    return mCurves[index];
}

//-----------------------------------------------------------------------------
// Name : getSupportedChannels ( )
/// <summary>
/// Retrieve an array containing the string identifiers of the animation
/// channels that this controller supports (if any) in the order that they
/// will be identified when referenced by index.
/// </summary>
//-----------------------------------------------------------------------------
const cgStringArray & cgScaleXYZTargetController::getSupportedChannels( ) const
{
    static const cgString channels[] = { _T("x"), _T("y"), _T("z"), cgString::Empty };
    static const cgStringArray channelVector( channels, channels + 3 );
    return channelVector;
}

///////////////////////////////////////////////////////////////////////////////
// cgUniformScaleTargetController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgUniformScaleTargetController () (Constructor)
/// <summary>
/// cgUniformScaleTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgUniformScaleTargetController::cgUniformScaleTargetController( )
{
}

//-----------------------------------------------------------------------------
//  Name : cgUniformScaleTargetController () (Constructor)
/// <summary>
/// cgUniformScaleTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgUniformScaleTargetController::cgUniformScaleTargetController( const cgUniformScaleTargetController & init, const cgRange & frameRange )
{
    cgBezierSpline2 & destSpline = mCurve.data;
    const cgBezierSpline2::SplinePointArray & srcPoints = init.mCurve.data.getPoints();
    for ( size_t j = 0; j < srcPoints.size(); ++j )
    {
        if ( srcPoints[j].point.x >= (cgFloat)frameRange.min && srcPoints[j].point.x <= (cgFloat)frameRange.max )
        {
            // Offset to start of range and add.
            cgBezierSpline2::SplinePoint pt = srcPoints[j];
            pt.point.x -= (cgFloat)frameRange.min;
            pt.controlPointIn.x -= (cgFloat)frameRange.min;
            pt.controlPointOut.x -= (cgFloat)frameRange.min;
            destSpline.addPoint( pt );
        
        } // End if in range

    } // Next point

    // Channel is dirty and needs serializing.
    mCurve.dirty = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgUniformScaleTargetController () (Destructor)
/// <summary>
/// cgUniformScaleTargetController Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgUniformScaleTargetController::~cgUniformScaleTargetController()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgUniformScaleTargetController::dispose( bool disposeBase )
{
    // Clean up.
    mCurve.clear();
    
    // Dispose of base classes
    if ( disposeBase )
        cgAnimationTargetController::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this animation
/// target controller and any associated child animation channels.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUniformScaleTargetController::serialize( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Call base class implementation.
        if ( !cgAnimationTargetController::serialize( targetDataId, controllerIdentifier, world, CG_NULL, 0 ) )
            return false;

        // Allow the animation channel to serialize.
        mCurve.serialize( mDatabaseId, _T("xyz"), world );

    } // End if can insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserializeChannel ()
/// <summary>
/// Import the data for the specified animation channel (referenced by index
/// matching the order from 'getSupportedChannels()') associated with this 
/// controller from the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUniformScaleTargetController::deserializeChannel( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut )
{
    return mCurve.deserialize( channelQuery, cloning, minFrameOut, maxFrameOut );
}

//-----------------------------------------------------------------------------
//  Name : evaluate ()
/// <summary>
/// Evaluate and retrieve the control value at the specified position / time.
/// </summary>
//-----------------------------------------------------------------------------
void cgUniformScaleTargetController::evaluate( cgDouble position, cgFloat & s, cgFloat default )
{
    if ( !mCurve.isEmpty() )
        s = mCurve.data.evaluateForX( (cgFloat)position, true );
    else
        s = default;
}

//-----------------------------------------------------------------------------
//  Name : addLinearKey ()
/// <summary>
/// Insert a new key-frame into the controller's animation channels to produce 
/// a linear interpolation between the previous and new keys.
/// </summary>
//-----------------------------------------------------------------------------
void cgUniformScaleTargetController::addLinearKey( cgInt32 frame, cgFloat value )
{
    mCurve.addLinearKey( frame, value );
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the keyframe data associated with this controller.
/// </summary>
//-----------------------------------------------------------------------------
const cgFloatCurveAnimationChannel & cgUniformScaleTargetController::getAnimationChannel( ) const
{
    return mCurve;
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the keyframe data associated with this controller.
/// </summary>
//-----------------------------------------------------------------------------
cgFloatCurveAnimationChannel & cgUniformScaleTargetController::getAnimationChannel( )
{
    return mCurve;
}

//-----------------------------------------------------------------------------
// Name : getSupportedChannels ( )
/// <summary>
/// Retrieve an array containing the string identifiers of the animation
/// channels that this controller supports (if any) in the order that they
/// will be identified when referenced by index.
/// </summary>
//-----------------------------------------------------------------------------
const cgStringArray & cgUniformScaleTargetController::getSupportedChannels( ) const
{
    static const cgString channels[] = { _T("xyz"), cgString::Empty };
    static const cgStringArray channelVector( channels, channels + 1 );
    return channelVector;
}

///////////////////////////////////////////////////////////////////////////////
// cgQuaternionTargetController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgQuaternionTargetController () (Constructor)
/// <summary>
/// cgQuaternionTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgQuaternionTargetController::cgQuaternionTargetController( )
{
}

//-----------------------------------------------------------------------------
//  Name : cgEulerAnglesTargetController () (Constructor)
/// <summary>
/// cgEulerAnglesTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgQuaternionTargetController::cgQuaternionTargetController( const cgQuaternionTargetController & init, const cgRange & frameRange )
{
    cgQuaternionAnimationChannel::QuaternionKeyArray & destFrames = mKeyFrames.data;
    const cgQuaternionAnimationChannel::QuaternionKeyArray & srcFrames = init.mKeyFrames.data;
    for ( size_t j = 0; j < srcFrames.size(); ++j )
    {
        if ( srcFrames[j].frame >= frameRange.min && srcFrames[j].frame <= frameRange.max )
        {
            destFrames.push_back( srcFrames[j] );

            // Offset to start of range.
            destFrames.back().frame -= frameRange.min;

        } // End if in-range

    } // Next point

    // Channel is dirty and needs serializing.
    mKeyFrames.dirty = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgQuaternionTargetController () (Destructor)
/// <summary>
/// cgQuaternionTargetController Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgQuaternionTargetController::~cgQuaternionTargetController()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgQuaternionTargetController::dispose( bool disposeBase )
{
    // Clean up.
    mKeyFrames.clear();
    
    // Dispose of base classes
    if ( disposeBase )
        cgAnimationTargetController::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this animation
/// target controller and any associated child animation channels.
/// </summary>
//-----------------------------------------------------------------------------
bool cgQuaternionTargetController::serialize( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Call base class implementation.
        if ( !cgAnimationTargetController::serialize( targetDataId, controllerIdentifier, world, CG_NULL, 0 ) )
            return false;

        // Allow the animation channel to serialize.
        mKeyFrames.serialize( mDatabaseId, _T("q"), world );

    } // End if can insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserializeChannel ()
/// <summary>
/// Import the data for the specified animation channel (referenced by index
/// matching the order from 'getSupportedChannels()') associated with this 
/// controller from the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgQuaternionTargetController::deserializeChannel( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut )
{
    return mKeyFrames.deserialize( channelQuery, cloning, minFrameOut, maxFrameOut );
}

//-----------------------------------------------------------------------------
//  Name : evaluate ()
/// <summary>
/// Evaluate and retrieve the control value at the specified position / time.
/// </summary>
//-----------------------------------------------------------------------------
void cgQuaternionTargetController::evaluate( cgDouble position, cgQuaternion & q, const cgQuaternion & default )
{
    if ( mKeyFrames.isEmpty() )
    {
        q = default;
        return;
    
    } // End if no data
    else if ( mKeyFrames.data.size() == 1 )
    {
        q = mKeyFrames.data[0].value;
        return;
    
    } // End if single key

    // Find the correct segment for this X location
    cgQuaternionAnimationChannel::QuaternionKeyArray & keys = mKeyFrames.data;
    cgInt32 first  = 0;
    cgInt32 last   = (cgInt32)keys.size() - 1;

    // Compute boundaries for the two candidate keys
    cgFloat x         = (cgFloat)position;
    cgFloat firstKeyX = (cgFloat)keys[ first ].frame;
    cgFloat lastKeyX  = (cgFloat)keys[ last ].frame;

    // Test to see if the position is out of range first of all
    if ( x <= firstKeyX )
    {
        q = cgQuaternion(keys[ first ].value);
        return;
    
    } // End if prior to first key
    else if ( x >= lastKeyX )
    {
        q = cgQuaternion(keys[ last ].value);
        return;
    
    } // End if after last key
    else
    {
        // Perform a binary search for the correct scale key starting in the middle
        cgInt32 center = (first + last) / 2;
        for ( ; ; )
        {
            // Have we reached the final two keys?
            if ( (last - first) < 2 )
            {
                const cgQuaternionAnimationChannel::QuaternionKeyFrame & k1 = keys[first];
                const cgQuaternionAnimationChannel::QuaternionKeyFrame & k2 = keys[last];
                cgInt32 segmentDist = k2.frame - k1.frame;
                if ( segmentDist < 0 )
                {
                    q = cgQuaternion( k1.value );
                    return;
                
                } // End if div0
                else
                {
                    cgFloat delta = (x - (cgFloat)k1.frame) / (cgFloat)segmentDist;
                    cgQuaternion::slerp( q, k1.value, k2.value, delta );
                    return;

                } // End if !div0

            } // End if final key
            else
            {
                // Compute x for the center key
                firstKeyX = (cgFloat)keys[ center ].frame;

                if ( x < firstKeyX )
                {
                    last   = center;
                    center = (first + last) / 2;
                
                } // End if prior to center key
                else if ( x > firstKeyX )
                {
                    first  = center;
                    center = (first + last) / 2;
                
                } // End if after center key
                else
                {
                    q = cgQuaternion( keys[ center ].value );
                    return;

                } // End if exact match

            } // End if more testing may be necessary

        } // Next Key

    } // End if not out of range
}

//-----------------------------------------------------------------------------
//  Name : addKey ()
/// <summary>
/// Insert a new key-frame into the controller's animation channels to produce 
/// a linear interpolation between the previous and new keys.
/// </summary>
//-----------------------------------------------------------------------------
void cgQuaternionTargetController::addKey( cgInt32 frame, const cgQuaternion & value )
{
    mKeyFrames.addKey( frame, value );
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the quaternion keyframe data associated with this controller.
/// </summary>
//-----------------------------------------------------------------------------
const cgQuaternionAnimationChannel & cgQuaternionTargetController::getAnimationChannel( ) const
{
    return mKeyFrames;
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the quaternion keyframe data associated with this controller.
/// </summary>
//-----------------------------------------------------------------------------
cgQuaternionAnimationChannel & cgQuaternionTargetController::getAnimationChannel( )
{
    return mKeyFrames;
}

//-----------------------------------------------------------------------------
// Name : getSupportedChannels ( )
/// <summary>
/// Retrieve an array containing the string identifiers of the animation
/// channels that this controller supports (if any) in the order that they
/// will be identified when referenced by index.
/// </summary>
//-----------------------------------------------------------------------------
const cgStringArray & cgQuaternionTargetController::getSupportedChannels( ) const
{
    static const cgString channels[] = { _T("q"), cgString::Empty };
    static const cgStringArray channelVector( channels, channels + 1 );
    return channelVector;
}

///////////////////////////////////////////////////////////////////////////////
// cgEulerAnglesTargetController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgEulerAnglesTargetController () (Constructor)
/// <summary>
/// cgEulerAnglesTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgEulerAnglesTargetController::cgEulerAnglesTargetController( cgEulerAnglesOrder::Base order /* = cgEulerAnglesOrder::YXZ */ )
{
    mRotationOrder = order;
}

//-----------------------------------------------------------------------------
//  Name : cgEulerAnglesTargetController () (Constructor)
/// <summary>
/// cgEulerAnglesTargetController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgEulerAnglesTargetController::cgEulerAnglesTargetController( const cgEulerAnglesTargetController & init, const cgRange & frameRange )
{
    mRotationOrder = init.mRotationOrder;
    for ( size_t i = 0; i < 3; ++i )
    {
        cgBezierSpline2 & destSpline = mCurves[i].data;
        const cgBezierSpline2::SplinePointArray & srcPoints = init.mCurves[i].data.getPoints();
        for ( size_t j = 0; j < srcPoints.size(); ++j )
        {
            if ( srcPoints[j].point.x >= (cgFloat)frameRange.min && srcPoints[j].point.x <= (cgFloat)frameRange.max )
            {
                // Offset to start of range and add.
                cgBezierSpline2::SplinePoint pt = srcPoints[j];
                pt.point.x -= (cgFloat)frameRange.min;
                pt.controlPointIn.x -= (cgFloat)frameRange.min;
                pt.controlPointOut.x -= (cgFloat)frameRange.min;
                destSpline.addPoint( pt );
            
            } // End if in range

        } // Next point

        // Channel is dirty and needs serializing.
        mCurves[i].dirty = true;
    
    } // Next channel
}

//-----------------------------------------------------------------------------
//  Name : ~cgEulerAnglesTargetController () (Destructor)
/// <summary>
/// cgEulerAnglesTargetController Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgEulerAnglesTargetController::~cgEulerAnglesTargetController()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgEulerAnglesTargetController::dispose( bool disposeBase )
{
    // Clean up.
    mCurves[0].clear();
    mCurves[1].clear();
    mCurves[2].clear();

    // Dispose of base classes
    if ( disposeBase )
        cgAnimationTargetController::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this animation
/// target controller and any associated child animation channels.
/// </summary>
//-----------------------------------------------------------------------------
bool cgEulerAnglesTargetController::serialize( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Call base class implementation.
        cgUInt32 customData = (cgUInt32)mRotationOrder;
        if ( !cgAnimationTargetController::serialize( targetDataId, controllerIdentifier, world, &customData, sizeof(cgUInt32) ) )
            return false;

        // Allow each of the animation channels to serialize.
        mCurves[0].serialize( mDatabaseId, _T("x"), world );
        mCurves[1].serialize( mDatabaseId, _T("y"), world );
        mCurves[2].serialize( mDatabaseId, _T("z"), world );

    } // End if can insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserialize ()
/// <summary>
/// Import the data associated with this animation target controller based on
/// the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgEulerAnglesTargetController::deserialize( cgWorldQuery & controllerQuery, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut, void *& customDataOut, cgUInt32 & customDataSizeOut )
{
    if ( !cgAnimationTargetController::deserialize( controllerQuery, cloning, minFrameOut, maxFrameOut, customDataOut, customDataSizeOut ) )
        return false;

    // Load custom data.
    if ( customDataOut && customDataSizeOut == sizeof(cgUInt32) )
        mRotationOrder = (cgEulerAnglesOrder::Base)(*(cgUInt32*)customDataOut);

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserializeChannel ()
/// <summary>
/// Import the data for the specified animation channel (referenced by index
/// matching the order from 'getSupportedChannels()') associated with this 
/// controller from the supplied query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgEulerAnglesTargetController::deserializeChannel( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut )
{
    return mCurves[channelIndex].deserialize( channelQuery, cloning, minFrameOut, maxFrameOut );
}

//-----------------------------------------------------------------------------
//  Name : evaluate ()
/// <summary>
/// Evaluate and retrieve the control value at the specified position / time.
/// </summary>
//-----------------------------------------------------------------------------
void cgEulerAnglesTargetController::evaluate( cgDouble position, cgEulerAngles & e, const cgEulerAngles & default )
{
    if ( !mCurves[0].isEmpty() )
        e.x = mCurves[0].data.evaluateForX( (cgFloat)position, true );
    else
        e.x = default.x;
    if ( !mCurves[1].isEmpty() )
        e.y = mCurves[1].data.evaluateForX( (cgFloat)position, true );
    else
        e.y = default.y;
    if ( !mCurves[2].isEmpty() )
        e.z = mCurves[2].data.evaluateForX( (cgFloat)position, true );
    else
        e.z = default.z;

    // Set the rotation order for the angles.
    e.order = mRotationOrder;
}

//-----------------------------------------------------------------------------
//  Name : evaluate ()
/// <summary>
/// Evaluate and retrieve the control value at the specified position / time.
/// </summary>
//-----------------------------------------------------------------------------
void cgEulerAnglesTargetController::evaluate( cgDouble position, cgEulerAngles & e, const cgQuaternion & default )
{
    cgInt nDefaults = 0;
    if ( !mCurves[0].isEmpty() )
        e.x = mCurves[0].data.evaluateForX( (cgFloat)position, true );
    else
        nDefaults |= 1;
    if ( !mCurves[1].isEmpty() )
        e.y = mCurves[1].data.evaluateForX( (cgFloat)position, true );
    else
        nDefaults |= 2;
    if ( !mCurves[2].isEmpty() )
        e.z = mCurves[2].data.evaluateForX( (cgFloat)position, true );
    else
        nDefaults |= 4;

    // default any which were not available.
    if ( nDefaults )
    {
        // Convert default quaternion to euler only as necessary.
        cgEulerAngles defaultEuler( default, mRotationOrder );

        // Some keys were unavailable, apply defaults.
        if ( nDefaults & 1 )
            e.x = defaultEuler.x;
        if ( nDefaults & 2 )
            e.y = defaultEuler.y;
        if ( nDefaults & 4 )
            e.z = defaultEuler.z;

    } // End if default

    // Set the rotation order for the angles.
    e.order = mRotationOrder;
}

//-----------------------------------------------------------------------------
//  Name : addLinearKey ()
/// <summary>
/// Insert a new key-frame into the controller's animation channels to produce 
/// a linear interpolation between the previous and new keys.
/// </summary>
//-----------------------------------------------------------------------------
void cgEulerAnglesTargetController::addLinearKey( cgInt32 frame, const cgEulerAngles & value )
{
    for ( cgInt i = 0; i < 3; ++i )
        mCurves[i].addLinearKey( frame, value[i] );
}

//-----------------------------------------------------------------------------
//  Name : addLinearKey ()
/// <summary>
/// Insert a new key-frame into the controller's animation channels to produce 
/// a linear interpolation between the previous and new keys.
/// </summary>
//-----------------------------------------------------------------------------
void cgEulerAnglesTargetController::addLinearKey( cgInt32 frame, const cgQuaternion & value )
{
    cgEulerAngles e( value );
    for ( cgInt i = 0; i < 3; ++i )
    {
        // In order to correctly synchronize rotations converted from a quaternion
        // we need to avoid issues when the angle wraps around 360->0 degrees.
        // In addition, we want to make sure that we can support a continuous rotation
        // of larger than 360 degrees. Try and 'estimate' a good value for this key 
        // in comparison to the previous key to ensure a reasonable rotation.
        if ( !mCurves[i].isEmpty() )
        {
            const cgBezierSpline2::SplinePoint & pt = mCurves[i].data.getPoints().back();
            while ( fabsf(e[i] - pt.point.y) > CGE_PI )
                e[i] = (pt.point.y < e[i]) ? (e[i] - CGE_TWO_PI) : (e[i] + CGE_TWO_PI);

        } // End if has prior keys

        // Add the control point
        mCurves[i].addLinearKey( frame, e[i] );

    } // Next curve
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the data associated with a specific animation channel by index.
/// Valid indices are 0=X, 1=Y, 2=Z
/// </summary>
//-----------------------------------------------------------------------------
const cgFloatCurveAnimationChannel & cgEulerAnglesTargetController::getAnimationChannel( cgUInt32 index ) const
{
    static const cgFloatCurveAnimationChannel Empty;
    if ( index >= 3 )
        return Empty;
    return mCurves[index];
}

//-----------------------------------------------------------------------------
//  Name : getAnimationChannel ()
/// <summary>
/// Retrieve the data associated with a specific animation channel by index.
/// Valid indices are 0=X, 1=Y, 2=Z
/// </summary>
//-----------------------------------------------------------------------------
cgFloatCurveAnimationChannel & cgEulerAnglesTargetController::getAnimationChannel( cgUInt32 index )
{
    return mCurves[index];
}

//-----------------------------------------------------------------------------
// Name : getSupportedChannels ( )
/// <summary>
/// Retrieve an array containing the string identifiers of the animation
/// channels that this controller supports (if any) in the order that they
/// will be identified when referenced by index.
/// </summary>
//-----------------------------------------------------------------------------
const cgStringArray & cgEulerAnglesTargetController::getSupportedChannels( ) const
{
    static const cgString channels[] = { _T("x"), _T("y"), _T("z"), cgString::Empty };
    static const cgStringArray channelVector( channels, channels + 3 );
    return channelVector;
}