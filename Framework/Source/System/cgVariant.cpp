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
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgVariant Module Includes
//-----------------------------------------------------------------------------
#include <System/cgVariant.h>
#include <System/cgStringUtility.h>
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
    mData        = CG_NULL;
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
    mData        = new cgInt32(nValue);
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
    mData        = new cgUInt32(nValue);
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
        mData        = new cgInt64(nValue);
    #else
        mStorageType = Type_int32;
        mData        = new cgInt32(nValue);
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
        mData        = new cgUInt64(nValue);
    #else
        mStorageType = Type_uint32;
        mData        = new cgUInt32(nValue);
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
    mData        = new cgInt64(nValue);
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
    mData        = new cgUInt64(nValue);
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
    mData        = new cgInt8(nValue);
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
    mData        = new cgUInt8(nValue);
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
    mData        = new bool(bValue);
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
    mData        = new cgFloat(fValue);
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( const cgVector2 & vValue )
{
    // Initialize variables
    mStorageType = Type_vector2;
    mData        = new cgVector2(vValue);
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( const cgVector3 & vValue )
{
    // Initialize variables
    mStorageType = Type_vector3;
    mData        = new cgVector3(vValue);
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( const cgVector4 & vValue )
{
    // Initialize variables
    mStorageType = Type_vector4;
    mData        = new cgVector4(vValue);
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
    mData        = new cgDouble(fValue);
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
    mData        = new cgString(strValue);
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
    mData        = new cgString(strValue);
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( const cgVariant & v )
{
    // Initialize variables
    mData        = CG_NULL;
    mStorageType = Type_none;

    // Use assignment
    *this = v;
}

//-----------------------------------------------------------------------------
//  Name : cgVariant () (Constructor)
/// <summary>
/// cgVariant Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::cgVariant( void * pData, VariantType Type )
{
    // Initialize variables
    mData        = CG_NULL;
    mStorageType = Type_none;

    // Use assignment
    if ( pData )
    {
        switch ( Type )
        {
            case Type_int8:
                *this = *(cgInt8*)pData;
                break;
            case Type_uint8:
                *this = *(cgUInt8*)pData;
                break;
            case Type_bool:
                *this = *(bool*)pData;
                break;
            case Type_int32:
                *this = *(cgInt32*)pData;
                break;
            case Type_uint32:
                *this = *(cgUInt32*)pData;
                break;
            case Type_int64:
                *this = *(cgInt64*)pData;
                break;
            case Type_uint64:
                *this = *(cgUInt64*)pData;
                break;
            case Type_float:
                *this = *(cgFloat*)pData;
                break;
            case Type_vector2:
                *this = *(cgVector2*)pData;
                break;
            case Type_vector3:
                *this = *(cgVector3*)pData;
                break;
            case Type_vector4:
                *this = *(cgVector4*)pData;
                break;
            case Type_double:
                *this = *(cgDouble*)pData;
                break;
            case Type_string:
                *this = (const cgTChar*)pData;
                break;

        } // End switch type

    } // End if valid data
}

//-----------------------------------------------------------------------------
//  Name : ~cgVariant () (Destructor)
/// <summary>
/// cgVariant Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::~cgVariant( )
{
    releaseData();
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgInt)
/// <summary>
/// Assignment operator for storing cgInt values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( const cgVariant & v )
{
    switch ( v.mStorageType )
    {
        case Type_int8:
            *this = (cgInt8)v;
            break;
        case Type_uint8:
            *this = (cgUInt8)v;
            break;
        case Type_bool:
            *this = (bool)v;
            break;
        case Type_int32:
            *this = (cgInt32)v;
            break;
        case Type_uint32:
            *this = (cgUInt32)v;
            break;
        case Type_int64:
            *this = (cgInt64)v;
            break;
        case Type_uint64:
            *this = (cgUInt64)v;
            break;
        case Type_float:
            *this = (cgFloat)v;
            break;
        case Type_vector2:
            *this = (cgVector2)v;
            break;
        case Type_vector3:
            *this = (cgVector3)v;
            break;
        case Type_vector4:
            *this = (cgVector4)v;
            break;
        case Type_double:
            *this = (cgDouble)v;
            break;
        case Type_string:
            *this = (cgString)v;
            break;
        default:
            releaseData();
            break;

    } // End switch type
    return *this;
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
        releaseData();
        mStorageType = Type_int64;
        mData        = new cgInt64(nValue);
    #else
        releaseData();
        mStorageType = Type_int32;
        mData        = new cgInt32(nValue);
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
        releaseData();
        mStorageType = Type_uint64;
        mData        = new cgUInt64(nValue);
    #else
        releaseData();
        mStorageType = Type_uint32;
        mData        = new cgUInt32(nValue);
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
    releaseData();
    mStorageType = Type_int32;
    mData        = new cgInt32(nValue);
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
    releaseData();
    mStorageType = Type_uint32;
    mData        = new cgUInt32(nValue);
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
    releaseData();
    mStorageType = Type_int64;
    mData        = new cgInt64(nValue);
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
    releaseData();
    mStorageType = Type_uint64;
    mData        = new cgUInt64(nValue);
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
    releaseData();
    mStorageType = Type_int8;
    mData        = new cgInt8(nValue);
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
    releaseData();
    mStorageType = Type_uint8;
    mData        = new cgUInt8(nValue);
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
    releaseData();
    mStorageType = Type_bool;
    mData        = new bool(bValue);
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
    releaseData();
    mStorageType = Type_float;
    mData        = new cgFloat(fValue);
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgVector2)
/// <summary>
/// Assignment operator for storing vector2 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( const cgVector2 & vValue )
{
    // Initialize variables
    releaseData();
    mStorageType = Type_vector2;
    mData        = new cgVector2(vValue);
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgVector3)
/// <summary>
/// Assignment operator for storing vector3 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( const cgVector3 & vValue )
{
    // Initialize variables
    releaseData();
    mStorageType = Type_vector3;
    mData        = new cgVector3(vValue);
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgVector4)
/// <summary>
/// Assignment operator for storing vector4 values.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgVariant::operator=( const cgVector4 & vValue )
{
    // Initialize variables
    releaseData();
    mStorageType = Type_vector4;
    mData        = new cgVector4(vValue);
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
    releaseData();
    mStorageType = Type_double;
    mData        = new cgDouble(fValue);
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
    releaseData();
    mStorageType = Type_string;
    mData        = new cgString(strValue);
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
    releaseData();
    mStorageType = Type_string;
    mData        = new cgString(strValue);
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
            return (cgUInt8)(*(cgInt8*)mData);
        case Type_uint8:
            return *(cgUInt8*)mData;
        case Type_bool:
            return (cgUInt8)(*(bool*)mData);
        case Type_int32:
            return (cgUInt8)(*(cgInt32*)mData);
        case Type_uint32:
            return (cgUInt8)(*(cgUInt32*)mData);
        case Type_int64:
            return (cgUInt8)(*(cgInt64*)mData);
        case Type_uint64:
            return (cgUInt8)(*(cgUInt64*)mData);;
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return (cgUInt8)(*(cgFloat*)mData);
        case Type_double:
            return (cgUInt8)(*(cgDouble*)mData);
        case Type_string:
        {
            cgUInt32 nValue;
            cgStringParser( *(cgString*)mData ) >> nValue;
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
            return *(cgInt8*)mData;
        case Type_uint8:
            return (cgInt8)(*(cgUInt8*)mData);
        case Type_bool:
            return (cgInt8)(*(bool*)mData);
        case Type_int32:
            return (cgInt8)(*(cgInt32*)mData);
        case Type_uint32:
            return (cgInt8)(*(cgUInt32*)mData);
        case Type_int64:
            return (cgInt8)(*(cgInt64*)mData);
        case Type_uint64:
            return (cgInt8)(*(cgUInt64*)mData);
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return (cgInt8)(*(cgFloat*)mData);
        case Type_double:
            return (cgInt8)(*(cgDouble*)mData);
        case Type_string:
        {
            cgInt32 nValue;
            cgStringParser( *(cgString*)mData ) >> nValue;
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
            return (*(cgInt8*)mData) != 0;
        case Type_uint8:
            return (*(cgUInt8*)mData) != 0;
        case Type_bool:
            return *(bool*)mData;
        case Type_int32:
            return (*(cgInt32*)mData) != 0;
        case Type_uint32:
            return (*(cgUInt32*)mData) != 0;
        case Type_int64:
            return (*(cgInt64*)mData) != 0;
        case Type_uint64:
            return (*(cgUInt64*)mData) != 0;
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return (*(cgFloat*)mData) != 0.0f;
        case Type_double:
            return (*(cgDouble*)mData) != 0.0;
        case Type_string:
        {
            cgInt32 nValue;
            cgStringParser( *(cgString*)mData ) >> nValue; 
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
            return (cgUInt32)(*(cgInt8*)mData);
        case Type_uint8:
            return (cgUInt32)(*(cgUInt8*)mData);
        case Type_bool:
            return (cgUInt32)(*(bool*)mData);
        case Type_int32:
            return (cgUInt32)(*(cgInt32*)mData);
        case Type_uint32:
            return *(cgInt32*)mData;
        case Type_int64:
            return (cgUInt32)(*(cgInt64*)mData);
        case Type_uint64:
            return (cgUInt32)(*(cgUInt64*)mData);
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return (cgUInt32)(*(cgFloat*)mData);
        case Type_double:
            return (cgUInt32)(*(cgDouble*)mData);
        case Type_string:
        {
            // Find the first numeric component
            const cgString & StringData = *(cgString*)mData;
            size_t nStart = StringData.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = StringData.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = StringData.size();
            
                // Parse the value.
                cgUInt32 nValue;
                cgStringParser( StringData.substr( nStart, nEnd ) ) >> nValue;
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
            return (cgInt32)(*(cgInt8*)mData);
        case Type_uint8:
            return (cgInt32)(*(cgUInt8*)mData);
        case Type_bool:
            return (cgInt32)(*(bool*)mData);
        case Type_int32:
            return *(cgInt32*)mData;
        case Type_uint32:
            return (cgInt32)(*(cgUInt32*)mData);
        case Type_int64:
            return (cgInt32)(*(cgInt64*)mData);
        case Type_uint64:
            return (cgInt32)(*(cgUInt64*)mData);
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return (cgInt32)(*(cgFloat*)mData);
        case Type_double:
            return (cgInt32)(*(cgDouble*)mData);
        case Type_string:
        {
            // Find the first numeric component
            const cgString & StringData = *(cgString*)mData;
            size_t nStart = StringData.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = StringData.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = StringData.size();
            
                // Parse the value.
                cgInt32 nValue;
                cgStringParser( StringData.substr( nStart, nEnd ) ) >> nValue;
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
            return (cgUInt64)(*(cgInt8*)mData);
        case Type_uint8:
            return (cgUInt64)(*(cgUInt8*)mData);
        case Type_bool:
            return (cgUInt64)(*(bool*)mData);
        case Type_int32:
            return (cgUInt64)(*(cgInt32*)mData);
        case Type_uint32:
            return (cgUInt64)(*(cgUInt32*)mData);
        case Type_int64:
            return (cgUInt64)(*(cgInt64*)mData);
        case Type_uint64:
            return *(cgInt8*)mData;
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return (cgUInt64)(*(cgFloat*)mData);
        case Type_double:
            return (cgUInt64)(*(cgDouble*)mData);
        case Type_string:
        {
            // Find the first numeric component
            const cgString & StringData = *(cgString*)mData;
            size_t nStart = StringData.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = StringData.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = StringData.size();
            
                // Parse the value.
                cgUInt64 nValue;
                cgStringParser( StringData.substr( nStart, nEnd ) ) >> nValue;
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
            return (cgInt64)(*(cgInt8*)mData);
        case Type_uint8:
            return (cgInt64)(*(cgUInt8*)mData);
        case Type_bool:
            return (cgInt64)(*(bool*)mData);
        case Type_int32:
            return (cgInt64)(*(cgInt32*)mData);
        case Type_uint32:
            return (cgInt64)(*(cgUInt32*)mData);
        case Type_int64:
            return *(cgInt64*)mData;
        case Type_uint64:
            return (cgInt64)(*(cgUInt64*)mData);
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return (cgInt64)(*(cgFloat*)mData);
        case Type_double:
            return (cgInt64)(*(cgDouble*)mData);
        case Type_string:
        {
            // Find the first numeric component
            const cgString & StringData = *(cgString*)mData;
            size_t nStart = StringData.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = StringData.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = StringData.size();
            
                // Parse the value.
                cgInt64 nValue;
                cgStringParser( StringData.substr( nStart, nEnd ) ) >> nValue;
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
            return (cgFloat)(*(cgInt8*)mData);
        case Type_uint8:
            return (cgFloat)(*(cgUInt8*)mData);
        case Type_bool:
            return (cgFloat)(*(bool*)mData);
        case Type_int32:
            return (cgFloat)(*(cgInt32*)mData);
        case Type_uint32:
            return (cgFloat)(*(cgUInt32*)mData);
        case Type_int64:
            return (cgFloat)(*(cgInt64*)mData);
        case Type_uint64:
            return (cgFloat)(*(cgUInt64*)mData);
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return *(cgFloat*)mData;
        case Type_double:
            return (cgFloat)(*(cgDouble*)mData);;
        case Type_string:
        {
            // Find the first numeric component
            const cgString & StringData = *(cgString*)mData;
            size_t nStart = StringData.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = StringData.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = StringData.size();
            
                // Parse the value.
                cgFloat fValue;
                cgStringParser( StringData.substr( nStart, nEnd ) ) >> fValue;
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
            return (cgDouble)(*(cgInt8*)mData);
        case Type_uint8:
            return (cgDouble)(*(cgUInt8*)mData);
        case Type_bool:
            return (cgDouble)(*(bool*)mData);
        case Type_int32:
            return (cgDouble)(*(cgInt32*)mData);
        case Type_uint32:
            return (cgDouble)(*(cgUInt32*)mData);
        case Type_int64:
            return (cgDouble)(*(cgInt64*)mData);
        case Type_uint64:
            return (cgDouble)(*(cgUInt64*)mData);
        case Type_float:
        case Type_vector2:
        case Type_vector3:
        case Type_vector4:
            return (cgDouble)(*(cgFloat*)mData);
        case Type_double:
            return *(cgDouble*)mData;
        case Type_string:
        {
            // Find the first numeric component
            const cgString & StringData = *(cgString*)mData;
            size_t nStart = StringData.find_first_of( _T("-1234567890.") );
            if ( nStart != cgString::npos )
            {
                size_t nEnd = StringData.find_first_not_of( _T("1234567890."), nStart + 1 );
                if ( nEnd == cgString::npos )
                    nEnd = StringData.size();
            
                // Parse the value.
                cgDouble fValue;
                cgStringParser( StringData.substr( nStart, nEnd ) ) >> fValue;
                return fValue;
            
            } // End if found numeric start.
            return 0;
        
        } // End Type_string
        default:
            return 0.0;

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgVector2()
/// <summary>
/// Cast operator to cast to a vector2.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgVector2() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return cgVector2((cgFloat)(*(cgInt8*)mData), 0 );
        case Type_uint8:
            return cgVector2((cgFloat)(*(cgUInt8*)mData), 0 );
        case Type_bool:
            return cgVector2((cgFloat)(*(bool*)mData), 0 );
        case Type_int32:
            return cgVector2((cgFloat)(*(cgInt32*)mData), 0 );
        case Type_uint32:
            return cgVector2((cgFloat)(*(cgUInt32*)mData), 0 );
        case Type_int64:
            return cgVector2((cgFloat)(*(cgInt64*)mData), 0 );
        case Type_uint64:
            return cgVector2((cgFloat)(*(cgUInt64*)mData), 0 );
        case Type_float:
            return cgVector2(*(cgFloat*)mData, 0 );
        case Type_vector2:
            return *(cgVector2*)mData;
        case Type_vector3:
            return (cgVector2&)(*(cgVector3*)mData);
        case Type_vector4:
            return (cgVector2&)(*(cgVector4*)mData);
        case Type_double:
            return cgVector2((cgFloat)(*(cgDouble*)mData), 0 );
        case Type_string:
        {
            // Tokenize the string into separate components.
            cgStringArray aTokens;
            if ( !cgStringUtility::tokenize( *(cgString*)mData, aTokens, _T(",") ) )
                return cgVector2(0,0);

            // Convert components.
            cgVector2 vOut(0,0);
            size_t nComponents = std::min<size_t>( aTokens.size(), 2 );
            for ( size_t i = 0; i < nComponents; ++i )
                cgStringParser( aTokens[i] ) >> vOut[i];
            return vOut;
        
        } // End Type_string
        default:
            return cgVector2(0,0);

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgVector3()
/// <summary>
/// Cast operator to cast to a vector3.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgVector3() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return cgVector3((cgFloat)(*(cgInt8*)mData), 0, 0 );
        case Type_uint8:
            return cgVector3((cgFloat)(*(cgUInt8*)mData), 0, 0 );
        case Type_bool:
            return cgVector3((cgFloat)(*(bool*)mData), 0, 0 );
        case Type_int32:
            return cgVector3((cgFloat)(*(cgInt32*)mData), 0, 0 );
        case Type_uint32:
            return cgVector3((cgFloat)(*(cgUInt32*)mData), 0, 0 );
        case Type_int64:
            return cgVector3((cgFloat)(*(cgInt64*)mData), 0, 0 );
        case Type_uint64:
            return cgVector3((cgFloat)(*(cgUInt64*)mData), 0, 0 );
        case Type_float:
            return cgVector3(*(cgFloat*)mData, 0, 0 );
        case Type_vector2:
            return cgVector3( (*(cgVector2*)mData).x, (*(cgVector2*)mData).y, 0 );
        case Type_vector3:
            return *(cgVector3*)mData;
        case Type_vector4:
            return (cgVector3&)(*(cgVector4*)mData);
        case Type_double:
            return cgVector3((cgFloat)(*(cgDouble*)mData), 0, 0 );
        case Type_string:
        {
            // Tokenize the string into separate components.
            cgStringArray aTokens;
            if ( !cgStringUtility::tokenize( *(cgString*)mData, aTokens, _T(",") ) )
                return cgVector3(0,0,0);

            // Convert components.
            cgVector3 vOut(0,0,0);
            size_t nComponents = std::min<size_t>( aTokens.size(), 3 );
            for ( size_t i = 0; i < nComponents; ++i )
                cgStringParser( aTokens[i] ) >> vOut[i];
            return vOut;
        
        } // End Type_string
        default:
            return cgVector3(0,0,0);

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : operator cgVector4()
/// <summary>
/// Cast operator to cast to a vector4.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant::operator cgVector4() const
{
    // What are we currently storing?
    switch ( mStorageType )
    {
        case Type_int8:
            return cgVector4((cgFloat)(*(cgInt8*)mData), 0, 0, 0 );
        case Type_uint8:
            return cgVector4((cgFloat)(*(cgUInt8*)mData), 0, 0, 0 );
        case Type_bool:
            return cgVector4((cgFloat)(*(bool*)mData), 0, 0, 0 );
        case Type_int32:
            return cgVector4((cgFloat)(*(cgInt32*)mData), 0, 0, 0 );
        case Type_uint32:
            return cgVector4((cgFloat)(*(cgUInt32*)mData), 0, 0, 0 );
        case Type_int64:
            return cgVector4((cgFloat)(*(cgInt64*)mData), 0, 0, 0 );
        case Type_uint64:
            return cgVector4((cgFloat)(*(cgUInt64*)mData), 0, 0, 0 );
        case Type_float:
            return cgVector4(*(cgFloat*)mData, 0, 0, 0 );
        case Type_vector2:
            return cgVector4( (*(cgVector2*)mData).x, (*(cgVector2*)mData).y, 0, 0 );
        case Type_vector3:
            return cgVector4( (*(cgVector3*)mData).x, (*(cgVector3*)mData).y, (*(cgVector3*)mData).z, 0 );
        case Type_vector4:
            return *(cgVector4*)mData;
        case Type_double:
            return cgVector4((cgFloat)(*(cgDouble*)mData), 0, 0, 0 );
        case Type_string:
        {
            // Tokenize the string into separate components.
            cgStringArray aTokens;
            if ( !cgStringUtility::tokenize( *(cgString*)mData, aTokens, _T(",") ) )
                return cgVector4(0,0,0,0);

            // Convert components.
            cgVector4 vOut(0,0,0,0);
            size_t nComponents = std::min<size_t>( aTokens.size(), 4 );
            for ( size_t i = 0; i < nComponents; ++i )
                cgStringParser( aTokens[i] ) >> vOut[i];
            return vOut;
        
        } // End Type_string
        default:
            return cgVector4(0,0,0,0);

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
            strParser << (cgInt32)(*(cgInt8*)mData);
            return strParser.str();
        case Type_uint8:
            strParser << (cgUInt32)(*(cgUInt8*)mData);
            return strParser.str();
        case Type_bool:
            strParser << (cgInt32)(*(bool*)mData);
            return strParser.str();
        case Type_int32:
            strParser << (*(cgInt32*)mData);
            return strParser.str();
        case Type_uint32:
            strParser << (*(cgUInt32*)mData);
            return strParser.str();
        case Type_int64:
            strParser << (*(cgInt64*)mData);
            return strParser.str();
        case Type_uint64:
            strParser << (*(cgUInt64*)mData);
            return strParser.str();
        case Type_float:
            strParser << (*(cgFloat*)mData);
            return strParser.str();
        case Type_vector2:
            strParser << (*(cgVector2*)mData).x << _T(",") << (*(cgVector2*)mData).y;
            return strParser.str();
        case Type_vector3:
            strParser << (*(cgVector3*)mData).x << _T(",") << (*(cgVector3*)mData).y << _T(",") << (*(cgVector3*)mData).z;
            return strParser.str();
        case Type_vector4:
            strParser << (*(cgVector4*)mData).x << _T(",") << (*(cgVector4*)mData).y << _T(",") << (*(cgVector4*)mData).z << _T(",") << (*(cgVector4*)mData).w;
            return strParser.str();
        case Type_double:
            strParser << (*(cgDouble*)mData);
            return strParser.str();
        case Type_string:
            return *(cgString*)mData;
        default:
            #if defined( UNICODE )
                return L"";
            #else
                return "";
            #endif

    } // End switch storage type
}

//-----------------------------------------------------------------------------
//  Name : releaseData () (Private)
/// <summary>
/// Release internal storage allocated for this variant.
/// </summary>
//-----------------------------------------------------------------------------
void cgVariant::releaseData( )
{
     // Compare data
    switch ( mStorageType )
    {
        case Type_bool:
            delete (bool*)mData;
            break;
        case Type_int8:
            delete (cgInt8*)mData;
            break;
        case Type_uint8:
            delete (cgUInt8*)mData;
            break;
        case Type_int32:
            delete (cgInt32*)mData;
            break;
        case Type_uint32:
            delete (cgUInt32*)mData;
            break;
		case Type_int64:
            delete (cgInt64*)mData;
            break;
        case Type_uint64:
            delete (cgUInt64*)mData;
            break;
        case Type_float:
            delete (cgFloat*)mData;
            break;
        case Type_vector2:
            delete (cgVector2*)mData;
            break;
        case Type_vector3:
            delete (cgVector3*)mData;
            break;
        case Type_vector4:
            delete (cgVector4*)mData;
            break;
        case Type_double:
            delete (cgDouble*)mData;
            break;
		case Type_string:
            delete (cgString*)mData;
            break;

    } // End switch storage type
    mData = CG_NULL;
    mStorageType = Type_none;
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
        case Type_vector2:
            *this = (cgVector2)*this;
            break;
        case Type_vector3:
            *this = (cgVector3)*this;
            break;
        case Type_vector4:
            *this = (cgVector4)*this;
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
        case Type_vector2:
            return sizeof(cgVector2);
            break;
        case Type_vector3:
            return sizeof(cgVector3);
            break;
        case Type_vector4:
            return sizeof(cgVector4);
            break;
        case Type_double:
            return sizeof(cgDouble);
            break;
        case Type_string:
			return (cgUInt32)((((cgString*)mData)->size() + 1) * sizeof(cgTChar)); // INCLUDE TERMINATOR!
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
    if ( mStorageType == Type_string )
        return ((cgString*)mData)->c_str();
    else
        return mData;
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
        _T("uint32"), _T("int64"), _T("uint64"), _T("float"), _T("Vector2"), 
        _T("Vector3"), _T("Vector4"), _T("double"), _T("String")
    };
    return TypeNames[ (cgInt)mStorageType ];
}

//-----------------------------------------------------------------------------
//  Name : operator ==
/// <summary>
/// Comparison operator. Storage types must match.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVariant::operator == ( const cgVariant& rhs ) const
{
	// Compare types
	if ( mStorageType != rhs.mStorageType )
        return false;

    // Empty ?
    if ( mStorageType == Type_none )
        return true;

    // Compare data
    switch ( mStorageType )
    {
        case Type_bool:
            return *(bool*)mData == *(bool*)rhs.mData;

        case Type_int8:
        case Type_uint8:
            return *(cgUInt8*)mData == *(cgUInt8*)rhs.mData;

        case Type_int32:
        case Type_uint32:
        case Type_float:
            return *(cgUInt32*)mData == *(cgUInt32*)rhs.mData;

		case Type_int64:
        case Type_uint64:
        case Type_double:
            return *(cgUInt64*)mData == *(cgUInt64*)rhs.mData;

        case Type_vector2:
            return *(cgVector2*)mData == *(cgVector2*)rhs.mData;

        case Type_vector3:
            return *(cgVector3*)mData == *(cgVector3*)rhs.mData;

        case Type_vector4:
            return *(cgVector4*)mData == *(cgVector4*)rhs.mData;

		case Type_string:
            return *(cgString*)mData == *(cgString*)rhs.mData;

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