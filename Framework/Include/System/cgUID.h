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
// Name : cgUID.h                                                            //
//                                                                           //
// Desc : Globally unique identifier.                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGUID_H_ )
#define _CGE_CGUID_H_

//-----------------------------------------------------------------------------
// cgUID Header Includes
//-----------------------------------------------------------------------------
#include <cgAPI.h>

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgUID (Class)
/// <summary>
/// Globally unique identifier
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUID
{
public:
    //-------------------------------------------------------------------------
	// Public Static Methods
	//-------------------------------------------------------------------------
    static cgUID generateRandom( );

    //-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    // This allows us to use cgUID as a key in a unordered_map or unordered_set
    operator size_t( ) const
    {
        const unsigned __int8 * __x = (const unsigned __int8*)&data1;
        unsigned __int32 __h = 0;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        __h = 5*__h + *__x++;
        return size_t(__h);
    }

    //-------------------------------------------------------------------------
	// Public Variables
	//-------------------------------------------------------------------------
    unsigned __int32 data1;
    unsigned __int16 data2;
    unsigned __int16 data3;
    unsigned __int8  data4[ 8 ];

    //-------------------------------------------------------------------------
	// Public Static Variables
	//-------------------------------------------------------------------------
    // Empty UID
    static const cgUID Empty;
};

//-----------------------------------------------------------------------------
// Global Comparison Operators.
//-----------------------------------------------------------------------------
// This allows us to use a cgUID as a key in a map or set
inline bool operator < (const cgUID& u1, const cgUID& u2 )
{
    unsigned __int32 * p1 = (unsigned __int32*)&u1, * p2 = (unsigned __int32*)&u2;
    __int64 nDifference = (__int64)*p2++ - (__int64)*p1++;
    if ( nDifference != 0 ) return (nDifference < 0 );
    nDifference = (__int64)*p2++ - (__int64)*p1++;
    if ( nDifference != 0 ) return (nDifference < 0 );
    nDifference = (__int64)*p2++ - (__int64)*p1++;
    if ( nDifference != 0 ) return (nDifference < 0 );
    nDifference = (__int64)*p2++ - (__int64)*p1++;
    if ( nDifference != 0 ) return (nDifference < 0 );
    return false;

}
inline bool operator == (const cgUID& u1, const cgUID& u2 )
{
    return ( u1.data1 == u2.data1 && u1.data2 == u2.data2 && u1.data3 == u2.data3 &&
             (unsigned __int64&)u1.data4 == (unsigned __int64&)u2.data4 );
}
inline bool operator != (const cgUID& u1, const cgUID& u2 )
{
    return ( u1.data1 != u2.data1 || u1.data2 != u2.data2 || u1.data3 != u2.data3 ||
             (unsigned __int64&)u1.data4 != (unsigned __int64&)u2.data4 );
}
#if defined(__SGI_STL_PORT)
// This allows us to use cgUID as a key in a unordered_map or unordered_set
namespace std
{
    template<> struct hash<cgUID>
    {
        inline size_t operator()(const cgUID& x) const { return (size_t)x; }
    };
};
#endif // __SGI_STL_PORT

#endif // !_CGE_CGUID_H_