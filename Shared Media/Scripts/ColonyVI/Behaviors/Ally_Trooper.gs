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
#include_once "../API/NPCAgent.gsh"

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
    float mLastAimAngle;
    
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
        mFactionId                  = 0;
        maxDetectionConeH           = 200.0f;
        maxDetectionConeV           = 90.0f;
        maxDetectionRange           = 40.0f;
        maxFiringConeH              = 15.0f;
        maxFiringConeV              = 45.0f;
        maxFiringRange              = 30.0f;
        minFiringRange              = 16.0f;
        idealFiringRange            = 17.0f;
        maxSpeed                    = 5.0f;
        canFireWhileMoving          = false;
        walkAnimationName           = "Weapon Move Fast";
        backpeddleAnimationName     = "Weapon Move Fast";
        strafeLeftAnimationName     = "Weapon Strafe Left";
        strafeRightAnimationName    = "Weapon Strafe Right";
        firingAnimationName         = "Weapon Stand Fire Loop";
        walkAnimationSpeed          = 0.625f;
        backpeddleAnimationSpeed    = -0.625f;
        strafeAnimationSpeed        = 0.625f;

        // Setup agent description
        mMaximumHealth              = 200;
        mMaximumArmor               = 200;
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
        mLastAimAngle       = 0.0f;

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

        // Should we be aiming?
        float angle = 0.0f;
        if ( targetAcquired )
        {
            // Compute the angle to the target.
            Vector3 dir;
            Vector3 from = actor.getPosition() + (actor.getYAxis() * getHeight() * 0.66f);
            Vector3 to   = targetLastKnownPos + actor.getYAxis() * (targetHeight * 0.66f);
            vec3Normalize( dir, to - from );
            angle = dir.y;
            if ( angle < -1.0f )
                angle = -1.0f;
            if ( angle > 1.0f )
                angle = 1.0f;
            angle = CGEToDegree(asin( angle ));
        
        } // End if has target
        
        // Smooth angle over time
        float aimSmoothFactor = 0.1f / elapsedTime;
        angle = smooth( angle, mLastAimAngle, aimSmoothFactor );
        mLastAimAngle = angle;

        // If there is any adjustment to be made, do it.
        if ( angle < -0.1f || angle > 0.1f )
        {
            // Adjust skeleton
            // TODO: Cache object
            ObjectNode @ joint = actor.findChild( "joint12", true );
            joint.rotateAxis( -angle, actor.getXAxis(), joint.getPosition() );
        
        } // End if acquired

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

        // Call base class implementation
        NPCAgent::kill( hitNode, hitPoint, hitImpulse );
    }

} // End Class Ally_Trooper