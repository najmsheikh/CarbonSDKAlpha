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
#include_once "DestructibleLight.gs"
#include_once "EmergencyLight.gs"

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
    private int     mSequenceIndex; // Which generator?
    private uint    mNextObjective; // Reference ID of next objective.
    
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
        mSequenceIndex = int(properties.getProperty( "sequence_index" ));
        mNextObjective = uint(properties.getProperty( "next_objective", uint(0) ));

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

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (Objective)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : onActivate () (Event)
	// Desc : This objective was activated. Returns new objective (if any)
	//-------------------------------------------------------------------------
    Objective @ onActivate( ObjectNode @ issuer )
    {
        // If this is the final generator objective, deactivate hall 
        // lights and activate the emergency lights.
        if ( mSequenceIndex == 3 )
        {
            // First, hallway lights off.
            Scene @ scene = mNode.getScene();
            array<ObjectNode@> nodes = scene.getObjectNodesByType( RTID_MeshObject );
            for ( uint i = 0; i < nodes.length(); ++i )
            {
                ObjectNode @ node = nodes[i];
                if ( node.getBehaviorCount() == 0 )
                    continue;
                DestructibleLight @ light = cast<DestructibleLight>(node.getScriptedBehavior(0));
                if ( @light != null )
                    light.deactivate();
            
            } // Next node

            // Next, emergency lights on.
            nodes = scene.getObjectNodesByType( RTID_SpotLightObject );
            for ( uint i = 0; i < nodes.length(); ++i )
            {
                ObjectNode @ node = nodes[i];
                if ( node.getBehaviorCount() == 0 )
                    continue;
                EmergencyLight @ light = cast<EmergencyLight>(node.getScriptedBehavior(0));
                if ( @light != null )
                    light.activate();
            
            } // Next node

            // Switch to escape music.
            getAppAudioDriver().loadAmbientTrack( "Music", "Music/Mistake the Getaway.ogg", 0, 0.3f );

        } // End if final objective

        // Get the next objective.
        if ( mNextObjective != 0 )
        {
            Scene @ scene = mNode.getScene();
            ObjectNode @ objectiveNode = scene.getObjectNodeById( mNextObjective );
            if ( objectiveNode.getBehaviorCount() > 0 )
                return cast<Objective>(objectiveNode.getScriptedBehavior(0));
        
        } // End if has other objective

        // No new objective
        return null;
    }
}