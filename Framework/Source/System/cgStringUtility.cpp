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
// Name : cgStringUtility.cpp                                                //
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

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgStringUtility Module Includes
//-----------------------------------------------------------------------------
#include <System/cgStringUtility.h>
#include <Math/cgMathTypes.h>
#include <tchar.h>

//-----------------------------------------------------------------------------
// cgString Static Constant Definitions
//-----------------------------------------------------------------------------
const cgString cgString::Empty = _T("");

//-----------------------------------------------------------------------------
// Module Local Classes
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
// cgStringUtility Namespace Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : tokenize ()
/// <summary>
/// Tokenize or split the specified string into an array of component
/// parts / tokens based on the provided delimters.
/// </summary>
//------------------------------------------------------------------------------
bool cgStringUtility::tokenize( const cgString & strValue, cgStringArray & Tokens, const cgString & strDelimiters )
{
    cgString::size_type lastPos = 0, pos = 0;  
    cgString::size_type i, count = 0;

    // Be polite and clear output token list
    Tokens.clear();

    // Validate
    if ( strValue.length() < 1 )
        return false;

    // Skip any delimiters at the beginning.
    lastPos = strValue.find_first_not_of( strDelimiters, 0 );

    // Check for (and insert) empty fields between delimeters
    if ( strValue.substr( 0, lastPos-pos ).length() > 0 )
    {
        count = strValue.substr( 0, lastPos-pos ).length();  	
        for( i = 0; i < count; ++i )  	
            Tokens.push_back(cgString::Empty);

        if( cgString::npos == lastPos )
            Tokens.push_back(cgString::Empty);
    
    } // End if empty fields

    // Find first "non-delimiter".
    pos = strValue.find_first_of( strDelimiters, lastPos );
    
    // Extract tokens.
    while ( cgString::npos != pos || cgString::npos != lastPos )
    {
        // Found a token, add it to the vector.
        Tokens.push_back( cgString(strValue.substr( lastPos, pos - lastPos )).trim() );

        // Skip delimiters.  Note the "not_of"
        lastPos = strValue.find_first_not_of( strDelimiters, pos );   	   	    

        // Empty fields found?
        if ( (cgString::npos != pos) && (strValue.substr( pos, lastPos-pos ).length() > 1) )  		
        {
            count = strValue.substr( pos, lastPos-pos ).length();
            for( i = 0; i < count; ++i )
                Tokens.push_back(cgString::Empty);
        
        } // End if empty fields

        // Find next token (if any)
        pos = strValue.find_first_of( strDelimiters, lastPos );
    
    } // Next Iteration

    // Return discovered contents.
    return (Tokens.empty() == false);

}

//-----------------------------------------------------------------------------
//  Name : toString ()
/// <summary>
/// Convert the specified UID to string format.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStringUtility::toString( const cgUID & value, const cgString & Format )
{
    cgString Layout = cgString::toUpper( Format );
    if ( Layout == _T("B") || true ) /*ToDo: Add more formats in line with .NET*/
    {
        cgString strUID = cgString::format( _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"), 
                                            value.data1, value.data2, value.data3, value.data4[0], value.data4[1],
                                            value.data4[2], value.data4[3], value.data4[4], value.data4[5], value.data4[6], value.data4[7] );
        return strUID.toUpper();

    } // End if 'B'
}

//-----------------------------------------------------------------------------
//  Name : tryParse ()
/// <summary>
/// Attempt to parse the specified string into a cgPoint
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strIn, cgPoint & ptOut )
{
    cgStringArray aTokens;
    if ( tokenize( strIn, aTokens, _T(",") ) == false || aTokens.size() != 2 )
        return false;
    cgStringParser( aTokens[0] ) >> ptOut.x;
    cgStringParser( aTokens[1] ) >> ptOut.y;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : tryParse ()
/// <summary>
/// Attempt to parse the specified string into a cgPointF
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strIn, cgPointF & ptOut )
{
    cgStringArray aTokens;
    if ( tokenize( strIn, aTokens, _T(",") ) == false || aTokens.size() != 2 )
        return false;
    cgStringParser( aTokens[0] ) >> ptOut.x;
    cgStringParser( aTokens[1] ) >> ptOut.y;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : tryParse ()
/// <summary>
/// Attempt to parse the specified string into a cgSize
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strIn, cgSize & sizeOut )
{
    cgStringArray aTokens;
    if ( tokenize( strIn, aTokens, _T(",") ) == false || aTokens.size() != 2 )
        return false;
    cgStringParser( aTokens[0] ) >> sizeOut.width;
    cgStringParser( aTokens[1] ) >> sizeOut.height;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : tryParse ()
/// <summary>
/// Attempt to parse the specified string into a cgRect (LTRB)
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strIn, cgRect & rcOut )
{
    cgStringArray aTokens;
    if ( tokenize( strIn, aTokens, _T(",") ) == false || aTokens.size() != 4 )
        return false;
    cgStringParser( aTokens[0] ) >> rcOut.left;
    cgStringParser( aTokens[1] ) >> rcOut.top;
    cgStringParser( aTokens[2] ) >> rcOut.right;
    cgStringParser( aTokens[3] ) >> rcOut.bottom;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : tryParse ()
/// <summary>
/// Attempt to parse the specified string into a cgColorValue (RGBA)
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strIn, cgColorValue & colorOut )
{
    cgStringArray aTokens;
    if ( tokenize( strIn, aTokens, _T(",") ) == false || aTokens.size() != 4 )
        return false;
    cgStringParser( aTokens[0] ) >> colorOut.r;
    cgStringParser( aTokens[1] ) >> colorOut.g;
    cgStringParser( aTokens[2] ) >> colorOut.b;
    cgStringParser( aTokens[3] ) >> colorOut.a;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : tryParse ()
/// <summary>
/// Attempt to parse the specified string into a cgVector2
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strIn, cgVector2 & vecOut )
{
    cgStringArray aTokens;
    if ( tokenize( strIn, aTokens, _T(",") ) == false || aTokens.size() != 2 )
        return false;
    cgStringParser( aTokens[0] ) >> vecOut.x;
    cgStringParser( aTokens[1] ) >> vecOut.y;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : tryParse ()
/// <summary>
/// Attempt to parse the specified string into a cgVector3
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strIn, cgVector3 & vecOut )
{
    cgStringArray aTokens;
    if ( tokenize( strIn, aTokens, _T(",") ) == false || aTokens.size() != 3 )
        return false;
    cgStringParser( aTokens[0] ) >> vecOut.x;
    cgStringParser( aTokens[1] ) >> vecOut.y;
    cgStringParser( aTokens[2] ) >> vecOut.z;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : tryParse ()
/// <summary>
/// Attempt to parse the specified string into a cgVector4
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strIn, cgVector4 & vecOut )
{
    cgStringArray aTokens;
    if ( tokenize( strIn, aTokens, _T(",") ) == false || aTokens.size() != 4 )
        return false;
    cgStringParser( aTokens[0] ) >> vecOut.x;
    cgStringParser( aTokens[1] ) >> vecOut.y;
    cgStringParser( aTokens[2] ) >> vecOut.z;
    cgStringParser( aTokens[3] ) >> vecOut.w;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseBool ()
/// <summary>
/// Parse boolean text and interpret "true" / "false" strings as well as
/// numeric values.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::parseBool( const cgString & strInput )
{
    bool bResult = false;
    if ( strInput.compare( _T("true"), true ) == 0 )
        return true;
    else if ( strInput.compare( _T("false"), true ) == 0 )
        return false;
    cgStringParser( strInput ) >> bResult;
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : tryParse()
/// <summary>
/// Parse cgUID as string value and return as struct.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::tryParse( const cgString & strInput, cgUID & uidOut )
{
    // Sample GUIDs (with or without braces {})
    // D74D0965-6B4C-4EEC-9752-A6EEEF168767
    // 0xD74D0965-0x6B4C-0x4EEC-0x9752-0xA6EEEF168767
    // D74D09656B4C4EEC9752A6EEEF168767
    cgUInt64 nData;
    
    // Clear return UID
    memset( &uidOut, 0, sizeof(cgUID) );

    // If no cgUID string was passed simply return
    if ( strInput.empty() == true )
        return false;

    // First of all remove any outer braces.
    cgString strGUID = cgString::trim( strInput );
    if ( strGUID.beginsWith( _T("{") ) == true )
        strGUID = strGUID.substr( 1 );
    if ( strGUID.endsWith( _T("}") ) == true )
        strGUID = strGUID.substr( 0, strGUID.length() - 1 );

    // Attempt to tokenize for cgUID strings that use hypen separated format.
    cgStringArray aTokens;
    if ( tokenize( strGUID, aTokens, _T("-") ) == false )
        return false;
    
    // Hyphen separated or standard?
    if ( aTokens.size() == 5 )
    {
        cgStringParser( aTokens[0] ) >> std::hex >> uidOut.data1;
        cgStringParser( aTokens[1] ) >> std::hex >> uidOut.data2;
        cgStringParser( aTokens[2] ) >> std::hex >> uidOut.data3;
        cgStringParser( aTokens[3] ) >> std::hex >> nData;
        uidOut.data4[0] = (cgByte)((nData & 0xFF00) >> 8);
        uidOut.data4[1] = (cgByte)(nData & 0x00FF);
        cgStringParser( aTokens[4] ) >> std::hex >> nData;
        uidOut.data4[2] = (cgByte)((nData & 0xFF0000000000) >> 40);
        uidOut.data4[3] = (cgByte)((nData & 0x00FF00000000) >> 32);
        uidOut.data4[4] = (cgByte)((nData & 0x0000FF000000) >> 24);
        uidOut.data4[5] = (cgByte)((nData & 0x000000FF0000) >> 16);
        uidOut.data4[6] = (cgByte)((nData & 0x00000000FF00) >> 8);
        uidOut.data4[7] = (cgByte)(nData & 0x0000000000FF);

    } // End if separated
    else if ( aTokens.size() == 1 )
    {
        // Could just be a standard 128 bit hexadecimal number.
        // ToDo:
        return false;

    } // End if not separated
   
    // Success!
	return true;
}

//-----------------------------------------------------------------------------
//  Name : getPrivateProfileIntEx () (Private)
/// <summary>
/// Enhanced version of GetPrivateProfileInt which supports hex values.
/// </summary>
//-----------------------------------------------------------------------------
int cgStringUtility::getPrivateProfileIntEx( const cgTChar * lpAppName, const cgTChar * lpKeyName, cgInt nDefault, const cgTChar * lpFileName )
{
    cgString strValue, strDefault;
    cgTChar  strBuffer[128];
    int      nValue;

    // Convert default to string
    strDefault = cgString::format( _T("%i"), nDefault );

    // Retrieve the value as a string
    GetPrivateProfileString( lpAppName, lpKeyName, strDefault.c_str(), strBuffer, 128, lpFileName );

    // ToDo: Replace stscanf with string parser and std::hex

    // Convert to a value, correctly handling the hex '0x' specifier case
    strValue = cgString::trim(strBuffer);
    if ( strValue.length() >= 3 && (strValue.substr( 0, 2 )== _T("0x") || strValue.substr( 0, 2 ) == _T("0X")) )
        _stscanf( strValue.c_str(), _T("0x%x"), &nValue );
    else
        _stscanf( strValue.c_str(), _T("%i"), &nValue );

    // Return the value
    return nValue;
}

//-----------------------------------------------------------------------------
//  Name : getPrivateProfileFloat () (Private)
/// <summary>
/// Retrieve a floating point value from an ini file.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgStringUtility::getPrivateProfileFloat( const cgTChar * lpAppName, const cgTChar * lpKeyName, cgFloat fDefault, const cgTChar * lpFileName )
{
    cgString strDefault;
    cgTChar  strBuffer[128];
    cgFloat  fValue;

    // Convert default to string
    strDefault = cgString::format( _T("%f"), fDefault );

    // Retrieve the value as a string
    GetPrivateProfileString( lpAppName, lpKeyName, strDefault.c_str(), strBuffer, 128, lpFileName );

    // Convert to a floating point value
    cgStringParser( strBuffer ) >> fValue;
    
    // Return the value
    return fValue;
}

//-----------------------------------------------------------------------------
//  Name : writePrivateProfileFloat () (Private)
/// <summary>
/// Allows us to write floating point profile strings.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::writePrivateProfileFloat( const cgTChar * lpAppName, const cgTChar * lpKeyName, cgFloat fValue, const cgTChar * lpFileName )
{
    cgString strValue;

    // Convert value to string
    strValue = cgString::format( _T("%g"), fValue );
    
    // Write the string
    return (WritePrivateProfileString( lpAppName, lpKeyName, strValue.c_str(), lpFileName ) == TRUE);
}

//-----------------------------------------------------------------------------
//  Name : writePrivateProfileIntEx () (Private)
/// <summary>
/// Allows us to write integer profile strings.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::writePrivateProfileIntEx( const cgTChar * lpAppName, const cgTChar * lpKeyName, cgInt nValue, const cgTChar * lpFileName, bool bHexadecimal /* = false */ )
{
    cgString strValue;

    // Convert value to string
    if ( bHexadecimal == false )
        strValue = cgString::format( _T("%i"), nValue );
    else
        strValue = cgString::format( _T("0x%x"), nValue );
    
    // Write the string
    return (WritePrivateProfileString( lpAppName, lpKeyName, strValue.c_str(), lpFileName ) == TRUE);
}

//-----------------------------------------------------------------------------
//  Name : getClipboardText ()
/// <summary>
/// Retrieve any text currently stored in the clipboard.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStringUtility::getClipboardText()
{
    HGLOBAL  hMem;
    cgString strClipText;

    // Open the clipboard
    if ( OpenClipboard(CG_NULL) == FALSE )
        return _T("");

    // Retrieve any text data from clipboard
    #if defined( UNICODE )
        hMem = GetClipboardData(CF_UNICODETEXT);
    #else // UNICODE
        hMem = GetClipboardData(CF_TEXT);
    #endif // !UNICODE

    if ( hMem != CG_NULL )
    {
        // Extract the string
        strClipText = (const cgTChar *)GlobalLock(hMem);
        GlobalUnlock( hMem );

    } // End if text available

    // Close the clipboard
    CloseClipboard();

    // Return the extracted text (if any)
    return strClipText;
}

//-----------------------------------------------------------------------------
//  Name : setClipboardText ()
/// <summary>
/// Clear the clipboard and insert the specified text as the current
/// entry.
/// </summary>
//-----------------------------------------------------------------------------
void cgStringUtility::setClipboardText( const cgString & strText )
{
    HGLOBAL  hMem;
    LPTSTR   pClipText = CG_NULL;

    // Open the clipboard
    if ( OpenClipboard(CG_NULL) == FALSE )
        return;

    // Allocate the global memory required to store the text (+terminator)
    hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, (strText.length() + 1) * sizeof(cgTChar) );

    // Lock the memory and copy data in
    pClipText = (LPTSTR)GlobalLock( hMem );
    lstrcpy( pClipText, strText.c_str() );
    GlobalUnlock(hMem);

    // Set the text data to the clipboard
    #if defined( UNICODE )
        SetClipboardData( CF_UNICODETEXT, hMem );
    #else // UNICODE
        SetClipboardData( CF_TEXT, hMem );
    #endif // !UNICODE

    // Close the clipboard
    CloseClipboard();
}

//-----------------------------------------------------------------------------
//  Name : fromStringTable ()
/// <summary>
/// Retrieve a string from the string table resource.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStringUtility::fromStringTable( void * pModuleInstance, cgUInt32 stringId )
{
    cgTChar  *pwchMem, *pwchCur;
    cgUInt32  idRsrcBlk = stringId / 16 + 1;
    int       strIndex  = stringId % 16;
    HINSTANCE hModule = (HINSTANCE)pModuleInstance;
    HRSRC     hResource = CG_NULL;

    hResource = FindResourceEx( hModule, RT_STRING, MAKEINTRESOURCE(idRsrcBlk), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) );

    if( hResource != CG_NULL )
    {
        pwchMem = (cgTChar*)LoadResource( hModule, hResource );

        if( pwchMem != CG_NULL )
        {
            pwchCur = pwchMem;
            for(int i = 0; i<16; i++ )
            {
                if( *pwchCur )
                {
                    int cchString = *pwchCur;  // String size in characters.
                    pwchCur++;
                    if( i == strIndex )
                    {
                        // The string has been found in the string table.
                        //cgTChar *pwchTemp = new cgTChar[ cchString ];
                        //wcsncpy( pwchTemp, pwchCur, cchString );
                        //return pwchTemp;
                        return cgString( pwchCur, cchString );
                        
                    }
                    pwchCur += cchString;
                }
                else
                    pwchCur++;
            }
        }
    }
    return _T("");
}

//-----------------------------------------------------------------------------
//  Name : isTextUnicode ()
/// <summary>
/// Determine if the specified string buffer contains unicode data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::isTextUnicode( const void* pData, cgUInt32 nBytes )
{
    // Validate requirements (buffer too small or odd in length)
    if ( nBytes < sizeof(cgWChar) || (nBytes & 1) != 0 )
        return false;

    // Check to see if the string starts with the
    // special unique two byte signatures.
    const cgWChar * lpszData = (const cgWChar*)pData;
    if ( *lpszData == 0xFFFE ) return false;
    if ( *lpszData == 0xFEFF ) return true;

    // Check a number of characters to see if any ASCII
    // characters are contained in the stream.
    cgUInt32 nASCIIChars = 0;
    cgUInt32 nCharacters = std::min<cgUInt32>( 256, nBytes / sizeof(cgWChar) );
    for ( cgUInt32 i = 0; i < nCharacters; ++i )
    {
        if ( lpszData[i] <= (cgUInt16)255 )
            ++nASCIIChars;

    } // Next Character

    // If over half were valid ASCII characters, this is UNICODE.
    if ( nASCIIChars > nCharacters / 2 )
        return true;

    // Check to see if unicode CG_NULL characters exist.
    for ( cgUInt32 i = 0; i < nCharacters; ++i )
    {
        if ( lpszData[i] == 0 )
            return true;

    } // Next Character

    // Not unicode.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : addSlashes ()
/// <summary>
/// Escape the specified string with slashes.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStringUtility::addSlashes( const cgString & Value )
{
    cgString strOut;
    strOut.reserve( Value.length() );
    for ( size_t i = 0; i < Value.length(); ++i )
    {
        cgTChar c = Value.at(i);
        switch ( c )
        {
            case 0:     // null
            case 8:     // backspace
            case 9:     // horizontal tab
            case 10:    // new line
            case 13:    // carriage return
            case 26:    // substitute
            case 34:    // double quote
            case 39:    // single quote
            case 92:    // backslash
            case 96:    // grave accent
                strOut += _T("\\");
                break;
        
        } // End if supported
        strOut += c;

    } // Next Character
    return strOut;
}

// ToDo: Remove or support
/*// Remove escape sequence from string.
static String ^ StripSlashes( String ^ value )
{
    // List of characters handled:
    // \000 null
    // \010 backspace
    // \011 horizontal tab
    // \012 new line
    // \015 carriage return
    // \032 substitute
    // \042 double quote
    // \047 single quote
    // \134 backslash
    // \140 grave accent

    //return System::Text::RegularExpressions::Regex::Replace( value, L"(\\\\)([\\000\\010\\011\\012\\015\\032\\042\\047\\134\\140])", L"$2");
}*/

//-----------------------------------------------------------------------------
//  Name : URLEncode ()
/// <summary>
/// Convert the specified string into one capable of being embedded in a URL.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStringUtility::URLEncode( const cgString & c )
{
    cgString Escaped = _T("");
    size_t nMax = c.length();
    for ( size_t i = 0; i < nMax; ++i )
    {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
             (65 <= c[i] && c[i] <= 90) ||//abc...xyz
             (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
             (c[i]==_T('~') || c[i]==_T('!') || c[i]==_T('*') || 
             c[i]==_T('(') || c[i]==_T(')') || c[i]==_T('\'') ) )
        {
            Escaped.append( &c[i], 1);
        }
        else
        {
            Escaped.append(_T("%") );
            Escaped.append( Char2HEX(c[i]) );
        }
    }
    return Escaped;
}

//-----------------------------------------------------------------------------
//  Name : Char2HEX ()
/// <summary>
/// Convert the specified character into a hexadecimal value string 
/// (ASCII only).
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStringUtility::Char2HEX( cgTChar c )
{
    cgTChar dig1 = (c&0xF0)>>4;
    cgTChar dig2 = (c&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    cgString r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}

//-----------------------------------------------------------------------------
//  Name : IsNextCharacterNewlineOrComment ()
/// <summary>
/// Used by 'beautifyCode()' for internal processing.
/// </summary>
//-----------------------------------------------------------------------------
bool IsNextCharacterNewlineOrComment( const cgString & strCode, size_t nCurrent )
{
    for ( size_t i = nCurrent + 1; i < strCode.size(); ++i )
    {
        cgTChar cCurrent = strCode.at(i);
        
        // Skip all whitespace
        switch ( cCurrent )
        {
            case _T('\t'):
            case _T(' '):
            case _T('\r'):
                break;

            case _T('/'):
                if ( i < (strCode.size() - 1)&& strCode.at(i+1) == _T('/') )
                    return true;
                break;

            case _T('\n'):
                return true;

            default:
                return false;

        } // End switch cCurrent

    } // Next character

    return false;
}

//-----------------------------------------------------------------------------
//  Name : beautifyCode ()
/// <summary>
/// Parse and 'beautify' any standard C++'esque code.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStringUtility::beautifyCode( const cgString & strCodeIn )
{
    cgString strCode;

    // Configuration
    const cgInt32 nIndentSpaces = 4;
    const cgInt32 nTabSpaces    = 4;

    // Tracking variables
    cgInt   nNewLineCount   = 1;
    bool    bInQuotes       = false;
    bool    bInBlockComment = false;
    cgInt32 nCurrentIndent  = 0;
    cgInt32 nCurrentColumn  = 0;

    // Process each character.
    cgTChar cPrevious = 0;
    for ( size_t i = 0; i < strCodeIn.size(); ++i )
    {
        cgTChar cCurrent = strCodeIn.at(i);

        // String handling?
        if ( !bInQuotes && !bInBlockComment )
        {
            switch ( cCurrent )
            {
                case _T('/'):

                    // Is this a comment?
                    if ( i < (strCodeIn.size() - 1)&& strCodeIn.at(i+1) == _T('/') )
                    {
                        // If this was a new line, indent as appropriate.
                        if ( nNewLineCount )
                        {
                            // If there was only one newline prior to this, insert another
                            // to keep things nice and separated.
                            if ( nNewLineCount == 1 )
                            {
                                strCode.append( _T("\n") );
                                nCurrentColumn = 0;
                                nNewLineCount++;
                            
                            } // End if insert a second newline before comment?

                            strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                            nCurrentColumn = nCurrentIndent * nIndentSpaces;
                        
                        } // End if new line

                        // If this was a new line, it no longer is.
                        nNewLineCount = 0;

                        // Single line comment (//), insert the whole thing up to the next newline.
                        for ( ; i < strCodeIn.size() && !nNewLineCount; ++i )
                        {
                            cCurrent = strCodeIn.at(i);
                            switch ( cCurrent )
                            {
                                case _T('\r'):
                                    continue;
                                
                                case _T('\t'):
                                {
                                    // Insert space to bring us to the next tab column multiple
                                    cgInt nSpaceCount = nTabSpaces - (nCurrentColumn % nTabSpaces);
                                    strCode.append( nSpaceCount, _T(' ') );
                                    nCurrentColumn += nSpaceCount;
                                    break;
                                
                                } // End case '\t'
                                case _T('\n'):
                                    strCode.append( 1, cCurrent );
                                    nCurrentColumn = 0;
                                    nNewLineCount=2; // Causes us to break out of the loop
                                    break;

                                default:
                                    strCode.append( 1, cCurrent );
                                    nCurrentColumn++;
                                    break;

                            } // End switch cCurrent
                        
                        } // Next character
                        --i;

                    } // End if '//'
                    else if ( i < (strCodeIn.size() - 1)&& strCodeIn.at(i+1) == _T('*') )
                    {
                        // If this was a new line, indent as appropriate.
                        if ( nNewLineCount )
                        {
                            // If there was only one newline prior to this, insert another
                            // to keep things nice and separated.
                            if ( nNewLineCount == 1 )
                            {
                                strCode.append( _T("\n") );
                                nCurrentColumn = 0;
                                nNewLineCount++;
                            
                            } // End if insert a second newline before comment?

                            strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                            nCurrentColumn = nCurrentIndent * nIndentSpaces;
                        
                        } // End if new line

                        // If this was a new line, it no longer is.
                        nNewLineCount = 0;

                        // Append the block comment open
                        strCode.append( _T("/*") );
                        i+=2;
                        nCurrentColumn += 2;

                        // Process block comment
                        bInBlockComment = true;

                    } // End if '/*'
                    else
                    {
                        // If this was a new line, indent as appropriate.
                        if ( nNewLineCount )
                        {
                            strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                            nCurrentColumn = nCurrentIndent * nIndentSpaces;
                        
                        } // End if new line

                        // Append character
                        strCode.append( 1, cCurrent );
                        nCurrentColumn++;

                        // If this was a new line, it no longer is.
                        nNewLineCount = 0;

                    } // End if '/'
                    break;

                case _T('\n'):
                    strCode.append( 1, cCurrent );
                    nCurrentColumn = 0;
                    nNewLineCount++;
                    break;

                case _T('\t'):
                {
                    // Skip whitespace if we're processing a new line.
                    if ( nNewLineCount )
                        continue;

                    // Replace tabs with spaces.
                    // Insert space to bring us to the next tab column multiple
                    cgInt nSpaceCount = nTabSpaces - (nCurrentColumn % nTabSpaces);
                    strCode.append( nSpaceCount, _T(' ') );
                    nCurrentColumn += nSpaceCount;
                    break;
                
                } // End case '\t'
                case _T(' '):
                {
                    // Skip whitespace if we're processing a new line.
                    if ( nNewLineCount )
                        continue;

                    // Insert the space
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;
                    break;
                
                } // End case ' '

                case _T('{'):
                {
                    // If this was a new line, indent as appropriate.
                    if ( nNewLineCount )
                    {
                        strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                        nCurrentColumn = nCurrentIndent * nIndentSpaces;
                    
                    } // End if new line

                    // Insert opening brace and increase new-line indent.
                    strCode.append( 1, cCurrent );
                    ++nCurrentIndent;
                    nCurrentColumn++;

                    // If this was a new line, it no longer is.
                    nNewLineCount = 0;
                    break;

                } // End case '{'

                case _T('}'):
                {
                    // Decrease indent
                    --nCurrentIndent;

                    // Paranoia test for uneven braces!
                    if ( nCurrentIndent < 0 )
                        nCurrentIndent = 0;

                    // If this was a new line, indent as appropriate.
                    if ( nNewLineCount )
                    {
                        strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                        nCurrentColumn = nCurrentIndent * nIndentSpaces;
                    
                    } // End if new line

                    // Insert closing brace.
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;

                    // If this was a new line, it no longer is.
                    nNewLineCount = 0;
                    break;

                } // End case '}'

                case _T(';'):
                {
                    // If this was a new line, indent as appropriate.
                    if ( nNewLineCount )
                    {
                        strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                        nCurrentColumn = nCurrentIndent * nIndentSpaces;
                    
                    } // End if new line

                    // Insert
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;

                    // If this was a new line, it no longer is.
                    nNewLineCount = 0;

                    // Force a new-line only if the next character is NOT a newline.
                    if ( !IsNextCharacterNewlineOrComment( strCodeIn, i ) )
                    {
                        strCode.append( _T("\n") );
                        nCurrentColumn = 0;
                        
                        // We're now processing a new line (discard whitespace)
                        nNewLineCount++;
                    
                    } // End if no upcoming newline
                    break;

                } // End case ';'
                
                case _T('"'):
                    // If this was a new line, indent as appropriate.
                    if ( nNewLineCount )
                    {
                        strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                        nCurrentColumn = nCurrentIndent * nIndentSpaces;
                    
                    } // End if new line

                    // Append
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;

                    // If this was a new line, it no longer is.
                    nNewLineCount = 0;

                    // We're now inside the quote case.
                    bInQuotes = true;
                    break;

                case _T('\r'):
                    // Ignore these characters altogether
                    break;

                default:
                    // If this was a new line, indent as appropriate.
                    if ( nNewLineCount )
                    {
                        strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                        nCurrentColumn = nCurrentIndent * nIndentSpaces;
                    
                    } // End if new line

                    // Append
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;

                    // If this was a new line, it no longer is.
                    nNewLineCount = 0;
                    break;

            } // End switch cCurrent

        } // End if ! in quotes
        else if ( bInQuotes )
        {
            switch (cCurrent)
            {
                case _T('"'):
                    // If the previous character was a back slash (\)
                    // then this is not the end of the string.
                    if ( i > 0 && strCodeIn.at(i-1) != _T('\\') )
                        bInQuotes = false;

                    // Append
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;
                    break;

                case _T('\n'):
                    // Append
                    strCode.append( 1, cCurrent );
                    nCurrentColumn = 0;
                    break;

                default:
                    // Append
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;
                    break;

            } // End switch cCurrent

        } // End if in quotes
        else if ( bInBlockComment )
        {
            switch ( cCurrent )
            {
                case _T('*'):

                    // Is this the end of the comment?
                    if ( i < (strCodeIn.size() - 1)&& strCodeIn.at(i+1) == _T('/') )
                    {
                        // We are no longer in the block comment.
                        bInBlockComment = false;

                    } // End if '*/'

                    // If this was a new line, indent as appropriate.
                    if ( nNewLineCount )
                    {
                        strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                        nCurrentColumn = nCurrentIndent * nIndentSpaces;
                    
                    } // End if new line

                    // If this was a new line, it no longer is.
                    nNewLineCount = 0;

                    // Append character
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;
                    break;

                case _T('\n'):
                    strCode.append( 1, cCurrent );
                    nCurrentColumn = 0;
                    nNewLineCount++;
                    break;

                case _T('\t'):
                {
                    // Skip whitespace if we're processing a new line.
                    if ( nNewLineCount )
                        continue;

                    // Replace tabs with spaces.
                    // Insert space to bring us to the next tab column multiple
                    cgInt nSpaceCount = nTabSpaces - (nCurrentColumn % nTabSpaces);
                    strCode.append( nSpaceCount, _T(' ') );
                    nCurrentColumn += nSpaceCount;
                    break;
                
                } // End case '\t'

                case _T(' '):
                {
                    // Skip whitespace if we're processing a new line.
                    if ( nNewLineCount )
                        continue;

                    // Insert the space
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;
                    break;
                
                } // End case ' '

                case _T('\r'):
                    // Ignore these characters altogether
                    break;

                default:
                    // If this was a new line, indent as appropriate.
                    if ( nNewLineCount )
                    {
                        strCode.append( nCurrentIndent * nIndentSpaces, _T(' ') );
                        nCurrentColumn = nCurrentIndent * nIndentSpaces;
                    
                    } // End if new line

                    // Append
                    strCode.append( 1, cCurrent );
                    nCurrentColumn++;

                    // If this was a new line, it no longer is.
                    nNewLineCount = 0;
                    break;

            } // End switch cCurrent

        } // End if in block comment
    
    } // Next character

    // Return result
    return strCode;
}

//-----------------------------------------------------------------------------
//  Name : parseCommandLine ()
/// <summary>
/// Split single string command line into its component parts.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStringUtility::parseCommandLine( const cgString & strCommandLine, cgStringArray & ArgumentsOut )
{
    // Be polite and clear output array
    ArgumentsOut.clear();

    // Parse one character at a time
    bool bInQuotes = false;
    bool bConstructingArgument = false;
    for ( size_t i = 0; i < strCommandLine.size(); ++i )
    {
        cgTChar c = strCommandLine.at( i );
        if ( bInQuotes )
        {
            if ( c == _T('\"') )
            {
                // Quotes are now closed
                bInQuotes = false;
            
            } // End if quote
            else
            {
                // Append to current argument.
                ArgumentsOut.back() += c;

            } // End if !quote

        } // End if in quotes
        else
        {
            switch ( c )
            {
                case _T('\"'):
                    // Opened quotes
                    bInQuotes = true;

                    // If we haven't started constructing an argument yet
                    // create space for one.
                    if ( !bConstructingArgument )
                        ArgumentsOut.resize( ArgumentsOut.size() + 1 );
                    bConstructingArgument = true;
                    break;

                case _T(' '):
                case _T('\t'):
                case _T('\r'):
                case _T('\n'):
                    // White space. Finish constructing current argument.
                    bConstructingArgument = false;
                    break;

                default:
                    // If we haven't started constructing an argument yet
                    // create space for one.
                    if ( !bConstructingArgument )
                        ArgumentsOut.resize( ArgumentsOut.size() + 1 );
                    bConstructingArgument = true;

                    // Append character to current argument.
                    ArgumentsOut.back() += c;
                    break;

            } // End switch c

        } // End if !in quotes

    } // Next character

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : stripTrailingNumbers ()
/// <summary>
/// Strip off any numbers that are found at the end of the string.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStringUtility::stripTrailingNumbers( const cgString & value )
{
    // Must have at least one character
    if ( value.empty() )
        return value;

    // Walk back from the end until we find a non numeric character
    size_t lastValidChar = value.size() - 1;
    while ( 1 )
    {
        if ( value[lastValidChar] < _T('0') || value[lastValidChar] > _T('9') )
            return value.substr( 0, lastValidChar + 1 );
        if ( lastValidChar == 0 )
            break;
        lastValidChar--;
    
    } // Next character

    // All numbers.
    return cgString::Empty;
}