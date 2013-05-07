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
// Name : cgNavigationTypes.h                                                //
//                                                                           //
// Desc : Common system file that defines various navigation related types   //
//        and common enumerations.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNAVIGATIONTYPES_H_ )
#define _CGE_CGNAVIGATIONTYPES_H_

//-----------------------------------------------------------------------------
// cgNavigationTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
namespace cgNavigationRegionType
{
    enum Base
    {
	    Ground  = 0,
        Water,
        Road,
        Door,
        Grass,
        Jump
    };

} // End Namespace : cgNavigationRegionType

namespace cgNavigationPolyFlags
{
    enum Base
    {
	    Walk        = 0x01,		// Ability to walk (ground, grass, road)
	    Swim        = 0x02,		// Ability to swim (water).
	    Door        = 0x04,		// Ability to move through doors.
	    Jump        = 0x08,		// Ability to jump.
	    Disabled    = 0x10,		// Disabled polygon
	    All         = 0xffff	// All abilities.
    };

} // End Namespace : cgNavigationPolyFlags

namespace cgNavigationAvoidanceQuality
{
    enum Base
    {
        Low     = 0,
        Medium  = 1,
        High    = 2,
        Best    = 3
    };

} // End Namespace : cgNavigationAvoidanceQuality

namespace cgNavigationAgentState
{
    enum Base
    {
        Invalid = 0,
        Ready,
        TraversingLink
    };

} // End Namespace : cgNavigationTargetState

namespace cgNavigationTargetState
{
    enum Base
    {
        None    = 0,
        Failed,
        Valid,
        Requesting,
        WaitingForQueue,
        WaitingForPath,
        Velocity,
        Arrived
    };

} // End Namespace : cgNavigationTargetState

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct cgNavigationMeshCreateParams
{
    cgFloat cellSize;
	cgFloat cellHeight;
    cgInt32 tileCells;
    cgFloat agentRadius;
    cgFloat agentHeight;
    cgFloat agentMaximumSlope;
    cgFloat agentMaximumStepHeight;
    cgFloat edgeMaximumLength;
    cgFloat edgeMaximumError;
    cgFloat regionMinimumSize;
    cgFloat regionMergedSize;
    cgInt32 verticesPerPoly;
    cgFloat detailSampleDistance;
    cgFloat detailSampleMaximumError;

    // Provide defaults
    cgNavigationMeshCreateParams()
    {
        cellSize                 = 0.3f;
	    cellHeight               = 0.2f;
        tileCells                = 32;
        agentRadius              = 0.6f;
        agentHeight              = 1.8f;
        agentMaximumSlope        = 45.0f;
        agentMaximumStepHeight   = 0.4f;
        edgeMaximumLength        = 12.0f;
        edgeMaximumError         = 1.3f;
        regionMinimumSize        = 8.0f;
        regionMergedSize         = 20.0f;
        verticesPerPoly          = 6;
        detailSampleDistance     = 6.0f;
        detailSampleMaximumError = 1.0f;
    }

}; // End cgNavigationMeshCreateParams

struct cgNavigationAgentCreateParams
{
    cgFloat agentRadius;
    cgFloat agentHeight;
    cgFloat slowDownRadius;
    bool    anticipateTurns;
    bool    optimizeVisibility;
    bool    optimizeTopology;
    bool    obstacleAvoidance;
    bool    separation;
    cgFloat separationWeight;
    cgFloat maximumAcceleration;
    cgFloat maximumSpeed;
    cgNavigationAvoidanceQuality::Base avoidanceQuality;
    
    // Provide defaults
    cgNavigationAgentCreateParams()
    {
        agentRadius         = 0.6f;
        agentHeight         = 1.8f;
        slowDownRadius      = 1.2f;
        anticipateTurns     = true;
        optimizeVisibility  = true;
        optimizeTopology    = true;
        obstacleAvoidance   = true;
        separation          = true;
        separationWeight    = 2.0f;
        maximumAcceleration = 8.0f;
        maximumSpeed        = 3.5f;
        avoidanceQuality    = cgNavigationAvoidanceQuality::Best;
    }

}; // End cgNavigationAgentCreateParams

#endif // !_CGE_CGNAVIGATIONTYPES_H_