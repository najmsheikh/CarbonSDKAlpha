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
// Name : cgScriptPackage.h                                                  //
//                                                                           //
// Desc : Base class from which all script packages should derive. These     //
//        package classes provide bindings and other support systems that    //
//        allow scripts to interact with the application.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCRIPTPACKAGE_H_ )
#define _CGE_CGSCRIPTPACKAGE_H_

//-----------------------------------------------------------------------------
// cgScriptPackage Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgBindingUtils.h>
#include <Scripting/cgScriptEngine.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Utility Macros
//-----------------------------------------------------------------------------
#define BEGIN_SCRIPT_PACKAGE( name ) \
    public: \
        const cgTChar * getName( ) const { return _CGE_T_STRING(name); } \
        static const cgTChar * getNamespace( ) { return _CGE_T_STRING(name); } \
        bool processChildPackages( cgScriptEngine * engine, bool declare ) {

#define DECLARE_PACKAGE_CHILD( nspace ) \
            if ( declare ) { \
                if ( !engine->declarePackage( new nspace::Package(), true ) ) return false; \
            } else { \
                if ( !engine->bindPackage( nspace::Package::getNamespace( ), true ) ) return false; \
            }

#define END_SCRIPT_PACKAGE( ) \
            return true; }

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgScriptPackage (Class)
/// <summary>
/// Contains definitions for binding application functionality with
/// any available script engine.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScriptPackage
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgScriptPackage( ) {};
    virtual ~cgScriptPackage( ) {};

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual const cgTChar * getName             ( ) const = 0;
    virtual bool            processChildPackages( cgScriptEngine * pEngine, bool bDeclare ) = 0;
    virtual void            bind                ( cgScriptEngine * pEngine ) {};
    virtual void            declare             ( cgScriptEngine * pEngine ) {};
};

#endif // !_CGE_CGSCRIPTLIBRARY_H_