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
// Name : cgScriptPreprocessor.h                                             //
//                                                                           //
// Desc : Utility class designed to provide script parsing / pre-processing  //
//        support. Handles includes, defines, pragmas etc.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCRIPTPREPROCESSOR_H_ )
#define _CGE_CGSCRIPTPREPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgScriptPreprocessor Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgScript.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScript;
class cgPropertyContainer;
struct cgConstantBufferDesc;
struct cgConstantTypeDesc;
struct cgSamplerBlockDesc;

// AngelScript.
class asIScriptModule;
class asIScriptEngine;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgScriptPreprocessor (Class)
/// <summary>
/// Provides support for script file parsing / pre-processing ready for 
/// compilation. Handles standard '#' directives such as include, define, etc.
/// as well as providing support for our engine proprietary 'surface shader'
/// tag syntax.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScriptPreprocessor
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(std::string,std::string,DefinitionMap)

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgScriptPreprocessor( cgScript * script );
    virtual ~cgScriptPreprocessor( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool        process         ( cgInputStream stream, const DefinitionMap & defines, cgScript::SourceFileArray & sourceFiles );
    
    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    static bool isValidMacro    ( std::string macroName, const std::string & macroValue );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct TokenData
    {
        size_t  sectionBegin;   // Origin of the string section being processed.
        size_t  sectionEnd;     // Last character of the section being processed.
        size_t  position;       // Current offset within the string section 
        size_t  tokenStart;     // Position denoting the start of the token
        size_t  tokenLength;    // Length of the token.
        cgInt   tokenType;      // Type of the token.
    };

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool    getNextToken                ( TokenData & t, const std::string & strScript, bool skipCommentAndWS );
    bool    expandStatement             ( TokenData & t, std::string & statement );
    bool    loadScriptSection           ( asIScriptModule * module, cgInputStream & stream, cgScript::SourceFileArray & sourceFiles, bool includeOnce );
    bool    processScriptSection        ( asIScriptModule * module, const std::string & inputCode, const cgString & sectionName, cgScript::SourceFileArray & sourceFiles );
    void    skipStatement               ( TokenData & t, const std::string & script );
    void    excludeCode                 ( TokenData & t, std::string & script );
    void    overwriteCode               ( std::string & script, size_t start, size_t length );
    
    // Script code parsers
    bool    parsePreprocessorDirective  ( TokenData & t, std::string & script, std::stack<bool> & conditionStack, asIScriptModule * module, const cgString & sectionName, cgScript::SourceFileArray & sourceFiles );
    bool    parseScriptString           ( TokenData & t, std::string & script, bool shaderParameter, bool addParentheses = false );
    bool    parseShaderBlock            ( TokenData & t, std::string & script, const std::string & parentNamespace, bool shaderCallFunction );
    bool    parseShaderCallDeclaration  ( TokenData & t, std::string & script, cgShaderCallFunctionDesc & description, const std::string & parentNamespace );

    // Shader code parsers
    bool    parseCBufferDescriptor      ( TokenData & t, const std::string & script, cgConstantBufferDesc & description, const std::string & parentNamespace );
    bool    parseTypeDescriptor         ( TokenData & t, const std::string & script, cgConstantTypeDesc & description, const std::string & parentNamespace );
    bool    parseSamplerBlock           ( TokenData & t, const std::string & script, cgSamplerBlockDesc & description, const std::string & parentNamespace );
    bool    parseShaderParameterList    ( TokenData & t, const std::string & script, cgPropertyContainer & parameters );

    //-------------------------------------------------------------------------
    // Protected Static Methods
    //-------------------------------------------------------------------------
    static bool     preprocessorConditionResult ( void * p0, cgInt t0 );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgScript          * mScript;
    DefinitionMap       mDefinitions;   // Any words/macros defined in the script (#define) or by app.
    bool                mShaderScript;  // Interpret shader replacement sections during pre-processing phase.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    static asIScriptEngine* mEngine;
};

#endif // !_CGE_CGSCRIPTPREPROCESSOR_H_