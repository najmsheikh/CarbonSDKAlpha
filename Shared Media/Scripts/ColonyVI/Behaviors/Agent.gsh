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
// Name : Agent.gsh                                                          //
//                                                                           //
// Desc : Base classes and core script logic for all game agents, including  //
//        NPCs and the player. Specific types of agents should derive from   //
//        class and implement their custom logic as necessary.               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "Weapon.gsh"
#include_once "AgentState.gsh"

//-----------------------------------------------------------------------------
// Enumerations
//-----------------------------------------------------------------------------
shared enum AgentStateType
{
    Idle,               // Agent is idling
    Searching,          // Agent is searching.
    Firing,             // Weapon is being fired
    MovingAndFiring,    // Weapon is being fired whilst moving.
    Reloading,          // Weapon is being reloaded
    ThrowGrenade,       // Throwing grenade
    Dead                // Agent is no longer alive.
};

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Agent (Base Class)
// Desc : Base classes and core script logic for all game agents, including
//        NPCs and the player. Specific types of agents should derive from
//        class and implement their custom logic as necessary.
//-----------------------------------------------------------------------------
shared class Agent : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@             mNode;                      // The node to which we are attached.
    private ObjectNode@             mCurrentWeaponNode;         // The scene object node representing the weapon currently equipped by the agent (if any).
    private Weapon@                 mCurrentWeapon;             // The weapon currently equipped by the agent (if any).
    private AgentState@             mState;                     // Current state associated with the agent (i.e. idling, firing, etc.)
    private bool                    mIsAlive;                   // Is the agent alive?

    // Agent description
    private float                   mMaximumHealth;             // Maximum health points for this agent.
    private float                   mMaximumArmor;              // Maximum armor points for this agent.

    // Agent state
    private float                   mCurrentHealth;             // Current health points for this agent.
    private float                   mCurrentArmor;              // Current armor points for this agent.


    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : FirstPersonActor () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Agent( )
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
        Scene @ scene = object.getScene();

        // Initialize variables
        @mState     = null;
        mIsAlive    = true;
        
        // Cache references to required objects.
        @mNode  = object;
        
        // Retrieve weapon script.
        if ( @mCurrentWeaponNode != null && mCurrentWeaponNode.getBehaviorCount() > 0 )
        {
            @mCurrentWeapon = cast<Weapon>(mCurrentWeaponNode.getScriptedBehavior(0));

            // Select current weapon
            if ( @mCurrentWeapon != null )
            {
                mCurrentWeapon.setOwnerAgent( this );
                mCurrentWeapon.select();
            
            } // End if valid
        
        } // End if has weapon
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        @mNode              = null;
        @mCurrentWeapon     = null;
        @mCurrentWeaponNode = null;
        @mState             = null;
    }

    //-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Do nothing if we are not attached.
        if ( @mNode == null || @mState == null )
            return;

        // Allow assigned state to update
        mState.update( elapsedTime );
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : getSceneNode ()
	// Desc : Get the scene node representing this agent.
	//-------------------------------------------------------------------------
    ObjectNode@ getSceneNode( )
    {
        return @mNode;
    }

    //-------------------------------------------------------------------------
	// Name : getState ()
	// Desc : Get the current state object for this agent.
	//-------------------------------------------------------------------------
    AgentState @ getState( )
    {
        return @mState;
    }

    //-------------------------------------------------------------------------
	// Name : getCurrentHealth ()
	// Desc : Retrieve the total current health points that remain for this
    //        agent.
	//-------------------------------------------------------------------------
    float getCurrentHealth( )
    {
        return mCurrentHealth;
    }

    //-------------------------------------------------------------------------
	// Name : getCurrentArmor ()
	// Desc : Retrieve the total current armor points that remain for this
    //        agent.
	//-------------------------------------------------------------------------
    float getCurrentArmor( )
    {
        return mCurrentArmor;
    }

    //-------------------------------------------------------------------------
	// Name : getMaximumHealth ()
	// Desc : Retrieve the maximum amount of health points that this agent can
    //        have.
	//-------------------------------------------------------------------------
    float getMaximumHealth( )
    {
        return mMaximumHealth;
    }

    //-------------------------------------------------------------------------
	// Name : getMaximumArmor ()
	// Desc : Retrieve the maximum amount of armor points that this agent can
    //        have.
	//-------------------------------------------------------------------------
    float getMaximumArmor( )
    {
        return mMaximumArmor;
    }

    //-------------------------------------------------------------------------
	// Name : getCurrentWeapon ()
	// Desc : Get the weapon currently selected for this agent.
	//-------------------------------------------------------------------------
    Weapon @ getCurrentWeapon( )
    {
        return @mCurrentWeapon;
    }

    //-------------------------------------------------------------------------
	// Name : getHeight ()
	// Desc : Calculate current height of the agent.
	//-------------------------------------------------------------------------
    float getHeight( )
    {
        return 2.0f; // Derived class should implement!
    }

    //-------------------------------------------------------------------------
	// Name : switchState ()
	// Desc : Switch the current agent state to that specified, ending the
    //        previous and beginning the new state appropriately.
	//-------------------------------------------------------------------------
    void switchState( AgentState @ state )
    {
        // Attempt to begin the new state first. It may return false
        // if chose a different state to switch to.
        if ( @state != null )
        {
            if ( !state.begin( mState ) )
                return;
        
        } // End if valid new state

        // Complete any old state
        if ( @mState != null )
            mState.end( state );

        // We're switched.
        @mState = state;
    }

    //-------------------------------------------------------------------------
	// Name : setAlive ()
	// Desc : Set the alive status of this agent.
	//-------------------------------------------------------------------------
    void setAlive( bool alive )
    {
        mIsAlive = alive;
    }

    //-------------------------------------------------------------------------
	// Name : isAlive ()
	// Desc : Determine if this agent is still alive.
	//-------------------------------------------------------------------------
    bool isAlive( )
    {
        return mIsAlive;
    }

    //-------------------------------------------------------------------------
	// Name : onProjectileHit () (Event)
	// Desc : Agent was hit by a projectile.
	//-------------------------------------------------------------------------
    void onProjectileHit( Weapon @ weapon, ObjectNode @ intersectedNode, const Vector3 & from, const Vector3 & to )
    {
        // Nothing in base implementation.
    }

    //-------------------------------------------------------------------------
	// Name : onExplosionHit () (Event)
	// Desc : Agent was near to an explosion.
	//-------------------------------------------------------------------------
    void onExplosionHit( const Vector3 & source, float maxDamage, float damageScale, float force )
    {
        // Nothing in base implementation.
    }
}