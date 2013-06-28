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
// Name : GamePlay.gs                                                        //
//                                                                           //
// Desc : Script containing the majority of the logic for controlling how    //
//        primary gameplay plays out in the Colony VI example game.          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------
#include_once "../API/AgentManager.gsh"
#include_once "../API/AudioManager.gsh"
#include_once "../Behaviors/PlayerAgent.gs"
#include_once "../Behaviors/DestructibleLight.gs"
#include_once "../Behaviors/EmergencyLight.gs"

//-----------------------------------------------------------------------------
// Enumerations and Constants
//-----------------------------------------------------------------------------
shared enum ReactorShutdownStage
{
    None,
    FiveMinutes,
    FiveMinutesRepeat,
    FourMinutes,
    ThreeMinutes,
    SixtySeconds,
    ThirtySeconds,
    Final,
    Explode
};

shared enum AgentSpawnContext
{
    Survival_Scene  = 1000001
};

const float ScreenMessageFadeTime = 0.5f;
const float ExplosionFadeTime = 5.0f;

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
shared class ScreenMessage
{
    String message;
    int    offset;
    int    height;
    float  displayTime;
};

//-----------------------------------------------------------------------------
// Name : GamePlay (Class)
// Desc : Scripted application state object used as the main entry point for
//        this framework demonstration.
//-----------------------------------------------------------------------------
shared class GamePlay : IScriptedAppState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AppState@               mState;                 // Application side state object
    private AppState@               mLoadingState;
	private int                     mReferenceId;           // The reference identifier for this state.
    private bool                    mShowDebug;             // Show debug statistics.
    private World@                  mWorld;                 // Main game world object.
    private Scene@                  mScene;                 // The scene we've loaded.
    private RenderView@             mSceneView;             // Render view into which the scene will be rendered.

    private AudioManager@           mAudioManager;          // Manager handling game audio resources and playback.

    private ObjectNode@             mPlayerNode;            // Object node used to represent dummy player object.
    private PlayerAgent@            mPlayer;                // Cached reference to the main player behavior.
    private AgentManager@           mAgentManager;          // Manager responsible for handling agents in the scene.

    // Screen Elements
    private array<ScreenMessage@>   mScreenMessages;
    private float                   mScreenMessageDisplayTime;

    // Game sequence
    private bool                    mVolumeDipped;
    private bool                    mNarrationDipEnabled;
    private bool                    mProcessScene;
    private String                  mSequenceIdentifier;    // Current gameplay sequence.
    private double                  mSequenceTime;          // Amount of time spent in the current sequence so far.
    private int                     mSequenceStep;          // Internal tracking variable for tasks processed in this sequence.
    private double                  mSequenceStepTime;      // Amount of time spent in the current step of this sequence so far.
    private bool                    mProcessQuakes;
    private bool                    mQuakeDustEnabled;
    private double                  mQuakeTimer;
    private ReactorShutdownStage    mReactorStage;
    private IntervalTimer           mReactorTimer;
    private SoundRef@               mFinalSceneHeartBeat;
	
    private SoundRef@               mHelicopter;
	private bool 					mHelicopterComing;
	private float					mHelicopterStepTime;
	private float					mHelicopterRampTime;

    // Pre-warmed resource cache
    private array<MeshHandle>           mMeshCache;
    private array<ScriptHandle>         mScriptCache;
    private array<AudioBufferHandle>    mSoundCache;
    private array<AnimationSetHandle>   mAnimationCache;
    private array<WorldObject@>         mObjectCache;

    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedAppState)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : initialize ()
    // Desc : The state has been registered and is being initialized (usually
    //        at application startup), but is not yet necessarily being 
    //        activated. The script can perform any registration time 
    //        processing at this point.
    //-------------------------------------------------------------------------
    bool initialize( AppState @ state )
    {
        // Store required values.
        @mState             = state;
        @mLoadingState      = null;
		mReferenceId        = mState.getReferenceId();
        mShowDebug          = false;
        mSequenceIdentifier = "";
        mProcessQuakes      = false;
        mQuakeDustEnabled   = false;
        mQuakeTimer         = 0;
	
        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : begin ()
    // Desc : This event signifies that the state has actually been selected
    //        and activated by the state management system. This will generally
    //        be the point at which any specific resources relevant for the
    //        execution of this state will be built/loaded.
    //-------------------------------------------------------------------------
    bool begin( )
    {
        // Set initial simulation speed (in case it was modified).
        getAppTimer().setSimulationSpeed( 1.0f );

        // Renable resource destruction (this only applies if the level is restarting and
        // destruction was disabled before the scene was unloaded).
        getAppResourceManager().enableDestruction( true );

        // Switch in to the loading state.
        @mLoadingState = mState.spawnChildState( "Loading", true );
        mLoadingState.render();

        // Retrieve the application's main world object instance.
        @mWorld = getAppWorld();

        // Initialize the state
        @mAudioManager          = AudioManager( 32 );
        mReactorStage           = ReactorShutdownStage::None;
        @mFinalSceneHeartBeat   = null;
        mScreenMessageDisplayTime = 2.0f;
        mProcessScene           = true;
        mVolumeDipped           = false;
        mNarrationDipEnabled    = true;

		@mHelicopter            = null;
		mHelicopterComing       = false;
		mHelicopterStepTime     = 0;
		mHelicopterRampTime     = 60.0;
		
        // Pre-warm resources.
        preloadResources();

        // Allocate a new scene rendering view. This represents a collection of
        // surfaces / render targets, into which the scene will be rendered. It
        // is possible to create more than one scene view for multiple outputs
        // (perhaps split screen multi player) but in this case we only need one
        // view that spans the entire screen.
        RenderDriver @ renderDriver = getAppRenderDriver();
        @mSceneView = renderDriver.createRenderView( "Game View", ScaleMode::Relative, RectF(0,0,1,1) );

        // Load the main scene
        if ( !loadScene( ) )
            return false;

        // Register event handlers with agent manager.
        mAgentManager.registerOnAgentDeath( AgentDeathEvent( this.onAgentDeath ) );

        // Load the HUD elements.
        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.addImageLibrary( "Textures/UI/HUDElements.xml", "HUDElements" );
        interfaceManager.addFont( "Textures/UI/Fonts/HUDFont_012.fnt" );
        interfaceManager.addFont( "Textures/UI/Fonts/ScreenFont_020.fnt" );

        // Switch to direct mouse input mode (no cursor)
        InputDriver @ inputDriver = getAppInputDriver();
        inputDriver.setMouseMode( MouseHandlerMode::Direct );

        // Warm the timer.
        for ( int i = 0; i < 20; ++i )
            getAppTimer().tick();

        // Play initial gameplay music.
        mAudioManager.playAmbientTrack( "BGExp", "Music/0050_explosions_distant.ogg", 0.2f, 0.2f, true );
        mAudioManager.playAmbientTrack( "BGAmb", "Music/0010_ambient_012.ogg", 0.5f, 0.5f, true );
		
        // Switch to the first sequence.
        setCurrentGameSequence( "spawn" );

        //setCurrentGameSequence( "trigger_generator_4" );

        // Kill loading state.
        mLoadingState.end();
        @mLoadingState = null;

        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : end ()
    // Desc : This state is no longer required / running, and should clean up
    //        any allocated resources.
    //-------------------------------------------------------------------------
    void end( )
    {
        // Clear pre-warmed resource cache
        clearResourceCache();

        // Unload resources.
        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.removeImageLibrary( "HUDElements" );

        // Clear allocated arrays
        mScreenMessages.resize(0);

        // Dispose of loaded agents.
        mAgentManager.shutdown();

        // Dispose of scene rendering view
        if ( @mSceneView != null )
            mSceneView.deleteReference();
        
        // Unload the scene
        if ( @mScene != null )
            mScene.unload();

        // Stop and unload sounds.
        if ( @mAudioManager != null )
            mAudioManager.shutdown();

        // Restore simulation speed (in case it was modified).
        getAppTimer().setSimulationSpeed( 1.0f );

        // Clean up
        @mLoadingState  = null;
        @mPlayer        = null;
        @mPlayerNode    = null;
        @mSceneView     = null;
        @mScene         = null;
        @mWorld         = null;
        @mAgentManager  = null;
        @mAudioManager  = null;
    }

    //-------------------------------------------------------------------------
    // Name : suspend ()
    // Desc : Called by the game state manager in order to notify us that this
    //        state has entered into a suspended state.
    //-------------------------------------------------------------------------
    void suspend( )
    {
        // Pause all currently playing sound effects.
        if ( @mAudioManager != null )
            mAudioManager.pauseSounds();
    }

    //-------------------------------------------------------------------------
    // Name : resume ()
    // Desc : Called by the game state manager in order to notify us that this
    //        state has resumed from its prior suspended state.
    //-------------------------------------------------------------------------
    void resume( )
    {
        // Resume all previously playing sound effects
        if ( @mAudioManager != null )
            mAudioManager.resumeSounds();
    }

    //-------------------------------------------------------------------------
    // Name : raiseEvent ()
    // Desc : Called by the game state manager in order to notify us that an
    //        event is being raised on this state.
    //-------------------------------------------------------------------------
    bool raiseEvent( const String & eventId )
    {
        if ( eventId == "Restart" )
        {
            // Delay the unloading of all resources.
            getAppResourceManager().enableDestruction( false );
        
        } // End if restart

        // Allow event to continue
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : update ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to perform any processing in its entirety
    //        prior to the rendering process.
    //-------------------------------------------------------------------------
    void update( )
    {
        // Run the update process if the state is not currently suspended.
        if ( !mState.isSuspended() )
        {
            // Allow the scene to perform any necessary update tasks.
            if ( mProcessScene && @mScene != null )
                mScene.update();

            // Handle screen message lifetime
            float elapsedTime = getAppTimer().getTimeElapsed();
            updateScreenMessages( elapsedTime );

            // Handle game sequence.
            updateCurrentGameSequence( elapsedTime );

            // Allow agent manager to process.
            if ( @mAgentManager != null )
                mAgentManager.process( elapsedTime );

            // Allow audio manager to process LAST so that
            // everyone still has access to their channel data.
            if ( @mAudioManager != null )
            {
                mAudioManager.process( elapsedTime );

                // Dip volumes whenever narration is playing
                if ( mNarrationDipEnabled )
                {
                    if ( mAudioManager.isAmbientTrackPlaying( "Narration" ) )
                    {
                        if ( !mVolumeDipped )
                        {
                            mAudioManager.setVolumeScale( 0.4f );
                            mVolumeDipped = true;
                        
                        } // End if !dipped
                    
                    } // End if narration playing
                    else
                    {
                        // Restore volumes
                        if ( mVolumeDipped )
                        {
                            mAudioManager.setVolumeScale( 1.0f );
                            mVolumeDipped = false;
                        
                        } // End if dipped

                    } // End if no narration
                
                } // End if dip enabled.
            
            } // End if has audio manager

            // Is the player dead?
            if ( @mPlayer != null && !mPlayer.isAlive() )
            {
                // Wait 7 seconds then restart.
                if ( mPlayer.getTimeSinceDamage() > 7 )
                    mState.raiseEvent( "Restart" );
            
            } // End if dead

            // Pause on escape.
            InputDriver @ inputDriver = getAppInputDriver();
            if ( inputDriver.isKeyPressed( Keys::Escape, true ) )
                mState.raiseEvent( "Pause" );

            // Process random ground quakes if enabled.
            if ( mProcessQuakes )
            {
                // Check every 500ms
                mQuakeTimer += elapsedTime;
                if ( mQuakeTimer > 0.5f )
                {
                    mQuakeTimer = 0.0f;
                    if ( randomFloat( 0, 1 ) < 0.05f )
                        groundQuake();
                
                } // End if elapsed
            
            } // End if quakes enabled

            if ( inputDriver.isKeyPressed( Keys::F7, true ) )
            {
                groundQuake();
                setCurrentGameSequence( "survival_scene_1" );

                // Start playing reactor countdown.
                mReactorStage = ReactorShutdownStage::SixtySeconds;
                mReactorTimer.setInterval( 5.0f, 5.0f ); // Play in 5 seconds
            }

        } // End if !suspended
    }

    //-------------------------------------------------------------------------
    // Name : render ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to render whatever is necessary.
    //-------------------------------------------------------------------------
    void render( )
    {
        if ( @mSceneView == null )
            return;

        // Start rendering to our created scene view (full screen output in this case).
        if ( mSceneView.begin() )
        {
            // Allow the scene to render
            if ( mProcessScene && @mScene != null )
                mScene.render();

            // Render the HUD elements.
            drawHUD();

            // Draw some debug statistics.
            if ( mShowDebug )
            {
                UIManager @ interfaceManager = getAppUIManager();
                interfaceManager.selectDefaultFont( );

                // Get the current state of the character (airborne, walking, etc.) in order 
                // to print this information to the screen
                String output = "Walking";
                CharacterController @ controller = cast<CharacterController>(mPlayerNode.getPhysicsController());
                CharacterState state = controller.getCharacterState();
                if ( state == CharacterState::Airborne )
                    output = "Airborne";
                else if ( state == CharacterState::OnRamp )
                    output = "Sliding";

                // Append the current 'standing' mode of the character too (i.e. is
                // the character currently crouching, etc.).
                CharacterStandingMode mode = controller.getActualStandingMode();
                if ( mode == CharacterStandingMode::Standing )
                    output += "\nStanding";
                else if ( mode == CharacterStandingMode::Crouching )
                    output += "\nCrouching";
                else if ( mode == CharacterStandingMode::Prone )
                    output += "\nProne";
                
                // Print it to the screen.
                Size screenSize = mSceneView.getSize();
                Rect rcScreen( 10, 10, screenSize.width - 10, screenSize.height - 10 );
                interfaceManager.printText( rcScreen, output, TextFlags::Multiline | TextFlags::AlignRight, 0xFFFF0000 );

                // Find the closest agent
                NPCAgent @ closestAgent = null;
                float closestDistance = 9999999999.0f;
                for ( uint f = 0; f < mAgentManager.factions.length(); ++f )
                {
                    Faction @ faction = mAgentManager.factions[f];
                    for ( uint i = 0; i < faction.agents.length(); ++i )
                    {
                        AgentData@ data = faction.agents[i];

                        // Agent has been unloaded?
                        if ( !data.active || data.node.isDisposed() || !data.agent.isAlive() )
                            continue;

                        // Agent must be an NPC.
                        NPCAgent @ agent = cast<NPCAgent>(data.agent);
                        if ( @agent == null )
                            continue;

                        // Closest so far?
                        float distance = vec3Length(data.node.getPosition() - mPlayerNode.getPosition());
                        if ( distance < closestDistance )
                        {
                            @closestAgent = agent;
                            closestDistance = distance;
                        
                        } // End if closer

                    } // Next agent

                } // Next faction

                // Output state description
                if ( @closestAgent != null )
                {
                    NPCAgentDebug @ d = closestAgent.debugStats;

                    // Agent
                    String output = "[c=#88AAAAFF]Closest Agent State[/c]\n";
                    Vector3 pos = closestAgent.getSceneNode().getPosition();
                    output += "Health/Armor: " + closestAgent.getCurrentHealth() + "/" + closestAgent.getCurrentArmor() + "\n";
                    output += "Position: " + pos.x + ", " + pos.y + ", " + pos.z + "\n";
                    output += "Current state: " + d.stateName + "\n";
                    output += "Animation: " + d.animationName + " (" + d.animationPos + " sec)\n";
                    output += "Angle to target H: " + d.angleH + "\n";
                    output += "Angle to target V: " + d.angleV + "\n";
                    output += "Range to target: " + d.range + "\n";
                    output += "Target acquired: " + ((d.acquired) ? "true" : "false") + "\n";
                    output += "Target damage: " + d.damageReceivedFromTarget + "\n";
                    output += "Target last known: <" + d.targetLastKnownPos.x + ", " + d.targetLastKnownPos.y + ", " + d.targetLastKnownPos.z + ">\n";
                    output += "Target in cones: " + ((d.inDetectionCones) ? "true" : "false") + "\n";
                    output += "Target in LoS: " + ((d.inLoS) ? "true" : "false") + " (" + int(d.visibilityScore*100) + "%)\n";
                    output += "Willing to fire: " + ((d.willingToFire) ? "true" : "false") + "\n";

                    if ( d.agentLog != "" )
                    {
                        output += "\n[c=#88AAFFAA]Agent's Log[/c]\n";
                        output += d.agentLog;
                    
                    } // End if has log

                    // Weapon
                    Weapon @ weapon = closestAgent.getCurrentWeapon();
                    if ( @weapon != null )
                    {
                        output += "\n[c=#88FFAAAA]Agent's Weapon[/c]\n";
                        output += "Class:" + weapon.getClass() + " (" + weapon.getIdentifier() + ")\n";
                        output += "Current state: ";
                        switch ( weapon.getState() )
                        {
                            case WeaponState::Ready:
                                output += "Ready\n";
                                break;
                            case WeaponState::Empty:
                                output += "Empty\n";
                                break;
                            case WeaponState::Firing:
                                output += "Firing\n";
                                break;
                            case WeaponState::Cooldown:
                                output += "Cooldown\n";
                                break;
                            default:
                                output += "Unknown\n";
                                break;
                        } // End switch

                        output += "Firing mode: ";
                        switch ( weapon.getFiringMode() )
                        {
                            case WeaponFiringMode::SingleShot:
                                output += "Single shot\n";
                                break;
                            case WeaponFiringMode::Burst:
                                output += "Burst\n";
                                break;
                            case WeaponFiringMode::FullBurst:
                                output += "Full Burst\n";
                                break;
                            case WeaponFiringMode::FullyAutomatic:
                                output += "Fully automatic\n";
                                break;

                        } // End switch

                    } // End if has weapon
                    
                    interfaceManager.printText( Point(10, 10), output, TextFlags::Multiline | TextFlags::AllowFormatCode, 0x88FFFFFF );
                
                } // End if found
            
            } // End if show debug

            // Present the view to the frame buffer
            mSceneView.end( );

        } // End if begun
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : addScreenMessage ()
    // Desc : Add a new text message to display on the screen.
    //-------------------------------------------------------------------------
    void addScreenMessage( const String & message )
    {
        ScreenMessage @ msg = ScreenMessage();
        msg.message = message;
        msg.height  = 0;
        msg.offset  = 0;
        msg.displayTime = 0;
        mScreenMessages.resize( mScreenMessages.length() + 1 );
        @mScreenMessages[ mScreenMessages.length() - 1 ] = msg;
    }

    //-------------------------------------------------------------------------
    // Name : setCurrentGameSequence ()
    // Desc : Switch to the specified gameplay sequence.
    //-------------------------------------------------------------------------
    void setCurrentGameSequence( const String & sequenceIdentifier )
    {
        // Reset sequence tracking
        mSequenceTime = 0;
        mSequenceStep = 0;
        mSequenceStepTime = 0;

        // Perform any initial sequence actions.
        if ( sequenceIdentifier == "spawn" )
        {
			// Initially deactivate player automatic armor recharging
            // in order to force them to recharge their suit
			mPlayer.enableAutoArmorRecharge( false );
            mPlayer.setCurrentArmor( 0 );
		
            // Three friendly troopers in main spawn area.
            mAgentManager.spawnAgent( GameAgentType::Trooper, 0x89D, true );
            mAgentManager.spawnAgent( GameAgentType::Trooper, 0x89B, true );
            mAgentManager.spawnAgent( GameAgentType::Trooper, 0x89F, true );

        } // End if spawn
        else if ( sequenceIdentifier == "spawn_armor_charge" )
        {
            // User triggered initial armor station
			
			// Allow automatic armor recharging now
			mPlayer.enableAutoArmorRecharge( true );

        } // End if spawn_armor_charge
        else if ( sequenceIdentifier == "spawn_elevator_travel" )
        {
			mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration03.ogg", 1.0f, 1.0f, false, true );
			addScreenMessage( "Your final orders are to infiltrate the facilty and deactivate the primary generator's coolant supply.");
			mScreenMessageDisplayTime = 6.0f;
			
        } // End if spawn_elevator_travel
        else if ( sequenceIdentifier == "hallway_turret_retreat" )
        {
		
        } // End if hallway_turret_retreat
        else if ( sequenceIdentifier == "trigger_generator_1" )
        {
            mAudioManager.playSound( "Sounds/Coolant Shutdown.ogg", true, false, 1.0f, mPlayer.getSceneNode().getPosition(), null );
			
        } // End if 'trigger_generator_1'
        else if ( sequenceIdentifier == "trigger_generator_2" )
        {
		    mAudioManager.playSound( "Sounds/Coolant Shutdown.ogg", true, false, 1.0f, mPlayer.getSceneNode().getPosition(), null );
        
        } // End if 'trigger_generator_2'
        else if ( sequenceIdentifier == "trigger_generator_3" )
        {
            mAudioManager.playSound( "Sounds/Coolant Shutdown.ogg", true, false, 1.0f, mPlayer.getSceneNode().getPosition(), null );
			
        } // End if 'trigger_generator_3'
        else if ( sequenceIdentifier == "trigger_generator_4" )
        {
            mAudioManager.playSound( "Sounds/Coolant Shutdown.ogg", true, false, 1.0f, mPlayer.getSceneNode().getPosition(), null );

            // If this is the final generator objective, deactivate hall  lights and 
            // activate the emergency lights. First, turn hallway lights off.
            array<ObjectNode@> nodes;
            mScene.getObjectNodesByType( RTID_MeshObject, nodes );
            for ( uint i = 0; i < nodes.length(); ++i )
            {
                ObjectNode @ node = nodes[i];
                if ( node.getBehaviorCount() == 0 )
                    continue;
                DestructibleLight @ light = cast<DestructibleLight>(node.getScriptedBehavior(0));
                if ( @light != null )
                    light.deactivate();
            
            } // Next node

            // Next, turn emergency lights on.
            mScene.getObjectNodesByType( RTID_SpotLightObject, nodes );
            for ( uint i = 0; i < nodes.length(); ++i )
            {
                ObjectNode @ node = nodes[i];
                if ( node.getBehaviorCount() == 0 )
                    continue;
                EmergencyLight @ light = cast<EmergencyLight>(node.getScriptedBehavior(0));
                if ( @light != null )
                    light.activate();
            
            } // Next node

            // Switch to escape music.
            mAudioManager.playAmbientTrack( "Music", "Music/Mistake the Getaway.ogg", 0, 0.3f, true );

			// Turn on alarm
			mAudioManager.playAmbientTrack( "BGAlarm", "Sounds/Generator Alarm.ogg", 0.4f, 0.4f, true );
			
			// Switch ambient background to something more menacing
			mAudioManager.playAmbientTrack( "BGAmb", "Music/0010_ambient_010.ogg", 0, 0.6f, true );

			// Start playing reactor countdown.
            mReactorStage = ReactorShutdownStage::FiveMinutes;
            mReactorTimer.setInterval( 5.0f, 5.0f ); // Play in 5 seconds

            // Enable quakes.
            mProcessQuakes = true;
            mQuakeDustEnabled = true;

        } // End if trigger_generator_4
        else if ( sequenceIdentifier == "escape_elevator_broken" )
        {
            // Spawn enemies in the warehouse
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xC24 );
            mAgentManager.spawnAgent( GameAgentType::Hound, 0xC26 );
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xC28 );
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xC2A );
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xC2C );
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xC2E );
            
            // and one just outside for good measure.
            mAgentManager.spawnAgent( GameAgentType::Orc, 0x3FA );

        } // End if escape_elevator_broken
        else if ( sequenceIdentifier == "collected_elevator_part" )
        {
            // Spawn another couple of enemies for the return journey.
            mAgentManager.spawnAgent( GameAgentType::Hound, 0x3D4 );
            mAgentManager.spawnAgent( GameAgentType::Orc, 0x82F );

        } // End if collected_elevator_part
        else if ( sequenceIdentifier == "escape_elevator_travel" )
        {
            // Turn off the steam emitters in the generator room.
            ObjectNode @ emitter = mScene.getObjectNodeById( 0xDFE );
            emitter.showNode( false, false );
            emitter.setUpdateRate( UpdateRate::Never );
            @emitter = mScene.getObjectNodeById( 0xC60 );
            emitter.showNode( false, false );
            emitter.setUpdateRate( UpdateRate::Never );
            @emitter = mScene.getObjectNodeById( 0xC5F );
            emitter.showNode( false, false );
            emitter.setUpdateRate( UpdateRate::Never );
            @emitter = mScene.getObjectNodeById( 0xDFD );
            emitter.showNode( false, false );
            emitter.setUpdateRate( UpdateRate::Never );
            @emitter = mScene.getObjectNodeById( 0xDFC );
            emitter.showNode( false, false );
            emitter.setUpdateRate( UpdateRate::Never );
            @emitter = mScene.getObjectNodeById( 0xC62 );
            emitter.showNode( false, false );
            emitter.setUpdateRate( UpdateRate::Never );
            @emitter = mScene.getObjectNodeById( 0xC61 );
            emitter.showNode( false, false );
            emitter.setUpdateRate( UpdateRate::Never );
            @emitter = mScene.getObjectNodeById( 0xDFB );
            emitter.showNode( false, false );
            emitter.setUpdateRate( UpdateRate::Never );

            // Turn on the steam emitters in the hangar.
            @emitter = mScene.getObjectNodeById( 0xE07 );
            emitter.setUpdateRate( UpdateRate::Always );
            @emitter = mScene.getObjectNodeById( 0xE05 );
            emitter.setUpdateRate( UpdateRate::Always );
            @emitter = mScene.getObjectNodeById( 0xE06 );
            emitter.setUpdateRate( UpdateRate::Always );

            // Stop spawning dust when quakes happen.
            mQuakeDustEnabled = false;

        } // End if escape_elevator_travel
        else if ( sequenceIdentifier == "survival_scene_1" )
        {
            // Spawn an orc and a mech ready to leap down.
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xDC2, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );
            mAgentManager.spawnAgent( GameAgentType::StandardMech, 0xDC4, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );
		
        } // End if survival_scene_1
        else if ( sequenceIdentifier == "survival_scene_2" )
        {
            // Spawn an orc and a mech ready to leap down.
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xDCA, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );
            mAgentManager.spawnAgent( GameAgentType::StandardMech, 0xDCC, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );

        } // End if survival_scene_2
        else if ( sequenceIdentifier == "survival_scene_3" )
        {
            // Spawn an orc and a mech ready to leap down.
            mAgentManager.spawnAgent( GameAgentType::StandardMech, 0xDC0, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xDC6, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );

        } // End if survival_scene_3
        else if ( sequenceIdentifier == "survival_scene_4" )
        {
            // Spawn an orc and a mech ready to leap down.
            mAgentManager.spawnAgent( GameAgentType::StandardMech, 0xDC8, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xDCE, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );

        } // End if survival_scene_4
        else if ( sequenceIdentifier == "survival_scene_5" )
        {
            // Spawn an orc and a mech ready to leap down.
            mAgentManager.spawnAgent( GameAgentType::Orc, 0xDC2, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );
            mAgentManager.spawnAgent( GameAgentType::StandardMech, 0xDC4, AgentStateId::LeapDown, AgentSpawnContext::Survival_Scene );

        } // End if survival_scene_4
        else if ( sequenceIdentifier == "final_explosion" )
        {
            // Turn off regular quakes
            mProcessQuakes = false;

            // Disable automatic narration dipping.
            mNarrationDipEnabled = false;

            // Stop music and narration.
            mAudioManager.stopAmbientTrack( "Music" );
            mAudioManager.stopAmbientTrack( "Narration" );

            // Player should no longer take any damage.
            mPlayer.enableInvincibility( true );

            // Start playing the player heart beat, but make sure it is quiet to begin with.
            mAudioManager.stopSound( mFinalSceneHeartBeat );
            @mFinalSceneHeartBeat = mAudioManager.playSound( "Sounds/Heartbeat Normal.ogg", true, true, 0.0f );

        } // End if final_explosion
        else if ( sequenceIdentifier == "end_sequence" )
        {
            // Shut down scene processing.
            mProcessScene = false;
            mAgentManager.shutdown();

            // Restore time scales
            getAppTimer().setSimulationSpeed( 1 );

        } // End if end_sequence

        // We're now in the new sequence.
        mSequenceIdentifier = sequenceIdentifier;
    }

    //-------------------------------------------------------------------------
    // Name : getPlayer ()
    // Desc : Retrieve the player agent object.
    //-------------------------------------------------------------------------
    PlayerAgent @ getPlayer( )
    {
        return mPlayer;
    }

    //-------------------------------------------------------------------------
    // Name : getAudioManager ()
    // Desc : Retrieve the game play audio manager.
    //-------------------------------------------------------------------------
    AudioManager @ getAudioManager( )
    {
        return mAudioManager;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Events (AgentManager)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : onAgentDeath () (Private)
    // Desc : Triggered whenever an agent that is managed by our agent
    //        manager dies.
    //-------------------------------------------------------------------------
    void onAgentDeath( AgentDeathEventArgs @ e )
    {
        // If this agent was part of the survival sequence, increment the game sequence step.
        if ( e.agent.getSpawnContext() == AgentSpawnContext::Survival_Scene && mSequenceIdentifier != "final_explosion" )
            stepGameSequence();
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : groundQuake () (Private)
    // Desc : Shake the screen, spawn dust from the ceiling, etc.
    //-------------------------------------------------------------------------
    private void groundQuake( )
    {
        mPlayer.shakeCamera( 1.5f, 0.65f );

        // TODO: Placeholder sound
        mAudioManager.playSound( "Sounds/0027_dungeon_foley_hit_metal_03.ogg", true, false, 0.4f );

        // Spawn dust from all of the destructible lights
        if ( mQuakeDustEnabled )
        {
            array<ObjectNode@> nodes;
            mScene.getObjectNodesByType( RTID_MeshObject, nodes );
            for ( uint i = 0; i < nodes.length(); ++i )
            {
                ObjectNode @ node = nodes[i];
                if ( node.getBehaviorCount() == 0 || vec3Length(node.getPosition() - mPlayerNode.getPosition()) > 100.0f )
                    continue;
                DestructibleLight @ light = cast<DestructibleLight>(node.getScriptedBehavior(0));
                if ( @light != null )
                {
                    // 50% chance
                    if ( randomFloat( 0, 1 ) > 0.5f )
                    {
                        ObjectNode @ dustEmitter = mScene.loadObjectNode( 0xDF9, CloneMethod::ObjectInstance, true );
                        if ( @dustEmitter != null )
                            dustEmitter.setPosition( node.getPosition() );
                    
                    } // End if chance
                
                } // End if light
            
            } // Next node

        } // End if quake dust
    }

    //-------------------------------------------------------------------------
    // Name : stepGameSequence () (Private)
    // Desc : Move to the next step in the current game sequence.
    //-------------------------------------------------------------------------
    private void stepGameSequence( )
    {
        mSequenceStep++;
        mSequenceStepTime = 0;

        // Trigger the next update immediately so that sequence steps aren't
        // missed if more than one step is triggered before the next frame update.
        updateCurrentGameSequence( 0 );
    }

    //-------------------------------------------------------------------------
    // Name : updateCurrentGameSequence () (Private)
    // Desc : Perform any updates based on the current game sequence over time
    //        as necessary.
    //-------------------------------------------------------------------------
    private void updateCurrentGameSequence( float elapsedTime )
    {
        mSequenceTime += elapsedTime;
        mSequenceStepTime += elapsedTime;

		if ( mSequenceIdentifier == "spawn" )
        {
            switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > mScreenMessageDisplayTime )
                    {
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration00.ogg", 1.0f, 1.0f, false, true );
                        addScreenMessage( "Alright listen up. We've lost the colony and we're evacuating all of the civilians now. Evac for military personnel starts in a few minutes."); 
						stepGameSequence();
						mScreenMessageDisplayTime = 10.0f;
                    
                    } // End if elapsed
                    break;

                case 1:
                    if ( mSequenceStepTime > mScreenMessageDisplayTime )
                    {
                        addScreenMessage( "But before you go you're gonna leave them a little something to remember us by.");
                        stepGameSequence();
						mScreenMessageDisplayTime = 7.0f;
						
                    } // End if elapsed
                    break;

                case 2:
                    if ( mSequenceStepTime > mScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Head over to the armor recharge station first and top up. It's highlighted on your H.U.D..");
                        stepGameSequence();
						mScreenMessageDisplayTime = 15.0f;
                    
                    } // End if elapsed
                    break;

            } // End switch

        } // End if 'spawn'
        else if ( mSequenceIdentifier == "spawn_armor_charge" )
        {
            if ( mSequenceStep == 0 )
            {
				mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration01.ogg", 1.0f, 1.0f, false, true );
				addScreenMessage( "Remember, your armor will automatically regenerate, but it's slow. Recharge stations like this get the job done much faster.");
				stepGameSequence();
				mScreenMessageDisplayTime = 9.0f;
            }
			else
			{
				// If we are fully charged up, set the next goal and play the final message
				if ( mPlayer.getCurrentArmor() == mPlayer.getMaximumArmor() && mSequenceStep == 1 && mSequenceStepTime > 10.0f )
				{
					mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration02.ogg", 1.0f, 1.0f, false, true );
					addScreenMessage( "Good. Now take the elevator up to the maintenance level. We don't have alot of time so I'll be monitoring you the whole way.");
					stepGameSequence();
					mScreenMessageDisplayTime = 15.0f;
				}
			
			}
			
        } // End if spawn_armor_charge
        else if ( mSequenceIdentifier == "spawn_elevator_travel" )
        {
            switch (mSequenceStep)
            {
                case 0:
				
                    if ( mSequenceStepTime > 1.5f )
                    {
                        // Activate the turret.
                        mAgentManager.initializePreSpawnedAgent( GameAgentType::Turret, 0xDF5 );

                        ////////////////////////////////////////////////////////////////
                        // Hallway after spawn_elevator_travel
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3D6 );
                        mAgentManager.spawnAgent( GameAgentType::Hound, 0x82F );
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3D2, AgentStateId::UsingTurret ); // On turret!
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3D4 );
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3DA );
                        
                        // Back side hallways
                        mAgentManager.spawnAgent( GameAgentType::Hound, 0x3DC );
                        mAgentManager.spawnAgent( GameAgentType::Hound, 0x3DE );
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3E0 );
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3E2 );
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3E4 );
                        
                        // Generator room
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3EC );
                        mAgentManager.spawnAgent( GameAgentType::Orc, 0x3F0 );
                        mAgentManager.spawnAgent( GameAgentType::Hound, 0x3F6 );
                        
                        stepGameSequence();
                    }
                    break;
                case 1:
                    if ( mSequenceStepTime > 6.0f )
                    {
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration04.ogg", 1.0f, 1.0f, false, true );
                        addScreenMessage( "Alright, you're in. The corridor on your left leads to the coolant pump control room. We should be able to shut things down from there. Let's get moving.");
						mScreenMessageDisplayTime = 9.0f;
                        stepGameSequence();
	                    
                    } // End if elapsed
                    break;

            } // End switch
        
        } // End if 'spawn_elevator_travel'
        else if ( mSequenceIdentifier == "hallway_turret_retreat" )
        {
            switch ( mSequenceStep )
            {
                case 0:
                    if ( mSequenceStepTime > 1.0f )
                    {
                        mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration05.ogg", 1.0f, 1.0f, false, true );
                        addScreenMessage( "That turret's gonna cut you to ribbons. Head back the other way, go past the elevator, and take the service tunnels instead.");
                        stepGameSequence();
                        mScreenMessageDisplayTime = 8.0f;
                    
                    } // End if elapsed
                    break;

            } // End switch
		
        } // End if 'hallway_turret_retreat'
        else if ( mSequenceIdentifier == "trigger_generator_1" )
        {
            switch (mSequenceStep)
            {
                case 0:
					
                    if ( mSequenceStepTime > 1.2f )
                    {
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration06.ogg", 1.0f, 1.0f, false, true );
		                addScreenMessage( "Nice! That's the first one down. Just hit the three remaining pumps and this thing's gonna go critical.");
                        mScreenMessageDisplayTime = 6.0f;
                        stepGameSequence();
                        
                    } // End if elapsed
                    break;

            } // End switch

        } // End if 'trigger_generator_1'
        else if ( mSequenceIdentifier == "trigger_generator_4" )
        {
			switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > 1.0f )
                    {
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration07.ogg", 1.0f, 1.0f, false, true );
                        addScreenMessage( "Good work, that's all of them." );
                        mScreenMessageDisplayTime = 2.0f;
                        stepGameSequence();

                    } // End if elapsed
                    break;
                
                case 1:
                    if ( mSequenceStepTime > mScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Now get yourself back to the hangar. Evac is coming soon, but we're cutting it close.");
                        mScreenMessageDisplayTime = 5.0f;
                        stepGameSequence();
                    
                    } // End if elapsed
                    break;
            
            } // End switch

        } // End if 'trigger_generator_4'
        else if ( mSequenceIdentifier == "escape_elevator_broken" )
        {
            switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > 1.0f )
                    {
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration08.ogg", 1.0f, 1.0f, false, true );
                        addScreenMessage( "Damn! That surge from the generator must have fried the circuits.");
                        mScreenMessageDisplayTime = 3.5f;
                        stepGameSequence();

                    } // End if elapsed
                    break;
                
                case 1:
                    if ( mSequenceStepTime > mScreenMessageDisplayTime )
                    {
                        addScreenMessage( "OK, there's a supply warehouse down the corridor, back where you just were.");
                        mScreenMessageDisplayTime = 5.0f;
                        stepGameSequence();
                    
                    } // End if elapsed
                    break;

                case 2:
                    if ( mSequenceStepTime > mScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Head down the stairs under the coolant pump control room.");
                        mScreenMessageDisplayTime = 3.5f;
                        stepGameSequence();
                    
                    } // End if elapsed
                    break;
					
                case 3:
                    if ( mSequenceStepTime > mScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Go back and see if you can find a spare panel.");
                        mScreenMessageDisplayTime = 3.5f;
                        stepGameSequence();
                    
                    } // End if elapsed
                    break;
            
            } // End switch
        
        } // End if 'escape_elevator_broken'
        else if ( mSequenceIdentifier == "collected_elevator_part" )
        {
            switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > 1.0f )
                    {
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration09b.ogg", 1.0f, 1.0f, false, true );
                        addScreenMessage( "Yeah, that looks like the part, and it looks like you found yourself some new hardware too. That should come in handy. Now get back to the elevator as quick as you can, you don't have much time!");
                        mScreenMessageDisplayTime = 11.5f;
                        stepGameSequence();

                    } // End if elapsed
                    break;
            
            } // End switch
        
        } // End if 'collected_elevator_part'
        else if ( mSequenceIdentifier == "escape_elevator_repair" )
        {
			// Play a sound to indicate a repair
			// playSound( mRepairSound.getResource(true) );
        
        } // End if 'escape_elevator_entry'
        
        else if ( mSequenceIdentifier == "escape_elevator_entry" )
        {
            switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > 1.0f )
                    {
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration010.ogg", 1.0f, 1.0f, false, true );
                        addScreenMessage( "Good, it worked. Time to go. The birds are touching down soon, so haul ass!");
                        mScreenMessageDisplayTime = 7.0f;
                        stepGameSequence();

                    } // End if elapsed
                    break;
            
            } // End switch
        
        } // End if 'escape_elevator_entry'
        else if ( mSequenceIdentifier == "escape_elevator_travel" )
        {
            switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > 6.0f )
                    {
                        //addScreenMessage( "Uhhhh... that's... not good.");
                        // Switch to first survival scene wave
                        setCurrentGameSequence( "survival_scene_1" );
                    
                    } // End if elapsed
                    break;
            
            } // End switch
        
        } // End if 'escape_elevator_travel'
        else if ( mSequenceIdentifier == "survival_scene_1" )
        {
            switch (mSequenceStep)
            {
                //case 0:
                    // Both spawned agents are alive
                    //break;

                //case 1:
				
                    // First spawned agent is dead
                   // break;

                case 2:

					// Second spawned agent is dead
                    // After a couple of seconds, play the alien "skittering" sound.
                    if ( mSequenceStepTime > 4.0f )
                    {
                        mAudioManager.playSound( "Sounds/Wildlife 1.ogg", false, false, 0.2f );
                        stepGameSequence();

						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration012a.ogg", 1.0f, 1.0f, false, true );
						
                    } // End if elapsed
                    break;

                    // ToDo: Ideally it would be nice to have a couple more steps in this sequence
                    // with crawling through vent sounds, followed by more skittering, and THEN
                    // followed by the smash through the vent. We may be limited on a good sound effect
                    // for this though.

                case 3:
				
                    // After another few seconds, spawn up a hound.
                    if ( mSequenceStepTime > 3.0f )
                    {
                        // Throw open vent door
                        ObjectNode @ node = mScene.getObjectNodeById( 0xE39 ); // 0xE0C
                        node.setPhysicsModel( PhysicsModel::RigidDynamic );
                        node.applyImpulse( node.getZAxis() * 20.0f, node.getPosition() + node.getYAxis() * 0.5f );

                        // Play crash sound
                        mAudioManager.playSound( "Sounds/0027_dungeon_foley_hit_metal_01.ogg", false, false, 1.0f, node.getPosition(), null );

                        // Spawn a hound inside the vent (once it's dead, it will auto step the sequence).
                        mAgentManager.spawnAgent( GameAgentType::Hound, 0xE3C, AgentStateId::Searching, true, AgentSpawnContext::Survival_Scene ); // 0xE3E

                        // Step!
                        stepGameSequence();

                    } // End if elapsed
                    break;

                //case 4:
                    // Hound is alive during this step. Sequence will automatically
                    // step once we detect that it is dead.
                    //break;

                case 5:
                    
                    // Hound is now dead, after a couple of seconds, move on to the next wave.
                    if ( mSequenceStepTime > 2.0f )
                    {
						// Play some words of encouragement
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration012b.ogg", 1.0f, 1.0f, false, true );
					
                        setCurrentGameSequence( "survival_scene_2" );
                    
                    } // End if elapsed
                    break;
            
            } // End switch

        } // End if 'survival_scene_1'
        else if ( mSequenceIdentifier == "survival_scene_2" )
        {
            switch (mSequenceStep)
            {
                //case 0:
                    // Both spawned agents are alive
                    //break;

                //case 1:
                    // First spawned agent is dead
                    //break;

                case 2:
                    // Second spawned agent is dead.
                    if ( mSequenceStepTime > 4.0f )
                    {
						// Play some words of encouragement
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration012c.ogg", 1.0f, 1.0f, false, true );

						setCurrentGameSequence( "survival_scene_3" );
                    
                    } // End if elapsed
                    break;
            
            } // End switch

        } // End if 'survival_scene_2'
        else if ( mSequenceIdentifier == "survival_scene_3" )
        {
            switch (mSequenceStep)
            {
                //case 0:
                    // Both spawned agents are alive
                    //break;

                case 1:
                {
                    // First spawned agent is dead, spawn two hounds just for the hell of it.
                    // Throw open the second vent door
                    ObjectNode @ node = mScene.getObjectNodeById( 0xE0C );
                    node.setPhysicsModel( PhysicsModel::RigidDynamic );
                    node.applyImpulse( node.getZAxis() * 20.0f, node.getPosition() + node.getYAxis() * 0.5f );

                    // Play crash sound
                    mAudioManager.playSound( "Sounds/0027_dungeon_foley_hit_metal_01.ogg", false, false, 1.0f, node.getPosition(), null );

                    // Spawn a hound inside the second vent.
                    mAgentManager.spawnAgent( GameAgentType::Hound, 0xE3E, AgentStateId::Searching, true );

                    // Spawn another hound inside the first vent a second from now.
                    mAgentManager.spawnAgent( GameAgentType::Hound, 0xE3C, AgentStateId::Searching, false, 0, 1.0f );

                    // Step the sequence and wait for second agent to be killed.
                    stepGameSequence();
                    break;
                }
                //case 2:
                    // Waiting for second agent to be killed.
                    //break;

                case 3:
                    // Second spawned agent is dead.
                    if ( mSequenceStepTime > 5.0f )
                    {
						// Play some words of encouragement
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration012d.ogg", 1.0f, 1.0f, false, true );
					
                        setCurrentGameSequence( "survival_scene_4" );
                    
                    } // End if elapsed
                    break;
            
            } // End switch

        } // End if 'survival_scene_3'
        else if ( mSequenceIdentifier == "survival_scene_4" )
        {
            switch (mSequenceStep)
            {
                //case 0:
                    // Both spawned agents are alive
                    //break;

                //case 1:
                    // First spawned agent is dead
                    //break;

                case 2:
                    // Second spawned agent is dead.
                    if ( mSequenceStepTime > 4.0f )
                    {
                        setCurrentGameSequence( "survival_scene_5" );
                    
                    } // End if elapsed
                    break;
            
            } // End switch

        } // End if 'survival_scene_4'
        else if ( mSequenceIdentifier == "survival_scene_5" )
        {
            switch (mSequenceStep)
            {
                //case 0:
                    // Both spawned agents are alive
                    //break;

                case 1:
                {
                    // First spawned agent is dead, spawn two hounds just for the hell of it.
                    mAgentManager.spawnAgent( GameAgentType::Hound, 0xE3E, AgentStateId::Searching, true );

                    // Spawn another hound inside the second vent a second from now.
                    mAgentManager.spawnAgent( GameAgentType::Hound, 0xE3C, AgentStateId::Searching, false, 0, 1.0f );

                    // Step the sequence and wait for second agent to be killed.
                    stepGameSequence();
                    break;
                }
                //case 2:
                    // Waiting for second agent to be killed.
                    //break;

                case 3:
                    // Second spawned agent is dead.
                    if ( mSequenceStepTime > 3.0f )
                    {
                        // Loop scene 4->5->4->5 until the end.
						// Play some words of encouragement
						mAudioManager.playAmbientTrack( "Narration", "Sounds/Narration/narration012c.ogg", 1.0f, 1.0f, false, true );
						
                        setCurrentGameSequence( "survival_scene_4" );
                    
                    } // End if elapsed
                    break;
            
            } // End switch

        } // End if 'survival_scene_3'
        else if ( mSequenceIdentifier == "final_explosion" )
        {
            // Gradually add camera shake.
            float maxIntensity = 0.65f;
            float intensity = min( maxIntensity, (mSequenceStepTime / (ExplosionFadeTime/3.0f)) * maxIntensity );
            mPlayer.shakeCamera( 1, intensity );
            
            // Gradually increase light intensity.
            array<ObjectNode@> nodes;
            mScene.getObjectNodesInBounds( mPlayerNode.getPosition(), 100, nodes );
            for ( uint i = 0; i < nodes.length(); ++i )
            {
                if ( !nodes[i].queryReferenceType( RTID_PointLightNode ) )
                    continue;
                PointLightNode @ light = cast<PointLightNode>(nodes[i]);
                if ( @light != null )
                    light.setDiffuseHDRScale( light.getDiffuseHDRScale() + (5.0f * elapsedTime) );
            
            } // Next object

            // Gradually add DoF blur
            float maxBlur = 100.0f;
            float blur = min( maxBlur, (mSequenceStepTime / ExplosionFadeTime) * maxBlur );
            CameraNode @ camera = mPlayer.getCamera();
            camera.enableDepthOfField( true );
            camera.setForegroundExtents( 0, blur );
            camera.setForegroundBlur( 1, 2, -1, 1, 1, -1.0f );
            camera.setBackgroundExtents( 0, 0 );

            // Gradually adjust FOV
            float maxAdjust = 35.0f;
            float adjust = min( maxAdjust, (mSequenceStepTime / ExplosionFadeTime) * maxAdjust );
            camera.setFOV( 75.0f - adjust );

            // Gradually adjust simulation speed
            float speed = max( 0.5f, (1.0f - (mSequenceStepTime / ExplosionFadeTime)));
            getAppTimer().setSimulationSpeed( speed );
            
            // Slow down sound effects
            mAudioManager.setPitchScale( speed );
            mAudioManager.setVolumeScale( speed );

            // We want the heartbeat to play at normal volume and pitch.
            mAudioManager.setSoundPitch( mFinalSceneHeartBeat, 1.0f / speed );
            mAudioManager.setSoundVolume( mFinalSceneHeartBeat, 1.0f / speed );

            // Play outro sound at the relevant time.
            if ( mSequenceTime > ExplosionFadeTime - 1.0f && !mAudioManager.isAmbientTrackPlaying( "Outro" ) )
                mAudioManager.playAmbientTrack( "Outro", "Sounds/Narration/narration012e.ogg", 1.0f, 1.0f, false, true, true );

            // Switch to end sequence after enough time has elapsed
            if ( mSequenceTime > ExplosionFadeTime )
                setCurrentGameSequence( "end_sequence" );

        } // End if 'final_explosion'
        else if ( mSequenceIdentifier == "end_sequence" )
        {
            switch ( mSequenceStep )
            {
                case 0:
                {
                    // Continue to fade sounds.
                    float volume = max( 0.0f, (1.0f - (mSequenceTime / 3.0f)) ) * 0.5f;
                    if ( volume == 0.0f )
                    {
                        // Stop all sound effects.
                        mAudioManager.stopSounds();

                        // Stop relevant tracks once they reach silent volume.
                        mAudioManager.stopAmbientTrack( "BGExp" );
                        mAudioManager.stopAmbientTrack( "BGAmb" );
                        mAudioManager.stopAmbientTrack( "BGAlarm" );
                        mAudioManager.stopAmbientTrack( "Countdown" );

                        // Revert to full volume and pitch.
                        mAudioManager.setVolumeScale( 1 );
                        mAudioManager.setPitchScale( 1 );

                        // Move on to next step.
                        stepGameSequence();
	
                    } // End if stop
                    else
                    {
                        // Keep fading.
                        mAudioManager.setVolumeScale( volume );

                    } // End if fading

                } // End case step 0
                break;
			
            case 1:
			
                // Play music as the text appears on the screen.
                if ( mSequenceTime >= 5.0f )
                {
                    mAudioManager.playAmbientTrack( "Music", "Music/Mechanolith.ogg", 0.7f, 0.7f, false, true, true );
                    stepGameSequence();
                
                } // End if 5 seconds
                break;
            
            } // End switch
        
        } // End if end_sequence

        // Process countdown timers.
        if ( mReactorStage != ReactorShutdownStage::None && mReactorTimer.update(elapsedTime) )
        {
            switch ( mReactorStage )
            {
                case ReactorShutdownStage::FiveMinutes:
                    mAudioManager.playAmbientTrack( "Countdown", "Sounds/Narration/reactor5.ogg", 1.0f, 1.0f, false );
                    mReactorTimer.setInterval( 15.0f, 15.0f ); // Repeat 15 seconds from now.
                    mReactorStage = ReactorShutdownStage::FiveMinutesRepeat;
                    break;

                case ReactorShutdownStage::FiveMinutesRepeat:
                    mAudioManager.playAmbientTrack( "Countdown", "Sounds/Narration/reactor5.ogg", 1.0f, 1.0f, false );
                    mReactorTimer.setInterval( 34.0f, 34.0f ); // Play all other sounds 6 seconds early (60 - 5 - 15 - 6)
                    mReactorStage = ReactorShutdownStage::FourMinutes;
                    break;
                
                case ReactorShutdownStage::FourMinutes:
                    mAudioManager.playAmbientTrack( "Countdown", "Sounds/Narration/reactor4.ogg", 1.0f, 1.0f, false );
                    mReactorTimer.setInterval( 60.0f, 60.0f );
                    mReactorStage = ReactorShutdownStage::ThreeMinutes;
                    break;

                case ReactorShutdownStage::ThreeMinutes:
                    mAudioManager.playAmbientTrack( "Countdown", "Sounds/Narration/reactor3.ogg", 1.0f, 1.0f, false );
                    mReactorTimer.setInterval( 120.0f, 120.0f );
                    mReactorStage = ReactorShutdownStage::SixtySeconds;
                    break;

                case ReactorShutdownStage::SixtySeconds:
                    mAudioManager.playAmbientTrack( "Countdown", "Sounds/Narration/reactor60s.ogg", 1.0f, 1.0f, false );
                    mReactorTimer.setInterval( 30.0f, 30.0f );
                    mReactorStage = ReactorShutdownStage::ThirtySeconds;

                    // Start playing the helicopter, but make sure it is quiet to begin with.
                    mAudioManager.stopSound( mHelicopter );
                    @mHelicopter = mAudioManager.playSound( "Sounds/Helicopter2.ogg", true, true, 0.0f );
                    mHelicopterComing = true;
                    break;

                case ReactorShutdownStage::ThirtySeconds:
                    mAudioManager.playAmbientTrack( "Countdown", "Sounds/Narration/reactor30s.ogg", 1.0f, 1.0f, false );
                    mReactorTimer.setInterval( 20.0f, 20.0f );
                    mReactorStage = ReactorShutdownStage::Final;
                    break;

                case ReactorShutdownStage::Final:
                    mAudioManager.playAmbientTrack( "Countdown", "Sounds/Narration/reactor10s.ogg", 1.0f, 1.0f, false );
                    mReactorTimer.setInterval( 17.0f, 17.0f );
                    mReactorStage = ReactorShutdownStage::Explode;
                    break;

                case ReactorShutdownStage::Explode:
                    setCurrentGameSequence( "final_explosion" );
                    mReactorStage = ReactorShutdownStage::None;
                    break;

                
            } // End switch stage

        } // End if next reactor trigger

        // Play incoming chopper sound during final battle
		if ( mHelicopterComing )
		{
			float maxVolume = 0.9f;
			float helicopterVolume = min( maxVolume, (mHelicopterStepTime / mHelicopterRampTime) * maxVolume );
			mAudioManager.setSoundVolume( mHelicopter, helicopterVolume );
			mHelicopterStepTime += elapsedTime;
			
		} // End if helicopter
    }
    
    //-------------------------------------------------------------------------
    // Name : drawHUD () (Private)
    // Desc : Draw the player heads up display elements.
    //-------------------------------------------------------------------------
    private void drawHUD( )
    {
        RenderDriver @ driver = getAppRenderDriver();
        UIManager @ interfaceManager = getAppUIManager();
        Size screenSize = mSceneView.getSize();

        // First draw any current objective overlay
        drawObjectiveOverlay( );

        // Player is alive?
        float timeSinceDamage = mPlayer.getTimeSinceDamage();
        if ( mPlayer.isAlive() )
        {
            // Flash screen if player is taking damage
            float flashLength = 0.1f;
            if ( timeSinceDamage <= flashLength )
            {
                float alpha = sin( (timeSinceDamage / flashLength) * CGE_PI ) * 0.5f;
                driver.drawRectangle( Rect(0,0,screenSize.width, screenSize.height), ColorValue(1, 1, 1, alpha), true );
            
            } // End if damaged

        } // End if alive
        else
        {
            // Wait for a while and then fade out to black.
            float waitToFade = 3.0f;
            float fadeTime = 4.0f;
            if ( timeSinceDamage > waitToFade )
            {
                float alpha = min( 1.0f, (timeSinceDamage - waitToFade) / fadeTime );
                driver.drawRectangle( Rect(0,0,screenSize.width, screenSize.height), ColorValue(0, 0, 0, alpha), true );

            } // End if 3 seconds after death

        } // End if dead

        // Draw explosion white-out?
        if ( mSequenceIdentifier == "final_explosion" )
        {
            float alpha = max( 0.0f, min( 1.0f, ((mSequenceStepTime - (ExplosionFadeTime/3.0f)) / (ExplosionFadeTime / 3.0f) ) ) );
            driver.drawRectangle( Rect(0,0,screenSize.width, screenSize.height), ColorValue(1, 1, 1, alpha), true );

        } // End if final_explosion
        else if ( mSequenceIdentifier == "end_sequence" )
        {
            // Full white out
            driver.drawRectangle( Rect(0,0,screenSize.width, screenSize.height), ColorValue(1, 1, 1, 1), true );

        } // End if end_sequence
        
        // Select rendering font
        interfaceManager.selectFont( "HUDFont_012" );

        // Next draw the statistics and status background.
        interfaceManager.drawImage( Point( 10, screenSize.height - 165 ), "HUDElements", "stats_background" );

        // Draw current health.
        float maximumHealth = mPlayer.getMaximumHealth();
        float healthPercent = (maximumHealth > 0) ? (mPlayer.getCurrentHealth() / maximumHealth) : 0;
        int health = int(healthPercent * 100.0f);
        Rect statusRegion( 85, (screenSize.height - 156) + int((1.0f - healthPercent) * 132.0f), 85 + 66, (screenSize.height - 156) + 132 );
        if ( statusRegion.height > 0 )
        {
            driver.pushScissorRect( statusRegion );
            interfaceManager.drawImage( Point( 85, screenSize.height - 156 ), "HUDElements", "health_bar" );
            driver.popScissorRect();
 
        } // End if health remains
        statusRegion = Rect( 95, screenSize.height - 165, 140, screenSize.height - 10);
        interfaceManager.printText( statusRegion, String(health), TextFlags::VAlignCenter, 0xCCFFFFFF, 2 );
        
        // Draw current armor.
        float maximumArmor = mPlayer.getMaximumArmor();
        float armorPercent = (maximumArmor > 0 ) ? (mPlayer.getCurrentArmor() / maximumArmor) : 0;
        int armor = int(armorPercent * 100.0f);
        statusRegion = Rect( 19, (screenSize.height - 156) + int((1.0f - armorPercent) * 132.0f), 19 + 66, (screenSize.height - 156) + 132 );
        if ( statusRegion.height > 0 )
        {
            driver.pushScissorRect( statusRegion );
            interfaceManager.drawImage( Point( 19, screenSize.height - 156 ), "HUDElements", "armor_bar" );
            driver.popScissorRect();
        } // End if armor remains
        statusRegion = Rect( 30, screenSize.height - 165, 75, screenSize.height - 10);
        interfaceManager.printText( statusRegion, String(armor), TextFlags::AlignRight | TextFlags::VAlignCenter, 0xCCFFFFFF, 2 );

        // Draw weapon status
        Weapon @ weapon = mPlayer.getCurrentWeapon();
        if ( @weapon != null )
        {
            // Get ammo amounts.
            int roundsInMag = weapon.getCurrentMagazineRounds();
            int totalRounds = weapon.getAvailableRounds();

            // Output ammo in current magazine
            Rect ammoRegion = Rect( 179, screenSize.height - 62, 219, screenSize.height - 31);
            interfaceManager.printText( ammoRegion, String(roundsInMag), TextFlags::AlignCenter, 0xCCFFFFFF, 2 ); // Center top

            // Output total rounds (less current magazine)
            interfaceManager.printText( ammoRegion, String(totalRounds - roundsInMag), TextFlags::AlignCenter | TextFlags::VAlignBottom, 0xCCFFFFFF, 2 );

        } // End if has weapon

        // Render the cross hair
        Size crossHairSize = interfaceManager.getImageSize( "HUDElements", "cross_hair" );
        interfaceManager.drawImage( Point( (screenSize.width - crossHairSize.width) / 2, (screenSize.height - crossHairSize.height) / 2), "HUDElements", "cross_hair", 0xAAFFFFFF );

        // Restore default font
        interfaceManager.selectDefaultFont( );

        // Display screen messages (subtitles, etc.)
        drawScreenMessages();

        // Fade to black in end sequence.
        if ( mSequenceIdentifier == "end_sequence" )
        {
            float alpha = min( 1.0f, mSequenceTime / 5.0f);
            driver.drawRectangle( Rect(0,0,screenSize.width, screenSize.height), ColorValue(0, 0, 0, alpha), true );

            if ( mSequenceTime > 5 )
            {
                ColorValue questionMarkColor( 1, 1, 1, max( 0.0f, min( 1.0f, (mSequenceTime - 8) / 3.0f)));
                String colorCode = questionMarkColor.toString( "x" );

                // Select rendering font
                interfaceManager.selectFont( "Nasalization" );
                interfaceManager.printText( Rect( 0, -250, screenSize.width, screenSize.height), "The End [c=#" + colorCode + "]?[/c]", TextFlags::AlignCenter | TextFlags::VAlignCenter | TextFlags::AllowFormatCode, 0xFFFFFFFF );
                interfaceManager.selectDefaultFont( );
            }

            if ( mSequenceTime > 12 )
            {
                ColorValue tagLineColor( 1, 1, 1, min( 1.0f, (mSequenceTime - 12) / 1.0f));
                ColorValue emphasisColor( 1, 0, 0, tagLineColor.a );
                String colorCode = emphasisColor.toString( "x" );

                // Select rendering font
                interfaceManager.selectFont( "HUDFont_012" );
                interfaceManager.printText( Rect( 0, -150, screenSize.width, screenSize.height), "DEVELOP YOUR [c=#" + colorCode + "]OWN[/c] GAMES AT", TextFlags::AlignCenter | TextFlags::VAlignCenter | TextFlags::AllowFormatCode, tagLineColor, 2 );
                interfaceManager.selectDefaultFont( );
            }

            if ( mSequenceTime > 15 )
            {
                ColorValue tagLineColor( 1, 1, 1, min( 1.0f, (mSequenceTime - 15) / 1.0f));
                ColorValue logoColor = tagLineColor;
                logoColor.a *= 1.0f;

                // Fade in the logo at the same time.
                Size logoSize = interfaceManager.getImageSize( "HUDElements", "gi_logo" );
                interfaceManager.drawImage( Point( ((screenSize.width - logoSize.width) / 2), (screenSize.height / 2) - 35 ), "HUDElements", "gi_logo", logoColor );
                
                // Select rendering font
                interfaceManager.selectFont( "ScreenFont_020" );
                interfaceManager.printText( Rect( 0, 200, screenSize.width, screenSize.height), "WWW.GAMEINSTITUTE.COM", TextFlags::AlignCenter | TextFlags::VAlignCenter | TextFlags::AllowFormatCode, tagLineColor, 2 );
                interfaceManager.selectDefaultFont( );
            }
		
        } // End if end_sequence
    }

    //-------------------------------------------------------------------------
    // Name : drawScreenMessages () (Private)
    // Desc : Display screen messages (subtitles, etc.)
    //-------------------------------------------------------------------------
    private void drawScreenMessages( )
    {
        UIManager @ interfaceManager = getAppUIManager();
        Size screenSize = mSceneView.getSize();

        // Setup font
        interfaceManager.selectFont( "ScreenFont_020" );

        // Output screen messages from most recent to least.
        Rect rcTextArea = Rect( 250, 10, max( 630, screenSize.width - 250), screenSize.height - 50 );
        for ( int i = mScreenMessages.length() - 1; i >= 0; --i )
        {
            ScreenMessage @ msg = mScreenMessages[i];
            
            // Messages fade in over time.
            float fadeIn = min( 1.0f, msg.displayTime / ScreenMessageFadeTime );

            // Then fade out after n seconds.
            float fadeOut = 1.0f - min( 1.0f, max(0.0f, (msg.displayTime - mScreenMessageDisplayTime)) / ScreenMessageFadeTime );

            // Select final color
            float fadeFinal = fadeIn * fadeOut;
            ColorValue c( 1, 1, 1, fadeFinal );

            // Output.
            Rect rcLine = interfaceManager.printText( rcTextArea, msg.message, TextFlags::Multiline | TextFlags::AllowFormatCode | TextFlags::AlignCenter | TextFlags::VAlignBottom, c );

            // Slide up / down.
            rcTextArea.bottom -= int(float(rcLine.height + 5) * fadeFinal);
        
        } // Next message

        // Restore default font
        interfaceManager.selectDefaultFont( );
    }

    //-------------------------------------------------------------------------
    // Name : updateScreenMessages() (Private)
    // Desc : Process the screen message list in order to cycle old messages
    //        out.
    //-------------------------------------------------------------------------
    private void updateScreenMessages( float elapsedTime )
    {
        int finalLength = mScreenMessages.length();
        for ( int i = 0; i < finalLength; ++i )
        {
            mScreenMessages[i].displayTime += elapsedTime;
            if ( mScreenMessages[i].displayTime > (mScreenMessageDisplayTime + ScreenMessageFadeTime) )
            {
                // Remove this message.
                for ( int j = i + 1; j < finalLength; ++j )
                    @mScreenMessages[j-1] = mScreenMessages[j];

                // Test the new element in this slot.
                finalLength--;
                i--;
            
            } // End if expired

        } // Next message

        // Resize the array if it's different.
        if ( mScreenMessages.length() != finalLength )
            mScreenMessages.resize( finalLength );
    }

    //-------------------------------------------------------------------------
    // Name : drawObjectiveOverlay () (Private)
    // Desc : If the player has a current objective, draw the HUD overlay.
    //-------------------------------------------------------------------------
    private void drawObjectiveOverlay( )
    {
        Objective @ currentObjective = mPlayer.getCurrentObjective();
        if ( @mScene == null || @currentObjective == null )
            return;

        // Get the position of the objective in the world
        Vector3 worldPos = currentObjective.getSceneNode().getPosition();

        // Convert into a screen space position.
        Vector3 screenPos;
        Size screenSize = mSceneView.getSize();
        CameraNode @ camera = mScene.getActiveCamera();
        if ( camera.worldToViewport( screenSize, worldPos, screenPos, true, true, true ) )
        {
            // Draw the objective indicator overlay.
            UIManager @ interfaceManager = getAppUIManager();
            Point position = Point( int(screenPos.x) - 21, int(screenPos.y) - 21 );
            interfaceManager.drawImage( position, "HUDElements", "objective_indicator" );

            // Generate display string.
            String output = "", font = "";
            if ( currentObjective.isObjectiveInRange( mPlayerNode.getPosition() ) )
            {
                output = "Press 'E' to activate.";
                font = "Nasalization";
            
            } // End if in range
            else
            {
                output = currentObjective.distanceToObjective( mPlayerNode.getPosition() ) + "m";
                font = "fixed_v01_white";
            
            } // End if out of range

            // Output display string.
            Rect displayRect( int(screenPos.x) - screenSize.width, position.y - 12 , int(screenPos.x) + screenSize.width, position.y + 10 );
            interfaceManager.selectFont( font );
            interfaceManager.printText( displayRect, output, TextFlags::AlignCenter, 0x88FFFFFF );

        } // End if in the viewport

    }

    //-------------------------------------------------------------------------
    // Name : loadScene () (Private)
    // Desc : Contains code responsible for loading the main scene.
    //-------------------------------------------------------------------------
    private bool loadScene( )
    {
        // Load the first scene from the file.
        @mScene = mWorld.loadScene( 0x1 );
        if ( @mScene == null )
            return false;

        // Create a dummy object to represent the player
        @mPlayerNode = mScene.createObjectNode( true, RTID_DummyObject, false );

        // We want the objects update process to execute every frame so that
        // its behavior script (assigned in a moment) will get a chance to execute.
        mPlayerNode.setUpdateRate( UpdateRate::Always );

        // Position the player in the world.
        ObjectNode @ playerSpawnPoint = mScene.getObjectNodeById( 0xC13 );
        mPlayerNode.setWorldTransform( playerSpawnPoint.getWorldTransform() );

        // Load the main player agent script
        ObjectBehavior @ behavior = ObjectBehavior( );
        behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/PlayerAgent.gs", "" );

        // Cache reference to main player script object.
        @mPlayer = cast<PlayerAgent>(behavior.getScriptObject());
        if ( @mPlayer == null )
            return false;

        // Create the agent manager and assign the player to it (initializes values in player script).
        @mAgentManager = AgentManager( mAudioManager, mScene, mPlayerNode, mPlayer, mShowDebug );
        
        // Assign custom behavior script to the player object. Immediately following 
        // the call to 'addBehavior()', the script's 'onAttach()' method will be
        // automatically triggered, where additional player setup occurs.
        mPlayerNode.addBehavior( behavior );

        // Assign the first objective to the player (elevator).
        //ObjectNode @ objectiveNode = mScene.getObjectNodeById( 0xC15 ); // Elevator
        ObjectNode @ objectiveNode = mScene.getObjectNodeById( 0xDEA ); // Armor recharge
        if ( @objectiveNode != null )
        {
            Objective @ objective = cast<Objective>(objectiveNode.getScriptedBehavior(0));
            mPlayer.setCurrentObjective( objective );
        
        } // End if found objective

        // Success!
        return true;
    }

    void preloadResources()
    {
        ResourceManager @ resources = getAppResourceManager();

        // Pre-load objects.
        mObjectCache.resize(11);
        @mObjectCache[0]  = mWorld.loadObject( RTID_ActorObject, 0x30A, CloneMethod::ObjectInstance ); // Elite Trooper Actor
        mLoadingState.render();
        @mObjectCache[1]  = mWorld.loadObject( RTID_ActorObject, 0x5AD, CloneMethod::ObjectInstance ); // FPS Arms Actor
        mLoadingState.render();
        @mObjectCache[2]  = mWorld.loadObject( RTID_ActorObject, 0x692, CloneMethod::ObjectInstance ); // Orc Actor
        mLoadingState.render();
        @mObjectCache[3]  = mWorld.loadObject( RTID_ActorObject, 0x80C, CloneMethod::ObjectInstance ); // Hound Actor
        mLoadingState.render();
        @mObjectCache[4]  = mWorld.loadObject( RTID_ActorObject, 0xD8D, CloneMethod::ObjectInstance ); // Mech Actor
        mLoadingState.render();
        @mObjectCache[5]  = mWorld.loadObject( RTID_SkinObject,  771, CloneMethod::ObjectInstance ); // Elite Trooper Skin
        mLoadingState.render();
        @mObjectCache[6]  = mWorld.loadObject( RTID_SkinObject, 1448, CloneMethod::ObjectInstance ); // FPS Arms Skin
        mLoadingState.render();
        @mObjectCache[7]  = mWorld.loadObject( RTID_SkinObject, 1679, CloneMethod::ObjectInstance ); // Orc Skin
        mLoadingState.render();
        @mObjectCache[8]  = mWorld.loadObject( RTID_SkinObject, 2053, CloneMethod::ObjectInstance ); // Hound Skin
        mLoadingState.render();
        @mObjectCache[9]  = mWorld.loadObject( RTID_SkinObject, 3380, CloneMethod::ObjectInstance ); // Mech Skin
        mLoadingState.render();
        @mObjectCache[10] = mWorld.loadObject( RTID_MeshObject, 1623, CloneMethod::ObjectInstance ); // Orc Shotgun Mesh
        mLoadingState.render();
        for ( int i = 0; i < mObjectCache.length(); ++i )
        {
            if ( @mObjectCache[i] != null )
                mObjectCache[i].addReference( null );
        } // Next object
        
        // Pre-load meshes.
        mMeshCache.resize(6);
        resources.loadMesh( mMeshCache[0], mWorld,  776, false, true, 0, DebugSource() ); // Elite Trooper
        mLoadingState.render();
        resources.loadMesh( mMeshCache[1], mWorld, 1451, false, true, 0, DebugSource() ); // FPS Arms
        mLoadingState.render();
        resources.loadMesh( mMeshCache[2], mWorld, 1680, false, true, 0, DebugSource() ); // Orc
        mLoadingState.render();
        resources.loadMesh( mMeshCache[3], mWorld, 2058, false, true, 0, DebugSource() ); // Hound
        mLoadingState.render();
        resources.loadMesh( mMeshCache[4], mWorld, 3381, false, true, 0, DebugSource() ); // Mech Skin
        mLoadingState.render();
        resources.loadMesh( mMeshCache[5], mWorld, 1625, false, true, 0, DebugSource() ); // Orc Shotgun Mesh
        mLoadingState.render();

        // Pre-load scripts.
        mScriptCache.resize(7);
        resources.loadScript( mScriptCache[0], "Scripts/ColonyVI/Behaviors/Enemy_Explosive_Hound.gs", 0, DebugSource() );
        mLoadingState.render();
        resources.loadScript( mScriptCache[1], "Scripts/ColonyVI/Behaviors/Enemy_Orc.gs", 0, DebugSource() );
        mLoadingState.render();
        resources.loadScript( mScriptCache[2], "Scripts/ColonyVI/Behaviors/Enemy_Mech.gs", 0, DebugSource() );
        mLoadingState.render();
        resources.loadScript( mScriptCache[3], "Scripts/ColonyVI/Behaviors/Ally_Trooper.gs", 0, DebugSource() );
        mLoadingState.render();
        resources.loadScript( mScriptCache[4], "Scripts\\ColonyVI\\Behaviors\\Weapon_Mech_Miniguns.gs", 0, DebugSource() );
        mLoadingState.render();
        resources.loadScript( mScriptCache[5], "Scripts\\ColonyVI\\Behaviors\\Weapon_Orc_Shotgun.gs", 0, DebugSource() );
        mLoadingState.render();
        resources.loadScript( mScriptCache[6], "Scripts\\ColonyVI\\Behaviors\\Casing.gs", 0, DebugSource() );
        mLoadingState.render();

        // Pre-load sounds.
        mSoundCache.resize(25);
        resources.loadAudioBuffer( mSoundCache[0], "Sounds/Growl 1.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[1], "Sounds/Growl 2.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[2], "Sounds/Growl 3.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[3], "Sounds/Growl 4.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        mLoadingState.render();
        resources.loadAudioBuffer( mSoundCache[4], "Sounds/Growl 5.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[5], "Sounds/Growl Aggressive 0.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[6], "Sounds/Growl Aggressive 1.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[7], "Sounds/Growl Aggressive 2.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        mLoadingState.render();
        resources.loadAudioBuffer( mSoundCache[8], "Sounds/Orc Death 0.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[9], "Sounds/0042_impact_splat.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[10], "Sounds/Growl 7.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        mLoadingState.render();
        resources.loadAudioBuffer( mSoundCache[11], "Sounds/Growl 8.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[12], "Sounds/Growl 9.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[13], "Sounds/Wildlife 1.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        mLoadingState.render();
        resources.loadAudioBuffer( mSoundCache[14], "Sounds/Wildlife 2.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[15], "Sounds/Wildlife 3.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[16], "Sounds/Monster Demon Screech.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        mLoadingState.render();
		resources.loadAudioBuffer( mSoundCache[17], "Sounds/Bug Movement.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[18], "Sounds/Grenade Explosion.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[19], "Sounds/Elevator Ride.ogg", AudioBufferFlags::Complex, 0, DebugSource() );
        mLoadingState.render();
        resources.loadAudioBuffer( mSoundCache[20], "Sounds/Elevator Doors.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[22], "Sounds/Elevator Panel Fail.ogg", AudioBufferFlags::Complex, 0, DebugSource() );
        mLoadingState.render();
        resources.loadAudioBuffer( mSoundCache[23], "Sounds/0022_repair.ogg", AudioBufferFlags::Complex, 0, DebugSource() );
        resources.loadAudioBuffer( mSoundCache[24], "Sounds/Shotgun Fire.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        mLoadingState.render();
    }

    void clearResourceCache()
    {
        for ( int i = 0; i < mObjectCache.length(); ++i )
        {
            if ( @mObjectCache[i] != null )
                mObjectCache[i].removeReference( null );
        } // Next object

        mMeshCache.resize(0);
        mScriptCache.resize(0);
        mSoundCache.resize(0);
        mAnimationCache.resize(0);
        mObjectCache.resize(0);
    }

};