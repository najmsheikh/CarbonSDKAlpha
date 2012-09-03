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
// Name : cgAnimationController.h                                            //
//                                                                           //
// Desc : Basic animation controller classes, provides support for animating //
//        any registered animation target based on either loaded file data,  //
//        or custom controller / script generated keys.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGANIMATIONCONTROLLER_H_ )
#define _CGE_CGANIMATIONCONTROLLER_H_

//-----------------------------------------------------------------------------
// cgAnimationController Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declaration
//-----------------------------------------------------------------------------
class cgAnimationTarget;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAnimationController (Class)
/// <summary>
/// Basic animation controller class, provides support for animating any
/// registered animation target based on either loaded file data, or
/// custom controller / script generated keys.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAnimationController : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgAnimationController, "AnimationController" )

public:
    //-------------------------------------------------------------------------
	// Public Typedefs, Structures and Enumerations
	//-------------------------------------------------------------------------
    struct CGE_API TrackDesc
    {
        cgDouble        position;   // Current position of this track in seconds.
        cgFloat         weight;     // Current blending weight of this track.
        cgFloat         speed;      // Rate at which the track should play.
        cgInt32         firstFrame; // Minimum key frame index to limit the animation set sampler.
        cgInt32         lastFrame;  // Maximum key frame index to limit the animation set sampler.
        bool            enabled;    // Track is currently playing back (will still contribute even when false, just will not advance)

        // Constructor
        TrackDesc() :
            position(0), speed(1), weight(1), 
            firstFrame(0x7FFFFFFF), lastFrame(0x7FFFFFFF), enabled(true) {}
    
    }; // End struct TrackDesc

    // Use to indicate a mapping of instance identifier to target.
    CGE_MAP_DECLARE( cgString, cgAnimationTarget*, TargetMap )

    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgAnimationController( );
	virtual ~cgAnimationController( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                        advanceTime         ( cgDouble timeDelta, const TargetMap & targets );
    cgDouble                    getTime             ( ) const;
    void                        resetTime           ( );
    void                        setTrackLimit       ( cgUInt16 maxTracks );
    bool                        setTrackAnimationSet( cgUInt16 track, const cgAnimationSetHandle & set );
    const cgAnimationSetHandle& getTrackAnimationSet( cgUInt16 track ) const;
    bool                        setTrackWeight      ( cgUInt16 track, cgFloat weight );
    bool                        setTrackSpeed       ( cgUInt16 track, cgFloat speed );
    bool                        setTrackPosition    ( cgUInt16 track, cgDouble position );
    cgDouble                    getTrackPosition    ( cgUInt16 track ) const;
    bool                        setTrackFrameLimits ( cgUInt16 track, cgInt32 minFrame, cgInt32 maxFrame );
    bool                        setTrackDesc        ( cgUInt16 track, const TrackDesc & desc );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSceneController)
    //-------------------------------------------------------------------------
    virtual void                update              ( cgFloat timeDelta );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
	// Protected Typedefs, Structures and Enumerations
	//-------------------------------------------------------------------------
    struct Track
    {
        TrackDesc               desc;   // The descriptor that contains all of the track properties.
        cgAnimationSetHandle    set;    // The animation set currently applied to this track.
    
    }; // End struct Track
    CGE_VECTOR_DECLARE      (Track, TrackArray)

    //-------------------------------------------------------------------------
	// Protected Methods
	//-------------------------------------------------------------------------
    void                updateTargets       ( const TargetMap & targets );

    //-------------------------------------------------------------------------
	// Protected Variables
	//-------------------------------------------------------------------------
    TrackArray      mTracks;            // Vector containing all of the tracks currently set to the controller.
    cgDouble        mPosition;          // Position (in seconds) of the animation playhead.
    cgStringSet     mValidTargetIds;    // List of all currently valid target identifiers for the applied tracks.
};

#endif // !_CGE_CGANIMATIONCONTROLLER_H_