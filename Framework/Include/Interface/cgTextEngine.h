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
// Name : cgTextEngine.h                                                     //
//                                                                           //
// Desc : Contains classes which provide support for rendering text to the   //
//        screen using bitmap fonts.                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGTEXTENGINE_H_ )
#define _CGE_CGTEXTENGINE_H_

//-----------------------------------------------------------------------------
// cgTextEngine Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;
class cgBillboardBuffer;
class cgXMLNode;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgFontSet (Class)
/// <summary>
/// Maintains information about each of the possible characters available
/// with any individual font.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFontSet
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgTextEngine;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgFontSet( const cgString & name );
    virtual ~cgFontSet( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    
protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    // Contains the description for an individual character in the font set
    struct CharDesc
    {
        cgUInt16            x;                      // The left position of the character image in the texture.
        cgUInt16            y;                      // The top position of the character image in the texture.
        cgUInt16            width;                  // The width of the character image in the texture.
        cgUInt16            height;                 // The height of the character image in the texture.
        cgInt16             offsetX;                // How much the current position should be offset when copying the image from the texture to the screen.
        cgInt16             offsetY;                // How much the current position should be offset when copying the image from the texture to the screen.         
        cgUInt16            advanceX;               // How much the current position should be advanced after drawing the character.
        cgUInt16            pageId;                 // The texture page where the character image is found.
        cgInt16             frameIndex;             // The index into the billboard buffer's frame array for this character
    
    }; // End Struct CharDesc
    CGE_VECTOR_DECLARE(CharDesc*, CharacterArray)

    // Font data may spill over onto multiple pages. This structure contains the page definition.
    struct FontSetPage
    {
        cgUInt16            pageId;                 // The Id of this page in the font set.
        cgString            texture;                // The texture resource to load for this font page.
        cgBillboardBuffer * billboards;             // The screen space billboards that will represent the individual characters.

        // Constructor
        FontSetPage()
        {
            billboards = CG_NULL;
        
        } // End Constructor

    }; // End Struct FontSetPage
    CGE_VECTOR_DECLARE(FontSetPage*, PageArray)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool    parseFontSet        ( const cgXMLNode & node, const cgString & definitionName );
    bool    initializeFontPages ( cgRenderDriver * renderDriver, cgInputStream shader );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgString        mFontName;      // The name of the font set.
    cgInt32         mFontSize;      // Size of the original font used, in pixels
    cgInt32         mLineHeight;    // height of an individual line of text using this font
    cgInt32         mLineBase;      // The number of pixels from the absolute top of the line to the base of the characters.
    cgInt32         mOutline;       // Size of any outline applied to the font.
    cgInt32         mPageWidth;     // Width of each page texture
    cgInt32         mPageHeight;    // Height of each page texture
    bool            mBold;          // Original font used 'bold' style during generation?
    bool            mItalic;        // Original font used 'italic' style during generation?
    bool            mUnicode;       // Includes unicode data?
    PageArray       mFontPages;     // The page data that contains the actual font image data.
    CharacterArray  mCharacterMap;  // A vector mapping an individual character Id to the physical character data.
};

//-----------------------------------------------------------------------------
//  Name : cgTextMetrics (Class)
/// <summary>
/// Stores information about the piece of text for which metrics were
/// requested via a call to cgTextEngine::computeTextMetrics().
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTextMetrics
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgTextEngine;

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    // Structure containing individual character reference information from the metrics data returned by computeTextMetrics
    struct MetricRef
    {
        cgInt32 line;       // The line index to which the character belongs (-1 = prior to first line, -2 = after last line)
        cgInt32 column;     // The character / column index. (-1 = prior to first char, -2 = after last char)

    }; // End Struct MetricRef

    // Structure containing prepared data for each character in the line
    struct TextChar
    {
        cgUInt16 pageId;
        cgInt16  frameIndex;
        cgInt32  originalChar;
        cgUInt32 color;
        cgRect   displayBounds;
        cgRect   bounds;
    };
    CGE_VECTOR_DECLARE(TextChar, TextCharArray)

    // Structure containing prepared data for each line of text to render
    struct TextLine
    {
        cgRect          bounds;
        cgInt32         firstCharacter;
        cgInt32         lastCharacter;
        TextCharArray   characters;

        void reset()
        {
            bounds.top     = 2147483647;        // INT_MAX
            bounds.left    = 2147483647;        // INT_MAX
            bounds.bottom  = (-2147483647 - 1); // INT_MIN
            bounds.right   = (-2147483647 - 1); // INT_MIN
            firstCharacter = -1;
            lastCharacter  = -1;
            characters.clear();
        }
    };
    CGE_MAP_DECLARE(cgInt32, TextLine, TextLineMap) // ToDo: unordered_map?

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgTextMetrics( );
    ~cgTextMetrics( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    clear                   ( );
    bool                    resolveLocation         ( const MetricRef & location, const TextLine ** lineOut = CG_NULL, const TextChar ** characterOut = CG_NULL ) const;
    cgInt32                 characterFromPoint      ( const cgPoint & position, MetricRef * locationOut = CG_NULL ) const;
    cgInt32                 characterFromLocation   ( const MetricRef & location ) const;
    bool                    characterToLocation     ( cgInt32 characterIndex, MetricRef & locationOut ) const;
    bool                    characterToLocation     ( cgInt32 characterIndex, MetricRef & locationOut, bool & isExactMatchOut ) const;
    cgInt32                 characterToLine         ( cgInt32 characterIndex ) const;
    cgRectArray             computeTextRectangles   ( const MetricRef & firstLocation, const MetricRef & lastLocation ) const;
    const TextLineMap     & getMetrics              ( ) const;
    const TextLine        & getLineMetrics          ( cgInt32 lineIndex ) const;
    cgUInt32                getLineCount            ( ) const;
    cgUInt32                getLineHeight           ( ) const;
    cgInt32                 getFirstVisibleLine     ( ) const;
    cgInt32                 getLastVisibleLine      ( ) const;
    const cgRect          & getFullBounds           ( ) const;
    bool                    isOverflowing           ( ) const;
    bool                    isValidLocation         ( const MetricRef & location ) const;

private:
    //-------------------------------------------------------------------------
    // Private Structures
    //-------------------------------------------------------------------------
    // Stores character references for all lines. Allows for quick lookups of where
    // a character falls after wrapping etc. for all text, not just those visible
    struct LineRange
    {
        cgUInt32 firstCharacter;
        cgUInt32 lastCharacter;
    };
    CGE_VECTOR_DECLARE(LineRange, LineRangeArray)

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgFontSet         * mFontSet;           // The font used to generate this metric structure
    TextLineMap         mTextLines;         // Metrics for each visible line in the text
    LineRangeArray      mLineRanges;        // Stores character start/end formation for each line
    cgInt32             mFirstLine;         // The first line referenced in the above map
    cgInt32             mLastLine;          // The last line referenced in the above map
    bool                mMultiline;         // Used multiline metric computation?
    cgUInt32            mTextLength;        // Total number of characters in the metric
    cgUInt32            mLineHeight;        // The height of each line of text.
    cgUInt32            mLineCount;         // Total number of lines (wrapped) in the text.
    bool                mTextOverflowing;   // Did the text overflow the original rectangle?
    cgRect              mFullBounds;        // The rectangle of the full text as a whole
    cgRect              mVisibleBounds;     // The rectangle of only the visible portion of text.
    cgUInt32            mFlags;             // The flags used when computing the text metrics.
    cgUInt32            mColor;             // The color to use for rendering.
};

//-----------------------------------------------------------------------------
//  Name : cgTextEngine (Class)
/// <summary>
/// Class responsible for managing and rendering the data required in
/// order to represent text drawn to the framebuffer.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTextEngine
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgTextEngine( cgRenderDriver * renderDriver );
    virtual ~cgTextEngine( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgString    addFont             ( cgInputStream definition, cgInputStream shader = _T("") );
    bool        setCurrentFont      ( const cgString & fontName );
    cgRect      printText           ( const cgRect & destination, cgUInt32 flags, const cgString & text );
    cgRect      printText           ( const cgRect & destination, cgUInt32 flags, const cgString & text, const cgPoint & offset );
    cgRect      printText           ( const cgTextMetrics & metrics );
    cgRect      printText           ( const cgTextMetrics & metrics, const cgPoint & offset );
    cgInt32     getLineHeight       ( ) const;
    void        setLineSpacing      ( cgInt32 spacing );
    void        setKerning          ( cgInt32 kerning );
    void        setColor            ( cgUInt32 color );
    cgInt32     getLineSpacing      ( ) const;
    cgInt32     getKerning          ( ) const;
    cgUInt32    getColor            ( ) const;
    bool        computeTextMetrics  ( const cgRect & destination, cgUInt32 flags, const cgString & text, const cgPoint & offset, cgTextMetrics & metricsOut );
    bool        computeTextMetrics  ( const cgRect & destination, cgUInt32 flags, const cgString & text, cgTextMetrics & metricsOut );

    // ToDo: Hook up to system message?
    void        onDeviceLost        ( );
    void        onDeviceReset       ( );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgString, cgFontSet*, FontSetMap )

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool        parseFontSet        ( const cgXMLNode & node, const cgString & definitionName, cgInputStream & shader, cgString & fontName );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgRenderDriver    * mDriver;        // The device being used for managing/rendering the text.
    cgFontSet         * mCurrentFont;   // Currently selected font set
    cgInt32             mKerning;       // Amount of space in pixels to leave between each character column
    cgInt32             mLineSpacing;   // Amount of space in pixels to leave between each line of text drawn
    cgUInt32            mColor;         // The text rendering color currently in use.
    FontSetMap          mFontSets;      // All loaded fonts, sorted by name.
};

#endif // !_CGE_CGTEXTENGINE_H_