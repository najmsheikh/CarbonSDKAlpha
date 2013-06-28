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
// Name : Enemy_Turret.gs                                                    //
//                                                                           //
// Desc : Behavior script associated with the turret NPC.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../API/NPCAgent.gsh"
#include_once "EmergencyLight.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Enemy_Turret (Class)
// Desc : Behavior script associated with the turret enemy NPC.
//-----------------------------------------------------------------------------
shared class Enemy_Turret : NPCAgent
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private bool            mTriggered;
    private ObjectNode@     mWarningLightNode;
    private EmergencyLight@ mWarningLight;
	
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Enemy_Mech () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Enemy_Turret( )
    {
        // Setup NPC description
        mFactionId                  = 1;
        mLOSRadius                  = 2.0f;
        maxDetectionConeH           = 90.0f;
        maxDetectionConeV           = 90.0f;
        maxDetectionRange           = 30.0f;
        maxFiringRange              = 30.0f;
        maxFiringConeH              = 20.0f;
        mMaximumHealth              = 9999999;
        mMaximumArmor               = 9999999;
        turnRate                    = 2.0f;
        supportsRagdoll             = false;
        canFireWhileMoving          = false;
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
        // Get the weapon associated with the mech.
        @mCurrentWeaponNode = object.findChild( "Weapon_Turret_Miniguns", true );

        // Get the warning / activation light
        @mWarningLightNode = object.findChild( "Turret_Warning_Light", true );
        if ( @mWarningLightNode != null )
            @mWarningLight = cast<EmergencyLight>(mWarningLightNode.getScriptedBehavior(0));
            
        // Setup initial states.
        mCurrentHealth      = mMaximumHealth;
        mCurrentArmor       = mMaximumArmor;
        mTriggered          = false;


        // Trigger base class implementation
        NPCAgent::onAttach( object );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release our references.
        @mWarningLightNode = null;
        @mWarningLight     = null;

        // Call base class implementation
        NPCAgent::onDetach( object );
    }

    //-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // If we're in the firing state, trigger gameplay events.
        if ( !mTriggered && @mState != null && mState.getStateIdentifier() == AgentStateId::Firing )
        {
            mTriggered = true;

            // Activate the warning light
            if ( @mWarningLight != null )
                mWarningLight.activate();

            // Progress gameplay sequence.
            AppStateManager @ stateManager = getAppStateManager();
            AppState @ state = stateManager.getState( "GamePlay" );
            if ( @state != null )
            {
                GamePlay @ gameManager = cast<GamePlay>( state.getScriptObject() );
                if ( @gameManager != null )
                    gameManager.setCurrentGameSequence( "hallway_turret_retreat" );
            
            } // End if active

        } // End if first triggering

        // Call base class implementation
        NPCAgent::onUpdate( elapsedTime );
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (NPCAgent)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : getHeight ()
	// Desc : Calculate current height of the agent.
	//-------------------------------------------------------------------------
    float getHeight( )
    {
        return 0.8f;
    }

    //-------------------------------------------------------------------------
	// Name : kill ()
	// Desc : Trigger NPC death.
	//-------------------------------------------------------------------------
    void kill( ObjectNode @ hitNode, const Vector3 & hitPoint, const Vector3 & hitImpulse )
    {
        // Make sure the warning light is deactivated
        if ( @mWarningLight != null )
            mWarningLight.deactivate();

        // Call the base class implementation first to finish up.
        NPCAgent::kill( hitNode, hitPoint, hitImpulse );
    }

    //-------------------------------------------------------------------------
	// Name : enableNavigation ()
	// Desc : Enable / disable navigation for this agent (should be called
    //        only once the agent has been fully initialized).
	//-------------------------------------------------------------------------
    void enableNavigation( bool enabled )
    {
        // This agent cannot navigate
    }
	
} // End Class Enemy_Turret