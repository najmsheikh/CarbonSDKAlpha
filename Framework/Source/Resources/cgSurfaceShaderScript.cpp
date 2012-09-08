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
// Name : cgSurfaceShaderScript.cpp                                          //
//                                                                           //
// Desc : Contains a specialized (derived) version of the cgScript class     //
//        that provides support for features specific to surface shader      //
//        scripts.                                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSurfaceShaderScript Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgSurfaceShaderScript.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgConstantBuffer.h>
#include <Rendering/cgRenderDriver.h>
#include <Scripting/cgScriptEngine.h>

// Angelscript.
#include <angelscript.h>

///////////////////////////////////////////////////////////////////////////////
// cgSurfaceShaderScript Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSurfaceShaderScript () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderScript::cgSurfaceShaderScript( cgUInt32 nReferenceId, const cgInputStream & Stream, const cgString & strThisType, cgScriptEngine * pEngine ) : cgScript( nReferenceId, Stream, strThisType, pEngine )
{
    // Initialize variables to sensible defaults.
    mConstantBufferLinker = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSurfaceShaderScript () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderScript::~cgSurfaceShaderScript( )
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
void cgSurfaceShaderScript::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear processed description arrays.
    mConstantBuffers.clear();
    mConstantTypes.clear();
    mConstantTypeLUT.clear();
    mConstantBufferLUT.clear();
    mSamplerBlocks.clear();
    mSamplerBlockLUT.clear();
    mSamplerRegisters.clear();
    mShaderCallFunctions.clear();
    mShaderCallLUT.clear();

    // Dispose base.
    if ( bDisposeBase )
        cgScript::dispose( true );
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
bool cgSurfaceShaderScript::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SurfaceShaderScriptResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::loadResource( )
{
    // Call base class implementation.
    if ( !cgScript::loadResource( ) )
        return false;

    // Create and / or bind the common (shared) system exports class.
    // Take care: the 'getShaderInterface()' method adds a reference to the object before returning.
    // Also note that this method may also return NULL if the driver is still in the
    // process of initializing.
    cgRenderDriver * pDriver = mManager->getRenderDriver();
    cgScriptObject * pShaderInterface = pDriver->getShaderInterface();
    if ( pShaderInterface )
    {
        STRING_CONVERT;  // For string conversion macro

        // First convert the module name we're going to use to a standard ANSI
        // string if the application is currently using unicode.
        const cgChar * strModuleName = stringConvertT2CA( getResourceName().c_str() );
        
        // Retrieve the angelscript core engine
        asIScriptEngine * pInternalEngine = mScriptEngine->getInternalEngine();

        // Retrieve the module that owns the 'System' object variable.
        asIScriptModule * pModule = pInternalEngine->GetModule( strModuleName, asGM_CREATE_IF_NOT_EXISTS );

        // Get the address of the local script 'System' variable.
        asIScriptObject ** pObject = (asIScriptObject**)pModule->GetAddressOfGlobalVar( pModule->GetGlobalVarIndexByName( "System" ) );
        if ( pObject )
        {
            // Assign the shader interface to this global variable.
            *pObject = pShaderInterface->getInternalObject();

            // Increment the reference count of the /internal/ angelscript
            // object that represents the shared 'SystemExports' object.
            // A matching release() call will be made automatically by
            // angelscript when this script is unloaded.
            (*pObject)->AddRef();
        
        } // End if has 'System'

        // We're done with the shader interface.
        pShaderInterface->release();

    } // End if driver initialized

    // Now population is complete, create constant buffer linker that
    // will be responsible for building the appropriate hardware / register 
    // mappings and generating shader code declarations for the buffers.
    // Since this keeps a cache of common information, this 'linker' can
    // ideally be shared by all surface shader instances that utilize this
    // surface shader script.
    mConstantBufferLinker = cgConstantBufferLinker::createInstance( (!mConstantBuffers.empty()) ? &mConstantBuffers[0] : CG_NULL, mConstantBuffers.size(), 
                                                               (!mConstantTypes.empty()) ? &mConstantTypes[0] : CG_NULL, mConstantTypes.size() );

    // Ensure that constant mappings have been generated for all referenced 
    // constant buffers before leaving this method. cgConstantBuffer may
    // duplicate one (or more) of the description and type structures
    // which should, as a result, be fully pre-populated.
    mConstantBufferLinker->generateConstantMappings();


    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::unloadResource( )
{
    // Call base class implementation
    if ( !cgScript::unloadResource() )
        return false;

    // Destroy cbuffer linker (data will potentially now be invalid).
    delete mConstantBufferLinker;
    mConstantBufferLinker = CG_NULL;

    // Clear out member containers.
    mConstantTypes.clear();
    mConstantTypeLUT.clear();
    mConstantBuffers.clear();
    mConstantBufferLUT.clear();
    mSamplerBlocks.clear();
    mSamplerBlockLUT.clear();
    mSamplerRegisters.clear();
    mShaderCallFunctions.clear();
    mShaderCallLUT.clear();
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addShaderCallFuncDesc ()
/// <summary>
/// During pre-processing, any functions marked with the __shadercall modifier
/// are parsed and their declarations added to the script object with this 
/// method for later retrieval.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::addShaderCallFuncDesc( const cgShaderCallFunctionDesc & Desc )
{
    // Validate requirements
    cgAssertEx( Desc.name.empty() == false, "Cannot add a declaration for an unnamed shader call function." );

    // Shader call declaration must not already exist.
    HandleArrayMap::iterator itDesc;
    cgString strKey = (Desc.parentNamespace.empty()) ? Desc.name : Desc.parentNamespace + _T("::") + Desc.name;
    itDesc = mShaderCallLUT.find( strKey );
    if ( itDesc != mShaderCallLUT.end() )
    {
        bool bDuplicateDeclaration = true;
        for ( size_t i = 0; i < itDesc->second.size() && bDuplicateDeclaration; ++i )
        {
            // Can only be identical if parameter count is the same.
            const cgShaderCallFunctionDesc & TestDesc = mShaderCallFunctions[itDesc->second[i]];
            if ( Desc.parameters.size() != TestDesc.parameters.size() )
                bDuplicateDeclaration = false;

            // Process arguments (automatically skipped if bDuplicateDeclaration is false).
            for ( size_t j = 0; j < TestDesc.parameters.size() && bDuplicateDeclaration; ++j )
            {
                const cgShaderCallParameterDesc & Param = Desc.parameters[i];
                const cgShaderCallParameterDesc & TestParam = TestDesc.parameters[i];
                
                // Test to see if the parameters are the same.
                if ( Param.typeName != TestParam.typeName )
                    bDuplicateDeclaration = false;
                else if ( Param.modifier != TestParam.modifier )
                    bDuplicateDeclaration = false;

            } // Next parameter.

        } // Next function

        // Fail if a duplicate was found.
        if ( bDuplicateDeclaration )
            return -1;

    } // End if exists

    // Insert new shader call declaration
    cgInt32 nHandle = mShaderCallFunctions.size();
    mShaderCallFunctions.push_back( Desc );
    mShaderCallLUT[strKey].push_back( nHandle );
    return nHandle;
}

//-----------------------------------------------------------------------------
//  Name : addConstantBufferDesc ()
/// <summary>
/// During pre-processing, constant buffers are parsed and their declarations
/// added to the script object with this method for later retrieval.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::addConstantBufferDesc( const cgConstantBufferDesc & Desc )
{
    // Validate requirements
    cgAssertEx( Desc.name.empty() == false, "Cannot add a declaration for an unnamed surface shader constant buffer." );

    // Constant buffer declaration must not already exist.
    cgString strKey = (Desc.parentNamespace.empty()) ? Desc.name : Desc.parentNamespace + _T("::") + Desc.name;
    if ( mConstantBufferLUT.find( strKey ) != mConstantBufferLUT.end() )
        return -1;

    // Insert new constant buffer declaration
    cgInt32 nHandle = mConstantBuffers.size();
    mConstantBuffers.push_back( Desc );
    mConstantBufferLUT[strKey] = nHandle;
    return nHandle;
}

//-----------------------------------------------------------------------------
//  Name : addConstantTypeDesc ()
/// <summary>
/// During pre-processing, constant types are parsed and their
/// declarations added to the script object with this method for later 
/// retrieval.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::addConstantTypeDesc( const cgConstantTypeDesc & Desc )
{
    // Validate requirements
    cgAssertEx( Desc.name.empty() == false, "Cannot add a declaration for an unnamed surface shader constant type." );

    // Constant structure declaration must not already exist.
    cgString strKey = (Desc.parentNamespace.empty()) ? Desc.name : Desc.parentNamespace + _T("::") + Desc.name;
    if ( mConstantTypeLUT.find( strKey ) != mConstantTypeLUT.end() )
        return -1;

    // Insert new constant structure declaration
    cgInt32 nHandle = mConstantTypes.size();
    mConstantTypes.push_back( Desc );
    mConstantTypeLUT[strKey] = nHandle;
    return nHandle;
}

//-----------------------------------------------------------------------------
//  Name : addSamplerBlockDesc ()
/// <summary>
/// During pre-processing, sampler blocks are parsed and their declarations
/// added to the script object with this method for later retrieval.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::addSamplerBlockDesc( const cgSamplerBlockDesc & Desc )
{
    // Validate requirements
    cgAssertEx( Desc.name.empty() == false, "Cannot add a declaration for an unnamed surface shader sampler block." );

    // Sampler block declaration must not already exist.
    cgString strKey = (Desc.parentNamespace.empty()) ? Desc.name : Desc.parentNamespace + _T("::") + Desc.name;
    if ( mSamplerBlockLUT.find( strKey ) != mSamplerBlockLUT.end() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Duplicate sampler block declaration '%s' in surface shader script %s.\n"), strKey.c_str(), getResourceName().c_str() );
        return -1;
    
    } // End if duplicate

    // Insert new constant buffer declaration
    cgInt32 nHandle = mSamplerBlocks.size();
    mSamplerBlocks.push_back( Desc );
    mSamplerBlockLUT[strKey] = nHandle;

    // Insert all referenced samplers into the register lookup table.
    for ( size_t i = 0; i < Desc.samplers.size(); ++i )
    {
        const cgSamplerDesc & sDesc = Desc.samplers[i];
        strKey = (sDesc.parentNamespace.empty()) ? sDesc.name : sDesc.parentNamespace + _T("::") + sDesc.name;
        if ( mSamplerRegisters.find( strKey ) != mSamplerRegisters.end() )
        {
            cgAppLog::write( cgAppLog::Error, _T("Duplicate sampler declaration '%s' in surface shader script %s.\n"), strKey.c_str(), getResourceName().c_str() );
            return -1;
        
        } // End if duplicate
        mSamplerRegisters[strKey] = sDesc.registerIndex;
    
    } // Next Sampler
    
    // Return block handle
    return nHandle;
}

//-----------------------------------------------------------------------------
//  Name : findShaderCallFunc ()
/// <summary>
/// Retrieve the handle for the shader call function with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32Array cgSurfaceShaderScript::findShaderCallFunc( const cgString & strName ) const
{
    // Global namespace only.
    HandleArrayMap::const_iterator itDesc;
    if ( (itDesc = mShaderCallLUT.find( strName )) != mShaderCallLUT.end() )
        return itDesc->second;
    return cgInt32Array();
}

//-----------------------------------------------------------------------------
//  Name : findShaderCallFunc ()
/// <summary>
/// Retrieve the handles for the shader call function with the specified name.
/// Be aware that there may be more than one function with the same name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32Array cgSurfaceShaderScript::findShaderCallFunc( const cgString & strSourceNamespace, const cgString & strName ) const
{
    HandleArrayMap::const_iterator itDesc;

    // Attempt to find shader call function inside the source namespace first.
    if ( !strSourceNamespace.empty() )
    {
        cgString strKey = strSourceNamespace + _T("::") + strName;
        if ( (itDesc = mShaderCallLUT.find( strKey )) != mShaderCallLUT.end() )
            return itDesc->second;

    } // End if inside namespace

    // Otherwise try the global namespace.
    if ( (itDesc = mShaderCallLUT.find( strName )) != mShaderCallLUT.end() )
        return itDesc->second;

    // Not found
    return cgInt32Array();
}

//-----------------------------------------------------------------------------
//  Name : getShaderCallFuncDesc ()
/// <summary>
/// Retrieve description for the referenced shader call function as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getShaderCallFuncDesc( cgInt32 nHandle, cgShaderCallFunctionDesc & Desc ) const
{
    if ( nHandle < 0 || nHandle >= (cgInt32)mShaderCallFunctions.size() )
        return false;
    Desc = mShaderCallFunctions[nHandle];
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getShaderCallFuncDesc ()
/// <summary>
/// Retrieve descriptions for the referenced shader call function (by name) as 
/// it was parsed during script pre-processing. Be aware that there may be more
/// than one function with the same name.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getShaderCallFuncDesc( const cgString & strName, cgShaderCallFunctionDesc::Array & Descriptions ) const
{
    return getShaderCallFuncDesc( cgString::Empty, strName, Descriptions );
}

//-----------------------------------------------------------------------------
//  Name : getShaderCallFuncDesc ()
/// <summary>
/// Retrieve descriptions for the referenced shader call function as it
/// was parsed during script pre-processing. Be aware that there may be more
/// than one function with the same name.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getShaderCallFuncDesc( const cgString & strSourceNamespace, const cgString & strName, cgShaderCallFunctionDesc::Array & Descriptions ) const
{
    HandleArrayMap::const_iterator itDesc;

    // Attempt to find constant buffer inside the source namespace first.
    if ( !strSourceNamespace.empty() )
    {
        cgString strKey = strSourceNamespace + _T("::") + strName;
        if ( (itDesc = mShaderCallLUT.find( strKey )) != mShaderCallLUT.end() )
        {
            Descriptions.resize( itDesc->second.size() );
            for ( size_t i = 0; i < itDesc->second.size(); ++i )
                Descriptions[i] = mShaderCallFunctions[itDesc->second[i]];
            return true;

        } // End if found

    } // End if inside namespace

    // Otherwise try the global namespace.
    if ( (itDesc = mShaderCallLUT.find( strName )) != mShaderCallLUT.end() )
    {
        Descriptions.resize( itDesc->second.size() );
        for ( size_t i = 0; i < itDesc->second.size(); ++i )
            Descriptions[i] = mShaderCallFunctions[itDesc->second[i]];
        return true;

    } // End if found

    // Not found
    return false;
}

//-----------------------------------------------------------------------------
//  Name : findConstantBuffer ()
/// <summary>
/// Retrieve the handle for the constant buffer with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::findConstantBuffer( const cgString & strName ) const
{
    return findConstantBuffer( cgString::Empty, strName );
}

//-----------------------------------------------------------------------------
//  Name : findConstantBuffer ()
/// <summary>
/// Retrieve the handle for the constant buffer with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::findConstantBuffer( const cgString & strSourceNamespace, const cgString & strName ) const
{
    HandleMap::const_iterator itDesc;
    
    // Attempt to find constant buffer inside the source namespace first.
    if ( !strSourceNamespace.empty() )
    {
        cgString strKey = strSourceNamespace + _T("::") + strName;
        if ( (itDesc = mConstantBufferLUT.find( strKey )) != mConstantBufferLUT.end() )
            return itDesc->second;
        
    } // End if inside namespace

    // Otherwise try the global namespace.
    if ( (itDesc = mConstantBufferLUT.find( strName )) != mConstantBufferLUT.end() )
        return itDesc->second;
    
    // Not found
    return -1;
}

//-----------------------------------------------------------------------------
//  Name : findSamplerRegister ()
/// <summary>
/// Find the register index for the sampler with the specified name (minus
/// the implicit 's' prefix used to denote a sampler).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::findSamplerRegister( const cgString & strName ) const
{
    return findSamplerRegister( cgString::Empty, strName );
}

//-----------------------------------------------------------------------------
//  Name : findSamplerRegister ()
/// <summary>
/// Find the register index for the sampler with the specified name (minus
/// the implicit 's' prefix used to denote a sampler), in the current or global 
/// namespace.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::findSamplerRegister( const cgString & strSourceNamespace, const cgString & strName ) const
{
    HandleMap::const_iterator itDesc;
    
    // Attempt to find constant buffer inside the source namespace first.
    if ( !strSourceNamespace.empty() )
    {
        cgString strKey = strSourceNamespace + _T("::s") + strName;
        if ( (itDesc = mSamplerRegisters.find( strKey )) != mSamplerRegisters.end() )
            return itDesc->second;
        
    } // End if inside namespace

    // Otherwise try the global namespace.
    if ( (itDesc = mSamplerRegisters.find( _T("s") + strName )) != mSamplerRegisters.end() )
        return itDesc->second;
    
    // Not found
    return -1;
}

//-----------------------------------------------------------------------------
//  Name : getSamplerBlockDesc ()
/// <summary>
/// Retrieve description structure for the referenced sampler block as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getSamplerBlockDesc( cgInt32 nHandle, cgSamplerBlockDesc & Desc ) const
{
    if ( nHandle < 0 || nHandle >= (cgInt32)mSamplerBlocks.size() )
        return false;
    Desc = mSamplerBlocks[nHandle];
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSamplerBlockDesc ()
/// <summary>
/// Retrieve description structure for the referenced sampler block as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getSamplerBlockDesc( const cgString & strName, cgSamplerBlockDesc & Desc ) const
{
    return getSamplerBlockDesc( cgString::Empty, strName, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getSamplerBlockDesc ()
/// <summary>
/// Retrieve description structure for the referenced sampler block as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getSamplerBlockDesc( const cgString & strSourceNamespace, const cgString & strName, cgSamplerBlockDesc & Desc ) const
{
    HandleMap::const_iterator itDesc;
    
    // Attempt to find constant buffer inside the source namespace first.
    if ( !strSourceNamespace.empty() )
    {
        cgString strKey = strSourceNamespace + _T("::") + strName;
        if ( (itDesc = mSamplerBlockLUT.find( strKey )) != mSamplerBlockLUT.end() )
        {
            Desc = mSamplerBlocks[itDesc->second];
            return true;
        
        } // End if found
    
    } // End if inside namespace

    // Otherwise try the global namespace.
    if ( (itDesc = mSamplerBlockLUT.find( strName )) != mSamplerBlockLUT.end() )
    {
        Desc = mSamplerBlocks[itDesc->second];
        return true;
    
    } // End if found

    // Not found
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getConstantBufferDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant buffer as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getConstantBufferDesc( cgInt32 nHandle, cgConstantBufferDesc & Desc ) const
{
    if ( nHandle < 0 || nHandle >= (cgInt32)mConstantBuffers.size() )
        return false;
    Desc = mConstantBuffers[nHandle];
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getConstantBufferDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant buffer as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getConstantBufferDesc( const cgString & strName, cgConstantBufferDesc & Desc ) const
{
    return getConstantBufferDesc( cgString::Empty, strName, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getConstantBufferDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant buffer as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getConstantBufferDesc( const cgString & strSourceNamespace, const cgString & strName, cgConstantBufferDesc & Desc ) const
{
    HandleMap::const_iterator itDesc;
    
    // Attempt to find constant buffer inside the source namespace first.
    if ( !strSourceNamespace.empty() )
    {
        cgString strKey = strSourceNamespace + _T("::") + strName;
        if ( (itDesc = mConstantBufferLUT.find( strKey )) != mConstantBufferLUT.end() )
        {
            Desc = mConstantBuffers[itDesc->second];
            return true;
        
        } // End if found
    
    } // End if inside namespace

    // Otherwise try the global namespace.
    if ( (itDesc = mConstantBufferLUT.find( strName )) != mConstantBufferLUT.end() )
    {
        Desc = mConstantBuffers[itDesc->second];
        return true;
    
    } // End if found

    // Not found
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getConstantBuffers ()
/// <summary>
/// Retrieve a list of all constant buffer description structures parsed during 
/// script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
const cgSurfaceShaderScript::ConstBufferArray & cgSurfaceShaderScript::getConstantBuffers( ) const
{
    return mConstantBuffers;
}

//-----------------------------------------------------------------------------
//  Name : findConstantType ()
/// <summary>
/// Retrieve the handle for the constant type with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::findConstantType( const cgString & strName ) const
{
    return findConstantType( cgString::Empty, strName );
}

//-----------------------------------------------------------------------------
//  Name : findConstantType ()
/// <summary>
/// Retrieve the handle for the constant type with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShaderScript::findConstantType( const cgString & strSourceNamespace, const cgString & strName ) const
{
    HandleMap::const_iterator itDesc;
    
    // Attempt to find constant buffer inside the source namespace first.
    if ( !strSourceNamespace.empty() )
    {
        cgString strKey = strSourceNamespace + _T("::") + strName;
        if ( (itDesc = mConstantTypeLUT.find( strKey )) != mConstantTypeLUT.end() )
            return itDesc->second;
        
    } // End if inside namespace

    // Otherwise try the global namespace.
    if ( (itDesc = mConstantTypeLUT.find( strName )) != mConstantTypeLUT.end() )
        return itDesc->second;
    
    // Not found
    return cgConstantType::Unresolved;
}

//-----------------------------------------------------------------------------
//  Name : getConstantTypeDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant type as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getConstantTypeDesc( cgInt32 nHandle, cgConstantTypeDesc & Desc ) const
{
    if ( nHandle < 0 || nHandle >= (cgInt32)mConstantTypes.size() )
        return false;
    Desc = mConstantTypes[nHandle];
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getConstantTypeDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant type as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getConstantTypeDesc( const cgString & strName, cgConstantTypeDesc & Desc ) const
{
    return getConstantTypeDesc( cgString::Empty, strName, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getConstantTypeDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant type as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShaderScript::getConstantTypeDesc( const cgString & strSourceNamespace, const cgString & strName, cgConstantTypeDesc & Desc ) const
{
    HandleMap::const_iterator itDesc;
    
    // Attempt to find constant structure inside the source namespace first.
    if ( !strSourceNamespace.empty() )
    {
        cgString strKey = strSourceNamespace + _T("::") + strName;
        if ( (itDesc = mConstantTypeLUT.find( strKey )) != mConstantTypeLUT.end() )
        {
            Desc = mConstantTypes[itDesc->second];
            return true;
        
        } // End if found
    
    } // End if inside namespace

    // Otherwise try the global namespace.
    if ( (itDesc = mConstantTypeLUT.find( strName )) != mConstantTypeLUT.end() )
    {
        Desc = mConstantTypes[itDesc->second];
        return true;
    
    } // End if found

    // Not found
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getConstantTypes ()
/// <summary>
/// Retrieve a list of all constant type description structures parsed during 
/// script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
const cgSurfaceShaderScript::ConstTypeArray & cgSurfaceShaderScript::getConstantTypes( ) const
{
    return mConstantTypes;
}

//-----------------------------------------------------------------------------
//  Name : getConstantBufferLinker ()
/// <summary>
/// Retrieve the common / shared constant linker that provides hardare register 
/// mappings and shader code declarations for the constant buffers declared 
/// within the script.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBufferLinker * cgSurfaceShaderScript::getConstantBufferLinker( ) const
{
    return mConstantBufferLinker;
}