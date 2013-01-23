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
// Name : AdvanceFiringState.gsh                                             //
//                                                                           //
// Desc : NPC agent is firing whilst advancing.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../AgentState.gsh"
#include_once "../NPCAgent.gsh"
#include_once "FiringState.gs"
#include_once "AdvancingState.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : AdvanceFiringState (Class)
// Desc : NPC agent is firing whilst advancing.
//-----------------------------------------------------------------------------
shared class AdvanceFiringState : FiringState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AdvancingState@ mSubState;   // The advancing state we should continue to update.

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : AdvanceFiringState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	AdvanceFiringState( Agent @ agent, AdvancingState @ subState )
    {
        // Call base class constructor
        super( agent, subState );

        // Initialize variables
        @mSubState = subState;
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
        // Call base class implementation first
        if ( !FiringState::begin( from ) )
            return false;

        // We can only come from Advancing state, so nothing else to do here.
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
        if ( @mNPC.getState() != @this )
            return;

        // Allow sub state (advancing state) to update
        mSubState.update( elapsedTime );
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Advancing & Firing";
    }
}