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
// Name : cgPropertyContainer.cpp                                            //
//                                                                           //
// Desc : Class responsible for storing named properties, whose value is of  //
//        a variant type. Provides simple management and access to lists of  //
//        such properties.                                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPropertyContainer Module Includes
//-----------------------------------------------------------------------------
#include <System/cgPropertyContainer.h>
#include <System/cgStringUtility.h>
#include <System/cgAppLog.h>
#include <System/cgXML.h>
#include <algorithm>

//-----------------------------------------------------------------------------
//  Name : cgPropertyContainer () (Constructor)
/// <summary>
/// cgPropertyContainer class constructor
/// </summary>
//-----------------------------------------------------------------------------
cgPropertyContainer::cgPropertyContainer()
{
    // Nothing at this time.
}

//-----------------------------------------------------------------------------
//  Name : cgPropertyContainer () (Constructor)
/// <summary>
/// cgPropertyContainer class constructor
/// </summary>
//-----------------------------------------------------------------------------
cgPropertyContainer::cgPropertyContainer( const cgPropertyContainer & init )
{
    mProperties = init.mProperties;
}

//-----------------------------------------------------------------------------
//  Name : ~cgPropertyContainer () (Destructor)
/// <summary>
/// cgPropertyContainer class destructor
/// </summary>
//-----------------------------------------------------------------------------
cgPropertyContainer::~cgPropertyContainer()
{
    dispose(false);
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgPropertyContainer::dispose( bool bDisposeBase )
{
    // Clear out properties
    clear();
}

//-----------------------------------------------------------------------------
//  Name : clear ()
/// <summary>
/// Simply clear the existing contents of the property container.
/// </summary>
//-----------------------------------------------------------------------------
void cgPropertyContainer::clear( )
{
    mProperties.clear();
}

//-----------------------------------------------------------------------------
//  Name : isPropertyDefined ()
/// <summary>
/// This method simply determines if a property is defined or not.
/// Note : You should ideally call this method to check if a property exists
/// before you attempt to retrieve it with a call to 'getProperty'. If
/// you do not, and then property did not exist, getProperty will
/// automatically create an empty property with that name.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPropertyContainer::isPropertyDefined( const cgString &strName ) const
{
    return mProperties.find( strName ) != mProperties.end();
}

//-----------------------------------------------------------------------------
//  Name : getProperty ()
/// <summary>
/// Retrieve the variant property value with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgPropertyContainer::getProperty( const cgString &strName )
{
    return mProperties[ strName ];
}

//-----------------------------------------------------------------------------
//  Name : getProperty () {const version}
/// <summary>
/// Retrieve the variant property value with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
const cgVariant & cgPropertyContainer::getProperty( const cgString & strName ) const
{
	Collection::const_iterator i = mProperties.find( strName );
	return i->second;
}

//-----------------------------------------------------------------------------
//  Name : getProperty ()
/// <summary>
/// Retrieve the variant property value with the specified name. This
/// overload allows a default value to be specified if the property
/// does not exist.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant cgPropertyContainer::getProperty( const cgString &strName, const cgVariant& DefaultValue ) const
{
    Collection::const_iterator itProperty;
    
    // Find in the collection
    itProperty = mProperties.find( strName );

    // If not available, simply return the default value
    if ( itProperty == mProperties.end() )
        return DefaultValue;

    // Otherwise, return the property
    return itProperty->second;
}

//-----------------------------------------------------------------------------
//  Name : removeProperty ()
/// <summary>
/// Remove the property with the specified name and free up memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgPropertyContainer::removeProperty( const cgString & propertyName )
{
    mProperties.erase( propertyName );
}

//-----------------------------------------------------------------------------
//  Name : setProperty ()
/// <summary>
/// Set the variant property value with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
void cgPropertyContainer::setProperty( const cgString &strName, const cgVariant &Value )
{
    mProperties[strName] = Value;
}

//-----------------------------------------------------------------------------
//  Name : parseKeyValuePairs ()
/// <summary>
/// Initialize the property container based on the items included in a
/// key/value pair string
/// </summary>
//-----------------------------------------------------------------------------
void cgPropertyContainer::parseKeyValuePairs( const cgTChar * strData, cgTChar Delim /*=_T('\n')*/ )
{
    parseKeyValuePairs( cgString( strData ), Delim );
}

//-----------------------------------------------------------------------------
//  Name : CarriageFilter () (Private)
/// <summary>
/// Predicate method for stripping carriage returns.
/// </summary>
//-----------------------------------------------------------------------------
bool CarriageFilter( cgString::value_type v )
{
    return (v == _T('\r'));
}

//-----------------------------------------------------------------------------
//  Name : parseKeyValuePairs ()
/// <summary>
/// Initialize the property container based on the items included in a
/// key/value pair string
/// </summary>
//-----------------------------------------------------------------------------
void cgPropertyContainer::parseKeyValuePairs( cgString & strData, cgTChar Delim /*=_T('\n')*/ )
{
    cgString        strKeyValue, strName, strValue;
    cgStringParser  strDataBuffer;

    // Initialize string stream with incoming data
    strDataBuffer.str( strData );

    // Get each line in the stream
    for ( ; std::getline( strDataBuffer, strKeyValue, Delim ); )
    {   
        // Replace all carriage returns
        strKeyValue.erase( std::remove_if( strKeyValue.begin(), strKeyValue.end(), CarriageFilter ) );

        // Split the line around the delimeter (=)
        cgString::size_type nDelim = strKeyValue.find( _T("=") );

        // Found an equals?
		if ( nDelim != cgString::npos )
        {
            // Get name / value
            strName  = strKeyValue.substr( 0, nDelim );
            strValue = strKeyValue.substr( nDelim + 1 );

            // Trim the name
            cgString::size_type pos = strName.find_last_not_of(_T(' '));
            if ( pos != cgString::npos )
            {
                strName.erase(pos + 1);
                pos = strName.find_first_not_of(_T(' '));
                if(pos != cgString::npos) strName.erase(0, pos);
            }
            else
                strName.erase(strName.begin(), strName.end());

            // Add to property container
            setProperty( strName, strValue );

        } // End if found equals

    } // Next Line
}

//-----------------------------------------------------------------------------
//  Name : parseXML ()
/// <summary>
/// Initialize the property container based on the items included in the
/// specified XML hierarchy.
/// </summary>
//-----------------------------------------------------------------------------
void cgPropertyContainer::parseXML( const cgXMLNode & xData )
{
    cgXMLNode               xProperty, xChild;
    cgString                strName, strType;
    cgVariant               Value;
    cgVariant::VariantType  Type;

    // Find all child property nodes
    for ( cgUInt32 i = 0; ; )
    {
        // Retrieve the next child property node
        xProperty = xData.getNextChildNode( _T("Property"), i );
        if ( xProperty.isEmpty() == true )
            break;

        // Retrieve the name
        if ( xProperty.getAttributeText( _T("name"), strName ) == false )
            continue;

        // Retrieve the type (assumed to be string if not provided)
        strType = _T("string");
        Type    = cgVariant::Type_string;
        xProperty.getAttributeText( _T("type"), strType );

        // Select the correct type
        strType.toLower();
        if ( strType == _T("string") )
            Type = cgVariant::Type_string;
        else if ( strType == _T("int") || strType == _T("integer") || strType == _T("int32") )
            Type = cgVariant::Type_int32;
        else if ( strType == _T("uint") || strType == _T("uint32") )
            Type = cgVariant::Type_uint32;
        else if ( strType == _T("bool") || strType == _T("boolean") )
            Type = cgVariant::Type_bool;
        else if ( strType == _T("float") || strType == _T("single") )
            Type = cgVariant::Type_float;
        else if ( strType == _T("vector2") )
            Type = cgVariant::Type_vector2;
        else if ( strType == _T("vector3") )
            Type = cgVariant::Type_vector3;
        else if ( strType == _T("vector4") )
            Type = cgVariant::Type_vector4;
        else if ( strType == _T("double") )
            Type = cgVariant::Type_double;
        else if ( strType == _T("int8") )
            Type = cgVariant::Type_int8;
        else if ( strType == _T("uint8") )
            Type = cgVariant::Type_uint8;
        else if ( strType == _T("int64") )
            Type = cgVariant::Type_int64;
        else if ( strType == _T("uint64") )
            Type = cgVariant::Type_uint64;
        else
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Property '%s' contained an unrecognized 'type' attribute of '%s' when parsing property container data. Defaulting to 'string'.\n"), strName.c_str(), strType.c_str() );

        // Retrieve the value
        Value = cgVariant( xProperty.getText() );

        // Convert to the requested type for efficient storage and access
        if ( Type != cgVariant::Type_string )
            Value.convert( Type );

        // Store it
        setProperty( strName, Value );

    } // Next child property
}

//-----------------------------------------------------------------------------
//  Name : operator == 
/// <summary>
/// Comparison operator
/// </summary>
//-----------------------------------------------------------------------------
bool cgPropertyContainer::operator == ( const cgPropertyContainer &rhs ) const
{
	// Fetch the input collection
	const Collection& coll = rhs.getProperties( );

	// If both are empty, they match
	if ( mProperties.empty( ) && coll.empty( ) ) return true;

	// If data sizes are different, no match
	if ( mProperties.size( ) != coll.size( ) ) return false;

	// For each item in our collection...
	Collection::const_iterator it;
	for ( it = mProperties.begin(); it != mProperties.end( ); ++it ) 
	{
		// If this property is not defined in the other collection, no match
		if ( !rhs.isPropertyDefined( it->first ) ) return false;

		// If this property does not have the same value in the other collection, no match
		if ( rhs.getProperty( it->first ) != it->second ) return false;
	}

	// Match found
	return true;
}

//-----------------------------------------------------------------------------
//  Name : operator != 
/// <summary>
/// Comparison operator
/// </summary>
//-----------------------------------------------------------------------------
bool cgPropertyContainer::operator != ( const cgPropertyContainer& rhs ) const
{
	return !( *this == rhs );
}

//-----------------------------------------------------------------------------
//  Name : operator = 
/// <summary>
/// Assignment operator
/// </summary>
//-----------------------------------------------------------------------------
cgPropertyContainer & cgPropertyContainer::operator= ( const cgPropertyContainer& rhs )
{
	mProperties = rhs.mProperties;
    return *this;
}
