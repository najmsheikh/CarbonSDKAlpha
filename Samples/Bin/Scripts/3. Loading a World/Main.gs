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
// Name : Main.gs (3. Loading a World)                                       //
//                                                                           //
// Desc : Script containing the majority of the logic for demonstrating how  //
//        to load a master world / project file from which all game data     //
//        will be loaded.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Main (Class)
// Desc : Scripted application state object used as the main entry point for
//        a given framework demonstration.
//-----------------------------------------------------------------------------
class Main : IScriptedAppState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    AppState @ mState;  // Application side state object
    World    @ mWorld;  // Main game world object.

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
        // Retrieve the application's main world object instance.
        @mWorld = getAppWorld();

        // Establish a connection to the master world databse we need.
        if (!mWorld.open( "Colony VI.cwm" ))
        {
            @mWorld = null;
            return false;
        
        } // End if failed

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
        // Close the world file we opened.
        if ( @mWorld != null )
            mWorld.close();
        @mWorld = null;
    }

    //-------------------------------------------------------------------------
    // Name : update ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to perform any processing in its entirety
    //        prior to the rendering process.
    //-------------------------------------------------------------------------
    void update( )
    {
        // Exit event?
        InputDriver @ inputDriver = getAppInputDriver();
        if ( inputDriver.isKeyPressed( Keys::Escape ) )
            mState.raiseEvent( "Exit" );

    }

    //-------------------------------------------------------------------------
    // Name : render ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to render whatever is necessary.
    //-------------------------------------------------------------------------
    void render( )
    {
        String output;
        
        // Build a string to display.
        if ( @mWorld != null )
            output = "\n\n\nHello World!\nThe world has been loaded and it contains " + mWorld.getSceneCount() + " scene(s).";
        else
            output = "\n\n\nHello World!\nThe world file failed to open.";

        // Print it to the screen.
        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.selectFont( "fixed_v01_white" );
        interfaceManager.printText( Point(10,10), output, TextFlags::Multiline, 0xFFFF0000, 0, 10 );
    }
};