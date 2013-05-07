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
#include_once "Agent States/IdleState.gs"
#include_once "Agent States/DeadState.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
shared class NPCAgentDebug
{
    bool  willingToFire;
    bool  inDetectionCones;
    bool  inLoS;
    float angleH;
    float angleV;
    float range;
    String stateName;

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
    Vector3                         targetLastKnownPos;         // Last known position of target.
    bool                            targetAcquired;             // We have acquired an enemy target.
    bool                            willingToFire;              // Agent is willing to fire (target is in firing range / cone angles).
    
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
    float                           minVisTestRange;            // Range within which visibility tests are not required.
    bool                            supportsRagdoll;            // Should a ragdoll be created when the agent dies (Default=true)?
    bool                            dynamicDamageAnimation;     // Use the dynamics system (ragdoll) for the agent animation when hit by projectiles, etc.
    bool                            canFireWhileMoving;         // Agent can fire while moving?
    String                          idleAnimationName;
    String                          reloadAnimationName;
    String                          walkAnimationName;
    String                          strafeLeftAnimationName;
    String                          strafeRightAnimationName;
    String                          firingAnimationName;
    String                          ragdollRootJoint;

    // Debug
    NPCAgentDebug                   debugStats;
    bool                            debugEnabled;

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : NPCAgent () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	NPCAgent( )
    {
        debugEnabled                = false;
        maxSpeed                    = 3.0f;
        maxDetectionConeH           = 200.0f;
        maxDetectionConeV           = 90.0f;
        maxDetectionRange           = 50.0f;
        maxAudioDetectionRange      = 50.0f;
        maxFiringConeH              = 5.0f;
        maxFiringConeV              = 45.0f;
        maxFiringRange              = 40.0f;
        minVisTestRange             = 2.0f;
        supportsRagdoll             = true;
        dynamicDamageAnimation      = false;
        canFireWhileMoving          = true;
        idleAnimationName           = "Weapon Idle";
        reloadAnimationName         = "Weapon Reload";
        walkAnimationName           = "";
        strafeLeftAnimationName     = "";
        strafeRightAnimationName    = "";
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

        // Initialize variables
        timeSinceTargetCheck    = 0;
        timeSinceTargetAcquire  = 0;
        timeSinceTargetLost     = 99999999.0f;
        timeSinceTargetHeard    = 99999999.0f;
        targetAcquired          = false;
        damageRecoveryPhase     = 0;
        requestedOrientation    = object.getWorldTransform().orientation();

        // Get properties
        PropertyContainer @ properties = object.getCustomProperties();
        ragdollRootJoint = String(properties.getProperty( "ragdoll_root", "" ));
        if ( ragdollRootJoint == "" )
            supportsRagdoll = false;

        // Call base class implementation to update base members.
        Agent::onAttach( object );

        // Select the initial state of this agent
        switchState( IdleState( this ) );

	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        @playerNode  = null;
        @playerAgent = null;
        @actor       = null;

        // Call base class implementation last
        Agent::onDetach( object );
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Output debug stats
        if ( debugEnabled )
            debugStats.stateName = ( @mState != null ) ? mState.getStateName() : "<None>";

        // Keep turning toward requested orientation.
        Quaternion currentOrientation = actor.getWorldTransform().orientation();
        quaternionSlerp( currentOrientation, currentOrientation, requestedOrientation, elapsedTime * 7 );
        actor.setOrientation( currentOrientation );

        // Update this NPCs target status (is one nearby, have we lost our previous, etc.)
        updateTargetStatus( elapsedTime );

        // Call base class implementation.
        Agent::onUpdate( elapsedTime );
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (Agent)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : getHeight ()
	// Desc : Calculate current height of the agent.
	//-------------------------------------------------------------------------
    float getHeight( )
    {
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

        // Compute total damage
        float damage = weapon.computeDamage( vec3Length( to - from ) );

        // Scale base damage by damage scale amount if any is available.
        PropertyContainer @ properties = intersectedNode.getCustomProperties();
        damage *= float(properties.getProperty( "dmg_mult", 1.0f ));

        // Apply damage to armor (if there is any)
        float armorDamage = min( mCurrentArmor, damage );
        mCurrentArmor -= armorDamage;
        damage -= armorDamage;

        // Apply damage to health (if there is any)
        float healthDamage = min( mCurrentHealth, damage );
        mCurrentHealth -= healthDamage;
        damage -= healthDamage;

        // Request that we turn toward wherever the shot came from if it
        // is the player that shot.
        PlayerAgent @ playerAgent = cast<PlayerAgent>(sourceAgent);
        if ( @playerAgent != null )
        {
            targetLastKnownPos = playerAgent.getSceneNode().getPosition();
            turnTowardPosition( targetLastKnownPos );
        
        } // End if player

        // Kill the NPC if there is no health left, otherwise perform
        // damage reaction operations (animation, etc.)
        Vector3 impulse;
        vec3Normalize( impulse, to - from );
        if ( mCurrentHealth <= CGE_EPSILON )
            kill( intersectedNode, to, impulse * 40.0f );
        else
            beginDamageRecovery( intersectedNode, to, impulse * 20.0f );
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
            ObjectNode @ rootBone = actor.findChild( "joint0" );
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
                turnTowardPosition( targetLastKnownPos );
            
            } // End if not already acquired

        } // End if !dead
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
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
            controller.setCharacterHeight( 2.0f );
            controller.setCharacterRadius( 0.4f );
            controller.setMaximumWalkSpeed( maxSpeed );
            controller.setWalkAcceleration( 20.0f );
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

        // Switch to the dead state.
        switchState( DeadState( this, hitNode, hitPoint, hitImpulse ) );
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
	// Name : lineOfSightToTarget ()
	// Desc : Check if we have line of sight to target. Returns it as a
    //        0-1 percentage.
	//-------------------------------------------------------------------------
    float lineOfSightToTarget( )
    {
        Vector3 direction, up(0, 1, 0), right;
        Vector3 srcPos = actor.getPosition();
        Vector3 destPos = playerNode.getPosition();
        vec3Normalize( direction, destPos - srcPos );
        vec3Cross( right, direction, up );
        vec3Normalize( right, right );

        // Compute the relative height of the source so we can use roughly
        // eye level as the source of the ray cast.
        float srcHeight = getHeight();
        srcPos += actor.getYAxis() * (srcHeight * 0.95f);

        // Destination positions are always in front of the player, toward
        // the NPC.
        srcPos  += direction * 0.6f; // ToDo: Rather than shifting the source position, use raycast filtering to exclude both characters
        destPos -= direction * 0.6f;

        // Test 6 points in a grid for visibility, from center of the character's
        // height, to its top.
        CollisionContact contact;
        PhysicsWorld @ world = actor.getScene().getPhysicsWorld();
        float destHeight = playerAgent.getHeight();
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
	// Name : isPlayerVisible ()
	// Desc : Determine if the player is currently visible to this NPC.
	//-------------------------------------------------------------------------
    bool isPlayerVisible( bool & canFire, bool & performedLoS, float & visibilityScore )
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

        // Player is dead?
        if (! playerAgent.isAlive() )
            return false;

        // Get the direction to target.
        Vector3 srcPos = actor.getPosition(), destPos = playerNode.getPosition();
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

        // If the player was NOT previously visible (within a period of time), 
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
            visibilityScore = lineOfSightToTarget();
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
	// Name : updateTargetStatus ()
	// Desc : Check to see if an enemy target is visible / within range.
	//-------------------------------------------------------------------------
    void updateTargetStatus( float elapsedTime )
    {
        timeSinceTargetCheck   += elapsedTime;
        timeSinceTargetAcquire += elapsedTime;
        timeSinceTargetLost    += elapsedTime;
        timeSinceTargetHeard   += elapsedTime;

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
                if ( !isPlayerVisible( willingToFire, performedLoS, targetVisibilityScore ) )
                {
                    // We have lost our target.
                    targetAcquired = false;
                    timeSinceTargetLost = 0;
                
                } // End if !visible

            } // End if acquired
            else
            {
                // Can we see our target?
                if ( isPlayerVisible( willingToFire, performedLoS, targetVisibilityScore ) )
                {
                    targetAcquired = true;
                    timeSinceTargetAcquire = 0;
                
                } // End if visible

            } // End if !acquired

            // If the player has fired recently, update known position
            // if it's within audio detection range (unless acquired, and
            // then we don't need to know).
            if ( !targetAcquired )
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
                            targetVisibilityScore = lineOfSightToTarget();
                        if ( (targetVisibilityScore > CGE_EPSILON) || (distanceToTarget <= (maxAudioDetectionRange * 0.5f)) )
                        {
                            timeSinceTargetHeard = 0.0f;
                            targetLastKnownPos = playerNode.getPosition();
                        
                        } // End if LoS check
                    
                    }  // End if within range
                
                } // End if recently fired weapon

            } // End if !acquired

        } // End if update due

        // Record last known target position if acquired
        if ( targetAcquired )
            targetLastKnownPos = playerNode.getPosition();
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
    
} // End class : NPCAgent