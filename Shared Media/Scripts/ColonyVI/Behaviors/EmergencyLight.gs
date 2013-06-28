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
// Name : EmergencyLight.gs                                                 //
//                                                                           //
// Desc : Script attached to emergency lights (initially disabled) so that   //
//        they light up and spin when activated.                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : EmergencyLight (Class)
// Desc : Script attached to emergency lights (initially disabled) so that
//        they light up and spin when activated.
//-----------------------------------------------------------------------------
shared class EmergencyLight : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode @ mNode;
    private bool         mActivated;

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : EmergencyLight () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	EmergencyLight( )
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
        @mNode = object;
        
        // Hide light first of all
        mActivated = false;
        mNode.setUpdateRate( UpdateRate::Never );
        mNode.rotateLocal( 0, randomFloat( 0, 360.0f ), 0 );
        mNode.showNode( false, true );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Hide light when we leave.
        object.showNode( false, true );
    }

    //-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        if ( mActivated )
            mNode.rotateLocal( 0, 270.0f * elapsedTime, 0 );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : activate ()
	// Desc : Activate the emergency lights.
	//-------------------------------------------------------------------------
    void activate( )
    {
        mActivated = true;
        mNode.showNode( true, true );
        mNode.setUpdateRate( UpdateRate::Always );
    }

    //-------------------------------------------------------------------------
	// Name : deactivate ()
	// Desc : Deactivate the emergency lights.
	//-------------------------------------------------------------------------
    void deactivate( )
    {
        mActivated = false;
        mNode.showNode( false, true );
        mNode.setUpdateRate( UpdateRate::Never );
    }
}