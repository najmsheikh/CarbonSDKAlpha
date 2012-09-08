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
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9VertexShader Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX9HardwareShaders.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <System/cgStringUtility.h>
#include <System/cgExceptions.h>

///////////////////////////////////////////////////////////////////////////////
// Module Local Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : PrettyCompileErrorsDX9()
/// <summary>
/// Convert D3DX effect compile error into something a little more in line with
/// the SG log output.
/// </summary>
//-----------------------------------------------------------------------------
void PrettyCompileErrorsDX9( const cgChar * lpszError, const cgString & strSourceFile, const cgString & strCurrentDir )
{
    STRING_CONVERT;

    // Sometimes, compiler can spit out many duplicate errors. This
    // set stores unique output errors in an attempt to filter them.
    std::set<cgString> DuplicateFilter;

    // Convert to required character format
    cgString strError( stringConvertA2CT(lpszError) );
    
    // Strip out 'ID3DXEffectCompiler: ' notices.
    strError.replace( _T("ID3DXEffectCompiler: "), cgString::Empty );

    // Tokenize the error into multiple lines for output
    cgStringArray aLines;
    cgStringUtility::tokenize( strError, aLines, _T("\n") );

    // Process each line
    for ( size_t i = 0; i < aLines.size(); ++i )
    {
        // Skip empty lines.
        strError = aLines[i];
        if ( strError.empty() == true )
            continue;

        // Skip any pointless errors.
        if ( strError == _T("Compilation failed") )
            continue;

        // Skip any duplicates.
        if ( DuplicateFilter.find( strError ) != DuplicateFilter.end() )
            continue;
        DuplicateFilter.insert( strError );

        // ToDo: 9999 - Come up with a better way to find the file name.
        //              Directories with parenthesis in them (i.e. "MyDir(4)\"
        //              could really screw this up.
        // Is a path identifier? (hacky way to find, but close enough).
        bool bPathFound = false;
        cgString::size_type nPosBegin = strError.find( _T('(') );
        if ( nPosBegin != cgString::npos )
        {
            cgString::size_type nComma    = strError.find( _T(','), nPosBegin );
            cgString::size_type nPosEnd   = strError.find( _T(')'), nPosBegin );
            if ( nComma != cgString::npos && nPosEnd != cgString::npos && (nPosBegin < nComma && nComma < nPosEnd) )
            {
                // Retrieve the file identifier.
                cgString strFile = strError.substr( 0, nPosBegin );

                // Generate the full path to this file.
                strFile = cgFileSystem::getAbsolutePath( strFile, strCurrentDir );

                // Retrieve the line / column identifier
                cgString strLineCol = strError.substr( nPosBegin + 1, nPosEnd - nPosBegin );

                // Tokenize line / column
                cgStringArray aTokens;
                cgInt nLine = -1, nCol = -1;
                if ( cgStringUtility::tokenize( strLineCol, aTokens, _T(",") ) == true && aTokens.size() == 2 )
                {
                    cgStringParser( aTokens[0] ) >> nLine;
                    cgStringParser( aTokens[1] ) >> nCol;
                
                } // End if 2 tokens
                else
                {
                    cgStringParser( strLineCol ) >> nLine;

                } // End if !2 tokens

                // Replace error with our own link
                strError = strError.substr( nPosEnd + 1 );
                strError = cgString::format( _T("[[a href=\"speqtrecode://%s?line=%i\"]]%s(%i)[[/a]]%s"), strFile.c_str(), nLine, cgFileSystem::getFileName( strFile ).c_str(), nLine, strError.c_str() );
                bPathFound = true;

            } // End if found '(line,col)'

        } // End if found open paren

        // Error or warning?
        cgUInt32 nLevel = 0;
        if ( strError.find( _T(": warning X") ) != cgString::npos )
            nLevel = cgAppLog::Warning;
        else 
            nLevel = cgAppLog::Error;
        
        // Output error
        if ( bPathFound == false )
        {
            cgAppLog::write( nLevel, _T("[[a href=\"speqtrecode://%s\"]]%s(0)[[/a]]: %s\n"), strSourceFile.c_str(), cgFileSystem::getFileName( strSourceFile).c_str(), strError.c_str() );
        } // End if no path
        else
            cgAppLog::write( nLevel, _T("%s\n"), strError.c_str() );

    } // Next Line
}

///////////////////////////////////////////////////////////////////////////////
// cgDX9VertexShader Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9VertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9VertexShader::cgDX9VertexShader( cgUInt32 nReferenceId, const cgInputStream & SourceFile, const cgString & strEntryPoint ) : 
    cgVertexShader( nReferenceId, SourceFile, strEntryPoint )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgDX9VertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9VertexShader::cgDX9VertexShader( cgUInt32 nReferenceId, const cgInputStream & SourceFile, bool bCompiled ) : 
    cgVertexShader( nReferenceId, SourceFile, bCompiled )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgDX9VertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9VertexShader::cgDX9VertexShader( cgUInt32 nReferenceId, const cgString & strSource, const cgString & strEntryPoint, const cgShaderIdentifier * pIdentifier ) : 
    cgVertexShader( nReferenceId, strSource, strEntryPoint, pIdentifier )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgDX9VertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9VertexShader::cgDX9VertexShader( cgUInt32 nReferenceId, const cgByteArray & aByteCode, const cgShaderIdentifier * pIdentifier ) : 
    cgVertexShader( nReferenceId, aByteCode, pIdentifier )
{
    // Initialize variables to sensible defaults
    mShader           = CG_NULL;
    mConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9VertexShader () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9VertexShader::~cgDX9VertexShader( )
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
void cgDX9VertexShader::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( bDisposeBase == true )
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
bool cgDX9VertexShader::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9VertexShaderResource )
        return true;

    // Supported by base?
    return cgVertexShader::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DShader ()
/// <summary>
/// Retrieve the internal D3D9 vertex shader object.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DVertexShader9 * cgDX9VertexShader::getD3DShader( ) const
{
    // Add reference, we're returning a pointer
    if ( mShader )
        mShader->AddRef();

    // Return the resource item
    return mShader;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9VertexShader::deviceLost()
{
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
void cgDX9VertexShader::deviceRestored()
{
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
void cgDX9VertexShader::releaseShader( )
{
    // We should release our underlying resource(s)
    if ( mConstantTable )
        mConstantTable->Release();
    if ( mShader )
        mShader->Release();
    mShader           = CG_NULL;
    mConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createShader ()
/// <summary>
/// Create the internal shader.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9VertexShader::createShader( )
{
    STRING_CONVERT;
    LPD3DXBUFFER pShaderData = CG_NULL;
    LPD3DXBUFFER pCompileLog = CG_NULL;
    HRESULT      hRet        = D3DERR_INVALIDCALL;

    // Already loaded?
    if ( mShader )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for shader creation (required DX9 class driver)
    IDirect3DDevice9 * pDevice = CG_NULL;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
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
                cgUInt32 nShaderFlags = 0;
                if ( pDriver->getConfig().debugVShader )
                    nShaderFlags = D3DXSHADER_DEBUG | D3DXSHADER_PREFER_FLOW_CONTROL | D3DXSHADER_SKIPOPTIMIZATION;

                // Clean up the source code in debug mode
                if ( pDriver->getConfig().debugVShader )
                    mSourceCode = cgStringUtility::beautifyCode( mSourceCode );

                // Compile from source to a memory buffer.
                LPCSTR lpszVSProfile = "vs_3_0"; // ToDo: 9999 - Get max version (configurable by INI?)
                hRet = D3DXCompileShader( stringConvertT2CA(mSourceCode.c_str()), mSourceCode.size(), CG_NULL, CG_NULL, 
                                          stringConvertT2CA(mSourceEntryPoint.c_str()), lpszVSProfile, nShaderFlags, &pShaderData, 
                                          &pCompileLog, &mConstantTable );

                // Errors / Warnings?
                if ( FAILED( hRet ) )
                {
                    // ToDo: 9999 - Need to correctly map errors back to the original script!
                    if ( pCompileLog != CG_NULL )
                        PrettyCompileErrorsDX9( (LPCSTR)pCompileLog->GetBufferPointer(), _T("TODO") /*strSourceFile*/, cgString::Empty /*cgFileSystem::GetDirectoryName( strSourceFile )*/ );
                    else
                        cgAppLog::write( cgAppLog::Error, _T("An unexpected parsing or compilation error occurred whilst attempting to compile '%s'.\n"), getResourceName().c_str() );
                    throw cgExceptions::ResultException( hRet, _T("D3DXCompileShader"), cgDebugSource() );
                    
                } // End if failed
                else if ( pCompileLog != CG_NULL )
                {
                    // May still have spit out warnings.
                    // ToDo: 9999 - Need to correctly map errors back to the original script!
                    // ToDo: 9999 - Optional warnings!
                    //PrettyCompileErrorsDX9( (LPCSTR)pCompileLog->GetBufferPointer(), _T("TODO") /*strSourceFile*/, cgString::Empty /*cgFileSystem::GetDirectoryName( strSourceFile )*/ );

                } // End if success

                // Shader compiled OK. Extract byte code.
                mByteCode.resize( pShaderData->GetBufferSize() );
                memcpy( &mByteCode[0], pShaderData->GetBufferPointer(), mByteCode.size() );
                mCompiled = true;

                // Free up memory by releasing source code. Any subsequent calls to 'CreateResource()' 
                // will now just load the shader resource from the byte code itself.
                mSourceCode = cgString::Empty;

                // Release any outstanding D3D resources.
                if ( pShaderData ) pShaderData->Release();
                if ( pCompileLog ) pCompileLog->Release();
                pShaderData = CG_NULL;
                pCompileLog = CG_NULL;

                // Write the shader to the on-disk cache if it doesn't already exist.
                if ( ! pDriver->getConfig().debugVShader && !mIdentifier.sourceFiles.empty() )
                {
                    // First build the unique hashed file name for this shader permutation 
                    // based on the source files, the parameter data and the shader identifier / name.
                    // If this file does not exist, write the identifier and the byte array to file.
                    cgString strCacheFile = _T("sys://Cache/Shaders/") + mIdentifier.generateHashName() + _T(".vs3");
                    strCacheFile = cgFileSystem::resolveFileLocation( strCacheFile );
                    if ( !cgFileSystem::fileExists( strCacheFile ) )
                        writePermutationCache( strCacheFile );
                    
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
            if ( FAILED(hRet = pDevice->CreateVertexShader( (DWORD*)&mByteCode[0], &mShader ) ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to create hardware device compatible vertex shader for '%s'.\n"), getResourceName().c_str() );
                throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::CreateVertexShader"), cgDebugSource() );
                
            } // End if failed

        } // End if compiled

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
        if ( pShaderData ) pShaderData->Release();
        if ( pCompileLog ) pCompileLog->Release();
        pDevice->Release();
        return false;

    } // End Catch

    // Release the D3D device, we're all done with it
    pDevice->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9VertexShader::loadResource( )
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
bool cgDX9VertexShader::unloadResource( )
{
    // Release the shader
    releaseShader();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX9PixelShader Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9PixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9PixelShader::cgDX9PixelShader( cgUInt32 nReferenceId, const cgInputStream & SourceFile, const cgString & strEntryPoint ) :
    cgPixelShader( nReferenceId, SourceFile, strEntryPoint )
{
    // Initialize variables to sensible defaults
    m_pShader           = CG_NULL;
    m_pConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgDX9PixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9PixelShader::cgDX9PixelShader( cgUInt32 nReferenceId, const cgInputStream & SourceFile, bool bCompiled ) :
    cgPixelShader( nReferenceId, SourceFile, bCompiled )
{
    // Initialize variables to sensible defaults
    m_pShader           = CG_NULL;
    m_pConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgDX9PixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9PixelShader::cgDX9PixelShader( cgUInt32 nReferenceId, const cgString & strSource, const cgString & strEntryPoint, const cgShaderIdentifier * pIdentifier ) :
    cgPixelShader( nReferenceId, strSource, strEntryPoint, pIdentifier )
{
    // Initialize variables to sensible defaults
    m_pShader           = CG_NULL;
    m_pConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgDX9PixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9PixelShader::cgDX9PixelShader( cgUInt32 nReferenceId, const cgByteArray & aByteCode, const cgShaderIdentifier * pIdentifier ) : 
    cgPixelShader( nReferenceId, aByteCode, pIdentifier )
{
    // Initialize variables to sensible defaults
    m_pShader           = CG_NULL;
    m_pConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9PixelShader () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9PixelShader::~cgDX9PixelShader( )
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
void cgDX9PixelShader::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( bDisposeBase == true )
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
bool cgDX9PixelShader::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9PixelShaderResource )
        return true;

    // Supported by base?
    return cgPixelShader::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : GetD3DShader ()
/// <summary>
/// Retrieve the internal D3D9 pixel shader object.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DPixelShader9 * cgDX9PixelShader::GetD3DShader( ) const
{
    // Add reference, we're returning a pointer
    if ( m_pShader )
        m_pShader->AddRef();

    // Return the resource item
    return m_pShader;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9PixelShader::deviceLost()
{
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    ReleaseShader();

    // Resource data is now lost
    setResourceLost( true );
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9PixelShader::deviceRestored()
{
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    CreateShader();
}

//-----------------------------------------------------------------------------
//  Name : ReleaseShader ()
/// <summary>
/// Release the internal shader.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9PixelShader::ReleaseShader( )
{
    // We should release our underlying resource(s)
    if ( m_pConstantTable )
        m_pConstantTable->Release();
    if ( m_pShader )
        m_pShader->Release();
    m_pShader           = CG_NULL;
    m_pConstantTable    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : CreateShader ()
/// <summary>
/// Create the internal shader.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9PixelShader::CreateShader( )
{
    STRING_CONVERT;
    LPD3DXBUFFER pShaderData = CG_NULL;
    LPD3DXBUFFER pCompileLog = CG_NULL;
    HRESULT      hRet        = D3DERR_INVALIDCALL;

    // Already loaded?
    if ( m_pShader )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for shader creation (required DX9 class driver)
    IDirect3DDevice9 * pDevice = CG_NULL;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
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
                cgUInt32 nShaderFlags = 0;
                if ( pDriver->getConfig().debugPShader )
                    nShaderFlags = D3DXSHADER_DEBUG | D3DXSHADER_PREFER_FLOW_CONTROL | D3DXSHADER_SKIPOPTIMIZATION;

                // Clean up the source code in debug mode
                if ( pDriver->getConfig().debugPShader )
                    mSourceCode = cgStringUtility::beautifyCode( mSourceCode );

                // Compile from source to a memory buffer.
                LPCSTR lpszVSProfile = "ps_3_0"; // ToDo: 9999 - Get max version (configurable by INI?)
                hRet = D3DXCompileShader( stringConvertT2CA(mSourceCode.c_str()), mSourceCode.size(), CG_NULL, CG_NULL, 
                                          stringConvertT2CA(mSourceEntryPoint.c_str()), lpszVSProfile, nShaderFlags, &pShaderData, 
                                          &pCompileLog, &m_pConstantTable );

                // Errors / Warnings?
                if ( FAILED( hRet ) )
                {
                    // ToDo: 9999 - Need to correctly map errors back to the original script!
                    if ( pCompileLog != CG_NULL )
                        PrettyCompileErrorsDX9( (LPCSTR)pCompileLog->GetBufferPointer(), _T("TODO") /*strSourceFile*/, cgString::Empty /*cgFileSystem::GetDirectoryName( strSourceFile )*/ );
                    else
                        cgAppLog::write( cgAppLog::Error, _T("An unexpected parsing or compilation error occurred whilst attempting to compile '%s'.\n"), getResourceName().c_str() );
                    throw cgExceptions::ResultException( hRet, _T("D3DXCompileShader"), cgDebugSource() );
                    
                } // End if failed
                else if ( pCompileLog != CG_NULL )
                {
                    // May still have spit out warnings.
                    // ToDo: 9999 - Need to correctly map errors back to the original script!
                    // ToDo: 9999 - Optional warnings!
                    //PrettyCompileErrorsDX9( (LPCSTR)pCompileLog->GetBufferPointer(), _T("TODO") /*strSourceFile*/, cgString::Empty /*cgFileSystem::GetDirectoryName( strSourceFile )*/ );

                } // End if success

                // Shader compiled OK. Extract byte code.
                mByteCode.resize( pShaderData->GetBufferSize() );
                memcpy( &mByteCode[0], pShaderData->GetBufferPointer(), mByteCode.size() );
                mCompiled = true;

                // Free up memory by releasing source code. Any subsequent calls to 'CreateResource()' 
                // will now just load the shader resource from the byte code itself.
                mSourceCode = cgString::Empty;

                // Release any outstanding D3D resources.
                if ( pShaderData ) pShaderData->Release();
                if ( pCompileLog ) pCompileLog->Release();
                pShaderData = CG_NULL;
                pCompileLog = CG_NULL;

                // Write the shader to the on-disk cache if it doesn't already exist.
                if ( !pDriver->getConfig().debugPShader && !mIdentifier.sourceFiles.empty() )
                {
                    // First build the unique hashed file name for this shader permutation 
                    // based on the source files, the parameter data and the shader identifier / name.
                    // If this file does not exist, write the identifier and the byte array to file.
                    cgString strCacheFile = _T("sys://Cache/Shaders/") + mIdentifier.generateHashName() + _T(".ps3");
                    strCacheFile = cgFileSystem::resolveFileLocation( strCacheFile );
                    if ( !cgFileSystem::fileExists( strCacheFile ) )
                        writePermutationCache( strCacheFile );
                    
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
            if ( FAILED(hRet = pDevice->CreatePixelShader( (DWORD*)&mByteCode[0], &m_pShader ) ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to create hardware device compatible pixel shader for '%s'.\n"), getResourceName().c_str() );
                throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::CreatePixelShader"), cgDebugSource() );
                
            } // End if failed

        } // End if compiled

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
        if ( pShaderData ) pShaderData->Release();
        if ( pCompileLog ) pCompileLog->Release();
        pDevice->Release();
        return false;

    } // End Catch

    // Release the D3D device, we're all done with it
    pDevice->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9PixelShader::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the shader
    if ( !CreateShader() )
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
bool cgDX9PixelShader::unloadResource( )
{
    // Release the shader
    ReleaseShader();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

#endif // CGE_DX9_RENDER_SUPPORT