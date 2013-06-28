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
// Name : RepositionFiringState.gsh                                          //
//                                                                           //
// Desc : NPC agent is firing whilst repositioning.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../AgentState.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : RepositionFiringState (Class)
// Desc : NPC agent is firing whilst repositioning.
//-----------------------------------------------------------------------------
shared class RepositionFiringState : FiringState
{
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : RepositionFiringState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	RepositionFiringState( NPCAgent @ agent )
    {
        // Call base class constructor
        super( agent );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : begin ()
	// Desc : State has been selected and is about to begin. Return false
    //        to cancel state switch, and call 'Agent::switchState()' to select 
    //        the actual state you want to switch to (unless you want the 
    //        current state to remain).
	//-------------------------------------------------------------------------
	bool begin( AgentState @ from )
	{
        // We need our parent to continue to run.
        @mParentState = from;

        // Call base class implementation first
        if ( !FiringState::begin( from ) )
            return false;

        // We can only come from Repositioning state, so nothing else to do here.
        return true;
	}

    //-------------------------------------------------------------------------
	// Name : end ()
	// Desc : State has been deselected and is about to end.
	//-------------------------------------------------------------------------
    void end( AgentState @ to )
    {
        FiringState::end( to );
    }

    //-------------------------------------------------------------------------
	// Name : update ()
	// Desc : Allow the state to perform its update processing.
	//-------------------------------------------------------------------------
    void update( float elapsedTime )
    {
        FiringState::update( elapsedTime );

        // Base state switched away?
        if ( @mAgent.getState() != @this )
            return;

        // Allow parent (repositioning state) to update
        mParentState.update( elapsedTime );
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Repositioning & Firing";
    }

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::RepositionFiring;
    }
}