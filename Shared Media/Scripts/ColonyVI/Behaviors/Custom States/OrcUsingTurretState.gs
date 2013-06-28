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
// Name : OrcUsingTurretState.gsh                                            //
//                                                                           //
// Desc : State that allows the enemy orc to stand and use a turret.         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../../API/AgentState.gsh"
#include_once "../Enemy_Turret.gs"

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------
const int TurretRefId = 0xDF5;

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : OrcUsingTurretState (Class)
// Desc : Agent state that allows the enemy orc to stand and use a turret.
//-----------------------------------------------------------------------------
shared class OrcUsingTurretState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@     mTurretNode;
    private Enemy_Turret@   mTurretAgent;
    private float           mOldMaxArmor;
    
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : OrcUsingTurretState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	OrcUsingTurretState( NPCAgent @ agent )
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
        // Find the turret node.
        Scene @ scene = mAgent.actor.getScene();
        @mTurretNode = scene.getObjectNodeById( TurretRefId );
        if ( @mTurretNode != null )
            @mTurretAgent = cast<Enemy_Turret>(mTurretNode.getScriptedBehavior(0));

        // Valid?
        if ( @mTurretAgent == null )
            return false;

        // Switch to idle animation
        mAgent.selectAnimation( "Crouch Idle", AnimationPlaybackMode::Loop, 0.0f );

        // Hide the orc's weapon.
        mAgent.getCurrentWeapon().deselect();

        // Orc is invulnerable initially.
        mOldMaxArmor = mAgent.getMaximumArmor();
        //mAgent.setMaximumArmor( 99999999 );
        //mAgent.setCurrentArmor( 99999999 );

        // Switch success.
        return true;
	}

    //-------------------------------------------------------------------------
	// Name : end ()
	// Desc : State has been deselected and is about to end.
	//-------------------------------------------------------------------------
    void end( AgentState @ to )
    {
        // If we're switching away from this state because our agent is dead,
        // kill the turret.
        if ( @mTurretAgent != null && to.getStateIdentifier() == AgentStateId::Dead )
            mTurretAgent.kill();

        // Release our references
        @mTurretAgent = null;
        @mTurretNode  = null;
    }

    //-------------------------------------------------------------------------
	// Name : update ()
	// Desc : Allow the state to perform its update processing.
	//-------------------------------------------------------------------------
    void update( float elapsedTime )
    {
        // When the turret is dead, or we take damage from the side, we are free!
        if ( @mTurretAgent != null )
        {
            bool hasUnknownThreat = (@mAgent.threatTable != null && mAgent.threatTable.damage > 0);
            if ( hasUnknownThreat )
            {
                if ( vec3Dot( -mAgent.threatTable.lastProjectileDir, mAgent.getSceneNode().getZAxis() ) >= 0.707f )
                    hasUnknownThreat = false;

            } // End if threat?

            // Are we free to start moving?
            if ( !mTurretAgent.isAlive() || hasUnknownThreat )
            {
                // Kill the turret if it is still alive
                if ( mTurretAgent.isAlive() )
                    mTurretAgent.kill();

                // Orc is no longer invulnerable
                //mAgent.setMaximumArmor( mOldMaxArmor );
                //mAgent.setCurrentArmor( mOldMaxArmor );

                // Show the orc's weapon.
                mAgent.getCurrentWeapon().select();

                // Enable NPC navigation now.
                mAgent.enableNavigation( true );

                // Switch to standard behavior
                mAgent.switchState( AgentStateId::Idle );

            }  // End if turret is dead

        } // End if has agent
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Using Turret";
    }
    
    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::UsingTurret;
    }
}