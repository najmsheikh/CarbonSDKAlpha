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
#include_once "../Behaviors/NPCAgent.gsh"
#include_once "../Behaviors/PlayerAgent.gs"

#include_once "../Behaviors/DestructibleLight.gs"
#include_once "../Behaviors/EmergencyLight.gs"

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

shared enum GameAgentType
{
    Hound,
    Orc,
    Trooper
};

const float ScreenMessageDisplayTime = 5.0f;
const float ScreenMessageFadeTime = 0.5f;

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
    private bool                    mShowDebug;             // Show debug statistics.
    private String                  mSequenceIdentifier;    // Current gameplay sequence.
    private double                  mSequenceTime;          // Amount of time spent in the current sequence so far.
    private int                     mSequenceStep;          // Internal tracking variable for tasks processed in this sequence.
    private double                  mSequenceStepTime;      // Amount of time spent in the current step of this sequence so far.

    private World@                  mWorld;                 // Main game world object.
    private Scene@                  mScene;                 // The scene we've loaded.
    private RenderView@             mSceneView;             // Render view into which the scene will be rendered.

    private ObjectNode@             mPlayerNode;            // Object node used to represent dummy player object.
    private PlayerAgent@            mPlayer;                // Cached reference to the main player behavior.

    // Navigation
    private array<ObjectNode@>      mAgents;            // List of navigation agents that we spawned and would like to control.

    // Screen Elements
    private array<ScreenMessage@>   mScreenMessages;

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
        mShowDebug          = false;
        mSequenceIdentifier = "";

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
        // Renable resource destruction (this only applies if the level is restarting and
        // destruction was disabled before the scene was unloaded).
        getAppResourceManager().enableDestruction( true );

        // Long range sounds please!
        getAppAudioDriver().set3DRolloffFactor( 0.1f );

        // Retrieve the application's main world object instance.
        @mWorld = getAppWorld();

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
        getAppAudioDriver().loadAmbientTrack( "Music", "Music/Mechanolith.ogg", 0.3f, 0.3f );

        // Switch to the first sequence.
        setCurrentGameSequence( "spawn" );

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
        // Unload resources.
        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.removeImageLibrary( "HUDElements" );

        // Stop music.
        getAppAudioDriver().stopAmbientTrack( "Music" );

        // Dispose of scene rendering view
        if ( @mSceneView != null )
            mSceneView.deleteReference();
        
        // Reload our references to scene objects.
        @mPlayer        = null;
        @mPlayerNode    = null;

        // Unload the scene
        if ( @mScene != null )
            mScene.unload();

        // Clean up
        @mSceneView     = null;
        @mScene         = null;
        @mWorld         = null;
        mAgents.resize(0);
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
        if ( @mScene != null && !mState.isSuspended() )
        {
            // Allow the scene to perform any necessary update tasks.
            mScene.update();

            // Handle screen message lifetime
            float elapsedTime = getAppTimer().getTimeElapsed();
            updateScreenMessages( elapsedTime );

            // Handle game sequence.
            updateCurrentGameSequence( elapsedTime );

            // Is the player dead?
            if ( @mPlayer != null && !mPlayer.isAlive() )
            {
                // Wait 7 seconds then restart.
                if ( mPlayer.getTimeSinceDamage() > 7 )
                {
                    // Delay the unloading of all resources.
                    getAppResourceManager().enableDestruction( false );

                    // Restart the level.
                    mState.raiseEvent( "Restart" );
                
                } // End if 7 seconds
            
            } // End if dead
        
        } // End if !suspended

        // Exit event?
        InputDriver @ inputDriver = getAppInputDriver();
        if ( inputDriver.isKeyPressed( Keys::Escape ) )
            mState.raiseEvent( "Exit" );
        else if ( inputDriver.isKeyPressed( Keys::F9, true ) )
        {
            // Delay the unloading of all resources.
            getAppResourceManager().enableDestruction( false );

            // Restart the level.
            mState.raiseEvent( "Restart" );
        
        } // End if F9
    }

    //-------------------------------------------------------------------------
    // Name : render ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to render whatever is necessary.
    //-------------------------------------------------------------------------
    void render( )
    {
        if ( @mSceneView == null || @mScene == null )
            return;

        // Start rendering to our created scene view (full screen output in this case).
        if ( mSceneView.begin() )
        {
            // Allow the scene to render
            mScene.render();

            // Render the HUD elements.
            drawHUD();

            // Draw some debug statistics.
            if ( mShowDebug )
            {
                // Find the closest agent
                NPCAgent @ closestAgent = null;
                float closestDistance = 9999999999.0f;
                for ( uint i = 0; i < mAgents.length(); ++i )
                {
                    // Agent has been unloaded?
                    if ( mAgents[i].isDisposed() || mAgents[i].getBehaviorCount() == 0 )
                        continue;

                    NPCAgent @ agent = cast<NPCAgent>(mAgents[i].getScriptedBehavior(0));
                    if ( @agent != null && agent.isAlive() )
                    {
                        float distance = vec3Length(mAgents[i].getPosition() - mPlayerNode.getPosition());
                        if ( distance < closestDistance )
                        {
                            @closestAgent = agent;
                            closestDistance = distance;
                        
                        } // End if closer
                    
                    } // End if alive

                } // Next agent

                // Output state description
                if ( @closestAgent != null )
                {
                    UIManager @ interfaceManager = getAppUIManager();
                    NPCAgentDebug @ d = closestAgent.debugStats;
                    String output = "[c=#88AAAAFF]Closest Agent State[/c]\n";
                    Vector3 pos = closestAgent.getSceneNode().getPosition();
                    output += "Position: " + pos.x + ", " + pos.y + ", " + pos.z + "\n";
                    output += "Current state: " + d.stateName + "\n";
                    output += "Angle to Target H: " + d.angleH + "\n";
                    output += "Angle to Target V: " + d.angleV + "\n";
                    output += "Range to Target: " + d.range + "\n";
                    output += "Target in cones: " + ((d.inDetectionCones) ? "true" : "false") + "\n";
                    output += "Target in LoS: " + ((d.inLoS) ? "true" : "false") + "\n";
                    output += "Willing to Fire: " + ((d.willingToFire) ? "true" : "false") + "\n";
                    interfaceManager.selectDefaultFont( );
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
            // Spawn in all agents (initially deactivated).
            // Hallway after spawn_elevator_travel
            spawnAgent( GameAgentType::Orc, 0x3D6 );
            spawnAgent( GameAgentType::Hound, 0x82F );
            spawnAgent( GameAgentType::Orc, 0x3D2 );
            spawnAgent( GameAgentType::Orc, 0x3D4 );
            spawnAgent( GameAgentType::Orc, 0x3DA );
            
            // Back side hallways
            spawnAgent( GameAgentType::Hound, 0x3DC );
            spawnAgent( GameAgentType::Hound, 0x3DE );
            spawnAgent( GameAgentType::Orc, 0x3E0 );
            spawnAgent( GameAgentType::Orc, 0x3E2 );
            spawnAgent( GameAgentType::Orc, 0x3E4 );
            
            // Generator room
            spawnAgent( GameAgentType::Orc, 0x3EC );
            spawnAgent( GameAgentType::Orc, 0x3F0 );
            spawnAgent( GameAgentType::Hound, 0x3F6 );
            

            // In the warehouse after escape_elevator_broken
            spawnAgent( GameAgentType::Orc, 0xC24 );
            spawnAgent( GameAgentType::Hound, 0xC26 );
            spawnAgent( GameAgentType::Orc, 0xC28 );
            spawnAgent( GameAgentType::Orc, 0xC2A );
            spawnAgent( GameAgentType::Orc, 0xC2C );
            spawnAgent( GameAgentType::Orc, 0xC2E );
            spawnAgent( GameAgentType::Orc, 0x3FA );

            // Three friendly troopers in main spawn area.
            spawnAgent( GameAgentType::Trooper, 0x89D );
            spawnAgent( GameAgentType::Trooper, 0x89B );
            spawnAgent( GameAgentType::Trooper, 0x89F );
            activateAgentAtPoint( 0x89D );
            activateAgentAtPoint( 0x89B );
            activateAgentAtPoint( 0x89F );

        } // End if spawn
        else if ( sequenceIdentifier == "spawn_elevator_travel" )
        {
            activateAgentAtPoint( 0x3D6 );
            activateAgentAtPoint( 0x82F );
            activateAgentAtPoint( 0x3D2 );
            activateAgentAtPoint( 0x3D4 );
            activateAgentAtPoint( 0x3DA );

            activateAgentAtPoint( 0x3DC );
            activateAgentAtPoint( 0x3DE );
            activateAgentAtPoint( 0x3E0 );
            activateAgentAtPoint( 0x3E2 );
            activateAgentAtPoint( 0x3E4 );
            
            activateAgentAtPoint( 0x3EC );
            activateAgentAtPoint( 0x3F0 );
            activateAgentAtPoint( 0x3F6 );

        } // End if spawn_elevator_travel
        else if ( sequenceIdentifier == "trigger_generator_4" )
        {
            // If this is the final generator objective, deactivate hall  lights and 
            // activate the emergency lights. First, turn hallway lights off.
            array<ObjectNode@> nodes = mScene.getObjectNodesByType( RTID_MeshObject );
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
            nodes = mScene.getObjectNodesByType( RTID_SpotLightObject );
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
            getAppAudioDriver().loadAmbientTrack( "Music", "Music/Mistake the Getaway.ogg", 0, 0.3f );

        } // End if "trigger_generator_4
        else if ( sequenceIdentifier == "escape_elevator_broken" )
        {
            // Activate the enemies spawned in the warehouse
            activateAgentAtPoint( 0xC24 );
            activateAgentAtPoint( 0xC26 );
            activateAgentAtPoint( 0xC28 );
            activateAgentAtPoint( 0xC2A );
            activateAgentAtPoint( 0xC2C );
            activateAgentAtPoint( 0xC2E );

            // And the one just outside for good measure.
            activateAgentAtPoint( 0x3FA );

        } // End if escape_elevator_broken

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

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
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
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "The colony is completely overrun, and all security personnel are missing or K.I.A");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;

                case 1:
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "We need to wipe these... creatures... out, but we can't risk a full frontal assault.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;

                case 2:
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Your orders are to infiltrate the facilty, deactivate the primary generator coolant supply, and get out.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;

                case 3:
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Should be a walk in the park...");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;

                case 4:
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "You can enter the facility through the elevator on the far side of the hall. I've highlighted it for you on your H.U.D.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;

                case 5:
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Good luck. H.Q. out.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;

            } // End switch

        } // End if 'spawn'
        else if ( mSequenceIdentifier == "spawn_elevator_travel" )
        {
            switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > 6.0f )
                    {
                        addScreenMessage( "Good, you're in. The corridor to your left should lead to the primary generator room.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;
            
            } // End switch
        
        } // End if 'spawn_elevator_travel'
        else if ( mSequenceIdentifier == "trigger_generator_1" )
        {
            switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > 1.0f )
                    {
                        addScreenMessage( "That's the first one down! Shut down the three remaining coolant pumps and this thing will go critical.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
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
                        addScreenMessage( "Nice work, that's all of them.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;
                
                case 1:
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Make your way back to the hangar. E.T.A on evac' is approximately 4 minutes.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
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
                        addScreenMessage( "Damn it! A surge from the generator must have fried the circuits.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;
                
                case 1:
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "There's a supply warehouse down the corridor back underneath the generator room.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;

                case 2:
                    if ( mSequenceStepTime > ScreenMessageDisplayTime )
                    {
                        addScreenMessage( "Head back and see if you can find a spare panel.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
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
                        addScreenMessage( "OK, that looks like it. Get back to the elevator as quick as you can, you don't have much time!");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;
            
            } // End switch
        
        } // End if 'collected_elevator_part'
        else if ( mSequenceIdentifier == "escape_elevator_entry" )
        {
            switch (mSequenceStep)
            {
                case 0:
                    if ( mSequenceStepTime > 1.0f )
                    {
                        addScreenMessage( "Good it worked. Time to leave. Evac should be arriving any second.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
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
                        addScreenMessage( "That's... not good.");
                        mSequenceStep++;
                        mSequenceStepTime = 0;
                    
                    } // End if elapsed
                    break;
            
            } // End switch
        
        } // End if 'escape_elevator_travel'
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

        // Select rendering font
        interfaceManager.selectFont( "HUDFont_012" );

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

        // Next draw the statistics and status background.
        interfaceManager.drawImage( Point( 10, screenSize.height - 165 ), "HUDElements", "stats_background" );

        // Draw current health.
        float healthPercent = (mPlayer.getCurrentHealth() / mPlayer.getMaximumHealth());
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
        float armorPercent = (mPlayer.getCurrentArmor() / mPlayer.getMaximumArmor());
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

        // Restore default font
        interfaceManager.selectDefaultFont( );

        // Display screen messages (subtitles, etc.)
        drawScreenMessages();
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
            float fadeOut = 1.0f - min( 1.0f, max(0.0f, (msg.displayTime - ScreenMessageDisplayTime)) / ScreenMessageFadeTime );

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
            if ( mScreenMessages[i].displayTime > (ScreenMessageDisplayTime + ScreenMessageFadeTime) )
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
        if ( @currentObjective == null )
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

        // Assign custom behavior script to the player object. Immediately following 
        // the call to 'addBehavior()', the script's 'onAttach()' method will be
        // automatically triggered, where additional player setup occurs.
        ObjectBehavior @ behavior = ObjectBehavior( );
        behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/PlayerAgent.gs", "" );
        mPlayerNode.addBehavior( behavior );

        // Cache reference to main player script object.
        @mPlayer = cast<PlayerAgent>(mPlayerNode.getScriptedBehavior(0));

        // Assign the first objective to the player (generator room).
        //ObjectNode @ objectiveNode = mScene.getObjectNodeById( 0x6E9 );
        ObjectNode @ objectiveNode = mScene.getObjectNodeById( 0xC15 ); // Elevator
        if ( @objectiveNode != null )
        {
            Objective @ objective = cast<Objective>(objectiveNode.getScriptedBehavior(0));
            mPlayer.setCurrentObjective( objective );
        
        } // End if found objective

        // Spawn in the NPC agents.
        //if ( !spawnTestAgents( ) )
            //return false;

        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : spawnAgent ()
    // Desc : Spawn in a single agent at the requested spawn point.
    //-------------------------------------------------------------------------
    bool spawnAgent( GameAgentType type, uint spawnPointRefId )
    {
        //return false;
        uint agentRefId = 0;
        switch ( type )
        {
            case GameAgentType::Hound:
                agentRefId = 0x80B;
                break;

            case GameAgentType::Orc:
                agentRefId = 0x691;
                break;

            case GameAgentType::Trooper:
                agentRefId = 0x309;
                break;
        
        } // End switch

        // Retrieve the specified spawn point.
        ObjectNode @ spawnPoint = mScene.getObjectNodeById( spawnPointRefId );
        if ( @spawnPoint == null )
            return false;

        // Generate starting position
        Vector3 pos = spawnPoint.getPosition();
        
        // Spawn in an NPC actor at the selected position.
        ObjectNode @ agentNode = mScene.loadObjectNode( agentRefId, CloneMethod::ObjectInstance, true );
        agentNode.setPosition( pos );
        
        // Random orientation
        agentNode.rotateLocal( 0, randomFloat( 0, 360.0f ), 0 );

        // Set the agent to be hidden initially and make sure it never updates.
        agentNode.showNode( false, true );
        agentNode.setUpdateRate( UpdateRate::Never );

        // Mark the waypoint as assigned.
        PropertyContainer @ properties = spawnPoint.getCustomProperties();
        properties.setProperty( "wp_assigned_to", Variant(agentNode.getReferenceId()) );

        // Store so we can update them later
        mAgents.resize( mAgents.length() + 1 );
        @mAgents[ mAgents.length() - 1 ] = agentNode;
        
        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : activateAgentAtPoint () (Private)
    // Desc : Enable the agent spawned at the specified spawn point.
    //-------------------------------------------------------------------------
    private bool activateAgentAtPoint( uint spawnPointRefId )
    {
        //return false;
        // Get the specified spawn point.
        ObjectNode @ spawnPoint = mScene.getObjectNodeById( spawnPointRefId );
        if ( @spawnPoint == null )
            return false;

        // Get the agent node at this point.
        PropertyContainer @ properties = spawnPoint.getCustomProperties();
        uint agentRefId = uint(properties.getProperty( "wp_assigned_to", uint(0) ));
        ObjectNode @ agentNode = mScene.getObjectNodeById( agentRefId );
        if ( @agentNode == null )
            return false;

        // Create an object behavior specific to the type of agent.
        GameAgentType type;
        ObjectBehavior @ behavior = ObjectBehavior( );
        switch ( agentNode.getReferencedObject().getReferenceId() )
        {
            case 0x80C:
                type = GameAgentType::Hound;
                behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Enemy_Explosive_Hound.gs", "" );
                break;

            case 0x692:
                type = GameAgentType::Orc;
                behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Enemy_Orc.gs", "" );
                break;

            case 0x30A:
                type = GameAgentType::Trooper;
                behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Ally_Trooper.gs", "" );
                break;
            
            default:
                logWrite( "Unrecognized agent type with object reference id " + agentNode.getReferencedObject().getReferenceId() + "\n" );
                return false;
        
        } // End switch object id

        // Show the agent.
        agentNode.showNode( true, true );

        // Allow it to update
        agentNode.setUpdateRate( UpdateRate::Always );

        // Attach the new behavior.
        agentNode.addBehavior( behavior );

        // Send the agent a reference to the main player. This saves each agent
        // having to find the player itself.
        NPCAgent @ agent = cast<NPCAgent>(agentNode.getScriptedBehavior(0));
        agent.setPlayer( mPlayerNode );
        agent.debugEnabled = mShowDebug;
        agent.requestedOrientation = agentNode.getWorldTransform().orientation();

        // Allow the agent to navigate freely.
        agent.enableNavigation( true );

        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : spawnTestAgents () (Private)
    // Desc : Spawn in a bunch of test agents for.... testing. :)
    //-------------------------------------------------------------------------
    private bool spawnTestAgents( )
    {
        // Hound - 0x80B
        // Orc - 0x691
        // Trooper - 0x309

        // Get a list of all of the navigation patrol points on the map.
        array<ObjectNode@> patrolPoints = mScene.getObjectNodesByType( RTID_NavigationPatrolPointObject );
        
        // Spawn a number of NPC agents on one of the patrol points.
        int agentCount = min( patrolPoints.length(), 5 );
        for ( int i = 0; i < agentCount; ++i )
        {
            ObjectNode @ waypoint = null;

            // Pick a patrol point furthest away from the player.
            float furthestDistance = -1;
            for ( int j = 0; j < patrolPoints.length(); ++j )
            {
                ObjectNode @ test = patrolPoints[j];
                PropertyContainer @ properties = test.getCustomProperties();
                if ( properties.isPropertyDefined( "wp_assigned_to" ) )
                    continue;

                // Furthest so far?
                float distance = vec3Length( test.getPosition() - mPlayerNode.getPosition() );
                if ( distance > furthestDistance )
                {
                    @waypoint = test;
                    furthestDistance = distance;
                
                } // End if further
            
            } // Next waypoint

            // Generate starting position
            Vector3 pos = waypoint.getPosition();
            
            // Spawn in an NPC actor at the selected position.
            //uint typeId = (randomInt(0,1) != 0) ? 0x691 : 0x80B;
            //uint typeId = 0x691;
            uint typeId = 0x309;
            ObjectNode @ agentNode = mScene.loadObjectNode( typeId, CloneMethod::ObjectInstance, true );
            agentNode.setPosition( pos );
            
            // Random orientation
            agentNode.rotateLocal( 0, randomFloat( 0, 360.0f ), 0 );

            // Send the agent a reference to the main player. This saves each agent
            // having to find the player itself.
            NPCAgent @ agent = cast<NPCAgent>(agentNode.getScriptedBehavior(0));
            agent.setPlayer( mPlayerNode );
            agent.debugEnabled = mShowDebug;
            agent.requestedOrientation = agentNode.getWorldTransform().orientation();

            // Allow the agent to navigate freely.
            agent.enableNavigation( true );
            
            // Mark the waypoint as assigned.
            PropertyContainer @ properties = waypoint.getCustomProperties();
            properties.setProperty( "wp_assigned_to", Variant(agentNode.getReferenceId()) );
            
            // Store so we can update them later
            mAgents.resize( mAgents.length() + 1 );
            @mAgents[ mAgents.length() - 1 ] = agentNode;
        
        } // Next character

        // Remove temporary assigned state from waypoints.
        for ( int i = 0; i < patrolPoints.length(); ++i )
        {
            PropertyContainer @ properties = patrolPoints[i].getCustomProperties();
            properties.removeProperty( "wp_assigned_to" );

        } // Next waypoint

        // Success!
        return true;
    }
};