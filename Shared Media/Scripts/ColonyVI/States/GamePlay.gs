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

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
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
    private AppState@       mState;             // Application side state object
    private bool            mShowDebug;         // Show debug statistics.
    
    private World@          mWorld;             // Main game world object.
    private Scene@          mScene;             // The scene we've loaded.
    private RenderView@     mSceneView;         // Render view into which the scene will be rendered.

    private ObjectNode@     mPlayerNode;        // Object node used to represent dummy player object.
    private PlayerAgent@    mPlayer;            // Cached reference to the main player behavior.

    // Navigation
    private array<ObjectNode@>  mAgents;        // List of navigation agents that we spawned and would like to control.

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
        @mState = state;
        mShowDebug = true;

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

        // Switch to direct mouse input mode (no cursor)
        InputDriver @ inputDriver = getAppInputDriver();
        inputDriver.setMouseMode( MouseHandlerMode::Direct );

        // Warm the timer.
        for ( int i = 0; i < 20; ++i )
            getAppTimer().tick();

        // Play initial gameplay music.
        getAppAudioDriver().loadAmbientTrack( "Music", "Music/Mechanolith.ogg", 0.3f, 0.3f );

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
                    if ( mAgents[i].isDisposed() )
                        continue;

                    NPCAgent @ agent = cast<NPCAgent>(mAgents[i].getScriptedBehavior(0));
                    if ( agent.isAlive() )
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
        mPlayerNode.setPosition( Vector3(0.0f, 0.01f, -2.5f) );
        //mPlayerNode.setPosition( Vector3(-54.204f, 0.01f, 22.905f) );

        // Assign custom behavior script to the player object. Immediately following 
        // the call to 'addBehavior()', the script's 'onAttach()' method will be
        // automatically triggered, where additional player setup occurs.
        ObjectBehavior @ behavior = ObjectBehavior( );
        behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/PlayerAgent.gs", "" );
        mPlayerNode.addBehavior( behavior );

        // Cache reference to main player script object.
        @mPlayer = cast<PlayerAgent>(mPlayerNode.getScriptedBehavior(0));

        // Assign the first objective to the player (generator room).
        ObjectNode @ objectiveNode = mScene.getObjectNodeById( 0x6E9 );
        if ( @objectiveNode != null )
        {
            Objective @ objective = cast<Objective>(objectiveNode.getScriptedBehavior(0));
            mPlayer.setCurrentObjective( objective );
        
        } // End if found objective

        // Spawn in the NPC agents.
        if ( !spawnAgents( ) )
            return false;

        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : spawnAgents () (Private)
    // Desc : Spawn in a bunch of test agents for.... testing. :)
    //-------------------------------------------------------------------------
    private bool spawnAgents( )
    {
        // Hound - 0x80B
        // Orc - 0x691
        // Trooper - 0x309

        // Get a list of all of the navigation patrol points on the map.
        array<ObjectNode@> patrolPoints = mScene.getObjectNodesByType( RTID_NavigationPatrolPointObject );
        
        // Spawn a number of NPC agents on one of the patrol points.
        int agentCount = min( patrolPoints.length(), 16 );
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
            uint typeId = (randomInt(0,1) != 0) ? 0x691 : 0x80B;
            //uint typeId = 0x691;
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