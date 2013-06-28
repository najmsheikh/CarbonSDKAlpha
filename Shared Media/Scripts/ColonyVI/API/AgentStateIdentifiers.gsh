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
// Name : AgentStateIdentifiers.gsh                                          //
//                                                                           //
// Desc : Identifiers for all valid agent states.                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Identifier Defines
//-----------------------------------------------------------------------------
shared enum AgentStateId
{
    Unknown,

    // Basic standard states
    Idle,
    Firing,
    Repositioning,
    RepositionFiring,
    Searching,
    Flanking,
    Dead,
    Reloading,

    // Custom scripted states
    LeapDown,
    UsingTurret
};