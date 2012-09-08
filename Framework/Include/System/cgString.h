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
// Name : cgString.h                                                         //
//                                                                           //
// Desc : String class implementation, based on providing the ease of use of //
//        the speed and power of basic_string whilst allowing for the        //
//        provision of additional functionality.                             //
//                                                                           //
// Note : A special thanks to Joe O'Leary (http://www.joeo.net/stdstring.htm)//
//        for the original implementation 'CStdString' on which parts of     //
//        this class are based.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSTRING_H_ )
#define _CGE_CGSTRING_H_

//-----------------------------------------------------------------------------
// cgString Header Includes
//-----------------------------------------------------------------------------
#include <cgAPI.h>
#include <string>   // std::string / std::wstring
#include <sstream>  // std::stringstream / std::wstringstream
#include <vector>   // std::vector
#include <cctype>   // toupper / tolower
#include <stdarg.h>
#include <algorithm>
#include <locale>

//-----------------------------------------------------------------------------
// String and Character Typedefs
//-----------------------------------------------------------------------------
// Character
typedef char                    cgChar;
typedef unsigned char           cgUChar;
typedef wchar_t                 cgWChar;
#if defined(UNICODE) || defined(_UNICODE)
typedef wchar_t                 cgTChar;
#else // UNICODE
typedef char                    cgTChar;
#endif // !UNICODE

// Wide character literal prefix (_T("String"))
#if !defined(_T)
#if defined(UNICODE) || defined(_UNICODE)
    #define __T(x)  L ## x
    #define _T(x)   __T(x)
#else  // UNICODE
    #define _T(x)
#endif // !UNICODE
#endif // !_T()

// String Parser
#if defined(UNICODE) || defined(_UNICODE)
typedef std::wstringstream      cgStringParser;
#else // UNICODE
typedef std::stringstream       cgStringParser;
#endif // !UNICODE

//-----------------------------------------------------------------------------
// cgString Configuration
//-----------------------------------------------------------------------------
// Count the number of times an cgString is constructed from a specific string 
// literal (or LPCTSTR specifically). This information is then output to the
// std stream automatically on exit. Useful for identifying candidates for
// string pooling / local static const string objects.
//#define STRING_CONSTRUCT_REPORT

//-----------------------------------------------------------------------------
// cgString Conversion Routines
//-----------------------------------------------------------------------------
#define STRING_CONVERT cgInt32 _cvt = 0; _cvt; cgUInt32 _acp = 0 /*CP_ACP*/; \
                       _acp; const cgWChar * _pw=0; _pw; const cgChar * _pa=0; _pa

typedef std::codecvt<cgWChar, cgChar, mbstate_t> CodeCvtFacet;
inline cgWChar * stringCodeCvt( cgWChar * pDstW, int nDst, const cgChar * pSrcA, int nSrc,
                                const std::locale& loc = std::locale())
{
    pDstW[0] = '\0';
    if ( nSrc > 0 )
    {
        const cgChar  * pNextSrcA   = pSrcA;
        cgWChar       * pNextDstW   = pDstW;
        CodeCvtFacet::result res    = CodeCvtFacet::ok;
        const CodeCvtFacet& conv    = std::use_facet<CodeCvtFacet>(loc);
        CodeCvtFacet::state_type st = { 0 };
        res							= conv.in(st, pSrcA, pSrcA + nSrc, pNextSrcA, pDstW, pDstW + nDst, pNextDstW);

        // Null terminate the converted string
        if ( pNextDstW - pDstW > nDst )
            *(pDstW + nDst) = '\0';
        else
            *pNextDstW = '\0';
    }
    return pDstW;
}

inline cgChar * stringCodeCvt( cgChar * pDstA, int nDst, const cgWChar * pSrcW, int nSrc,
                               const std::locale& loc=std::locale())
{
    pDstA[0] = '\0';	

    if ( nSrc > 0 )
    {
        cgChar        * pNextDstA   = pDstA;
        const cgWChar * pNextSrcW   = pSrcW;
        CodeCvtFacet::result res    = CodeCvtFacet::ok;
        const CodeCvtFacet& conv    = std::use_facet<CodeCvtFacet>(loc);
        CodeCvtFacet::state_type st = { 0 };
        res                         = conv.out(st, pSrcW, pSrcW + nSrc, pNextSrcW, pDstA, pDstA + nDst, pNextDstA);

        // Null terminate the converted string
        if ( pNextDstA - pDstA > nDst )
            *(pDstA + nDst) = '\0';
        else
            *pNextDstA = '\0';
    }
    return pDstA;
}

#define stringConvertA2W(pa) (\
    ((_pa = pa) == 0) ? 0 : (\
    _cvt = (cgInt32)(strlen(_pa)),\
    stringCodeCvt((cgWChar*)_alloca((_cvt+1)*2), (_cvt+1)*2, \
    _pa, _cvt)))
#define stringConvertW2A(pw) (\
    ((_pw = pw) == 0) ? 0 : (\
    _cvt = (cgInt32)(wcslen(_pw)),\
    stringCodeCvt((cgChar*) _alloca((_cvt+1)*2), (_cvt+1)*2, \
    _pw, _cvt)))
#define stringConvertA2CW(pa) ((const cgWChar*)stringConvertA2W((pa)))
#define stringConvertW2CA(pw) ((const cgChar*)stringConvertW2A((pw)))

#ifdef UNICODE
#define stringConvertT2A	stringConvertW2A
#define stringConvertA2T	stringConvertA2W
#define stringConvertT2CA	stringConvertW2CA
#define stringConvertA2CT	stringConvertA2CW
inline cgWChar       * stringConvertT2W( cgTChar * p ) { return p; }
inline cgTChar       * stringConvertW2T( cgWChar * p ) { return p; }
inline const cgWChar * stringConvertT2CW( const cgTChar * p ) { return p; }
inline const cgTChar * stringConvertW2CT( const cgWChar * p ) { return p; }
#else // UNICODE
#define stringConvertT2W	stringConvertA2W
#define stringConvertW2T	stringConvertW2A
#define stringConvertT2CW	stringConvertA2CW
#define stringConvertW2CT	stringConvertW2CA
inline cgChar        * stringConvertT2A( cgTChar * p ) { return p; }
inline cgTChar       * stringConvertA2T( cgChar  * p ) { return p; }
inline const cgChar  * stringConvertT2CA( const cgTChar * p ) { return p; }
inline const cgTChar * stringConvertA2CT( const cgChar  * p ) { return p; }
#endif // !UNICODE

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Name : StrICmp (Class)
// Desc : Case insensitive string compare class.
//-------------------------------------------------------------------------
class StrICmp
{
public:
    //---------------------------------------------------------------------
    // Constructors & Destructors
    //---------------------------------------------------------------------
    StrICmp( const cgChar * lpszLang = "english" ) : m_Locale( lpszLang ) {}
    
    //---------------------------------------------------------------------
    // Public Classes
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Name : CharLessI (Class)
    // Desc : Case insensitive character less-than operator
    //---------------------------------------------------------------------
    class CharLessI
    {
    public:
        //-----------------------------------------------------------------
        // Constructors & Destructors
        //-----------------------------------------------------------------
        CharLessI( std::locale & Locale ) : m_Locale( Locale ) {}
        
        //-----------------------------------------------------------------
        // Public Operators
        //-----------------------------------------------------------------
        template<typename T>
        bool operator()(T c1, T c2)
        {
            return std::tolower(c1, m_Locale) < std::tolower(c2, m_Locale);
        
        } // End Operator

    private:
        //-----------------------------------------------------------------
        // Private Member Variables
        //-----------------------------------------------------------------
        std::locale & m_Locale;
    };

    //---------------------------------------------------------------------
    // Public Operators
    //---------------------------------------------------------------------
    int operator()(const std::basic_string<cgTChar> & s1, const std::basic_string<cgTChar> & s2)
    {
        if (std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), CharLessI(m_Locale)))
            return -1;
        if (std::lexicographical_compare(s2.begin(), s2.end(), s1.begin(), s1.end(), CharLessI(m_Locale)))
            return 1;
        return 0;
    
    } // End Operator

private:
    //---------------------------------------------------------------------
    // Private Member Variables
    //---------------------------------------------------------------------
    std::locale m_Locale;
};

//-----------------------------------------------------------------------------
//  Name : cgString (Class)
/// <summary>
/// String class implementation, based on providing the ease of use of the 
/// speed and power of basic_string whilst allowing for the  provision of 
/// additional functionality.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgString : public std::basic_string<cgTChar>
{
    // Useful typedefs.
    #define MYBASE std::basic_string<cgTChar>
    typedef cgString                    MYTYPE;
    typedef MYBASE::const_pointer       PCMYSTR;
    typedef MYBASE::pointer             PMYSTR;
    typedef MYBASE::const_iterator      MYCITER;
    typedef MYBASE::size_type           MYSIZE;
    typedef MYBASE::value_type          MYVAL;
    typedef MYBASE::allocator_type      MYALLOC;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgString( ) {}
    cgString(const MYTYPE& str)
        : MYBASE(str) {}
    cgString( const MYBASE & str )
        : MYBASE( str ) {}
    cgString(PCMYSTR pT)
        : MYBASE(pT) { 
#ifdef STRING_CONSTRUCT_REPORT
            buildStringConstructReport();
#endif
        }
    cgString(PCMYSTR pT, MYSIZE n)
        : MYBASE(pT, n) {}
    cgString(MYCITER first, MYCITER last)
        : MYBASE(first, last) {}
    cgString(MYSIZE nSize, MYVAL ch, const MYALLOC& al=MYALLOC())
        : MYBASE(nSize, ch, al) {}

    // Promote overloads.
    using MYBASE::compare;
    using MYBASE::replace;
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    #ifdef STRING_CONSTRUCT_REPORT
    static std::map<cgString, size_t> & getConstructReport()
    {
        static std::map<cgString, size_t> Report;
        return Report;
    }
    static void outputStringConstructReport( )
    {
        std::multimap<size_t, cgString> sortedReport;
        std::map<cgString,size_t>::const_iterator itString = getConstructReport().begin();
        for ( ; itString != getConstructReport().end(); ++itString )
            sortedReport.insert( std::pair<size_t,cgString>( itString->second, itString->first ) );
        std::multimap<size_t, cgString>::iterator itSortedString = sortedReport.begin();
        for ( ; itSortedString != sortedReport.end(); ++itSortedString )
        {
            #ifdef UNICODE
                wprintf( L"%s = %i\n", itSortedString->second.c_str(), itSortedString->first );
            #else
                printf( "%s = %i\n", itSortedString->second.c_str(), itSortedString->first );
            #endif
        }
    }
    void buildStringConstructReport( )
    {
        getConstructReport()[*this]++;
    }
    #endif

    //-------------------------------------------------------------------------
    //  Name : compare ()
    /// <summary>
    /// Compare the two strings with optional case ignore.
    /// </summary>
    //-------------------------------------------------------------------------
    int compare( const cgString & s2, bool ignoreCase, const cgChar * language = "english" ) const
    {
        if ( !ignoreCase )
            return MYBASE::compare( s2 );
        else
            return StrICmp(language)(*this, s2);
    }

    //-------------------------------------------------------------------------
    //  Name : trim()
    /// <summary>
    /// Strips all white space (space and tab characters) from both the head 
    /// and tail of this string, returning a reference to self (not copy).
    /// </summary>
    //-------------------------------------------------------------------------
    MYTYPE& trim( )
    {
        MYSIZE pos;
        
        // Early out
        if ( this->empty() == true )
            return *this;

        // Trim Left
        pos = this->find_first_not_of(_T(" \t"));
        if ( pos != cgString::npos )
            this->erase(0,pos);
        else
            this->clear();

        // Early out
        if ( this->empty() == true )
            return *this;

        // Trim right
        pos = this->find_last_not_of(_T(" \t"));
        if ( pos != cgString::npos )
            this->erase(pos+1);
        else
            this->clear();

        // Return self.
        return *this;
    }

    //-------------------------------------------------------------------------
    //  Name : toUpper()
    /// <summary>
    /// Process the string, transforming all characters to upper case where
    /// appropriate. Returns a reference to self (not a copy).
    /// </summary>
    //-------------------------------------------------------------------------
    MYTYPE& toUpper( )
    {
        std::transform( this->begin(), this->end(), this->begin(), (int(*)(int))toupper );
        return *this;
    }

    //-------------------------------------------------------------------------
    //  Name : toLower()
    /// <summary>
    /// Process the string, transforming all characters to lower case where
    /// appropriate. Returns a reference to self (not a copy).
    /// </summary>
    //-------------------------------------------------------------------------
    MYTYPE& toLower( )
    {
        std::transform( this->begin(), this->end(), this->begin(), (int(*)(int))tolower );
        return *this;
    }

    //-------------------------------------------------------------------------
    //  Name : beginsWith ()
    /// <summary>
    /// Determine if this string begins with the string provided.
    /// </summary>
    //-------------------------------------------------------------------------
    bool beginsWith( const cgString & value, bool ignoreCase = false ) const
    {
        // Validate requirements
        if ( this->length() < value.length() )
            return false;
        if ( this->empty() == true || value.empty() == true )
            return false;

        // Do the subsets match?
        if ( cgString(this->substr( 0, value.length() )).compare( value, ignoreCase ) == 0 )
            return true;

        // No match
        return false;
    }

    //-------------------------------------------------------------------------
    //  Name : endsWith ()
    /// <summary>
    /// Determine if this string ends with the string provided.
    /// </summary>
    //-------------------------------------------------------------------------
    bool endsWith( const cgString & value, bool ignoreCase = false ) const
    {
        // Validate requirements
        if ( this->size() < value.size() )
            return false;
        if ( this->empty() == true || value.empty() == true )
            return false;

        // Do the subsets match?
        if ( cgString(this->substr( this->length() - value.length() )).compare( value, ignoreCase ) == 0 )
            return true;

        // No match
        return false;
    }

    //-------------------------------------------------------------------------
    //  Name : replace ()
    /// <summary>
    /// Replacing any occurances of the specified character sequence with 
    /// another directly in-place within this string.
    /// </summary>
    //-------------------------------------------------------------------------
    MYTYPE& replace( const MYTYPE & oldSequence, const MYTYPE & newSequence )
    {
        MYSIZE location  = 0;
        MYSIZE oldLength = oldSequence.length();
        MYSIZE newLength = newSequence.length();
        
        // Search for all replace string occurances.
        if ( this->empty() == false )
        {
            while ( MYTYPE::npos != ( location = this->find( oldSequence, location ) ) )
            {
                MYBASE::replace( location, oldLength, newSequence );
                location += newLength;

                // Break out if we're done
                if ( location >= this->length() )
                    break;
            
            } // Next occurance
        
        } // End if not empty

        // Return self
        return *this;
    }

    //-------------------------------------------------------------------------
    //  Name : replace ()
    /// <summary>
    /// Replacing any occurances of the specified character with  another 
    /// directly in-place within this string.
    /// </summary>
    //-------------------------------------------------------------------------
    MYTYPE& replace( MYVAL oldCharacter, MYVAL newCharacter )
    {
        MYSIZE location = 0;
       
        // Search for all replace string occurances.
        if ( this->empty() == false )
        {
            while ( MYTYPE::npos != ( location = this->find( oldCharacter, location ) ) )
            {
                MYBASE::replace( location, 1, 1, newCharacter );
                location += 1;

                // Break out if we're done
                if ( location >= this->length() )
                    break;
            
            } // Next occurance
        
        } // End if not empty

        // Return self
        return *this;
    }

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : trim() (Static)
    /// <summary>
    /// Strips all white space (space and tab characters) from both the head 
    /// and tail of this string, returning a reference to self (not copy).
    /// </summary>
    //-------------------------------------------------------------------------
    inline static MYTYPE trim( const cgString & value )
    {
        return cgString(value).trim();
    }

    //-------------------------------------------------------------------------
    //  Name : toUpper() (Static)
    /// <summary>
    /// Process the string, transforming all characters to upper case where
    /// appropriate. Returns a new copy.
    /// </summary>
    //-------------------------------------------------------------------------
    inline static MYTYPE toUpper( const cgString & value )
    {
        return cgString( value ).toUpper();
    }

    //-------------------------------------------------------------------------
    //  Name : toLower() (Static)
    /// <summary>
    /// Process the string, transforming all characters to lower case where
    /// appropriate. Returns a new copy.
    /// </summary>
    //-------------------------------------------------------------------------
    inline static MYTYPE toLower( const cgString & value )
    {
        return cgString( value ).toLower();
    }

    //-------------------------------------------------------------------------
    //  Name : replace () (Static)
    /// <summary>
    /// Return a new copy of the provided string, replacing any occurances of 
    /// the specified character sequence with another.
    /// </summary>
    //-------------------------------------------------------------------------
    static cgString replace( const cgString & value, const cgString & oldSequence, const cgString & newSequence )
    {
        return cgString( value ).replace( oldSequence, newSequence );
    }

    //-------------------------------------------------------------------------
    //  Name : replace () (Static)
    /// <summary>
    /// Return a new copy of the provided string, replacing any occurances of 
    /// the specified character with another.
    /// </summary>
    //-------------------------------------------------------------------------
    static cgString replace( const cgString & value, MYVAL oldCharacter, MYVAL newCharacter )
    {
        return cgString( value ).replace( oldCharacter, newCharacter );
    }

    //-------------------------------------------------------------------------
    //  Name : format ()
    /// <summary>
    /// Generate a newly formatted string based on a format descriptor and
    /// an optional variable bufferLength list of arguments.
    /// </summary>
    //-------------------------------------------------------------------------
    static cgString format( const cgTChar * formatPattern, va_list ap )
    {
        int      result = -1, bufferLength = 256;
        cgTChar* buffer = 0;
        cgString output;

        // Validate parameters
        if ( formatPattern == 0 || formatPattern[0] == 0 )
            return cgString::Empty;

        // Keep trying to re-allocate until buffer is large enough
        for ( ; result == -1; bufferLength *= 2 )
        {
            // Re-allocate
            if ( buffer != 0 ) delete []buffer;
            buffer = new cgTChar[bufferLength + 1];
            memset( buffer, 0, bufferLength + 1);

            // Attempt to format
            #if _MSC_VER >= 1400
                #if defined( _UNICODE )
                    result = _vsnwprintf_s( buffer, bufferLength, _TRUNCATE, formatPattern, ap );
                #else
                    result = _vsnprintf_s( buffer, bufferLength, _TRUNCATE, formatPattern, ap );
                #endif
            #else
                #if defined( _UNICODE )
                    result = _vsnwprintf( buffer, bufferLength, formatPattern, ap );
                #else
                    result = _vsnprintf( buffer, bufferLength, formatPattern, ap );
                #endif
            #endif
            
        } // Next attempt

        // Finished building string
        output = cgString(buffer);

        // Clean up and return
        delete []buffer;
        return output;
    }

    //-------------------------------------------------------------------------
    //  Name : format ()
    /// <summary>
    /// Generate a newly formatted string based on a format descriptor and
    /// an optional variable length list of arguments.
    /// </summary>
    //-------------------------------------------------------------------------
    static cgString format( const cgTChar * formatPattern, ... )
    {
        int       result = -1, length = 256;
        cgTChar * buffer = 0;
        cgString  output;

        // Validate parameters
        if ( formatPattern == 0 || formatPattern[0] == 0 )
            return cgString::Empty;

        // Begin processing the variable argument list
        va_list	ap;
        va_start(ap, formatPattern);
        
        // Keep trying to re-allocate until buffer is large enough
        for ( ; result == -1; length *= 2 )
        {
            // Re-allocate
            if ( buffer != 0 ) delete []buffer;
            buffer = new cgTChar[length + 1];
            memset( buffer, 0, length + 1);

            // Attempt to format
            #if _MSC_VER >= 1400
                #if defined( _UNICODE )
                    result = _vsnwprintf_s( buffer, length, _TRUNCATE, formatPattern, ap );
                #else
                    result = _vsnprintf_s( buffer, length, _TRUNCATE, formatPattern, ap );
                #endif
            #else
                #if defined( _UNICODE )
                    result = _vsnwprintf( buffer, length, formatPattern, ap );
                #else
                    result = _vsnprintf( buffer, length, formatPattern, ap );
                #endif
            #endif
            
        } // Next attempt

        // Finished building string
        output = cgString(buffer);
        va_end(ap);

        // Clean up and return
        delete []buffer;
        return output;
    }

    //-------------------------------------------------------------------------
    //  Name : wordWrap ()
    /// <summary>
    /// Wraps the string up to the maximum length and optionally inserts an
    /// aribitrary padding string at the beginning of each new line
    /// </summary>
    //-------------------------------------------------------------------------
    static cgString wordWrap( const cgString & value, MYSIZE maximumLength, const cgString & linePadding = cgString::Empty )
    {
        cgString wrapString, currentLine;
        MYSIZE lastSpace = -1, lineLength = 0;
        
        // ToDo: Add support for tab character to wrapping method.

        // Loop through each character in the string
        for ( MYSIZE i = 0, length = value.length(); i < length; ++i )
        {
            cgTChar character = value[i];
            switch( character )
            {
                case _T('\r'):
                    // Character type not supported
                    break;

                case _T(' '):
                    // A space was found, firstly does it exceed the max line length?
                    if ( lineLength == maximumLength )
                    {
                        // Simply wrap without inserting anything
                        wrapString += currentLine + _T("\n");
                        currentLine = cgString::Empty;
                        lineLength    = 0;
                        lastSpace     = -1;
                    
                    } // End if will exceed line length
                    else
                    {
                        // Add padding if we haven'value2 already
                        if ( wrapString.empty() == false && lineLength == 0 )
                        {
                            currentLine = linePadding;
                            lineLength    = (int)currentLine.length();
                        
                        } // End if padding required

                        // Record it's position and insert the space
                        currentLine += character;
                        lastSpace      = lineLength;
                        lineLength++;

                    } // End if no length exceed
                    break;

                case _T('\n'):
                    // When we encounter a newline, just break automatically
                    wrapString += currentLine + _T("\n");
                    currentLine = cgString::Empty;
                    lineLength    = 0;
                    lastSpace     = -1;
                    break;

                default:
                    // Exceeding line length?
                    if ( lineLength == maximumLength )
                    {
                        // No space found on this line yet?
                        if ( lastSpace == -1 )
                        {
                            // Just break onto a new line
                            wrapString += currentLine + _T("\n");
                            currentLine = cgString::Empty;
                            lineLength    = 0;
                            lastSpace     = -1;
                        
                        } // End if no space found
                        else
                        {
                            // Break onto a new-line where the space was found
                            if ( lastSpace > 0 ) wrapString += currentLine.substr( 0, lastSpace ) + _T("\n");
                            currentLine = linePadding + currentLine.substr( lastSpace + 1 );
                            lineLength    = currentLine.length();
                            lastSpace     = -1;
                            

                        } // End if space found on line

                    } // End if exceeding line length

                    // Add padding if we haven'value2 already
                    if ( wrapString.empty() == false && lineLength == 0 )
                    {
                        currentLine = linePadding;
                        lineLength    = currentLine.length();
                    
                    } // End if padding required

                    // Add character
                    currentLine += character;
                    lineLength++;

            } // End character switch
            
        } // Next character

        // Add anything that's left to the wrap string
        wrapString += currentLine;

        // Return it
        return wrapString;
    }

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    MYTYPE& operator=(const MYTYPE& value)
    { 
        if ( this->c_str() != value.c_str() )
        {
            erase();
            assign(value);
        } // End if not equal
        return *this;
    }

    MYTYPE& operator=(const MYBASE& value)
    {
        if ( this->c_str() != value.c_str() )
        {
            erase();
            assign(value);
        } // End if not equal
        return *this;
    }

    MYTYPE& operator=(PCMYSTR value)
    { 
        if ( this->c_str() != value )
        {
            erase();
            assign(value);
        } // End if not equal
        return *this;
    }

    MYTYPE& operator=(MYVAL value)
    {
    #if defined ( _MSC_VER ) && ( _MSC_VER < 1200 )
        erase();
    #endif
        assign(1,value);
        return *this;
    }

    MYTYPE& operator+=(const MYTYPE& value)
    {
        append( value );
        return *this;
    }

    MYTYPE& operator+=(const MYBASE& value)
    {
        append( value );
        return *this;
    }

    MYTYPE& operator+=(PCMYSTR value)
    {
        append( value );
        return *this;
    }

    MYTYPE& operator+=(MYVAL value)
    {
        append(1, value);
        return *this;
    }

    // Allows 'cgString' to function with 'unordered_map' and 'unordered_set'.
    operator size_t() const
    {
        const cgTChar * __s = this->c_str();
        unsigned long __h = 0;
        for ( ; *__s; ++__s)
            __h = 5*__h + *__s;
        return size_t(__h);
    }

    //-------------------------------------------------------------------------
    // Public Static Variables
    //-------------------------------------------------------------------------
    static const cgString Empty;
};

#if defined(__SGI_STL_PORT)
// This allows us to use cgString as a key in a unordered_map or unordered_set
namespace std
{
    template<> struct hash<cgString>
    {
        inline size_t operator()(const cgString& x) const { return (size_t)x; }
    };
};
#endif // __SGI_STL_PORT

//-----------------------------------------------------------------------------
// Public Global Operators
//-----------------------------------------------------------------------------
inline cgString operator+(const cgString& value1, const cgString& value2)
{
    cgString result(value1);
    result.append(value2);
    return result;
}
inline cgString operator+(const cgString& value1, cgString::value_type value2)
{
    cgString result(value1);
    result.append(1, value2);
    return result;
}
inline cgString operator+ ( const cgString& value1, const cgString::value_type * value2)
{
    cgString result(value1);
    result.append(value2);
    return result;
}
inline cgString operator+( const cgString::value_type * value1, const cgString & value2)
{
    cgString result( value1 );
    result.append( value2 );
    return result;
}

#endif	// !_CGE_CGSTRING_H_