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
// Name : PartObjective.gs                                                   //
//                                                                           //
// Desc : Objective that must be activated by the player to progress.        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "Objective.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : PartObjective (Class)
// Desc : Objective that must be activated by the player to progress.
//-----------------------------------------------------------------------------
class PartObjective : Objective
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : PartObjective () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	PartObjective( )
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
        // Get properties
        PropertyContainer @ properties = object.getCustomProperties();

        // Don't run update process until we're ready.
        object.setUpdateRate( UpdateRate::Never );

        // Get the currently active state (GamePlay)
        AppStateManager @ stateManager = getAppStateManager();
        AppState @ state = stateManager.getActiveState();
        if ( @state != null )
        {
            @mGamePlayState = cast<GamePlay>( state.getScriptObject() );
        
        } // End if active

        // Call base class implementation last.
        Objective::onAttach( object );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release references.
        @mGamePlayState = null;

        // Call base class implementation last
        Objective::onDetach( object );
    }

    //-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (Objective)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : onActivate () (Event)
	// Desc : This objective was activated. Returns new objective (if any)
	//-------------------------------------------------------------------------
    Objective @ onActivate( ObjectNode @ issuer )
    {
        // Trigger this gameplay sequence.
        mGamePlayState.setCurrentGameSequence( mSequenceIdentifier );

        // Switch to next objective
        if ( mNextObjectiveId != 0 )
        {
            Scene @ scene = mNode.getScene();
            ObjectNode @ objectiveNode = scene.getObjectNodeById( mNextObjectiveId );
            if ( objectiveNode.getBehaviorCount() > 0 )
                return cast<Objective>(objectiveNode.getScriptedBehavior(0));

        } // End if has other objective

        // No new objective.
        return null;
    }
}