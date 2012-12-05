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
// cgHardwareShaders Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgHardwareShaders.h>

// Platform specific implementations
#include <Resources/Platform/cgDX9HardwareShaders.h>
#include <Resources/Platform/cgDX11HardwareShaders.h>

///////////////////////////////////////////////////////////////////////////////
// cgVertexShader Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgVertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader::cgVertexShader( cgUInt32 nReferenceId, const cgInputStream & SourceFile, const cgString & strEntryPoint ) : 
    cgResource( nReferenceId )
{
    // Store resource initialization vars
    mSourceFile       = SourceFile;
    mSourceEntryPoint = strEntryPoint;
    mCompiled         = false;
    
    // Setup initial cached responses
    mResourceType     = cgResourceType::VertexShader;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : cgVertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader::cgVertexShader( cgUInt32 nReferenceId, const cgInputStream & SourceFile, bool bCompiled ) : 
    cgResource( nReferenceId )
{
    // Store resource initialization vars
    mSourceFile        = SourceFile;
    mCompiled         = bCompiled;
    
    // Setup initial cached responses
    mResourceType      = cgResourceType::VertexShader;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : cgVertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader::cgVertexShader( cgUInt32 nReferenceId, const cgString & strSource, const cgString & strEntryPoint, const cgShaderIdentifier * pIdentifier ) : 
    cgResource( nReferenceId ), mIdentifier( pIdentifier )
{
    // Store resource initialization vars
    mSourceCode       = strSource;
    mSourceEntryPoint = strEntryPoint;
    mCompiled         = false;
    
    // Setup initial cached responses
    mResourceType     = cgResourceType::VertexShader;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : cgVertexShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader::cgVertexShader( cgUInt32 nReferenceId, const cgByteArray & aByteCode, const cgShaderIdentifier * pIdentifier ) : 
    cgResource( nReferenceId ), mIdentifier( pIdentifier )
{
    // Store resource initialization vars
    mByteCode         = aByteCode;
    mCompiled         = true;
    
    // Setup initial cached responses
    mResourceType      = cgResourceType::VertexShader;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgVertexShader () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader::~cgVertexShader( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader * cgVertexShader::createInstance( cgUInt32 nReferenceId, const cgInputStream & SourceFile, const cgString & strEntryPoint )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9VertexShader( nReferenceId, SourceFile, strEntryPoint );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11VertexShader( nReferenceId, SourceFile, strEntryPoint );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI            

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader * cgVertexShader::createInstance( cgUInt32 nReferenceId, const cgInputStream & SourceFile, bool bCompiled )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9VertexShader( nReferenceId, SourceFile, bCompiled );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11VertexShader( nReferenceId, SourceFile, bCompiled );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI            

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader * cgVertexShader::createInstance( cgUInt32 nReferenceId, const cgString & strSource, const cgString & strEntryPoint, const cgShaderIdentifier * pIdentifier )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9VertexShader( nReferenceId, strSource, strEntryPoint, pIdentifier );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11VertexShader( nReferenceId, strSource, strEntryPoint, pIdentifier );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShader * cgVertexShader::createInstance( cgUInt32 nReferenceId, const cgByteArray & aByteCode, const cgShaderIdentifier * pIdentifier )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9VertexShader( nReferenceId, aByteCode, pIdentifier );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11VertexShader( nReferenceId, aByteCode, pIdentifier );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgVertexShader::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Nothing in base implementation

    // Dispose base(s).
    if ( bDisposeBase == true )
        cgResource::dispose( true );
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
bool cgVertexShader::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_VertexShaderResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getIdentifier ()
/// <summary>
/// Return a reference to the internal identifier information, used to uniquely
/// identify the specific shader and permutation data managed by this object.
/// </summary>
//-----------------------------------------------------------------------------
const cgShaderIdentifier & cgVertexShader::getIdentifier( ) const
{
    return mIdentifier;
}

//-----------------------------------------------------------------------------
//  Name : getByteCode ()
/// <summary>
/// Retrieve the compiled byte code for this shader.
/// </summary>
//-----------------------------------------------------------------------------
const cgByteArray & cgVertexShader::getByteCode( ) const
{
    return mByteCode;
}

//-----------------------------------------------------------------------------
//  Name : readCompiledShader() (Protected)
/// <summary>
/// Deserialize the compiled shader from the specified file (i.e. from shader 
/// permutation cache.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgVertexShader::readCompiledShader( cgInputStream stream )
{
    cgToDo( "Carbon General", "Validate all data." );

    STRING_CONVERT;
    if ( stream.open() )
    {
        // Read file header (CGEVSC)
        cgChar Header[6];
        stream.read( Header, 6 );
        
        // Read version information (1.0.0)
        cgUInt32 nVersion = stream.readUInt32();

        // Start populating identifier, name first.
        cgShaderIdentifier Identifier;
        Identifier.shaderIdentifier = stream.readString( stream.readUInt16() );
        
        // Number of source files that contributed.
        Identifier.sourceFiles.resize( stream.readUInt16() );

        // Source files.
        for ( size_t i = 0; i < Identifier.sourceFiles.size(); ++i )
        {
            cgShaderIdentifier::SourceFileInfo & SourceFile = Identifier.sourceFiles[i];
            SourceFile.name = stream.readString( stream.readUInt16() );
            stream.read( SourceFile.hash, 20 );

        } // Next File

        // Parameter data.
        cgUInt16 nLength = stream.readUInt16();
        if ( nLength > 0 )
        {
            Identifier.parameterData.resize( nLength / 4 );
            stream.read( &Identifier.parameterData[0], nLength );
        
        } // End if has parameters

        // Read the shader input signature hash
        stream.read( Identifier.inputSignatureHash, 5 * sizeof(cgUInt32) );

        // Read the compiled shader data
        mByteCode.resize( stream.readUInt32() );
        if ( !mByteCode.empty() )
            stream.read( &mByteCode[0], mByteCode.size() );

        cgToDo( "Carbon General", "Ensure shader identifiers match if one was supplied?" );
        mIdentifier = Identifier;

        // We're done
        stream.close();

        cgAppLog::write( cgAppLog::Debug, _T("Loaded compiled permutation of vertex shader '%s' from '%s'\n"), mIdentifier.shaderIdentifier.c_str(), cgFileSystem::getFileName(stream.getName()).c_str() );

        // Valid
        return true;
        
    } // End if good

    // Invalid
    return false;
}

//-----------------------------------------------------------------------------
//  Name : writePermutationCache() (Protected)
/// <summary>
/// Serialize the compiled shader to the shader permutation cache.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexShader::writePermutationCache( const cgString & strCacheFile )
{
    STRING_CONVERT;
    std::ofstream stream;
    stream.open( stringConvertT2CA(strCacheFile.c_str()), std::ios::out | std::ios::trunc | std::ios::binary );
    if ( stream.good() )
    {
        // Write file header (CGEVSC)
        cgChar Header[] = { 'C', 'G', 'E', 'V', 'S', 'C' };
        stream.write( Header, 6 );

        // Write version information (1.0.0)
        cgUInt32 nVersion = 0x01000000;
        stream.write( (char*)&nVersion, 4 );

        // Shader identifier name first.
        cgUInt16 nLength = (cgUInt16)mIdentifier.shaderIdentifier.size();
        stream.write( (char*)&nLength, 2 );
        if ( nLength )
            stream.write( stringConvertT2CA(mIdentifier.shaderIdentifier.c_str()), nLength );

        // Number of source files that contributed.
        nLength = (cgUInt16)mIdentifier.sourceFiles.size();
        stream.write( (char*)&nLength, 2 );

        // Source files.
        for ( size_t i = 0; i < mIdentifier.sourceFiles.size(); ++i )
        {
            const cgShaderIdentifier::SourceFileInfo & SourceFile = mIdentifier.sourceFiles[i];

            // Source name first.
            nLength = (cgUInt16)SourceFile.name.size();
            stream.write( (char*)&nLength, 2 );
            if ( nLength )
                stream.write( stringConvertT2CA(SourceFile.name.c_str()), nLength );

            // Source file hash
            stream.write( (char*)SourceFile.hash, 20 );

        } // Next File

        // Parameter data.
        nLength = (cgUInt16)mIdentifier.parameterData.size() * sizeof(cgUInt32);
        stream.write( (char*)&nLength, 2 );
        if ( nLength )
            stream.write( (char*)&mIdentifier.parameterData[0], nLength );
        
        // Write the shader input signature hash
        stream.write( (char*)mIdentifier.inputSignatureHash, 5 * sizeof(cgUInt32) );

        // Write the compiled shader data
        cgUInt32 nLength32 = (cgUInt32)mByteCode.size();
        stream.write( (char*)&nLength32, 4 );
        if ( nLength32 )
            stream.write( (char*)&mByteCode[0], nLength32 );

        // We're done
        stream.close();

        cgAppLog::write( cgAppLog::Debug, _T("Saved compiled permutation of vertex shader '%s' into '%s'\n"), mIdentifier.shaderIdentifier.c_str(), cgFileSystem::getFileName(strCacheFile).c_str() );
        
    } // End if good
}


///////////////////////////////////////////////////////////////////////////////
// cgPixelShader Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader::cgPixelShader( cgUInt32 nReferenceId, const cgInputStream & SourceFile, const cgString & strEntryPoint ) :
    cgResource( nReferenceId )
{
    // Store resource initialization vars
    mSourceFile        = SourceFile;
    mSourceEntryPoint    = strEntryPoint;
    mCompiled         = false;
    
    // Setup initial cached responses
    mResourceType      = cgResourceType::PixelShader;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : cgPixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader::cgPixelShader( cgUInt32 nReferenceId, const cgInputStream & SourceFile, bool bCompiled ) :
    cgResource( nReferenceId )
{
    // Store resource initialization vars
    mSourceFile        = SourceFile;
    mCompiled         = bCompiled;
    
    // Setup initial cached responses
    mResourceType      = cgResourceType::PixelShader;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : cgPixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader::cgPixelShader( cgUInt32 nReferenceId, const cgString & strSource, const cgString & strEntryPoint, const cgShaderIdentifier * pIdentifier ) :
    cgResource( nReferenceId ), mIdentifier( pIdentifier )
{
    // Store resource initialization vars
    mSourceCode     = strSource;
    mSourceEntryPoint    = strEntryPoint;
    mCompiled         = false;
    
    // Setup initial cached responses
    mResourceType      = cgResourceType::PixelShader;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : cgPixelShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader::cgPixelShader( cgUInt32 nReferenceId, const cgByteArray & aByteCode, const cgShaderIdentifier * pIdentifier ) : 
    cgResource( nReferenceId ), mIdentifier( pIdentifier )
{
    // Store resource initialization vars
    mByteCode         = aByteCode;
    mCompiled         = true;
    
    // Setup initial cached responses
    mResourceType      = cgResourceType::PixelShader;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgPixelShader () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader::~cgPixelShader( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader * cgPixelShader::createInstance( cgUInt32 nReferenceId, const cgInputStream & SourceFile, const cgString & strEntryPoint )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9PixelShader( nReferenceId, SourceFile, strEntryPoint );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11PixelShader( nReferenceId, SourceFile, strEntryPoint );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader * cgPixelShader::createInstance( cgUInt32 nReferenceId, const cgInputStream & SourceFile, bool bCompiled )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9PixelShader( nReferenceId, SourceFile, bCompiled );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11PixelShader( nReferenceId, SourceFile, bCompiled );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader * cgPixelShader::createInstance( cgUInt32 nReferenceId, const cgString & strSource, const cgString & strEntryPoint, const cgShaderIdentifier * pIdentifier )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9PixelShader( nReferenceId, strSource, strEntryPoint, pIdentifier );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11PixelShader( nReferenceId, strSource, strEntryPoint, pIdentifier );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShader * cgPixelShader::createInstance( cgUInt32 nReferenceId, const cgByteArray & aByteCode, const cgShaderIdentifier * pIdentifier )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9PixelShader( nReferenceId, aByteCode, pIdentifier );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11PixelShader( nReferenceId, aByteCode, pIdentifier );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI            

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgPixelShader::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Nothing in base implementation

    // Dispose base(s).
    if ( bDisposeBase == true )
        cgResource::dispose( true );
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
bool cgPixelShader::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_PixelShaderResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getIdentifier ()
/// <summary>
/// Return a reference to the internal identifier information, used to uniquely
/// identify the specific shader and permutation data managed by this object.
/// </summary>
//-----------------------------------------------------------------------------
const cgShaderIdentifier & cgPixelShader::getIdentifier( ) const
{
    return mIdentifier;
}

//-----------------------------------------------------------------------------
//  Name : getByteCode ()
/// <summary>
/// Retrieve the compiled byte code for this shader.
/// </summary>
//-----------------------------------------------------------------------------
const cgByteArray & cgPixelShader::getByteCode( ) const
{
    return mByteCode;
}

//-----------------------------------------------------------------------------
//  Name : readCompiledShader() (Protected)
/// <summary>
/// Deserialize the compiled shader from the specified file (i.e. from shader 
/// permutation cache.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgPixelShader::readCompiledShader( cgInputStream stream )
{
    cgToDo( "Carbon General", "Validate all data." );

    STRING_CONVERT;
    if ( stream.open() )
    {
        // Read file header (CGEPSC)
        cgChar Header[6];
        stream.read( Header, 6 );
        
        // Read version information (1.0.0)
        cgUInt32 nVersion = stream.readUInt32();

        // Start populating identifier, name first.
        cgShaderIdentifier Identifier;
        Identifier.shaderIdentifier = stream.readString( stream.readUInt16() );
        
        // Number of source files that contributed.
        Identifier.sourceFiles.resize( stream.readUInt16() );

        // Source files.
        for ( size_t i = 0; i < Identifier.sourceFiles.size(); ++i )
        {
            cgShaderIdentifier::SourceFileInfo & SourceFile = Identifier.sourceFiles[i];
            SourceFile.name = stream.readString( stream.readUInt16() );
            stream.read( SourceFile.hash, 20 );

        } // Next File

        // Parameter data.
        cgUInt16 nLength = stream.readUInt16();
        if ( nLength > 0 )
        {
            Identifier.parameterData.resize( nLength / 4 );
            stream.read( &Identifier.parameterData[0], nLength );
        
        } // End if has parameters

        // Read the shader input signature hash
        stream.read( Identifier.inputSignatureHash, 5 * sizeof(cgUInt32) );
        
        // Read the compiled shader data
        mByteCode.resize( stream.readUInt32() );
        if ( !mByteCode.empty() )
            stream.read( &mByteCode[0], mByteCode.size() );
        
        cgToDo( "Carbon General", "Ensure shader identifiers match if one was supplied?" );
        mIdentifier = Identifier;

        // We're done
        stream.close();

        cgAppLog::write( cgAppLog::Debug, _T("Loaded compiled permutation of pixel shader '%s' from '%s'\n"), mIdentifier.shaderIdentifier.c_str(), cgFileSystem::getFileName(stream.getName()).c_str() );

        // Valid
        return true;
        
    } // End if good

    // Invalid
    return false;
}

//-----------------------------------------------------------------------------
//  Name : writePermutationCache() (Protected)
/// <summary>
/// Serialize the compiled shader to the shader permutation cache.
/// </summary>
//-----------------------------------------------------------------------------
void cgPixelShader::writePermutationCache( const cgString & strCacheFile )
{
    STRING_CONVERT;
    std::ofstream stream;
    stream.open( stringConvertT2CA(strCacheFile.c_str()), std::ios::out | std::ios::trunc | std::ios::binary );
    if ( stream.good() )
    {
        // Write file header (CGEPSC)
        cgChar Header[] = { 'C', 'G', 'E', 'P', 'S', 'C' };
        stream.write( Header, 6 );

        // Write version information (1.0.0)
        cgUInt32 nVersion = 0x01000000;
        stream.write( (char*)&nVersion, 4 );

        // Shader identifier name first.
        cgUInt16 nLength = (cgUInt16)mIdentifier.shaderIdentifier.size();
        stream.write( (char*)&nLength, 2 );
        if ( nLength )
            stream.write( stringConvertT2CA(mIdentifier.shaderIdentifier.c_str()), nLength );

        // Number of source files that contributed.
        nLength = (cgUInt16)mIdentifier.sourceFiles.size();
        stream.write( (char*)&nLength, 2 );

        // Source files.
        for ( size_t i = 0; i < mIdentifier.sourceFiles.size(); ++i )
        {
            const cgShaderIdentifier::SourceFileInfo & SourceFile = mIdentifier.sourceFiles[i];

            // Source name first.
            nLength = (cgUInt16)SourceFile.name.size();
            stream.write( (char*)&nLength, 2 );
            if ( nLength )
                stream.write( stringConvertT2CA(SourceFile.name.c_str()), nLength );

            // Source file hash
            stream.write( (char*)SourceFile.hash, 20 );

        } // Next File

        // Parameter data.
        nLength = (cgUInt16)mIdentifier.parameterData.size() * sizeof(cgUInt32);
        stream.write( (char*)&nLength, 2 );
        if ( nLength )
            stream.write( (char*)&mIdentifier.parameterData[0], nLength );

        // Write the shader input signature hash
        stream.write( (char*)mIdentifier.inputSignatureHash, 5 * sizeof(cgUInt32) );
        
        // Write the compiled shader data
        cgUInt32 nLength32 = (cgUInt32)mByteCode.size();
        stream.write( (char*)&nLength32, 4 );
        if ( nLength32 )
            stream.write( (char*)&mByteCode[0], nLength32 );

        // We're done
        stream.close();

        cgAppLog::write( cgAppLog::Debug, _T("Saved compiled permutation of pixel shader '%s' into '%s'\n"), mIdentifier.shaderIdentifier.c_str(), cgFileSystem::getFileName(strCacheFile).c_str() );
        
    } // End if good
}