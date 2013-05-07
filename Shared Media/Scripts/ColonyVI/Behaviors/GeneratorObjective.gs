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
// Name : GeneratorObjective.gs                                              //
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
// Name : GeneratorObjective (Class)
// Desc : Objective that must be activated by the player to progress.
//-----------------------------------------------------------------------------
shared class GeneratorObjective : Objective
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private uint                    mCoverRefId;        // Reference ID of the cover that drops down when the coolant supply is shut down
    private ObjectNode@             mCover;             // Cached reference to the cover object node
    private uint                    mSteamRefId;        // Reference ID of the steam particle emitter that is enabled coolant supply is shut down
    private ParticleEmitterNode@    mSteam;             // Cached reference to the emitter.
    private float                   mSlideAmount;       // Amount that the cover has been moved so far.

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : GeneratorObjective () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	GeneratorObjective( )
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
        mCoverRefId = uint(properties.getProperty( "cover_refid", uint(0) ) );
        mSteamRefId = uint(properties.getProperty( "emitter_refid", uint(0) ) );
        
        // Cache references to objects
        Scene @ scene = object.getScene();
        if ( mCoverRefId != 0 )
            @mCover = scene.getObjectNodeById( mCoverRefId );
        if ( mSteamRefId != 0 )
            @mSteam = cast<ParticleEmitterNode>(scene.getObjectNodeById( mSteamRefId ) );

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
        // Call base class implementation last
        Objective::onDetach( object );
    }
    
    //-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Slide the cover down.
        if ( @mCover != null )
        {
            const float CoverMaxDistance = 1.67f;

            // Compute amount to open.
            float moveAmount = elapsedTime * 0.5f;
            mSlideAmount += moveAmount;
            if ( mSlideAmount >  CoverMaxDistance )
            {
                moveAmount -= (mSlideAmount - CoverMaxDistance);
                mSlideAmount = CoverMaxDistance;
            
            } // End if overshoots

            // Adjust cover
            mCover.moveLocal( 0, -moveAmount, 0 );

            // Done sliding?
            if ( mSlideAmount >= CoverMaxDistance )
                mNode.setUpdateRate( UpdateRate::Never );

        } // End if has cover
        else
            mNode.setUpdateRate( UpdateRate::Never );
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

        // Enable updates for this objective so we can slide the cover down.
        if ( @mCover != null )
            mNode.setUpdateRate( UpdateRate::Always );
        mSlideAmount = 0;

        // Enable updates to particle emitter.
        if ( @mSteam != null )
        {
            mSteam.setUpdateRate( UpdateRate::Always );
            mSteam.enableLayerEmission( 0, true );
        
        } // End if has steam

        // Get the next objective.
        if ( mNextObjectiveId != 0 )
        {
            Scene @ scene = mNode.getScene();
            ObjectNode @ objectiveNode = scene.getObjectNodeById( mNextObjectiveId );
            if ( objectiveNode.getBehaviorCount() > 0 )
            {
                Objective @ nextObjective = cast<Objective>(objectiveNode.getScriptedBehavior(0));
                if ( mSequenceIdentifier == "trigger_generator_4" )
                {
                    // Trigger the next objective (elevator doors will close).
                    nextObjective.onActivate( issuer );
                
                } // End if final trigger
                return nextObjective;

            } // End if has script
        
        } // End if has other objective

        // No new objective
        return null;
    }
}