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
// Name : cgScriptLibrary.h                                                  //
//                                                                           //
// Desc : Base class from which all script libraries should derive. These    //
//        library classes provide bindings and other support systems that    //
//        allow scripts to interact with the application.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCRIPTLIBRARY_H_ )
#define _CGE_CGSCRIPTLIBRARY_H_

// ToDo: 6767 - Can this go for C++ side script libraries (superceded by packages?)

//-----------------------------------------------------------------------------
// cgScriptLibrary Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <vector>
#include <map>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScriptEngine;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgScriptLibrary (Class)
/// <summary>
/// Contains definitions for binding application functionality with
/// any available script engine.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScriptLibrary
{
public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    typedef cgScriptLibrary* (*LibraryAllocFunc)( const cgString& );
    
    // Describes an external dependency on which this library relies.
    struct Dependency
    {
        Dependency() {}
        Dependency( const cgString & _lib, bool _dec ) :
            library( _lib ), declareOnly(_dec) {}

        cgString library;       // The library on which this one depends
        bool     declareOnly;   // Is a declaration only, or full link required?
    
    }; // End Struct Dependency
    CGE_ARRAY_DECLARE(Dependency, DependencyArray)

    // Describes the result of a declaration or linking step.
    enum LinkResult
    {
        Success          = 0,
        AlreadyProcessed = 1,
        Failed           = 2
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgScriptLibrary( const cgString & name );
    virtual ~cgScriptLibrary( );

    //-------------------------------------------------------------------------
    // Public Static Method
    //-------------------------------------------------------------------------
    static void                 registerType    ( const cgString & typeName, LibraryAllocFunc pFunction );
    static cgScriptLibrary    * createInstance  ( const cgString & typeName );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgString            & getName         ( ) const;
    bool                        isDeclared      ( ) const;
    bool                        isLinked        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual LinkResult          declare         ( cgScriptEngine * scriptEngine );
    virtual LinkResult          link            ( cgScriptEngine * scriptEngine );
    virtual DependencyArray     getDependencies ( ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgString    mLibraryName;       // Name of this script library
    bool        mIsDeclared;        // Has the declaration pass been run?
    bool        mIsLinked;          // Has the linking pass been run?

    //-------------------------------------------------------------------------
    // Private Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgString, LibraryAllocFunc, LibraryAllocTypeMap)

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static LibraryAllocTypeMap  mRegisteredLibs;  // All of the library types registered with the system
};

#endif // !_CGE_CGSCRIPTLIBRARY_H_