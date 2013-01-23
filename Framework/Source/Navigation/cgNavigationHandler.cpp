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
// Name : cgNavigationHandler.cpp                                            //
//                                                                           //
// Desc : The top level class that owns and manages all currently defined    //
//        navigation agents.                                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgNavigationHandler Module Includes
//-----------------------------------------------------------------------------
#include <Navigation/cgNavigationHandler.h>
#include <Navigation/cgNavigationAgent.h>
#include <Navigation/cgNavigationMesh.h>
#include "../../Lib/DetourCrowd/Include/DetourCrowd.h"

///////////////////////////////////////////////////////////////////////////////
// cgNavigationHandler Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationHandler () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationHandler::cgNavigationHandler( )
{
    // Initialize variables to sensible defaults
    mCrowd      = CG_NULL;
    mNavMesh    = CG_NULL;
    mQuery      = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationHandler () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationHandler::~cgNavigationHandler( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgNavigationHandler::dispose( bool disposeBase )
{
    // Agent destruction is the responsibility of whoever is 
    // referencing them. We just clear out our internal list here.
    mAgents.clear();

    // Release any allocated resources.
    dtFreeCrowd( mCrowd );
    dtFreeNavMeshQuery( mQuery );
    
    // Clear variables
    mCrowd      = CG_NULL;
    mNavMesh    = CG_NULL;
    mQuery      = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the navigation handler for the spcified navigation mesh.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationHandler::initialize( cgNavigationMesh * navMesh, cgUInt32 maxAgents )
{
    // ToDo: Handle errors.

    // Initialize required members.
    mCrowd   = dtAllocCrowd();
    mQuery   = dtAllocNavMeshQuery();
    mNavMesh = navMesh;

    // Attempt to initialize the navigation query.
    dtStatus status = mQuery->init( navMesh->getInternalMesh(), 2048 );

    // Initialize the crowd
    mCrowd->init( maxAgents, navMesh->getParameters().agentRadius, navMesh->getInternalMesh() );
		
    // Make polygons with 'disabled' flag invalid.
    mCrowd->getEditableFilter()->setExcludeFlags( cgNavigationPolyFlags::Disabled );
		
    // Setup local avoidance params to different qualities.
	dtObstacleAvoidanceParams params;
	
    // Use mostly default settings, copy from dtCrowd.
	memcpy(&params, mCrowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));
		
	// Low (11)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 1;
	mCrowd->setObstacleAvoidanceParams(0, &params);
	
	// Medium (22)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5; 
	params.adaptiveRings = 2;
	params.adaptiveDepth = 2;
	mCrowd->setObstacleAvoidanceParams(1, &params);
	
	// High (45)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 3;
	mCrowd->setObstacleAvoidanceParams(2, &params);
	
	// Best (66)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 3;
	params.adaptiveDepth = 3;
	mCrowd->setObstacleAvoidanceParams(3, &params);

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createAgent ()
/// <summary>
/// Create a new navigation agent and add it to the world at the specified
/// location.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationAgent * cgNavigationHandler::createAgent( const cgNavigationAgentCreateParams & params, const cgVector3 & position )
{
    // Allocate the new agent.
    cgNavigationAgent * newAgent = new cgNavigationAgent( this );
    if ( !newAgent->initialize( params, position ) )
    {
        newAgent->deleteReference();
        return CG_NULL;
    
    } // End if failed

    // Add the list of agents being managed.
    mAgents.push_back( newAgent );

    // Return new agent.
    return newAgent;
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Allow navigation handler to step.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationHandler::update( cgFloat timeDelta )
{
    static float timeAcc = 0;

    // ToDo: 6767 - Configurable rate and maximum steps (0 = unlimited, warning)!

    // Update sample simulation.
	const float SIM_RATE = 60;
	const float DELTA_TIME = 1.0f/SIM_RATE;
    timeAcc += timeDelta;
    if ( timeAcc < -1 )
        timeAcc = -1;
    if ( timeAcc > 1 )
        timeAcc = 1;
	int simIter = 0;
	while (timeAcc > DELTA_TIME)
	{
		timeAcc -= DELTA_TIME;
		if (simIter < 5)
		    mCrowd->update( DELTA_TIME, CG_NULL );
        simIter++;
		
    } // Next iteration

    // Notify all agents that an update has occurred.
    AgentList::iterator itAgent;
    for ( itAgent = mAgents.begin(); itAgent != mAgents.end(); ++itAgent )
        (*itAgent)->onNavigationAgentReposition( &cgNavigationAgentRepositionEventArgs( (*itAgent)->getPosition(), true ) );
}

//-----------------------------------------------------------------------------
//  Name : step ()
/// <summary>
/// Perform a single step of the simulation iteration. Specified delta period
/// should be consistent between calls!
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationHandler::step( cgFloat timeDelta )
{
    mCrowd->update( timeDelta, CG_NULL );

    // Notify all agents that an update has occurred.
    AgentList::iterator itAgent;
    for ( itAgent = mAgents.begin(); itAgent != mAgents.end(); ++itAgent )
        (*itAgent)->onNavigationAgentReposition( &cgNavigationAgentRepositionEventArgs( (*itAgent)->getPosition(), true ) );
}