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
// Name : cgScriptPreprocessor.cpp                                           //
//                                                                           //
// Desc : Utility class designed to provide script parsing / pre-processing  //
//        support. Handles includes, defines, pragmas etc.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgScriptPreprocessor Module Includes
//-----------------------------------------------------------------------------
#include <Scripting/cgScriptPreprocessor.h>
#include <Scripting/cgScriptEngine.h>
#include <Resources/cgScript.h>
#include <Resources/cgSurfaceShaderScript.h>
#include <Resources/cgResourceTypes.h> // cgConstantBufferDesc, cgConstantTypeDesc, cgConstantDesc
#include <System/cgStringUtility.h>

// Angelscript.
#include <angelscript.h>

//-----------------------------------------------------------------------------
// Module Local Variables
//-----------------------------------------------------------------------------
namespace
{
    const cgChar IdentifierBeginChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    const cgChar IdentifierChars[]      = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
    const cgChar NumericChars[]         = "0123456789";
};

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
asIScriptEngine * cgScriptPreprocessor::mEngine = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgScriptPreprocessor Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgScriptPreprocessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptPreprocessor::cgScriptPreprocessor( cgScript * pScript )
{
    // Initialize variables to sensible defaults
    mScript       = pScript;
    mShaderScript = pScript->queryReferenceType( RTID_SurfaceShaderScriptResource );
}

//-----------------------------------------------------------------------------
//  Name : ~cgScriptPreprocessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptPreprocessor::~cgScriptPreprocessor( )
{
}

//-----------------------------------------------------------------------------
//  Name : isValidMacro()
/// <summary>
/// Determine if the specified macro / define name / value are valid.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::isValidMacro( std::string strWord, const std::string & strValue )
{
    // Anything to do?
    if ( strWord.empty() )
        return false;

    // Trim whitespace.
    size_t nStart = strWord.find_first_not_of( " \t" );
    size_t nEnd = strWord.find_last_not_of( " \t" );
    if ( nStart == std::string::npos )
        nStart = 0;
    if ( nEnd == std::string::npos )
        nEnd = strWord.size() - 1;
    strWord = strWord.substr( nStart, (nEnd - nStart) + 1 );

    // Validate
    if ( strWord.empty() )
        return false;
    if ( strWord.find_first_not_of( IdentifierChars ) != cgString::npos )
        return false;
    if ( strWord.substr( 0, 1 ).find_first_not_of( IdentifierBeginChars ) != cgString::npos )
        return false;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : process()
/// <summary>
/// Load and pre-process the specified script file, adding to the script's
/// module as we go.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::process( cgInputStream Stream, const DefinitionMap & Defines, cgScript::SourceFileArray & aSourceFiles )
{
    STRING_CONVERT;  // For string conversion macro
    
    // Create an angelscript engine capable of parsing the file, as well
    // as executing arbitrary #if directives if one does not already exist.
    if ( !mEngine )
    {
        mEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
        mEngine->RegisterGlobalFunction( "bool preprocessorConditionResult( const ? &in )", asFUNCTION( preprocessorConditionResult ), asCALL_CDECL );
    
    } // End if no engine
    else
        mEngine->AddRef();

    // Retrieve the module into which processed code will be placed.
    const cgChar * strModuleName = stringConvertT2CA( mScript->getResourceName().c_str() );
    asIScriptEngine * pScriptsEngine = mScript->getScriptEngine()->getInternalEngine();
    asIScriptModule * pModule = pScriptsEngine->GetModule( strModuleName, asGM_CREATE_IF_NOT_EXISTS );

    // Load the top level script.
    aSourceFiles.clear();
    mDefinitions = Defines;
    if ( loadScriptSection( pModule, Stream, aSourceFiles ) == false )
    {
        if ( mEngine->Release() == 0 )
            mEngine = CG_NULL;
        return false;
    
    } // End if failed

    // If the top level stream was a memory source, we should reset the stream to conserve space
    if ( Stream.getType() == cgStreamType::Memory )
        Stream.reset();

    // Register the "this" global accessors if requested.
    if ( mScript->getThisType().empty() == false )
    {
        std::string strThisType = stringConvertT2CA( mScript->getThisType().c_str() );
        std::string strThisCode = strThisType + "@ this; void __GlobalSetThis(" + strThisType + "@ o){ @this = o; }";
        pModule->AddScriptSection( "__ThisAccessor", strThisCode.c_str(), strThisCode.size(), 0 );
    
    } // End if requested "this"

    // If this is a shader script, add the global variables necessary for the
    // shader replacement blocks to function correctly. Also inject the code
    // from the system definition file (cbuffers, system data class, etc.)
    if ( mShaderScript == true )
    {
        cgInputStream SysDefStream( _T("sys://Shaders/SystemDefs.shh") );
        if ( Stream.getName() != SysDefStream.getName() )
        {
            std::string strShaderGlobals = "String __shx, __shi, __sho, __shg, __cbr, __sbr;SystemExports @ System;\n";
            strShaderGlobals.append( "String __shsyscmnvs(){ return System.SystemCommon(0);}\n" );
            strShaderGlobals.append( "String __shsyscmnps(){ return System.SystemCommon(1);}\n" );
            pModule->AddScriptSection( "__ShaderScriptGlobals", strShaderGlobals.c_str(), strShaderGlobals.size(), 0 );
            if ( loadScriptSection( pModule, SysDefStream, aSourceFiles ) == false )
            {
                if ( mEngine->Release() == 0 )
                    mEngine = CG_NULL;
                return false;     
            
            } // End if failed

        } // End if !Exports script
        
    } // End if shader script

    // We're done with our reference to the (static) engine.
    if ( mEngine->Release() == 0 )
        mEngine = CG_NULL;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadScriptSection () (Protected)
/// <summary>
/// Load the referenced script data, perform pre-processor step, and then
/// add to the specified module
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::loadScriptSection( asIScriptModule * pModule, cgInputStream & Stream, cgScript::SourceFileArray & aSourceFiles )
{
    STRING_CONVERT;  // For string conversion macro
    size_t           nCodeLength;
    std::string      strScript;
    std::ifstream    ScriptFile;

    cgToDo( "Effect Overhaul", "If file is not found, it steps into the memory / mapped file case!" );

    // Memory or file source?
    if ( Stream.getType() == cgStreamType::File )
    {
        // Attempt to open the specified script file (we'll use binary to maintain the exact file layout)
        ScriptFile.open( stringConvertT2CA( Stream.getSourceFile().c_str() ), std::ios_base::binary | std::ios::ate );
        if ( !ScriptFile.good() )
        {
            cgAppLog::write( cgAppLog::Error, _T("File not found while attempting to open script or script dependency '%s'.\n"), Stream.getName().c_str() );
            return false;

        } // End if failed to open

        // Retrieve the length of the file and then seek back to the beginning
        nCodeLength = (size_t)ScriptFile.tellg();
        ScriptFile.seekg( 0 );

        // Ensure this isn't a zero length script file
        if ( nCodeLength == 0 )
        {
            cgAppLog::write( cgAppLog::Error, _T("Zero length script encountered when loading file '%s'.\n"), Stream.getName().c_str() );
            return false;

        } // End if failed to read

        // Reserve enough space in an ANSI string to load the script data
        // and then read it from the specified file.
        strScript.resize( nCodeLength );
        if ( !ScriptFile.read( &strScript[0], (std::streamsize)nCodeLength ) && !ScriptFile.eof() )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to read contents of the script or script dependency '%s'.\n"), Stream.getName().c_str() );
            return false;

        } // End if failed to read
        
        // Close the file
        ScriptFile.close();

    } // End if file
    else
    {
        // Get the buffer
        cgByte * pBuffer = Stream.getBuffer( nCodeLength );
        if ( pBuffer == CG_NULL )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to read contents of the script or script dependency '%s'.\n"), Stream.getName().c_str() );
            return false;
        
        } // End if failed to access

        // Reserve enough space in an ANSI string to load the script data
        // and then copy it from the specified stream.
        strScript.resize( nCodeLength );
        memcpy( &strScript[0], pBuffer, nCodeLength );
        Stream.releaseBuffer();

    } // End if memory / mapped file

    // Record the information about this file in the source file list.
    // This includes the stream name and the file hash. The latter allows
    // us to determine if the source files have changed since last time
    // they were used to compile this script.
    cgUInt32 Hash[5];
    Stream.computeSHA1( Hash );
    bool bNewFile = true;
    for ( size_t i = 0; i < aSourceFiles.size(); ++i )
    {
        if ( memcmp( aSourceFiles[i].hash, Hash, 20 ) == 0 )
        {
            bNewFile = false;
            break;
        
        } // End if already listed
    
    } // Next source file
    if ( bNewFile )
    {
        aSourceFiles.resize( aSourceFiles.size() + 1 );
        cgScript::SourceFileInfo & Info = aSourceFiles.back();
        Info.name = Stream.getName();
        memcpy( Info.hash, Hash, 20 );
    
    } // End if insert new
    
    // Process the script to include all necessary headers, etc.
    if ( processScriptSection( pModule, strScript, Stream.getName(), aSourceFiles ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("An error occured while attempting to add code to script resource '%s'.\n"), mScript->getResourceName().c_str() );
        return false;

    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getNextToken () (Protected)
/// <summary>
/// Read the next token from the specified script code. Optionally, comments 
/// and whitespace can be skipped.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::getNextToken( TokenData & t, const std::string & strScript, bool bSkipCommentAndWS )
{
    cgInt nLength;
    
    // End of file?
    if ( t.position > t.sectionEnd )
        return false;

    // Retrieve next token, skipping comments and whitespace optionally
    while ( t.position <= t.sectionEnd )
    {
        t.tokenType   = mEngine->ParseToken( &strScript[t.position], (t.sectionEnd+1) - t.position, &nLength );
        t.tokenStart  = t.position;
        t.tokenLength = (size_t)nLength;
        t.position   += t.tokenLength;
        if ( bSkipCommentAndWS && (t.tokenType == asTC_COMMENT || t.tokenType == asTC_WHITESPACE) )
            continue;
        else
            break;
    
    } // Next Token
    return true;
}

//-----------------------------------------------------------------------------
//  Name : expandStatement () (Protected)
/// <summary>
/// Recursively resolve the specified statement until all defined words and
/// macros are expanded
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::expandStatement( TokenData & t, std::string & strStatement )
{
    std::string strToken, strValue;
    bool bModified = false;

    // Pre-process the statement until all definitions are discovered.
    while ( t.position <= t.sectionEnd )
    {
        if ( !getNextToken( t, strStatement, true ) )
            break;

        // What type of token is this?
        if ( t.tokenType == asTC_IDENTIFIER )
        {
            // If this is a defined word, expand it.
            strToken.assign( &strStatement[t.tokenStart], t.tokenLength );
            DefinitionMap::const_iterator itDef = mDefinitions.find( strToken );
            if ( itDef != mDefinitions.end() )
            {
                // We're modifying the incoming string
                bModified = true;

                // Definition was found. Expand further as necessary.
                TokenData tChild;
                strValue            = itDef->second;
                tChild.sectionBegin = 0;
                tChild.sectionEnd   = strValue.size() - 1;
                tChild.position     = 0;
                expandStatement( tChild, strValue );

                // Replace the section of the parent statement with the expanded token value
                // Enough space in the supplied string to just replace the existing block?
                size_t nNewSize = strValue.length();
                if ( nNewSize == t.tokenLength )
                {
                    // Exact match in size. We can just replace.
                    strStatement.replace( t.tokenStart, nNewSize, strValue );
                
                } // End if exact size
                else if ( nNewSize < t.tokenLength )
                {
                    // Smaller. We can replace the first part, and clear the rest.
                    strStatement.replace( t.tokenStart, nNewSize, strValue );
                    overwriteCode( strStatement, t.tokenStart + nNewSize, t.tokenLength - nNewSize );

                } // End if smaller
                else
                {
                    // Code is larger, we can replace the contents entirely but we must 
                    // also adjust the current read position to take this expanded string 
                    // size into account.
                    strStatement.replace( t.tokenStart, t.tokenLength, strValue );
                    t.sectionEnd += (cgInt)nNewSize - (cgInt)t.tokenLength;
                    t.position    = t.tokenStart + nNewSize;

                } // End if larger

            } // End if found

        } // End if identifier

    } // Next Token

    // Return modified / expanded status.
    return bModified;
}

//-----------------------------------------------------------------------------
//  Name : preprocessorConditionResult () (Static, Protected)
/// <summary>
/// Function called by the script in order to determine the boolean result of
/// any #if pre-processor conditional directive.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::preprocessorConditionResult( void * p0, cgInt t0 )
{
    // ToDo: Clean up with latest version of AS where the size of 
    // data types is available!
    /*// What is the data type?
    asIObjectType * pType = mEngine->GetObjectTypeById( t0 );
    if ( pType->GetFlags() & asOBJ_REF )
        return (p0 != CG_NULL);
    else
    {
        cgUInt nSize = pType->GetSize();
        cgByte * pData = (cgByte*)p0;
        for ( cgUInt i = 0; i < nSize; ++i )
        {
            // Return true if /any/ bit is set.
            if ( p0 )
                return true;

        } // Next Byte

    } // End if value
    return false;*/

    // What type is this?
    std::string strDec = mEngine->GetTypeDeclaration(t0 );
    if( strDec == "int8" ||
		strDec == "uint8" )
		return (*(cgByte*)(p0) != 0);

	if( strDec == "int16" ||
		strDec == "uint16" )
        return (*(cgUInt16*)(p0) != 0);

    if( strDec == "int" ||
		strDec == "uint" ||
        strDec == "float" )
        return (*(cgUInt16*)(p0) != 0);

	if( strDec == "double" ||
		strDec == "int64" ||
		strDec == "uint64" )
		return (*(cgUInt64*)(p0) != 0);

	if ( strDec == "bool" )
		return (*(cgByte*)(p0) != 0);

    // Is object type?
    asIObjectType * pType = mEngine->GetObjectTypeById( t0 );
    if ( pType != CG_NULL && pType->GetFlags() & asOBJ_REF )
        return (p0 != CG_NULL);
	return false;
}

//-----------------------------------------------------------------------------
//  Name : parsePreprocessorDirective () (Protected)
/// <summary>
/// Parse the pre-processor directive at the specified location and take any
/// appropriate action (i.e. handle #if, #else, #elif, #endif, #define, etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::parsePreprocessorDirective( TokenData & t, std::string & strScript, std::stack<bool> & ConditionStack, asIScriptModule * pModule, const cgString & strSectionName, cgScript::SourceFileArray & aSourceFiles )
{
    STRING_CONVERT;
    std::string strToken;
    size_t nDirectiveStart = t.tokenStart;

    // Reset position to just following the '#' character
    // in case the tokenizer pushed us past the directive label.
    t.position = t.tokenStart + 1;

    // Retrieve the directive name / label.
    if ( !getNextToken( t, strScript, true ) )
    {
        cgToDoAssert( "Script Preprocessor", "Missing pre-processor label." );
        return false;
    
    } // End if Error
    strToken.assign( &strScript[t.tokenStart], t.tokenLength );
    
    // Is this a directive token we know?
    bool bIfndef = false, bElse = false;
    if ( strToken == "if" || (bElse |= (strToken == "elif")) || (bElse |= (strToken == "elseif")) )
    {
        // If this is an 'else' style clause, has the condition already been met?
        if ( bElse )
        {
            // Check for validity
            if ( ConditionStack.empty() )
            {
                cgAppLog::write( cgAppLog::Error, _T("Unexpected #%s encountered. No matching #if, #ifdef or #ifndef directive.\n"), strToken.c_str() );
                return false;
            
            } // End if malformed

            // If condition has already been met on a prior occasion, strip.
            if ( ConditionStack.top() == true )
            {
                // Overwrite the #directive with spaces to avoid compiler error
                overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );

                // Strip up to the next conditional directive
                excludeCode( t, strScript );
                return true;
            
            } // End if already consumed

        } // End if 'else' clause

        // Retrieve the entire "#if" statement.
        std::string strStatement;
        size_t nNewLine = strScript.find_first_of( "\r\n", t.position );
        if ( nNewLine == std::string::npos )
            nNewLine = t.sectionEnd + 1;
        strStatement.assign( &strScript[t.position], nNewLine - t.position );
        t.position = nNewLine;

        // Overwrite the #directive with spaces to avoid compiler error
        overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );

        // Anything to do?
        if ( strStatement.empty() == false )
        {
            std::string strOriginalStatement = strStatement; // For error reporting

            // Find any 'defined(IDENTIFIER)' sections and replace with
            // either 'true' or 'false' depending on whether or not that
            // word has been previously defined.
            TokenData t2;
            t2.sectionBegin = 0;
            t2.sectionEnd = strStatement.size() - 1;
            t2.position = 0;
            while ( t2.position <= t2.sectionEnd )
            {
                if ( !getNextToken( t2, strStatement, true ) )
                    break;

                // What type of token is this?
                if ( t2.tokenType == asTC_IDENTIFIER )
                {
                    // If this is the 'defined' keyword?
                    strToken.assign( &strStatement[t2.tokenStart], t2.tokenLength );
                    if ( strToken == "defined" )
                    {
                        bool bValid = true;
                        size_t nDefinedStart = t2.tokenStart;

                        // This is the 'defined' keyword. Next three tokens should be an 
                        // opening parenthesis followed by the word identifier and finally
                        // a closing parenthesis.
                        if ( !getNextToken( t2, strStatement, true ) )
                            bValid = false;
                        if ( bValid && (t2.tokenType != asTC_KEYWORD || strStatement[t2.tokenStart] != '(') )
                            bValid = false;
                        if ( bValid && !getNextToken( t2, strStatement, true ) )
                            bValid = false;
                        if ( bValid && (t2.tokenType != asTC_IDENTIFIER) )
                            bValid = false;
                        else
                            strToken.assign( &strStatement[t2.tokenStart], t2.tokenLength );
                        if ( bValid && !getNextToken( t2, strStatement, true ) )
                            bValid = false;
                        if ( bValid && (t2.tokenType != asTC_KEYWORD || strStatement[t2.tokenStart] != ')') )
                            bValid = false;

                        // If we made it all the way here with 'bValid' set to true, we're
                        // good to go. The identifier to test is stored in 'strToken'.
                        if ( bValid == true )
                        {
                            if ( mDefinitions.find( strToken ) != mDefinitions.end() )
                            {
                                strStatement.replace( nDefinedStart, t2.position - nDefinedStart, "true" ); 
                                t2.sectionEnd = strStatement.size() - 1;
                                t2.position = nDefinedStart + 4;
                            
                            } // End if found
                            else
                            {
                                strStatement.replace( nDefinedStart, t2.position - nDefinedStart, "false" ); 
                                t2.sectionEnd = strStatement.size() - 1;
                                t2.position = nDefinedStart + 5;
                            
                            } // End if !found

                        } // End if valid
                        else
                        {
                            cgAppLog::write( cgAppLog::Error, _T("Syntax error while parsing pre-processor directive: %s.\n"), stringConvertA2CT( strOriginalStatement.c_str() ) );
                            return false;

                        } // End if invalid
                    
                    } // End if 'defined'

                } // End if identifier

            } // Next Token

            // Now recursively expand the statement to its full form (i.e. 
            // resolve any remaining defined words / macros).
            t2.sectionBegin = 0;
            t2.sectionEnd = strStatement.size() - 1;
            t2.position = 0;
            expandStatement( t2, strStatement );

            // Finally. Replace all booleans with numeric 1 or 0
            // (same for boolean operators)
            size_t nPos = 0;
            for ( nPos = 0; (nPos = strStatement.find( "true", nPos ) ) != std::string::npos; )
                strStatement.replace( nPos, 4, "1" );
            for ( nPos = 0; (nPos = strStatement.find( "false", nPos ) ) != std::string::npos; )
                strStatement.replace( nPos, 5, "0" );
            for ( nPos = 0; (nPos = strStatement.find( "||", nPos ) ) != std::string::npos; )
                strStatement.replace( nPos, 2, "|" );
            for ( nPos = 0; (nPos = strStatement.find( "&&", nPos ) ) != std::string::npos; )
                strStatement.replace( nPos, 2, "&" );
            for ( nPos = 0; (nPos = strStatement.find( "^^", nPos ) ) != std::string::npos; )
                strStatement.replace( nPos, 2, "^" );
            for ( nPos = 0; (nPos = strStatement.find( "!", nPos ) ) != std::string::npos; )
                strStatement.replace( nPos, 1, "~" );

            // Attempt to compile and execute the statement to see if it resolves to true.
            strStatement = "bool Resolve(){ return preprocessorConditionResult(" + strStatement + ");}";
            asIScriptModule * pProcessorModule = mEngine->GetModule( "__Preprocessor", asGM_CREATE_IF_NOT_EXISTS );
            pProcessorModule->AddScriptSection( "_Resolve", strStatement.c_str(), strStatement.size() );
            if ( pProcessorModule->Build() < 0 )
            {
                mEngine->DiscardModule( "__Preprocessor" );
                cgAppLog::write( cgAppLog::Error, _T("Syntax error or undefined symbol while parsing pre-processor directive: %s.\n"), stringConvertA2CT( strOriginalStatement.c_str() ) );
                return false;
            
            } // End if failed

            // Prepare the context to call the resolve function
            asIScriptFunction * pFunc = pProcessorModule->GetFunctionByName( "Resolve" );
            asIScriptContext * pContext = mEngine->CreateContext();
            pContext->Prepare( pFunc );

            // Execute the function
            pContext->Execute();
            bool bResult = *((bool*)pContext->GetAddressOfReturnValue());
            pContext->Release();
            mEngine->DiscardModule( "__Preprocessor" );

            // Push the result onto the condition stack for 'if' or
            // update condition stack for 'else'.
            if ( !bElse )
                ConditionStack.push( bResult );
            else
                ConditionStack.top() = bResult;

            // Strip code up to the next else / end if condition was negative.
            if ( bResult == false )
                excludeCode( t, strScript );

        } // End if has statement

    } // End if '#if' / '#elif' / '#elseif'
    else if ( t.tokenType == asTC_KEYWORD && strToken == "else" )
    {
        // Check for validity
        if ( ConditionStack.empty() )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unexpected #%s encountered. No matching #if, #ifdef or #ifndef directive.\n"), strToken.c_str() );
            return false;
        
        } // End if malformed

        // Overwrite the #directive with spaces to avoid compiler error
        overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );

        // If condition has already been met on a prior occasion, strip.
        // Otherwise, condition has now been met!
        if ( ConditionStack.top() == true )
        {
            // Strip up to the next conditional directive
            excludeCode( t, strScript );
        
        } // End if already consumed
        else
            ConditionStack.top() = true;

    } // End if '#else'
    else if ( t.tokenType == asTC_IDENTIFIER && (strToken == "ifdef" || (bIfndef = (strToken == "ifndef"))) )
    {
        // Retrieve word identifier.
        if ( !getNextToken( t, strScript, true ) )
        {
            cgToDoAssert( "Script Preprocessor", "Missing definition identifier." );
            return false;
        
        } // End if failed
        if ( t.tokenType == asTC_IDENTIFIER )
        {
            strToken.assign( &strScript[t.tokenStart], t.tokenLength );

            // Overwrite the #directive with spaces to avoid compiler error
            overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );

            // Has this identifier has not been defined by the application (or !defined).
            bool bResult = ((mDefinitions.find( strToken ) == mDefinitions.end()) == bIfndef );

            // Push the result onto the condition stack
            ConditionStack.push( bResult );

            // Strip up to the next conditional directive if result is negative.
            if ( !bResult )
                excludeCode( t, strScript );
            
        } // End if identifier
        else
        {
            cgToDoAssert( "Script Preprocessor", "Missing definition identifier." );
            return false;

        } // End if !identifier

    } // End if '#ifdef' || '#ifndef'
    else if ( strToken == "define" )
    {
        // Retrieve identifier.
        if ( !getNextToken( t, strScript, true ) )
        {
            cgToDoAssert( "Script Preprocessor", "Missing definition identifier." );
            return false;
        
        } // End if Error
        if ( t.tokenType == asTC_IDENTIFIER )
        {
            std::string strWord, strValue;
            strWord.assign( &strScript[t.tokenStart], t.tokenLength );

            // Skip to first non whitespace character.
            t.position = strScript.find_first_not_of( " \t", t.position );
            
            // Retrieve the definition value (up to the first newline)
            // ToDo: We may want to support '/' newline traversal.
            size_t nNewLine = strScript.find_first_of( "\r\n", t.position );
            if ( nNewLine == std::string::npos )
                nNewLine = t.sectionEnd + 1;
            strValue.assign( &strScript[t.position], nNewLine - t.position );
            t.position = nNewLine;

            // Overwrite the #define directive with spaces to avoid compiler error
            overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );

            // Add the definition to our list if not already specified
            if ( mDefinitions.find( strWord ) != mDefinitions.end() )
            {
                cgToDoAssert( "Script Preprocessor", "Duplicate definition." );
                // ToDo: Error
            
            } // End if already defined
            else
            {
                mDefinitions[ strWord ] = strValue;

            } // End if insert

        } // End if identifier
        else
        {
            cgToDoAssert( "Script Preprocessor", "Missing definition identifier." );
            // ToDo: Error

        } // End if !identifier

    } // End if '#define'
    else if ( strToken == "undef" )
    {
        // Retrieve identifier.
        if ( !getNextToken( t, strScript, true ) )
        {
            cgToDoAssert( "Script Preprocessor", "Missing definition identifier." );
            // ToDo: Error
            return false;
        
        } // End if Error
        if ( t.tokenType == asTC_IDENTIFIER )
        {
            std::string strWord, strValue;
            strWord.assign( &strScript[t.tokenStart], t.tokenLength );

            // Overwrite the #undef directive with spaces to avoid compiler error
            overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );

            // Remove from the word from our definition list
            mDefinitions.erase( strWord );

        } // End if identifier
        else
        {
            cgToDoAssert( "Script Preprocessor", "Missing definition identifier." );

        } // End if !identifier

    } // End if '#undef'
    else if ( strToken == "include" )
    {
        // Retrieve word identifier.
        if ( !getNextToken( t, strScript, true ) )
        {
            cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
            return false;
        
        } // End if failed
        if ( t.tokenType == asTC_VALUE && t.tokenLength > 2 && strScript[t.tokenStart] == '"' )
        {
            // Retrieve the referenced include file
            std::string strInclude;
            strInclude.assign( &strScript[t.tokenStart + 1], t.tokenLength - 2 );

            // Include referenced headers.
            std::string strParentScript( "\"" + std::string(stringConvertT2CA(strSectionName.c_str())) + "\"" );
            cgString strParentPath = cgFileSystem::getDirectoryName( strSectionName );
            if ( strParentPath.empty() == false )
                strParentPath += _T("/");

            // If the include file contains an absolute path protocol, then
            // just use it as is. Otherwise, prefix the current file's path.
            cgString strFile = stringConvertA2CT(strInclude.c_str());
            if ( strFile.find( _T("://") ) == cgString::npos )
                strFile = strParentPath + strFile;
            
            // All child scripts have '_PARENTSCRIPT' defined. First capture any previously
            // defined entry so that it can be restored once the included file has been processed.
            std::string strPrevParentScript;
            bool bPrevParentStringDefined = false;
            DefinitionMap::const_iterator itPrevParentScript = mDefinitions.find( "_PARENTSCRIPT" );
            if ( (bPrevParentStringDefined = (itPrevParentScript != mDefinitions.end()) ) )
                strPrevParentScript = itPrevParentScript->second;

            // Define current parent script.
            mDefinitions[ "_PARENTSCRIPT" ] = strParentScript;

            // Load the file
            if ( !loadScriptSection( pModule, cgInputStream( strFile ), aSourceFiles ) )
                return false;

            // Restore previous parent script macro (or remove if it didn't exist)
            if ( bPrevParentStringDefined )
                mDefinitions[ "_PARENTSCRIPT" ] = strPrevParentScript;
            else
                mDefinitions.erase( "_PARENTSCRIPT" );

        } // End if value

        // Remove the include directive to avoid compilation errors
        overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );
    
    } // End if '#include'
    else if ( strToken == "endif" )
    {
        // Only remove the #endif if there was a matching #if
        if ( ConditionStack.empty() )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unexpected #%s encountered. No matching #if, #ifdef or #ifndef directive.\n"), strToken.c_str() );
            return false;
        
        } // End if malformed

        // Strip directive
        overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );
        ConditionStack.pop();

    } // End if '#endif'

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseScriptString () (Protected)
/// <summary>
/// Parse an angel script side string literal value and resolve any references
/// that it contains. For instance "Welcome $name" will inject code to resolve
/// '$name' to include the actual contents of angel script variable 'name'.
/// This only applies to surface shader scripts.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::parseScriptString( TokenData & t, std::string & strScript, bool shaderParameter, bool addParentheses )
{
    bool bModified   = false;
    bool bProcessing = !shaderParameter; // We're not automatically processing data immediately when processing a __shadercall parameter

    // Search the string for the '$' symbol and replace as appropriate.
    std::string strInterior, strWhitespace;
    strInterior.reserve( (t.sectionEnd - t.sectionBegin) + 1 );
    for ( size_t i = t.position; i <= t.sectionEnd; ++i )
    {
        cgChar c = strScript[i];
        bool bProcessFurther = true;
        if ( !bProcessing && (c == ' ' || c == '\t' || c == '\n' || c == '\r') )
        {
            // Whitespace should be buffered and output only when necessary.
            strWhitespace += c;
            bProcessFurther = false;

        } // End if whitespace
        else if ( c == '$' )
        {
            // Variable references "$Identifier". Does this use scoping parentheses
            // to denote more than just a single identifier (i.e. "$(Identifier.Member)")
            cgChar nc;
            if ( (i+1) <= t.sectionEnd && (nc = strScript[i+1]) == '(' )
            {
                // Uses scoping parentheses. Find the closing paren.
                cgInt nDepth = 1;
                size_t nEndParen = i+2;
                for ( ; nEndParen <= t.sectionEnd; ++nEndParen )
                {
                    nc = strScript[nEndParen];
                    if ( nc == '(')
                        ++nDepth;
                    else if ( nc == ')' )
                    {
                        if ( --nDepth == 0 )
                            break;
                    
                    } // End if ')'
                
                } // Next character
                
                if ( nEndParen <= t.sectionEnd )
                {
                    cgInt nIdentifierLen = (cgInt)nEndParen - (i+2);

                    // For shader parameters, if we weren't currently processing this as 
                    // legitimate parameter data so far, then we need to dump out the whitespace
                    // before we do anything and output the variable reference AS IT IS.
                    // In all other cases, we need to close the currently open string, 
                    // output the variable reference, and then re-open the string.
                    if ( shaderParameter && !bProcessing )
                    {
                        strInterior.append( strWhitespace );
                        strWhitespace.clear();

                        // Note: The value is wrapped in parenthesis to avoid any
                        // operator precedence confusion in the final generated code.
                        // i.e. "$MyVar;" becomes "(MyVar);"
                        strInterior += '(';
                        strInterior.append( strScript.substr( i+2, nIdentifierLen ) );
                        strInterior += ')';

                    } // End if shader parameter
                    else
                    {
                        // Break the string and insert the variable value as a string.
                        // Note: The value is wrapped in parenthesis to avoid any
                        // operator precedence confusion in the final generated code.
                        // i.e. "Value=$MyVar;" becomes "Value=(" + String(MyVar) + ");"
                        strInterior.append( addParentheses ? "(\" + (" : "\" + (" );
                        strInterior.append( strScript.substr( i+2, nIdentifierLen ) );
                        strInterior.append( addParentheses ? ") + \")" : ") + \"" );

                    } // End if !shader parameter

                    // We modified the script code.
                    i += nIdentifierLen + 2; // +2 to skip both braces.
                    bModified = true;
                    bProcessFurther = false;

                } // End if found

                // NB: If we didn't find a closing parenthesis, we will automatically
                // drop into the standard character handling case and the dollar
                // symbol will simply be output.

            } // End if scoped
            else
            {
                // Just find end of first identifier.
                size_t nEndIdentifier = strScript.find_first_not_of( IdentifierChars, i + 1 );
                if ( nEndIdentifier == std::string::npos )
                    nEndIdentifier = t.sectionEnd + 1;
                size_t nLength = (cgInt)nEndIdentifier - (i+1);

                // For shader parameters, if we weren't currently processing this as 
                // legitimate parameter data so far, then we need to dump out the whitespace
                // before we do anything and output the variable reference AS IT IS.
                // In all other cases, we need to close the currently open string, 
                // output the variable reference, and then re-open the string.
                if ( shaderParameter && !bProcessing )
                {
                    strInterior.append( strWhitespace );
                    strWhitespace.clear();

                    // Note: The value is wrapped in parenthesis to avoid any
                    // operator precedence confusion in the final generated code.
                    // i.e. "$MyVar;" becomes "(MyVar);"
                    strInterior += '(';
                    strInterior.append( strScript.substr( i+1, nLength ) );
                    strInterior += ')';

                } // End if shader parameter
                else
                {
                    // Break the string and insert the variable value as a string.
                    // Note: The value is wrapped in parenthesis to avoid any
                    // operator precedence confusion in the final generated code.
                    // i.e. "Value=$MyVar;" becomes "Value=(" + String(MyVar) + ");"
                    strInterior.append( addParentheses ? "(\" + (" : "\" + (" );
                    strInterior.append( strScript.substr( i+1, nLength ) );
                    strInterior.append( addParentheses ? ") + \")" : ") + \"" );

                } // End if !shader parameter

                // We modified the script code.
                i += nLength;
                bModified = true;
                bProcessFurther = false;

            } // End if !scoped

        } // End if '$'
        
        // Should we process the character further?
        if ( bProcessFurther )
        {
            // Building shader parameter or not?
            if ( shaderParameter )
            {
                // For shader parameters, if we weren't currently processing this as 
                // legitimate parameter data so far, then we need to open the parameter
                // string and dump any buffered whitespace before we do so. Everything
                // else in this case will be assumed to be part of a string being built.
                if ( !bProcessing )
                {
                    // If the buffer has been modified already, but we're not actually 
                    // processing, then we need to APPEND to the existing data. An example
                    // of such a case would be "$foo.bar" where '$foo' is a script variable.
                    if ( bModified )
                        strInterior += " + ";

                    // Now open the string before outputting
                    strInterior += '"';
                    strInterior.append( strWhitespace );
                    strWhitespace.clear();
                    bProcessing = true;
                    bModified = true;

                } // End if !processing

                // Process "invalid" characters before we output them.
                switch ( c )
                {
                    case 0:     // null
                        strInterior += "\\0";
                        bModified = true;
                        break;
                    case 8:     // backspace
                        strInterior += "\\x8";
                        bModified = true;
                        break;
                    case 9:     // horizontal tab
                        strInterior += "\\t";
                        bModified = true;
                        break;
                    case 10:    // new line
                        // We respect the physical newline here by closing the
                        // output and re-opening on a new line in order to ensure 
                        // line counts remain the same.
                        strInterior += "\\n\"\n\"";
                        bModified = true;
                        break;
                    case 13:    // carriage return
                        // Carriage returns are stripped.
                        bModified = true;
                        break;
                    case 26:    // substitute
                        strInterior += "\\x1A";
                        bModified = true;
                        break;
                    case 34:    // double quote
                        strInterior += "\\\"";
                        bModified = true;
                        break;
                    case 39:    // single quote
                        strInterior += "\\'";
                        bModified = true;
                        break;
                    case 92:    // backslash
                        strInterior += "\\\\";
                        bModified = true;
                        break;
                    case 96:    // grave accent
                        strInterior += "\\`";
                        bModified = true;
                        break;

                    default:
                        strInterior += c;
                        break;

                } // End switch
            
            } // End if shader parameter
            else
            {
                // Just output the character
                strInterior += c;
            
            } // End if !shader parameter
            
        } // End if other character

    } // Next Character

    // If we were processing shader parameter data and we reached the end
    // then there are two potential cases to resolve. If we opened a string
    // we need to dump whitespace and close it. Otherwise we need simply
    // output the whitespace and we're done.
    if ( shaderParameter )
    {
        strInterior.append( strWhitespace );
        if ( bProcessing )
        {
            strInterior += '"';
            bModified = true;
        
        } // End if processing
    
    } // End if shaderParameter

    // If we discovered that changes were necessary, replace the script
    // code with the new 'interior' section.
    if ( bModified == true )
    {
        // Enough space in the script string to just replace the existing block?
        size_t nNewSize = strInterior.length();
        size_t nOldSize = (t.sectionEnd - t.sectionBegin) + 1;
        if ( nNewSize == nOldSize )
        {
            // Exact match in size. We can just replace.
            strScript.replace( t.sectionBegin, nNewSize, strInterior );
        
        } // End if exact size
        else if ( nNewSize < nOldSize )
        {
            // Smaller. We can replace the first part, and clear the rest.
            strScript.replace( t.sectionBegin, nNewSize, strInterior );
            overwriteCode( strScript, t.sectionBegin + nNewSize, nOldSize - nNewSize );
            
        } // End if smaller
        else
        {
            // Code is larger, we can replace the contents entirely but we must 
            // also adjust the current read position to take this expanded string 
            // size into account.
            strScript.replace( t.sectionBegin, nOldSize, strInterior );
            t.sectionEnd += (cgInt)nNewSize - (cgInt)nOldSize;

        } // End if larger  

    } // End if code modified

    // Move to end of section
    t.position = t.sectionEnd + 1;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseShaderBlock () (Protected)
/// <summary>
/// Parse any included shader script replacement block '<?[type] block ?>' and
/// generate the appropriate angel script code to put in its place.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::parseShaderBlock( TokenData & t, std::string & strScript, const std::string & strNamespace, bool bShaderCallFunction )
{
    STRING_CONVERT;
    cgInt nType = 0; // Code

    // Start of a shader script replacement block '<?[type] block ?>'
    size_t nBlockStart = t.tokenStart;

    // First determine if there is an (optional) type.
    t.position++; // Skip '?'
    getNextToken( t, strScript, false );
    if ( t.tokenType != asTC_WHITESPACE )
    {
        // Type is possibly supplied (i.e. 'common', 'in', 'out', etc.) Extract it.
        std::string strType;
        strType.assign( &strScript[t.tokenStart], t.tokenLength );

        // If it is not one of our reserved type names, this is just code.
        if ( stricmp( strType.c_str(), "in" ) == 0 )
            nType = 1;
        else if ( stricmp( strType.c_str(), "out" ) == 0 )
            nType = 2;
        else if ( stricmp( strType.c_str(), "cbufferrefs" ) == 0 )
            nType = 3;
        else if ( stricmp( strType.c_str(), "cbuffer" ) == 0 )
            nType = 4;
        else if ( stricmp( strType.c_str(), "ctype" ) == 0 )
            nType = 5;
        else if ( stricmp( strType.c_str(), "common" ) == 0 )
            nType = 6;
        else if ( stricmp( strType.c_str(), "samplerrefs" ) == 0 )
            nType = 7;
        else if ( stricmp( strType.c_str(), "samplers" ) == 0 )
            nType = 8;
        else if ( stricmp( strType.c_str(), "global" ) == 0 )
            nType = 9;
        
        // This was not an identifier! Re-parse as code.
        if ( nType == 0 )
            t.position = t.tokenStart;
    
    } // End if optional type
    else
    {
        // We must consume white-space in order to ensure we maintain
        // correct line numbering.
        t.position = t.tokenStart;
    
    } // End if whitespace

    // We've found the start of the replacement block interior, now find the end.
    size_t nInteriorStart = t.position;
    size_t nInteriorEnd   = strScript.find( "?>", nInteriorStart );
    if ( nInteriorEnd == std::string::npos )
    {
        cgToDoAssert( "Script Preprocessor", "Expected closing tag '?>' for shader code section." );
        return false;
    
    } // End if invalid
    --nInteriorEnd;

    // Process based on the type of the block
    std::string strNewInterior;
    if ( nType == 4 )
    {
        // Constant buffer block.
        // Parse interior and generate CBuffer descriptor
        cgConstantBufferDesc Desc;
        TokenData tBuffer = t;
        tBuffer.sectionBegin = nInteriorStart;
        tBuffer.sectionEnd   = nInteriorEnd;
        if ( !parseCBufferDescriptor( tBuffer, strScript, Desc, strNamespace ) )
            return false;

        // Store the cbuffer descriptor for later retrieval and request a handle for it.
        cgInt32 nHandle = ((cgSurfaceShaderScript*)mScript)->addConstantBufferDesc( Desc );
        if ( nHandle < 0 )
        {
            cgToDoAssert( "Script Preprocessor", "Duplicate constant buffer declaration." );
            return false;
        
        } // End if duplicate
        
        // Build new interior code that simply returns the
        // handle for the constant buffer.
        std::stringstream strConcat;
        strConcat << "int __shcb_" << stringConvertT2CA(Desc.name.c_str()) << "(){return " << nHandle << ";}";
        strNewInterior = strConcat.str();

    } // End if cbuffer
    else if ( nType == 5 )
    {
        // Constant type block.
        // Parse interior and generate type descriptor
        cgConstantTypeDesc Desc;
        TokenData tType = t;
        tType.sectionBegin = nInteriorStart;
        tType.sectionEnd   = nInteriorEnd;
        if ( !parseTypeDescriptor( tType, strScript, Desc, strNamespace ) )
            return false;

        // Store the ctype descriptor for later retrieval and request a handle for it.
        cgInt32 nHandle = ((cgSurfaceShaderScript*)mScript)->addConstantTypeDesc( Desc );
        if ( nHandle < 0 )
        {
            cgToDoAssert( "Script Preprocessor", "Duplicate constant type declaration." );
            return false;
        
        } // End if duplicate

    } // End if ctype
    else if ( nType == 8 )
    {
        // Sampler block
        // Parse interior and generate sampler descriptor
        cgSamplerBlockDesc Desc;
        TokenData tSamplers = t;
        tSamplers.sectionBegin = nInteriorStart;
        tSamplers.sectionEnd   = nInteriorEnd;
        if ( !parseSamplerBlock( tSamplers, strScript, Desc, strNamespace ) )
            return false;

        // Store the sampler descriptor for later retrieval and request a handle for it.
        cgInt32 nHandle = ((cgSurfaceShaderScript*)mScript)->addSamplerBlockDesc( Desc );
        if ( nHandle < 0 )
            return false;
        
        // Build new interior code that simply returns the
        // handle for the sampler block.
        std::stringstream strConcat;
        strConcat << "int __shsmp_" << stringConvertT2CA(Desc.name.c_str()) << "(){return " << nHandle << ";}\n";
        strNewInterior = strConcat.str();

    } // End if samplers
    else
    {
        // Build a new interior code string.
        strNewInterior.reserve( (nInteriorEnd - nInteriorStart) + 1 );

        // Process the interior section of the script code.
        TokenData tCode = t;
        tCode.sectionBegin = nInteriorStart;
        tCode.sectionEnd   = nInteriorEnd;

        // Search for all identifiers in this block of code. We're going to attempt 
        // to determine if they match up to any shader call functions that were declared.
        // If they do, then we'll inject the script code necessary to get the call made.
        // Otherwise we'll replace invalid characters with their string counterparts.
        while ( getNextToken( tCode, strScript, false ) )
        {
            // Get the token.
            std::string strBuffer = strScript.substr( tCode.tokenStart, tCode.tokenLength );

            // If it's an identifier in a code block, we may be able to replace it with a shader call.
            if ( nType == 0 && tCode.tokenType == asTC_IDENTIFIER )
            {
                // Shader call exists with this name?
                STRING_CONVERT;
                cgInt32Array aFuncHandles = ((cgSurfaceShaderScript*)mScript)->findShaderCallFunc( stringConvertA2CT(strNamespace.c_str()), stringConvertA2CT(strBuffer.c_str()) );
                if ( !aFuncHandles.empty() )
                {
                    size_t nCallStart = tCode.tokenStart;

                    // Close the current shader string and start outputting function call.
                    std::string strNewCode = "\"+";
                    strNewCode.append( strBuffer );

                    // First token should be the opening parenthesis. Note: throughout this method, special 
                    // handling is required in order to ensure that we maintain an accurate representation of 
                    // all of the white-space used in the original code (including newlines). This ensures
                    // that the line-numbering in the output script remains unaltered.
                    bool bResult;
                    while ( (bResult = getNextToken(tCode, strScript, false)) && tCode.tokenType == asTC_WHITESPACE )
                        strNewCode.append( strScript.substr( tCode.tokenStart, tCode.tokenLength ) );
                    if ( !bResult || tCode.tokenLength != 1 || tCode.tokenType != asTC_KEYWORD || strScript.at( tCode.tokenStart) != '(' )
                        break;
                    strNewCode.append( strScript.substr( tCode.tokenStart, tCode.tokenLength ) );

                    // What follows should be the parameter list, keep processing until we find the closing paren.
                    cgInt nCallParenDepth = 1;
                    std::string strParameter;
                    bool bNewParam = true, bInShaderParam = false;
                    while ( bResult = getNextToken(tCode, strScript, false) )
                    {
                        if ( tCode.tokenType == asTC_WHITESPACE )
                        {
                            // Always append whitespace to preserve line count.
                            //strNewCode.append( strScript.substr( tCode.tokenStart, tCode.tokenLength ) );
                            strParameter.append( strScript.substr( tCode.tokenStart, tCode.tokenLength ) );

                        } // End if whitespace
                        else
                        {
                            cgChar c = strScript.at(tCode.tokenStart);
                            if ( nCallParenDepth == 1 && tCode.tokenLength == 1 && (c == ',' || c == ')') )
                            {
                                // Moving on to next parameter, or closing the function call.
                                // Parse the parameter data.
                                TokenData tParam;
                                tParam.position     = 0;
                                tParam.sectionBegin = 0;
                                tParam.sectionEnd   = strParameter.size() -1;
                                parseScriptString( tParam, strParameter, true, true );

                                // Append the parameter and the comma / closing paren
                                strNewCode += strParameter;
                                strNewCode += c;

                                // We're done with the parameter ready for the next.
                                strParameter.clear();

                                // Break out once we reach the final closing parenthesis?
                                if ( c == ')' && --nCallParenDepth == 0 )
                                    break;
                                
                            } // End if next/final param
                            else if ( tCode.tokenLength == 1 && tCode.tokenType == asTC_KEYWORD )
                            {
                                // What keyword character is this?
                                if ( c == '(' )
                                {
                                    // Append opening paren
                                    strParameter += c;

                                    // We're one level deeper into the nested parens
                                    ++nCallParenDepth;

                                } // End if open paren
                                else if ( c == ')' )
                                {
                                    // Append closing paren
                                    strParameter += c;

                                    // We've moved one level back up in the nested params.
                                    --nCallParenDepth;

                                } // End if close paren
                                else
                                    strParameter += c;
                            
                            } // End if 1 character keyword
                            else
                            {
                                strParameter.append( strScript.substr( tCode.tokenStart, tCode.tokenLength ) );
                            
                            } // End if other token

                        } // End if !whitespace

                    } // Next token

                    // If we found the final closing parenthesis then we can process.
                    if ( nCallParenDepth == 0 )
                    {
                        // Re-open the shader block again.
                        strNewCode.append( "+\"" );

                        // Append to the new interior code directly, do NOT parse characters
                        strNewInterior.append( strNewCode );
                        strBuffer.clear();

                    } // End if found closing parens

                } // End if found function

            } // End if identifier

            // If there is anything in the code append buffer, clean it up
            // before appending it to the new interior code string.
            if ( !strBuffer.empty() )
            {
                // Process interior contents, adding slashes and replacing control characters
                // where it is appropriate to do so.
                cgChar pc = 0; // Previous character
                for ( size_t i = 0; i < strBuffer.size(); ++i )
                {
                    cgChar c = strBuffer[i];
                    switch ( c )
                    {
                        case 0:     // null
                            strNewInterior += "\\0";
                            break;
                        case 8:     // backspace
                            strNewInterior += "\\x8";
                            break;
                        case 9:     // horizontal tab
                            strNewInterior += "\\t";
                            break;
                        case 10:    // new line
                            // We respect the physical newline here by closing the
                            // output and re-opening on a new line in order to ensure 
                            // line counts remain the same.
                            strNewInterior += "\\n\"\n\"";
                            break;
                        case 13:    // carriage return
                            // Carriage returns are stripped.
                            break;
                        case 26:    // substitute
                            strNewInterior += "\\x1A";
                            break;
                        case 34:    // double quote
                            strNewInterior += "\\\"";
                            break;
                        case 39:    // single quote
                            strNewInterior += "\\'";
                            break;
                        case 92:    // backslash
                            strNewInterior += "\\\\";
                            break;
                        case 96:    // grave accent
                            strNewInterior += "\\`";
                            break;
                        
                        default:
                            strNewInterior += c;
                            break;

                    } // End switch
                    pc = c;

                } // Next character
            
            } // End if data to append

        } // Next token       

        // Parse for variable referenced ($)
        TokenData tNew;
        tNew.position     = 0;
        tNew.sectionBegin = 0;
        tNew.sectionEnd   = strNewInterior.size() -1;
        parseScriptString( tNew, strNewInterior, false );

        // Wrap generated string with correct output variable based on type
        // i.e. __sh_code("[Interior Contents]");
        if ( nType == 1 )                   // Shader inputs
            strNewInterior = "__shi.append(\"" + strNewInterior + "\");";
        else if ( nType == 2 )              // Shader outputs
            strNewInterior = "__sho.append(\"" + strNewInterior + "\");";
        else if ( nType == 3 )              // cbuffer references
            strNewInterior = "__cbr.append(\"" + strNewInterior + "\");";
        else if ( nType == 6 )              // Common definitions/declarations
            strNewInterior = "String __shcmn(){return \""  + strNewInterior + "\";}";
        else if ( nType == 7 )              // Sampler references
            strNewInterior = "__sbr.append(\"" + strNewInterior + "\");";
        else if ( nType == 9 )              // Global code
            strNewInterior = "__shg.append(\"" + strNewInterior + "\");";
        else                                // Shader code
            strNewInterior = "__sh_code(\"" + strNewInterior + "\");";
        
    } // End if other type

    // Skip to just past the end of the block close tag.
    t.position = nInteriorEnd + 3;

    // Enough space in the script string to just replace the existing block?
    size_t nNewSize = strNewInterior.length();
    if ( nNewSize == (t.position - nBlockStart) )
    {
        // Exact match in size. We can just replace.
        strScript.replace( nBlockStart, nNewSize, strNewInterior );
    
    } // End if exact size
    else if ( nNewSize < (t.position - nBlockStart) )
    {
        // Smaller. We can replace the first part, and clear the rest.
        strScript.replace( nBlockStart, nNewSize, strNewInterior );
        overwriteCode( strScript, nBlockStart + nNewSize, t.position - (nBlockStart + nNewSize) );

    } // End if smaller
    else
    {
        // Code is larger, we can replace the contents entirely but we must 
        // also adjust the current read position to take this expanded string 
        // size into account.
        strScript.replace( nBlockStart, (t.position - nBlockStart), strNewInterior );
        t.sectionEnd += (cgInt)nNewSize - (cgInt)(t.position - nBlockStart);
        t.position    = nBlockStart + nNewSize;

    } // End if larger

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : processScriptSection () (Protected)
/// <summary>
/// Perform pre-processor pass on the script to include any referenced
/// headers, defines, etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::processScriptSection( asIScriptModule * pModule, const std::string & strInputCode, const cgString & strSectionName, cgScript::SourceFileArray & aSourceFiles )
{
    STRING_CONVERT; // For string conversion macros.

    std::string              strScript = strInputCode, strToken, strNamespace;
    cgInt                    nNamespaceDepth = 0, nFunctionDepth = 0;
    bool                     bFunctionParsing = false;
    std::stack<bool>         ConditionStack;
    cgShaderCallFunctionDesc ShaderCallDesc;
    cgStringArray            Includes;

    // First perform the checks for #if directives to exclude code that shouldn't be compiled
    TokenData t;
    t.sectionBegin = 0;
    t.sectionEnd   = strScript.size() - 1;
    t.position     = 0;
    while ( t.position <= t.sectionEnd )
    {
        // Retrieve next available token skipping comments and whitespace.
        if ( !getNextToken( t, strScript, true ) )
            break;
        
        // Is this a pre-processor directive?
        if ( t.tokenType == asTC_UNKNOWN && strScript[ t.tokenStart ] == '#' )
        {
            if ( !parsePreprocessorDirective( t, strScript, ConditionStack, pModule, strSectionName, aSourceFiles ) )
                return false;

        } // End if '#'
        else if ( mShaderScript == true && t.tokenType == asTC_VALUE && strScript[ t.tokenStart ] == '\"' )
        {
            TokenData tString;
            tString.position     = t.tokenStart;
            tString.sectionBegin = t.tokenStart;
            tString.sectionEnd   = t.tokenStart + (t.tokenLength - 1);
            if ( !parseScriptString( tString, strScript, false ) )
                return false;

            // We're now positioned after the string and the script has grown
            // appropriately.
            t.sectionEnd += tString.position - t.position;
            t.position    = tString.position;

        } // End if '"'
        else if ( mShaderScript == true && t.tokenType == asTC_KEYWORD && t.tokenLength == 1 && 
                  strScript[t.tokenStart] == '<' && (t.tokenStart < (strScript.length() - 2)) && 
                  strScript[t.tokenStart+1] == '?' )
        {
            if ( !parseShaderBlock( t, strScript, strNamespace, bFunctionParsing ) )
                return false;
            
        } // End if '<?'
        else if ( t.tokenType == asTC_KEYWORD && strNamespace.empty() && !bFunctionParsing &&
                  strScript.substr( t.tokenStart, t.tokenLength ) == "class" )
        {
            // Next token should be the class name.
            if ( getNextToken( t, strScript, true ) )
            {
                if ( t.tokenType != asTC_IDENTIFIER )
                {
                    cgToDoAssert( "Script Preprocessor", "Expected identifier." );
                    return false;
                
                } // End if !identifier
                strNamespace = strScript.substr( t.tokenStart, t.tokenLength );

            } // End if class name

        } // End if 'class'
        else if ( t.tokenType == asTC_IDENTIFIER && !bFunctionParsing && 
                  strScript.substr( t.tokenStart, t.tokenLength ) == "__shadercall" )
        {
            if ( !parseShaderCallDeclaration( t, strScript, ShaderCallDesc, strNamespace ) )
                return false;

            // Add this function to the list of those identified so far.
            cgInt32 nHandle = ((cgSurfaceShaderScript*)mScript)->addShaderCallFuncDesc( ShaderCallDesc );
            if ( nHandle < 0 )
            {
                cgToDoAssert( "Script Preprocessor", "Duplicate __shadercall function declaration." );
                return false;

            } // End if duplicate

            // We are now parsing the internals of the shader call function.
            bFunctionParsing = true;

        } // End if __shadercall
        else if ( t.tokenType == asTC_KEYWORD && t.tokenLength == 1 && strScript.at( t.tokenStart ) == '{' )
        {
            if ( !strNamespace.empty() )
                ++nNamespaceDepth;

            // Inside a shader call function?
            if ( bFunctionParsing )
            {
                // If this is the opening brace for a shader call function, we need
                // to create some local variables.
                if ( nFunctionDepth == 0 )
                {
                    STRING_CONVERT;

                    // Generate new function header. This is all placed on a single line with the
                    // opening brace in order to ensure that we maintain accurate line-numbering.
                    std::string strNewInterior = "{String __shxLocal, __shDecl=\"" + std::string(stringConvertT2CA(ShaderCallDesc.declaration.c_str())) + "\",";
                    strNewInterior += "__shFunc=\"" + std::string(stringConvertT2CA( ShaderCallDesc.name.c_str() )) + "\",";
                    if ( !ShaderCallDesc.callingParams.empty() )
                        strNewInterior += "__shParams=" + std::string(stringConvertT2CA( ShaderCallDesc.callingParams.c_str() )) + ";";
                    else
                        strNewInterior += "__shParams;";
                    strNewInterior += "__sh_push_code_output( __shxLocal );";

                    // We must now replace the single brace with the new function header,
                    // but we must  also adjust the current read position to take this expanded 
                    // string size into account.
                    size_t nNewSize = strNewInterior.length();
                    strScript.replace( t.tokenStart, 1, strNewInterior );
                    t.sectionEnd += (cgInt)nNewSize - 1;
                    t.position    = t.tokenStart + nNewSize;

                } // End if first brace
                ++nFunctionDepth;

            } // End if __shadercall function

        } // End if '{'
        else if ( t.tokenType == asTC_KEYWORD && t.tokenLength == 1 && strScript.at( t.tokenStart ) == '}' )
        {
            if ( bFunctionParsing )
            {
                // Is this the closing brace for a shader call function?
                if ( --nFunctionDepth == 0 )
                {
                    // Inject footer code required to complete the shader call generation.
                    std::string strNewInterior = "__sh_pop_code_output();";
                    strNewInterior += "return __sh_generate_shader_call( __shDecl, __shFunc, __shParams, __shxLocal );}";
                    
                    // We must now replace the single brace with the new function header,
                    // but we must  also adjust the current read position to take this expanded 
                    // string size into account.
                    size_t nNewSize = strNewInterior.length();
                    strScript.replace( t.tokenStart, 1, strNewInterior );
                    t.sectionEnd += (cgInt)nNewSize - 1;
                    t.position    = t.tokenStart + nNewSize;

                    // Reset shader call tracking variables
                    ShaderCallDesc.name.clear();
                    ShaderCallDesc.declaration.clear();
                    ShaderCallDesc.parameters.clear();
                    ShaderCallDesc.parentNamespace.clear();
                    ShaderCallDesc.returnType.clear();
                    ShaderCallDesc.requiresReturn = false;
                    bFunctionParsing = false;
                
                } // End if closing brace
            
            } // End if parsing function

            if ( !strNamespace.empty() )
            {
                if ( --nNamespaceDepth == 0 )
                    strNamespace.clear();
            
            } // End if in namespace

        } // End if '{'
        else if ( t.tokenType == asTC_IDENTIFIER )
        {
            // Try macro expansion
            TokenData tStatement;
            tStatement.sectionBegin = t.tokenStart;
            tStatement.sectionEnd   = (t.tokenStart + t.tokenLength) - 1;
            tStatement.position     = t.tokenStart;
            if ( expandStatement( tStatement, strScript ) )
            {
                // Script was modified. Adjust section values in case the size of the
                // script itself was altered.
                t.sectionEnd += (cgInt)((tStatement.sectionEnd - tStatement.sectionBegin) + 1) - (cgInt)t.tokenLength;
                t.position   = tStatement.sectionEnd + 1;
            
            } // End if modified
                
        } // End if identifier
        
    } // Next Token

    // ToDo: Debug output.
#if defined(_DEBUG)
    static int nf = 0;
    char fname[256];
    const cgChar * lpszScriptName = stringConvertT2CA(cgFileSystem::getFileName(strSectionName,true).c_str());
    sprintf( fname, "Build/as_%s_%04i.txt", lpszScriptName, nf );
    nf++;
    FILE * pf = fopen( fname, "w" );
    if ( pf )
    {
        fwrite( strScript.c_str(), strScript.size(), 1, pf );
        fclose( pf );
    }
#endif

    // Add the current code to the module.
    mEngine->SetEngineProperty( asEP_COPY_SCRIPT_SECTIONS, true );
    if ( pModule->AddScriptSection( stringConvertT2CA(strSectionName.c_str()), strScript.c_str(), strScript.size() ) < 0 )
    {
        cgAppLog::write( cgAppLog::Error, _T("An error occured while attempting to add code to script '%s'.\n"), mScript->getResourceName().c_str() );
        return false;
    
    } // End if failed to add section

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseCBufferDescriptor () (Protected)
/// <summary>
/// Parse the specified script code to determine the layout and description for
/// any following constant. Also moves the specified TokenData::position 
/// variable past the C-Buffer content.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::parseCBufferDescriptor( TokenData & t, const std::string & strScript, cgConstantBufferDesc & Desc, const std::string & strNamespace )
{
    // Type parser works for CBuffers too.
    if ( !parseTypeDescriptor( t, strScript, Desc, strNamespace ) )
        return false;

    // Pre-parse certain common cbuffer parameters.
    Desc.bufferRegister     = Desc.parameters.getProperty( _T("register"), -1 );
    Desc.globalOffset       = Desc.parameters.getProperty( _T("globaloffset"), -1 );
    Desc.bindVS             = false;
    Desc.bindPS             = false;
    
    // Parse shader type constraint information.
    cgStringArray aTokens;
    cgString strShaderConstraints = Desc.parameters.getProperty( _T("shaderconstraints"), _T("vs|ps") );
    if ( cgStringUtility::tokenize( strShaderConstraints, aTokens, _T("|") ) )
    {
        for ( size_t i = 0; i < aTokens.size(); ++i )
        {
            cgString strValue = cgString::toLower( aTokens[i].trim() );
            if ( strValue == _T("vs") )
                Desc.bindVS = true;
            else if ( strValue == _T("ps") )
                Desc.bindPS = true;

        } // Next token

    } // End if tokenized
    
    // Success!
    return true;
}
/*bool cgScriptPreprocessor::parseCBufferDescriptor( TokenData & t, const std::string & strScript, cgConstantBufferDesc & Desc )
{
    STRING_CONVERT;
    std::stringstream Parser;

    // Initialize constant buffer description structure.
    Desc.bufferRegister = -1;
    Desc.globalOffset   = -1;
    Desc.length         = 0;
    Desc.IsPacked       = false;
    Desc.constants.clear();

    // Retrieve the C buffer identifier
    if ( !getNextToken( t, strScript, true ) || t.tokenType != asTC_IDENTIFIER )
    {
        // ToDo: 9999 - Expected constant buffer identifier
        return false;
    
    } // End if !identifier

    // Retrieve the token name / identifier
    Desc.name = stringConvertA2CT( strScript.substr( t.tokenStart, t.tokenLength ).c_str() );

    // Next token should either be a parameter list identifier (':') or
    // the start of the c-buffer interior
    if ( !getNextToken( t, strScript, true ) )
    {
        // ToDo: 9999 - Unexpected end of file
        return false;
    
    } // End if eof

    // Parameter list?
    if ( t.tokenLength == 1 && strScript.at(t.tokenStart) == ':' )
    {
        DefinitionMap parameters;
        if ( !parseShaderParameterList( t, strScript, parameters ) )
            return false;

        // Process supplied parameters
        DefinitionMap::const_iterator itParam;
        for ( itParam = parameters.begin(); itParam != parameters.end(); ++itParam )
        {
            Parser.clear();
            if ( itParam->first == "register" )
            {
                cgChar cRegType;
                Parser << itParam->second;
                Parser >> cRegType;
                Parser >> Desc.bufferRegister;
                
            } // End if register
            else if ( itParam->first == "globaloffset" )
            {
                cgChar cRegType;
                Parser << itParam->second;
                Parser >> cRegType;
                Parser >> Desc.globalOffset;

            } // End if globaloffset
        
        } // Next Parameter
    
    } // End if params

    // Tokens should come in the following order:
    // Type, identifier, optional array elements, optional parameter list, optional default value
    // i.e. float3 Identifier[4] : packoffset(c0.y) = { 1, 2, 3 };
    while ( t.position <= t.sectionEnd )
    {
        // Create a new constant declaration entry.
        Desc.constants.resize( Desc.constants.size() + 1 );
        cgConstantDesc & cDesc = Desc.constants.back();
        cDesc.isUDT             = false;
        cDesc.registerIndex          = -1;
        cDesc.registerComponent = 0;
        cDesc.rows              = 0;
        cDesc.columns           = 0;
        cDesc.elements          = 1;
        cDesc.elementLength     = -1;
        cDesc.ByteOffset        = 0;
        cDesc.ByteLength        = 0;

        // Constant type or end of buffer '}'?
        if ( t.tokenType == asTC_IDENTIFIER || t.tokenType == asTC_KEYWORD )
        {
            // Constant type name (asTC_KEYWORD required for 'float', 'int', etc.)
            // Parse type name in order to determine if this is an intrinsic
            // type (float, int, bool, etc.) or whether this is a user defined
            // type. This information will be used to compute the correct element 
            // length (in registers) as well as to automatically compute the
            // register bindings for each constant buffer member as necessary.
            if ( strScript.compare( t.tokenStart, t.tokenLength, "matrix" ) == 0 )
            {
                cDesc.rows          = 4;
                cDesc.columns       = 4;
                cDesc.elementLength = 4;
            
            } // End if matrix
            else
            {
                size_t nBaseTypeLength = 0;
                if ( t.tokenLength >= 5 && strScript.compare( t.tokenStart, 5, "float" ) == 0 )
                    nBaseTypeLength = 5;
                else if ( t.tokenLength >= 3 && strScript.compare( t.tokenStart, 3, "int" ) == 0 )
                    nBaseTypeLength = 3;
                else if ( t.tokenLength >= 4 && strScript.compare( t.tokenStart, 4, "uint" ) == 0 )
                    nBaseTypeLength = 4;
                else if ( t.tokenLength >= 4 && strScript.compare( t.tokenStart, 4, "bool" ) == 0 )
                    nBaseTypeLength = 4;
                else
                    cDesc.isUDT = true;

                // If there are 3 remaining characters and the second character is 'x', this could possibly be a 
                // multi-dimensional object (i.e. float4x4, int2x3, etc.). If there is only 1 character remaining
                // this could be a single dimensional, multi-element object (i.e. float2, int3, etc.)
                if ( !cDesc.isUDT )
                {
                    if ( (t.tokenLength - nBaseTypeLength) == 3 && strScript.at( t.tokenStart + (t.tokenLength - 2) ) == 'x' )
                    {
                        // {type}nxn
                        cgChar cSeparator;
                        cgUInt nRows, nColumns;
                        Parser.clear();
                        Parser << strScript.substr( t.tokenStart + (t.tokenLength - 3), 3 );
                        Parser >> nColumns >> cSeparator >> nRows;

                        // Rows and columns must be between 1 and 4.
                        if ( nRows >= 1 && nRows <= 4 && nColumns >= 1 && nColumns <= 4 )
                        {
                            // This is a built-in intrinsic type
                            cDesc.columns       = nColumns;
                            cDesc.rows          = nRows;
                            cDesc.elementLength = nRows;
                        
                        } // End if intrinsic
                        else
                            cDesc.isUDT = true;

                    } // End if 'nxn'
                    else if ( (t.tokenLength - nBaseTypeLength) == 1 )
                    {
                        // {type}n
                        cgUInt nElements;
                        Parser.clear();
                        Parser << strScript.substr( t.tokenStart + (t.tokenLength - 1), 1 );
                        Parser >> nElements;

                        // Size must be between 1 and 4.
                        if ( nElements >=1 && nElements <=4 )
                        {
                            // This is a built-in intrinsic type
                            cDesc.columns       = nElements;
                            cDesc.rows          = 1;
                            cDesc.elementLength = 1;
                        
                        } // End if intrinsic
                        else
                            cDesc.isUDT = true;

                    } // End if 'n'
                    else if ( (t.tokenLength - nBaseTypeLength) == 0 )
                    {
                        // Flat, single element type
                        cDesc.columns       = 1;
                        cDesc.rows          = 1;
                        cDesc.elementLength = 1;

                    } // End if flat type
                    else
                        cDesc.isUDT = true;

                } // End if !UDT

            } // End if !Matrix

            // Store type name
            cDesc.typeName = stringConvertA2CT(strScript.substr( t.tokenStart, t.tokenLength ).c_str());
        
        } // End if type
        else
        {
            // ToDo: 9999 - Expected type identifier
            return false;

        } // End if invalid

        // Constant identifier?
        if ( !getNextToken( t, strScript, true ) || t.tokenType != asTC_IDENTIFIER )
        {
            // ToDo: 9999 - Expected variable identifier
            return false;

        } // End if eof
        cDesc.name = stringConvertA2CT( strScript.substr( t.tokenStart, t.tokenLength ).c_str() );

        // Array size '[x]', parameter list ':' or default value follows.
        if ( !getNextToken( t, strScript, true ) )
        {
            // ToDo: 9999 - Unexpected end of file
            return false;

        } // End if eof

        // Array size(s)? Remember, this may be multi-dimensional (array[1][2][3])
        cDesc.elements = 1;
        while ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == '[' )
        {
            // Retrieve size value.
            if ( !getNextToken( t, strScript, true ) || t.tokenType != asTC_VALUE )
            {
                // ToDo: 9999 - Expected literal array size value.
                return false;

            } // End if eof
            cgInt nSize;
            Parser.clear();
            Parser << strScript.substr( t.tokenStart, t.tokenLength );
            Parser >> nSize;
            cDesc.elements *= nSize;

            // Matching close?
            if ( !getNextToken( t, strScript, true ) || 
                 !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ']') )
            {
                // ToDo: 9999 - Mismatched '['
                return false;

            } // End if eof
            
            // Retrieve next token ready for next part of the constant declaration
            if ( !getNextToken( t, strScript, true ) )
            {
                // ToDo: 9999 - Unexpected end of file
                return false;

            } // End if eof
            
        } // End if array size
        
        // Parameter list?
        bool bHasPackOffset = false;
        if ( t.tokenLength == 1 && strScript.at(t.tokenStart) == ':' )
        {
            DefinitionMap parameters;
            if ( !parseShaderParameterList( t, strScript, parameters ) )
                return false;

            // Process supplied parameters
            DefinitionMap::const_iterator itParam;
            for ( itParam = parameters.begin(); itParam != parameters.end(); ++itParam )
            {
                Parser.clear();
                if ( itParam->first == "packoffset" )
                {
                    const std::string & strValue = itParam->second;
                    size_t nSeparator = strValue.find_first_of( "." );
                    if ( nSeparator != std::string::npos )
                    {
                        // Extract the single following register component identifier (x, y, z, w)
                        cgChar cComponent = 0;
                        if ( strValue.size() >= (nSeparator + 2) )
                            cComponent = strValue.at( nSeparator + 1 );
                        if ( cComponent == 'x' )
                            cDesc.registerComponent = 0;
                        else if ( cComponent == 'y' )
                            cDesc.registerComponent = 1;
                        else if ( cComponent == 'z' )
                            cDesc.registerComponent = 2;
                        else if ( cComponent == 'w' )
                            cDesc.registerComponent = 3;
                        else
                        {
                            // ToDo: 9999 - Expected following register component identifier (.xyzw)
                            return false;

                        } // End if no component
                        
                    } // End if has register component part

                    // Find the first numeric character (we'll ignore any starting characters like 'c150').
                    size_t nNumberStart = strValue.find_first_of( NumericChars );
                    if ( nNumberStart == std::string::npos )
                    {
                        // ToDo: 9999 - Expected numeric register index
                        return false;
                    
                    } // End if no number
                    size_t nNumberEnd = strValue.find_first_not_of( NumericChars, nNumberStart );
                    if ( nNumberEnd == std::string::npos )
                        nNumberEnd = strValue.size();

                    // Extract the register index
                    Parser << strValue.substr( nNumberStart , (nNumberEnd - nNumberStart) );
                    Parser >> cDesc.registerIndex;

                    // Constant specifies pack offset.
                    bHasPackOffset = true;
                    
                } // End if register
                else if ( itParam->first == "elementlength" )
                {
                    Parser << itParam->second;
                    Parser >> cDesc.elementLength;

                } // End if globaloffset
            
            } // Next Parameter
            
        } // End if parameter list

        // If a constant had previously specified a pack offset and this one did not,
        // or vice-versa, then we must fail.
        if ( Desc.constants.size() == 1 )
        {
            Desc.IsPacked = bHasPackOffset;
        
        } // End if first constant
        else if ( Desc.IsPacked != bHasPackOffset )
        {
            // ToDo: 9999 - Cannot mix constants that define a 'packoffset' parameter with those that do not.
            return false;

        } // End if other constants

        // Default value?
        if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == '=' )
        {
            // ToDo: 99999
            while ( !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ';') )
                getNextToken( t, strScript, true );

        } // End if default value
        
        // End of constant?
        if ( !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ';') )
        {
            // ToDo: 9999 - Expected ';'
            return false;
        
        } // End if invalid constant

        // Get next token ready for next iteration (should be constant type).
        // If there is no more data, we simply reached the end of the section.
        if ( !getNextToken( t, strScript, true ) )
            break;

    } // Next Token

    // Now that details have been recorded for the constant buffer, we must
    // process the constants and assign their final registers (if they haven't
    // already been assigned) and to compute final byte offsets and lengths.
    if ( Desc.IsPacked == true )
    {
        cgInt nMaxRegister = 0;
        for ( size_t i = 0; i < Desc.constants.size(); ++i )
        {
            cgConstantDesc & cDesc = Desc.constants[i];
            
            // If this is a UDT and no element length has been supplied
            // then we must fail.
            if ( cDesc.isUDT && cDesc.elementLength < 0 )
            {
                // ToDo: 9999 - UDT must supply element length parameter
                return false;
            
            } // End if invalid UDT

            // All UDTs must be aligned to the first register component
            if ( cDesc.isUDT && cDesc.registerComponent != 0 )
            {
                // ToDo: 9999 - UDT must be aligned to the start of its assigned register.
                return false;
            
            } // End if unaligned UDT

            // All arrays must be aligned to the first register component
            if ( cDesc.elements > 0 && cDesc.registerComponent != 0 )
            {
                // ToDo: 9999 - Arrays must be aligned to the start of its assigned register.
                return false;
            
            } // End if unaligned array

            // Packed or unpacked behavior?
            if ( cDesc.registerComponent == 0 )
            {
                // Compute buffer offset and length.
                cDesc.ByteLength    = (cDesc.elementLength * 64) * cDesc.elements;
                cDesc.ByteOffset    = cDesc.registerIndex * 64;
            
            } // End if unpacked

        } // Next Constant

    } // End if packed
    else
    {
        cgInt nCurrentRegister = 0;
        for ( size_t i = 0; i < Desc.constants.size(); ++i )
        {
            cgConstantDesc & cDesc = Desc.constants[i];
            
            // If this is a UDT and no element length has been supplied
            // then we must fail.
            if ( cDesc.isUDT && cDesc.elementLength < 0 )
            {
                // ToDo: 9999 - UDT must supply element length parameter
                return false;
            
            } // End if invalid UDT

            // Assign register and compute buffer offset and length.
            cDesc.registerIndex      = nCurrentRegister;
            cDesc.ByteLength    = (cDesc.elementLength * 64) * cDesc.elements;
            cDesc.ByteOffset    = cDesc.registerIndex * 64;

            // Increment current register based on the number
            // of registers consumed by this constant.
            nCurrentRegister += cDesc.elementLength * cDesc.elements;

            // Increment total cbuffer length by number of bytes consumed.
            Desc.length += cDesc.ByteLength;

        } // Next Constant

    } // End if !packed

    // Success!
    return true;
}*/

//-----------------------------------------------------------------------------
//  Name : parseShaderCallDeclaration () (Protected)
/// <summary>
/// Parse a __shadercall function declaration. These are HLSL-like functions
/// with full HLSL parameter and return type support. The function declaration
/// will be converted into angel-script version that can be called by a shader
/// (or other function) but will ultimately generate HLSL functions internally.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::parseShaderCallDeclaration( TokenData & t, std::string & strScript, cgShaderCallFunctionDesc & Desc, const std::string & strNamespace )
{
    STRING_CONVERT;
    bool bFirstParam = true;
    std::string strNewDeclaration = "String";
    std::string strFunctionName, strShaderDeclaration, strShaderParameters;

    // Reset necessary variables.
    Desc.requiresReturn = false;
    Desc.parameters.clear();
    
    // Record start of declaration (at start of __shadercall token)
    size_t nDeclarationBegin = t.tokenStart;
    size_t nDeclarationEnd   = 0;

    // First token should be the return type. Note: throughout this method, special handling
    // is required in order to ensure that we maintain an accurate representation of all of
    // the white-space used in the original declarations (including newlines). This ensures
    // that the line-numbering in the output script remains unaltered.
    bool bResult;
    while ( (bResult = getNextToken(t, strScript, false)) && t.tokenType == asTC_WHITESPACE )
    {
        // Duplicate whitespace to ensure we maintain accurate line-numbering
        strNewDeclaration.append( strScript.substr( t.tokenStart, t.tokenLength ) );
    
    } // Next whitespace token
    if ( !bResult )
    {
        cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
        return false;
    
    } // End if eof
    std::string strReturnType = strScript.substr(t.tokenStart, t.tokenLength);
    strShaderDeclaration.append( strReturnType );
    strShaderDeclaration.append( " " );

    // Next token should be function name
    while ( (bResult = getNextToken(t, strScript, false)) && t.tokenType == asTC_WHITESPACE )
    {
        // Duplicate whitespace to ensure we maintain accurate line-numbering
        strNewDeclaration.append( strScript.substr( t.tokenStart, t.tokenLength ) );
    
    } // Next whitespace token
    if ( !bResult || t.tokenType != asTC_IDENTIFIER )
    {
        cgToDoAssert( "Script Preprocessor", "Expected function name." );
        return false;
    
    } // End if !identifier
    strFunctionName = strScript.substr(t.tokenStart, t.tokenLength);
    strNewDeclaration.append( strFunctionName );
    strShaderDeclaration.append( strFunctionName );

    // Next token should be opening parenthesis
    while ( (bResult = getNextToken(t, strScript, false)) && t.tokenType == asTC_WHITESPACE )
    {
        // Duplicate whitespace to ensure we maintain accurate line-numbering
        strNewDeclaration.append( strScript.substr( t.tokenStart, t.tokenLength ) );
    
    } // Next whitespace token
    if ( !bResult )
    {
        cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
        return false;
    
    } // End if eof
    if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == '(' )
    {
        // Output opening parenthesis
        strNewDeclaration.append( "(" );
        strShaderDeclaration.append( "(" );

        // Angelscript function requires implicit return value parameter?
        Desc.requiresReturn = (strReturnType != "void" );
        /*if ( Desc.requiresReturn )
            strNewDeclaration.append( "const String& __functionResult," );*/

        // Keep parsing tokens until we find the closing paren.
        while ( nDeclarationEnd == 0 )
        {
            // Find next token
            while ( (bResult = getNextToken(t, strScript, false)) && t.tokenType == asTC_WHITESPACE )
            {
                // Duplicate whitespace to ensure we maintain accurate line-numbering
                strNewDeclaration.append( strScript.substr( t.tokenStart, t.tokenLength ) );
            
            } // Next whitespace token
            if ( !bResult )
            {
                cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
                return false;
            
            } // End if eof

            // Closing paren or (invalid) early parameter separator?
            if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD )
            {
                if ( strScript.at(t.tokenStart) == ')' )
                {
                    nDeclarationEnd = t.tokenStart;
                    strNewDeclaration.append( ")" );
                    break;
                
                } // End if ')'
                else if ( strScript.at(t.tokenStart) == ',' )
                {
                    cgToDoAssert( "Script Preprocessor", "Expected parameter declaration." );
                    return false;

                } // End if ','
            
            } // End if keyword

            // Add a parameter to the list.
            Desc.parameters.push_back( cgShaderCallParameterDesc() );
            cgShaderCallParameterDesc & ParamDesc = Desc.parameters.back();
            ParamDesc.scriptParameter = false;
            
            // Next token should be optional parameter modifier, or
            // parameter type.
            cgInt nModifier = 0; // 0 = none, 1 = in, 2 = out, 3 = inout, 4 = uniform, 5 = script
            size_t nModifierBegin = t.tokenStart;
            if ( t.tokenLength == 2 && strScript.substr(t.tokenStart, t.tokenLength) == "in" )
            {
                ParamDesc.modifier = _T("in");
                nModifier = 1;
            
            } // End if "in"
            else if ( t.tokenLength == 3 && strScript.substr(t.tokenStart, t.tokenLength) == "out" )
            {
                ParamDesc.modifier = _T("out");
                nModifier = 2;
            
            } // End if "out"
            else if ( t.tokenLength == 5 && strScript.substr(t.tokenStart, t.tokenLength) == "inout" )
            {
                ParamDesc.modifier = _T("inout");
                nModifier = 3;
            
            } // End if "inout"
            else if ( t.tokenLength == 7 && strScript.substr(t.tokenStart, t.tokenLength) == "uniform" )
            {
                ParamDesc.modifier = _T("uniform");
                nModifier = 4;
            
            } // End if "uniform"
            else if ( t.tokenLength == 6 && strScript.substr(t.tokenStart, t.tokenLength) == "script" )
            {
                ParamDesc.modifier = _T("script");
                ParamDesc.scriptParameter = true;
                nModifier = 5;

            } // End if "script"

            // If we found a modifier, we need another token which /should/ be the parameter type
            if ( nModifier )
            {
                while ( (bResult = getNextToken(t, strScript, false)) && t.tokenType == asTC_WHITESPACE )
                {
                    // Duplicate whitespace to ensure we maintain accurate line-numbering
                    strNewDeclaration.append( strScript.substr( t.tokenStart, t.tokenLength ) );
                
                } // Next whitespace token
                if ( !bResult )
                {
                    cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
                    return false;
                
                } // End if eof

                // Closing paren?
                if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && 
                     (strScript.at(t.tokenStart) == ')' || strScript.at(t.tokenStart) == ',') )
                {
                    cgToDoAssert( "Script Preprocessor", "Expected parameter type." );
                    return false;
                
                } // End if ')'

            } // End if modified parameter

            // Extract the parameter type.
            size_t nParamStart = t.tokenStart, nParamEnd = 0;
            std::string strParamType = strScript.substr(t.tokenStart, t.tokenLength);
            ParamDesc.typeName = stringConvertA2CT(strParamType.c_str());

            // Output parameter type to new declaration. Script parameters are output as-is, but
            // shader parameters are housed, passed and referenced via string variables.
            if ( nModifier == 5 )
                strNewDeclaration.append( strParamType );
            else
                strNewDeclaration.append( "const String&" );

            // Next token should be the (optional) parameter name, the end of the declaration
            // or a comma separator.
            while ( (bResult = getNextToken(t, strScript, false)) && t.tokenType == asTC_WHITESPACE )
            {
                // Duplicate whitespace to ensure we maintain accurate line-numbering
                strNewDeclaration.append( strScript.substr( t.tokenStart, t.tokenLength ) );
            
            } // Next whitespace token
            if ( !bResult )
            {
                cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
                return false;
            
            } // End if eof
            
            // Closing paren or parameter separator?
            std::string strParamName = "";
            if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ')' )
            {
                // Name-less parameter and end of declaration.
                nDeclarationEnd = t.tokenStart;
                nParamEnd = t.tokenStart - 1;
                strNewDeclaration.append( ")" );
            
            } // End if ')'
            else if ( !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ',') )
            {
                // Not a separator, so this must be the parameter name.
                strParamName = strScript.substr(t.tokenStart, t.tokenLength);

                // Output parameter name to new declaration (angelscript name matches in either script or shader case)
                strNewDeclaration.append( strParamName );

            } // End if !','
            else
            {
                nParamEnd = t.tokenStart - 1;
                strNewDeclaration.append( "," );
            
            } // End if ','
            ParamDesc.name = stringConvertA2CT( strParamName.c_str() );
            
            // Final token should be closing paren or parameter separator (if not already processed)
            if ( nDeclarationEnd == 0 && !strParamName.empty() )
            {
                while ( (bResult = getNextToken(t, strScript, false)) && t.tokenType == asTC_WHITESPACE )
                {
                    // Duplicate whitespace to ensure we maintain accurate line-numbering
                    strNewDeclaration.append( strScript.substr( t.tokenStart, t.tokenLength ) );
                
                } // Next whitespace token
                if ( !bResult )
                {
                    cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
                    return false;
                
                } // End if eof

                if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ')' )
                {
                    // Name-less parameter and end of declaration.
                    nDeclarationEnd = t.tokenStart;
                    nParamEnd = t.tokenStart - 1;
                    strNewDeclaration.append( ")" );
                
                } // End if ')'
                else if ( !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ',') )
                {
                    // Not a separator as expected!
                    cgToDoAssert( "Script Preprocessor", "Expected ','." );
                    return false;

                } // End if !','
                else
                {
                    nParamEnd = t.tokenStart - 1;
                    strNewDeclaration.append( "," );
                
                } // End if ','

            } // End if not processed

            // Output parameter to new shader declaration if this is not a script parameter.
            if ( nModifier != 5 )
            {
                // Build final declaration
                if ( !bFirstParam )
                    strShaderDeclaration.append(",");
                strShaderDeclaration.append( strScript.substr( nModifierBegin, (nParamEnd - nModifierBegin) + 1) );

                // Output parameter list (for *calling*)
                if ( bFirstParam )
                    strShaderParameters.append( "\"(\"+" );
                else
                    strShaderParameters.append( "\",(\"+" );
                strShaderParameters.append( strParamName );
                strShaderParameters.append( "+\")\"" );
                bFirstParam = false;
            
            } // End if !script param

        } // Next parameter

        // Finish up shader declaration
        strShaderDeclaration.append( ")" );
        
        // Replace '__shadercall' declaration with the newly generated angel script compatible section.
        // Is there enough space in the script string to just replace the existing block?
        size_t nNewSize = strNewDeclaration.length();
        if ( nNewSize == (t.position - nDeclarationBegin) )
        {
            // Exact match in size. We can just replace.
            strScript.replace( nDeclarationBegin, nNewSize, strNewDeclaration );
        
        } // End if exact size
        else if ( nNewSize < (t.position - nDeclarationBegin) )
        {
            // Smaller. We can replace the first part, and clear the rest.
            strScript.replace( nDeclarationBegin, nNewSize, strNewDeclaration );
            overwriteCode( strScript, nDeclarationBegin + nNewSize, t.position - (nDeclarationBegin + nNewSize) );

        } // End if smaller
        else
        {
            // Code is larger, we can replace the contents entirely but we must 
            // also adjust the current read position to take this expanded string 
            // size into account.
            strScript.replace( nDeclarationBegin, (t.position - nDeclarationBegin), strNewDeclaration );
            t.sectionEnd += (cgInt)nNewSize - (cgInt)(t.position - nDeclarationBegin);
            t.position    = nDeclarationBegin + nNewSize;

        } // End if larger
        
    } // End if '('
    else
    {
        cgToDoAssert( "Script Preprocessor", "Expected opening '('." );
        return false;

    } // End if !'('

    // Populate output descriptor.
    Desc.parentNamespace = stringConvertA2CT(strNamespace.c_str());
    Desc.name            = stringConvertA2CT(strFunctionName.c_str());
    Desc.declaration     = stringConvertA2CT(strShaderDeclaration.c_str());
    Desc.callingParams   = stringConvertA2CT(strShaderParameters.c_str());
    Desc.returnType      = stringConvertA2CT(strReturnType.c_str());

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseTypeDescriptor () (Protected)
/// <summary>
/// Parse the specified script code to determine the layout and description for
/// any following type / structure. Also moves the specified TokenData::position 
/// variable past the structure content.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::parseTypeDescriptor( TokenData & t, const std::string & strScript, cgConstantTypeDesc & tDesc, const std::string & strNamespace )
{
    STRING_CONVERT;
    std::stringstream Parser;

    // Initialize type description details.
    tDesc.length            = 0;
    tDesc.alignment         = 0;
    tDesc.registerCount     = 0;
    tDesc.constants.clear();

    // Retrieve the structure identifier
    if ( !getNextToken( t, strScript, true ) || t.tokenType != asTC_IDENTIFIER )
    {
        cgToDoAssert( "Script Preprocessor", "Expected type identifier." );
        return false;
    
    } // End if !identifier

    // Retrieve the token name / identifier
    tDesc.name = stringConvertA2CT( strScript.substr( t.tokenStart, t.tokenLength ).c_str() );

    // Next token should either be a parameter list identifier (':') or
    // the start of the c-buffer interior
    if ( !getNextToken( t, strScript, true ) )
    {
        cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
        return false;
    
    } // End if eof

    // Parameter list?
    if ( t.tokenLength == 1 && strScript.at(t.tokenStart) == ':' )
    {
        if ( !parseShaderParameterList( t, strScript, tDesc.parameters ) )
            return false;

    } // End if params

    // Pre-parse certain common type parameters.
    if ( tDesc.parameters.isPropertyDefined( _T("namespace") ) )
        tDesc.parentNamespace = tDesc.parameters.getProperty( _T("namespace") );
    else
        tDesc.parentNamespace = stringConvertA2CT(strNamespace.c_str());

    // Tokens should come in the following order:
    // Type, identifier, optional array elements, optional parameter list, optional default value
    // i.e. float3 Identifier[4] : parameter(xyz) = { 1, 2, 3 };
    while ( t.position <= t.sectionEnd )
    {
        // Create a new constant declaration entry.
        tDesc.constants.resize( tDesc.constants.size() + 1 );
        cgConstantDesc & cDesc = tDesc.constants.back();
        cDesc.isUDT             = false;
        cDesc.isArray           = false;
        cDesc.typeId            = cgConstantType::Unresolved;
        cDesc.rows              = 0;
        cDesc.columns           = 0;
        cDesc.elements          = 1;
        cDesc.elementLength     = 0;
        cDesc.offset            = 0;
        cDesc.totalLength       = 0;
        cDesc.alignment         = 0;
        cDesc.registerOffset    = 0xFFFFFFFF;
        cDesc.registerComponent = 0;
        cDesc.registerCount     = 0;
        cDesc.registerPacking   = false;

        // Constant type or end of buffer '}'?
        if ( t.tokenType == asTC_IDENTIFIER || t.tokenType == asTC_KEYWORD )
        {
            // Store type name.
            cDesc.fullTypeName = stringConvertA2CT(strScript.substr( t.tokenStart, t.tokenLength ).c_str());

            // Constant type name (asTC_KEYWORD required for 'float', 'int', etc.)
            // Parse type name in order to determine if this is an intrinsic
            // type (float, int, bool, etc.) or whether this is a user defined
            // type. This information will be used to compute the correct element 
            // length (in registers) as well as to automatically compute the
            // register bindings for each constant buffer member as necessary.
            if ( strScript.compare( t.tokenStart, t.tokenLength, "matrix" ) == 0 )
            {
                cDesc.rows              = 4;
                cDesc.columns           = 4;
                cDesc.elementLength     = 64;
                cDesc.alignment         = 4;
                cDesc.typeId            = cgConstantType::Float;
                cDesc.primitiveTypeName = _T("float");
            
            } // End if matrix
            else
            {
                size_t nBaseTypeLength = 0, nBaseTypeBytes = 0;
                if ( t.tokenLength >= 5 && strScript.compare( t.tokenStart, 5, "float" ) == 0 )
                {
                    nBaseTypeBytes          = 4;
                    nBaseTypeLength         = 5;
                    cDesc.alignment         = 4;
                    cDesc.typeId            = cgConstantType::Float;
                    cDesc.primitiveTypeName = _T("float");
                
                } // End if float
                else if ( t.tokenLength >= 3 && strScript.compare( t.tokenStart, 3, "int" ) == 0 )
                {
                    nBaseTypeBytes          = 4;
                    nBaseTypeLength         = 3;
                    cDesc.alignment         = 4;
                    cDesc.typeId            = cgConstantType::Int;
                    cDesc.primitiveTypeName = _T("int");
                
                } // End if int
                else if ( t.tokenLength >= 4 && strScript.compare( t.tokenStart, 4, "uint" ) == 0 )
                {
                    nBaseTypeBytes           = 4;
                    nBaseTypeLength          = 4;
                    cDesc.alignment          = 4;
                    cDesc.typeId             = cgConstantType::UInt;
                    cDesc.primitiveTypeName  = _T("uint");

                } // End if uint
                else if ( t.tokenLength >= 4 && strScript.compare( t.tokenStart, 4, "bool" ) == 0 )
                {
                    nBaseTypeBytes          = 1;
                    nBaseTypeLength         = 4;
                    cDesc.alignment         = 1;
                    cDesc.typeId            = cgConstantType::Bool;
                    cDesc.primitiveTypeName = _T("bool");
                
                } // End if bool
                else
                    cDesc.isUDT     = true;

                // If there are 3 remaining characters and the second character is 'x', this could possibly be a 
                // multi-dimensional object (i.e. float4x4, int2x3, etc.). If there is only 1 character remaining
                // this could be a single dimensional, multi-element object (i.e. float2, int3, etc.)
                if ( !cDesc.isUDT )
                {
                    if ( (t.tokenLength - nBaseTypeLength) == 3 && strScript.at( t.tokenStart + (t.tokenLength - 2) ) == 'x' )
                    {
                        // {type}nxn (Row major -- like C arrays! so rows come first followed by columns)
                        cgChar cSeparator;
                        cgUInt nRows, nColumns;
                        Parser.clear();
                        Parser << strScript.substr( t.tokenStart + (t.tokenLength - 3), 3 );
                        Parser >> nRows >> cSeparator >> nColumns;

                        // Rows and columns must be between 1 and 4.
                        if ( nRows >= 1 && nRows <= 4 && nColumns >= 1 && nColumns <= 4 )
                        {
                            // This is a built-in intrinsic type
                            cDesc.columns       = nColumns;
                            cDesc.rows          = nRows;
                            cDesc.elementLength = nRows * nColumns * nBaseTypeBytes;
                        
                        } // End if intrinsic
                        else
                            cDesc.isUDT = true;

                    } // End if 'nxn'
                    else if ( (t.tokenLength - nBaseTypeLength) == 1 )
                    {
                        // {type}n
                        cgUInt nElements;
                        Parser.clear();
                        Parser << strScript.substr( t.tokenStart + (t.tokenLength - 1), 1 );
                        Parser >> nElements;

                        // Size must be between 1 and 4.
                        if ( nElements >=1 && nElements <=4 )
                        {
                            // This is a built-in intrinsic type
                            cDesc.columns       = nElements;
                            cDesc.rows          = 1;
                            cDesc.elementLength = nElements * nBaseTypeBytes;
                        
                        } // End if intrinsic
                        else
                            cDesc.isUDT = true;

                    } // End if 'n'
                    else if ( (t.tokenLength - nBaseTypeLength) == 0 )
                    {
                        // Flat, single element type
                        cDesc.columns       = 1;
                        cDesc.rows          = 1;
                        cDesc.elementLength = nBaseTypeBytes;

                    } // End if flat type
                    else
                        cDesc.isUDT = true;

                } // End if !UDT

                // If this /is/ a UDT, retrieve the identifier of the underlying type
                // and also determine the alignment for the structure (cascades
                // into this constant). NB: do not use 'else' clause because the prior
                // conditional may have opted to convert the constant to a UDT after the fact)
                if ( cDesc.isUDT )
                {
                    // Assign type properties
                    cDesc.primitiveTypeName = cDesc.fullTypeName;

                    // Retrieve fixed typeId
                    cDesc.typeId = ((cgSurfaceShaderScript*)mScript)->findConstantType( tDesc.parentNamespace, cDesc.fullTypeName );
                    if ( cDesc.typeId < 0 )
                    {
                        cgToDoAssert( "Script Preprocessor", "Unrecognized type identifier." );
                        return false;
                    
                    } // End if invalid
                    
                    // Inherit alignment from type / struct.
                    cgConstantTypeDesc UDTDesc;
                    ((cgSurfaceShaderScript*)mScript)->getConstantTypeDesc( cDesc.typeId, UDTDesc );
                    cDesc.alignment = UDTDesc.alignment;
                    cDesc.elementLength = UDTDesc.length;

                } // End if UDT

            } // End if !Matrix
        
        } // End if type
        else
        {
            cgToDoAssert( "Script Preprocessor", "Expected type identifier." );
            return false;

        } // End if invalid

        // Constant identifier?
        if ( !getNextToken( t, strScript, true ) || t.tokenType != asTC_IDENTIFIER )
        {
            cgToDoAssert( "Script Preprocessor", "Expected variable identifier." );
            return false;

        } // End if eof
        cDesc.name = stringConvertA2CT( strScript.substr( t.tokenStart, t.tokenLength ).c_str() );

        // Array size '[x]', parameter list ':' or default value follows.
        if ( !getNextToken( t, strScript, true ) )
        {
            cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
            return false;

        } // End if eof

        // Array size(s)? Remember, this may be multi-dimensional (array[1][2][3])
        cDesc.elements = 1;
        while ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == '[' )
        {
            // Array tokens included.
            cDesc.isArray = true;

            // Retrieve size value.
            if ( !getNextToken( t, strScript, true ) || t.tokenType != asTC_VALUE )
            {
                cgToDoAssert( "Script Preprocessor", "Expected literal array size value." );
                return false;

            } // End if eof
            cgInt nSize;
            Parser.clear();
            Parser << strScript.substr( t.tokenStart, t.tokenLength );
            Parser >> nSize;
            cDesc.elements *= nSize;

            // Record supplied array dimensions
            cDesc.arrayDimensions.push_back( nSize );

            // Matching close?
            if ( !getNextToken( t, strScript, true ) || 
                 !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ']') )
            {
                cgToDoAssert( "Script Preprocessor", "Mismatched '[' token." );
                return false;

            } // End if eof
            
            // Retrieve next token ready for next part of the constant declaration
            if ( !getNextToken( t, strScript, true ) )
            {
                cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
                return false;

            } // End if eof
            
        } // End if array size
        
        // Parameter list?
        if ( t.tokenLength == 1 && strScript.at(t.tokenStart) == ':' )
        {
            if ( !parseShaderParameterList( t, strScript, cDesc.parameters ) )
                return false;
            
        } // End if parameter list

        // Default value?
        if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == '=' )
        {
            cgToDo( "Script Preprocessor", "Re-add support for default values when we figure out a fix." );
            while ( !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ';') )
                getNextToken( t, strScript, true );

        } // End if default value
        
        // End of constant?
        if ( !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ';') )
        {
            cgToDoAssert( "Script Preprocessor", "Expected ';'." );
            return false;
        
        } // End if invalid constant

        // Get next token ready for next iteration (should be constant type).
        // If there is no more data, we simply reached the end of the section.
        if ( !getNextToken( t, strScript, true ) )
            break;

    } // Next Token

    // Now we need to correctly compute packing and offset for the structure
    // to match that of an appropriate C++ side structure (4 byte aligned).
    for ( size_t i = 0; i < tDesc.constants.size(); ++i )
    {
        cgConstantDesc & cDesc = tDesc.constants[i];

        // Add any necessary padding to the buffer layout in order to 
        // correctly align this constant to the appropriate word boundary.
        tDesc.length  = (tDesc.length + (cDesc.alignment - 1)) & ~(cDesc.alignment-1);

        // Compute final offset and size details for this constant
        cDesc.offset = tDesc.length;
        cDesc.totalLength = cDesc.elementLength * cDesc.elements;

        // Buffer now includes this constant
        tDesc.length += cDesc.totalLength;

        // The buffer itself needs to be padded out (at the end)
        // based on the required alignment of the /largest/ member.
        // Record this here.
        if ( cDesc.alignment > tDesc.alignment )
            tDesc.alignment = cDesc.alignment;

    } // Next Constant

    // Pad the buffer as necessary based on the alignment of the largest member.
    tDesc.length = (tDesc.length + (tDesc.alignment - 1)) & ~(tDesc.alignment-1);

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseShaderParameterList () (Protected)
/// <summary>
/// Parse a shader element parameter list usually following the ':' separating
/// character. Users can supply additional control parameters for elements
/// such as constant buffers, constant variables, samplers, etc. Such 
/// parameters include register assignment, packing data and so on.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::parseShaderParameterList( TokenData & t, const std::string & strScript, cgPropertyContainer & Parameters )
{
    STRING_CONVERT;

    // Be polite and clear output map.
    Parameters.clear();

    // Keep searching until we run out of data.
    while ( t.position <= t.sectionEnd )
    {
        // First token should be the parameter name / identifier
        if ( !getNextToken( t, strScript, true ) || t.tokenType != asTC_IDENTIFIER )
        {
            cgToDoAssert( "Script Preprocessor", "Expected parameter name." );
            return false;
        
        } // End if !identifier
        std::string strParamName = strScript.substr(t.tokenStart, t.tokenLength);

        // Next token should be parenthesis
        if ( !getNextToken( t, strScript, true ) )
        {
            cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
            return false;
        
        } // End if eof
        if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == '(' )
        {
            size_t nValueStart = t.position, nValueLength = 0;
            cgInt nNested = 1;
            
            // Keep parsing tokens until we find the closing paren.
            while ( 1 )
            {
                if ( !getNextToken( t, strScript, true ) )
                {
                    cgToDoAssert( "Script Preprocessor", "Mismatched '('." );
                    return false;
                
                } // End if eof

                // Another opening paren, or closing?
                if ( t.tokenLength == 1 && t.tokenType == asTC_KEYWORD )
                {
                    if ( strScript.at(t.tokenStart) == '(' )
                    {
                        nNested++;
                    
                    } // End if increased nesting
                    else if ( strScript.at(t.tokenStart) == ')' )
                    {
                        if ( --nNested == 0 )
                        {
                            nValueLength = t.tokenStart - nValueStart;
                            break;
                        
                        } // End if complete

                    } // End if decreased nesting
                    
                } // End if keyword

            } // Next Token

            // Add parameter
            Parameters.setProperty( stringConvertA2CT(strParamName.c_str()),
                                    stringConvertA2CT(strScript.substr(nValueStart, nValueLength).c_str()) );
            
        } // End if '('
        else
        {
            cgToDoAssert( "Script Preprocessor", "Expected opening '('." );
            return false;

        } // End if !'('

        // If the next token is not a comma (next parameter) then we're done.
        if ( !getNextToken( t, strScript, true ) ||
             !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ',') )
            break;
        
    } // Next Token

    // Success!
    return true;
}

/*//-----------------------------------------------------------------------------
//  Name : ParseSamplerBlockDetails () (Protected)
/// <summary>
/// Parse the specified script code to determine the supplied sampler block 
/// name. Also moves the specified TokenData::position variable to the start of
/// the sampler block content.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::ParseSamplerBlockDetails( TokenData & t, const std::string & strScript, std::string & strSamplerBlockName )
{
    size_t nCurrentPos = t.position;

    // Find the next newline
    size_t nNewlinePos = strScript.find_first_of( "\n", nCurrentPos );
    if ( nNewlinePos != std::string::npos )
    {
        strSamplerBlockName = strScript.substr( nCurrentPos, nNewlinePos - nCurrentPos );

        // Move caller on to start of content
        t.position = nNewlinePos + 1;

        // Trim whitespace from name
        size_t nTrimPos = strSamplerBlockName.find_first_not_of(" \t\r");
        if ( nTrimPos != std::string::npos )
        {
            strSamplerBlockName.erase(0,nTrimPos);
            nTrimPos = strSamplerBlockName.find_last_not_of(" \t\r");
            if ( nTrimPos != std::string::npos )
                strSamplerBlockName.erase(nTrimPos+1);
            else
                strSamplerBlockName.clear();
        
        } // End if found
        else
            strSamplerBlockName.clear();

        // Was this an unnamed sampler block?
        if ( strSamplerBlockName.empty() )
            strSamplerBlockName = "_unnamed_";

    } // End if found new-line
    else
    {
        cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
        return false;

    } // End if no new-line

    // Success!
    return true;
}*/

//-----------------------------------------------------------------------------
//  Name : parseSamplerBlock () (Protected)
/// <summary>
/// Parse the specified script code to determine the details for the upcoming
/// sampler block. Also moves the specified TokenData::position variable past 
/// the block content.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptPreprocessor::parseSamplerBlock( TokenData & t, const std::string & strScript, cgSamplerBlockDesc & tDesc, const std::string & strNamespace )
{
    STRING_CONVERT;
    std::stringstream Parser;

    // Initialize type description details.
    tDesc.samplers.clear();

    // If the next token comes *after* a following newline, or defines
    // a parameter list (:) then it is an unnamed sampler block. Otherwise 
    // the next token is the name. First retrieve the position of the newline
    // or parameter list separator.
    size_t nEndOfName = strScript.find_first_of( "\n:", t.position );
    if ( nEndOfName == std::string::npos )
        nEndOfName = strScript.size();

    // Retrieve the next token.
    if ( !getNextToken( t, strScript, true ) )
    {
        cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
        return false;
    
    } // End if !identifier

    // Before or after end of name token?
    if ( t.tokenStart < nEndOfName )
    {
        // Before the end of name token (newline or parameter separator)
        // so this is the sampler block name / identifier.
        if ( t.tokenType != asTC_IDENTIFIER )
        {
            cgToDoAssert( "Script Preprocessor", "Expected sampler block identifier." );
            return false;
        
        } // End if !identifier

        // Retrieve the token name / identifier
        tDesc.name = stringConvertA2CT( strScript.substr( t.tokenStart, t.tokenLength ).c_str() );

        // Next token should either be a parameter list identifier (':') or
        // the start of the block interior
        if ( !getNextToken( t, strScript, true ) )
        {
            cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
            return false;
        
        } // End if eof

    } // End if named
    else
    {
        // Use the default name. Default sampler blocks are always included
        // in every pixel shader.
        tDesc.name = _T("_default_");

    } // End if unnamed

    // Parameter list up next?
    if ( t.tokenLength == 1 && strScript.at(t.tokenStart) == ':' )
    {
        if ( !parseShaderParameterList( t, strScript, tDesc.parameters ) )
            return false;

    } // End if params

    // Pre-parse certain common type parameters.
    if ( tDesc.parameters.isPropertyDefined( _T("namespace") ) )
        tDesc.parentNamespace = tDesc.parameters.getProperty( _T("namespace") );
    else
        tDesc.parentNamespace = stringConvertA2CT(strNamespace.c_str());

    // Tokens should come in the following order:
    // Type, identifier, optional parameter list
    // i.e. samplerType Identifier : parameter(xyz);
    while ( t.position <= t.sectionEnd )
    {
        // Create a new sampler declaration entry.
        tDesc.samplers.resize( tDesc.samplers.size() + 1 );
        cgSamplerDesc & sDesc = tDesc.samplers.back();
        sDesc.registerIndex  = -1;
        sDesc.parentNamespace = tDesc.parentNamespace;
        
        // Constant type or end of buffer '?>'?
        if ( t.tokenType == asTC_IDENTIFIER )
        {
            // Store type name.
            sDesc.typeName = stringConvertA2CT(strScript.substr( t.tokenStart, t.tokenLength ).c_str());

        } // End if type
        else
        {
            cgToDoAssert( "Script Preprocessor", "Expected type identifier." );
            return false;

        } // End if invalid

        // Sampler identifier?
        if ( !getNextToken( t, strScript, true ) || t.tokenType != asTC_IDENTIFIER )
        {
            cgToDoAssert( "Script Preprocessor", "Expected variable identifier." );
            return false;

        } // End if eof
        sDesc.name = stringConvertA2CT( strScript.substr( t.tokenStart, t.tokenLength ).c_str() );

        // Optional parameter list ':' follows.
        if ( !getNextToken( t, strScript, true ) )
        {
            cgToDoAssert( "Script Preprocessor", "Unexpected end of file." );
            return false;

        } // End if eof

        // Parameter list?
        if ( t.tokenLength == 1 && strScript.at(t.tokenStart) == ':' )
        {
            if ( !parseShaderParameterList( t, strScript, sDesc.parameters ) )
                return false;
            
        } // End if parameter list

        // Pre-parse certain common sampler parameters.
        if ( sDesc.parameters.isPropertyDefined( _T("register") ) )
            sDesc.registerIndex = sDesc.parameters.getProperty(_T("register") );
    
        // End of constant?
        if ( !(t.tokenLength == 1 && t.tokenType == asTC_KEYWORD && strScript.at(t.tokenStart) == ';') )
        {
            cgToDoAssert( "Script Preprocessor", "Expected ';'." );
            return false;
        
        } // End if invalid constant

        // Get next token ready for next iteration (should be constant type).
        // If there is no more data, we simply reached the end of the section.
        if ( !getNextToken( t, strScript, true ) )
            break;

    } // Next Token

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : skipStatement () (Protected)
/// <summary>
/// Skip to the end of the current statement block.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptPreprocessor::skipStatement( TokenData & t, const std::string & strScript )
{
    // Skip until ; or { whichever comes first
    while ( strScript[ t.tokenStart ] != ';' && strScript[ t.tokenStart ] != '{' )
    {
        if ( !getNextToken( t, strScript, false ) )
            break;
    
    } // Next Token
    
    // Skip entire statement block
    if ( strScript[ t.tokenStart ] == '{' )
    {
        // Find the end of the statement block
        cgInt nLevel = 1;
        while ( nLevel > 0 )
        {
            if ( !getNextToken( t, strScript, false ) )
                break;
            if( t.tokenType == asTC_KEYWORD )
            {
                if ( strScript[ t.tokenStart ] == '{' )
                    nLevel++;
                else if ( strScript[ t.tokenStart ] == '}' )
                    nLevel--;
            
            } // End if keyword
        
        } // Next Token
    
    } // End if statement block
}

//-----------------------------------------------------------------------------
//  Name : excludeCode () (Protected)
/// <summary>
/// Overwrite all code with blanks until the matching #endif
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptPreprocessor::excludeCode( TokenData & t, std::string & strScript )
{
    cgInt nNested = 0;
    std::string strToken;

    // Keep processing tokens.
    while( 1 )
    {
        if ( !getNextToken( t, strScript, true ) )
            break;

        // Is this a character we need to recognize?
        size_t nDirectiveStart = t.tokenStart;
        if ( strScript[ t.tokenStart ] == '#' )
        {
            // Reset position to just following the '#' character
            // in case the tokenizer pushed us past the directive label.
            t.position = t.tokenStart + 1;

            // Retrieve the directive name / label.
            if ( !getNextToken( t, strScript, true ) )
            {
                cgToDoAssert( "Script Preprocessor", "Missing pre-processor label." );
                break;
            
            } // End if Error
            strToken.assign( &strScript[t.tokenStart], t.tokenLength );

            // Is it an #if or #endif directive?
            if ( strToken == "if" || strToken == "ifdef" || strToken == "ifndef" )
            {
                // Replace entire directive with spaces to avoid compilation errors.
                overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );

                // Plus one level deep in conditional nesting
                nNested++;

            } // End if '#if'
            else if ( strToken == "else" || strToken == "elif" || strToken == "elseif" )
            {
                // If we are currently nested, this whole following 'else' block is
                // being destroyed. Otherwise, we have reached the termination point.
                if ( nNested > 0 )
                {
                    // Replace entire directive with spaces to avoid compilation errors.
                    overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );

                } // End if nested
                else
                {
                    // Reposition back at the start of the director for
                    // further processing by the caller.
                    t.position = nDirectiveStart;
                    break;
                
                } // End if !nested
            }
            else if ( strToken == "endif" )
            {
                // If we are currently nested, this endif directive is being destroyed. 
                // Otherwise, we have reached the termination point.
                if ( nNested > 0 )
                {
                    // Replace entire directive with spaces to avoid compilation errors.
                    overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );
                    --nNested;

                } // End if nested
                else
                {
                    // Reposition back at the start of the director for
                    // further processing by the caller.
                    t.position = nDirectiveStart;
                    break;
                
                } // End if !nested
            
            } // End if '#endif'
            else
            {
                // Unknown directive.
                // Replace entire directive with spaces to avoid compilation errors.
                overwriteCode( strScript, nDirectiveStart, t.position - nDirectiveStart );
            
            } // End if '#other'
        
        } // End if '#'
        else if ( strScript[ t.tokenStart ] != '\n' )
        {
            overwriteCode( strScript, t.tokenStart, t.tokenLength );
        
        } // End if not newline

    } // Next token
}

//-----------------------------------------------------------------------------
//  Name : overwriteCode () (Protected)
/// <summary>
/// Overwrite all characters except line breaks with whitespace.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptPreprocessor::overwriteCode( std::string & strScript, size_t nStart, size_t nLength )
{
    cgChar * lpszCode = &strScript[ nStart ];
    for( size_t i = 0; i < nLength; ++i, ++lpszCode )
    {
        if ( *lpszCode != '\n' )
            *lpszCode = ' ';
    
    } // Next Character
}
