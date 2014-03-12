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
// Name : cgScript.h                                                         //
//                                                                           //
// Desc : Contains classes that provide the application with the ability to  //
//        interact with individual loaded script modules.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCRIPT_H_ )
#define _CGE_CGSCRIPT_H_

//-----------------------------------------------------------------------------
// cgScript Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResource.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScriptEngine;
class cgScriptObject;

// Angelscript.
class asIScriptObject;
class asIScriptModule;
class asIScriptContext;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {4766B4E9-D0D7-4AA7-B704-AF04894C8F29}
const cgUID RTID_ScriptResource = {0x4766B4E9, 0xD0D7, 0x4AA7, {0xB7, 0x4, 0xAF, 0x4, 0x89, 0x4C, 0x8F, 0x29}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgScript (Class)
/// <summary>
/// An individual script item, loaded from disk or otherwise, that
/// contains / manages all script code for execution by the engine.
/// Note : This is a resource type and as a result should be managed by the
/// resource manager to prevent duplicate entries and provide deferred
/// loading support.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScript : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgScript, cgResource, "Script" )

public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    struct SourceFileInfo
    {
        cgString    name;
        cgUInt32    hash[5];        
    };
    CGE_ARRAY_DECLARE( SourceFileInfo, SourceFileArray )

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgScript( cgUInt32 referenceId, const cgInputStream & stream, const cgString & thisType, cgScriptEngine * engine );
    virtual ~cgScript( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgScriptEngine        * getScriptEngine         ( );
    const cgInputStream   & getInputStream          ( ) const;
    cgScriptFunctionHandle  getFunctionHandle       ( const cgString & declaration );
    bool                    executeFunctionBool     ( const cgString & functionName, const cgScriptArgument::Array & arguments, bool optional = false, bool * successOut = CG_NULL );
    bool                    executeFunctionBool     ( cgScriptFunctionHandle functionHandle, const cgScriptArgument::Array & arguments, bool * successOut = CG_NULL );
    bool                    executeFunctionBool     ( const cgString & functionName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    void                    executeFunctionVoid     ( const cgString & functionName, const cgScriptArgument::Array & arguments, bool optional = false, bool   * successOut = CG_NULL );
    void                    executeFunctionVoid     ( const cgString & functionName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    cgInt32                 executeFunctionInt      ( const cgString & functionName, const cgScriptArgument::Array & arguments, bool optional = false, bool * successOut = CG_NULL );
    cgInt32                 executeFunctionInt      ( const cgString & functionName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    cgInt32                 executeFunctionInt      ( cgScriptFunctionHandle functionHandle, const cgScriptArgument::Array & arguments, bool * successOut = CG_NULL );
    cgString                executeFunctionString   ( const cgString & functionName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    cgString                executeFunctionString   ( const cgString & functionName, const cgScriptArgument::Array & arguments, bool optional = false, bool * successOut = CG_NULL );
    cgScriptObject        * executeFunctionObject   ( const cgString & objectType, const cgString & functionName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    cgScriptObject        * executeFunctionObject   ( const cgString & objectType, const cgString & functionName, const cgScriptArgument::Array & arguments, bool optional = false, bool * successOut = CG_NULL );
    cgScriptObject        * createObjectInstance    ( const cgString & objectType );
    cgScriptObject        * createObjectInstance    ( const cgString & objectType, bool uninitialized );
    cgScriptObject        * createObjectInstance    ( const cgString & objectType, const cgScriptArgument::Array & arguments );
    cgScriptObject        * createObjectInstance    ( const cgString & objectType, bool uninitialized, const cgScriptArgument::Array & arguments );
    bool                    setThisObject           ( void * value );
    const SourceFileArray & getSourceInfo           ( ) const;
    const cgString        & getThisType             ( ) const;
    bool                    defineMacro             ( const cgString & name, const cgString & value );
    bool                    isMacroDefined          ( const cgString & name ) const;
    bool                    isFailed                ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool            loadResource            ( );
    virtual bool            unloadResource          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_ScriptResource; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgString, cgScriptFunctionHandle, FunctionMap)
    CGE_UNORDEREDMAP_DECLARE(std::string,std::string,DefinitionMap)
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    asIScriptContext      * scriptExecute           ( const cgString & declaration, const cgScriptArgument::Array & arguments, bool optional );
    asIScriptContext      * scriptExecute           ( cgScriptFunctionHandle functionHandle, const cgScriptArgument::Array & arguments );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    DefinitionMap       mDefinitions;   // Any words/macros defined in the script (#define) or by app.
    cgInputStream       mInputStream;   // The stream from which to load the script
    cgScriptEngine    * mScriptEngine;  // The parent script engine to which this script is assigned
    FunctionMap         mFunctionCache; // Cached list of function handles sorted by associated declarator.
    cgString            mThisTypeName;  // If requested, scripts can have access to a "this" global of the specified type.
    SourceFileArray     mSourceFiles;   // List of source files (and their hashes) that were utilized when generating the script code.
    bool                mFailedState;   // Script failed to compile / load last time it loadResource() was called.
};

//-----------------------------------------------------------------------------
//  Name : cgScriptObject (Class)
/// <summary>
/// Wrapper class designed to manage objects allocated and of a type
/// created exclusively within the script itself.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScriptObject
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgScriptObject( cgScript * script, void * object );
    virtual ~cgScriptObject( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    addRef              ( );
    cgUInt32                release             ( );
    void                  * executeMethod       ( cgScriptFunctionHandle methodHandle, const cgScriptArgument::Array & arguments, bool * successOut = CG_NULL );
    void                  * executeMethod       ( cgScriptFunctionHandle methodHandle, const cgScriptCompatibleStruct & argumentStruct, bool * successOut = CG_NULL );
    void                    executeMethodVoid   ( const cgString & methodName, const cgScriptArgument::Array & arguments, bool optional = false, bool * successOut = CG_NULL );
    void                    executeMethodVoid   ( const cgString & methodName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    bool                    executeMethodBool   ( const cgString & methodName, const cgScriptArgument::Array & arguments, bool optional = false, bool * successOut = CG_NULL );
    bool                    executeMethodBool   ( const cgString & methodName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    cgInt32                 executeMethodInt    ( const cgString & methodName, const cgScriptArgument::Array & arguments, bool optional = false, bool * successOut = CG_NULL );
    cgInt32                 executeMethodInt    ( const cgString & methodName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    cgString                executeMethodString ( const cgString & methodName, const cgScriptArgument::Array & arguments, bool optional = false, bool * successOut = CG_NULL );
    cgString                executeMethodString ( const cgString & methodName, const cgScriptCompatibleStruct & argumentStruct, bool optional = false, bool * successOut = CG_NULL );
    cgString                getTypeName         ( ) const;
    cgScriptFunctionHandle  getMethodHandle     ( const cgString & declaration );
    void                  * getAddressOfMember  ( const cgString & memberName );
    asIScriptObject       * getInternalObject   ( );
    void                    setUserData         ( cgUInt32 id, void * value );
    void                  * getUserData         ( cgUInt32 id );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgString,cgScriptFunctionHandle,MethodMap)
    CGE_UNORDEREDMAP_DECLARE(cgScriptFunctionHandle,cgString,MethodDeclMap)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                buildMethodDeclaration  ( const cgString & returnType, const cgString & methodName, const cgScriptArgument::Array & arguments, cgString & declaration );
    asIScriptContext  * scriptExecute           ( cgScriptFunctionHandle methodHandle, const cgScriptArgument::Array & arguments );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32            mRefCount;          // Number of references currently held to this script object.
    cgScript          * mScript;            // The parent script to which this object belongs.
    asIScriptObject   * mObject;            // Core angelscript object wrapper.
    MethodMap           mMethodCache;       // Cached list of method identifiers sorted by associated declarator.
    // ToDo: 6767 -- I think this can be removed now.
    MethodDeclMap       mMethodDeclCache;   // Cached list of method declarators sorted by associated method identifier (reverse lookup for above).
};

#endif // !_CGE_CGSCRIPT_H_