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
// Name : cgBase.cpp                                                         //
//                                                                           //
// Desc : Base / common source module for the CGE library. Defines           //
//        common types and housekeeping routines (such as initialization /   //
//        destruction)                                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBase Module Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgApplication.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>        // Warning: Portability
#undef WIN32_LEAN_AND_MEAN
#include <gdiplusenums.h>   // Warning: Portability
#include <gdiplustypes.h>   // Warning: Portability
#include <gdiplusinit.h>    // Warning: Portability
#include <conio.h>

// Override new handler.
#if _MSC_VER >= 1400 // VC2005
#include <new>
#endif

// Singleton / Managers
#include <Resources/cgResourceManager.h>
#include <Rendering/cgRenderDriver.h>
#include <States/cgAppStateManager.h>
#include <Physics/cgPhysicsEngine.h>
#include <Interface/cgUIManager.h>
#include <World/cgWorld.h>
#include <Input/cgInputDriver.h>
#include <Audio/cgAudioDriver.h>
#include <System/cgTimer.h>
#include <System/cgProfiler.h>
#include <System/cgThreading.h>

// Object types
#include <World/Objects/cgParticleEmitterObject.h>
#include <World/Objects/cgDirectionalLight.h>
#include <World/Objects/cgHemisphereLight.h>
#include <World/Objects/cgProjectorLight.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgDummyObject.h>
#include <World/Objects/cgGroupObject.h>
#include <World/Objects/cgActor.h>
#include <World/Objects/cgBoneObject.h>
#include <World/Objects/cgSkinObject.h>
#include <World/Objects/cgMeshObject.h>
#include <World/Objects/cgPointLight.h>
#include <World/Objects/cgSpotLight.h>
#include <World/Objects/cgTargetObject.h>
#include <World/Objects/cgSplineObject.h>
#include <World/Objects/cgKinematicControllerJointObject.h>
#include <World/Objects/cgFixedAxisJointObject.h>
#include <World/Objects/cgHingeJointObject.h>
#include <World/Objects/cgSoundEmitterObject.h>

// Object sub-element types
#include <World/Objects/Elements/cgBoxCollisionShapeElement.h>
#include <World/Objects/Elements/cgSphereCollisionShapeElement.h>
#include <World/Objects/Elements/cgCylinderCollisionShapeElement.h>
#include <World/Objects/Elements/cgCapsuleCollisionShapeElement.h>
#include <World/Objects/Elements/cgConeCollisionShapeElement.h>
#include <World/Objects/Elements/cgHullCollisionShapeElement.h>
#include <World/Objects/Elements/cgAnimationSetElement.h>

// Scene element types
#include <World/Elements/cgNavigationMeshElement.h>
#include <World/Elements/cgSkyElement.h>

// Physics Controllers
#include <Physics/Controllers/cgCharacterController.h>

// Scripting
#include <Scripting/cgScriptEngine.h>
#include <Scripting/Packages/Core.h>

//-----------------------------------------------------------------------------
// Module Local Function Declarations
//-----------------------------------------------------------------------------
void NewHandler();

//-----------------------------------------------------------------------------
// Module Local Variables
//-----------------------------------------------------------------------------
namespace
{
    CGEConfig           EngineConfig;                       // Application supplied engine configuration.
    bool                bSwitchThreadSupported  = false;    // Is the NT based 'SwitchToThread' function available on this system?
    bool                bDisableResourceCheck   = false;    // Check for outstanding resources?
    new_handler         pPreviousNewHandler     = CG_NULL;  // The previous 'new_handler' that was installed (if any).
    int                 nOriginalFPUPrecision   = 0;        // The original FPU precision flags prior to enabling double precision.
    bool                bDoubleFPUPrecision     = false;    // Double FPU precision is currently selected?
    ULONG_PTR           GDIPlusToken            = 0;        // Token used to initialize and shut down GDI+ (cgWinImage)
    cgCriticalSection * pToDoSection            = CG_NULL;  // Critical section designed to protect the cgToDoHandler() internals for cross thread access.
    cgCriticalSection * pAssertSection          = CG_NULL;  // Critical section designed to protect the cgAssertHandler() internals for cross thread access.

} // End Unnamed Namespace

//-----------------------------------------------------------------------------
//  Name : NewHandler()
/// <summary>
/// If ever memory fails to be allocated, this function will be called.
/// </summary>
//-----------------------------------------------------------------------------
void NewHandler()
{
    static LPCTSTR strOOMMessage = _T("The system has run low on memory resources and, as a result, allocations have begun to fail.\nThis is a fatal error and the application must now exit.\n");
    static LPCTSTR strOOMHeader  = _T("Allocation Failed    ");

    // Replace new_handler with the original to prevent it triggering again.
    std::set_new_handler( pPreviousNewHandler );
    
    // Disable resource checking on exit. Firstly, this will consume more memory than is
    // necessary at this point. Secondly, this will incorrectly report outstanding resources
    // because we are bypassing the natural shutdown of the application. Most resources 
    // should still however be freed succesfully.
    bDisableResourceCheck = true;

    // Clean up the engine
    cgEngineCleanup();
    
    // Write error to log file
    cgAppLog::write( cgAppLog::Error, strOOMMessage );

    // Display the error message now some memory has been freed up
    ::MessageBox( CG_NULL, strOOMMessage, strOOMHeader, MB_ICONSTOP );

    // exit the application immediately.
    _set_abort_behavior( 0, _WRITE_ABORT_MSG );
    abort();
    // exit(0)
}

//-----------------------------------------------------------------------------
//  Name : cgEngineInit()
/// <summary>
/// Initialize the engine ready for use.
/// </summary>
//-----------------------------------------------------------------------------
bool cgEngineInit( const CGEConfig & Config, cgLogOutput * pOutput /* = CG_NULL */ )
{
    bool bResult = true;

    #if _MSC_VER >= 1400 // VC2005
        
        // Set our engine new_handler to handle out of memory situations
        pPreviousNewHandler = std::set_new_handler(NewHandler);
    
    #endif // VC2005

    // Record selected engine configuration.
    EngineConfig = Config;

    // ToDo: kernel32.dll for 64 bit too?
    // Determine if specific functions are available on this system
    HMODULE hLibrary = ::LoadLibrary( _T("kernel32.dll") );
    if ( hLibrary != CG_NULL )
    {
        // Kernel32.dll contains the 'SwitchToThread' export?
        if ( ::GetProcAddress( hLibrary, "SwitchToThread" ) != CG_NULL )
            bSwitchThreadSupported = true;

        // Clean up
        ::FreeLibrary( hLibrary );
    
    } // End if library loaded

    // High priority application required? (i.e. full screen game, rather than embedded in application / editor)
    if ( EngineConfig.highPriority )
    {
        // Set up our process and thread priorities
        SetPriorityClass( GetModuleHandle(CG_NULL), REALTIME_PRIORITY_CLASS);

        // ToDo: FixMe - Setting thread priority to highest can cause lag in directinput unless VSYNCing (doesn't seem to happen on Vista).
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
        //SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );

    } // End if high priority requested
    
    // Force the thread on which timing processes are run
    // onto a single processor / core.
    /*DWORD_PTR dwProcessAffinityMask = 0;
    DWORD_PTR dwSystemAffinityMask = 0;
    HANDLE hCurrentProcess = GetCurrentProcess();
    if( GetProcessAffinityMask( hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ) != 0 && dwProcessAffinityMask )
    {
        // Find the lowest processor that our process is allowed to run against
        DWORD_PTR dwAffinityMask = ( dwProcessAffinityMask & ((~dwProcessAffinityMask) + 1 ) );

        // Set this as the processor that our thread must always run against
        // This must be a subset of the process affinity mask
        HANDLE hCurrentThread = GetCurrentThread();
        if ( INVALID_HANDLE_VALUE != hCurrentThread )
        {
            SetThreadAffinityMask( hCurrentThread, dwAffinityMask );
            CloseHandle( hCurrentThread );
        
        } // End if thread valid
    
    } // End if process valid
    CloseHandle( hCurrentProcess );*/

    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup( &GDIPlusToken, &gdiplusStartupInput, CG_NULL);

    // Initialize base critical sections.
    pToDoSection = cgCriticalSection::createInstance();
    pAssertSection = cgCriticalSection::createInstance();

    // Begin the logging process
    if ( pOutput != CG_NULL ) cgAppLog::registerOutput( pOutput );
    cgAppLog::beginLogging( );

    // Write debug info
    cgAppLog::write( cgAppLog::Info, _T("cgEngineInit()\n") );

    // Create application singletons
    cgTimer::createSingleton();
    cgProfiler::createSingleton();
    cgRenderDriver::createSingleton();
    cgResourceManager::createSingleton();
    cgInputDriver::createSingleton();
    cgAudioDriver::createSingleton();
    cgWorld::createSingleton();
    cgPhysicsEngine::createSingleton();
    cgAppStateManager::createSingleton();
    cgScriptEngine::createSingleton();
    cgUIManager::createSingleton();

    // Add system required path protocols and packages (if they exist)
    if ( !cgFileSystem::pathProtocolDefined( _T("sys") ) )
        cgFileSystem::addPathProtocol( _T("sys"), _T("System/") );
    if ( !cgFileSystem::pathProtocolDefined( _T("currentdir") ) )
        cgFileSystem::addPathProtocol( _T("currentdir"), _T("") );
    if ( !cgFileSystem::pathProtocolDefined( _T("appdir") ) )
        cgFileSystem::addPathProtocol( _T("appdir"), cgFileSystem::getAppDirectory() );
    cgFileSystem::addPackage( _T("System.pkg") );

    // Register standard physics controller types
    cgPhysicsController::registerType( _T("Core::PhysicsControllers::Character"), cgCharacterController::allocate );

    // Register standard object types.
    cgWorldObject::registerType( RTID_CameraObject          , _T("Camera")           , cgCameraObject::allocateNew, cgCameraObject::allocateClone, cgCameraNode::allocateNew, cgCameraNode::allocateClone );
    cgWorldObject::registerType( RTID_MeshObject            , _T("Mesh")             , cgMeshObject::allocateNew, cgMeshObject::allocateClone, cgMeshNode::allocateNew, cgMeshNode::allocateClone );
    cgWorldObject::registerType( RTID_SkinObject            , _T("Skin")             , cgSkinObject::allocateNew, cgSkinObject::allocateClone, cgSkinNode::allocateNew, cgSkinNode::allocateClone );
    cgWorldObject::registerType( RTID_DummyObject           , _T("Dummy")            , cgDummyObject::allocateNew, cgDummyObject::allocateClone, cgDummyNode::allocateNew, cgDummyNode::allocateClone );
    cgWorldObject::registerType( RTID_GroupObject           , _T("Group")            , cgGroupObject::allocateNew, CG_NULL, cgGroupNode::allocateNew, CG_NULL );
    cgWorldObject::registerType( RTID_ActorObject           , _T("Actor")            , cgActorObject::allocateNew, cgActorObject::allocateClone, cgActorNode::allocateNew, cgActorNode::allocateClone );
    cgWorldObject::registerType( RTID_SpotLightObject       , _T("Spot Light")       , cgSpotLightObject::allocateNew, cgSpotLightObject::allocateClone, cgSpotLightNode::allocateNew, cgSpotLightNode::allocateClone );
    cgWorldObject::registerType( RTID_PointLightObject      , _T("Point Light")      , cgPointLightObject::allocateNew, cgPointLightObject::allocateClone, cgPointLightNode::allocateNew, cgPointLightNode::allocateClone );
    cgWorldObject::registerType( RTID_DirectionalLightObject, _T("Directional Light"), cgDirectionalLightObject::allocateNew, cgDirectionalLightObject::allocateClone, cgDirectionalLightNode::allocateNew, cgDirectionalLightNode::allocateClone );
    cgWorldObject::registerType( RTID_HemisphereLightObject , _T("Hemisphere Light") , cgHemisphereLightObject::allocateNew, cgHemisphereLightObject::allocateClone, cgHemisphereLightNode::allocateNew, cgHemisphereLightNode::allocateClone );
    cgWorldObject::registerType( RTID_ProjectorLightObject  , _T("Projector Light")  , cgProjectorLightObject::allocateNew, cgProjectorLightObject::allocateClone, cgProjectorLightNode::allocateNew, cgProjectorLightNode::allocateClone );
    cgWorldObject::registerType( RTID_TargetObject          , _T("Target")           , cgTargetObject::allocateNew, cgTargetObject::allocateClone, cgTargetNode::allocateNew, cgTargetNode::allocateClone );
    cgWorldObject::registerType( RTID_BoneObject            , _T("Bone")             , cgBoneObject::allocateNew, cgBoneObject::allocateClone, cgBoneNode::allocateNew, cgBoneNode::allocateClone );
    cgWorldObject::registerType( RTID_ParticleEmitterObject , _T("Particle Emitter") , cgParticleEmitterObject::allocateNew, cgParticleEmitterObject::allocateClone, cgParticleEmitterNode::allocateNew, cgParticleEmitterNode::allocateClone );
    cgWorldObject::registerType( RTID_SplineObject          , _T("Spline")           , cgSplineObject::allocateNew, cgSplineObject::allocateClone, cgSplineNode::allocateNew, cgSplineNode::allocateClone );
    cgWorldObject::registerType( RTID_SoundEmitterObject    , _T("Sound Emitter")    , cgSoundEmitterObject::allocateNew, cgSoundEmitterObject::allocateClone, cgSoundEmitterNode::allocateNew, cgSoundEmitterNode::allocateClone );
    
    // Register physics joint object types
    cgWorldObject::registerType( RTID_KinematicControllerJointObject, _T("Kinematic Controller Joint"), cgKinematicControllerJointObject::allocateNew, cgKinematicControllerJointObject::allocateClone, cgKinematicControllerJointNode::allocateNew, cgKinematicControllerJointNode::allocateClone );
    cgWorldObject::registerType( RTID_FixedAxisJointObject          , _T("Fixed Axis Joint")          , cgFixedAxisJointObject::allocateNew, cgFixedAxisJointObject::allocateClone, cgFixedAxisJointNode::allocateNew, cgFixedAxisJointNode::allocateClone );
    cgWorldObject::registerType( RTID_HingeJointObject              , _T("Hinge Joint")               , cgHingeJointObject::allocateNew, cgHingeJointObject::allocateClone, cgHingeJointNode::allocateNew, cgHingeJointNode::allocateClone );

    // Register standard object sub-element types.
    cgObjectSubElement::registerType( RTID_BoxCollisionShapeElement     , _T("Box Collision Shape")        , cgBoxCollisionShapeElement::allocateNew, cgBoxCollisionShapeElement::allocateClone );
    cgObjectSubElement::registerType( RTID_SphereCollisionShapeElement  , _T("Sphere Collision Shape")     , cgSphereCollisionShapeElement::allocateNew, cgSphereCollisionShapeElement::allocateClone );
    cgObjectSubElement::registerType( RTID_CylinderCollisionShapeElement, _T("Cylinder Collision Shape")   , cgCylinderCollisionShapeElement::allocateNew, cgCylinderCollisionShapeElement::allocateClone );
    cgObjectSubElement::registerType( RTID_CapsuleCollisionShapeElement , _T("Capsule Collision Shape")    , cgCapsuleCollisionShapeElement::allocateNew, cgCapsuleCollisionShapeElement::allocateClone );
    cgObjectSubElement::registerType( RTID_ConeCollisionShapeElement    , _T("Cone Collision Shape")       , cgConeCollisionShapeElement::allocateNew, cgConeCollisionShapeElement::allocateClone );
    cgObjectSubElement::registerType( RTID_HullCollisionShapeElement    , _T("Convex Hull Collision Shape"), cgHullCollisionShapeElement::allocateNew, cgHullCollisionShapeElement::allocateClone );
    cgObjectSubElement::registerType( RTID_AnimationSetElement          , _T("Animation Set")              , cgAnimationSetElement::allocateNew, cgAnimationSetElement::allocateClone );

    // Register standard scene element types
    cgSceneElement::registerType( RTID_NavigationMeshElement, _T("Navigation Mesh"), cgNavigationMeshElement::allocateNew );
    cgSceneElement::registerType( RTID_SkyElement           , _T("Sky")            , cgSkyElement::allocateNew );

    // Initialize the main script engine
    cgScriptEngine * pEngine = cgScriptEngine::getInstance();
    if ( bResult = pEngine->initialize() )
    {
        // Bind all standard (core) script packages with the main scripting engine.
        if ( bResult = pEngine->declarePackage( new cgScriptPackages::Core::Package(), true ) )
            bResult = pEngine->bindPackage( cgScriptPackages::Core::Package::getNamespace(), true );

    } // End if success

    // Binding step failed?
    if ( !bResult )
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : cgEngineCleanup()
/// <summary>
/// Cleanup the engine after use.
/// </summary>
//-----------------------------------------------------------------------------
void cgEngineCleanup()
{
    cgRenderDriver    * pRenderDriver    = cgRenderDriver::getInstance();
    cgAudioDriver     * pAudioDriver     = cgAudioDriver::getInstance();
    cgResourceManager * pResourceManager = cgResourceManager::getInstance();
    cgScriptEngine    * pScriptEngine    = cgScriptEngine::getInstance();
    cgAppStateManager * pAppStates       = cgAppStateManager::getInstance();

    // Shutdown any pooled threads (trigger their completion events)
    cgThreadPool::shutdown( true );

    // Make sure all application states are ended and destroyed.
    if ( pAppStates )
        pAppStates->dispose( false );

    // Shut down the application reference manager and messaging system.
    cgReferenceManager::shutdown( );

    // Remove any log output items that have scripts as a target.
    for ( size_t i = 0; i < cgAppLog::getOutputCount(); )
    {
        cgLogOutput * pOutput = cgAppLog::getOutput(i);
        if ( dynamic_cast<cgLogOutputScript*>(pOutput) )
            cgAppLog::removeOutput( pOutput, true );
        else
            ++i;

    } // Next Output

    // Ensure all resident scripts are shut down and any objects
    // they still maintain references to are released.
    if ( pScriptEngine )
        pScriptEngine->unloadScripts();

    // Destroy initial application singletons (for guaranteed destruction ordering, and cleanup PRIOR to application finish)
    // Only the resource manager, audio and render drivers should remain in-tact in order for us to perform the resource check.
    cgAppStateManager::destroySingleton();
    cgUIManager::destroySingleton();
    cgWorld::destroySingleton();
    cgPhysicsEngine::destroySingleton();
    cgInputDriver::destroySingleton();
    
    // Main render driver should release any resources it still holds
    if ( pRenderDriver != CG_NULL )
        pRenderDriver->releaseOwnedResources();

    // Main audio driver should release any resources it still holds
    if ( pAudioDriver != CG_NULL )
        pAudioDriver->releaseOwnedResources();

    // Empty out the resource manager's garbage list and close references 
    // to any "AlwaysResident" resources.
    if ( pResourceManager != CG_NULL )
    {
        pResourceManager->emptyGarbage();
        pResourceManager->releaseOwnedResources();

    } // End if exists

    // Shut down script engine last
    cgScriptEngine::destroySingleton();

    // Output resources which remain allocated (effectively leaked).
    if ( pResourceManager != CG_NULL && bDisableResourceCheck == false )
    {
        cgAppLog::write( cgAppLog::Debug, _T("Checking for outstanding resources.\n") );
        pResourceManager->debugResources();
    
    } // End if perform check

    // Release remaining singletons
    cgResourceManager::destroySingleton();
    cgAudioDriver::destroySingleton();
    cgRenderDriver::destroySingleton();
    cgTimer::destroySingleton();
    cgProfiler::destroySingleton();

    // Write final line
    cgAppLog::write( cgAppLog::Info, _T("cgEngineCleanup()\n") );

    // Dump string literal->cgString construction report.
    #ifdef STRING_CONSTRUCT_REPORT
        cgString::outputStringConstructReport();
        _getch();
    #endif

    // End the logging process
    cgAppLog::endLogging( );

    // Shutdown GDI+
    if ( GDIPlusToken != 0 )
        GdiplusShutdown(GDIPlusToken);

    // Close base critical sections.
    delete pToDoSection;
    delete pAssertSection;
    pToDoSection = CG_NULL;
    pAssertSection = CG_NULL;

    #if _MSC_VER >= 1400 // VC2005
        
        // Replace new_handler with the original
        std::set_new_handler( pPreviousNewHandler );
    
    #endif // VC2005
}

//-----------------------------------------------------------------------------
//  Name : cgGetEngineConfig()
/// <summary>
/// Retrieve the configuration values that were specified when initializing
/// the CGE library.
/// </summary>
//-----------------------------------------------------------------------------
const CGEConfig & cgGetEngineConfig()
{
    return EngineConfig;
}

//-----------------------------------------------------------------------------
//  Name : cgEngineYield()
/// <summary>
/// Yield to any engine threads that need to process.
/// </summary>
//-----------------------------------------------------------------------------
void cgEngineYield()
{
    // Yield to loading threads in the least impact manner possible
    if ( bSwitchThreadSupported == true )
        SwitchToThread();
    else
        Sleep( 1 );
}

//-----------------------------------------------------------------------------
//  Name : cgFPUDoublePrecision()
/// <summary>
/// Allow the FPU to run in double precision mode until the next call
/// to cgFPURestorePrecision().
/// </summary>
//-----------------------------------------------------------------------------
void cgFPUDoublePrecision()
{
    if ( bDoubleFPUPrecision == true )
        return;

    // Record the current FPU precision details
    nOriginalFPUPrecision = _controlfp( 0, 0 );

    // Enable double precision.
    _controlfp( _CW_DEFAULT, 0xfffff );
    bDoubleFPUPrecision = true;
}

//-----------------------------------------------------------------------------
//  Name : cgFPUDoublePrecision()
/// <summary>
/// Restore the FPU's previous precision prior to any call to
/// cgFPUDoublePrecision().
/// </summary>
//-----------------------------------------------------------------------------
void cgFPURestorePrecision()
{
    if ( bDoubleFPUPrecision == false )
        return;

    // Restore previous precision
    _controlfp( nOriginalFPUPrecision, 0xfffff );
    bDoubleFPUPrecision = false;
}

//-----------------------------------------------------------------------------
//  Name : cgAssertHandler()
/// <summary>
/// Used by the 'cgAssert' macro to output critical (debug) runtime errors
/// and to halt the execution of the application.
/// </summary>
//-----------------------------------------------------------------------------
void cgAssertHandler( wchar_t * _Expr, wchar_t * _File, int _Line, wchar_t * _Msg )
{
    // Protect internals for cross thread access.
    pAssertSection->enter();

    // Get the name of the module
    cgTChar lpszModule[513];
    GetModuleFileName( NULL, lpszModule, 512 );

    // Retrieve additional information
    STRING_CONVERT;
    cgString strAdditional( _Msg ? stringConvertW2CT(_Msg) : cgString::Empty );
    if ( _Msg ) strAdditional.append( _T(" ") );

    // Build the log file string layout
    cgString strFormatLog( 
    _T("Assertion failed in '%s' at '%s(%i)'. %s")
    _T("The following expression was not found to be true during evaluation: %s\n")
    );

    // Output the message
    cgString strSourceFile( cgFileSystem::getFileName(stringConvertW2CT(_File)) );
    cgString strModuleFile( cgFileSystem::getFileName(lpszModule) );
    cgAppLog::write( cgAppLog::Error, strFormatLog.c_str(), strModuleFile.c_str(), strSourceFile.c_str(), _Line, strAdditional.c_str(), stringConvertW2CT(_Expr) );

    // Build the message box string layout.
    cgString strFormat( 
    _T("Assertion failed in '%s' at '%s(%i)'. %s\n")
    _T("\n")
    _T("The following expression was not found to be true during evaluation:\n\n%s\n")
    _T("\n")
#if defined(_DEBUG)
    _T("(Press Abort to exit the application)\n")
    _T("(Press Retry to debug the application - JIT must be enabled)\n")
    _T("(Press Ignore to resume execution - undefined behavior may result)")
#else
    _T("(Press OK to exit the application)")
#endif
    );

    // Generate the string
    cgString strMessage( cgString::format( strFormat.c_str(), strModuleFile.c_str(), strSourceFile.c_str(), _Line, strAdditional.c_str(), stringConvertW2CT(_Expr) ) );

    // Display the message box
#if defined(_DEBUG)
    int nResult = MessageBox( NULL, strMessage.c_str(), _T("Carbon Game Engine"), MB_TASKMODAL | MB_ABORTRETRYIGNORE | MB_ICONERROR );
    
    // Handle user selection
    switch ( nResult )
    {
        case IDIGNORE:
            // Allow application to continue.
            return;
        
        case IDRETRY:
            // Debug application
            __debugbreak();
            break;

        default:
            // Includes IDABORT case. Kill application (do not display second 'abnormal abort' message).
            _set_abort_behavior( 0, _WRITE_ABORT_MSG );
            abort();
            return;
    
    } // End switch nResult

#else
    MessageBox( NULL, strMessage.c_str(), _T("Carbon Game Engine"), MB_OK );

    // Kill program execution (do not display second 'abnormal abort' message).
    _set_abort_behavior( 0, _WRITE_ABORT_MSG );
    abort();
#endif

    // Leave critical section
    pAssertSection->exit();
}

//-----------------------------------------------------------------------------
//  Name : cgToDoHandler()
/// <summary>
/// Used by the 'cgToDoAssert' macro to output critical ToDo messages at
/// runtime and to halt the execution of the application.
/// </summary>
//-----------------------------------------------------------------------------
void cgToDoHandler( wchar_t * _Process, wchar_t * _File, int _Line, wchar_t * _Note )
{
    // Protect internals for cross thread access.
    pToDoSection->enter();

    // Set used to prevent duplicate notes.
    static std::set<cgString> IgnoredMessages;

    // Get the name of the module
    cgTChar lpszModule[513];
    GetModuleFileName( NULL, lpszModule, 512 );

    // Retrieve additional information
    STRING_CONVERT;
    cgString strMessage( _Note ? stringConvertW2CT(_Note) : _T("No additional information was supplied.") );

    // Build the log file string layout
    cgString strFormatLog;
    if ( _Process )
    {
        strFormatLog = 
        _T("In progress feature encountered in '%s' at '%s(%i)' relating to development ")
        _T("process %s. %s");
        
    } // End if process provided
    else
    {
        strFormatLog = 
        _T("In progress feature encountered in '%s' at '%s(%i)'. %s");
        
    } // End if process provided

    // Construct final message.
    cgString strOutputLog;
    cgString strSourceFile( cgFileSystem::getFileName(stringConvertW2CT(_File)) );
    cgString strModuleFile( cgFileSystem::getFileName(lpszModule) );
    if ( _Process )
        strOutputLog = cgString::format( strFormatLog.c_str(), strModuleFile.c_str(), strSourceFile.c_str(), _Line, _Process, strMessage.c_str() );
    else
        strOutputLog = cgString::format( strFormatLog.c_str(), strModuleFile.c_str(), strSourceFile.c_str(), _Line, strMessage.c_str() );

    // Is this a duplicate?
    if ( IgnoredMessages.find( strOutputLog ) != IgnoredMessages.end() )
        return;
    
    // Output the message
    cgAppLog::write( cgAppLog::Warning, _T("%s\n"), strOutputLog.c_str() );

    // Build the message box string layout.
    cgString strFormat;
    if ( _Process )
    {
        strFormat =
        _T("In progress feature encountered in '%s' at '%s(%i)' relating to development\n")
        _T("process %s.\n")
        _T("\n")
        _T("%s\n")
        _T("\n")
        #if defined(_DEBUG)
        _T("(Press Abort to exit the application)\n")
        _T("(Press Retry to debug the application - JIT must be enabled)\n")
        _T("(Press Ignore to resume execution - undefined behavior may result)");
        #else
        _T("(Press OK to exit the application)");
        #endif
    } // End if _Process
    else
    {
        strFormat =
        _T("In progress feature encountered in '%s' at '%s(%i)'.\n")
        _T("\n")
        _T("%s\n")
        _T("\n")
        #if defined(_DEBUG)
        _T("(Press Abort to exit the application)\n")
        _T("(Press Retry to debug the application - JIT must be enabled)\n")
        _T("(Press Ignore to resume execution - undefined behavior may result)");
        #else
        _T("(Press OK to exit the application)");
        #endif
    } // End if !_Process

    // Generate the string
    cgString strOutput;
    if ( _Process )
        strOutput = cgString::format( strFormat.c_str(), strModuleFile.c_str(), strSourceFile.c_str(), _Line, _Process, strMessage.c_str() );
    else
        strOutput = cgString::format( strFormat.c_str(), strModuleFile.c_str(), strSourceFile.c_str(), _Line, strMessage.c_str() );

    // Display the message box
#if defined(_DEBUG)
    int nResult = MessageBox( NULL, strOutput.c_str(), _T("Carbon Game Engine"), MB_TASKMODAL | MB_ABORTRETRYIGNORE | MB_ICONWARNING );
    
    // Handle user selection
    switch ( nResult )
    {
        case IDIGNORE:
            // Ignore future warnings and allow application to continue.
            IgnoredMessages.insert( strOutputLog );
            return;
        
        case IDRETRY:
            // Debug application
            __debugbreak();
            break;

        default:
            // Includes IDABORT case. Kill application (do not display second 'abnormal abort' message).
            _set_abort_behavior( 0, _WRITE_ABORT_MSG );
            abort();
            return;
    
    } // End switch nResult

#else
    MessageBox( NULL, strOutput.c_str(), _T("Carbon Game Engine"), MB_OK );

    // Kill program execution (do not display second 'abnormal abort' message).
    _set_abort_behavior( 0, _WRITE_ABORT_MSG );
    abort();
#endif

    // Leave critical section
    pToDoSection->exit();
}

//-----------------------------------------------------------------------------
//  Name : cgGetSandboxMode()
/// <summary>
/// Determine if the engine is currently configured to run in either of two
/// so-called 'sandbox' modes ('Enabled' for full editing and serialization,
/// or 'Preview' for in-editor execution) or the standard runtime mode
/// ('Disabled').
/// </summary>
//-----------------------------------------------------------------------------
cgSandboxMode::Base cgGetSandboxMode( )
{
    return EngineConfig.sandboxMode;
}

//-----------------------------------------------------------------------------
//  Name : cgSetSandboxMode()
/// <summary>
/// Set the current mode in which the engine is running. Valid sandbox modes
/// accepted by this function are 'Enabled' or 'Preview'. You may not switch
/// the sandbox mode from or to 'Disabled' in any case.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSetSandboxMode( cgSandboxMode::Base mode )
{
    if ( EngineConfig.sandboxMode == cgSandboxMode::Disabled )
        return false;
    EngineConfig.sandboxMode = mode;
    return true;
}