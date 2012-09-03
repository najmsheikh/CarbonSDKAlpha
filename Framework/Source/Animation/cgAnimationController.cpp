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
// Name : cgAnimationController.cpp                                          //
//                                                                           //
// Desc : Basic animation controller classes, provides support for animating //
//        any registered animation target based on either loaded file data,  //
//        or custom controller / script generated keys.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAnimationController Module Includes
//-----------------------------------------------------------------------------
#include <Animation/cgAnimationController.h>
#include <Animation/cgAnimationTarget.h>
#include <Resources/cgAnimationSet.h>
#include <Math/cgMathTypes.h>

///////////////////////////////////////////////////////////////////////////////
// cgAnimationController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAnimationController () (Constructor)
/// <summary>
/// cgAnimationController Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationController::cgAnimationController( ) : cgScriptInterop::DisposableScriptObject( )
{
    // Set variables to sensible defaults
    mPosition = 0.0f;

    // By default, we allocate space for one track
    mTracks.resize( 1 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgAnimationController () (Destructor)
/// <summary>
/// cgAnimationController Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationController::~cgAnimationController()
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
void cgAnimationController::dispose( bool bDisposeBase )
{
    // Clear containers
    mValidTargetIds.clear();
    mTracks.clear(); 
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// cgSceneController 'update' functionality. Simply advances time
/// automatically while enabled.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationController::update( cgFloat fTimeElapsed )
{
    advanceTime( fTimeElapsed, TargetMap() );
}

//-----------------------------------------------------------------------------
//  Name : advanceTime ()
/// <summary>
/// Advance the animation playhead by the specified amount (in seconds)
/// and update any relevant animation targets.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationController::advanceTime( cgDouble fTimeElapsed, const TargetMap & Targets )
{
    // Advance the controller's internal playhead
    mPosition += fTimeElapsed;

    // Iterate through each available track
    TrackArray::iterator itTrack;
    for ( itTrack = mTracks.begin(); itTrack != mTracks.end(); ++itTrack )
    {
        Track & Item = *itTrack;

        // Move the track to the new position
        if ( Item.set.isValid() && Item.desc.enabled )
            Item.desc.position += fTimeElapsed * Item.desc.speed;

    } // Next playing track

    // Finally update the registered animation targets
    updateTargets( Targets );
}

//-----------------------------------------------------------------------------
//  Name : updateTargets () (Protected)
/// <summary>
/// Update all of the registered animation targets based on the animation
/// sets currently applied to the tracks.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationController::updateTargets( const TargetMap & Targets )
{
    cgFloat      fWeightTheta = 0.0f;
    cgVector3    vecScale, vecTranslation;
    cgQuaternion qtRotation;
    size_t       i;
    
    // Resize vectors to store enough data for each track
    std::vector<cgTransform> TrackTransforms( mTracks.size() );
    std::vector<cgFloat> TrackWeights( mTracks.size(), 0.0f );

    // Iterate through each applied animation target
    for ( cgStringSet::iterator itTargetId = mValidTargetIds.begin(); itTargetId != mValidTargetIds.end(); ++itTargetId )
    {
        // Find the matching supplied animation target instance.
        TargetMap::const_iterator itTarget = Targets.find( *itTargetId );
        if ( itTarget == Targets.end() )
            continue;
        cgAnimationTarget * pTarget = itTarget->second;
        
        // Reset normalizing "length" value.
        fWeightTheta = 0.0f;

        // Iterate through each available track and determine which sets (if any)
        // contain appropriate animation data for this target.
        for ( i = 0; i < mTracks.size(); ++i )
        {
            Track & Item = mTracks[i];
            
            // Skip if the track is disabled.
            if ( !Item.desc.enabled || !Item.desc.weight || !Item.set.isValid() )
            {
                TrackWeights[i] = 0.0f;
                continue;
            
            } // End if disabled

            // Retrieve track SRT data.
            cgAnimationSet * pSet = Item.set.getResource(true);
            if ( pSet->getSRT( Item.desc.position * (cgDouble)pSet->getFrameRate(), *itTargetId, 
                               Item.desc.firstFrame, Item.desc.lastFrame, vecScale, qtRotation, vecTranslation ) )
            {
                //fWeightTheta   += Item.desc.weight * Item.desc.weight;
                fWeightTheta   += Item.desc.weight;
                TrackWeights[i] = Item.desc.weight;

                // Generate the matrix from these values
                TrackTransforms[i].compose( vecScale,cgVector3( 0, 0, 0 ), qtRotation, vecTranslation );

            } // End if retrieved SRT data
            else
            {
                TrackWeights[i] = 0;
            
            } // End if no SRT

        } // Next playing track

        // Generate the blended transform if any data found
        if ( fWeightTheta > CGE_EPSILON )
        {
            cgTransform t;

            // Allows us to multiply instead of divide
            fWeightTheta = 1.0f / fWeightTheta;

            // Check each track.
            t.zero();
            for ( i = 0; i < mTracks.size(); ++i )
            {
                cgFloat fWeight = TrackWeights[i];

                // Track plays any part?
                if ( fWeight > CGE_EPSILON )
                    t += TrackTransforms[i] * (fWeight * fWeightTheta);
                
            } // Next playing track

            // Apply this to the animation target
            pTarget->onAnimationTransformUpdated( t );

        } // End if found data

    } // Next Animation Target
}

//-----------------------------------------------------------------------------
//  Name : resetTime ()
/// <summary>
/// Reset the global playhead time to zero.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationController::resetTime( )
{
    // Reset the controller's internal playhead
    mPosition = 0;
}

//-----------------------------------------------------------------------------
//  Name : getTime ()
/// <summary>
/// Retrieve the current time (in seconds) of the animation playhead.
/// </summary>
//-----------------------------------------------------------------------------
cgDouble cgAnimationController::getTime( ) const
{
    return mPosition;
}

//-----------------------------------------------------------------------------
//  Name : setTrackLimit ()
/// <summary>
/// Set the maximum number of tracks that will be required by this
/// animation controller (defaults to 1)
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationController::setTrackLimit( cgUInt16 nMaxTracks )
{
    // Set the size of the track container
    if ( nMaxTracks == (cgUInt16)mTracks.size() )
        return;
    mTracks.resize( nMaxTracks );
}

//-----------------------------------------------------------------------------
//  Name : setTrackAnimationSet ()
/// <summary>
/// Place the specified animation set in the required track for playback.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationController::setTrackAnimationSet( cgUInt16 nTrack, const cgAnimationSetHandle & hSet )
{
    // Track index valid?
    if ( nTrack >= mTracks.size() )
        return false;

    // If this is a no-op, skip.
    if ( hSet == mTracks[nTrack].set )
        return true;

    // Swap the animation set
    mTracks[ nTrack ].set = hSet;

    // Build a unique set of targets identifiers from all applied animation sets.
    mValidTargetIds.clear();
    for ( size_t i = 0; i < mTracks.size(); ++i )
    {
        cgAnimationSet * pSet = mTracks[i].set.getResource( true );
        if ( pSet )
        {
            // Get a list of the registered targets and add any unique entries to our 
            // list of currently *valid* target instance identifiers.
            cgAnimationSet::TargetDataMap::const_iterator itTarget;
            const cgAnimationSet::TargetDataMap & TargetData = pSet->getTargetData();
            for ( itTarget = TargetData.begin(); itTarget != TargetData.end(); ++itTarget )
                mValidTargetIds.insert( itTarget->first );

        } // End if valid set

    } // Next track

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getTrackAnimationSet ()
/// <summary>
/// Retrieve the animation set that is currently bound to the specified track 
/// for playback.
/// </summary>
//-----------------------------------------------------------------------------
const cgAnimationSetHandle & cgAnimationController::getTrackAnimationSet( cgUInt16 nTrack ) const
{
    if ( nTrack >= mTracks.size() )
        return cgAnimationSetHandle::Null;
    return mTracks[nTrack].set;
}

//-----------------------------------------------------------------------------
//  Name : setTrackWeight ()
/// <summary>
/// Set the blending weight of the specified track (i.e. how much it will
/// contribute to the final controller output)
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationController::setTrackWeight( cgUInt16 nTrack, cgFloat fWeight )
{
    // Track index valid?
    if ( nTrack >= mTracks.size() )
        return false;

    // Set the property
    mTracks[ nTrack ].desc.weight = fWeight;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setTrackSpeed ()
/// <summary>
/// Set the playback speed of this track (1.0 is normal speed).
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationController::setTrackSpeed( cgUInt16 nTrack, cgFloat fSpeed )
{
    // Track index valid?
    if ( nTrack >= mTracks.size() )
        return false;

    // Set the property
    mTracks[ nTrack ].desc.speed = fSpeed;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setTrackPosition ()
/// <summary>
/// Set the current position (in seconds) of the track playhead. This
/// value is independant from the global time / position.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationController::setTrackPosition( cgUInt16 nTrack, cgDouble fPosition )
{
    // Track index valid?
    if ( nTrack >= mTracks.size() )
        return false;

    // Set the property
    mTracks[ nTrack ].desc.position = fPosition;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getTrackPosition ()
/// <summary>
/// Get the current position (in seconds) of the track playhead. This
/// value is independant from the global time / position.
/// </summary>
//-----------------------------------------------------------------------------
cgDouble cgAnimationController::getTrackPosition( cgUInt16 nTrack ) const
{
    // Track index valid?
    if ( nTrack >= mTracks.size() )
        return 0;
    return mTracks[ nTrack ].desc.position;
}

//-----------------------------------------------------------------------------
//  Name : setTrackFrameLimits ()
/// <summary>
/// Indices of the frames to limit the animation set sampler to. Specify both
/// as 0x7FFFFFFF to disable limits.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationController::setTrackFrameLimits( cgUInt16 nTrack, cgInt32 nMinFrame, cgInt32 nMaxFrame )
{
    // Track index valid?
    if ( nTrack >= mTracks.size() )
        return false;

    // Set the property
    mTracks[ nTrack ].desc.firstFrame = nMinFrame;
    mTracks[ nTrack ].desc.lastFrame = nMaxFrame;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setTrackDesc ()
/// <summary>
/// Set all of the track playback properties in one go.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationController::setTrackDesc( cgUInt16 nTrack, const TrackDesc & Desc )
{
    // Track index valid?
    if ( nTrack >= mTracks.size() )
        return false;

    cgToDo( "Animation", "Test for no-op, and update valid targets if it's a new set." )

    // Set the property
    mTracks[ nTrack ].desc = Desc;

    // Success!
    return true;
}