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
// Name : cgPropertyContainer.h                                              //
//                                                                           //
// Desc : Class responsible for storing named properties, whose value is of  //
//        a variant type. Provides simple management and access to lists of  //
//        such properties.                                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPROPERTYCONTAINER_H_ )
#define _CGE_CGPROPERTYCONTAINER_H_

//-----------------------------------------------------------------------------
// cgPropertyContainer Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <System/cgVariant.h>
#include <map>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgXMLNode;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPropertyContainer (Class)
/// <summary>
/// Provides simple management and access to lists of variant typed 
/// properties.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPropertyContainer
{
public:
	//-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgPropertyContainer( );

	//-------------------------------------------------------------------------
	// Public Typedefs, Structures and Enumerations
	//-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgString, cgVariant, Collection)

    //-------------------------------------------------------------------------
	// Public Functions
	//-------------------------------------------------------------------------
    bool                isPropertyDefined   ( const cgString & propertyName ) const;
    cgVariant         & getProperty         ( const cgString & propertyName );
    const cgVariant   & getProperty         ( const cgString & propertyName ) const;
    cgVariant           getProperty         ( const cgString & propertyName, const cgVariant & defaultValue ) const;
	bool                getProperty         ( const cgString & propertyName, cgFloat * vectorValue, cgUInt32 vectorSize );
	void                setProperty         ( const cgString & propertyName, const cgVariant & value );
    void                removeProperty      ( const cgString & propertyName );
    void                clear               ( );
    void                parseKeyValuePairs  ( const cgTChar * data, cgTChar delimeter =_T('\n') );
    void                parseKeyValuePairs  ( cgString & data, cgTChar delimeter =_T('\n') );
    void                parseXML            ( const cgXMLNode & data );
	Collection        & getProperties       ( ) { return mProperties; }
	const Collection  & getProperties       ( ) const { return mProperties; }

    //-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    bool                operator ==         ( const cgPropertyContainer & rhs ) const;
	bool                operator !=         ( const cgPropertyContainer & rhs ) const;

private:
    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    Collection  mProperties;
};

#endif // !_CGE_CGPROPERTYCONTAINER_H_