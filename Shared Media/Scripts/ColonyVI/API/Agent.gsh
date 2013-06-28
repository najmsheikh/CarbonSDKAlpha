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
#include_once "AudioManager.gsh"
#include_once "AgentManager.gsh"
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
    private AudioManager@           mAudioManager;              // Reference to the application audio manager.
    private AgentManager@           mAgentManager;              // The manager to which this agent is assigned.
    private ObjectNode@             mNode;                      // The node to which we are attached.
    private ObjectNode@             mCurrentWeaponNode;         // The scene object node representing the weapon currently equipped by the agent (if any).
    private Weapon@                 mCurrentWeapon;             // The weapon currently equipped by the agent (if any).
    private AgentState@             mState;                     // Current state associated with the agent (i.e. idling, firing, etc.)
    private bool                    mIsAlive;                   // Is the agent alive?
    private int                     mFactionId;                 // What faction does the agent belong to (0 = allies, 1 = enemy)
    private float                   mLOSRadius;                 // Radius of the character for the purposes of line of sight testing.
    private int                     mSpawnStateId;              // Used by the agent to understand the state to spawn in.
    private int                     mSpawnContext;              // The application defined context that can be used to recognize agents spawned under certain conditions.

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
        mFactionId      = 0;
        mLOSRadius      = 1.0f;
        mSpawnStateId   = AgentStateId::Unknown;
        mSpawnContext   = 0;
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
        @mState         = null;
        mIsAlive        = true;
        
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
	// Name : createStateById ()
	// Desc : Create a new instance of the agent state associated with the 
    //        specified identifier. Can be overriden by derived types to supply
    //        custom states specific to their type for each identifier.
	//-------------------------------------------------------------------------
    AgentState @ createStateById( int id )
    {
        return null;
    }

    //-------------------------------------------------------------------------
	// Name : setAgentManager ()
	// Desc : Set the manager to which this agent belongs.
	//-------------------------------------------------------------------------
    void setAgentManager( AgentManager@ manager )
    {
        @mAgentManager = manager;
        @mAudioManager = manager.getAudioManager();
    }

    //-------------------------------------------------------------------------
	// Name : getAgentManager ()
	// Desc : Retrieve the manager to which this agent belongs.
	//-------------------------------------------------------------------------
    AgentManager@ getAgentManager( )
    {
        return @mAgentManager;
    }

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
	// Name : setSpawnDetails ()
	// Desc : Set the context used by the agent to understand the state to 
    //        spawn in.
	//-------------------------------------------------------------------------
    void setSpawnDetails( int stateId, int context )
    {
        mSpawnStateId = stateId;
        mSpawnContext = context;
    }

    //-------------------------------------------------------------------------
	// Name : getSpawnContext ()
	// Desc : Retrieve the application defined context that can be used to 
    //         recognize agents spawned under certain conditions.
	//-------------------------------------------------------------------------
    int getSpawnContext( )
    {
        return mSpawnContext;
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
	// Name : setCurrentArmor ()
	// Desc : Set the total current health points that remain for this
    //        agent.
	//-------------------------------------------------------------------------
    void setCurrentHealth( float health )
    {
        if ( health > mMaximumHealth )
            health = mMaximumHealth;
        mCurrentHealth = health;
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
	// Name : setCurrentArmor ()
	// Desc : Set the total current armor points that remain for this
    //        agent.
	//-------------------------------------------------------------------------
    void setCurrentArmor( float armor )
    {
        if ( armor > mMaximumArmor )
            armor = mMaximumArmor;
        mCurrentArmor = armor;
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
	// Name : setMaximumArmor ()
	// Desc : Set the maximum amount of health points that this agent can
    //        have.
	//-------------------------------------------------------------------------
    void setMaximumHealth( float health )
    {
        mMaximumHealth = health;
        if ( mCurrentHealth < mMaximumHealth )
            mCurrentHealth = mMaximumHealth;
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
	// Name : setMaximumArmor ()
	// Desc : Set the maximum amount of armor points that this agent can
    //        have.
	//-------------------------------------------------------------------------
    void setMaximumArmor( float armor )
    {
        mMaximumArmor = armor;
        if ( mCurrentArmor < mMaximumArmor )
            mCurrentArmor = mMaximumArmor;
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
	// Name : getLOSRadius ()
	// Desc : Get the radius of the agent for the purposes of line of sight
    //        testing.
	//-------------------------------------------------------------------------
    float getLOSRadius()
    {
        return mLOSRadius;
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
    void switchState( int stateId )
    {
        AgentState @ newState = createStateById( stateId );
        switchState( newState );
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
	// Name : getFaction ()
	// Desc : Get the identifier of the faction that this agent belongs to.
	//-------------------------------------------------------------------------
    int getFaction( )
    {
        return mFactionId;
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
    
    //-------------------------------------------------------------------------
	// Name : kill ()
	// Desc : Trigger agent death.
	//-------------------------------------------------------------------------
    void kill( )
    {
        // Trigger agent death event
        if ( @mAgentManager != null )
            mAgentManager.onAgentDeath( AgentDeathEventArgs( this ) );
    }
 
}