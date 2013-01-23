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
// Name : cgNavigationAgent.cpp                                              //
//                                                                           //
// Desc : A navigation agent represents an entity that can be issued with    //
//        instructions for navigating along / through a provided navigation  //
//        mesh. This object will ultimately provide transformation details   //
//        to the owner outlining where the agent is in the world at any      //
//        point in time as it navigates.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgNavigationAgent Module Includes
//-----------------------------------------------------------------------------
#include <Navigation/cgNavigationAgent.h>
#include <Navigation/cgNavigationHandler.h>
#include <Navigation/cgNavigationTypes.h>
#include "../../Lib/DetourCrowd/Include/DetourCrowd.h"
#include "../../Lib/Detour/Include/DetourCommon.h"

///////////////////////////////////////////////////////////////////////////////
// cgNavigationAgent Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationAgent () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationAgent::cgNavigationAgent( cgNavigationHandler * handler ) : cgReference( cgReferenceManager::generateInternalRefId() )
{
    // Initialize variables to sensible defaults
    mHandler        = handler;
    mDetourIndex    = -1;
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationAgent () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationAgent::~cgNavigationAgent( )
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
void cgNavigationAgent::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Remove from the crowd
    if ( mDetourIndex >= 0 )
        mHandler->mCrowd->removeAgent( mDetourIndex );

    // Remove from the parent handler
    if ( mHandler )
    {
        // ToDo: replace with mHandler->removeAgent()
        cgNavigationHandler::AgentList::iterator itAgent;
        itAgent = std::find( mHandler->mAgents.begin(), mHandler->mAgents.end(), this );
        if ( itAgent != mHandler->mAgents.end() )
            mHandler->mAgents.erase( itAgent );
    
    } // End if has handler

    // Release any allocated resources.
    
    // Clear variables
    mDetourIndex = -1;

    // Call base if requested.
    if ( disposeBase )
        cgReference::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationAgent::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_NavigationAgent )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize and add the agent to the world at the specified position. The
/// application would not generally call this method directly, instead the
/// agent should be created via 'cgNavigationHandler::createAgent()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationAgent::initialize( const cgNavigationAgentCreateParams & params, const cgVector3 & position )
{
    // Store the parameters being used.
    mParams = params;

    // Build detour parameters
    dtCrowdAgentParams dtparams;
	memset( &dtparams, 0, sizeof(dtCrowdAgentParams) );
    dtparams.slowDownRadius        = params.slowDownRadius;
	dtparams.radius                = params.agentRadius;
	dtparams.height                = params.agentHeight;
	dtparams.maxAcceleration       = params.maximumAcceleration;
	dtparams.maxSpeed              = params.maximumSpeed;
	dtparams.collisionQueryRange   = dtparams.radius * 12.0f;
	dtparams.pathOptimizationRange = dtparams.radius * 30.0f;
	dtparams.updateFlags           = 0; 
    dtparams.obstacleAvoidanceType = (cgByte)params.avoidanceQuality;
	dtparams.separationWeight      = params.separationWeight;
	if ( params.anticipateTurns )
        dtparams.updateFlags      |= DT_CROWD_ANTICIPATE_TURNS;
	if ( params.optimizeVisibility )
	    dtparams.updateFlags      |= DT_CROWD_OPTIMIZE_VIS;
	if ( params.optimizeTopology )
	    dtparams.updateFlags      |= DT_CROWD_OPTIMIZE_TOPO;
	if ( params.obstacleAvoidance )
	    dtparams.updateFlags      |= DT_CROWD_OBSTACLE_AVOIDANCE;
	if ( params.separation )
	    dtparams.updateFlags      |= DT_CROWD_SEPARATION;
    
    // Add to detour
    mDetourIndex = mHandler->mCrowd->addAgent( position, &dtparams );
	
    // Added new agent?
    return (mDetourIndex != -1);
}

//-----------------------------------------------------------------------------
//  Name : updateParameters ()
/// <summary>
/// Update the parameters that describe the agent's properties and behavior.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationAgent::updateParameters( const cgNavigationAgentCreateParams & params )
{
    // Store the parameters being used.
    mParams = params;

    // Anything else to do?
    if ( mDetourIndex < 0 )
        return;

    // Build detour parameters
    dtCrowdAgentParams dtparams;
	memset( &dtparams, 0, sizeof(dtCrowdAgentParams) );
    dtparams.slowDownRadius        = params.slowDownRadius;
	dtparams.radius                = params.agentRadius;
	dtparams.height                = params.agentHeight;
	dtparams.maxAcceleration       = params.maximumAcceleration;
	dtparams.maxSpeed              = params.maximumSpeed;
	dtparams.collisionQueryRange   = dtparams.radius * 12.0f;
	dtparams.pathOptimizationRange = dtparams.radius * 30.0f;
	dtparams.updateFlags           = 0; 
    dtparams.obstacleAvoidanceType = (cgByte)params.avoidanceQuality;
	dtparams.separationWeight      = params.separationWeight;
	if ( params.anticipateTurns )
        dtparams.updateFlags      |= DT_CROWD_ANTICIPATE_TURNS;
	if ( params.optimizeVisibility )
	    dtparams.updateFlags      |= DT_CROWD_OPTIMIZE_VIS;
	if ( params.optimizeTopology )
	    dtparams.updateFlags      |= DT_CROWD_OPTIMIZE_TOPO;
	if ( params.obstacleAvoidance )
	    dtparams.updateFlags      |= DT_CROWD_OBSTACLE_AVOIDANCE;
	if ( params.separation )
	    dtparams.updateFlags      |= DT_CROWD_SEPARATION;
	
    // Update detour
    mHandler->mCrowd->updateAgentParameters( mDetourIndex, &dtparams );
}

//-----------------------------------------------------------------------------
//  Name : getParameters ()
/// <summary>
/// Retrieve the parameters that describe the agent's properties and behavior
/// specified when the agent was created, or in a subsequent call to
/// 'updateParameters()'.
/// </summary>
//-----------------------------------------------------------------------------
const cgNavigationAgentCreateParams & cgNavigationAgent::getParameters( ) const
{
    return mParams;
}

//-----------------------------------------------------------------------------
//  Name : setMoveTarget ()
/// <summary>
/// Attempt to navigate the specified position.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationAgent::setMoveTarget( const cgVector3 & position, bool adjust )
{
    dtNavMeshQuery      * navQuery = mHandler->mQuery;
	dtCrowd             * crowd    = mHandler->mCrowd;
	const dtQueryFilter * filter   = crowd->getFilter();
	const cgFloat       * extents  = crowd->getQueryExtents();

    // ToDo: error check and return error results.

    // Find the closest point that we can potentially navigate to.
    navQuery->findNearestPoly( position, extents, filter, &mTargetRef, mTargetPosition);
    if ( !mTargetRef )
        return false;

    // Request that we navigate to this position.
    const dtCrowdAgent * agent = crowd->getAgent( mDetourIndex );
    if ( agent && agent->active )
        return crowd->requestMoveTarget( mDetourIndex, mTargetRef, mTargetPosition );

    // Not a valid operation.
    return false;
		
	/*if (adjust)
	{
		float vel[3];
		// Request velocity
		if (m_agentDebug.idx != -1)
		{
			const dtCrowdAgent* ag = crowd->getAgent(m_agentDebug.idx);
			if (ag && ag->active)
			{
				calcVel(vel, ag->npos, p, ag->params.maxSpeed);
				crowd->requestMoveVelocity(m_agentDebug.idx, vel);
			}
		}
		else
		{
			for (int i = 0; i < crowd->getAgentCount(); ++i)
			{
				const dtCrowdAgent* ag = crowd->getAgent(i);
				if (!ag->active) continue;
				calcVel(vel, ag->npos, p, ag->params.maxSpeed);
				crowd->requestMoveVelocity(i, vel);
			}
		}
	}
	else
	{
		navquery->findNearestPoly(p, ext, filter, &m_targetRef, m_targetPos);
		
		if (m_agentDebug.idx != -1)
		{
			const dtCrowdAgent* ag = crowd->getAgent(m_agentDebug.idx);
			if (ag && ag->active)
				crowd->requestMoveTarget(m_agentDebug.idx, m_targetRef, m_targetPos);
		}
		else
		{
			for (int i = 0; i < crowd->getAgentCount(); ++i)
			{
				const dtCrowdAgent* ag = crowd->getAgent(i);
				if (!ag->active) continue;
				crowd->requestMoveTarget(i, m_targetRef, m_targetPos);
			}
		}
	}*/
}

//-----------------------------------------------------------------------------
//  Name : setMoveVelocity ()
/// <summary>
/// Attempt to move the agent based on the specified velocity.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationAgent::setMoveVelocity( const cgVector3 & velocity )
{
	dtCrowd * crowd    = mHandler->mCrowd;

    // Request that we navigate to this position.
    const dtCrowdAgent * agent = crowd->getAgent( mDetourIndex );
    if ( agent && agent->active )
        return crowd->requestMoveVelocity( mDetourIndex, velocity );

    // Not a valid operation.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getPosition ()
/// <summary>
/// Retrieve the current position of the agent.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgNavigationAgent::getPosition( ) const
{
    dtCrowd * crowd = mHandler->mCrowd;
    const dtCrowdAgent * agent = crowd->getAgent( mDetourIndex );
    if ( !agent || !agent->active )
        return cgVector3(0,0,0);
    return agent->npos;
}

//-----------------------------------------------------------------------------
//  Name : getDesiredVelocity ()
/// <summary>
/// Retrieve the velocity that the agent is currently trying to achieve.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgNavigationAgent::getDesiredVelocity( ) const
{
    dtCrowd * crowd = mHandler->mCrowd;
    const dtCrowdAgent * agent = crowd->getAgent( mDetourIndex );
    if ( !agent || !agent->active )
        return cgVector3(0,0,0);
    return agent->dvel;
}

//-----------------------------------------------------------------------------
//  Name : getActualVelocity ()
/// <summary>
/// Retrieve the current actual velocity for the agent.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgNavigationAgent::getActualVelocity( ) const
{
    dtCrowd * crowd = mHandler->mCrowd;
    const dtCrowdAgent * agent = crowd->getAgent( mDetourIndex );
    if ( !agent || !agent->active )
        return cgVector3(0,0,0);
    return agent->vel;
}

//-----------------------------------------------------------------------------
// Name : onNavigationAgentReposition() (Virtual)
/// <summary>
/// Can be overridden or called by derived class when the agent is being 
/// repositioned in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationAgent::onNavigationAgentReposition( cgNavigationAgentRepositionEventArgs * e )
{
    // Trigger 'onNavigationAgentReposition' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList Listeners = mEventListeners;
    for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
        (static_cast<cgNavigationAgentEventListener*>(*itListener))->onNavigationAgentReposition( this, e );
}

//-----------------------------------------------------------------------------
// Name : getAgentState()
/// <summary>
/// Determine the current state of the agent, i.e. whether it is currently 
/// in a position to receieve navigation requests.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationAgentState::Base cgNavigationAgent::getAgentState( ) const
{
    dtCrowd * crowd = mHandler->mCrowd;
    const dtCrowdAgent * agent = crowd->getAgent( mDetourIndex );
    if ( !agent || !agent->active )
        return cgNavigationAgentState::Invalid;
    return (cgNavigationAgentState::Base)agent->state;
}

//-----------------------------------------------------------------------------
// Name : getTargetState()
/// <summary>
/// Determine the current state of the most recent navigation request.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationTargetState::Base cgNavigationAgent::getTargetState( ) const
{
    dtCrowd * crowd = mHandler->mCrowd;
    const dtCrowdAgent * agent = crowd->getAgent( mDetourIndex );
    if ( !agent || !agent->active )
        return cgNavigationTargetState::None;

    // Determine if we've arrived at the target
    if ( agent->targetState == DT_CROWDAGENT_TARGET_VALID )
    {
        if ( agent->ncorners )
        {
            // Is the agent at the end of its path?
		    const bool endOfPath = (agent->cornerFlags[agent->ncorners-1] & DT_STRAIGHTPATH_END) ? true : false;
	        if ( endOfPath )
            {
                // Within its own radius of the goal?
                if ( dtVdist2D(agent->npos, &agent->cornerVerts[(agent->ncorners-1)*3]) <= agent->params.radius )
                    return cgNavigationTargetState::Arrived;
            
            } // End if reaching end

        } // End if has path
    	
    } // End if valid

    // Return the underlying state.
    return (cgNavigationTargetState::Base)agent->targetState;
}

//-----------------------------------------------------------------------------
// Name : getHandler ()
/// <summary>
/// Get the handler that is responsible for managing this navigation agent.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationHandler * cgNavigationAgent::getHandler( ) const
{
    return mHandler;
}