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
// Name : Enemy_Orc.gs                                                       //
//                                                                           //
// Desc : Behavior script associated with the Orc enemy NPC.                 //
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
// Name : Enemy_Orc (Class)
// Desc : Behavior script associated with the Orc enemy NPC.
//-----------------------------------------------------------------------------
shared class Enemy_Orc : NPCAgent
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Enemy_Orc () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Enemy_Orc( )
    {
        // Setup NPC description
        maxDetectionConeH           = 200.0f;
        maxDetectionConeV           = 90.0f;
        maxDetectionRange           = 40.0f;
        maxFiringConeH              = 15.0f;
        maxFiringConeV              = 45.0f;
        maxFiringRange              = 30.0f;
        walkAnimationName           = "Weapon Move Slow";
        strafeLeftAnimationName     = "Weapon Strafe Left";
        strafeRightAnimationName    = "Weapon Strafe Right";

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
        @mCurrentWeaponNode = object.findChild( "Weapon_Orc_Shotgun", true );

        // Setup initial states.
        mCurrentHealth      = mMaximumHealth;
        mCurrentArmor       = mMaximumArmor;

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

        // Agent no longer gets any updates.
        mNode.setUpdateRate( UpdateRate::Never );

        // Call base class implementation
        NPCAgent::kill( hitNode, hitPoint, hitImpulse );
    }

} // End Class Enemy_Orc