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
// Name : cgVariant.cpp                                                      //
//                                                                           //
// Desc : Variable class that allows the storage of and conversion between   //
//        multiple variable types.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgVariant Module Includes
//-----------------------------------------------------------------------------
#include <System/cgVariant.h>
#include <limits.h> // UINT_MAX

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant()
{
    // Initialize variables
    mStorageType = Type_none;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgInt32 nValue )
{
    // Initialize variables
    mStorageType = Type_int32;
    mNumeric32   = (cgUInt32&)nValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgUInt32 nValue )
{
    // Initialize variables
    mStorageType = Type_uint32;
    mNumeric32   = nValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgInt nValue )
{
    // Initialize variables
    #if (UINT_MAX > 0xFFFFFFFF)
        mStorageType = Type_int64;
        mNumeric64   = (cgUInt64&)nValue;
    #else
        mStorageType = Type_int32;
        mNumeric32   = (cgUInt32&)nValue;
    #endif
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgUInt nValue )
{
    // Initialize variables
    #if (UINT_MAX > 0xFFFFFFFF)
        mStorageType = Type_uint64;
        mNumeric64   = (cgUInt64&)nValue;
    #else
        mStorageType = Type_uint32;
        mNumeric32   = (cgUInt32&)nValue;
    #endif
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgInt64 nValue )
{
    // Initialize variables
    mStorageType = Type_int64;
    mNumeric64   = (cgUInt64&)nValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgUInt64 nValue )
{
    // Initialize variables
    mStorageType = Type_uint64;
    mNumeric64   = nValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgInt8 nValue )
{
    // Initialize variables
    mStorageType = Type_int8;
    mNumeric8    = (cgUInt8&)nValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgUInt8 nValue )
{
    // Initialize variables
    mStorageType = Type_uint8;
    mNumeric8    = nValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( bool bValue )
{
    // Initialize variables
    mStorageType = Type_bool;
    mNumeric8    = (cgUInt8&)bValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgFloat fValue )
{
    // Initialize variables
    mStorageType = Type_float;
    mNumeric32   = (cgUInt32&)fValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( cgDouble fValue )
{
    // Initialize variables
    mStorageType = Type_double;
    mNumeric64   = (cgUInt64&)fValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( const cgString & strValue )
{
    // Initialize variables
    mStorageType = Type_string;
    mString      = strValue;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( const cgTChar * strValue )
{
    // Initialize variables
    mStorageType = Type_string;
    mString      = strValue;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgInt)
/// <summary>
/// Assignment operator for storing cgInt values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgInt nValue )
{
    // Initialize variables
    #if (UINT_MAX > 0xFFFFFFFF)
        mStorageType = Type_int64;
        mNumeric32   = 0;
        mNumeric8    = 0;
        mNumeric64   = (cgUInt64&)nValue;
        mString.clear();
    #else
        mStorageType = Type_int32;
        mNumeric32   = (cgUInt32&)nValue;
        mNumeric8    = 0;
        mNumeric64   = 0;
        mString.clear();
    #endif
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgUInt)
/// <summary>
/// Assignment operator for storing cgUInt values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgUInt nValue )
{
    // Initialize variables
    #if (UINT_MAX > 0xFFFFFFFF)
        mStorageType = Type_uint64;
        mNumeric32   = 0;
        mNumeric8    = 0;
        mNumeric64   = (cgUInt64&)nValue;
        mString.clear();
    #else
        mStorageType = Type_uint32;
        mNumeric32   = (cgUInt32&)nValue;
        mNumeric8    = 0;
        mNumeric64   = 0;
        mString.clear();
    #endif
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgInt32)
/// <summary>
/// Assignment operator for storing cgInt32 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgInt32 nValue )
{
    // Initialize variables
    mStorageType = Type_int32;
    mNumeric32   = (cgUInt32&)nValue;
    mNumeric8    = 0;
    mNumeric64   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgUInt32)
/// <summary>
/// Assignment operator for storing cgUInt32 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgUInt32 nValue )
{
    // Initialize variables
    mStorageType = Type_uint32;
    mNumeric32   = nValue;
    mNumeric8    = 0;
    mNumeric64   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgInt64)
/// <summary>
/// Assignment operator for storing cgInt64 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgInt64 nValue )
{
    // Initialize variables
    mStorageType = Type_int64;
    mNumeric64   = (cgUInt64&)nValue;
    mNumeric8    = 0;
    mNumeric32   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgUInt64)
/// <summary>
/// Assignment operator for storing cgUInt64 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgUInt64 nValue )
{
    // Initialize variables
    mStorageType = Type_uint64;
    mNumeric64   = nValue;
    mNumeric8    = 0;
    mNumeric32   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgInt8)
/// <summary>
/// Assignment operator for storing cgInt8 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgInt8 nValue )
{
    // Initialize variables
    mStorageType = Type_int8;
    mNumeric8    = (cgUInt8&)nValue;
    mNumeric32   = 0;
    mNumeric64   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgUInt8)
/// <summary>
/// Assignment operator for storing cgUInt8 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgUInt8 nValue )
{
    // Initialize variables
    mStorageType = Type_uint8;
    mNumeric8    = nValue;
    mNumeric32   = 0;
    mNumeric64   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (bool)
/// <summary>
/// Assignment operator for storing bool values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( bool bValue )
{
    // Initialize variables
    mStorageType = Type_bool;
    mNumeric8    = (cgUInt8&)bValue;
    mNumeric32   = 0;
    mNumeric64   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgFloat)
/// <summary>
/// Assignment operator for storing float values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgFloat fValue )
{
    // Initialize variables
    mStorageType = Type_float;
    mNumeric32   = (cgUInt32&)fValue;
    mNumeric8    = 0;
    mNumeric64   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgDouble)
/// <summary>
/// Assignment operator for storing double values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( cgDouble fValue )
{
    // Initialize variables
    mStorageType = Type_double;
    mNumeric64   = (cgUInt64&)fValue;
    mNumeric8    = 0;
    mNumeric32   = 0;
    mString.clear();
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (const cgString&)
/// <summary>
/// Assignment operator for storing std::string / std::wstring values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( const cgString & strValue )
{
    // Initialize variables
    mStorageType = Type_string;
    mString      = strValue;
    mNumeric8    = 0;
    mNumeric32   = 0;
    mNumeric64   = 0;
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (const cgTChar*)
/// <summary>
/// Assignment operator for storing std::string / std::wstring values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( const cgTChar* strValue )
{
    // Initialize variables
    mStorageType = Type_string;
    mString      = strValue;
    mNumeric8    = 0;
    mNumeric32   = 0;
    mNumeric64   = 0;
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator cgUInt8()
/// <summary>
/// Cast operator to cast to an cgUInt8.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgUInt8() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return (cgUInt8)((cgInt8&)mNumeric8);
        case Type_uint8:
            return mNumeric8;
        case Type_bool:
            return (cgUInt8)((bool&)mNumeric8);
        case Type_int32:
            return (cgUInt8)((cgInt32&)mNumeric32);
        case Type_uint32:
            return (cgUInt8)mNumeric32;
        case Type_int64:
            return (cgUInt8)((cgInt64&)mNumeric64);
        case Type_uint64:
            return (cgUInt8)mNumeric64;
        case Type_float:
            return (cgUInt8)((cgFloat&)mNumeric32);
        case Type_double:
            return (cgUInt8)((cgDouble&)mNumeric64);
        case Type_string:
        {
            cgUInt32 nValue;
            cgStringParser( mString ) >> nValue;
            return (cgUInt8)nValue;
        
        } // End Type_string
        default:
            return 0;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgInt8()
/// <summary>
/// Cast operator to cast to an cgInt8.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgInt8() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return (cgInt8&)mNumeric8;
        case Type_uint8:
            return (cgInt8)mNumeric8;
        case Type_bool:
            return (cgInt8)((bool&)mNumeric8);
        case Type_int32:
            return (cgInt8)((cgInt32&)mNumeric32);
        case Type_uint32:
            return (cgInt8)mNumeric32;
        case Type_int64:
            return (cgInt8)((cgInt64&)mNumeric64);
        case Type_uint64:
            return (cgInt8)mNumeric64;
        case Type_float:
            return (cgInt8)((cgFloat&)mNumeric32);
        case Type_double:
            return (cgInt8)((cgDouble&)mNumeric64);
        case Type_string:
        {
            cgInt32 nValue;
            cgStringParser( mString ) >> nValue;
            return (cgInt8)nValue;
        
        } // End Type_string
        default:
            return 0;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator bool()
/// <summary>
/// Cast operator to cast to an bool.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator bool() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return ((cgInt8&)mNumeric8) != 0;
        case Type_uint8:
            return mNumeric8 != 0;
        case Type_bool:
            return (bool&)mNumeric8;
        case Type_int32:
            return ((cgInt32&)mNumeric32) != 0;
        case Type_uint32:
            return mNumeric32 != 0;
        case Type_int64:
            return ((cgInt64&)mNumeric64) != 0;
        case Type_uint64:
            return mNumeric64 != 0;
        case Type_float:
            return ((cgFloat&)mNumeric32) != 0.0f;
        case Type_double:
            return ((cgDouble&)mNumeric64) != 0.0f;
        case Type_string:
        {
            cgInt32 nValue;
            cgStringParser( mString ) >> nValue; 
            return nValue != 0;
        
        } // End Type_string
        default:
            return false;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgUInt()
/// <summary>
/// Cast operator to cast to an cgUInt.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgUInt() const
{
    #if (UINT_MAX > 0xFFFFFFFF)
        return (cgUInt)((cgUInt64)(*this));
    #else
        return (cgUInt)((cgUInt32)(*this));
    #endif
}

//-----------------------------------------------------------------------------
//  Name : operator cgInt()
/// <summary>
/// Cast operator to cast to an cgInt.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgInt() const
{
    #if (UINT_MAX > 0xFFFFFFFF)
        return (cgInt)((cgInt64)(*this));
    #else
        return (cgInt)((cgInt32)(*this));
    #endif
}

//-----------------------------------------------------------------------------
//  Name : operator cgUInt32()
/// <summary>
/// Cast operator to cast to an cgUInt32.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgUInt32() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return (cgUInt32)((cgInt8&)mNumeric8);
        case Type_uint8:
            return (cgUInt32)mNumeric8;
        case Type_bool:
            return (cgUInt32)((bool&)mNumeric8);
        case Type_int32:
            return (cgUInt32)((cgInt32&)mNumeric32);
        case Type_uint32:
            return mNumeric32;
        case Type_int64:
            return (cgUInt32)((cgInt64&)mNumeric64);
        case Type_uint64:
            return (cgUInt32)mNumeric64;
        case Type_float:
            return (cgUInt32)((cgFloat&)mNumeric32);
        case Type_double:
            return (cgUInt32)((cgDouble&)mNumeric64);
        case Type_string:
        {
            // Find the first numeric component
            size_t nStart = mString.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = mString.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = mString.size();
            
                // Parse the value.
                cgUInt32 nValue;
                cgStringParser( mString.substr( nStart, nEnd ) ) >> nValue;
                return nValue;
            
            } // End if found numeric start.
            return 0;
        
        } // End Type_string
        default:
            return 0;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgInt32()
/// <summary>
/// Cast operator to cast to an cgInt32.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgInt32() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return (cgInt32)((cgInt8&)mNumeric8);
        case Type_uint8:
            return (cgInt32)mNumeric8;
        case Type_bool:
            return (cgInt32)((bool&)mNumeric8);
        case Type_int32:
            return (cgInt32&)mNumeric32;
        case Type_uint32:
            return (cgInt32)mNumeric32;
        case Type_int64:
            return (cgInt32)((cgInt64&)mNumeric64);
        case Type_uint64:
            return (cgInt32)mNumeric64;
        case Type_float:
            return (cgInt32)((cgFloat&)mNumeric32);
        case Type_double:
            return (cgInt32)((cgDouble&)mNumeric64);
        case Type_string:
        {
            // Find the first numeric component
            size_t nStart = mString.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = mString.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = mString.size();
            
                // Parse the value.
                cgInt32 nValue;
                cgStringParser( mString.substr( nStart, nEnd ) ) >> nValue;
                return nValue;
            
            } // End if found numeric start.
            return 0;
        
        } // End Type_string
        default:
            return 0;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgUInt64()
/// <summary>
/// Cast operator to cast to an cgUInt64.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgUInt64() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return (cgUInt64)((cgInt8&)mNumeric8);
        case Type_uint8:
            return (cgUInt64)mNumeric8;
        case Type_bool:
            return (cgUInt64)((bool&)mNumeric8);
        case Type_int32:
            return (cgUInt64)((cgInt32&)mNumeric32);
        case Type_uint32:
            return (cgUInt64)mNumeric32;
        case Type_int64:
            return (cgUInt64)((cgInt64&)mNumeric64);
        case Type_uint64:
            return mNumeric64;
        case Type_float:
            return (cgUInt64)((cgFloat&)mNumeric32);
        case Type_double:
            return (cgUInt64)((cgDouble&)mNumeric64);
        case Type_string:
        {
            // Find the first numeric component
            size_t nStart = mString.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = mString.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = mString.size();
            
                // Parse the value.
                cgUInt64 nValue;
                cgStringParser( mString.substr( nStart, nEnd ) ) >> nValue;
                return nValue;
            
            } // End if found numeric start.
            return 0;
        
        } // End Type_string
        default:
            return 0;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgInt64()
/// <summary>
/// Cast operator to cast to an cgInt64.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgInt64() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return (cgInt64)((cgInt8&)mNumeric8);
        case Type_uint8:
            return (cgInt64)mNumeric8;
        case Type_bool:
            return (cgInt64)((bool&)mNumeric8);
        case Type_int32:
            return (cgInt64)((cgInt32&)mNumeric32);
        case Type_uint32:
            return (cgInt64)mNumeric32;
        case Type_int64:
            return (cgInt64)((cgInt64&)mNumeric64);
        case Type_uint64:
            return (cgInt64)mNumeric64;
        case Type_float:
            return (cgInt64)((cgFloat&)mNumeric32);
        case Type_double:
            return (cgInt64)((cgDouble&)mNumeric64);
        case Type_string:
        {
            // Find the first numeric component
            size_t nStart = mString.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = mString.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = mString.size();
            
                // Parse the value.
                cgInt64 nValue;
                cgStringParser( mString.substr( nStart, nEnd ) ) >> nValue;
                return nValue;
            
            } // End if found numeric start.
            return 0;
        
        } // End Type_string
        default:
            return 0;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgFloat()
/// <summary>
/// Cast operator to cast to a float.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgFloat() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return (cgFloat)((cgInt8&)mNumeric8);
        case Type_uint8:
            return (cgFloat)mNumeric8;
        case Type_bool:
            return (cgFloat)((bool&)mNumeric8);
        case Type_int32:
            return (cgFloat)((cgInt32&)mNumeric32);
        case Type_uint32:
            return (cgFloat)mNumeric32;
        case Type_int64:
            return (cgFloat)((cgInt64&)mNumeric64);
        case Type_uint64:
            return (cgFloat)mNumeric64;
        case Type_float:
            return (cgFloat&)mNumeric32;
        case Type_double:
            return (cgFloat)((cgDouble&)mNumeric64);
        case Type_string:
        {
            // Find the first numeric component
            size_t nStart = mString.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = mString.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = mString.size();
            
                // Parse the value.
                cgFloat fValue;
                cgStringParser( mString.substr( nStart, nEnd ) ) >> fValue;
                return fValue;
            
            } // End if found numeric start.
            return 0;
        
        } // End Type_string
        default:
            return 0.0f;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgDouble()
/// <summary>
/// Cast operator to cast to a double.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgDouble() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return (cgDouble)((cgInt8&)mNumeric8);
        case Type_uint8:
            return (cgDouble)mNumeric8;
        case Type_bool:
            return (cgDouble)((bool&)mNumeric8);
        case Type_int32:
            return (cgDouble)((cgInt32&)mNumeric32);
        case Type_uint32:
            return (cgDouble)mNumeric32;
        case Type_int64:
            return (cgDouble)((cgInt64&)mNumeric64);
        case Type_uint64:
            return (cgDouble)mNumeric64;
        case Type_float:
            return (cgDouble)((cgFloat&)mNumeric32);
        case Type_double:
            return (cgDouble&)mNumeric64;
        case Type_string:
        {
            // Find the first numeric component
            size_t nStart = mString.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = mString.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = mString.size();
            
                // Parse the value.
                cgDouble fValue;
                cgStringParser( mString.substr( nStart, nEnd ) ) >> fValue;
                return fValue;
            
            } // End if found numeric start.
            return 0;
        
        } // End Type_string
        default:
            return 0.0;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgString()
/// <summary>
/// Cast operator to cast to an cgString.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgString() const
{
    cgStringParser strParser;

    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            strParser << (cgInt32)((cgInt8&)mNumeric8);
            return strParser.str();
        case Type_uint8:
            strParser << (cgUInt32)mNumeric8;
            return strParser.str();
        case Type_bool:
            strParser << (cgInt32)((bool&)mNumeric8);
            return strParser.str();
        case Type_int32:
            strParser << ((cgInt32&)mNumeric32);
            return strParser.str();
        case Type_uint32:
            strParser << ((cgUInt32&)mNumeric32);
            return strParser.str();
        case Type_int64:
            strParser << ((cgInt64&)mNumeric64);
            return strParser.str();
        case Type_uint64:
            strParser << ((cgUInt64&)mNumeric64);
            return strParser.str();
        case Type_float:
            strParser << ((cgFloat&)mNumeric32);
            return strParser.str();
        case Type_double:
            strParser << ((cgDouble&)mNumeric64);
            return strParser.str();
        case Type_string:
            return mString;
        default:
            #if defined( UNICODE )
                return L"";
            #else
                return "";
            #endif

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : convert ()
/// <summary>
/// Convert the internal storage format of the variant type.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVariant::convert( VariantType Type )
{
    switch ( Type )
    {
        case Type_int8:
            *this = (cgInt8)*this;
            break;
        case Type_uint8:
            *this = (cgUInt8)*this;
            break;
        case Type_bool:
            *this = (bool)*this;
            break;
        case Type_int32:
            *this = (cgInt32)*this;
            break;
        case Type_uint32:
            *this = (cgUInt32)*this;
            break;
        case Type_int64:
            *this = (cgInt64)*this;
            break;
        case Type_uint64:
            *this = (cgUInt64)*this;
            break;
        case Type_float:
            *this = (cgFloat)*this;
            break;
        case Type_double:
            *this = (cgDouble)*this;
            break;
        case Type_string:
            *this = (cgString)*this;
            break;
        default:
            return false;

    } // End switch storage type

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSize ()
/// <summary>
/// Returns the length of the data (in bytes) stored here.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgVariant::getSize( ) const
{
    switch ( mStorageType )
    {
        case Type_int8:
            return sizeof(cgInt8);
            break;
        case Type_uint8:
            return sizeof(cgUInt8);
            break;
        case Type_bool:
            return sizeof(bool);
            break;
        case Type_int32:
            return sizeof(cgInt32);
            break;
        case Type_uint32:
            return sizeof(cgUInt32);
            break;
        case Type_int64:
            return sizeof(cgInt64);
            break;
        case Type_uint64:
            return sizeof(cgUInt64);
            break;
        case Type_float:
            return sizeof(cgFloat);
            break;
        case Type_double:
            return sizeof(cgDouble);
            break;
        case Type_string:
			return (cgUInt32)((cgString)*this).size( );
            break;
        default:
            return 0;

    } // End switch storage type

    // Failed
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : getData ()
/// <summary>
/// Returns a void pointer to the data stored here.
/// </summary>
//-----------------------------------------------------------------------------
const void * cgVariant::getData( ) const
{
    switch ( mStorageType )
    {
        case Type_int8:
            return (void *)(cgInt8 *)&mNumeric8;
        case Type_uint8:
            return (void *)(cgUInt8 *)&mNumeric8;
        case Type_bool:
            return (void *)(bool *)&mNumeric8;
        case Type_int32:
            return (void *)(cgInt32 *)&mNumeric32;
        case Type_uint32:
            return (void *)(cgUInt32 *)&mNumeric32;
        case Type_int64:
            return (void *)(cgInt64 *)&mNumeric64;
        case Type_uint64:
            return (void *)(cgUInt64 *)&mNumeric64;
        case Type_float:
            return (void *)(cgFloat *)&mNumeric32;
        case Type_double:
            return (void *)(cgDouble *)&mNumeric64;
        case Type_string:
            return (void *)(cgByte*)mString.c_str( );
        default:
            return CG_NULL;

    } // End switch storage type

	return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getTypeName ()
/// <summary>
/// Returns a string representation of the internal data type name.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgVariant::getTypeName( ) const
{
    static const cgString TypeNames[] =
    {
        _T("Undefined"), _T("bool"), _T("int8"), _T("uint8"), _T("int32"), 
        _T("uint32"), _T("int64"), _T("uint64"), _T("float"), _T("double"),
        _T("String")
    };
    return TypeNames[ (cgInt)mStorageType ];
}

//-----------------------------------------------------------------------------
//  Name : operator ==
/// <summary>
/// Comparison operator
/// </summary>
//-----------------------------------------------------------------------------
bool cgVariant::operator == ( const cgVariant& rhs ) const
{
	// Compare types
	if ( mStorageType != rhs.mStorageType ) return false;

    // Compare data
    switch ( mStorageType )
    {
        case Type_int8:
        case Type_uint8:
        case Type_bool:
            return mNumeric8 == rhs.mNumeric8;

        case Type_int32:
        case Type_uint32:
        case Type_float:
            return mNumeric32 == rhs.mNumeric32;

		case Type_int64:
        case Type_uint64:
        case Type_double:
            return mNumeric64 == rhs.mNumeric64;

		case Type_string:
            return mString == rhs.mString;

    } // End switch storage type

	return false;
}

//-----------------------------------------------------------------------------
//  Name : operator !=
/// <summary>
/// Comparison operator
/// </summary>
//-----------------------------------------------------------------------------
bool cgVariant::operator != ( const cgVariant& rhs ) const
{
	return !( *this == rhs );
}