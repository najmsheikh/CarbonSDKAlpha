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
// Name : cgAppStateManager.cpp                                              //
//                                                                           //
// Desc : This module houses classes responsible for managing the state of   //
//        the application i.e. Main menu, game play, game over, credits etc. //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAppStateManager Module Includes
//-----------------------------------------------------------------------------
#include <States/cgAppStateManager.h>
#include <Resources/cgScript.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgAppStateManager * cgAppStateManager::mSingleton = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgAppStateManager Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAppStateManager () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAppStateManager::cgAppStateManager()
{
    // Initialize variables to sensible defaults
    mActiveState  = CG_NULL;
    mStateHistory = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgAppStateManager () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAppStateManager::~cgAppStateManager()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgAppStateManager::dispose( bool bDisposeBase )
{
    // End all currently running states
    stop();

    // Reset variables
    mActiveState  = CG_NULL;
    mStateHistory = CG_NULL;

    // Destroy any registered states
    StateMap::iterator itState;
    for ( itState = mRegisteredStates.begin(); itState != mRegisteredStates.end(); ++itState )
    {
        StateDesc & Desc = itState->second;
        if ( Desc.state )
            Desc.state->scriptSafeDispose();

    } // Next registered state

    // Clear any STL containers
    mRegisteredStates.clear();
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgAppStateManager * cgAppStateManager::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createSingleton () (Static)
/// <summary>
/// Creates the singleton. You would usually allocate the singleton in
/// the static member definition, however sometimes it's necessary to
/// call for allocation to allow for correct allocation ordering
/// and destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppStateManager::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
        mSingleton = new cgAppStateManager();
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppStateManager::destroySingleton( )
{
    // Destroy!
    if ( mSingleton != CG_NULL )
        delete mSingleton;
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : registerState ()
/// <summary>
/// Register a given type of state with the state management system.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppStateManager::registerState( cgAppState * pGameState )
{
    StateDesc          Desc;
    StateMap::iterator itState;

    // Validate requirements
    if ( !pGameState ) return false;

    // State already exists in the state map?
    itState = mRegisteredStates.find( pGameState->getStateId() );
    if ( itState != mRegisteredStates.end() ) return false;

    // If it doesn't exist, add it to the list of available state types
    Desc.type       = (pGameState->isTransitionState()) ? StateType_Transition : StateType_Normal;
    Desc.state = pGameState;
    mRegisteredStates[ pGameState->getStateId() ] = Desc;

    // Set up state properties
    pGameState->mStateManager = this;

    // Initialize the state and return result
    return pGameState->initialize();
}

//-----------------------------------------------------------------------------
//  Name : getActiveState ()
/// <summary>
/// Retrieve the currently active state (if any).
/// Note : Other states may still be resident and transitioning out / in if 
/// this is a transition state and can be accessed through the member
/// function exposed by the cgAppTransitionState class.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState * cgAppStateManager::getActiveState( )
{
    return mActiveState;
}

//-----------------------------------------------------------------------------
//  Name : setActiveState ()
/// <summary>
/// Immediately remove all other active states and set to the specified
/// state (if available).
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppStateManager::setActiveState( const cgString & strStateId )
{
    StateDesc Desc;

    // First find the requested state
    if ( !getStateDesc( strStateId, &Desc ) ) return false;

    // The state was found, end any currently executing states
    if ( mStateHistory ) mStateHistory->end();

    // Link this new state
    mStateHistory = Desc.state;
    mActiveState  = Desc.state;

    // Start it running.
    if ( mActiveState->isBegun() == false )
        mActiveState->begin();

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : stop ()
/// <summary>
/// Stop all running states, and simply place the game into an idle state.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppStateManager::stop( )
{
    // Stop all active states
    if ( mStateHistory ) mStateHistory->end();
}

//-----------------------------------------------------------------------------
//  Name : transitionState () (Private)
/// <summary>
/// State transition should occuring using a more comprehensive transition
/// process. This is achieved through a special type of game state
/// called a transition state (cgAppTransitionState). This transition
/// state will become the active state and decides when to ultimately
/// perform the transition.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppStateManager::transitionState( cgAppState * pFromState, const cgString & strToStateId, const cgString & strViaStateId )
{
    StateDesc               ToDesc, ViaDesc;
    cgAppState            * pParentState = CG_NULL, * pSelectedToState = CG_NULL, * pState = CG_NULL;
    cgAppTransitionState  * pSelectedViaState = CG_NULL;

    // Validate Requirements
    if ( !pFromState ) return false;

    // States can only be transitioned if they are in the current state history
    for ( pState = mStateHistory; pState != CG_NULL; pState = pState->mChildState )
    {
        if ( pState == pFromState ) break;
        
    } // Next in history list

    // Not in list?
    if ( pState == CG_NULL )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("You can only transition game states that currently exist in the state history.\n" ) );
        return false;

    } // End if from state not found

    // Select the state we're transitioning to
    // Note: We allow there to be no outgoing state (application will transition to exit?)
    ToDesc.state = CG_NULL;
    if ( getStateDesc( strToStateId, &ToDesc ) )
    {
        // Is this a valid state type?
        if ( ToDesc.type != StateType_Normal )
        {
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to transition to any state type other than 'normal' using complex transitioning.\n" ) );
            return false;
        
        } // End if not a normal state
    
    } // End if to state not found

    // Select the state we're transitioning via
    if ( !getStateDesc( strViaStateId, &ViaDesc ) )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The specified game state that should be used to perform a transition was not found.\n" ) );
        return false;
    
    } // End if to state not found

    // Is this a valid state type?
    if ( ViaDesc.type != StateType_Transition )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to perform a transition between two states using any state type other than 'transition'.\n" ) );
        return false;
    
    } // End if not a transition state

    // Store the parent of the state we're ending, we may have to attach to it
    pParentState = pFromState->mParentState;

    // Attach the new transition state to the necessary items
    pSelectedViaState                       = (cgAppTransitionState*)ViaDesc.state;
    pSelectedViaState->mParentState       = pParentState;

    // Update information in all states for pointer navigation
    pSelectedToState                        = ToDesc.state;
    pSelectedViaState->mFromState         = pFromState;
    pSelectedViaState->mToState           = pSelectedToState;
    pFromState->mOutgoingTransition       = pSelectedViaState;
    if ( pSelectedToState != CG_NULL )
        pSelectedToState->mIncomingTransition = pSelectedViaState;
    
    // If any parent is available, it will be attached as a child
    // otherwise it will become the root of our state history
    if ( pParentState )
        pParentState->mChildState = pSelectedViaState;
    else
        mStateHistory = pSelectedViaState;

    // The transition will become our newly active state 
    mActiveState = pSelectedViaState;

    // Begin the transition state
    if ( mActiveState->isBegun() == false )
        mActiveState->begin();
    
    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : transitionState () (Private, Overload)
/// <summary>
/// Simple state transition. This will simply end one state, and move
/// on to another.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppStateManager::transitionState( cgAppState * pFromState, const cgString & strToStateId )
{
    StateDesc     Desc;
    cgAppState * pParentState = CG_NULL, * pSelectedState = CG_NULL, * pState = CG_NULL;

    // Validate Requirements
    if ( !pFromState ) return false;

    // States can only be transitioned if they are in the current state history
    for ( pState = mStateHistory; pState != CG_NULL; pState = pState->mChildState )
    {
        if ( pState == pFromState ) break;
        
    } // Next in history list

    // Not in list?
    if ( pState == CG_NULL )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("You can only transition game states that currently exist in the state history.\n" ) );
        return false;

    } // End if from state not found

    // Select the state we're transitioning to
    if ( !getStateDesc( strToStateId, &Desc ) )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The specified game state to which we should transition was not found.\n" ) );
        return false;
    
    } // End if to state not found

    // Is this a valid state type?
    if ( Desc.type != StateType_Normal )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to transition to any state type other than 'normal' using simple transitioning.\n" ) );
        return false;
    
    } // End if not a normal state

    // Store the parent of the state we're ending, we may have to attach to it
    pParentState = pFromState->mParentState;

    // End the specified state
    pFromState->end();

    // Update necessary navigation pointers
    pSelectedState                        = Desc.state;
    pSelectedState->mParentState        = pParentState;
    pSelectedState->mIncomingTransition = CG_NULL;
    
    // If any parent is available, it will be attached as a child
    // otherwise it will become the root of our state history
    if ( pParentState )
        pParentState->mChildState = pSelectedState;
    else
        mStateHistory = pSelectedState;

    // This must become our newly active state since any below the previous
    // state have been ended
    mActiveState = pSelectedState->getTerminalState();

    // Begin the new state
    if ( mActiveState->isBegun() == false )
        mActiveState->begin();
    
    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : spawnChildState () (Private)
/// <summary>
/// A new state should be created at the bottom of the currently active
/// state list. Optionally, the parent should be put into a suspended
/// mode.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppStateManager::spawnChildState( cgAppState * pParentState, const cgString & strNewStateId, bool bSuspendParent /* = false */ )
{
    StateDesc     Desc;
    cgAppState * pSelectedState = CG_NULL, * pState = CG_NULL;

    // Nothing to do if there is no active state
    if ( mActiveState == CG_NULL ) return false;
    
    // Now retrieve the selected state of the correct type
    if ( !getStateDesc( strNewStateId, &Desc ) ) return false;
    pSelectedState = Desc.state;

    // Is this state already begun?
    if ( pSelectedState->isBegun() == true )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Attempting to spawn a new child game state which is already active.\n") );
        return false;
    
    } // End if state is in use
    
    // Attach this to the bottom of the restorable state list
    pSelectedState->mParentState  = pParentState;
    pParentState->mChildState     = pSelectedState;

    // This should now be the state that is currently active
    // if the parent was previously active.
    if ( mActiveState == pParentState )
        mActiveState = pSelectedState;

    // Begin the new state
    if ( pSelectedState->isBegun() == false )
        pSelectedState->begin();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getStateDesc()
/// <summary>
/// Retrieve the information about the state with the specified identifier.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppStateManager::getStateDesc( const cgString & strStateId, StateDesc * pDesc )
{
    StateMap::iterator itState;

    // Clear description for convenience
    if ( pDesc )
        memset( pDesc, 0, sizeof(StateDesc) );

    // First retrieve the details about the state specified, if none
    // was found this is clearly an invalid state identifier.
    itState = mRegisteredStates.find( strStateId );
    if ( itState == mRegisteredStates.end() )
        return false;

    // A registered state was found, retrieve it's description
    if ( pDesc )
        *pDesc = itState->second;

    // State found!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : stateEnded () (Private)
/// <summary>
/// A state has ended, we should update our internal data to reflect that
/// fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppStateManager::stateEnded( cgAppState * pState )
{
    // Paranoia check!
    if ( pState == CG_NULL ) return;

    // If this is the currently active state, and there is another restorable
    // state above it, switch to its parent
    if ( pState == mActiveState )
        mActiveState = pState->mParentState;

    // If this was at the root of the state history, we're all out of states
    if ( pState == mStateHistory )
        mStateHistory = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Starts the update process for all currently running states.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppStateManager::update( )
{
    if ( mStateHistory ) mStateHistory->update();
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Starts the render process for all currently running states.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppStateManager::render( )
{
    if ( mStateHistory ) mStateHistory->render();
}

///////////////////////////////////////////////////////////////////////////////
// cgAppState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAppState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState::cgAppState( const cgString & strStateId ) : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mStateManager        = CG_NULL;
    mStateId             = strStateId;
    mStateSuspended      = false;
    mChildState          = CG_NULL;
    mParentState         = CG_NULL;
    mOutgoingTransition  = CG_NULL;
    mIncomingTransition  = CG_NULL;
    mBegun               = false;
    mScriptObject        = CG_NULL;
    mResourceManager     = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgAppState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState::cgAppState( const cgString & stateId, const cgInputStream & scriptStream, cgResourceManager * resourceManager )  : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mStateManager        = CG_NULL;
    mStateId             = stateId;
    mStateSuspended      = false;
    mChildState          = CG_NULL;
    mParentState         = CG_NULL;
    mOutgoingTransition  = CG_NULL;
    mIncomingTransition  = CG_NULL;
    mBegun               = false;
    mScriptObject        = CG_NULL;
    mScriptStream        = scriptStream;
    mResourceManager     = resourceManager;
}

//-----------------------------------------------------------------------------
//  Name : ~cgAppState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState::~cgAppState()
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
void cgAppState::dispose( bool bDisposeBase )
{
    // End the state if it is begun.
    if ( mBegun == true )
        end();

    // Release any script objects we retain.
    if ( mScriptObject != CG_NULL )
        mScriptObject->release();
    mScriptObject = CG_NULL;

    // Release resources
    mScript.close();

    // Clear variables
    mStateManager         = CG_NULL;
    mStateSuspended       = false;
    mChildState           = CG_NULL;
    mParentState          = CG_NULL;
    mOutgoingTransition   = CG_NULL;
    mIncomingTransition   = CG_NULL;
    mResourceManager      = CG_NULL;
    mBegun                = false;

    // Clear any STL containers
    mEventActions.clear();

    // Pass through to base class if required.
    if ( bDisposeBase == true )
        cgReference::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AppState )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getManager ()
/// <summary>
/// Retrieve the application state manager to which this state belongs.
/// </summary>
//-----------------------------------------------------------------------------
cgAppStateManager * cgAppState::getManager( )
{
    return mStateManager;
}

//-----------------------------------------------------------------------------
//  Name : isActive ()
/// <summary>
/// Determine if this state is the currently active state (other states 
/// above this one may still be resident and restorable)
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppState::isActive( ) const
{
    if ( !mStateManager ) return false;
    return ( this == mStateManager->getActiveState() );
}

//-----------------------------------------------------------------------------
//  Name : isSuspended ()
/// <summary>
/// Determine if this game state is currently suspended.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppState::isSuspended( ) const
{
    return mStateSuspended;
}

//-----------------------------------------------------------------------------
//  Name : getStateId ()
/// <summary>
/// Retrieve the state identifier for this registered state.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgAppState::getStateId() const
{
    return mStateId;
}

//-----------------------------------------------------------------------------
//  Name : GetOutgiongTransition ()
/// <summary>
/// If this state is currently in the process of transitioning using a
/// more comprehensive transitioning process, this will return the current
/// object that is being used to transition this state out.
/// </summary>
//-----------------------------------------------------------------------------
cgAppTransitionState * cgAppState::getOutgoingTransition( )
{
    return mOutgoingTransition;
}

//-----------------------------------------------------------------------------
//  Name : getIncomingTransition ()
/// <summary>
/// If this state is currently in the process of transitioning using a
/// more comprehensive transitioning process, this will return the current
/// object that is being used to transition this state in.
/// </summary>
//-----------------------------------------------------------------------------
cgAppTransitionState * cgAppState::getIncomingTransition( )
{
    return mIncomingTransition;
}

//-----------------------------------------------------------------------------
//  Name : getRootState ()
/// <summary>
/// Irrespective of this state's position in the restorable history list,
/// retrieve the state currently at the root of the list.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState * cgAppState::getRootState( )
{
    cgAppState * pCurrentState = this;
    
    // Loop until there is no longer a parent
    for ( ; pCurrentState->mParentState != CG_NULL; )
        pCurrentState = pCurrentState->mParentState;
    
    // Return the state item
    return pCurrentState;
}

//-----------------------------------------------------------------------------
//  Name : getParentState ()
/// <summary>
/// Retrieve the state object that is immediately above this state in the
/// active state history list.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState * cgAppState::getParentState( )
{
    return mParentState;
}

//-----------------------------------------------------------------------------
//  Name : getTerminalState ()
/// <summary>
/// Irrespective of this state's position in the restorable history list,
/// retrieve the state currently at the "tail" of the list.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState * cgAppState::getTerminalState( )
{
    cgAppState * pCurrentState = this;

    // Loop until there is no longer a child
    for ( ; pCurrentState->mChildState != CG_NULL; )
        pCurrentState = pCurrentState->mChildState;

    // Return the state item.
    return pCurrentState;
}

//-----------------------------------------------------------------------------
//  Name : isScripted ()
/// <summary>
/// Returns true if the state is implemented through a script. Returns false
/// if this behavior has a native application side implementation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppState::isScripted( ) const
{
    return mScript.isValid();
}

//-----------------------------------------------------------------------------
//  Name : getScript ()
/// <summary>
/// Retrieve the handle the script containing the state logic if this is
/// a scripted application state (see cgAppState::isScripted()).
/// </summary>
//-----------------------------------------------------------------------------
const cgScriptHandle & cgAppState::getScript( ) const
{
    return mScript;
}

//-----------------------------------------------------------------------------
//  Name : getScriptObject ()
/// <summary>
/// Retrieve the script object referencing the specific instance of the
/// application state class associated with this state if scripted 
/// (see cgAppState::isScripted()).
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgAppState::getScriptObject( )
{
    return mScriptObject;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Allow the state to initialize (occurs during registration).
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppState::initialize()
{
    // Load state script if supplied.
    if ( mResourceManager && mScriptStream.sourceExists() )
    {
        // Attempt to load the state script.
        if ( !mResourceManager->loadScript( &mScript, mScriptStream, cgString::Empty, cgString::Empty, 0, cgDebugSource() ) )
            return false;

        // Create the scripted state object.
        cgScript * script = mScript.getResource(true);
        if ( script )
        {
            // Attempt to create the IScriptedAppState
            // object whose name matches the name of the file.
            cgString objectType = cgFileSystem::getFileName(mScriptStream.getName(), true);
            mScriptObject = script->createObjectInstance( objectType );

            // Collect handles to any supplied methods.
            if ( mScriptObject )
            {
                mScriptMethods.initialize = mScriptObject->getMethodHandle( _T("bool initialize( AppState@+ )") );
                mScriptMethods.begin      = mScriptObject->getMethodHandle( _T("bool begin()") );
                mScriptMethods.end        = mScriptObject->getMethodHandle( _T("void end()") );
                mScriptMethods.update     = mScriptObject->getMethodHandle( _T("void update()") );
                mScriptMethods.render     = mScriptObject->getMethodHandle( _T("void render()") );
                mScriptMethods.suspend    = mScriptObject->getMethodHandle( _T("void suspend()") );
                mScriptMethods.resume     = mScriptObject->getMethodHandle( _T("void resume()") );

                // Attempt to call the 'initialize' method (optional).
                if ( mScriptMethods.initialize )
                {
                    try
                    {
                        cgScriptArgument::Array scriptArgs;
                        scriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("AppState@+"), this ) );
                        mScriptObject->executeMethod( mScriptMethods.initialize, scriptArgs );
            
                    } // End try to execute

                    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
                    {
                        cgAppLog::write( cgAppLog::Error, _T("Failed to execute initialize() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
                        return false;

                    } // End catch exception
                
                } // End if defines method
                
            } // End if valid object

        } // End if valid.

    } // End if load script

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : begin () (Virtual)
/// <summary>
/// This signifies that the state has actually been selected and
/// activated by the state management system. This will generally be the
/// point at which any specific resources relevant for the execution of
/// this state will be built/loaded.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppState::begin( )
{
    bool bResult = true;
    
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.begin )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            bResult = *(bool*)mScriptObject->executeMethod( mScriptMethods.begin, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute begin() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return false;
        
        } // End catch exception

    } // End if valid

    if ( bResult )
        mBegun = true;
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : end () (Virtual)
/// <summary>
/// The state is no longer needed and should clean up and remove itself
/// including any child states.
/// Note : This MUST be called in the derived class' function if it overrides 
/// this method.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::end( )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.end )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            mScriptObject->executeMethod( mScriptMethods.end, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute end() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid

    // First recurse into any child state
    if ( mChildState )
        mChildState->end();
    mChildState = CG_NULL;

    // Notify manager that the state ended
    mStateManager->stateEnded( this );

    // Now detach this from parent
    if ( mParentState )
        mParentState->mChildState = CG_NULL;

    // We are no longer attached to anything (simply being stored in
    // the registered state list maintained by the manager).
    mStateSuspended       = false;
    mParentState          = CG_NULL;
    mOutgoingTransition   = CG_NULL;
    mIncomingTransition   = CG_NULL;
    mBegun                = false;
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Provides a basic implementation which passes the update on to any
/// child state.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::update( )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.update )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            mScriptObject->executeMethod( mScriptMethods.update, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute update() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid

    if ( mChildState )
        mChildState->update();
}

//-----------------------------------------------------------------------------
//  Name : render () (Virtual)
/// <summary>
/// Provides a basic implementation which passes the render on to any
/// child state.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::render( )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.render )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            mScriptObject->executeMethod( mScriptMethods.render, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute render() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid

    if ( mChildState )
        mChildState->render();
}

//-----------------------------------------------------------------------------
//  Name : suspend () (Virtual)
/// <summary>
/// The game state is to be put into a suspended mode. Derived classes
/// can determine if this is the case by calling isSuspended().
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::suspend( )
{
    mStateSuspended = true;

    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.suspend )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            mScriptObject->executeMethod( mScriptMethods.suspend, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute suspend() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : resume () (Virtual)
/// <summary>
/// If in the suspended state, this function will take the state object
/// out of a suspended mode.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::resume( )
{
    mStateSuspended = false;

    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.resume )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            mScriptObject->executeMethod( mScriptMethods.resume, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute resume() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : raiseEvent () (Virtual)
/// <summary>
/// Called by the derived class (or potentially a child class) whenever
/// an event has occured.
/// Note : States which are not currently active may not raise events.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::raiseEvent( const cgString & strEventName )
{
    raiseEvent( strEventName, false );
}

//-----------------------------------------------------------------------------
//  Name : raiseEvent () (Virtual)
/// <summary>
/// Called by the derived class (or potentially a child class) whenever
/// an event has occured.
/// Note : States which are not currently active may not raise events.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::raiseEvent( const cgString & strEventName, bool bForce )
{
    // States that are not active are blocked from raising events by default.
    // This helps to automatically prevent inactive states from messing
    // up the application flow.
    if ( bForce == false && this->isActive() == false )
        return;

    // Process the event!
    processEvent( strEventName );
}

//-----------------------------------------------------------------------------
//  Name : processEvent () (Private)
/// <summary>
/// Called in order to actually process any raised event. This function 
/// will potentially trigger a transition to a new state, or spawn a new
/// child state object.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::processEvent( const cgString & strEventName )
{
    EventActionMap::iterator itEventAction;

    // Any event action registered with this name?
    itEventAction = mEventActions.find( strEventName );
    if ( itEventAction == mEventActions.end() ) return;

    // An action was found, let's retrieve it
    EventActionDesc & Desc = itEventAction->second;

    // Now process based on the type of the event action
    switch ( Desc.actionType )
    {
        case ActionType_Transition:
            
            // This type will transition the state on which the event was raised, to a new state.
            if ( Desc.flags & ActionFlag_UseTransitionState )
            {
                // The application requested that we should use a transition 
                // state to perform a more comprehensive transition process.
                mStateManager->transitionState( this, Desc.toStateId, Desc.transitionStateId );

            } // End if use transition state
            else
            {
                // Just transition immediately
                mStateManager->transitionState( this, Desc.toStateId );

            } // End if immediate / simple transition

            break;

        case ActionType_TransitionRoot:
            
            // This type will transition the root state in the relationship list, to a new state
            // irrespective of which state object the event was raised on.
            if ( Desc.flags & ActionFlag_UseTransitionState )
            {
                // The application requested that we should use a transition 
                // state to perform a more comprehensive transition process.
                mStateManager->transitionState( getRootState(), Desc.toStateId, Desc.transitionStateId );

            } // End if use transition state
            else
            {
                // Just transition immediately
                mStateManager->transitionState( getRootState(), Desc.toStateId );

            } // End if immediate / simple transition

            break;

        case ActionType_SpawnChild:
            spawnChildState( Desc.toStateId, (Desc.flags & ActionFlag_SuspendParent) != 0 );
            break;

        case ActionType_EndState:

            // The state should simply end
            end();
            break;

        case ActionType_EndRoot:

            // The who system should simply end.
            mStateManager->stop();
            break;

        case ActionType_PassUp:
            
            // This action should simply pass the message on to a parent (if any)
            if ( mParentState ) mParentState->raiseEvent( strEventName, true );
            break;

        case ActionType_PassDown:
            
            // This action should simply pass the message on to a child (if any)
            if ( mChildState ) mChildState->raiseEvent( strEventName, true );
            break;
    
    } // End Switch actionType
}

//-----------------------------------------------------------------------------
//  Name : spawnChildState () (Virtual)
/// <summary>
/// Spawn a new child state and, if requested, also suspend this state.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppState::spawnChildState( const cgString & stateId, bool suspendParent )
{
    return mStateManager->spawnChildState( this, stateId, suspendParent );
}

//-----------------------------------------------------------------------------
//  Name : registerEventAction () (Virtual)
/// <summary>
/// This function should be called in order to add a specific action
/// that should be performed whenever the specified event is raised by
/// the game state object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppState::registerEventAction( const cgString & strEventName, const EventActionDesc & Desc )
{
    EventActionMap::iterator itEventAction;

    // Does an action with this matching event name already exist?
    itEventAction = mEventActions.find( strEventName );
    if ( itEventAction != mEventActions.end() ) return false;

    // If no action existed, store the action description specified
    mEventActions[ strEventName ] = Desc;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unregisterEventAction () (Virtual)
/// <summary>
/// Call this function in order to remove any previously registered event
/// action from this game state object.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppState::unregisterEventAction( const cgString & strEventName )
{
    EventActionMap::iterator itEventAction;

    // Does an action with this matching event name already exist?
    itEventAction = mEventActions.find( strEventName );
    if ( itEventAction == mEventActions.end() ) return;

    // If we found an action, remove it from the list
    mEventActions.erase( itEventAction );
}

///////////////////////////////////////////////////////////////////////////////
// cgAppTransitionState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAppTransitionState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAppTransitionState::cgAppTransitionState( const cgString & strStateId ) : cgAppState( strStateId )
{
    // Initialize variables to sensible defaults
    mFromState = CG_NULL;
    mToState   = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgAppTransitionState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAppTransitionState::~cgAppTransitionState()
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
void cgAppTransitionState::dispose( bool bDisposeBase )
{
    // Clear variables
    mFromState = CG_NULL;
    mToState   = CG_NULL;

    // Pass through to base class if required.
    if ( bDisposeBase == true )
        cgAppState::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppTransitionState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AppTransitionState )
        return true;

    // Supported by base?
    return cgAppState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getFromState ()
/// <summary>
/// Retrieve the state object that we are currently transitioning from.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState * cgAppTransitionState::getFromState( )
{
    return mFromState;
}

//-----------------------------------------------------------------------------
//  Name : getToState ()
/// <summary>
/// Retrieve the state object that we are currently transitioning to.
/// </summary>
//-----------------------------------------------------------------------------
cgAppState * cgAppTransitionState::getToState( )
{
    return mToState;
}

//-----------------------------------------------------------------------------
//  Name : transitionComplete ()
/// <summary>
/// Called in order to notify the system that the transition was
/// completed and the new state should take over.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppTransitionState::transitionComplete( )
{
    if ( mToState != CG_NULL )
        mStateManager->transitionState( this, mToState->getStateId() );
    else
        end();
}

//-----------------------------------------------------------------------------
//  Name : end () (Virtual)
/// <summary>
/// End the transition state, we've moved on to the target state.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppTransitionState::end( )
{
    // If we are being ended (for whatever reason) we must also 
    // end the state that we are transitioning from.
    if ( mFromState ) mFromState->end();
    
    // Clear required members, we've ended
    mFromState = CG_NULL;
    mToState   = CG_NULL;

    // Call base class implementation
    cgAppState::end();
}