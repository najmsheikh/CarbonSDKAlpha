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
// Name : cgScriptEngine.h                                                   //
//                                                                           //
// Desc : Contains classes which provides the core of the scripting support  //
//        in the engine.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCRIPTENGINE_H_ )
#define _CGE_CGSCRIPTENGINE_H_

//-----------------------------------------------------------------------------
// cgScriptEngine Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceHandles.h>
#include "../../Lib/AngelScript/include/angelscript.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgInputStream;
class cgResourceManager;
class cgScriptLibrary;
class cgScriptPackage;
class cgScript;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgScriptEngine (Class)
/// <summary>
/// The scripting engine class is designed as a base class from which the
/// engine or application can derive their own engine types. This is not
/// a requirement however as the combination of the base script engine
/// class and appropriate binding packages should suffice.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScriptEngine
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgScript;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgScriptEngine( );
    virtual ~cgScriptEngine( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgScriptEngine * getInstance     ( );
    static void             createSingleton ( );
    static void             destroySingleton( );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                initialize                  ( );
    bool                isLibraryDeclared           ( const cgString & library ) const;
    bool                isLibraryLinked             ( const cgString & library ) const;
    bool                declareLibrary              ( const cgString & library );
    bool                linkLibrary                 ( const cgString & library );
    bool                linkLibrary                 ( const cgInputStream & stream, const cgString & libraryName, cgResourceManager * resourceManager = CG_NULL );
    void                unloadScripts               ( );
    void                enableVerboseOutput         ( bool enable );
    void                garbageCollect              ( );

    bool                declarePackage              ( cgScriptPackage * package, bool declareChildren );
    bool                bindPackage                 ( const cgString & packageNamespace, bool bindChildren );


    // Registration
    cgInt               setDefaultNamespace         ( const cgChar * namespaceNames );
    cgInt               registerObjectMethod        ( const cgChar * typeName, const cgChar * declaration, const asSFuncPtr & functionPointer, cgUInt32 callingConvention );
    cgInt               registerObjectProperty      ( const cgChar * typeName, const cgChar * declaration, cgInt byteOffset );
    cgInt               registerObjectType          ( const cgChar * typeName, cgInt byteSize, cgUInt32 flags );
    cgInt               registerObjectBehavior      ( const cgChar * typeName, asEBehaviours behavior, const cgChar * declaration, const asSFuncPtr & functionPointer, cgUInt32 callingConvention );
    cgInt               registerInterface           ( const cgChar * typeName );
    cgInt               registerInterfaceMethod     ( const cgChar * typeName, const cgChar * declaration );
    cgInt               registerStringFactory       ( const cgChar * typeName, const asSFuncPtr & factoryFunctionPointer, cgUInt32 callingConvention );
    cgInt               registerGlobalFunction      ( const cgChar * declaration, const asSFuncPtr & functionPointer, cgUInt32 callingConvention );
    cgInt               registerGlobalProperty      ( const cgChar * declaration, void * value );
    cgInt               registerEnum                ( const cgChar * typeName );
    cgInt               registerEnumValue           ( const cgChar * typeName, const cgChar * valueName, cgInt value );

    // Internal methods
    asIScriptEngine   * getInternalEngine           ( ) { return mEngine; }
    asIScriptContext  * getIdleExecuteContext       ( );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct CGE_API PackageEntry
    {
        CGE_UNORDEREDMAP_DECLARE(cgString, PackageEntry, Map)
        cgScriptPackage   * package;
        Map                 childPackages;

        // Constructor
        PackageEntry() : package(CG_NULL) {}
    };

    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_LIST_DECLARE        (asIScriptContext*, ContextList)
    CGE_UNORDEREDMAP_DECLARE(cgString, cgScriptLibrary*, LinkedLibraryMap)
    CGE_UNORDEREDMAP_DECLARE(cgString, cgScriptHandle, ScriptedLibraryMap )
    CGE_UNORDEREDSET_DECLARE(void*, LoadedScriptSet )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                unbindPackage           ( PackageEntry * entry );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    asIScriptEngine   * mEngine;                // Core angelscript engine object
    ContextList         mExecuteContexts;       // A list of contexts in which scripts will execute.
    PackageEntry        mRootPackage;           // Root of hierarchical structure of registered script packages (arranged by namespace).
    /// ToDo: 6767 - Can C++ side library stuff go?
    LinkedLibraryMap    mLinkedLibraries;       // Array containing all libraries that have been linked with this engine
    ScriptedLibraryMap  mScriptedLibraries;     // Array containing /scripted/ libraries that have been linked with this engine
    LoadedScriptSet     mLoadedScripts;         // Scripts currently bound to this engine.
    bool                mVerboseOutput;         // Output compiler failures to log?

private:
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgScriptEngine * mSingleton;       // Static singleton object instance.

    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static void messageCallback ( const asSMessageInfo *msg, void *param );
};

#endif // !_CGE_CGSCRIPTENGINE_H_