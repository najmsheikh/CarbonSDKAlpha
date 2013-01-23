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
// Name : AlienActor.gs                                                      //
//                                                                           //
// Desc : Behavior script associated with the actor designed to represent    //
//        the alien enemy NPC.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Enumerations
//-----------------------------------------------------------------------------
shared enum AnimationState
{
    Idle,   // We're currently idling
    Moving  // We're currently moving
};

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : AlienActor (Class)
// Desc : Behavior script associated with the actor designed to represent
//        the alien enemy NPC.
//-----------------------------------------------------------------------------
shared class AlienActor : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ActorNode@      mActor;                 // The actor to which we are attached.
    private AnimationState  mState;                 // Current state of the character (idling, walking, etc.)


	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : AlienActor () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	AlienActor( )
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
        // Initialize variables
        mState = AnimationState::Idle;
        
        // Cache references to required objects.
        @mActor = cast<ActorNode>(object);

        // Set initial animation
        if ( @mActor != null )
            mActor.playAnimationSet( "Primary", "Idle" );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        @mActor = null;
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Do nothing if we are not attached.
        if ( @mActor == null )
            return;

        AnimationController @ controller = mActor.getAnimationController();
        Vector3 velocity = mActor.getVelocity();
        float   speed    = vec3Dot( velocity, mActor.getZAxis() );

        // Act appropriately based on the current character state.
        switch ( mState )
        {
            case AnimationState::Idle:

                // Use idle animation
                mActor.playAnimationSet( "Primary", "Idle" );
                
                // Switching to move?
                if ( speed > 0.4f )
                    mState = AnimationState::Moving;
                break;

            case AnimationState::Moving:
                
                // Use walk/run animation.
                if ( speed >= 4 )
                    mActor.playAnimationSet( "Primary", "Run", speed / 10, 0 );
                else
                    mActor.playAnimationSet( "Primary", "Walk", speed / 2, 0 );

                // Switching to idle?
                if ( speed <= 0.4f )
                    mState = AnimationState::Idle;
                break;

        } // End switch state
	}

} // End Class AlienActor