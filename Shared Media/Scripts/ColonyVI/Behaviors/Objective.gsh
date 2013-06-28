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
// Name : Objective.gsh                                                      //
//                                                                           //
// Desc : Base classes and core script logic for player objectives and user  //
//        activated game event triggers.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../States/GamePlay.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Objective (Base Class)
// Desc : Base classes and core script logic for player objectives and user
//        activated game event triggers.
//-----------------------------------------------------------------------------
shared class Objective : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private GamePlay@       mGamePlayState;         // Top level application state driving gameplay.
    private ObjectNode@     mNode;                  // The node to which we are attached.

    // Objective description.
    private float           mMaxTriggerRange;       // Maximum range at which this objective can be triggered (<= 0 cannot be triggered).
    private uint            mNextObjectiveId;       // Reference ID of next objective.
    private String          mSequenceIdentifier;    // Name of the game-play sequence triggered when the objective is activated.

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Objective () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Objective( )
    {
    }

    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedObjectBehavior)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : onAttach () (Event)
	// Desc : Called by the application when the behavior is first attached to
	//        an object and can initialize.
	//-------------------------------------------------------------------------
	void onAttach( ObjectNode @ object )
	{
        // Cache references to required objects.
        @mNode = object;

        // Get properties
        mMaxTriggerRange = float(object.getCustomProperty( "trigger_range", 2.0f ));
        mSequenceIdentifier = String(object.getCustomProperty( "sequence_id", Variant("") ));
        mNextObjectiveId = uint(object.getCustomProperty( "next_objective", 0 ));

        // Get the currently active state (GamePlay)
        AppStateManager @ stateManager = getAppStateManager();
        AppState @ state = stateManager.getState( "GamePlay" );
        if ( @state != null )
            @mGamePlayState = cast<GamePlay>( state.getScriptObject() );

	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        @mNode = null;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : getSceneNode ()
	// Desc : Get the scene node representing this objective.
	//-------------------------------------------------------------------------
    ObjectNode@ getSceneNode( )
    {
        return mNode;
    }

    //-------------------------------------------------------------------------
	// Name : isObjectiveInRange ()
	// Desc : Is the objective in range for triggering?
	//-------------------------------------------------------------------------
    bool isObjectiveInRange( Vector3 sourcePosition )
    {
        if ( mMaxTriggerRange <= 0 )
            return false;
        return (vec3LengthSq( mNode.getPosition() - sourcePosition ) <= (mMaxTriggerRange * mMaxTriggerRange));
    }

    //-------------------------------------------------------------------------
	// Name : distanceToObjective ()
	// Desc : Retrieve the current distance to the objective (in meters).
	//-------------------------------------------------------------------------
    float distanceToObjective( Vector3 sourcePosition )
    {
        return vec3Length( mNode.getPosition() - sourcePosition );
    }

    //-------------------------------------------------------------------------
	// Name : onActivate () (Event)
	// Desc : This objective was activated. Returns new objective (if any)
	//-------------------------------------------------------------------------
    Objective @ onActivate( ObjectNode @ issuer )
    {
        // Nothing in base implementation.
        return null;
    }
}