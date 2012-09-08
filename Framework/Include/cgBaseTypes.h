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
// Name : cgBaseTypes.h                                                      //
//                                                                           //
// Desc : Standalone common header file for the CGE library that             //
//        defines all of the required basic types (i.e. numeric) in a        //
//        platform independant fashion.                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBASETYPES_H_ )
#define _CGE_CGBASETYPES_H_

//-----------------------------------------------------------------------------
// System Setup Pragmas / Defines
//-----------------------------------------------------------------------------
// Disable warnings when using managed C++ (/clr) 
#pragma warning ( disable : 4793 )  
// Disable compile time CRT warnings
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#   define _CRT_SECURE_NO_DEPRECATE 1
#endif // !_CRT_SECURE_NO_DEPRECATE
#if !defined(_CRT_SECURE_NO_WARNINGS)
#   define _CRT_SECURE_NO_WARNINGS 1
#endif // !_CRT_SECURE_NO_DEPRECATE
#if !defined(_CRT_NONSTDC_NO_DEPRECATE)
#   define _CRT_NONSTDC_NO_DEPRECATE 1
#endif // !_CRT_NONSTDC_NO_DEPRECATE
// Disable (slow) STL iterator debugging.
#if !defined(_HAS_ITERATOR_DEBUGGING)
#   define _HAS_ITERATOR_DEBUGGING 0
#endif // !_HAS_ITERATOR_DEBUGGING
// Undefine <windef.h> min/max macros
#if defined(min)
#   undef min
#endif // min
#if defined(max)
#   undef max
#endif // min

//-----------------------------------------------------------------------------
// cgBaseTypes Header Includes
//-----------------------------------------------------------------------------
#include <System/cgString.h>    // cgString, <cgAPI.h>
#include <System/cgUID.h>       // cgUID

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgWorldObject;
class cgObjectNode;
class cgObjectSubElement;
class cgSpatialTreeLeaf;

//-----------------------------------------------------------------------------
// STL Integration
//-----------------------------------------------------------------------------
#if defined(__SGI_STL_PORT)
// Allows various types to function with 'hash_map' and 'hash_set'.
namespace std
{
    template<> struct hash<cgWorldObject*>
    {
        inline size_t operator()(const cgWorldObject* x ) const { return std::hash<void*>()( (void*)x ); }
    };
    template<> struct hash<cgObjectNode*>
    {
        inline size_t operator()(const cgObjectNode* x ) const { return std::hash<void*>()( (void*)x ); }
    };
    template<> struct hash<cgSpatialTreeLeaf*>
    {
        inline size_t operator()(const cgSpatialTreeLeaf* x ) const { return std::hash<void*>()( (void*)x ); }
    };

} // End Namespace std
#endif // __SGI_STL_PORT

//-----------------------------------------------------------------------------
// Base Typedefs
//-----------------------------------------------------------------------------
// Numeric
typedef unsigned char           cgByte;
typedef char                    cgInt8;
typedef unsigned char           cgUInt8;
typedef short                   cgInt16;
typedef unsigned short          cgUInt16;
typedef long                    cgInt32;
typedef unsigned long           cgUInt32;
typedef signed __int64          cgInt64;
typedef unsigned __int64        cgUInt64;
typedef int                     cgInt;      // Platform specific integer type (i.e. 32 vs 64 bit).
typedef unsigned int            cgUInt;     // Platform specific unsigned integer type (i.e. 32 vs 64 bit).
typedef float                   cgFloat;    // Explicit single precision.
typedef double                  cgDouble;   // Explicit double precision.
typedef float                   cgReal;     // Configurable single / double precision.

// ToDo: LPVOID, LPCVOID
// ToDo: LPTSTR, LPCTSTR, LPCSTR, LPSTR etc.
// ToDo: HRESULT?

//-----------------------------------------------------------------------------
// Base System Defines
//-----------------------------------------------------------------------------
#define CG_NULL 0

//-----------------------------------------------------------------------------
// System Structures
//-----------------------------------------------------------------------------
/// <summary>For functions which accept a debug source structure (Source File, Source Line)</summary>
struct CGE_API cgDebugSourceInfo
{
    cgString    source;
    cgUInt32    line;
    cgDebugSourceInfo( ) :
        source( _T("") ),
        line( 0 ) {}
    cgDebugSourceInfo( const cgString & _source, cgUInt32 _line ) :
        source( _source ),
        line( _line ) {}
};

//-----------------------------------------------------------------------------
// System Macros
//-----------------------------------------------------------------------------
//#if defined(_DEBUG)
    #define __CGTEXT(quote) L##quote
    #define CGTEXT(quote) __CGTEXT(quote)
    #define cgDebugSource() cgDebugSourceInfo(CGTEXT(__FILE__),__LINE__)
//#else
    //#define cgDebugSource() cgDebugSourceInfo(cgString::Empty,0)
//#endif

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct CGE_API cgHalf
{
public:
    cgHalf() {};
    cgHalf( cgFloat f )
    {
        const cgUInt32 data = (cgUInt32&)f;
        const cgUInt32 signBit = (data >> 31);
        cgUInt32 exponent = ((data >> 23) & 0xFF);
        cgUInt32 mantissa = (data & 0x7FFFFF);

        // Handle cases
        if ( exponent == 255 ) // NaN or inf
            exponent = 31;
        else if ( exponent < 102 ) // (127-15)-10
            exponent = mantissa = 0;
        else if ( exponent >= 143 ) // 127+(31-15)
        {
            exponent = 31;
            mantissa = 0;
        }
        else if( exponent <= 112 ) // 127-15
        {
            mantissa |= (1<<23);
            mantissa = mantissa >> (1 + (112 - exponent));
            exponent = 0;
        }
        else
            exponent -= 112;

        // Store
        _data = (cgUInt16)((signBit << 15) | (exponent << 10) | (mantissa >> 13));
    }

    cgHalf( const cgHalf & h ) :
        _data( h._data ) {}

    // Inline operators
    inline bool operator == ( const cgHalf & h ) const
    {
        return _data == h._data;
    }
    inline bool operator != ( const cgHalf & h ) const
    {
        return _data != h._data;
    }
    inline operator cgFloat() const
    {
        const cgUInt32 signBit = (_data >> 15);
        cgUInt32 exponent = ((_data >> 10) & 0x1F);
        cgUInt32 mantissa = (_data & 0x03FF);
        
        // Handle cases
        if(exponent == 31)
            exponent = 255;
        else if(exponent == 0) 
            exponent = 0;
        else
            exponent += 112;
        
        // Convert
        const cgUInt32 data = (signBit << 31) | (exponent << 23) | (mantissa << 13);
        return (cgFloat&)data;
    }

private:
    // Private Members
    cgUInt16 _data;

}; // End Struct cgHalf

struct cgCallbackData
{
    void * functionPointer;
    void * context;

}; // End Struct cgCallbackData

struct CGE_API cgPoint
{
    cgPoint(){}
    cgPoint( cgInt32 _x, cgInt32 _y ) :
        x(_x), y(_y) {}
    cgInt32 x;
    cgInt32 y;

    // Inline operators
    inline bool operator==( const cgPoint & b ) const
    {
        return (x == b.x && y == b.y);
    }
    inline bool operator!=( const cgPoint & b ) const
    {
        return (x != b.x || y != b.y);
    }

}; // End Struct cgPoint

struct CGE_API cgPointF
{
    cgPointF(){}
    cgPointF( cgFloat _x, cgFloat _y ) :
        x(_x), y(_y) {}
    cgFloat x;
    cgFloat y;

    // Inline operators
    inline bool operator==( const cgPointF & b ) const
    {
        return (x == b.x && y == b.y);
    }
    inline bool operator!=( const cgPointF & b ) const
    {
        return (x != b.x || y != b.y);
    }

}; // End Struct cgPointF

struct CGE_API cgSize
{
    cgSize(){}
    cgSize( cgInt32 _width, cgInt32 _height ) :
        width(_width), height(_height) {}
    cgInt32 width;
    cgInt32 height;

    // Inline operators
    inline bool operator==( const cgSize & b ) const
    {
        return (width == b.width && height == b.height);
    }
    inline bool operator!=( const cgSize & b ) const
    {
        return (width != b.width || height != b.height);
    }
    inline bool operator<( const cgSize & b ) const
    {
		if ( width > b.width )
			return false;
		if ( width < b.width )
			return true;
		return ( height < b.height );
    }
    inline bool operator>( const cgSize & b ) const
    {
		if ( width < b.width )
			return false;
		if ( width > b.width )
			return true;
		return ( height > b.height );
    }

}; // End Struct cgSize

struct CGE_API cgSizeF
{
    cgSizeF(){}
    cgSizeF( cgFloat _width, cgFloat _height ) :
        width(_width), height(_height) {}
    cgFloat width;
    cgFloat height;

    // Inline operators
    inline bool operator==( const cgSizeF & b ) const
    {
        return (width == b.width && height == b.height);
    }
    inline bool operator!=( const cgSizeF & b ) const
    {
        return (width != b.width || height != b.height);
    }
    inline bool operator<( const cgSizeF & b ) const
    {
		if ( width > b.width )
			return false;
		if ( width < b.width )
			return true;
		return ( height < b.height );
    }
    inline bool operator>( const cgSizeF & b ) const
    {
		if ( width < b.width )
			return false;
		if ( width > b.width )
			return true;
		return ( height > b.height );
    }
    
}; // End Struct cgSizeF

struct CGE_API cgRange
{
    cgRange(){}
    cgRange( cgInt32 _min, cgInt32 _max ) :
        min(_min), max(_max) {}
    cgInt32 min;
    cgInt32 max;

    // Inline operators
    inline bool operator==( const cgRange & b ) const
    {
        return (min == b.min && max == b.max);
    }
    inline bool operator!=( const cgRange & b ) const
    {
        return (min != b.min || max != b.max);
    }

}; // End Struct cgRange

struct CGE_API cgRangeF
{
    cgRangeF(){}
    cgRangeF( cgFloat _min, cgFloat _max ) :
        min(_min), max(_max) {}
    cgFloat min;
    cgFloat max;

    // Inline operators
    inline bool operator==( const cgRangeF & b ) const
    {
        return (min == b.min && max == b.max);
    }
    inline bool operator!=( const cgRangeF & b ) const
    {
        return (min != b.min || max != b.max);
    }

}; // End Struct cgRangeF

struct CGE_API cgRect
{
    cgRect(){}
    cgRect( cgInt32 _left, cgInt32 _top, cgInt32 _right, cgInt32 _bottom ) : 
        left(_left), top(_top), right(_right), bottom(_bottom) {}
    cgInt32 left;
    cgInt32 top;
    cgInt32 right;
    cgInt32 bottom;

    // Inline functions
    inline bool isEmpty( ) const { return ( left == 0 && right == 0 && top == 0 && bottom == 0 ); }
    inline cgInt32 width( ) const { return right - left; }
    inline cgInt32 height( ) const { return bottom - top; }
    inline cgSize size( ) const { return cgSize( right - left, bottom - top ); }
    inline bool containsPoint( const cgPoint & point ) const
    {
        return ( point.x >= left && point.x <= right &&
                 point.y >= top && point.y <= bottom );
    }
    static cgRect intersect( const cgRect & a, const cgRect & b )
    {
        cgRect c( __max( a.left, b.left ), __max( a.top, b.top ),
                  __min( a.right, b.right ), __min( a.bottom, b.bottom ) );
        
        // If no intersection occurred, just return an empty rectangle
        if ( c.left > c.right || c.top > c.bottom )
            return cgRect( 0,0,0,0 );
        else
            return c;
    }

    // Inline operators
    inline bool operator==( const cgRect & b ) const
    {
        return (left == b.left && top == b.top && right == b.right && bottom == b.bottom );
    }
    inline bool operator!=( const cgRect & b ) const
    {
        return (left != b.left || top != b.top || right != b.right || bottom != b.bottom );
    }
    inline cgRect & operator+=( const cgPoint & p )
    {
        left += p.x;
        right += p.x;
        top += p.y;
        bottom += p.y;
        return *this;
    }
    inline cgRect & operator-=( const cgPoint & p )
    {
        left -= p.x;
        right -= p.x;
        top -= p.y;
        bottom -= p.y;
        return *this;
    }

    // Static inline functions
    static cgRect inflate( const cgRect & rc, cgInt32 x, cgInt32 y )
    {
        return cgRect( rc.left - x, rc.top - y, rc.right + x, rc.bottom + y );
    }

}; // End Struct cgRect

struct CGE_API cgRectF
{
    cgRectF(){}
    cgRectF( cgFloat _left, cgFloat _top, cgFloat _right, cgFloat _bottom ) : 
        left(_left), top(_top), right(_right), bottom(_bottom) {}
    cgFloat left;
    cgFloat top;
    cgFloat right;
    cgFloat bottom;

    // Inline functions
    inline cgFloat width( ) const { return right - left; }
    inline cgFloat height( ) const { return bottom - top; }
    inline bool containsPoint( const cgPointF & Point ) const
    {
        return ( Point.x >= left && Point.x <= right &&
                 Point.y >= top && Point.y <= bottom );
    }

    // Inline operators
    inline bool operator==( const cgRectF & b ) const
    {
        return (left == b.left && top == b.top && right == b.right && bottom == b.bottom );
    }
    inline bool operator!=( const cgRectF & b ) const
    {
        return (left != b.left || top != b.top || right != b.right || bottom != b.bottom );
    }
    inline cgRectF & operator+=( const cgPointF & p )
    {
        left += p.x;
        right += p.x;
        top += p.y;
        bottom += p.y;
        return *this;
    }
    inline cgRectF & operator-=( const cgPointF & p )
    {
        left -= p.x;
        right -= p.x;
        top -= p.y;
        bottom -= p.y;
        return *this;
    }

    // Static inline functions
    static cgRectF inflate( const cgRectF & rc, cgFloat x, cgFloat y )
    {
        return cgRectF( rc.left - x, rc.top - y, rc.right + x, rc.bottom + y );
    }

}; // End Struct cgRectF

struct CGE_API cgColorValue
{
    cgColorValue() {}
    cgColorValue( cgFloat _r, cgFloat _g, cgFloat _b, cgFloat _a ) :
        r(_r), g(_g), b(_b), a(_a) {}
    cgColorValue( cgUInt32 c ) :
        r( (cgFloat)(cgByte)(c >> 16) / 255.0f ),
        g( (cgFloat)(cgByte)(c >> 8) / 255.0f ),
        b( (cgFloat)(cgByte)(c >> 0) / 255.0f ),
        a( (cgFloat)(cgByte)(c >> 24) / 255.0f ) {}
    cgFloat r;
    cgFloat g;
    cgFloat b;
    cgFloat a;

    // casting
    operator cgUInt32() const
    {
        // Compute color
        return ((cgUInt32)(std::min<cgFloat>( 255.0f, a * 255.0f )) << 24) |
               ((cgUInt32)(std::min<cgFloat>( 255.0f, r * 255.0f )) << 16) |
               ((cgUInt32)(std::min<cgFloat>( 255.0f, g * 255.0f )) << 8 ) |
               ((cgUInt32)(std::min<cgFloat>( 255.0f, b * 255.0f ))      );
    
    } // End Cast

    inline operator cgFloat* ()
    {
        return (cgFloat*)&r;
    
    } // End Cast

    inline operator const cgFloat* () const
    {
        return (const cgFloat*)&r;
    
    } // End Cast

    // operators.
    cgColorValue & operator*= ( cgFloat value )
    {
        r *= value;
        g *= value;
        b *= value;
        a *= value;
        return *this;
    
    } // End operator*=

    cgColorValue operator* ( cgFloat value ) const
    {
        return cgColorValue( r * value, g * value, b * value, a * value );
    
    } // End operator*

    cgColorValue & operator/= ( cgFloat value )
    {
        r /= value;
        g /= value;
        b /= value;
        a /= value;
        return *this;
    
    } // End operator/=

    cgColorValue operator/ ( cgFloat value ) const
    {
        return cgColorValue( r / value, g / value, b / value, a / value );
    
    } // End operator/

    cgColorValue & operator+= ( const cgColorValue & c )
    {
        r += c.r;
        g += c.g;
        b += c.b;
        a += c.a;
        return *this;
    
    } // End operator+=

    cgColorValue operator+ ( const cgColorValue & c ) const
    {
        return cgColorValue( r + c.r, g + c.g, b + c.b, a + c.r );
    
    } // End operator+

    cgColorValue & operator-= ( const cgColorValue & c )
    {
        r -= c.r;
        g -= c.g;
        b -= c.b;
        a -= c.a;
        return *this;
    
    } // End operator-=

    cgColorValue operator- ( const cgColorValue & c ) const
    {
        return cgColorValue( r - c.r, g - c.g, b - c.b, a - c.r );
    
    } // End operator-

    bool operator==( const cgColorValue & c ) const
    {
        return (r == c.r && g == c.g && b == c.b && a == c.a );
    
    } // End operator ==

    friend cgColorValue CGE_API operator * ( cgFloat, const cgColorValue& );

    // utility methods
    static cgColorValue cgColorValue::clamp( const cgColorValue & c )
    {
        cgColorValue out;
        if ( c.a < 0.0f ) out.a = 0.0f;
        else if ( c.a > 1.0f ) out.a = 1.0f;
        else out.a = c.a;

        if ( c.r < 0.0f ) out.r = 0.0f;
        else if ( c.r > 1.0f ) out.r = 1.0f;
        else out.r = c.r;

        if ( c.g < 0.0f ) out.g = 0.0f;
        else if ( c.g > 1.0f ) out.g = 1.0f;
        else out.g = c.g;

        if ( c.b < 0.0f ) out.b = 0.0f;
        else if ( c.b > 1.0f ) out.b = 1.0f;
        else out.b = c.b;

        return out;
    
    } // End clamp()

    // utility methods
    static cgColorValue cgColorValue::lerp( const cgColorValue & c1, const cgColorValue & c2, cgFloat s )
    {
        cgColorValue out;
        out.a = c1.a + (s * (c2.a - c1.a));
        out.r = c1.r + (s * (c2.r - c1.r));
        out.g = c1.g + (s * (c2.g - c1.g));
        out.b = c1.b + (s * (c2.b - c1.b));
        return out;
    
    } // End lerp()

}; // End Struct cgColorValue
inline cgColorValue CGE_API operator* ( cgFloat value, const cgColorValue& c )
{
    return cgColorValue( c.r * value, c.g * value, c.b * value, c.a * value );

} // End global operator*

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
namespace cgConfigResult
{
    enum Base
    {
        /// <summary>
        /// The configuration options were loaded sucessfully, and they were also 
        /// matched exactly to a supported feature or mode.
        /// </summary>
        Valid       = 0,

        /// <summary>
        /// The configuration information could not be matched to any valid 
        /// modes supported by the system in question. In this case, it's most 
        /// likely that the software, hardware or drivers were changed since last
        /// time the application was run. If this value is returned, then the 
        /// application should notify the user, and then call the LoadDefaultConfig 
        /// function to have  it pick a sensible set of default options.
        /// </summary>
        Mismatch    = 1,

        /// <summary>
        /// An error occured whilst attempting to load and / or validate the
        /// configuration options specified. The application should probably exit.
        /// </summary>
        Error       = 2     
    };

} // End Namespace : cgConfigResult

/// <summary>
/// Describes the various types of units that the system can support.
/// Most commonly used as the means by which the physics systems are
/// initialized.
/// </summary>
namespace cgUnitType
{
    enum Base
    {
        Meters = 0,
        Centimeters,
        Millimeters,
        Inches,
        Feet,
        Yards
    };
    inline const cgString & toString( cgUnitType::Base type )
    {
        static const cgString types[] =
        {
            _T("Meters"), _T("Centimeters"), _T("Millimeters"),
            _T("Inches"), _T("Feet"), _T("Yards")
        };
        return types[(cgInt)type];
    }
    inline const cgString & toNotationString( cgUnitType::Base type )
    {
        static const cgString types[] =
        {
            _T("m"), _T("cm"), _T("mm"),
            _T("in"), _T("ft"), _T("yds")
        };
        return types[(cgInt)type];
    }

    inline cgDouble convert( cgDouble value, cgUnitType::Base typeFrom, cgUnitType::Base typeTo )
    {
        static const cgDouble conversionTable[6][6] = 
        {
            // Src = Meters
            //     m,    cm,    mm,           in,           ft,          yds
            {      1,   100,  1000,   39.3700787,    3.2808399,    1.0936133 },
            // Src = Centimeters
            //     m,    cm,    mm,           in,           ft,          yds
            {   0.01,     1,    10,  0.393700787,  0.032808399,  0.010936133 },
            // Src = Millimeters
            //     m,    cm,    mm,           in,           ft,          yds
            {  0.001,   0.1,     1, 0.0393700787, 0.0032808399, 0.0010936133 },
            // Src = Inches
            //     m,    cm,    mm,           in,           ft,          yds
            { 0.0254,  2.54,  25.4,            1, 0.0833333333, 0.0277777778 },
            // Src = Feet
            //     m,    cm,    mm,           in,           ft,          yds
            { 0.3048, 30.48, 304.8,           12,            1,  0.333333333 },
            // Src = Yards
            //     m,    cm,    mm,           in,           ft,          yds
            { 0.9144, 91.44, 914.4,           36,            3,            1 }
        };

        // Convert!
        return value * conversionTable[typeFrom][typeTo];
    }

}; // End Namespace : cgUnitType

// Common Containers
CGE_VECTOR_DECLARE          ( cgByte, cgByteArray )
CGE_VECTOR_DECLARE          ( cgInt8, cgInt8Array )
CGE_VECTOR_DECLARE          ( cgInt16, cgInt16Array )
CGE_VECTOR_DECLARE          ( cgUInt16, cgUInt16Array )
CGE_VECTOR_DECLARE          ( cgInt32, cgInt32Array )
CGE_UNORDEREDSET_DECLARE    ( cgInt32, cgInt32Set )
CGE_VECTOR_DECLARE          ( cgUInt32, cgUInt32Array )
CGE_UNORDEREDSET_DECLARE    ( cgUInt32, cgUInt32Set )
CGE_UNORDEREDMAP_DECLARE    ( cgUInt32, cgUInt32, cgUInt32IndexMap )
CGE_VECTOR_DECLARE          ( cgFloat, cgFloatArray )
CGE_VECTOR_DECLARE          ( cgDouble, cgDoubleArray )
CGE_VECTOR_DECLARE          ( cgPoint, cgPointArray )
CGE_VECTOR_DECLARE          ( cgRect, cgRectArray )
CGE_VECTOR_DECLARE          ( cgString, cgStringArray )
CGE_UNORDEREDSET_DECLARE    ( cgString, cgStringSet )
CGE_VECTOR_DECLARE          ( cgUID, cgUIDArray )
CGE_LIST_DECLARE            ( cgWorldObject*, cgWorldObjectList )
CGE_UNORDEREDSET_DECLARE    ( cgWorldObject*, cgWorldObjectSet )
CGE_VECTOR_DECLARE          ( cgWorldObject*, cgWorldObjectArray )
CGE_LIST_DECLARE            ( cgObjectNode*, cgObjectNodeList )
CGE_UNORDEREDSET_DECLARE    ( cgObjectNode*, cgObjectNodeSet )
CGE_MAP_DECLARE             ( cgUInt32, cgObjectNode*, cgObjectNodeMap )
CGE_VECTOR_DECLARE          ( cgObjectNode*, cgObjectNodeArray )
CGE_MAP_DECLARE             ( cgUInt32, cgObjectSubElement*, cgObjectSubElementMap )
CGE_VECTOR_DECLARE          ( cgObjectSubElement*, cgObjectSubElementArray )
CGE_VECTOR_DECLARE          ( cgSpatialTreeLeaf*, cgSceneLeafArray )
CGE_UNORDEREDSET_DECLARE    ( cgSpatialTreeLeaf*, cgSceneLeafSet )

#endif // !_CGE_CGBASETYPES_H_