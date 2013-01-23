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
// Name : Objective.gsh                                                      //
//                                                                           //
// Desc : Base classes and core script logic for player objectives and user  //
//        activated game event triggers.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Objective (Base Class)
// Desc : Base classes and core script logic for player objectives and user
//        activated game event triggers.
//-----------------------------------------------------------------------------
shared class Objective : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@     mNode;              // The node to which we are attached.

    // Objective description.
    private float           mMaxTriggerRange;   // Maximum range at which this objective can be triggered.

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Objective () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Objective( )
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
        // Cache references to required objects.
        @mNode = object;

        // Get properties
        PropertyContainer @ properties = object.getCustomProperties();
        mMaxTriggerRange = float(properties.getProperty( "trigger_range", 2.0f ));
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        @mNode = null;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : getSceneNode ()
	// Desc : Get the scene node representing this objective.
	//-------------------------------------------------------------------------
    ObjectNode@ getSceneNode( )
    {
        return mNode;
    }

    //-------------------------------------------------------------------------
	// Name : isObjectiveInRange ()
	// Desc : Is the objective in range for triggering?
	//-------------------------------------------------------------------------
    bool isObjectiveInRange( Vector3 sourcePosition )
    {
        return (vec3LengthSq( mNode.getPosition() - sourcePosition ) <= (mMaxTriggerRange * mMaxTriggerRange));
    }

    //-------------------------------------------------------------------------
	// Name : distanceToObjective ()
	// Desc : Retrieve the current distance to the objective (in meters).
	//-------------------------------------------------------------------------
    float distanceToObjective( Vector3 sourcePosition )
    {
        return vec3Length( mNode.getPosition() - sourcePosition );
    }

    //-------------------------------------------------------------------------
	// Name : onActivate () (Event)
	// Desc : This objective was activated. Returns new objective (if any)
	//-------------------------------------------------------------------------
    Objective @ onActivate( ObjectNode @ issuer )
    {
        // Nothing in base implementation.
        return null;
    }
}