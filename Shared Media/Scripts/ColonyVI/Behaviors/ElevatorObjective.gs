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
// Name : ElevatorObjective.gs                                               //
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

enum ElevatorState
{
    Idle,
    OpeningEntrance,
    ClosingEntrance,
    Traveling,
    OpeningExit,
    ClosingExit,
    Broken
};

const float DoorMaxDistance = 0.824f;
const float MaxTravelTime = 4.0f;

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : ElevatorObjective (Class)
// Desc : Objective that must be activated by the player to progress.
//-----------------------------------------------------------------------------
class ElevatorObjective : Objective
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private uint            mTeleportExitRef;
    private ObjectNode@     mEntranceDoor1;
    private ObjectNode@     mEntranceDoor2;
    private ObjectNode@     mExitDoor1;
    private ObjectNode@     mExitDoor2;
    private ElevatorState   mState;
    private float           mOpenAmount; 
    private float           mTravelTime;

    // Sound effects.
    private SoundRef@       mTravelSoundChannel;    // Sound played during travel.

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : ElevatorObjective () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	ElevatorObjective( )
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
        mOpenAmount             = 0.0f;
        mState                  = ElevatorState::Idle;
        @mTravelSoundChannel    = null;

        // Get properties
        mTeleportExitRef = uint(object.getCustomProperty( "teleport_exit_ref", 0 ));
        uint entranceDoorRef1 = uint(object.getCustomProperty( "entrance_door_ref1", 0 ));
        uint entranceDoorRef2 = uint(object.getCustomProperty( "entrance_door_ref2", 0 ));
        uint exitDoorRef1 = uint(object.getCustomProperty( "exit_door_ref1", 0 ));
        uint exitDoorRef2 = uint(object.getCustomProperty( "exit_door_ref2", 0 ));

        // Find the elevator door objects.
        Scene @ scene = object.getScene();
        @mEntranceDoor1 = scene.getObjectNodeById( entranceDoorRef1 );
        @mEntranceDoor2 = scene.getObjectNodeById( entranceDoorRef2 );
        @mExitDoor1 = scene.getObjectNodeById( exitDoorRef1 );
        @mExitDoor2 = scene.getObjectNodeById( exitDoorRef2 );

        // Don't run update process until we're ready.
        object.setUpdateRate( UpdateRate::Never );

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
        @mGamePlayState         = null;
        @mEntranceDoor1         = null;
        @mEntranceDoor2         = null;
        @mExitDoor1             = null;
        @mExitDoor2             = null;
        @mTravelSoundChannel    = null;

        // Call base class implementation last
        Objective::onDetach( object );
    }

    //-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        bool switchObjective = false;
		bool playDoorSound = false;

        // What are we handling?
        if ( mState == ElevatorState::OpeningEntrance || mState == ElevatorState::OpeningExit )
        {
            // Play the door open sound if we just started the process
            if ( mOpenAmount == 0 )
				playDoorSound = true;
        
            // Compute amount to open.
            float moveAmount = elapsedTime * 0.60f;
            mOpenAmount += moveAmount;
            if ( mOpenAmount >  DoorMaxDistance )
            {
                moveAmount -= (mOpenAmount - DoorMaxDistance);
                mOpenAmount = DoorMaxDistance;
            
            } // End if overshoots

            // Adjust doors
            if ( mState == ElevatorState::OpeningEntrance )
            {
                mEntranceDoor1.moveLocal( moveAmount, 0, 0 );
                mEntranceDoor2.moveLocal( moveAmount, 0, 0 );
            
            } // End if entrance
            else
            {
                mExitDoor1.moveLocal( moveAmount, 0, 0 );
                mExitDoor2.moveLocal( moveAmount, 0, 0 );
            
            } // End if exit

            // Done opening?
            if ( mOpenAmount >= DoorMaxDistance )
            {
                // Complete
                mNode.setUpdateRate( UpdateRate::Never );
                mState = ElevatorState::Idle;
                switchObjective = true;
            
            } // End if done
        
        } // End if opening entrance
        else if ( mState == ElevatorState::ClosingEntrance )
        {
            // Play the door open sound if we just started the process
            if ( mSequenceIdentifier != "escape_elevator_broken" && mOpenAmount == DoorMaxDistance )
				playDoorSound = true;

            // Compute amount to close.
            float moveAmount = elapsedTime * 0.6f;
            mOpenAmount -= moveAmount;
            if ( mOpenAmount < 0 )
            {
                moveAmount += mOpenAmount;
                mOpenAmount = 0;
            
            } // End if overshoots

            // Adjust doors
            mEntranceDoor1.moveLocal( -moveAmount, 0, 0 );
            mEntranceDoor2.moveLocal( -moveAmount, 0, 0 );

            // Done closing?
            if ( mOpenAmount <= 0 )
            {
                if ( mSequenceIdentifier == "escape_elevator_broken" )
                {
                    // Elevator is now broken!
                    mState = ElevatorState::Broken;
                    mNode.setUpdateRate( UpdateRate::Never );
                
                } // End if breaks elevator
                else
                {
                    // Switch to traveling state
                    mState = ElevatorState::Traveling;
                    mTravelTime = 0;
                    
                    // Play the elevator traveling sound
                    AudioManager @ audioManager = getAudioManager();
                    @mTravelSoundChannel = audioManager.playSound( "Sounds/Elevator Ride.ogg", false, false, 1.0f );

                    // Teleport the player to the elevator exit
                    // Get the object representing the teleport exit.
                    Scene @ scene = mNode.getScene();
                    ObjectNode @ exitNode = scene.getObjectNodeById( mTeleportExitRef );

                    // Player must maintain the same relative relationship to the exit node
                    // as it does to this node in order to avoid any visual hiccups if the
                    // exit is rotated. First determine the player's transformation in relation
                    // this node.
                    PlayerAgent @ player = mGamePlayState.getPlayer();
                    ObjectNode  @ playerNode = player.getSceneNode();
                    Transform deltaTransform = mNode.getWorldTransform();
                    deltaTransform.invert();
                    deltaTransform = playerNode.getWorldTransform() * deltaTransform;

                    // Select the new transform relative to the exit node
                    Transform newTransform = deltaTransform * exitNode.getWorldTransform();

                    // Telport the player to the new location.
                    player.teleportTo( newTransform.position() );

                    // Update the orientaiton
                    playerNode.setOrientation( newTransform.orientation() );

                } // End if transporting
            
            } // End if done

        } // End if closing entrance
        else if ( mState == ElevatorState::Traveling )
        {
            // Wait until enough time has elapsed and then open exit doors.
            mTravelTime += elapsedTime;
            if ( mTravelTime >= MaxTravelTime )
            {
                mState = ElevatorState::OpeningExit;
                mOpenAmount = 0.0f;
                
                // Stop the elevator traveling sound
                AudioManager @ audioManager = getAudioManager();
                audioManager.stopSound( mTravelSoundChannel );
            
            } // End if finished traveling

        } // End if traveling
        
        // Should we play the door open/close sound?
        if ( playDoorSound )
        {
            AudioManager @ audioManager = getAudioManager();
            Vector3 playerPosition = mGamePlayState.getPlayer().getSceneNode().getPosition();
            audioManager.playSound( "Sounds/Elevator Doors.ogg", false, false, 1.0f, playerPosition, null );
        }
        
        // Switch to next objective now?
        if ( switchObjective )
        {
            if ( mNextObjectiveId != 0 )
            {
                Scene @ scene = mNode.getScene();
                ObjectNode @ objectiveNode = scene.getObjectNodeById( mNextObjectiveId );
                if ( objectiveNode.getBehaviorCount() > 0 )
                    mGamePlayState.getPlayer().setCurrentObjective( cast<Objective>(objectiveNode.getScriptedBehavior(0)) );

            } // End if has other objective
        
        } // End if applyObjective
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
        if ( mSequenceIdentifier == "spawn_elevator_entry" || mSequenceIdentifier == "escape_elevator_entry" )
        {
            if ( mSequenceIdentifier == "escape_elevator_entry" )
            {
                // Replace the non-emissive material of the control terminal with its emissive variant.
                MeshNode @ meshNode = cast<MeshNode>(mNode.getParent());
                if ( @meshNode != null )
                {
                    // First, get the identifiers of the materials we want to swap.
                    uint litMaterialId   = meshNode.getCustomProperty( "material_illuminated", 0 );
                    uint unlitMaterialId = meshNode.getCustomProperty( "material_deluminated", 0 );

                    // Get the unlit material (should already be resident).
                    Scene @ scene = meshNode.getScene();
                    MaterialHandle litMaterial, unlitMaterial;
                    ResourceManager @ resources = scene.getResourceManager();
                    if ( resources.getMaterialFromId( unlitMaterial, unlitMaterialId ) )
                    {
                        // Get/load the lit material.
                        if ( resources.loadMaterial( litMaterial, scene.getParentWorld(), MaterialType::Standard, litMaterialId, false, 0, DebugSource() ) )
                        {
                            // Swap!
                            meshNode.replaceMaterial( unlitMaterial, litMaterial );
                        
                        } // End if found unlit

                    } // End if found lit

                    // Reactivate light
                    LightNode @ light = cast<LightNode>(meshNode.findChildOfType( RTID_LightObject ));
                    if ( @light != null )
                        light.showNode( true, false );

                } // End if found mesh node
            
            } // End if escape_elevator_entry

            // Start opening the doors
            mNode.setUpdateRate( UpdateRate::Always );
            mOpenAmount = 0;
            mState = ElevatorState::OpeningEntrance;

            // Trigger this gameplay sequence.
            mGamePlayState.setCurrentGameSequence( mSequenceIdentifier );
        
        } // End if *_elevator_entry
        else if ( mSequenceIdentifier == "spawn_elevator_travel" || mSequenceIdentifier == "escape_elevator_travel"  )
        {
            // Start closing the doors
            mNode.setUpdateRate( UpdateRate::Always );
            mOpenAmount = DoorMaxDistance;
            mState = ElevatorState::ClosingEntrance;

            // Trigger this gameplay sequence.
            mGamePlayState.setCurrentGameSequence( mSequenceIdentifier );

        } // End if *_elevator_travel
        else if ( mSequenceIdentifier == "escape_elevator_broken"  )
        {
            // This objective is triggered twice, first by the final generator
            // objective where we simply make sure the doors are closed (but
            // we do not physically switch to this objective). The second is
            // when the user actually triggers it, sparks fly and we switch to
            // the objective to collect a part from the warehouse.
            if ( mState != ElevatorState::Broken )
            {
                // Start closing the doors
                mNode.setUpdateRate( UpdateRate::Always );
                mOpenAmount = DoorMaxDistance;
                mState = ElevatorState::ClosingEntrance;
            
            } // End if !broken
            else
            {
                // Trigger this gameplay sequence.
                mGamePlayState.setCurrentGameSequence( mSequenceIdentifier );

                // Replace the emissive material of the control terminal with its non emissive variant.
                MeshNode @ meshNode = cast<MeshNode>(mNode.getParent());
                if ( @meshNode != null )
                {
                    // First, get the identifiers of the materials we want to swap.
                    uint litMaterialId   = meshNode.getCustomProperty( "material_illuminated", 0 );
                    uint unlitMaterialId = meshNode.getCustomProperty( "material_deluminated", 0 );

                    // Get the lit material (should already be resident).
                    Scene @ scene = meshNode.getScene();
                    MaterialHandle litMaterial, unlitMaterial;
                    ResourceManager @ resources = scene.getResourceManager();
                    if ( resources.getMaterialFromId( litMaterial, litMaterialId ) )
                    {
                        // Get/load the unlit material.
                        if ( resources.loadMaterial( unlitMaterial, scene.getParentWorld(), MaterialType::Standard, unlitMaterialId, false, 0, DebugSource() ) )
                        {
                            // Swap!
                            meshNode.replaceMaterial( litMaterial, unlitMaterial );
                        
                        } // End if found unlit

                    } // End if found lit

                    // Deactivate light
                    LightNode @ light = cast<LightNode>(meshNode.findChildOfType( RTID_LightObject ));
                    if ( @light != null )
                        light.showNode( false, false );

                    // Throw sparks
                    ParticleEmitterNode @ emitter = cast<ParticleEmitterNode>(meshNode.findChildOfType( RTID_ParticleEmitterObject ));
                    if ( @emitter != null )
                    {
                        emitter.setUpdateRate( UpdateRate::Always );
                        emitter.enableLayerEmission( 0, true );
                    
                    } // End if found

					// Play a sound to indicate a short
                    AudioManager @ audioManager = getAudioManager();
                    audioManager.playSound( "Sounds/Elevator Panel Fail.ogg", false, false, 0.5f );

                } // End if found mesh node

                // Move to next objective
                if ( mNextObjectiveId != 0 )
                {
                    Scene @ scene = mNode.getScene();
                    ObjectNode @ objectiveNode = scene.getObjectNodeById( mNextObjectiveId );
                    if ( objectiveNode.getBehaviorCount() > 0 )
                        return cast<Objective>(objectiveNode.getScriptedBehavior(0));

                } // End if has other objective
            }

        } // End if escape_elevator_broken

        // No new objective until later.
        return null;
    }
}