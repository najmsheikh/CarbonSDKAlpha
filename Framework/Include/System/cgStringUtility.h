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
// Name : cgStringUtility.h                                                  //
//                                                                           //
// Desc : Simple namespace containing various different string utility       //
//        functions.                                                         //
//                                                                           //
// Note : A special thanks to Joe O'Leary (http://www.joeo.net/stdstring.htm)//
//        for the original implementation of the conversion routines on      //
//        which these are based.                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSTRINGUTILITY_H_ )
#define _CGE_CGSTRINGUTILITY_H_

//-----------------------------------------------------------------------------
// cgStringUtility Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <vector>
#include <locale>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgVector2;
class cgVector3;
class cgVector4;

//-----------------------------------------------------------------------------
// cgStringUtility Namespace
//-----------------------------------------------------------------------------
namespace cgStringUtility
{
    // Functions
    bool       CGE_API tokenize                ( const cgString & value, cgStringArray & tokens, const cgString & delimiters );
    bool       CGE_API parseBool               ( const cgString & value );
    bool       CGE_API tryParse                ( const cgString & value, cgUID & output );
    bool       CGE_API tryParse                ( const cgString & value, cgPoint & output );
    bool       CGE_API tryParse                ( const cgString & value, cgPointF & output );
    bool       CGE_API tryParse                ( const cgString & value, cgSize & output );
    bool       CGE_API tryParse                ( const cgString & value, cgRect & output );
    bool       CGE_API tryParse                ( const cgString & value, cgColorValue & output );
    bool       CGE_API tryParse                ( const cgString & value, cgVector2 & output );
    bool       CGE_API tryParse                ( const cgString & value, cgVector3 & output );
    bool       CGE_API tryParse                ( const cgString & value, cgVector4 & output );
    cgString   CGE_API toString                ( const cgUID & value, const cgString & format );
    int        CGE_API getPrivateProfileIntEx  ( const cgTChar * applicationName, const cgTChar * keyName, cgInt default, const cgTChar * fileName );
    cgFloat    CGE_API getPrivateProfileFloat  ( const cgTChar * applicationName, const cgTChar * keyName, cgFloat default, const cgTChar * fileName );
    bool       CGE_API writePrivateProfileIntEx( const cgTChar * applicationName, const cgTChar * keyName, cgInt value, const cgTChar * fileName, bool hexadecimal = false );
    bool       CGE_API writePrivateProfileFloat( const cgTChar * applicationName, const cgTChar * keyName, cgFloat value, const cgTChar * fileName );
    cgString   CGE_API getClipboardText        ( );
    void       CGE_API setClipboardText        ( const cgString & value );
    cgString   CGE_API fromStringTable         ( void * moduleInstance, cgUInt32 stringId );
    bool       CGE_API isTextUnicode           ( const void* data, cgUInt32 lengthInBytes );
    cgString   CGE_API addSlashes              ( const cgString & value );
    cgString   CGE_API URLEncode               ( const cgString & value );
    // ToDo: 6767 -- where is this used and do we need it?
    cgString   CGE_API Char2HEX                ( cgTChar c );
    cgString   CGE_API beautifyCode            ( const cgString & sourceCode );
    bool       CGE_API parseCommandLine        ( const cgString & commandLine, cgStringArray & argumentsOut );
    cgString   CGE_API stripTrailingNumbers    ( const cgString & value );

}; // End Namespace cgStringUtility

#endif // !_CGE_CGSTRINGUTILITY_H_