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
// Name : Ally_Trooper.gs                                                    //
//                                                                           //
// Desc : Behavior script associated with the Trooper friendly NPC.          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "NPCAgent.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Ally_Trooper (Class)
// Desc : Behavior script associated with the Trooper friendly NPC.
//-----------------------------------------------------------------------------
shared class Ally_Trooper : NPCAgent
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Ally_Trooper () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Ally_Trooper( )
    {
        // Setup NPC description
        maxDetectionConeH           = 200.0f;
        maxDetectionConeV           = 90.0f;
        maxDetectionRange           = 40.0f;
        maxFiringConeH              = 15.0f;
        maxFiringConeV              = 45.0f;
        maxFiringRange              = 30.0f;
        maxSpeed                    = 8.0f;
        walkAnimationName           = "Weapon Move Fast";
        strafeLeftAnimationName     = "Weapon Strafe Left";
        strafeRightAnimationName    = "Weapon Strafe Right";
        firingAnimationName         = "Weapon Stand Fire Loop";
        canFireWhileMoving          = false;

        // Setup agent description
        mMaximumHealth              = 100;
        mMaximumArmor               = 100;
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
        // Get the weapon associated with the orc.
        @mCurrentWeaponNode = object.findChild( "Weapon_Trooper_Rifle", true );

        // Setup initial states.
        mCurrentHealth      = mMaximumHealth;
        mCurrentArmor       = mMaximumArmor;

        // Trigger base class implementation
        NPCAgent::onAttach( object );

        // Hide one of the two choices of helmets at random.
        if ( @actor != null )
        {
            ObjectNode @ helmet = actor.findChild( (randomInt(0,1) != 0) ? "Helmet 1" : "Helmet 2", true );
            if ( @helmet != null )
                helmet.showNode( false, true );
        
        } // End if valid
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Call base class implementation
        NPCAgent::onDetach( object );
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Call base class implementation
        NPCAgent::onUpdate( elapsedTime );
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (NPCAgent)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : isPlayerVisible ()
	// Desc : Determine if the player is currently visible to this NPC.
	//-------------------------------------------------------------------------
    bool isPlayerVisible( bool & canFire, bool & performedLoS, float & visibilityScore )
    {
        // ToDo: temp - troopers currently act as hostile. Disable their visibility 
        // of the player so they don't shoot at us.
        canFire = false;
        performedLoS = false;
        visibilityScore = 0.0f;
        return false;
    }

    //-------------------------------------------------------------------------
	// Name : updateTargetStatus ()
	// Desc : Check to see if an enemy target is visible / within range.
	//-------------------------------------------------------------------------
    void updateTargetStatus( float elapsedTime )
    {
        // ToDo: temp - troopers currently act as hostile. Disable their target update
        // so they don't shoot at us.
        return;
    }

    //-------------------------------------------------------------------------
	// Name : kill ()
	// Desc : Trigger NPC death and apply an impulse to the specified node
    //        in the ragdoll that is spawned.
	//-------------------------------------------------------------------------
    void kill( ObjectNode @ hitNode, const Vector3 & hitPoint, const Vector3 & hitImpulse )
    {
        // No-op?
        if ( !isAlive() )
            return;

        // Drop the weapon.
        if ( @mCurrentWeaponNode != null )
        {
            mCurrentWeaponNode.setParent( null );
            mCurrentWeaponNode.setPhysicsModel( PhysicsModel::RigidDynamic );
        
        } // End if has weapon

        // Call base class implementation
        NPCAgent::kill( hitNode, hitPoint, hitImpulse );
    }

} // End Class Ally_Trooper