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
// Name : Casing.gsh                                                         //
//                                                                           //
// Desc : Behavior associated with casings ejected from a weapon.            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Casing (Class)
// Desc : Behavior associated with casings ejected from a weapon.
//-----------------------------------------------------------------------------
shared class Casing : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@             mNode;                  // The node to which we are attached.
    private Vector3                 mVelocity;              // Current velocity.
    private float                   mTimeAlive;             // Amount of time this has been alive.
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Casing () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Casing( )
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
        // Initialize variables
        @mNode     = object;
        mVelocity  = object.getXAxis() * randomFloat( 1.5f, 2.0f );
        mTimeAlive = 0;
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release our references.
        @mNode = null;
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Update casing location.
        mVelocity += Vector3( 0, -9.0f, 0 ) * elapsedTime;
        mNode.move( mVelocity * elapsedTime );

        // Destroy the casing after a second or so.
        mTimeAlive += elapsedTime;
        if ( mTimeAlive >= 1.0f )
            mNode.unload();
	}

} // End Class Casing