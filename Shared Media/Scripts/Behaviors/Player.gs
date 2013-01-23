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
// Name : Player.gs                                                          //
//                                                                           //
// Desc : Script responsible for managing the main player controlled         //
//        character and its components, including the handling of input,     //
//        camera control and other necessary behaviors.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------
#include_once "FirstPersonActor.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Player (Class)
// Desc : Behavior used to map input and other events to the main player
//        object.
//-----------------------------------------------------------------------------
shared class Player : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@         mParent;            // The object node to which this behavior is attached.
    private CameraNode@         mCamera;            // The camera associated with the player.
    private FirstPersonActor@   mFirstPersonActor;  // Script object currently attached to the player's first person actor.

    private Vector2             mTotalOffset;       // Total amount of mouse motion recorded subsequent to previous update.
	private Vector2             mPrevRotation;      // Previous rotation angles used for a single frame average.

    // Right click physics grab
    private ObjectNode@                 mSelectedNode;  // Node selected when right clicking to grab objects.
    private KinematicControllerJoint@   mJoint;         // Joint used for the physics constraint.
    private float                       mGrabDistance;  // Distance from the camera to the grabbed object on first click.

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Player () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Player( )
    {
        // Initialize variables to sensible defaults.
        mTotalOffset    = Vector2( 0.0f, 0.0f );
	    mPrevRotation   = Vector2( 0.0f, 0.0f );
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
        // Store a reference to the object we're attached to. We'll need it later.
        @mParent = object;

        // Retrieve a reference to the camera attached as an immediate child of the parent object.
        @mCamera = cast<CameraNode>(mParent.findChildOfType( RTID_CameraObject, false ));
        
        // Get the behavior associated with the first person actor attached as an 
        // immediate child of the camera so that we can interact with it and call
        // its methods where necessary.
        if ( @mCamera != null )
        {
            ObjectNode @ actorNode = mCamera.findChildOfType( RTID_ActorObject, false );
            if ( @actorNode != null )
                @mFirstPersonActor = cast<FirstPersonActor>(actorNode.getScriptedBehavior(0));
        
        } // End if camera found
	}

	//-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from an
	//        object and can shut down.
	//-------------------------------------------------------------------------
	void onDetach( ObjectNode @ object )
	{
        // Relinquish our references.
        @mParent            = null;
        @mCamera            = null;
        @mFirstPersonActor  = null;
	}

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Do nothing if we don't have references to the necessary objects.
        if ( @mParent == null || @mCamera == null )
            return;

        // Process input first of all in order to update the player and camera 
        // orientation, trigger other behaviors, etc.
        processInput( elapsedTime );

        // Recompute position of the joint that may be holding an object based
        // on the current (potentially new) orientation of the player camera
        // and any configured "grab distance" (mouse wheel scroll).
        if ( @mJoint != null )
        {
            RenderDriver @ driver = getAppRenderDriver();
            Viewport viewport = driver.getViewport();
            Size viewportSize = Size(viewport.width, viewport.height);

            // Convert the location at the center of the screen into a ray for picking
            // and update the location of the joint.
            Vector3 rayOrigin, rayDir;
            if ( mCamera.viewportToRay( viewportSize, Vector2(viewportSize.width/2.0f, viewportSize.height/2.0f), rayOrigin, rayDir ) )
                mJoint.setPosition( rayOrigin + rayDir * mGrabDistance );
        
        } // End if holding an object	    
	}

	//-------------------------------------------------------------------------
	// Name : onKeyPressed ( ) (Event)
	// Desc : Called by the input driver whenever a key is first pressed OR if
	//        the key repeat is fired.
	//-------------------------------------------------------------------------
	void onKeyPressed( int keyCode, uint controlCodes )
	{
		InputDriver @ input = getAppInputDriver();
	}

	//-------------------------------------------------------------------------
	// Name : onMouseMove () (Event)
	// Desc : Called by the input driver whenever the mouse moves at all.
	//-------------------------------------------------------------------------
	void onMouseMove( const Point & position, const Vector2 & offset )
	{
        InputDriver @ input = getAppInputDriver();
		if ( input.getMouseMode() == MouseHandlerMode::Cursor )
            return;

		// Only record object rotation if we're not in cursor mode
        mTotalOffset += offset;
	}

    //-------------------------------------------------------------------------
	// Name : onMouseWheelScroll () (Event)
	// Desc : Called by the input driver whenever the user scrolls the mouse
    //        wheel.
	//-------------------------------------------------------------------------
    void onMouseWheelScroll( int delta, const Point & position )
    {
        InputDriver @ input = getAppInputDriver();
		if ( input.getMouseMode() == MouseHandlerMode::Cursor )
            return;

        // Adjust object grab distance to bring it closer to / father away from the player.
        mGrabDistance += float(delta) * 0.5f;
        if ( mGrabDistance < 0.7 )
            mGrabDistance = 0.7;
    }

    //-------------------------------------------------------------------------
	// Name : onMouseButtonDown () (Event)
	// Desc : Called by the input driver whenever the user presses a mouse
    //        button.
	//-------------------------------------------------------------------------
    void onMouseButtonDown( int buttons, const Point & position )
    {
        InputDriver @ input = getAppInputDriver();
		if ( input.getMouseMode() == MouseHandlerMode::Cursor )
            return;

        // Grab and/or throw an object.
        if ( @mSelectedNode != null && (buttons & MouseButtons::Left) != 0 )
        {
            // Throw the object by applying an impulse at the point we're grabbing.
            mSelectedNode.applyImpulse( mCamera.getZAxis() * 20.0f, mJoint.getPosition() );

            // Destroy any joint holding up this body.
            @mJoint = null;

        } // End if object held
        else if ( (buttons & MouseButtons::Right) != 0 )
        {
            // Attempt to grab an object in the world.
            grabObject();
        
        } // End if RMB
    }

    //-------------------------------------------------------------------------
	// Name : onMouseButtonDown () (Event)
	// Desc : Called by the input driver whenever the user releases a mouse
    //        button.
	//-------------------------------------------------------------------------
    void onMouseButtonUp( int buttons, const Point & position )
    {
        // Always allow this to run even if UI is up so event doesn't get lost.
        if ( @mSelectedNode != null && (buttons & MouseButtons::Right) != 0 )
        {
            // Destroy any joint holding up this body.
            @mJoint = null;
            @mSelectedNode = null;

        } // End if RMB
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : processInput () (Private)
	// Desc : Process any input recorded in response to input events, or
    //        perform any additional input processing that we want to occur
    //        specifically during the object 'update()' pass.
	//-------------------------------------------------------------------------
    private void processInput( float elapsedTime )
    {
        // Bail if the cursor is currently visible.
        InputDriver @ input = getAppInputDriver();
        if ( input.getMouseMode() == MouseHandlerMode::Cursor )
        {
            // Set all input to zero.
            mParent.setInputChannelState( "ForwardBackward", 0 );
            mParent.setInputChannelState( "LeftRight", 0 );
            mParent.setInputChannelState( "ClimbSink", 0 );
            mParent.setInputChannelState( "Jump", 0 );
            return;
        
        } // End if cursor visible

		// Consume only a part of the total mouse motion (a hacky form of mouse smoothing :))
		// Try to avoid numbers lower than 35-40. The lag introduced can contribute to the
		// onset of motion sickness in even hardcore players! :D
		float consumeRate = 40.0f * elapsedTime;

		// We'll also add a single frame average to help out in the low frame-rate
		// cases (i.e. 30fps or below) where the above smoothing can't be applied.
		Vector2 rotation = mTotalOffset * consumeRate;
		//Vector2 finalRotation = (mPrevRotation + rotation) * 0.5f;
        //mPrevRotation = rotation;
		Vector2 finalRotation = mTotalOffset;
		consumeRate = 1;		

		// Rotate parent left/right and its child camera up/down.
		if ( finalRotation.x != 0.0f )
			mParent.rotateAxis( finalRotation.x, Vector3( 0, 1, 0 ) );
        if ( @mCamera != null && finalRotation.y != 0.0f )
            mCamera.rotateLocal( finalRotation.y, 0, 0 );

		// Remove consumed cursor motion (so we don't keep on applying it)
		mTotalOffset -= mTotalOffset * min( 1.0f, consumeRate );

        // Key based Rotate Left/Right
        float angularSpeed  = 420.0f * elapsedTime;
		if ( input.isKeyPressed( Keys::E ) )
			mParent.rotateAxis( angularSpeed, Vector3( 0, 1, 0 ) );
		if ( input.isKeyPressed( Keys::Q ) )
			mParent.rotateAxis( -angularSpeed, Vector3( 0, 1, 0 ) );

        // Check the relevant keys for movement input
        float strafeState = 0.0f, dollyState = 0.0f, jumpState = 0.0f, climbState = 0.0f;
	    if ( input.isKeyPressed( Keys::W ) || input.isKeyPressed( Keys::Up ) )
            dollyState += 1.0f;
	    if ( input.isKeyPressed( Keys::S ) || input.isKeyPressed( Keys::Down ) )
		    dollyState -= 1.0f;
	    if ( input.isKeyPressed( Keys::A ) || input.isKeyPressed( Keys::Left ))
		    strafeState -= 1.0f;
	    if ( input.isKeyPressed( Keys::D ) || input.isKeyPressed( Keys::Right ) )
		    strafeState += 1.0f;
        if ( input.isKeyPressed( Keys::Space ) )
            jumpState = 1.0f;
	    if ( input.isKeyPressed( Keys::R ) || input.isKeyPressed( Keys::Space )  )
		    climbState = 1.0f;
	    if ( input.isKeyPressed( Keys::F ) || input.isKeyPressed( Keys::LControl ) )
            climbState = -1.0f;

        // Set the input channel states (the character controller listens in on these).
        mParent.setInputChannelState( "ForwardBackward", dollyState );
        mParent.setInputChannelState( "LeftRight", strafeState );
        mParent.setInputChannelState( "ClimbSink", climbState );
        mParent.setInputChannelState( "Jump", jumpState );
    }

    //-------------------------------------------------------------------------
	// Name : grabObject () (Private)
	// Desc : Attempt to grab an object in the scene and pick it up (by 
    //        creating and attaching kinematic physics controller joint).
	//-------------------------------------------------------------------------
    private void grabObject( )
    {
        RenderDriver @ driver = getAppRenderDriver();
        Viewport viewport = driver.getViewport();
        Size viewportSize = Size(viewport.width, viewport.height);

        // Convert the location at the center of the screen into a ray for picking
        Vector3 rayOrigin, rayDir;
        if ( !mCamera.viewportToRay( viewportSize, Vector2(viewportSize.width/2.0f, viewportSize.height/2.0f), rayOrigin, rayDir ) )
            return;

        // Ask the scene to retrieve the closest intersected object node.
        Vector3 intersection;
        Scene @ scene = mParent.getScene();
        @mSelectedNode = scene.pickClosestNode( viewportSize, rayOrigin, rayDir, intersection );

        // Take action based on the selected object node
        if ( @mSelectedNode != null && @mSelectedNode.getPhysicsBody() != null )
        {
            // Compute the distance at which we 'grabbed' the node
            // so that this can be maintained.
            mGrabDistance = vec3Length( (intersection - rayOrigin) );

            // Create a new kinematic controller joint in the physics world at the intersection location.
            @mJoint = KinematicControllerJoint( scene.getPhysicsWorld(), mSelectedNode.getPhysicsBody(), intersection );

            // Set limits.
            mJoint.setConstraintMode( KinematicControllerMode::SinglePoint );
            mJoint.setMaxAngularFriction( 50.0f );
            mJoint.setMaxLinearFriction( 500.0f );

        } // End if is physics object
        else
        {
            @mSelectedNode = null;
        
        } // End if no physics object
    }

} // End Class Player