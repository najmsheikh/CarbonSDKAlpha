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
// Name : cgVariant.h                                                        //
//                                                                           //
// Desc : Variable class that allows the storage of and conversion between   //
//        multiple variable types.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGVARIANT_H_ )
#define _CGE_CGVARIANT_H_

//-----------------------------------------------------------------------------
// cgVariant Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <string>
#include <sstream>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgVector2;
class cgVector3;
class cgVector4;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgVariant (Class)
/// <summary>
/// Storage and utilities for managing and manipulating multiple types of
/// values within one variable.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgVariant
{
public:
    //-------------------------------------------------------------------------
	// Public Structures, Typedefs and Enumerations
	//-------------------------------------------------------------------------
    enum VariantType { Type_none = 0, Type_bool, Type_int8, Type_uint8, Type_int32, 
                       Type_uint32, Type_int64, Type_uint64, Type_float, Type_vector2, 
                       Type_vector3, Type_vector4, Type_double, Type_string };

	//-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgVariant( );
    cgVariant( void * data, VariantType type );
    cgVariant( cgInt8 value );
    cgVariant( cgUInt8 value );
    cgVariant( bool value );
    cgVariant( cgInt value );
    cgVariant( cgUInt value );
    cgVariant( cgInt32 value );
    cgVariant( cgUInt32 value );
    cgVariant( cgInt64 value );
    cgVariant( cgUInt64 value );
    cgVariant( cgFloat value );
    cgVariant( const cgVector2 & value );
    cgVariant( const cgVector3 & value );
    cgVariant( const cgVector4 & value );
    cgVariant( cgDouble value );
    cgVariant( const cgString & value );
    cgVariant( const cgTChar * value );
    cgVariant( const cgVariant & value );
    ~cgVariant();

	//-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    // Cast Operators
    operator cgUInt8    ( ) const;
    operator cgInt8     ( ) const;
    operator bool       ( ) const;
    operator cgUInt     ( ) const;
    operator cgInt      ( ) const;
    operator cgUInt32   ( ) const;
    operator cgInt32    ( ) const;
    operator cgUInt64   ( ) const;
    operator cgInt64    ( ) const;
    operator cgFloat    ( ) const;
    operator cgVector2  ( ) const;
    operator cgVector3  ( ) const;
    operator cgVector4  ( ) const;
    operator cgDouble   ( ) const;
    operator cgString   ( ) const;

    // Assignment Operators
    cgVariant & operator=       ( cgUInt8 value );
    cgVariant & operator=       ( cgInt8 value );
    cgVariant & operator=       ( bool value );
    cgVariant & operator=       ( cgUInt value );
    cgVariant & operator=       ( cgInt value );
    cgVariant & operator=       ( cgUInt32 value );
    cgVariant & operator=       ( cgInt32 value );
    cgVariant & operator=       ( cgUInt64 value );
    cgVariant & operator=       ( cgInt64 value );
    cgVariant & operator=       ( cgFloat value );
    cgVariant & operator=       ( const cgVector2 & value );
    cgVariant & operator=       ( const cgVector3 & value );
    cgVariant & operator=       ( const cgVector4 & value );
    cgVariant & operator=       ( cgDouble value );
    cgVariant & operator=       ( const cgString & value );
    cgVariant & operator=       ( const cgTChar * value );
    cgVariant & operator=       ( const cgVariant & value );

    // Comparison Operators
    bool            operator ==   ( const cgVariant& rhs ) const;
	bool            operator !=   ( const cgVariant& rhs ) const;

    //-------------------------------------------------------------------------
	// Public Functions
	//-------------------------------------------------------------------------
    VariantType     getType       ( ) const { return mStorageType; }
    const cgString& getTypeName   ( ) const;
    bool            convert       ( VariantType type );
	cgUInt32        getSize       ( ) const;
    const void    * getData       ( ) const;

private:
    //-------------------------------------------------------------------------
	// Private Methods
	//-------------------------------------------------------------------------
    void            releaseData     ( );

    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    void      * mData;          // Reference to data stored in the variant
    VariantType mStorageType;   // Describes the type of data currently stored.
};

#endif // !_CGE_CGVARIANT_H_