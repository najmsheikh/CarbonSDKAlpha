//---------------------------------------------------------------------------//
//                         _                         _                       //
//                        / \   ___ _ __   ___  __ _| |_                     //
//                       / _ \ / __| '_ \ / _ \/ _` | __|                    //
//                      / ___ \\__ \ |_) |  __/ (_| | |_                     //
//                     /_/   \_\___/ .__/ \___|\__, |\__|                    //
//                                 |_|    Speqtre |_| Engine                 //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : PlayerInput.gs                                                     //
//                                                                           //
// Desc : Rudimentary input behavior designed to apply yaw to any "player"   //
//        controllable object to which it is assigned. No pitch is applied   //
//        in order to ensure that we do not interfere with any attached      //
//        physics process. Pitch should instead be applied only to a child   //
//        camera for instance (see CameraLook.gs).                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2008 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
IScriptedObjectBehavior @ createBehaviorScript( ObjectBehavior @ owner )
{
    return PlayerInputBehavior( owner );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : PlayerInputBehavior (Class)
// Desc : Behavior used to map input and other events to the main player
//        object. This is a simple implementation that allows the player to
//        move by applying appropriate forces, etc.
//-----------------------------------------------------------------------------
class PlayerInputBehavior : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectBehavior@ mBehavior;          // The parent application behavior object.
	private Vector2         mTotalOffset;       // Total amount of mouse motion recorded subsequent to previous update.
	private Vector2         mPrevRotation;      // Previous rotation angles used for a single frame average.

    // Right click physics grab
    private ObjectNode@                 mSelectedNode;  // Node selected when right clicking.
    private KinematicControllerJoint@   mJoint;         // Joint used for the physics constraint
    private float                       mGrabDistance;  // Distance from the camera to the grabbed object on first click.

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : PlayerInputBehavior () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	PlayerInputBehavior( ObjectBehavior @ owner )
    {
        // Duplicate the handle to the application defined
        // 'ObjectBehavior' instance that owns us.
        @mBehavior      = owner;

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
	void onAttach( )
	{
		// Notify the behavior to which we belong that we want
        // to listen out for user supplied input.
        mBehavior.registerAsInputListener();
	}

	//-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from an
	//        object and can shut down.
	//-------------------------------------------------------------------------
	void onDetach( )
	{
		// We no longer want to listen for user supplied input.
		mBehavior.unregisterAsInputListener();
	}

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        InputDriver @ inputDriver   = getAppInputDriver();
		float         angularSpeed  = 120.0f * elapsedTime;
		ObjectNode  @ parentObject  = mBehavior.getParentObject();

		// Bail if the cursor is up.
        if ( inputDriver.getMouseMode() == MouseHandlerMode::Cursor )
            return;

		// Consume only a part of the total mouse motion (a hacky form of mouse smoothing :))
		// Try to avoid numbers lower than 35-40. The lag introduced can contribute to the
		// onset of motion sickness in even hardcore players! :D
		float consumeRate = 40.0f * elapsedTime;

		// We'll also add a single frame average to help out in the low frame-rate
		// cases (i.e. 30fps or below) where the above smoothing can't be applied.
		Vector2 rotation = mTotalOffset * consumeRate;
		Vector2 finalRotation = (mPrevRotation + rotation) * 0.5f;
		mPrevRotation = rotation;

        // Get the camera attached as an immediate child of the parent object.
        CameraNode @ camera = cast<CameraNode>(parentObject.findChildOfType( RTID_CameraObject, false ));

		// Rotate parent Left/Right
		if ( finalRotation.x != 0.0f )
			parentObject.rotateAxis( finalRotation.x, Vector3( 0, 1, 0 ) );
        if ( @camera != null && finalRotation.y != 0.0f )
            camera.rotateLocal( finalRotation.y, 0, 0 );

		// Remove consumed cursor motion (so we don't keep on applying it)
		mTotalOffset -= mTotalOffset * min( 1.0f, consumeRate );

        // Key based Rotate Left/Right
		if ( inputDriver.isKeyPressed( Keys::E ) )
			parentObject.rotateAxis( angularSpeed, Vector3( 0, 1, 0 ) );
		if ( inputDriver.isKeyPressed( Keys::Q ) )
			parentObject.rotateAxis( -angularSpeed, Vector3( 0, 1, 0 ) );

        // Check the relevant keys for movement input
        float strafeState = 0.0f, dollyState = 0.0f, jumpState = 0.0f, climbState = 0.0f;
	    if ( inputDriver.isKeyPressed( Keys::W ) || inputDriver.isKeyPressed( Keys::Up ) )
            dollyState += 1.0f;
	    if ( inputDriver.isKeyPressed( Keys::S ) || inputDriver.isKeyPressed( Keys::Down ) )
		    dollyState -= 1.0f;
	    if ( inputDriver.isKeyPressed( Keys::A ) || inputDriver.isKeyPressed( Keys::Left ))
		    strafeState -= 1.0f;
	    if ( inputDriver.isKeyPressed( Keys::D ) || inputDriver.isKeyPressed( Keys::Right ) )
		    strafeState += 1.0f;
        if ( inputDriver.isKeyPressed( Keys::Space ) )
            jumpState = 1.0f;
	    if ( inputDriver.isKeyPressed( Keys::R ) || inputDriver.isMouseButtonPressed( MouseButtons::Right )  )
		    climbState = 1.0f;
	    if ( inputDriver.isKeyPressed( Keys::F ) )
            climbState = -1.0f;

        // Set the input channel states (the character controller listens in on these).
        parentObject.setInputChannelState( "ForwardBackward", dollyState );
        parentObject.setInputChannelState( "LeftRight", strafeState );
        parentObject.setInputChannelState( "ClimbSink", climbState );
        parentObject.setInputChannelState( "Jump", jumpState );

        // Recompute position of joint holding object.
        if ( @mJoint != null )
        {
            RenderDriver @ driver = getAppRenderDriver();
            Viewport viewport = driver.getViewport();
            Size viewportSize = Size(viewport.width, viewport.height);

            // Convert the location at the center of the screen into a ray for picking
            // and update the location of the joint.
            Vector3 rayOrigin, rayDir;
            if ( camera.viewportToRay( viewportSize, Vector2(viewportSize.width/2.0f, viewportSize.height/2.0f), rayOrigin, rayDir ) )
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
		InputDriver @ inputDriver   = getAppInputDriver();
		ObjectNode  @ parentObject  = mBehavior.getParentObject();
	}

	//-------------------------------------------------------------------------
	// Name : onMouseMove () (Event)
	// Desc : Called by the input driver whenever the mouse moves at all.
	//-------------------------------------------------------------------------
	void onMouseMove( const Point & position, const Vector2 & offset )
	{
		InputDriver @ inputDriver = getAppInputDriver();

		// Only record object rotation if we're not in cursor mode
		if ( inputDriver.getMouseMode() != MouseHandlerMode::Cursor )
			mTotalOffset += offset;
	}

    //-------------------------------------------------------------------------
	// Name : onMouseWheelScroll () (Event)
	// Desc : Called by the input driver whenever the user scrolls the mouse
    //        wheel.
	//-------------------------------------------------------------------------
    void onMouseWheelScroll( int delta, const Point & position )
    {
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
        ObjectNode @ parentObject  = mBehavior.getParentObject();

        // Process input
        if ( @mSelectedNode != null && (buttons & MouseButtons::Left) != 0 )
        {
            // Get the camera attached as an immediate child of the parent object.
            ObjectNode @ camera = parentObject.findChildOfType( RTID_CameraObject, false );
            
            // Throw the object by applying an impulse at the point we're grabbing.
            if ( @camera != null )
                mSelectedNode.applyImpulse( camera.getZAxis() * 20.0f, mJoint.getPosition() );

            // Destroy any joint holding up this body.
            @mJoint = null;

        } // End if object held
        else if ( (buttons & MouseButtons::Right) != 0 )
        {
            RenderDriver @ driver = getAppRenderDriver();
            Viewport viewport = driver.getViewport();
            Size viewportSize = Size(viewport.width, viewport.height);

            // Get the camera attached as an immediate child of the parent object.
            CameraNode @ camera = cast<CameraNode>(parentObject.findChildOfType( RTID_CameraObject, false ));
            if ( @camera == null )
                return;

            // Convert the location at the center of the screen into a ray for picking
            Vector3 rayOrigin, rayDir;
            if ( !camera.viewportToRay( viewportSize, Vector2(viewportSize.width/2.0f, viewportSize.height/2.0f), rayOrigin, rayDir ) )
                return;

            // Ask the scene to retrieve the closest intersected object node.
            Vector3 intersection;
            Scene @ scene = camera.getScene();
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
        
        } // End if RMB
    }

    //-------------------------------------------------------------------------
	// Name : onMouseButtonDown () (Event)
	// Desc : Called by the input driver whenever the user releases a mouse
    //        button.
	//-------------------------------------------------------------------------
    void onMouseButtonUp( int buttons, const Point & position )
    {
        if ( @mSelectedNode != null && (buttons & MouseButtons::Right) != 0 )
        {
            // Destroy any joint holding up this body.
            @mJoint = null;
            @mSelectedNode = null;

        } // End if RMB

    }

} // End Class PlayerInputBehavior