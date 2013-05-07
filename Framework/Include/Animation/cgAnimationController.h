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
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
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
#include <Animation/cgAnimationTypes.h>

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
    void                            advanceTime         ( cgDouble timeDelta, const TargetMap & targets );
    void                            resetTime           ( );
    void                            setTrackLimit       ( cgUInt16 maxTracks );
    bool                            setTrackAnimationSet( cgUInt16 track, const cgAnimationSetHandle & set );
    bool                            setTrackWeight      ( cgUInt16 track, cgFloat weight );
    bool                            setTrackSpeed       ( cgUInt16 track, cgFloat speed );
    bool                            setTrackPosition    ( cgUInt16 track, cgDouble position );
    bool                            setTrackFrameLimits ( cgUInt16 track, cgInt32 minFrame, cgInt32 maxFrame );
    bool                            setTrackFrameLimits ( cgUInt16 track, const cgRange & range );
    bool                            setTrackPlaybackMode( cgUInt16 track, cgAnimationPlaybackMode::Base mode );
    bool                            setTrackEnabled     ( cgUInt16 track, bool enabled );
    bool                            setTrackDesc        ( cgUInt16 track, const cgAnimationTrackDesc & desc );

    cgDouble                        getTime             ( ) const;
    cgUInt16                        getTrackLimit       ( ) const;
    const cgAnimationSetHandle    & getTrackAnimationSet( cgUInt16 track ) const;
    cgFloat                         getTrackWeight      ( cgUInt16 track ) const;
    cgFloat                         getTrackSpeed       ( cgUInt16 track ) const;
    cgDouble                        getTrackPosition    ( cgUInt16 track ) const;
    cgRange                         getTrackFrameLimits ( cgUInt16 track ) const;
    cgAnimationPlaybackMode::Base   getTrackPlaybackMode( cgUInt16 track ) const;
    bool                            getTrackEnabled     ( cgUInt16 track ) const;
    const cgAnimationTrackDesc    & getTrackDesc        ( cgUInt16 track ) const;
    

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSceneController)
    //-------------------------------------------------------------------------
    virtual void                    update              ( cgFloat timeDelta );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
	// Protected Typedefs, Structures and Enumerations
	//-------------------------------------------------------------------------
    struct Track
    {
        cgAnimationTrackDesc    desc;   // The descriptor that contains all of the track properties.
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