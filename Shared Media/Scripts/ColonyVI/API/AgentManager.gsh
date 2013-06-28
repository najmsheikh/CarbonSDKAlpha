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
// Name : AgentManager.gsh                                                   //
//                                                                           //
// Desc : Class that is responsible for managing the game's agents (NPCs,    //
//        player, etc.)                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------
#include_once "AudioManager.gsh"
#include_once "AgentStateIdentifiers.gsh"
#include_once "NPCAgent.gsh"

//-----------------------------------------------------------------------------
// Event Argument Types
//-----------------------------------------------------------------------------
shared class AgentEventArgs
{
    AgentEventArgs( Agent @ _agent )
    {
        @agent = _agent;
    }
    Agent@  agent;
}

shared class AgentDeathEventArgs : AgentEventArgs
{
    AgentDeathEventArgs( Agent @ _agent )
    {
        super( _agent );
    }
}

//-----------------------------------------------------------------------------
// Delegate Definitions
//-----------------------------------------------------------------------------
funcdef void AgentDeathEvent( AgentDeathEventArgs @ e );

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
shared enum GameAgentType
{
    Hound,
    Orc,
    Trooper,
    Turret,
    StandardMech
};

shared class AgentData
{
    Agent@          agent;
    ObjectNode@     node;
    ObjectBehavior@ behavior;
    bool            active;
}

shared class Faction
{
    array<AgentData@>   agents;
}

shared class AgentSpawnItem
{
    GameAgentType   agentType;
    uint            spawnPointId;
    int             spawnStateId;
    bool            preSpawned;
    int             context;
    float           delay;
    float           elapsed;
    AgentSpawnItem@ next;
    AgentSpawnItem@ previous;
}

//-----------------------------------------------------------------------------
// Name : AgentManager (Class)
// Desc : Class that is responsible for managing the game's agents (NPCs,
//        player, etc.)
//-----------------------------------------------------------------------------
shared class AgentManager
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AudioManager@           mAudioManager;
    private Scene@                  mScene;
    private ObjectNode@             mPlayerNode;            // Object node used to represent dummy player object.
    private Agent@                  mPlayer;                // Cached reference to the main player behavior.
    private bool                    mDebugEnabled;
    private AgentSpawnItem@         mSpawnListHead;
    private AgentSpawnItem@         mSpawnListTail;
    private float                   mTimeSinceLastSpawn;
    
    // Event handlers
    private array<AgentDeathEvent@> mAgentDeathDelegates;

    ///////////////////////////////////////////////////////////////////////////
	// Public Member Variables
	///////////////////////////////////////////////////////////////////////////
    array<Faction@> factions;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : AgentManager () (Constructor)
	// Desc : constructor for this class.
	//-------------------------------------------------------------------------
    AgentManager( AudioManager@ audioManager, Scene@ scene, ObjectNode@ playerNode, Agent@ playerAgent, bool debugEnabled )
    {
        // Initialize variables
        @mAudioManager      = audioManager;
        @mScene             = scene;
        @mPlayerNode        = playerNode;
        @mPlayer            = playerAgent;
        mDebugEnabled       = debugEnabled;
        mTimeSinceLastSpawn = 0;

        // There are two factions.
        factions.resize(2);
        @factions[0] = Faction();
        @factions[1] = Faction();

        // Add the player to the correct faction.
        Faction @ faction = factions[playerAgent.getFaction()];
        faction.agents.resize( 1 );
        AgentData @ data = AgentData();
        @data.agent      = playerAgent;
        @data.node       = playerNode;
        @data.behavior   = playerNode.getBehavior(0);
        data.active      = true;
        @faction.agents[0] = data;

        // Attach player to this agent manager.
        playerAgent.setAgentManager( this );
        
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : registerOnAgentDeath ()
    // Desc : Allows the caller to register to receive a notification when
    //        an agent is killed.
    //-------------------------------------------------------------------------
    void registerOnAgentDeath( AgentDeathEvent @ delegate )
    {
        mAgentDeathDelegates.resize(mAgentDeathDelegates.length() + 1);
        mAgentDeathDelegates[mAgentDeathDelegates.length()-1] = delegate;
    }

    //-------------------------------------------------------------------------
    // Name : shutdown ()
    // Desc : Shut down the manager and clean up resources.
    //-------------------------------------------------------------------------
    void shutdown( )
    {
        factions.resize(0);
        @mScene             = null;
        @mPlayerNode        = null;
        @mPlayer            = null;
        @mSpawnListHead     = null;
        @mSpawnListTail     = null;
        mTimeSinceLastSpawn = 0;
    }

    //-------------------------------------------------------------------------
    // Name : initializePreSpawnedAgent ()
    // Desc : Initialize an agent that has been pre-spawned.
    //-------------------------------------------------------------------------
    bool initializePreSpawnedAgent( GameAgentType type, uint agentRefId )
    {
        return initializePreSpawnedAgent( type, agentRefId, AgentStateId::Idle );
    }

    //-------------------------------------------------------------------------
    // Name : initializePreSpawnedAgent ()
    // Desc : Initialize an agent that has been pre-spawned.
    //-------------------------------------------------------------------------
    bool initializePreSpawnedAgent( GameAgentType type, uint agentRefId, int spawnStateId )
    {
        AgentSpawnItem@ item = AgentSpawnItem();
        item.agentType    = type;
        item.spawnPointId = agentRefId;
        item.spawnStateId = spawnStateId;
        item.preSpawned   = true;
        return processSpawnItem( item );
    }

    //-------------------------------------------------------------------------
    // Name : spawnAgent ()
    // Desc : Spawn in a single agent at the requested spawn point.
    //-------------------------------------------------------------------------
    bool spawnAgent( GameAgentType type, uint spawnPointRefId, int context = 0 )
    {
        return spawnAgent( type, spawnPointRefId, AgentStateId::Idle, false, context );
    }

    //-------------------------------------------------------------------------
    // Name : spawnAgent ()
    // Desc : Spawn in a single agent at the requested spawn point.
    //-------------------------------------------------------------------------
    bool spawnAgent( GameAgentType type, uint spawnPointRefId, bool immediate, int context = 0 )
    {
        return spawnAgent( type, spawnPointRefId, AgentStateId::Idle, immediate, context );
    }

    //-------------------------------------------------------------------------
    // Name : spawnAgent ()
    // Desc : Spawn in a single agent at the requested spawn point.
    //-------------------------------------------------------------------------
    bool spawnAgent( GameAgentType type, uint spawnPointRefId, AgentStateId spawnStateId, int context = 0 )
    {
        return spawnAgent( type, spawnPointRefId, spawnStateId, false, context );
    }

    //-------------------------------------------------------------------------
    // Name : spawnAgent ()
    // Desc : Spawn in a single agent at the requested spawn point.
    //-------------------------------------------------------------------------
    bool spawnAgent( GameAgentType type, uint spawnPointRefId, AgentStateId spawnStateId, bool immediate, int context = 0, float delay = 0.0f )
    {
        // Build the new spawn instruction
        AgentSpawnItem@ item = AgentSpawnItem();
        item.agentType    = type;
        item.spawnPointId = spawnPointRefId;
        item.spawnStateId = spawnStateId;
        item.preSpawned   = false;
        item.context      = context;
        item.delay        = delay;
        item.elapsed      = 0.0f;

        // Immediate spawn?
        if ( !immediate )
        {
            // Add to the spawn list.
            if ( @mSpawnListTail == null )
            {
                @mSpawnListHead = item;
                @mSpawnListTail = item;
            
            } // End if no items
            else
            {
                @mSpawnListTail.next = item;
                @item.previous = mSpawnListTail;
                @mSpawnListTail = item;
            
            } // End if list exists

        } // End if !immediate
        else
            processSpawnItem( item );
        
        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : process ()
    // Desc : Allow the agent manager to perform runtime processing.
    //-------------------------------------------------------------------------
    void process( float elapsedTime )
    {
        mTimeSinceLastSpawn += elapsedTime;

        // Time to spawn a new item?
        if ( mTimeSinceLastSpawn >= 0.4f )
        {
            mTimeSinceLastSpawn -= 0.4f;

            // Anything to do?
            if ( @mSpawnListHead == null )
                return;

            // Get next item from the spawn list. Skip if
            // its delay has not elapsed.
            AgentSpawnItem @ item = mSpawnListHead;
            item.elapsed += 0.4f;
            if ( item.elapsed < item.delay )
                return;
            
            // Remove from the list.
            @mSpawnListHead = item.next;
            if ( @mSpawnListHead == null )
                @mSpawnListTail = null;
            else
                @mSpawnListHead.previous = null;

            // Spawn the item.
            processSpawnItem( item );

        } // End if spawn!
    }

    //-------------------------------------------------------------------------
    // Name : getPotentialTarget ()
    // Desc : Find an appropriate target from the alternative faction.
    //-------------------------------------------------------------------------
    Agent @ getPotentialTarget( Agent @ sourceAgent, int & indexStartAt )
    {
        int initialIndex = indexStartAt;
        int searchFaction = (sourceAgent.getFaction() == 0) ? 1 : 0;
        Faction @ faction = factions[searchFaction];

        // If there are no agents in this faction, bail.
        if ( faction.agents.length() == 0 )
            return null;

        // This agent appropriate?
        AgentData @ data = faction.agents[indexStartAt++];
        if ( indexStartAt >= faction.agents.length() )
            indexStartAt = 0;
        if ( data.active && data.agent.isAlive() && !data.node.isDisposed() )
            return @data.agent;
        
        // Search for more
        while ( indexStartAt != initialIndex )
        {
            @data = faction.agents[indexStartAt++];
            if ( indexStartAt >= faction.agents.length() )
                indexStartAt = 0;
            if ( data.active && data.agent.isAlive() && !data.node.isDisposed() )
                return @data.agent;

        } // Next test
        
        // Nothing found.
        return null;
    }

    //-------------------------------------------------------------------------
    // Name : getAudioManager ()
    // Desc : Retrieve the audio manager used for agent sounds.
    //-------------------------------------------------------------------------
    AudioManager@ getAudioManager()
    {
        return mAudioManager;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Event Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : onAgentDeath ()
    // Desc : Called in order to trigger all delegates who are listening for
    //        this event.
    //-------------------------------------------------------------------------
    void onAgentDeath( AgentDeathEventArgs @ e )
    {
        // Trigger delegate methods / functions.
        for ( uint i = 0; i < mAgentDeathDelegates.length(); ++i )
            mAgentDeathDelegates[i](e);
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : processSpawnItem () (Private)
    // Desc : Process the specified spawn item, and load the requested agent.
    //-------------------------------------------------------------------------
    private bool processSpawnItem( AgentSpawnItem @ item )
    {
        // Spawn the item.
        uint agentRefId = 0;
        ObjectBehavior @ behavior = ObjectBehavior( );
        switch ( item.agentType )
        {
            case GameAgentType::Turret:
                agentRefId = 0xDF5;
                if ( !behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Enemy_Turret.gs", "" ) )
                    return false;
                break;
                
            case GameAgentType::Hound:
                agentRefId = 0x80B;
                if ( !behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Enemy_Explosive_Hound.gs", "" ) )
                    return false;
                break;

            case GameAgentType::Orc:
                agentRefId = 0x691;
                if ( !behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Enemy_Orc.gs", "" ) )
                    return false;
                break;

            case GameAgentType::Trooper:
                agentRefId = 0x309;
                if ( !behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Ally_Trooper.gs", "" ) )
                    return false;
                break;

            case GameAgentType::StandardMech:
                agentRefId = 0xD8C;
                if ( !behavior.initialize( mScene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Enemy_Mech.gs", "" ) )
                    return false;
                break;
        
        } // End switch

        // Get the underlying agent script
        NPCAgent @ agent = cast<NPCAgent>(behavior.getScriptObject());
        if ( @agent == null )
            return false;

        // Object has been pre-spawned?
        ObjectNode @ agentNode = null;
        if ( item.preSpawned )
        {
            // Pre-spawned, just grab the existing object.
            @agentNode = mScene.getObjectNodeById( item.spawnPointId );
            if ( @agentNode == null )
                return false;

        } // End if pre-spawned
        else
        {
            // Retrieve the specified spawn point.
            ObjectNode @ spawnPoint = mScene.getObjectNodeById( item.spawnPointId );
            if ( @spawnPoint == null )
                return false;

            // Generate starting position
            Vector3 pos = spawnPoint.getPosition();
            
            // Spawn in an NPC actor at the selected position.
            @agentNode = mScene.loadObjectNode( agentRefId, CloneMethod::ObjectInstance, true );
            if ( @agentNode == null )
                return false;

            // Position the node.
            agentNode.setPosition( pos );
            agentNode.setOrientation( spawnPoint.getOrientation() );

        } // End if !pre-spawned

        // Attach agent to this manager and set its initial spawning state.
        agent.setAgentManager( this );
        agent.setSpawnDetails( item.spawnStateId, item.context );
        agent.targetLastKnownPos = mPlayerNode.getPosition();
        agent.targetHeight = mPlayer.getHeight();

        // Send the agent a reference to the main player. This saves each agent
        // having to find the player itself.
        agent.setPlayer( mPlayerNode );
        agent.debugEnabled = mDebugEnabled;
        agent.requestedOrientation = agentNode.getWorldTransform().orientation();

        // Add the agent to the appropriate faction.
        int factionId = agent.getFaction();
        int agentIndex = factions[factionId].agents.length();
        AgentData @ data = AgentData();
        @data.agent     = agent;
        @data.node      = agentNode;
        @data.behavior  = behavior;
        data.active     = true;
        factions[factionId].agents.resize( agentIndex + 1 );
        @factions[factionId].agents[ agentIndex ]     = data;

        // Attach the behavior to the agent and allow it to update 
        // so that it starts processing.
        agentNode.addBehavior( factions[factionId].agents[agentIndex].behavior );
        agentNode.setUpdateRate( UpdateRate::Always );

        // Automatically allow the agent to navigate freely 
        // in anything other than idle or searching state.
        if ( item.spawnStateId == AgentStateId::Idle )
            agent.enableNavigation( true );

        // Success!
        return true;
    }
};