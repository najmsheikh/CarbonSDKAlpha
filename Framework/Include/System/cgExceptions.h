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
// Name : cgExceptions.h                                                     //
//                                                                           //
// Desc : Contains definitions for simple exception types.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGEXCEPTIONS_H_ )
#define _CGE_CGEXCEPTIONS_H_

//-----------------------------------------------------------------------------
// cgExceptions Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// cgExceptions Namespace
//-----------------------------------------------------------------------------
namespace cgExceptions
{
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : ResultException (Class)
    // Desc : HRESULT return value exception class.
    //-------------------------------------------------------------------------
    class CGE_API ResultException
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
        ResultException( cgUInt32 hr, const cgString & sourceFunc, const cgDebugSourceInfo & _debugSource );
        ResultException( const cgString & message, const cgDebugSourceInfo & _debugSource );

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        cgString toString( bool includeSite = true ) const;

    protected:
        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        bool        mResultAvailable;
        cgUInt32    mResult;
        cgString    mResultSource;
        cgString    mTargetSite;
        
    }; // End Class ResultException
	

}; // End Namespace : cgExceptions

#endif // !_CGE_CGEXCEPTIONS_H_