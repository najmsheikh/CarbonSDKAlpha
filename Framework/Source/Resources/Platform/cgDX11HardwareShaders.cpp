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
// Name : cgHardwareShaders.cpp                                              //
//                                                                           //
// Desc : Contains classes responsible for loading and managing hardware     //
//        vertex, pixel and geometry shader resource data.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11VertexShader Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX11HardwareShaders.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX11RenderDriver.h>
#include <System/cgStringUtility.h>
#include <System/cgExceptions.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3DCompiler.h>

///////////////////////////////////////////////////////////////////////////////
// Module Local Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : PrettyCompileErrorsDX11()
/// <summary>
/// Convert D3DX effect compile error into something a little more in line with
/// the SG log output.
/// </summary>
//-----------------------------------------------------------------------------
void PrettyCompileErrorsDX11( const cgChar * inputError, const cgString & sourceFile, const cgString & currentDirectory )
{
    STRING_CONVERT;

    // Sometimes, compiler can spit out many duplicate errors. This
    // set stores unique output errors in an attempt to filter them.
    std::set<cgString> duplicateFilter;

    // Convert to required character format
    cgString message( stringConvertA2CT(inputError) );
    
    // Strip out 'ID3DXEffectCompiler: ' notices.
    message.replace( _T("ID3DXEffectCompiler: "), cgString::Empty );

    // Tokenize the error into multiple lines for output
    cgStringArray lines;
    cgStringUtility::tokenize( message, lines, _T("\n") );

    // Process each line
    for ( size_t i = 0; i < lines.size(); ++i )
    {
        // Skip empty lines.
        message = lines[i];
        if ( message.empty() == true )
            continue;

        // Skip any pointless errors.
        if ( message == _T("Compilation failed") )
            continue;

        // Skip any duplicates.
        if ( duplicateFilter.find( message ) != duplicateFilter.end() )
            continue;
        duplicateFilter.insert( message );

        // ToDo: 9999 - Come up with a better way to find the file name.
        //              Directories with parenthesis in them (i.e. "MyDir(4)\"
        //              could really screw this up.
        // Is a path identifier? (hacky way to find, but close enough).
        bool pathFound = false;
        cgString::size_type pathBegin = message.find( _T('(') );
        if ( pathBegin != cgString::npos )
        {
            cgString::size_type commaOffset = message.find( _T(','), pathBegin );
            cgString::size_type pathEnd     = message.find( _T(')'), pathBegin );
            if ( commaOffset != cgString::npos && pathEnd != cgString::npos && (pathBegin < commaOffset && commaOffset < pathEnd) )
            {
                // Retrieve the file identifier.
                cgString fileName = message.substr( 0, pathBegin );

                // Generate the full path to this file.
                fileName = cgFileSystem::getAbsolutePath( fileName, currentDirectory );

                // Retrieve the line / column identifier
                cgString errorLocation = message.substr( pathBegin + 1, pathEnd - pathBegin );

                // Tokenize line / column
                cgStringArray tokens;
                cgInt lineIndex = -1, columnIndex = -1;
                if ( cgStringUtility::tokenize( errorLocation, tokens, _T(",") ) == true && tokens.size() == 2 )
                {
                    cgStringParser( tokens[0] ) >> lineIndex;
                    cgStringParser( tokens[1] ) >> columnIndex;
                
                } // End if 2 tokens
                else
                {
                    cgStringParser( errorLocation ) >> lineIndex;

                } // End if !2 tokens

                // Replace error with our own link
                message = message.substr( pathEnd + 1 );
                message = cgString::format( _T("[[a href=\"carboncode://%s?line=%i\"]]%s(%i)[[/a]]%s"), fileName.c_str(), lineIndex, cgFileSystem::getFileName( fileName ).c_str(), lineIndex, message.c_str() );
                pathFound = true;

            } // End if found '(line,col)'

        } // End if found open paren

        // Error or warning?
        cgUInt32 messageLevel = 0;
        if ( message.find( _T(": warning X") ) != cgString::npos )
            messageLevel = cgAppLog::Warning;
        else 
            messageLevel = cgAppLog::Error;
        
        // Output error
        if ( !pathFound )
            cgAppLog::write( messageLevel, _T("[[a href=\"carboncode://%s\"]]%s(0)[[/a]]: %s\n"), sourceFile.c_str(), cgFileSystem::getFileName(sourceFile).c_str(), message.c_str() );
        else
            cgAppLog::write( messageLevel, _T("%s\n"), message.c_str() );

    } // Next Line
}

///////////////////////////////////////////////////////////////////////////////
// cgDX11VertexShader Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11VertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11VertexShader::cgDX11VertexShader( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint ) : 
    cgVertexShader( referenceId, sourceFile, entryPoint )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mIASignatureId    = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : cgDX11VertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11VertexShader::cgDX11VertexShader( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled ) : 
    cgVertexShader( referenceId, sourceFile, compiled )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mIASignatureId    = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : cgDX11VertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11VertexShader::cgDX11VertexShader( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier ) : 
    cgVertexShader( referenceId, sourceCode, entryPoint, identifier )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mIASignatureId    = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : cgDX11VertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11VertexShader::cgDX11VertexShader( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier ) : 
    cgVertexShader( referenceId, byteCode, identifier )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mIASignatureId    = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11VertexShader () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11VertexShader::~cgDX11VertexShader( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX11VertexShader::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( disposeBase == true )
        cgVertexShader::dispose( true );
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
bool cgDX11VertexShader::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11VertexShaderResource )
        return true;

    // Supported by base?
    return cgVertexShader::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DShader ()
/// <summary>
/// Retrieve the internal D3D11 vertex shader object.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11VertexShader * cgDX11VertexShader::getD3DShader( ) const
{
    // Add reference, we're returning a pointer
    if ( mShader )
        mShader->AddRef();

    // Return the resource item
    return mShader;
}

//-----------------------------------------------------------------------------
//  Name : getIAInputSignatureId ()
/// <summary>
/// Get the unique identifier associated with the input signature for this
/// shader permutation. This identifier is registered with the DX11 render
/// driver so that it can identify all shaders and shader permutations that
/// share the same input signature.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX11VertexShader::getIAInputSignatureId( ) const
{
    return mIASignatureId;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11VertexShader::deviceLost()
{
    // 7777 - Is this even necessary any more?
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseShader();

    // Resource data is now lost
    setResourceLost( true );
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11VertexShader::deviceRestored()
{
    // 7777 - Is this even necessary any more?
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createShader();
}

//-----------------------------------------------------------------------------
//  Name : releaseShader ()
/// <summary>
/// Release the internal shader.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11VertexShader::releaseShader( )
{
    // We should release our underlying resource(s)
    if ( mShader )
        mShader->Release();
    mShader = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createShader ()
/// <summary>
/// Create the internal shader.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11VertexShader::createShader( )
{
    STRING_CONVERT;
    ID3D10Blob * shaderData = CG_NULL;
    ID3D10Blob * compileLog = CG_NULL;
    HRESULT      result     = D3DERR_INVALIDCALL;

    // Already loaded?
    if ( mShader )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for shader creation (required DX11 class driver)
    ID3D11Device * device = CG_NULL;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(device = driver->getD3DDevice()) )
        return false;

    // Catch exceptions
    try
    {
        // If the shader code is not currently compiled, we must do so first.
        if ( mByteCode.empty() )
        {
            // From file, or source code?
            if ( mSourceCode.empty() == false )
            {
                 // Optionally turn on debugging for shaders
                cgUInt32 shaderFlags = 0;
                if ( driver->getConfig().debugVShader )
                    shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;

                // Clean up the source code in debug mode
                if ( driver->getConfig().debugVShader )
                    mSourceCode = cgStringUtility::beautifyCode( mSourceCode );

                // Compile from source to a memory buffer.
                LPCSTR shaderProfile = "vs_4_0"; // ToDo: 9999 - Get max version (configurable by INI?)
                result = D3DX11CompileFromMemory( stringConvertT2CA(mSourceCode.c_str()), mSourceCode.size(), 
                                                  stringConvertT2CA(mIdentifier.sourceFiles[0].name.c_str()), CG_NULL, CG_NULL,
                                                  stringConvertT2CA(mSourceEntryPoint.c_str()), shaderProfile, shaderFlags, 0, CG_NULL,
                                                  &shaderData, &compileLog, CG_NULL );

                // Errors / Warnings?
                if ( FAILED( result ) )
                {
                    // ToDo: 9999 - Need to correctly map errors back to the original script!
                    if ( compileLog != CG_NULL )
                        PrettyCompileErrorsDX11( (LPCSTR)compileLog->GetBufferPointer(), _T("TODO") /*sourceFile*/, cgString::Empty /*cgFileSystem::GetDirectoryName( sourceFile )*/ );
                    else
                        cgAppLog::write( cgAppLog::Error, _T("An unexpected parsing or compilation error occurred whilst attempting to compile '%s'.\n"), getResourceName().c_str() );
                    throw cgExceptions::ResultException( result, _T("D3DX11CompileFromMemory"), cgDebugSource() );
                    
                } // End if failed
                else if ( compileLog != CG_NULL )
                {
                    // May still have spit out warnings.
                    // ToDo: 9999 - Need to correctly map errors back to the original script!
                    // ToDo: 9999 - Optional warnings!
                    //PrettyCompileErrorsDX11( (LPCSTR)compileLog->GetBufferPointer(), _T("TODO") /*sourceFile*/, cgString::Empty /*cgFileSystem::GetDirectoryName( sourceFile )*/ );

                } // End if success

                // Shader compiled OK. Extract byte code.
                mByteCode.resize( shaderData->GetBufferSize() );
                memcpy( &mByteCode[0], shaderData->GetBufferPointer(), mByteCode.size() );
                mCompiled = true;

                // Free up memory by releasing source code. Any subsequent calls to 'CreateResource()' 
                // will now just load the shader resource from the byte code itself.
                mSourceCode = cgString::Empty;

                // Release any outstanding D3D resources.
                if ( shaderData ) shaderData->Release();
                if ( compileLog ) compileLog->Release();
                shaderData = CG_NULL;
                compileLog = CG_NULL;

                // Write the shader to the on-disk cache if it doesn't already exist.
                if ( !driver->getConfig().debugVShader && !mIdentifier.sourceFiles.empty() )
                {
                    // First build the unique hashed file name for this shader permutation 
                    // based on the source files, the parameter data and the shader identifier / name.
                    // If this file does not exist, write the identifier and the byte array to file.
                    cgString cacheFile = _T("sys://Cache/Shaders/") + mIdentifier.generateHashName() + _T(".vs4");
                    cacheFile = cgFileSystem::resolveFileLocation( cacheFile );
                    if ( !cgFileSystem::fileExists( cacheFile ) )
                        writePermutationCache( cacheFile );
                    
                } // End if write cache

            } // End if code
            else
            {
                // Is this a compiled source file?
                if ( mCompiled )
                    readCompiledShader( mSourceFile );
                else
                    cgToDoAssert( "Effect Overhaul", "Compile from source file!" );

            } // End if file

        } // End if !compiled

        // If we have compiled byte code, just load it.
        if ( mCompiled && !mByteCode.empty() )
        {
            if ( FAILED(result = device->CreateVertexShader( (void*)&mByteCode[0], mByteCode.size(), CG_NULL, &mShader ) ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to create hardware device compatible vertex shader for '%s'.\n"), getResourceName().c_str() );
                throw cgExceptions::ResultException( result, _T("ID3D11Device::CreateVertexShader"), cgDebugSource() );
                
            } // End if failed

        } // End if compiled

        // Register this permutation's unique input signature with the render driver.
        mIASignatureId = driver->registerIAInputSignature( mIdentifier.inputSignatureHash );

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
        if ( shaderData ) shaderData->Release();
        if ( compileLog ) compileLog->Release();
        device->Release();
        return false;

    } // End Catch

    // Release the D3D device, we're all done with it
    device->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11VertexShader::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the shader
    if ( !createShader() )
        return false;

    // We're now loaded
    mResourceLoaded = true;
    mResourceLost   = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11VertexShader::unloadResource( )
{
    // Release the shader
    releaseShader();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX11PixelShader Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11PixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11PixelShader::cgDX11PixelShader( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint ) :
    cgPixelShader( referenceId, sourceFile, entryPoint )
{
    // Initialize variables to sensible defaults
    mShader = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgDX11PixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11PixelShader::cgDX11PixelShader( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled ) :
    cgPixelShader( referenceId, sourceFile, compiled )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mIASignatureId    = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : cgDX11PixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11PixelShader::cgDX11PixelShader( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier ) :
    cgPixelShader( referenceId, sourceCode, entryPoint, identifier )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mIASignatureId    = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : cgDX11PixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11PixelShader::cgDX11PixelShader( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier ) : 
    cgPixelShader( referenceId, byteCode, identifier )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mIASignatureId    = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11PixelShader () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11PixelShader::~cgDX11PixelShader( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX11PixelShader::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( disposeBase == true )
        cgPixelShader::dispose( true );
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
bool cgDX11PixelShader::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11PixelShaderResource )
        return true;

    // Supported by base?
    return cgPixelShader::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DShader ()
/// <summary>
/// Retrieve the internal D3D11 pixel shader object.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11PixelShader * cgDX11PixelShader::getD3DShader( ) const
{
    // Add reference, we're returning a pointer
    if ( mShader )
        mShader->AddRef();

    // Return the resource item
    return mShader;
}

//-----------------------------------------------------------------------------
//  Name : getIAInputSignatureId ()
/// <summary>
/// Get the unique identifier  associated with the input signature for this
/// shader permutation. This identifier is registered with the DX11 render
/// driver so that it can identify all shaders and shader permutations that
/// share the same input signature.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX11PixelShader::getIAInputSignatureId( ) const
{
    return mIASignatureId;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11PixelShader::deviceLost()
{
    // 7777 - Is this even necessary any more?
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseShader();

    // Resource data is now lost
    setResourceLost( true );
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11PixelShader::deviceRestored()
{
    // 7777 - Is this even necessary any more?
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createShader();
}

//-----------------------------------------------------------------------------
//  Name : releaseShader ()
/// <summary>
/// Release the internal shader.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11PixelShader::releaseShader( )
{
    // We should release our underlying resource(s)
    if ( mShader )
        mShader->Release();
    mShader = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createShader ()
/// <summary>
/// Create the internal shader.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11PixelShader::createShader( )
{
    STRING_CONVERT;
    ID3D10Blob* shaderData = CG_NULL;
    ID3D10Blob* compileLog = CG_NULL;
    HRESULT     result     = D3DERR_INVALIDCALL;

    // Already loaded?
    if ( mShader )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for shader creation (required DX11 class driver)
    ID3D11Device * device = CG_NULL;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(device = driver->getD3DDevice()) )
        return false;

    // Catch exceptions
    try
    {
        // If the shader code is not currently compiled, we must do so first.
        if ( mByteCode.empty() )
        {
            // From file, or source code?
            if ( mSourceCode.empty() == false )
            {
                // Optionally turn on debugging for shaders
                cgUInt32 shaderFlags = 0;
                if ( driver->getConfig().debugPShader )
                    shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;

                // Clean up the source code in debug mode
                if ( driver->getConfig().debugPShader )
                    mSourceCode = cgStringUtility::beautifyCode( mSourceCode );

                // Compile from source to a memory buffer.
                LPCSTR shaderProfile = "ps_4_0"; // ToDo: 9999 - Get max version (configurable by INI?)
                result = D3DX11CompileFromMemory( stringConvertT2CA(mSourceCode.c_str()), mSourceCode.size(), 
                                                  stringConvertT2CA(mIdentifier.sourceFiles[0].name.c_str()), CG_NULL, CG_NULL,
                                                  stringConvertT2CA(mSourceEntryPoint.c_str()), shaderProfile, shaderFlags, 0, CG_NULL,
                                                  &shaderData, &compileLog, CG_NULL );

                // Errors / Warnings?
                if ( FAILED( result ) )
                {
                    // ToDo: 9999 - Need to correctly map errors back to the original script!
                    if ( compileLog != CG_NULL )
                        PrettyCompileErrorsDX11( (LPCSTR)compileLog->GetBufferPointer(), _T("TODO") /*sourceFile*/, cgString::Empty /*cgFileSystem::GetDirectoryName( sourceFile )*/ );
                    else
                        cgAppLog::write( cgAppLog::Error, _T("An unexpected parsing or compilation error occurred whilst attempting to compile '%s'.\n"), getResourceName().c_str() );
                    throw cgExceptions::ResultException( result, _T("D3DX11CompileFromMemory"), cgDebugSource() );
                    
                } // End if failed
                else if ( compileLog != CG_NULL )
                {
                    // May still have spit out warnings.
                    // ToDo: 9999 - Need to correctly map errors back to the original script!
                    // ToDo: 9999 - Optional warnings!
                    //PrettyCompileErrorsDX11( (LPCSTR)compileLog->GetBufferPointer(), _T("TODO") /*sourceFile*/, cgString::Empty /*cgFileSystem::GetDirectoryName( sourceFile )*/ );

                } // End if success

                // Shader compiled OK. Extract byte code.
                mByteCode.resize( shaderData->GetBufferSize() );
                memcpy( &mByteCode[0], shaderData->GetBufferPointer(), mByteCode.size() );
                mCompiled = true;

                // Free up memory by releasing source code. Any subsequent calls to 'CreateResource()' 
                // will now just load the shader resource from the byte code itself.
                mSourceCode = cgString::Empty;

                // Release any outstanding D3D resources.
                if ( shaderData ) shaderData->Release();
                if ( compileLog ) compileLog->Release();
                shaderData = CG_NULL;
                compileLog = CG_NULL;

                // Write the shader to the on-disk cache if it doesn't already exist.
                if ( !driver->getConfig().debugPShader && !mIdentifier.sourceFiles.empty() )
                {
                    // First build the unique hashed file name for this shader permutation 
                    // based on the source files, the parameter data and the shader identifier / name.
                    // If this file does not exist, write the identifier and the byte array to file.
                    cgString cacheFile = _T("sys://Cache/Shaders/") + mIdentifier.generateHashName() + _T(".ps4");
                    cacheFile = cgFileSystem::resolveFileLocation( cacheFile );
                    if ( !cgFileSystem::fileExists( cacheFile ) )
                        writePermutationCache( cacheFile );
                    
                } // End if write cache

            } // End if code
            else
            {
                // Is this a compiled source file?
                if ( mCompiled )
                    readCompiledShader( mSourceFile );
                else
                    cgToDoAssert( "Effect Overhaul", "Compile from source file!" );

            } // End if file

        } // End if !compiled

        // If we have compiled byte code, just load it.
        if ( mCompiled && !mByteCode.empty() )
        {
            if ( FAILED(result = device->CreatePixelShader( (void*)&mByteCode[0], mByteCode.size(), CG_NULL, &mShader ) ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to create hardware device compatible pixel shader for '%s'.\n"), getResourceName().c_str() );
                throw cgExceptions::ResultException( result, _T("ID3D11Device::CreatePixelShader"), cgDebugSource() );
                
            } // End if failed

        } // End if compiled

        // Register this permutation's unique input signature with the render driver.
        mIASignatureId = driver->registerIAInputSignature( mIdentifier.inputSignatureHash );

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
        if ( shaderData ) shaderData->Release();
        if ( compileLog ) compileLog->Release();
        device->Release();
        return false;

    } // End Catch

    // Release the D3D device, we're all done with it
    device->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11PixelShader::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the shader
    if ( !createShader() )
        return false;

    // We're now loaded
    mResourceLoaded = true;
    mResourceLost   = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11PixelShader::unloadResource( )
{
    // Release the shader
    releaseShader();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

#endif // CGE_DX11_RENDER_SUPPORT