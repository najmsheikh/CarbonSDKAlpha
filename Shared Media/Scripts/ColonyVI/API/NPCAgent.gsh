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
// Name : NPCAgent.gsh                                                       //
//                                                                           //
// Desc : Base classes and core script logic for non-player controlled       //
//        game agents. Specific types of agents should derive from this      //
//        class and implement their custom logic as necessary.               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "Agent.gsh"

// Known default states
#include_once "Agent States/RepositionState.gs"
#include_once "Agent States/RepositionFiringState.gs"
#include_once "Agent States/DeadState.gs"
#include_once "Agent States/FiringState.gs"
#include_once "Agent States/IdleState.gs"
#include_once "Agent States/ReloadingState.gs"
#include_once "Agent States/SearchingState.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
shared class IntervalTimer
{
    private float   timeElapsed;
    private float   timeIntervalCurrent;
    private float   timeIntervalMin;
    private float   timeIntervalMax;

    //-------------------------------------------------------------------------
	// Name : IntervalTimer () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	IntervalTimer( )
    {
		timeElapsed         = 0;
		timeIntervalCurrent = 0;
		timeIntervalMin     = 0;
		timeIntervalMax     = 0;
    }

	void setInterval( float minT, float maxT )
    {
		timeElapsed         = 0;
		timeIntervalMin     = minT;
		timeIntervalMax     = maxT;
		timeIntervalCurrent = randomFloat( timeIntervalMin, timeIntervalMax );
    }
    
    bool update( float elapsedTime )
    {
		timeElapsed += elapsedTime;
		if ( timeElapsed > timeIntervalCurrent )
		{
   			timeElapsed -= timeIntervalCurrent;
			timeIntervalCurrent = randomFloat( timeIntervalMin, timeIntervalMax );
			return true;
		}
		return false;
    }
}

shared class Threat
{
    Agent@      agent;
    ObjectNode@ node;
    float       damage;
    Vector3     lastProjectileDir;
    Threat@     next;
    Threat@     previous;
};

shared class NPCAgentDebug
{
    bool  willingToFire;
    bool  inDetectionCones;
    bool  acquired;
    bool  inLoS;
    float angleH;
    float angleV;
    float range;
    float visibilityScore;
    float damageReceivedFromTarget;
    String stateName;
    String animationName;
    float  animationPos;
    Vector3 targetLastKnownPos;
    String agentLog;

} // End class : NPCAgentDebug

//-----------------------------------------------------------------------------
// Name : NPCAgent (Class)
// Desc : Base classes and core script logic for non-player controlled game 
//        agents. Specific types of agents should derive from this class and 
//        implement their custom logic as necessary.
//-----------------------------------------------------------------------------
shared class NPCAgent : Agent
{
    ///////////////////////////////////////////////////////////////////////////
	// Public Member Variables
	///////////////////////////////////////////////////////////////////////////
    ObjectNode@                     playerNode;                 // Cached reference to the scene node representing the main player.
    Agent@                          playerAgent;                // Cached reference to the player's agent script.
    ActorNode@                      actor;                      // Cached reference to member 'mNode', cast to 'ActorNode'.
    CharacterController@            controller;                 // Character controller currently assigned to the agent.

    // Heading
    Quaternion                      requestedOrientation;

    // Target Tracking
    float                           timeSinceTargetCheck;       // Amount of time that has elapsed since we last checked if any enemys were visible.
    float                           timeSinceTargetAcquire;     // Amount of time that has elapsed since we last spotted our target.
    float                           timeSinceTargetLost;        // Amount of time that has elapsed since we last lost our target.
    float                           timeSinceTargetHeard;       // Amount of time that has elapsed since we last heard the target.
    float                           targetVisibilityScore;      // Amount of visibility we have of the target on a 0-1 scale.
    float                           targetHeight;               // The height of our last known target.
    Vector3                         targetLastKnownPos;         // Last known position of target.
    bool                            targetAcquired;             // We have acquired an enemy target.
    bool                            willingToFire;              // Agent is willing to fire (target is in firing range / cone angles).
    Threat@                         currentTarget;              // Our current target / threat.
    Threat@                         threatTable;                // List of threats to this agent.
    Threat@                         threatTableTail;            // Tail of the threat linked list

    int                             lastTargetCheckIndex;       // Manager index of the last *potential* target we checked.
    
    // Damage Tracking
    int                             damageRecoveryPhase;        // Agent has recently been hit and is performing a damage recovery step when > 0.
    float                           damageRecoveryTime;         // Amount of time since agent entered into damage recovery.

    // Animation
    int                             animationTrackIndex;        // Index of the controller track currently playing the agent's animation.
    String                          currentAnim;
    float                           currentAnimPos;
    AnimationPlaybackMode           currentAnimMode;

    // NPC Description
    float                           maxSpeed;                   // Maximum speed at which this NPC can travel.
    float                           maxDetectionConeH;          // Angle, in degrees, within which this NPC can detect targets along the horizontal plane.
    float                           maxDetectionConeV;          // Angle, in degrees, within which this NPC can detect targets along the vertical plane.
    float                           maxDetectionRange;          // Range, in meters, within which this NPC can detect targets.
    float                           maxAudioDetectionRange;     // Range, in meters, within which this NPC can detect targets based on gunshots, etc.
    float                           maxFiringConeH;             // Angle, in degrees, within which this NPC will fire on a target along the horizontal plane.
    float                           maxFiringConeV;             // Angle, in degrees, within which this NPC will fire on a target along the vertical plane.
    float                           maxFiringRange;             // Range, in meters, within which this NPC will fire on a target.
    float                           idealFiringRange;           // Range, in meters, that the NPC will try to advance to whilst firing.
    float                           minFiringRange;             // Range, in meters, within which this NPC will attempt to back up so that it is not too close to fire.
    float                           minVisTestRange;            // Range within which visibility tests are not required.
    float                           turnRate;                   // Rate at which the NPC can turn to face the target (default 7).
    bool                            throttlePathFinding;        // When enabled, a new path toward target will not be selected unless the target moves approximately 4m from their prior position.
    bool                            supportsRagdoll;            // Should a ragdoll be created when the agent dies (Default=true)?
    bool                            dynamicDamageAnimation;     // Use the dynamics system (ragdoll) for the agent animation when hit by projectiles, etc.
    bool                            canFireWhileMoving;         // Agent can fire while moving?
    String                          idleAnimationName;
    String                          reloadAnimationName;
    String                          walkAnimationName;
    float                           walkAnimationSpeed;
    String                          backpeddleAnimationName;
    float                           backpeddleAnimationSpeed;
    String                          deathAnimationName;
    String                          strafeLeftAnimationName;
    String                          strafeRightAnimationName;
    float                           strafeAnimationSpeed;
    String                          firingAnimationName;
    String                          ragdollRootJoint;
    float                           ragdollDefaultConeLimit;

    // Debug
    NPCAgentDebug                   debugStats;
    bool                            debugEnabled;
    MeshNode@                       debugNavigationTarget;

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : NPCAgent () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	NPCAgent( )
    {
        // Default description
        debugEnabled                = false;
        maxSpeed                    = 3.0f;
        maxDetectionConeH           = 200.0f;
        maxDetectionConeV           = 90.0f;
        maxDetectionRange           = 50.0f;
        maxAudioDetectionRange      = 50.0f;
        maxFiringConeH              = 5.0f;
        maxFiringConeV              = 45.0f;
        maxFiringRange              = 40.0f;
        idealFiringRange            = 15.0f;
        minFiringRange              = 7.0f;
        minVisTestRange             = 2.0f;
        turnRate                    = 7.0f;
        supportsRagdoll             = true;
        dynamicDamageAnimation      = false;
        canFireWhileMoving          = true;
        idleAnimationName           = "Weapon Idle";
        reloadAnimationName         = "Weapon Reload";
        walkAnimationName           = "";
        backpeddleAnimationName     = "";
        strafeLeftAnimationName     = "";
        strafeRightAnimationName    = "";
        backpeddleAnimationSpeed    = 1.0f;
        walkAnimationSpeed          = 1.0f;
        strafeAnimationSpeed        = 1.0f;
        throttlePathFinding         = true;
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
        @actor = cast<ActorNode>(object);

        // Setup actor states
        //actor.setTrackFadeTimes( 0.1f, 0.1f );

        // Create a debug aid to provide a visual representation of the
        // agent's current navigation target.
        if ( debugEnabled )
        {
            @debugNavigationTarget = cast<MeshNode>(scene.createObjectNode( true, RTID_MeshObject, false ));
            if ( @debugNavigationTarget != null )
                debugNavigationTarget.createSphere( 0.3f, 5, 10, false, MeshCreateOrigin::Center );
        
        } // End if debugging

        // Initialize variables
        timeSinceTargetCheck        = 0;
        timeSinceTargetAcquire      = 0;
        timeSinceTargetLost         = 99999999.0f;
        timeSinceTargetHeard        = 99999999.0f;
        targetAcquired              = false;
        damageRecoveryPhase         = 0;
        requestedOrientation        = object.getWorldTransform().orientation();
        lastTargetCheckIndex        = 0;
        animationTrackIndex         = 0;
        @threatTable                = null;
        @threatTableTail            = null;

        // Get properties
        ragdollRootJoint = String(object.getCustomProperty( "ragdoll_root", Variant("") ));
        ragdollDefaultConeLimit = float(object.getCustomProperty( "ragdoll_default_cone_limit", 20.0f ));
        if ( ragdollRootJoint == "" )
            supportsRagdoll = false;

        // Call base class implementation to update base members.
        Agent::onAttach( object );

        // Select the initial state of this agent if one was not already selected
        if ( @mState == null )
	        switchState( mSpawnStateId );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        @playerNode         = null;
        @playerAgent        = null;
        @currentTarget      = null;
        @actor              = null;
        @threatTable        = null;
        @threatTableTail    = null;

        // Call base class implementation last
        Agent::onDetach( object );
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Keep turning toward requested orientation.
        Quaternion currentOrientation = actor.getWorldTransform().orientation();
        quaternionSlerp( currentOrientation, currentOrientation, requestedOrientation, elapsedTime * turnRate );
        actor.setOrientation( currentOrientation );

        // Update this NPCs target status (is one nearby, have we lost our previous, etc.)
        updateTargetStatus( elapsedTime );

        // Call base class implementation.
        Agent::onUpdate( elapsedTime );

        // Debugging
        if ( debugEnabled )
        {
            // Position the visual representation of the agent's navigation point.
            if ( @controller != null )
                debugNavigationTarget.setPosition( controller.getNavigationTarget() );

            // Output debug stats
            AnimationController @ animController = actor.getAnimationController();
            debugStats.acquired = targetAcquired;
            debugStats.targetLastKnownPos = targetLastKnownPos;
            debugStats.stateName     = ( @mState != null ) ? mState.getStateName() : "<None>";
            debugStats.animationName = currentAnim;
            debugStats.animationPos  = animController.getTrackPosition( animationTrackIndex );
            debugStats.visibilityScore = targetVisibilityScore;
            debugStats.damageReceivedFromTarget = (@currentTarget != null) ? currentTarget.damage : 0;

            // TEST: UPDATE DEBUGGING
            debugStats.agentLog = "";
            Threat @threat = threatTable;
            int i = 1;
            while ( @threat != null )
            {
                debugStats.agentLog += i + ") " + threat.damage + "\n";
                @threat = threat.next;
                ++i;

            } // Next threat
        
        } // End if debug
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (Agent)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : createStateById ()
	// Desc : Create a new instance of the agent state associated with the 
    //        specified identifier. Can be overriden by derived types to supply
    //        custom states specific to their type for each identifier.
	//-------------------------------------------------------------------------
    AgentState @ createStateById( int id )
    {
        switch ( id )
        {
            case AgentStateId::Repositioning:
                return RepositionState( this );
            case AgentStateId::RepositionFiring:
                return RepositionFiringState( this );
            case AgentStateId::Dead:
                return DeadState( this );
            case AgentStateId::Firing:
                return FiringState( this );
            case AgentStateId::Idle:
                return IdleState( this );
            case AgentStateId::Reloading:
                return ReloadingState( this );
            case AgentStateId::Searching:
                return SearchingState( this );
        
        } // End switch
        
        // Call base
        return Agent::createStateById( id );
    }

    //-------------------------------------------------------------------------
	// Name : getHeight ()
	// Desc : Calculate current height of the agent.
	//-------------------------------------------------------------------------
    float getHeight( )
    {
        if ( @controller == null )
            return Agent::getHeight();
        return controller.getCharacterHeight( true );
    }

    //-------------------------------------------------------------------------
	// Name : onProjectileHit () (Event)
	// Desc : Agent was hit by a projectile.
	//-------------------------------------------------------------------------
    void onProjectileHit( Weapon @ weapon, ObjectNode @ intersectedNode, const Vector3 & from, const Vector3 & to )
    {
        // Alive?
        if ( !isAlive() )
            return;

        // Get a reference to the agent that is holding the weapon that shot us.
        Agent @ sourceAgent = weapon.getOwnerAgent();
        if ( @sourceAgent == null )
            return;

        // Compute projectile's direction.
        Vector3 direction;
        vec3Normalize( direction, to - from );

        // Compute total damage
        float damage = weapon.computeDamage( vec3Length( to - from ) );

        // Scale base damage by damage scale amount if any is available.
        damage *= float(intersectedNode.getCustomProperty( "dmg_mult", 1.0f ));
        float totalDamage = damage;

        // Apply damage to armor (if there is any)
        float armorDamage = min( mCurrentArmor, damage );
        mCurrentArmor -= armorDamage;
        damage -= armorDamage;

        // Apply damage to health (if there is any)
        float healthDamage = min( mCurrentHealth, damage );
        mCurrentHealth -= healthDamage;
        damage -= healthDamage;

        // If the source of the impact was part of a different faction,
        // add it as a threat, or promote it if it was known to us.
        if ( sourceAgent.getFaction() != getFaction() )
        {
            // Is this agent already a threat?
            Threat @ threat = getThreat( sourceAgent );
            
            // If not a threat, add it. Otherwise, promote it.
            if ( @threat == null )
                @threat = addThreat( sourceAgent, sourceAgent.getSceneNode(), totalDamage );
            else
                promoteThreat( threat, totalDamage );

            // Record last known projectile hit direction
            threat.lastProjectileDir = direction;

            // If the threat at the head of our threat table is different from our
            // current threat, then switch target.
            if ( @threatTable != @currentTarget )
            {
                @currentTarget          = threatTable;
                targetLastKnownPos      = currentTarget.node.getPosition();
                targetHeight            = currentTarget.agent.getHeight();
                targetAcquired          = true;
                timeSinceTargetAcquire  = 0;
                turnTowardPosition( targetLastKnownPos );
            
            } // End if switch target
        
        } // End if different faction

        // Kill the NPC if there is no health left, otherwise perform
        // damage reaction operations (animation, etc.)
        if ( mCurrentHealth <= CGE_EPSILON )
            kill( intersectedNode, to, direction * 40.0f );
        else
            beginDamageRecovery( intersectedNode, to, direction * 20.0f );
    }

    //-------------------------------------------------------------------------
	// Name : onExplosionHit () (Event)
	// Desc : Agent was near to an explosion.
	//-------------------------------------------------------------------------
    void onExplosionHit( const Vector3 & source, float maxDamage, float damageScale, float force )
    {
        // Alive?
        if ( !isAlive() )
            return;

        // Compute total damage
        float damage = maxDamage * damageScale;

        // Apply damage to armor (if there is any)
        float armorDamage = min( mCurrentArmor, damage );
        mCurrentArmor -= armorDamage;
        damage -= armorDamage;

        // Apply damage to health (if there is any)
        float healthDamage = min( mCurrentHealth, damage );
        mCurrentHealth -= healthDamage;
        damage -= healthDamage;

        // Kill the NPC if there is no health left.
        if ( mCurrentHealth <= CGE_EPSILON )
        {
            // Fling the agent from its root bone out.
            ObjectNode @ rootBone = actor.findChild( ragdollRootJoint );
            if ( @rootBone != null )
            {
                Vector3 impulse;
                vec3Normalize( impulse, (rootBone.getPosition() - source) );            
                kill( rootBone, rootBone.getPosition(), impulse * force );
            
            } // End if found
            else
            {
                kill();

            } // End if not found

        } // End if dead
        else
        {
            // Turn towards suspected target if we're not already on the hunt.
            if ( !targetAcquired || timeSinceTargetLost <= 10.0f )
            {
                targetLastKnownPos = source;
                targetHeight       = getHeight(); // Assume it's our own height until correct.
                turnTowardPosition( targetLastKnownPos );
            
            } // End if not already acquired

        } // End if !dead
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : log ()
	// Desc : Add the specified string to this agent's debug log if debugging
    //        is enabled.
	//-------------------------------------------------------------------------
    void log( const String & value )
    {
        if ( debugEnabled )
        {
            debugStats.agentLog += value;
            debugStats.agentLog += "\n";
        
        } // End if enabled
    }

    //-------------------------------------------------------------------------
	// Name : enableNavigation ()
	// Desc : Enable / disable navigation for this agent (should be called
    //        only once the agent has been fully initialized).
	//-------------------------------------------------------------------------
    void enableNavigation( bool enabled )
    {
        // No-op?
        if ( enabled == (@controller != null) )
            return;

        // Enabling or disabling?
        if ( enabled )
        {
            // Assign a character controller to the agent.
            Scene @ scene = actor.getScene();
            @controller = CharacterController( scene.getPhysicsWorld(), false );
            actor.setPhysicsController( controller );

            // Allow the controller to initialize.
            controller.setCharacterHeight( getHeight() );
            controller.setCharacterRadius( 0.4f );
            controller.setMaximumWalkSpeed( maxSpeed );
            controller.setWalkAcceleration( 50.0f );
            controller.setCharacterMass( 100.0f );
            controller.initialize( );

        } // End if enabling
        else
        {
            // Destroy the character's current controller.
            actor.setPhysicsController( null );
            @controller = null;

        } // End if disabling
    }

    //-------------------------------------------------------------------------
	// Name : turnTowardPosition ()
	// Desc : Request that the NPC turn toward the specified position.
	//-------------------------------------------------------------------------
    void turnTowardPosition( const Vector3 & position )
    {
        Transform t;
        Vector3 up(0,1,0), direction = (position - actor.getPosition()), right;
        vec3Cross( right, up, direction );
        vec3Cross( direction, right, up );
        t.setOrientation( right, up, direction );
        requestedOrientation = t.orientation();
    }
    
    //-------------------------------------------------------------------------
	// Name : kill ()
	// Desc : Trigger NPC death.
	//-------------------------------------------------------------------------
    void kill( )
    {
        kill( null, Vector3(0,0,0), Vector3(0,0,0) );
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

        // Call base class implementation
        Agent::kill();

        // Switch to the dead state.
        DeadState @ deadState = cast<DeadState>(createStateById( AgentStateId::Dead ));
        deadState.init( hitNode, hitPoint, hitImpulse );
        switchState( deadState );
    }

    //-------------------------------------------------------------------------
	// Name : setPlayer ()
	// Desc : Called to provide a reference to the main player object.
	//-------------------------------------------------------------------------
    void setPlayer( ObjectNode @ node )
    {
        @playerNode = node;
        @playerAgent = cast<Agent>(node.getScriptedBehavior(0));
    }

    //-------------------------------------------------------------------------
	// Name : lineOfSightToAgent ()
	// Desc : Check if we have line of sight to target. Returns it as a
    //        0-1 percentage.
	//-------------------------------------------------------------------------
    float lineOfSightToAgent( Agent@ agent, ObjectNode@ agentNode )
    {
        // Any current target?
        if ( @agentNode == null )
            return 0.0f;

        // Local members
        Vector3 direction, up(0, 1, 0), right;
        Vector3 srcPos = actor.getPosition();
        Vector3 destPos = agentNode.getPosition();
        vec3Normalize( direction, destPos - srcPos );
        vec3Cross( right, direction, up );
        vec3Normalize( right, right );

        // Compute the relative height of the source so we can use roughly
        // eye level as the source of the ray cast.
        float srcHeight = getHeight();
        srcPos += actor.getYAxis() * (srcHeight * 0.95f);

        // Destination positions are always in front of the source, toward
        // the NPC.
        srcPos  += direction * mLOSRadius; // ToDo: Rather than shifting the source position, use raycast filtering to exclude both characters
        destPos -= direction * agent.getLOSRadius();

        // Test 6 points in a grid for visibility, from center of the character's
        // height, to its top.
        CollisionContact contact;
        PhysicsWorld @ world = actor.getScene().getPhysicsWorld();
        float destHeight = agent.getHeight();
        float visibilityScore = 0.0f;
        if ( !world.rayCastClosest( srcPos, destPos + (up * (destHeight * 0.5f)), contact ) )
            visibilityScore += 1.0f;
        if ( !world.rayCastClosest( srcPos, destPos + (up * destHeight), contact ) )
            visibilityScore += 1.0f;
        if ( !world.rayCastClosest( srcPos, destPos + (right * 0.2f) + (up * (destHeight * 0.5f)), contact ) )
            visibilityScore += 1.0f;
        if ( !world.rayCastClosest( srcPos, destPos + (right * 0.2f) + (up * destHeight), contact ) )
            visibilityScore += 1.0f;
        if ( !world.rayCastClosest( srcPos, destPos + (right * -0.2f) + (up * (destHeight * 0.5f)), contact ) )
            visibilityScore += 1.0f;
        if ( !world.rayCastClosest( srcPos, destPos + (right * -0.2f) + (up * destHeight), contact ) )
            visibilityScore += 1.0f;

        // Return the result.
        return visibilityScore / 6.0f;
    }

    //-------------------------------------------------------------------------
	// Name : isAgentVisible ()
	// Desc : Determine if the specified agent is currently visible to this NPC.
	//-------------------------------------------------------------------------
    bool isAgentVisible( Agent@ agent, ObjectNode@ agentNode, bool & canFire, bool & performedLoS, float & visibilityScore )
    {
        // Reset debug statistics
        if ( debugEnabled )
        {
            debugStats.inDetectionCones = false;
            debugStats.inLoS            = false;
            debugStats.willingToFire    = false;
        
        } // End if debug

        // Be polite and reset output variables for the caller.
        canFire = false;
        visibilityScore = 0;
        performedLoS = false;

        // Agent is dead?
        if ( !agent.isAlive() )
            return false;

        // Get the direction to target.
        Vector3 srcPos = actor.getPosition(), destPos = agentNode.getPosition();
        Vector3 direction = (destPos - srcPos);
        float distance = vec3Length( direction );
        if ( distance <= CGE_EPSILON )
            return false;
        else
            direction /= distance;

        // Flatten direction onto the appropriate horizontal and vertical
        // planes so that we can perform tests against the independently
        // controlled cone angles.
        Vector3 directionH = actor.getYAxis(), directionV = actor.getXAxis();
        vec3Normalize( directionH, direction - directionH * vec3Dot( direction, directionH ) );
        vec3Normalize( directionV, direction - directionV * vec3Dot( direction, directionV ) );

        // ToDo: Fixme! the above can fail if the vectors are aligned. 

        // Update stats
        if ( debugEnabled )
        {
            debugStats.angleH = CGEToDegree( acos(vec3Dot( directionH, actor.getZAxis() )) );
            debugStats.angleV = CGEToDegree( acos(abs(vec3Dot( directionV, actor.getZAxis() ))) );
            debugStats.range  = distance;
        
        } // End if debug

        // If the target was NOT previously visible (within a period of time), 
        // first determine if this is within our detection range.
        if ( timeSinceTargetLost > 2.0f )
        {
            // Target is not within the detection radius?
            if ( distance > maxDetectionRange )
                return false;

            // Target is not within the detection cone?
            if ( vec3Dot( directionH, actor.getZAxis() ) < cos( CGEToRadian( maxDetectionConeH * 0.5f ) ) )
                return false;
            if ( abs(vec3Dot( directionV, actor.getZAxis() )) < cos( CGEToRadian( maxDetectionConeV * 0.5f ) ) )
                return false;
        
        } // End if not previously acquired

        // Also test to see if it's possible to fire.
        canFire = true;
        if ( distance > maxFiringRange ||
             vec3Dot( directionH, actor.getZAxis() ) < cos( CGEToRadian( maxFiringConeH * 0.5f ) ) ||
             abs(vec3Dot( directionV, actor.getZAxis() )) < cos( CGEToRadian( maxFiringConeV * 0.5f ) ) )
            canFire = false;

        // Update stats
        if ( debugEnabled )
        {
            debugStats.inDetectionCones = true;
            debugStats.willingToFire = canFire;

        } // End if debug

        // Skip actual visibility check if we're close enough.
        performedLoS = true;
        if ( distance > minVisTestRange )
        {
            visibilityScore = lineOfSightToAgent( agent, agentNode );
            if ( visibilityScore <= CGE_EPSILON )
                return false;
        
        } // End if within minimum
        else
            visibilityScore = 1.0f;

        // Update stats
        if ( debugEnabled )
            debugStats.inLoS = true;

        // Visible!
        return true;
    }

    //-------------------------------------------------------------------------
	// Name : selectAnimation ()
	// Desc : Begin playing the specified animation, and record the details
    //        so that we can determine the most recent request later.
	//-------------------------------------------------------------------------
    void selectAnimation( const String & setName, AnimationPlaybackMode mode, float position )
    {
        currentAnim         = setName;
        currentAnimPos      = position;
        currentAnimMode     = mode;
        animationTrackIndex = actor.playAnimationSet( "Primary", setName, mode, 1.0f, position );
    }

    //-------------------------------------------------------------------------
	// Name : selectAnimation ()
	// Desc : Begin playing the specified animation, and record the details
    //        so that we can determine the most recent request later.
	//-------------------------------------------------------------------------
    void selectAnimation( const String & setName, AnimationPlaybackMode mode, float position, float speed )
    {
        currentAnim         = setName;
        currentAnimPos      = position;
        currentAnimMode     = mode;
        animationTrackIndex = actor.playAnimationSet( "Primary", setName, mode, speed, position );
    }

    //-------------------------------------------------------------------------
	// Name : resetTarget ()
	// Desc : Clear out states that record our current target status so that
    //        we start searching for a new target.
	//-------------------------------------------------------------------------
    void resetTarget( )
    {
        @currentTarget       = null;
        targetAcquired       = false;
        timeSinceTargetLost  = 999999.0f;
        timeSinceTargetHeard = 999999.0f;
    }

    //-------------------------------------------------------------------------
	// Name : updateTargetStatus ()
	// Desc : Check to see if an enemy target is visible / within range.
	//-------------------------------------------------------------------------
    void updateTargetStatus( float elapsedTime )
    {
        timeSinceTargetCheck   += elapsedTime;
        timeSinceTargetAcquire += elapsedTime;
        timeSinceTargetLost    += elapsedTime;
        timeSinceTargetHeard   += elapsedTime;

        // If we have a target, and it is now dead, remove it.
        if ( @currentTarget != null && !currentTarget.agent.isAlive() )
        {
            removeThreat( currentTarget );
            resetTarget();
        
        } // End if target dead
        
        // Check visibility once every 500ms
        if ( timeSinceTargetCheck >= 0.5f )
        {
            // We've done a target check.
            timeSinceTargetCheck = 0;
            
            // If we currently have a target, check to see if we can still see it.
            // Otherwise, check to see if we can see a new target.
            bool performedLoS = false;
            if ( targetAcquired )
            {
                if ( !isAgentVisible( currentTarget.agent, currentTarget.node, willingToFire, performedLoS, targetVisibilityScore ) )
                {
                    // We have lost our target.
                    targetAcquired = false;
                    timeSinceTargetLost = 0;
                
                } // End if !visible

            } // End if acquired
            else
            {
                // Try and re-acquire our current target if we have one.
                if ( @currentTarget != null )
                {
                    if ( isAgentVisible( currentTarget.agent, currentTarget.node, willingToFire, performedLoS, targetVisibilityScore ) )
                    {
                        targetAcquired = true;
                        timeSinceTargetAcquire = 0;
                    
                    } // End if visible

                } // End if re-acquire
                else
                {
                    // TODO: Switch to another item in our threat table.

                    // Otherwise, search for a new target. Try a few.
                    for ( int i = 0; i < 4; ++i )
                    {
                        Agent @ potentialTarget = mAgentManager.getPotentialTarget( this, lastTargetCheckIndex );
                        if ( @potentialTarget == null )
                            break;

                        // Can we see the potential target?
                        if ( isAgentVisible( potentialTarget, potentialTarget.getSceneNode(), willingToFire, performedLoS, targetVisibilityScore ) )
                        {
                            // Is this already registered as a threat? If not, add it.
                            @currentTarget = getThreat(potentialTarget);
                            if ( @currentTarget == null )
                                @currentTarget = addThreat( potentialTarget, potentialTarget.getSceneNode(), 0 );
                            targetAcquired = true;
                            timeSinceTargetAcquire = 0;
                            break;
                        
                        } // End if visible
                    
                    } // Next potential

                    if ( !targetAcquired )
                    {
                        // Nothing found, does any nearby NPC from our current faction have 
                        // a target we can  steal?
                        Faction @ alliedFaction = mAgentManager.factions[getFaction()];
                        for ( int i = 0; i < alliedFaction.agents.length(); ++i )
                        {
                            AgentData @ data = alliedFaction.agents[i];
                            if ( !data.active || !data.agent.isAlive() || @data.agent == @this )
                                continue;

                            // Is an NPC that has a target?
                            NPCAgent @ npc = cast<NPCAgent>(data.agent);
                            if ( @npc != null && npc.targetAcquired )
                            {
                                // Ally we're inheriting from is within 20m of us?
                                if ( vec3Length(data.node.getPosition() - actor.getPosition()) <= 20 )
                                {
                                    // Is this already registered as a threat? If not, add it.
                                    @currentTarget = getThreat(npc.currentTarget.agent);
                                    if ( @currentTarget == null )
                                        @currentTarget = addThreat( npc.currentTarget.agent, npc.currentTarget.node, 0 );

                                    // We've inherited its target, but it isn't immediately 
                                    // acquired until we can actually see it
                                    targetLastKnownPos  = currentTarget.node.getPosition();
                                    targetHeight        = currentTarget.agent.getHeight();
                                    targetAcquired      = false;
                                    timeSinceTargetLost = 0;
                                    break;
                                
                                } // End if within 40m

                            } // End if has target
                        
                        } // Next agent

                    } // End if no target still

                } // End if no target

            } // End if !acquired

            // If the player has fired recently, update known position
            // if it's within audio detection range (unless acquired, and
            // then we don't need to know).
            /*if ( !targetAcquired )
            {
                Weapon @ playerWeapon = playerAgent.getCurrentWeapon();
                if ( @playerWeapon != null && playerWeapon.getTimeSinceFired() < 1.0f )
                {
                    float distanceToTarget = vec3Length( playerNode.getPosition() - mNode.getPosition() );
                    if ( distanceToTarget <= maxAudioDetectionRange )
                    {
                        // Range is 50% if the line of sight is blocked. We may have already
                        // performed the LoS check above, so re-use its results if we can.
                        if ( !performedLoS )
                            targetVisibilityScore = lineOfSightToAgent( playerAgent, playerNode );
                        if ( (targetVisibilityScore > CGE_EPSILON) || (distanceToTarget <= (maxAudioDetectionRange * 0.5f)) )
                        {
                            timeSinceTargetHeard = 0.0f;
                            targetLastKnownPos = playerNode.getPosition();
                            targetHeight
                        
                        } // End if LoS check
                    
                    }  // End if within range
                
                } // End if recently fired weapon

            } // End if !acquired*/

        } // End if update due

        // Record last known target position if acquired
        if ( targetAcquired )
        {
            targetLastKnownPos = currentTarget.node.getPosition();
            targetHeight       = currentTarget.agent.getHeight();
        
        } // End if acquired
    }

    //-------------------------------------------------------------------------
	// Name : beginDamageRecovery ()
	// Desc : Called when the agent has been hit and needs to perform its
    //        damage recovery processes (animations, dynamics, etc.)
	//-------------------------------------------------------------------------
    void beginDamageRecovery( ObjectNode @ hitNode, const Vector3 & hitPoint, const Vector3 & hitImpulse )
    {
        /*// If we're already in damage recovery, do nothing.
        if ( damageRecoveryPhase != 0 )
            return;

        // Are we using the dynamics system (ragdoll) to provide the animation for 
        // when the agent needs to react to taking damage?
        if ( supportsRagdoll && dynamicDamageAnimation )
        {
            // Find the root actor bone.
            ObjectNode @ rootBone = actor.findChild( "joint0" );
            if ( @rootBone != null )
            {
                // Attach a ragdoll controller to the root bone.
                Scene @ scene = actor.getScene();
                RagdollController @ ragdoll = RagdollController( scene.getPhysicsWorld() );
                rootBone.setPhysicsController( ragdoll );
                ragdoll.initialize( );

                // Apply an impulse the intersected bone.
                ragdoll.applyImpulseTo( hitNode, hitImpulse, hitPoint );

                // Record the position of the current animation, and then stop it for now.
                AnimationController @ animController = actor.getAnimationController();
                currentAnimPos = animController.getTrackPosition( animationTrackIndex );
                actor.stopAnimationTrack( "Primary", true );

                // Stop firing if we're currently firing.
                if ( mState == AgentState::Firing )
                    mCurrentWeapon.endFiring();

                // Switch to recovery
                mDamageRecoveryPhase = 1;
                mDamageRecoveryTime  = 0;

            } // End if found root bone

        } // End if dynamics based animation*/
    }

    //-------------------------------------------------------------------------
	// Name : updateDamageRecovery () (Private)
	// Desc : Check to see if this agent is recovering after having taken
    //        damage, and take appropriate steps to handle the animations
    //        and dynamics for this. Returns true if the agent was in a state
    //        of damage recovery.
	//-------------------------------------------------------------------------
    bool updateDamageRecovery( float timeDelta )
    {
        return false;

        /*// Returns false if agent was not recovering.
        if ( mDamageRecoveryPhase == 0 )
            return false;

        // Keep track of time since we entered recovery.
        mDamageRecoveryTime += timeDelta;
        
        // Are we using the dynamics system (ragdoll) to provide the animation for 
        // when the agent needs to react to taking damage?
        if ( mSupportsRagdoll && mDynamicDamageAnimation )
        {
            // Ragdoll is still simulating (1), or is animation being restored (2)?
            if ( mDamageRecoveryPhase == 1 )
            {
                // Allow ragdoll to simulate for 200ms (set up in 'beginDamageRecovery()')
                if ( mDamageRecoveryTime > 0.2f )
                {
                    // Ragdoll is still in effect. First find the root actor bone to which 
                    // the ragdoll controller is assigned.
                    ObjectNode @ rootBone = mNode.findChild( "joint0" );
                    
                    // Get the assigned ragdoll controller and restore the original bone 
                    // hierarchy before destroying the controller.
                    RagdollController @ ragdoll = cast<RagdollController>(rootBone.getPhysicsController());
                    ragdoll.restoreHierarchy( false );
                    rootBone.setPhysicsController( null );

                    // Take a snapeshot of the current pose of the actor, set it to the controller in its
                    // own track at full strength, and then set it to fade out so that we blend from the
                    // last state of the ragdoll pose and the current state of the animation.
                    mActor.playAnimationSet( "RecoverSourcePose", "_ActorPoseSnapshot", AnimationPlaybackMode::Loop, 0.0f, 0.0f, 1.0f, 1.0f );
                    mActor.stopAnimationTrack( "RecoverSourcePose", false );

                    // Resume the last played animation, fading back in.
                    selectAnimation( mCurrentAnim, mCurrentAnimMode, mCurrentAnimPos );

                    // Wait until animation fade is complete (phase 2)
                    mDamageRecoveryPhase = 2;

                } // End if > 200ms

            } // End if phase 1
            else
            {
                // Wait until animation is restored to complete recovery.
                if ( !mActor.isAnimationTrackPlaying( "RecoverSourcePose", true ) )
                    mDamageRecoveryPhase = 0;

            } // End if phase 2

        } // End if dynamics based animation

        // Still in damage recovery?
        return (mDamageRecoveryPhase != 0);*/
    }

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    private Threat@ getThreat( Agent@ agent )
    {
        Threat@ currentThreat = threatTable;
        while ( @currentThreat != null )
        {
            // Clean up the threat list as we go.
            if ( !currentThreat.agent.isAlive() )
            {
                @currentThreat = removeThreat( currentThreat );
            
            } // End if dead
            else
            {
                if ( @currentThreat.agent == @agent )
                    return currentThreat;
                @currentThreat = currentThreat.next;
            
            } // End if alive
        
        } // Next threat

        // Not found.
        return null;
    }

    private void promoteThreat( Agent@ agent, float damage )
    {
        // Find the threat
        Threat @ threat = getThreat( agent );
        if ( @threat == null )
            return;

        // Add the damage
        threat.damage += damage;

        // Will this promotion require us to re-sort the threat list?
        if ( @threat.previous != null && threat.damage > threat.previous.damage )
        {
            // Remove threat and re-add in the appropriate location
            // with the new damage.
            removeThreat( threat );
            addThreat( threat );
            
        } // End if sort required
    }

    private void promoteThreat( Threat@ threat, float damage )
    {
        // Remove threat and re-add with the new damage.
        removeThreat( threat );
        threat.damage += damage;
        addThreat( threat );
    }

    private Threat@ addThreat( Agent@ agent, ObjectNode@ node, float initialDamage )
    {
        Threat@ newThreat   = Threat();
        @newThreat.agent    = agent;
        @newThreat.node     = node;
        @newThreat.next     = null;
        @newThreat.previous = null;
        newThreat.damage    = initialDamage;
        addThreat( newThreat );
        return newThreat;
    }

    private void addThreat( Threat @ newThreat )
    {
        // Attach to threat table in decreasing order.
        Threat@ threat = threatTable;
        while ( @threat != null )
        {
            // If initial damage is greater than the damage applied
            // by this threat so far, it should be inserted just before.
            if ( newThreat.damage > threat.damage )
                break;
            @threat = threat.next;
        
        } // Next threat

        // If no threat was found, we should insert at the end of the list.
        if ( @threat == null )
        {
            @newThreat.next = null;
            @newThreat.previous = threatTableTail;
            if ( @threatTableTail != null )
                @threatTableTail.next = newThreat;
            @threatTableTail = newThreat;

            // This becomes the head of the threat table list
            // if there are no current threats.
            if ( @threatTable == null )
                @threatTable = newThreat;
        
        } // End if none found
        else
        {
            // Insert just before the discovered threat
            @newThreat.next     = threat;
            @newThreat.previous = threat.previous;
            if ( @threat.previous != null )
                @threat.previous.next = newThreat;
            @threat.previous    = newThreat;

            // This becomes the head if necessary.
            if ( @threatTable == @threat )
                @threatTable = newThreat;

        } // End if found insert point
    }

    private Threat @ removeThreat( Threat@ threat )
    {
        Threat @ returnThreat = threat.next;

        // Remove from middle of list.
        if ( @threat.previous != null )
            @threat.previous.next = threat.next;
        if ( @threat.next != null )
            @threat.next.previous = threat.previous;

        // Replaces head / tail?
        if ( @threatTable == @threat )
            @threatTable = threat.next;
        if ( @threatTableTail == @threat )
            @threatTableTail = threat.previous;

        // Clear current pointers
        @threat.previous = null;
        @threat.next = null;

        // Return next threat in list
        return returnThreat;
    }
    
} // End class : NPCAgent