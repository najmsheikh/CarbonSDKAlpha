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
// Name : cgExceptions.cpp                                                   //
//                                                                           //
// Desc : Contains definitions for simple exception types.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgExceptions Module Includes
//-----------------------------------------------------------------------------
#include <System/cgExceptions.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    // Warning: Portability
#include <dxerr.h>      // Warning: Portability
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgExceptions;

///////////////////////////////////////////////////////////////////////////////
// ResultException Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : ResultException() (Overload Constructor)
/// <summary>
/// Class constructor.
/// </summary>
//-----------------------------------------------------------------------------
ResultException::ResultException( cgUInt32 hr, const cgString & source, const cgDebugSourceInfo & _debugSource )
{
    mResultAvailable = true;
    mResult          = hr;
    mResultSource  = source;

    // Source of exception
    if ( _debugSource.source.empty() == true )
        mTargetSite = _T("Unknown Module");
    else
        mTargetSite = cgString::format( _T("%s(%i)"), _debugSource.source.c_str(), _debugSource.line );
}

//-----------------------------------------------------------------------------
//  Name : ResultException() (Overload Constructor)
/// <summary>
/// Class constructor.
/// </summary>
//-----------------------------------------------------------------------------
ResultException::ResultException( const cgString & message, const cgDebugSourceInfo & _debugSource )
{
    mResultAvailable = false;
    mResultSource  = message;

    // Source of exception
    if ( _debugSource.source.empty() == true )
        mTargetSite = _T("Unknown Module");
    else
        mTargetSite = cgString::format( _T("%s(%i)"), _debugSource.source.c_str(), _debugSource.line );
}

//-----------------------------------------------------------------------------
//  Name : toString()
/// <summary>
/// Convert this exception to a usable string representation.
/// </summary>
//-----------------------------------------------------------------------------
cgString ResultException::toString( bool bIncludeSite /* = true */ ) const
{
    if ( mResultAvailable == true )
    {
        cgString strMessage;
        if ( bIncludeSite == true )
        {
            if ( FAILED( mResult ) )
                strMessage = mResultSource + _T("() failed in ") + mTargetSite + _T(". The error reported was ");
            else
                strMessage = mResultSource + _T("() succeeded in ") + mTargetSite + _T(". The value returned was ");
        
        } // End if include site
        else
        {
            if ( FAILED( mResult ) )
                strMessage = mResultSource + _T("() failed. The error reported was ");
            else
                strMessage = mResultSource + _T("() succeeded. The value returned was ");

        } // End if exclude site
        strMessage += cgString::format( _T("(0x%x) %s - %s"), mResult, DXGetErrorString( mResult ), DXGetErrorDescription( mResult ));
        return strMessage;

    } // End HRESULT provided
    else
    {
        if ( bIncludeSite == true )
            return mResultSource + _T(" in ") + mTargetSite + _T(".");
        else
            return mResultSource;

    } // End if no HRESULT
}