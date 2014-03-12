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
// Name : cgVertexFormats.h                                                  //
//                                                                           //
// Desc : Contains the various vertex format classes used to construct the   //
//        different types of geometry within the scene.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGVERTEXFORMATS_H_ )
#define _CGE_CGVERTEXFORMATS_H_

//-----------------------------------------------------------------------------
// cgVertexFormats Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgVertexFormat (Class)
/// <summary>
/// Contains all of the information required in order for the engine to
/// understand arbitrary layouts of vertex data.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgVertexFormat
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
     cgVertexFormat( const cgVertexFormat & sourceFormat );
     cgVertexFormat( );
    ~cgVertexFormat( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    bool                            addVertexElement        ( cgUInt8 type, cgUInt8 usage );
    bool                            addVertexElement        ( cgUInt8 type, cgUInt8 usage, cgUInt8 index );
    bool                            isReadOnly              ( ) const;
    int                             getElementOffset        ( cgUInt32 usage, cgUInt8 index = 0 );
    const D3DVERTEXELEMENT9       * getElement              ( cgUInt32 usage, cgUInt8 index = 0 );
    
    // Object query methods
    cgUInt16                        getStride               ( );
    cgUInt16                        getElementCount         ( ) const { return mElementCount; }
    const D3DVERTEXELEMENT9       * getDeclarator           ( ) const { return mElements; }

    // Data access utilities
    cgVector3                     * getVertexPosition       ( cgByte * data );
    cgVector3                     * getVertexNormal         ( cgByte * data );
    cgVector3                     * getVertexBinormal       ( cgByte * data );
    cgVector3                     * getVertexTangent        ( cgByte * data );
    cgFloat                       * getVertexTextureCoords  ( cgByte * data, cgUInt8 index, cgUInt8 component = 0 );

    void                            setVertexPosition       ( cgByte * data, const cgVector3 & value  );
    void                            setVertexNormal         ( cgByte * data, const cgVector3 & value );
    void                            setVertexBinormal       ( cgByte * data, const cgVector3 & value );
    void                            setVertexTangent        ( cgByte * data, const cgVector3 & value );
    void                            setVertexTextureCoords  ( cgByte * data, cgUInt8 index, cgUInt8 component, cgFloat value );
    void                            setVertexBlendIndices   ( cgByte * data, cgUInt32 value );
    void                            setVertexBlendWeights   ( cgByte * data, cgVector4 & value );

    //-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    cgVertexFormat                & operator=               ( const cgVertexFormat & format );
    bool                            operator==              ( const cgVertexFormat & format ) const;
    bool                            operator!=              ( const cgVertexFormat & format ) const;

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgVertexFormat         * getDefaultFormat        ( );
    static void                     setDefaultFormat        ( cgVertexFormat * format );
    static cgVertexFormat         * formatFromFVF           ( cgUInt32 code );
    static cgVertexFormat         * formatFromDeclarator    ( const D3DVERTEXELEMENT9 declarator[] );
    static cgUInt32                 getDeclaratorTypeSize   ( D3DDECLTYPE type );
    static bool                     convertVertex           ( cgByte * dest, cgVertexFormat * destFormat, cgByte * src, cgVertexFormat * srcFormat );
    static bool                     convertVertices         ( cgByte * dest, cgVertexFormat * destFormat, cgByte * src, cgVertexFormat * srcFormat, cgUInt32 vertexCount );
    static cgInt                    compareVertices         ( cgByte * v1, cgByte * v2, cgVertexFormat * format, cgFloat epsilon );

private:
    //-------------------------------------------------------------------------
	// Private Typedefs, Structures and Enumerations
	//-------------------------------------------------------------------------
    // Structure for automatic cleanup of registered formats
    struct CGE_API FormatDatabase
    {
        // Members
        CGE_ARRAY_DECLARE(cgVertexFormat*, FormatArray)
        FormatArray formats;

        // Constructor / Destructor
        ~FormatDatabase()
        {
            // Destroy all formats
            for ( cgUInt32 i = 0; i < formats.size(); ++i )
                delete formats[i];

            // Clear vector
            formats.clear();

        } // End Destructor

    }; // End struct FormatDatabase

    //-------------------------------------------------------------------------
	// Private Methods
	//-------------------------------------------------------------------------
    void                            buildUsageData      ( );

    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static cgVertexFormat         * getExistingFormat   ( cgVertexFormat * format );

    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    D3DVERTEXELEMENT9     * mElements;      // The actual declaration / descriptor for this vertex format
    cgUInt16                mElementCount;  // Number of elements in the above declarator (EXCLUDING the 'CG_NULL' / 'D3DDECL_END' element)
    cgUInt16                mStride;        // Cached stride of this vertex format
    D3DVERTEXELEMENT9    ** mUsageMap;      // Internal lookup table for quickly finding the element structure for a given usage / index
    bool                    mElementsDirty; // Has data been added since the usage map was updated?
    bool                    mInDatabase;    // Is this format in the database currently?

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgVertexFormat * mDefaultFormat; // Default format to apply to all vertices that don't override
    static FormatDatabase   mFormats;       // Structure containing the actual formats
};

//-----------------------------------------------------------------------------
//  Name : cgVertex (Class)
/// <summary>
/// Vertex class used to construct & store vertex components for standard
/// geometry within the scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors For This Class.
    //-------------------------------------------------------------------------
    cgVertex( ); 
    cgVertex( cgFloat _x, cgFloat _y, cgFloat _z, const cgVector3 & _normal, cgFloat _tu = 0.0f, cgFloat _tv = 0.0f );
    cgVertex( const cgVector3 & _position, const cgVector3 & _normal, cgFloat _tu = 0.0f, cgFloat _tv = 0.0f );

    //-------------------------------------------------------------------------
    // Public Variables For This Class
    //-------------------------------------------------------------------------
	cgVector3 position;           // Vertex position
    cgVector3 normal;             // Vertex normal vector
	cgVector3 binormal;           // binormal axis
	cgVector3 tangent;            // tangent axis
    cgVector2 textureCoords[4];   // Texture map coordinates

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static cgInt             TextureCoordCount;
    const static D3DVERTEXELEMENT9 Declarator[9];
};

//-----------------------------------------------------------------------------
//  Name : cgScreenVertex (Class)
/// <summary>
/// Vertex class used to render simple screen-space geometry (i.e.
/// screen-space quad rendered by cgRenderControlPass)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScreenVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors For This Class.
    //-------------------------------------------------------------------------
    cgScreenVertex( ) :
      position( cgVector4( 0, 0, 0, 0 ) ), color( 0xFFFFFFFF ), textureCoords( cgVector2( 0, 0 ) ) {}
    cgScreenVertex( cgFloat _x, cgFloat _y, cgFloat _z, cgFloat _w, cgFloat _tu = 0.0f, cgFloat _tv = 0.0f, cgUInt32 _color = 0xFFFFFFFF ) :
      position( cgVector4( _x, _y, _z, _w ) ), color( _color ), textureCoords( cgVector2( _tu, _tv ) ) {}
    cgScreenVertex( cgFloat _x, cgFloat _y, cgFloat _z, cgFloat _w, cgUInt32 _color ) :
      position( cgVector4( _x, _y, _z, _w ) ), color( _color ), textureCoords( cgVector2( 0, 0 ) ) {}
    cgScreenVertex( const cgVector4 & _position, cgFloat _tu = 0.0f, cgFloat _tv = 0.0f, cgUInt32 _color = 0xFFFFFFFF ) :
      position( _position ), color( _color ), textureCoords( cgVector2( _tu, _tv ) ) {}
    cgScreenVertex( const cgVector4 & _position, cgUInt32 _color ) :
      position( _position ), color( _color ), textureCoords( cgVector2( 0, 0 ) ) {}

    //-------------------------------------------------------------------------
    // Public Variables For This Class
    //-------------------------------------------------------------------------
	cgVector4 position;           // Vertex position
    cgUInt32  color;              // Shade color
    cgVector2 textureCoords;      // Texture map coordinates

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static D3DVERTEXELEMENT9 Declarator[4];
};

//-----------------------------------------------------------------------------
//  Name : cgClipVertex (Class)
/// <summary>
/// Vertex class used to render simple clip-space geometry (i.e.
/// clip-space quad rendered by cgRenderControlPass)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgClipVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors For This Class.
    //-------------------------------------------------------------------------
    cgClipVertex( );
    cgClipVertex( cgFloat _x, cgFloat _y, cgFloat _z, cgFloat _tu = 0.0f, cgFloat _tv = 0.0f );
    cgClipVertex( const cgVector3 & _position, cgFloat _tu = 0.0f, cgFloat _tv = 0.0f );

    //-------------------------------------------------------------------------
    // Public Variables For This Class
    //-------------------------------------------------------------------------
	cgVector3 position;           // Vertex position
    cgVector2 textureCoords;      // Texture map coordinates

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static D3DVERTEXELEMENT9 Declarator[3];
};

//-----------------------------------------------------------------------------
//  Name : cgShadedVertex (Class)
/// <summary>
/// Vertex class used to render simple shaded (colored) geometry.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgShadedVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors For This Class.
    //-------------------------------------------------------------------------
    cgShadedVertex( ) :
        position( 0.0f, 0.0f, 0.0f ),
        color   ( 0xFFFFFFFF ) {}
    cgShadedVertex( cgFloat _x, cgFloat _y, cgFloat _z, cgUInt32 _color = 0xFFFFFFFF ) :
        position( _x, _y, _z ),
        color   ( _color ) {}
    cgShadedVertex( const cgVector3 & _position, cgUInt32 _color = 0xFFFFFFFF ) :
        position( _position ),
        color   ( _color ) {}

    //-------------------------------------------------------------------------
    // Public Variables For This Class
    //-------------------------------------------------------------------------
	cgVector3 position;           // Vertex position
    cgUInt32  color;              // Shade color

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static D3DVERTEXELEMENT9 Declarator[3];
};

//-----------------------------------------------------------------------------
//  Name : cgBillboard3DVertex (Class)
/// <summary>
/// Vertex class used to construct & store billboard vertex components.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBillboard3DVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    cgBillboard3DVertex( ) :
        position( 0.0f, 0.0f, 0.0f ), color( 0xFFFFFFFF ), textureCoords( 0.0f, 0.0f ),
        offset( 0.0f, 0.0f ), angle( 0.0f ), scale( 1.0f, 1.0f ), hdrScale(1.0f), direction( 0.0f, 0.0f, 0.0f ) {};

    cgBillboard3DVertex( cgFloat _x, cgFloat _y, cgFloat _z, cgColorValue _color, cgFloat _u = 0.0f, cgFloat _v = 0.0f ) :
        position( _x, _y, _z ), color ( _color ), textureCoords( _u, _v ),
        offset( 0.0f, 0.0f ), angle( 0.0f ), scale( 1.0f, 1.0f ), hdrScale(1.0f), direction ( 0.0f, 0.0f, 0.0f ) {};
    
    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    cgVector3     position;       // Vertex position
    cgUInt32      color;          // Tint color for the billboard
    cgVector2     textureCoords;  // Texture coordinates
    cgVector2     offset;         // Vertex offset descriptor in relation to the billboard center
    cgFloat       angle;          // Rotation angle for the billboard vertex
    cgVector2     scale;          // Scale factor for the billboard
    cgFloat       hdrScale;       // Scaling factor for the billboard intensity (HDR).
    cgVector3     direction;      // Optional axis to which a billboard's orientation can be aligned

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static D3DVERTEXELEMENT9 Declarator[7];
};

//-----------------------------------------------------------------------------
//  Name : cgBillboard2DVertex (Class)
/// <summary>
/// Vertex class used to construct & store billboard vertex components.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBillboard2DVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    cgBillboard2DVertex( ) :
        position( 0.0f, 0.0f, 0.0f, 0.0f ), color( 0xFFFFFFFF ), textureCoords( 0.0f, 0.0f ) {};

    cgBillboard2DVertex( cgFloat _x, cgFloat _y, cgFloat _z, cgFloat _w, cgUInt32 _color, cgFloat _u = 0.0f, cgFloat _v = 0.0f ) :
        position( _x, _y, _z, _w ), color( _color ), textureCoords( _u, _v ) {};
    
    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    cgVector4 position;       // Vertex position
    cgUInt32  color;          // Tint color for the billboard
    cgVector2 textureCoords;  // Texture coordinates

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static D3DVERTEXELEMENT9 Declarator[4];
};

//-----------------------------------------------------------------------------
// Name : cgPointVertex (Class)
// Desc : Vertex class used to render single points (i.e. point-based rendering)
//-----------------------------------------------------------------------------
class CGE_API cgPointVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors For This Class.
    //-------------------------------------------------------------------------
	cgPointVertex( ) : 
		position( 0, 0, 0 ) {};
	cgPointVertex( cgFloat _x, cgFloat _y, cgFloat _z ) : 
		position( _x, _y, _z ) {};
	cgPointVertex( const cgVector3 & _position ) : 
		position( _position ) {};

    //-------------------------------------------------------------------------
    // Public Variables For This Class
    //-------------------------------------------------------------------------
	cgVector3 position;           // Vertex position

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static D3DVERTEXELEMENT9 Declarator[2];
};

#endif // !_CGE_CGVERTEXFORMATS_H_