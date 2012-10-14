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
// Name : cgTextEngine.cpp                                                   //
//                                                                           //
// Desc : Contains classes which provide support for rendering text to the   //
//        screen using bitmap fonts.                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgTextEngine Module Includes
//-----------------------------------------------------------------------------
#include <Interface/cgTextEngine.h>
#include <Interface/cgUITypes.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgBillboardBuffer.h>
#include <System/cgXML.h>

//-----------------------------------------------------------------------------
// Module Local Variables
//-----------------------------------------------------------------------------
namespace
{
    static const int MaxTextCharacters = 1024;  // The maximum number of characters that can be rendered at any one time.

} // End Unnamed Namespace

///////////////////////////////////////////////////////////////////////////////
// cgTextEngine Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTextEngine () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTextEngine::cgTextEngine( cgRenderDriver * renderDriver )
{
    // Initialize variables to sensible defaults
    mDriver           = renderDriver;
    mCurrentFont      = CG_NULL;
    mKerning          = 0;
    mLineSpacing      = 0;
    mColor            = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : ~cgTextEngine () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTextEngine::~cgTextEngine( )
{
    // Iterate through fonts and release them
    FontSetMap::iterator itFont;
    for ( itFont = mFontSets.begin(); itFont != mFontSets.end(); ++itFont )
        delete itFont->second;
    mFontSets.clear();

    // clear variables
    mCurrentFont  = CG_NULL;
    mDriver       = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : addFont ()
/// <summary>
/// Load a font set into the text engine based on the XML font definition
/// file specified.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgTextEngine::addFont( cgInputStream definition, cgInputStream shader /* = _T("") */ )
{
    // Write debug to log.
    cgAppLog::write( cgAppLog::Debug, _T("Parsing font definition from file '%s'.\n"), definition.getName().c_str() );

    // Open and parse the XML file
    cgXMLDocument document;
    cgXMLError::Result result = document.open( definition, _T("font") );

    // Any error parsing XML file?
    if ( result != cgXMLError::Success )
    {
        if ( result == cgXMLError::NoDocumentTag )
        {
            // Write to the registered output streams
            cgAppLog::write( cgAppLog::Error, _T("%s : Document tag 'font' not found when parsing XML.\n"), definition.getName().c_str() );
        
        } // End if first tag not found
        else
        {
            // Write to the registered output streams
            // ToDo: Support line / column
            cgAppLog::write( cgAppLog::Error, _T("%s(%d,%d) : %s\n"), definition.getName().c_str(),
                             0, 0, cgXMLDocument::getErrorDescription( result ).c_str() );
        
        } // End if parse error

        return _T("");
    
    } // End if failed to parse

    // Attempt to parse the loaded data
    cgString fontName;
    if ( !parseFontSet( document.getDocumentNode(), definition.getName(), shader, fontName ) )
        return cgString::Empty;
    else
        return fontName;
}

//-----------------------------------------------------------------------------
//  Name : parseFontSet () (Private)
/// <summary>
/// Once the XML data has been loaded, this method is responsible for
/// parsing that data and setting up the internal structures based on the
/// information specified in the XML font definition file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextEngine::parseFontSet( const cgXMLNode & node, const cgString & fontDefinition, cgInputStream & shader, cgString & fontName )
{
    // Retrieve the font information
    cgXMLNode child = node.getChildNode( _T("info") );
    if ( child.isEmpty() == true )
    {
        // Write log information and bail
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("%s : No 'info' tag found when parsing font definition XML.\n"), fontDefinition.c_str() );
        return false;
    
    } // End if no info tag
    
    // Get the font face name.
    if ( child.definesAttribute( _T("face") ) == false )
    {
        // Write log information and bail
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("%s : No 'face' attribute found when parsing font definition XML.\n"), fontDefinition.c_str() );
        return false;
    
    } // End if no name attribute

    // Get the name
    fontName = child.getAttributeText( _T("face") );
    cgString fontMapKey = cgString::toLower(fontName);
    
    // Font already loaded?
    if ( mFontSets.find( fontMapKey ) != mFontSets.end() )
    {
        // Write log information and bail
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("%s : A font set with duplicate name '%s' was found when parsing font definition XML.\n"), fontDefinition.c_str(), fontName.c_str() );
        return false;
    
    } // End if no name attribute
    
    // Generate a new font set
    cgFontSet * newFont = new cgFontSet( fontName );
    if ( newFont->parseFontSet( node, fontDefinition ) == false )
    {
        delete newFont;
        return false;
    
    } // End if failed to parse

    // Prepare the font for rendering.
    if ( newFont->initializeFontPages( mDriver, shader ) == false )
    {
        delete newFont;
        return false;
    
    } // End if failed to initialize

    // Add the font to our list of available fonts
    mFontSets[ fontMapKey ] = newFont;

    // Set this as the currently active font
    mCurrentFont = newFont;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setCurrentFont ()
/// <summary>
/// Select the font that the application would like to use the next time
/// 'printText' is called.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextEngine::setCurrentFont( const cgString & fontName )
{
    // Font names are case insensitive
    cgString fontKey = cgString::toLower( fontName );

    // Find the font in our loaded list of fonts
    FontSetMap::iterator itFont = mFontSets.find( fontKey );
    if ( itFont == mFontSets.end() )
        return false;

    // Set this as the currently active font
    mCurrentFont = itFont->second;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setLineSpacing ()
/// <summary>
/// Set the spacing inbetween each line of text in pixels.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextEngine::setLineSpacing( cgInt32 spacing )
{
    mLineSpacing = spacing;
}

//-----------------------------------------------------------------------------
//  Name : setKerning ()
/// <summary>
/// Set the spacing in between each (column of) characters in pixels.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextEngine::setKerning( cgInt32 kerning )
{
    mKerning = kerning;
}

//-----------------------------------------------------------------------------
//  Name : setColor ()
/// <summary>
/// Set the color to use for rendering text.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextEngine::setColor( cgUInt32 color )
{
    mColor = color;
}

//-----------------------------------------------------------------------------
//  Name : getLineSpacing ()
/// <summary>
/// Get the currently configured spacing inbetween each line of text in pixels.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgTextEngine::getLineSpacing( ) const
{
    return mLineSpacing;
}

//-----------------------------------------------------------------------------
//  Name : getKerning ()
/// <summary>
/// Get the currently configured spacing in between each (column of) characters 
/// in pixels.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgTextEngine::getKerning( ) const
{
    return mKerning;
}

//-----------------------------------------------------------------------------
//  Name : getColor ()
/// <summary>
/// Get the currently configured color to use for rendering text.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTextEngine::getColor( ) const
{
    return mColor;
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgTextEngine::printText( const cgRect & destination, cgUInt32 flags, const cgString & text )
{
    return printText( destination, flags, text, cgPoint(0,0) );
}

//-----------------------------------------------------------------------------
//  Name : printText () (Overload)
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload allows you to specify an offset, in pixels, to offset
/// the text relative to the specified rectangle.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgTextEngine::printText( const cgRect & destination, cgUInt32 flags, const cgString & text, const cgPoint & offset )
{
    // Validate parameters
    if ( text.empty() || !mCurrentFont || !mDriver )
        return cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1) );

    // Process the text and retrieve all of the offset and frame information
    // for each character.
    cgTextMetrics metrics;
    if ( !computeTextMetrics( destination, flags, text, offset, metrics ) )
        return cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1) );

    // Pass through to metric rendering method
    return printText( metrics );
}

//-----------------------------------------------------------------------------
//  Name : printText () (Overload)
/// <summary>
/// Draw text from a pre-computed text metrics cache.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgTextEngine::printText( const cgTextMetrics & metrics )
{
    return printText( metrics, cgPoint(0,0) );
}

//-----------------------------------------------------------------------------
//  Name : printText () (Overload)
/// <summary>
/// Draw text from a pre-computed text metrics cache.
/// Note : This overload allows you to reposition the computed text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgTextEngine::printText( const cgTextMetrics & metrics, const cgPoint & offset )
{
    cgToDo( "User Interface", "Scissor rectangle enable state is currently hardcoded in Billboards.sh::billboard2D technique.. Optionally enable/disable this behavior?" );
    
    // TODO: Clean the static array allocation up.
    cgUInt16 pageInfo[ 100 ];
    memset( pageInfo, 0, 100 * sizeof(cgUInt16) );

    // Reset the result rectangle (INT_MAX, INT_MIN)
    cgRect result = cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1));

    // Extract area rectangle from metrics item and reposition if necessary
    cgRect destination = metrics.mVisibleBounds;
    destination += offset;
    
    // Enable clipping if requested
    bool clipping = ((metrics.mFlags & cgTextFlags::ClipRectangle) == cgTextFlags::ClipRectangle);
    if ( clipping && mDriver )
    {
        // Expand for outline if any.
        cgRect scissorRect = cgRect::inflate( destination, metrics.mFontSet->mOutline, metrics.mFontSet->mOutline );
        mDriver->pushScissorRect( &scissorRect );

    } // End if clipping
    
    // For each line of text
    cgTextMetrics::TextLineMap::const_iterator itLine;
    for ( itLine = metrics.mTextLines.begin(); itLine != metrics.mTextLines.end(); ++itLine )
    {
        const cgTextMetrics::TextLine * line = &itLine->second;

        // Grow drawn rect
        if ( line->bounds.left   < result.left   ) result.left   = line->bounds.left;
        if ( line->bounds.top    < result.top    ) result.top    = line->bounds.top;
        if ( line->bounds.right  > result.right  ) result.right  = line->bounds.right;
        if ( line->bounds.bottom > result.bottom ) result.bottom = line->bounds.bottom;
        
        // For each character in the line
        for ( size_t i = 0, characterCount = line->characters.size(); i < characterCount; ++i )
        {
            const cgTextMetrics::TextChar * character = &line->characters[i];

            // Get the page that this character references
            cgFontSet::FontSetPage * fontPage = metrics.mFontSet->mFontPages[ character->pageId ];

            // Retrieve the billboard from the buffer
            cgBillboard2D * billboard = (cgBillboard2D*)fontPage->billboards->getBillboard( pageInfo[ character->pageId ] );

            // position and size the billboard correctly
            billboard->setPosition( (cgFloat)character->displayBounds.left + offset.x, (cgFloat)character->displayBounds.top + offset.y, 0 );
            billboard->setSize( (cgFloat)(character->displayBounds.right - character->displayBounds.left), (cgFloat)(character->displayBounds.bottom - character->displayBounds.top) );
            billboard->setColor( character->color );
            billboard->setFrame( character->frameIndex );
            billboard->update();

            // We've processed a billboard in this page
            pageInfo[ character->pageId ]++;

            // If we reached our capacity for the page billboard buffer, render it
            if ( pageInfo[ character->pageId ] == MaxTextCharacters )
            {
                fontPage->billboards->render( 0, -1 );
                pageInfo[ character->pageId ] = 0;

            } // End if at capacity

        } // Next character

    } // Next line

    // Iterate through each of the pages and draw whatever remains
    for ( size_t i = 0; i < 100; ++i )
    {
        if ( pageInfo[i] > 0 )
        {
            cgFontSet::FontSetPage * fontPage = metrics.mFontSet->mFontPages[i];
            fontPage->billboards->render( 0, pageInfo[i] );
            pageInfo[i] = 0;

        } // End if any data to render

    } // Next Page

    // Offset the drawn rectangle to final position
    result += offset;

    // Disable clipping
    if ( clipping && mDriver )
    {
        mDriver->popScissorRect( );

        // Also clip the drawn rect to the clipping rectangle
        result = cgRect::intersect( destination, result );

    } // End if clipping enabled

    // Return drawn rectangle
    return result;
}

//-----------------------------------------------------------------------------
//  Name : computeTextMetrics () (Private Overload)
/// <summary>
/// Compute the position and size information for each line and character
/// for the specified piece of text. This will also adjust all of the
/// character rectangles to take into account the required alignment.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextEngine::computeTextMetrics( const cgRect & destination, cgUInt32 flags, const cgString & text, cgTextMetrics & metricsOut )
{
    return computeTextMetrics( destination, flags, text, cgPoint(0,0), metricsOut );
}

//-----------------------------------------------------------------------------
//  Name : computeTextMetrics () (Private)
/// <summary>
/// Compute the position and size information for each line and character
/// for the specified piece of text. This will also adjust all of the
/// character rectangles to take into account the required alignment.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextEngine::computeTextMetrics( const cgRect & destination, cgUInt32 flags, const cgString & text, const cgPoint & offset, cgTextMetrics & metrics )
{
    // We cannot process if we have no selected font
    if ( !mCurrentFont )
        return false;

    // Format modifiers
    std::stack<cgUInt32> colorStack;
    colorStack.push( mColor );

    // Clear the metrics structure (release previously allocated memory)
    metrics.clear();

    // Multiline requested?
    bool multiline = ( (flags & cgTextFlags::Multiline) == cgTextFlags::Multiline );

    // Set up metric properties
    metrics.mFontSet         = mCurrentFont;
    metrics.mMultiline       = multiline;
    metrics.mTextLength      = (cgUInt32)text.length();
    metrics.mLineHeight      = getLineHeight();
    metrics.mTextOverflowing = false;
    metrics.mVisibleBounds   = destination;
    metrics.mFlags           = flags;
    metrics.mColor           = mColor;

    // Character and line rectangles start at offset to begin with
    // actual alignment computations (i.e. left/right align) will happen separately.
    cgTextMetrics::TextLine wrapLine, currentLine;
    const cgInt width    = destination.width();
    const cgInt height   = destination.height();
    cgInt currentX = offset.x;
    cgInt currentY = offset.y;
    currentLine.reset();
    currentLine.firstCharacter = 0;

    // Iterate through each character in the text buffer and process
    // the character data.
    cgInt previousSpace = -1;
    for ( size_t i = 0, textLength = text.length(); i < textLength; ++i )
    {
        // Retrieve character
        cgTChar c = text.at(i);

        // Is this the start of a code block (i.e. [c=0])?
        if ( (flags & cgTextFlags::AllowFormatCode) && c == _T('[') && (i+1) < textLength )
        {
            size_t codeBlockStart = i + 1;
            
            // Find the end of the code block.
            size_t codeBlockEnd = text.find( _T(']'), codeBlockStart );
            if ( codeBlockEnd != cgString::npos )
            {
                cgString codeBlock = cgString::toLower( text.substr( codeBlockStart, (codeBlockEnd - codeBlockStart) ) );
                if ( codeBlock.size() > 2 && codeBlock.beginsWith( _T("c=") ) )
                {
                    // Hex color?
                    if ( codeBlock.size() > 3 && codeBlock.at(2) == _T('#') )
                    {
                        cgUInt32 color;
                        cgStringParser parser( codeBlock.substr( 3 ) );
                        parser >> std::hex >> color;
                        colorStack.push( color );

                    } // End if hex color

                    // Skip block now.
                    i = codeBlockEnd; // +1 but loop will ++i
                    continue;

                } // End if open color
                else if ( codeBlock.size() == 2 && codeBlock == _T("/c") )
                {
                    if ( colorStack.size() > 1 )
                        colorStack.pop();

                    // Skip block now.
                    i = codeBlockEnd; // +1 but loop will ++i
                    continue;
                
                } // End if close color

            } // End if !eof

        } // End if '['

        // Invalid or out of range character?
        if ( c < 0 )
        {
            // For the moment, substitute unknown character with a question mark
            c = _T('?');
        
        } // End if out of range

        // Ignore all carriage returns
        if ( c == _T('\r') )
            continue;

        // Is this line entirely clipped away?
        // TODO: Only update when currentY changes
        bool lineClipped        = ( (currentY + (cgInt32)metrics.mLineHeight) < 0 || currentY > height );
        bool linePartialClipped = ( currentY < 0 || (currentY + (cgInt32)metrics.mLineHeight) > height );

        // Text is to be considered overflowing if any line (or part of the line) was clipped away
        metrics.mTextOverflowing |= linePartialClipped;

        // line feeds move down in multiline mode
        if ( multiline && c == _T('\n') )
        {
            // Mark the previous line's ending character INCLUDING the newline.
            currentLine.lastCharacter = i;

            // Compute line rectangle
            currentLine.bounds = cgRect( offset.x, currentY, offset.x, currentY + getLineHeight() );
            for ( size_t j = 0, characterCount = currentLine.characters.size(); j < characterCount; ++j )
            {
                cgTextMetrics::TextChar * character = &currentLine.characters[j];

                // Add character bounds to line rectangles
                if ( character->bounds.left   < currentLine.bounds.left   ) currentLine.bounds.left   = character->bounds.left;
                if ( character->bounds.top    < currentLine.bounds.top    ) currentLine.bounds.top    = character->bounds.top;
                if ( character->bounds.right  > currentLine.bounds.right  ) currentLine.bounds.right  = character->bounds.right;
                if ( character->bounds.bottom > currentLine.bounds.bottom ) currentLine.bounds.bottom = character->bounds.bottom;
            
            } // Next character

            // Grow the entire text rectangle by this line
            if ( currentLine.bounds.left   < metrics.mFullBounds.left   ) metrics.mFullBounds.left   = currentLine.bounds.left;
            if ( currentLine.bounds.top    < metrics.mFullBounds.top    ) metrics.mFullBounds.top    = currentLine.bounds.top;
            if ( currentLine.bounds.right  > metrics.mFullBounds.right  ) metrics.mFullBounds.right  = currentLine.bounds.right;
            if ( currentLine.bounds.bottom > metrics.mFullBounds.bottom ) metrics.mFullBounds.bottom = currentLine.bounds.bottom;

            // If the line wasn't clipped, add it to the metrics
            if ( !lineClipped )
            {
                if ( metrics.mFirstLine < 0 )
                    metrics.mFirstLine = metrics.mLineCount;
                metrics.mLastLine = metrics.mLineCount;
                metrics.mTextLines[metrics.mLineCount] = currentLine;
            
            } // End if not clipped

            // Always add the line range whether clipped or otherwise
            cgTextMetrics::LineRange range;
            range.firstCharacter = currentLine.firstCharacter;
            range.lastCharacter  = currentLine.lastCharacter;
            metrics.mLineRanges.push_back( range );
            
            // Move down to the next line
            currentX  = offset.x;
            currentY += getLineHeight();
            metrics.mLineCount++;
            
            // Reset the previous line storage, and mark the starting character of this line
            currentLine.reset();
            currentLine.firstCharacter = i + 1;
            
            // No space so far on this line
            previousSpace = -1;
            continue;

        } // End if linefeed character

        // Retrieve the descriptor for this character
        const cgFontSet::CharDesc * characterDesc = mCurrentFont->mCharacterMap[ c ];
        
        // If no descriptor is available for this character, skip it
        if ( !characterDesc )
        {
            // For the moment, substitute character with a question mark
            c = _T('?');
            characterDesc = mCurrentFont->mCharacterMap[ c ];
        
        } // End if character not found

        // Is this character entirely clipped away?
        bool characterClipped = ( (currentX + characterDesc->offsetX + characterDesc->width) < 0 || 
                                  (currentX + characterDesc->offsetX) > width );

        // *******************************************************************************************
        // * Word Wrapping
        // *******************************************************************************************

        // Do we need to break onto a new line?
        if ( multiline && ((currentX + characterDesc->offsetX + characterDesc->width) > width) )
        {
            // Wrapping onto next line, first make a backup of the current line data
            // and then reset the current line storage
            wrapLine = currentLine;
            currentLine.reset();

            // Store currentX and currentY as they should be set after wrapping 
            // (These values will potentially be adjusted in the code below)
            cgUInt32 wrapCurrentX = offset.x;
            cgUInt32 wrapCurrentY = currentY + getLineHeight();

            // We need to copy any characters from the last space to the new line
            // if there was a space at all on this line.
            if ( previousSpace >= 0 )
            {
                // Do we need to copy any characters over when we wrap?
                if ( previousSpace + 1 < (signed)i )
                {
                    // Copy previous characters onto the new line
                    for ( size_t j = previousSpace + 1, characterCount = wrapLine.characters.size(); j < characterCount; ++j )
                    {
                        // Set the line's starting character to first character we add
                        if ( currentLine.firstCharacter == -1 )
                            currentLine.firstCharacter = wrapLine.characters[j].originalChar;

                        // Copy character over
                        currentLine.characters.push_back( wrapLine.characters[j] );
                        cgTextMetrics::TextChar * character = &currentLine.characters[ currentLine.characters.size() - 1 ];
                        
                        // Adjust the rectangles
                        character->displayBounds.left   -= character->bounds.left;
                        character->displayBounds.left   += wrapCurrentX;
                        character->displayBounds.right  -= character->bounds.left;
                        character->displayBounds.right  += wrapCurrentX;
                        character->displayBounds.top    -= character->bounds.top;
                        character->displayBounds.top    += wrapCurrentY;
                        character->displayBounds.bottom -= character->bounds.top;
                        character->displayBounds.bottom += wrapCurrentY;

                        character->bounds.right  -= character->bounds.left;
                        character->bounds.right  += wrapCurrentX;
                        character->bounds.bottom -= character->bounds.top;
                        character->bounds.bottom += wrapCurrentY;
                        character->bounds.left    = wrapCurrentX;
                        character->bounds.top     = wrapCurrentY;

                        // Next character
                        wrapCurrentX += character->bounds.right - character->bounds.left;
                    
                    } // Next character

                } // End if characters copied
                else
                {
                    // The start of the next line is just the character we're about to insert.
                    currentLine.firstCharacter = i;

                } // End if no characters copied

                // The end of the current line includes everything up to and including
                // the space we wrapped the line with.
                wrapLine.lastCharacter = wrapLine.characters[ previousSpace ].originalChar;

                // Remove these characters from the previous line (INCLUDING the space)
                wrapLine.characters.erase( wrapLine.characters.begin() + previousSpace, wrapLine.characters.end() );

            } // End if space inserted
            else
            {
                // Set the current line's end, and next line's start accordingly
                wrapLine.lastCharacter = i - 1;
                currentLine.firstCharacter  = i;
            
            } // End if no space available

            // Compute line rectangle
            wrapLine.bounds = cgRect( offset.x, currentY, offset.x, currentY + getLineHeight() );
            for ( size_t j = 0, characterCount = wrapLine.characters.size(); j < characterCount; ++j )
            {
                cgTextMetrics::TextChar * character = &wrapLine.characters[j];

                // Add character bounds to line rectangles
                if ( character->bounds.left   < wrapLine.bounds.left   ) wrapLine.bounds.left   = character->bounds.left;
                if ( character->bounds.top    < wrapLine.bounds.top    ) wrapLine.bounds.top    = character->bounds.top;
                if ( character->bounds.right  > wrapLine.bounds.right  ) wrapLine.bounds.right  = character->bounds.right;
                if ( character->bounds.bottom > wrapLine.bounds.bottom ) wrapLine.bounds.bottom = character->bounds.bottom;
            
            } // Next character

            // Grow the entire text rectangle by this line
            if ( wrapLine.bounds.left   < metrics.mFullBounds.left   ) metrics.mFullBounds.left   = wrapLine.bounds.left;
            if ( wrapLine.bounds.top    < metrics.mFullBounds.top    ) metrics.mFullBounds.top    = wrapLine.bounds.top;
            if ( wrapLine.bounds.right  > metrics.mFullBounds.right  ) metrics.mFullBounds.right  = wrapLine.bounds.right;
            if ( wrapLine.bounds.bottom > metrics.mFullBounds.bottom ) metrics.mFullBounds.bottom = wrapLine.bounds.bottom;

            // If the line wasn't clipped, add it to the metrics
            if ( !lineClipped )
            {
                if ( metrics.mFirstLine < 0 )
                    metrics.mFirstLine = metrics.mLineCount;
                metrics.mLastLine = metrics.mLineCount;
                metrics.mTextLines[metrics.mLineCount] = wrapLine;
            
            } // End if line not clipped

            // Always add the line range whether clipped or otherwise
            cgTextMetrics::LineRange range;
            range.firstCharacter = wrapLine.firstCharacter;
            range.lastCharacter  = wrapLine.lastCharacter;
            metrics.mLineRanges.push_back( range );

            // A line was added, move down to the next line
            currentX = wrapCurrentX;
            currentY = wrapCurrentY;
            metrics.mLineCount++;
                        
            // No space so far on this line
            previousSpace = -1;

        } // End if breaking onto new line

        // *******************************************************************************************
        // * End of Word Wrapping
        // *******************************************************************************************

        // Mark the location of the most recent space
        if ( c == _T(' ') )
            previousSpace = (cgInt)currentLine.characters.size();

        // Add another character to the line
        currentLine.characters.resize( currentLine.characters.size() + 1 );
        cgTextMetrics::TextChar * character = &currentLine.characters[ currentLine.characters.size() - 1 ];

        // Populate the character fields
        character->frameIndex    = characterDesc->frameIndex;
        character->pageId        = characterDesc->pageId;
        character->originalChar  = i;
        character->color         = colorStack.top();
        character->bounds.left   = currentX;
        character->bounds.top    = currentY;
        character->bounds.right  = currentX + characterDesc->advanceX + mKerning;
        character->bounds.bottom = currentY + getLineHeight();
        character->displayBounds.left   = currentX + characterDesc->offsetX;
        character->displayBounds.top    = currentY + characterDesc->offsetY;
        character->displayBounds.right  = character->displayBounds.left + characterDesc->width;
        character->displayBounds.bottom = character->displayBounds.top + characterDesc->height;

        // Update the current 'cursor' position
        currentX += characterDesc->advanceX + mKerning;

    } // Next character

    // The end of the current line is simply the total number of characters
    // included in the string.
    currentLine.lastCharacter = metrics.mTextLength;

    // Compute last line rectangle
    currentLine.bounds = cgRect( offset.x, currentY, offset.x, currentY + getLineHeight() );
    for ( size_t j = 0, characterCount = currentLine.characters.size(); j < characterCount; ++j )
    {
        cgTextMetrics::TextChar * character = &currentLine.characters[j];

        // Add character bounds to line rectangles
        if ( character->bounds.left   < currentLine.bounds.left   ) currentLine.bounds.left   = character->bounds.left;
        if ( character->bounds.top    < currentLine.bounds.top    ) currentLine.bounds.top    = character->bounds.top;
        if ( character->bounds.right  > currentLine.bounds.right  ) currentLine.bounds.right  = character->bounds.right;
        if ( character->bounds.bottom > currentLine.bounds.bottom ) currentLine.bounds.bottom = character->bounds.bottom;
    
    } // Next character

    // Grow the entire text rectangle by this line
    if ( currentLine.bounds.left   < metrics.mFullBounds.left   ) metrics.mFullBounds.left   = currentLine.bounds.left;
    if ( currentLine.bounds.top    < metrics.mFullBounds.top    ) metrics.mFullBounds.top    = currentLine.bounds.top;
    if ( currentLine.bounds.right  > metrics.mFullBounds.right  ) metrics.mFullBounds.right  = currentLine.bounds.right;
    if ( currentLine.bounds.bottom > metrics.mFullBounds.bottom ) metrics.mFullBounds.bottom = currentLine.bounds.bottom;

    // Add this line if it wasn't clipped
    if ( (currentY + (cgInt32)metrics.mLineHeight) >= 0 && currentY <= height )
    {
        if ( metrics.mFirstLine < 0 )
            metrics.mFirstLine = metrics.mLineCount;
        metrics.mLastLine = metrics.mLineCount;
        metrics.mTextLines[ metrics.mLineCount ] = currentLine;
    
    } // End if line not clipped
    metrics.mLineCount++;

    // Always add the line range whether clipped or otherwise
    cgTextMetrics::LineRange range;
    range.firstCharacter = currentLine.firstCharacter;
    range.lastCharacter  = currentLine.lastCharacter;
    metrics.mLineRanges.push_back( range );

    // Now we must align the text lines to the specified rectangle.
    cgPoint origin( destination.left, destination.top );
    cgTextMetrics::TextLineMap::iterator itLine = metrics.mTextLines.begin();
    for ( ; itLine != metrics.mTextLines.end(); ++itLine )
    {
        // Get the current line
        cgTextMetrics::TextLine * line = &itLine->second;

        // Horizontal alignment
        if ( (flags & cgTextFlags::AlignCenter) == cgTextFlags::AlignCenter )
            origin.x = destination.left + ((width / 2) - ((line->bounds.right - line->bounds.left) / 2));
        else if ( (flags & cgTextFlags::AlignRight) == cgTextFlags::AlignRight )
            origin.x = destination.right - (line->bounds.right - line->bounds.left);

        // Vertical alignment
        if ( (flags & cgTextFlags::VAlignCenter) == cgTextFlags::VAlignCenter )
            origin.y = destination.top + ((height / 2) - ((metrics.mFullBounds.bottom - metrics.mFullBounds.top) / 2));
        else if ( (flags & cgTextFlags::VAlignBottom) == cgTextFlags::VAlignBottom )
            origin.y = destination.bottom - metrics.mFullBounds.bottom;

        // Adjust the line rectangles
        line->bounds += origin;

        // Adjust the character rectangles too
        for ( size_t j = 0, characterCount = line->characters.size(); j < characterCount; ++j )
        {
            line->characters[j].displayBounds += origin;
            line->characters[j].bounds += origin;

        } // Next character

    } // Next line

    // Finally shift full metrics to the final position based on the draw rectangle (instead of being relative to 0,0).
    metrics.mFullBounds += origin;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getLineHeight ()
/// <summary>
/// Get the current height of each line of text ( in pixels ) based
/// on the currently set font.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgTextEngine::getLineHeight( ) const
{
    // Validate requirements
    if ( mCurrentFont == CG_NULL )
        return mLineSpacing;

    // Return current line height
    return mCurrentFont->mLineHeight + mLineSpacing;
}

///////////////////////////////////////////////////////////////////////////////
// cgFontSet Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgFontSet () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFontSet::cgFontSet( const cgString & name )
{
    // Initialize variables to sensible defaults
    mFontName     = name;
    mFontSize     = 0;
    mLineHeight   = 0;
    mLineBase     = 0;
    mOutline      = 0;
    mBold         = false;
    mItalic       = false;
    mUnicode      = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgFontSet () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFontSet::~cgFontSet( )
{
    // Iterate through all registered characters
    CharacterArray::iterator itChar;
    for ( itChar = mCharacterMap.begin(); itChar != mCharacterMap.end(); ++itChar )
        delete *itChar;
    mCharacterMap.clear();

    // Iterate through all registered pages
    PageArray::iterator      itPage;
    for ( itPage = mFontPages.begin(); itPage != mFontPages.end(); ++itPage )
    {
        FontSetPage * pPage = *itPage;
        
        // Release the billboard buffer
        if ( pPage ) 
            delete pPage->billboards;

        // Delete the page structure itself
        delete pPage;
    
    } // Next character
    mFontPages.clear();
}

//-----------------------------------------------------------------------------
//  Name : parseFontSet () (Private)
/// <summary>
/// Once the XML data has been loaded, this method is responsible for
/// parsing that data and setting up the internal structures based on the
/// information specified in the XML font definition file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFontSet::parseFontSet( const cgXMLNode & node, const cgString & fontDefinition )
{
    /*cgUInt16        pageCount, loadedPages, characterId;
    CharDesc      * pNewChar = CG_NULL, character;
    FontSetPage   * pNewPage = CG_NULL, Page;
    cgXMLNode       childNode, pagesNode, charactersNode, kerningsNode;
    cgString        value;*/

    try
    {
        // Retrieve the font information
        cgXMLNode childNode = node.getChildNode( _T("info") );
        if ( childNode.isEmpty() == true )
            throw( _T("No 'info' tag found") );
        
        // Retrieve the original font properties
        cgString value;
        if ( childNode.getAttributeText( _T("size"), value ) )
            cgStringParser( value ) >> mFontSize;
        if ( childNode.getAttributeText( _T("bold"), value ) )
            cgStringParser( value ) >> mBold;
        if ( childNode.getAttributeText( _T("italic"), value ) )
            cgStringParser( value ) >> mItalic;
        if ( childNode.getAttributeText( _T("unicode"), value ) )
            cgStringParser( value ) >> mUnicode;
        if ( childNode.getAttributeText( _T("outline"), value ) )
            cgStringParser( value ) >> mOutline;
        
        // Retrieve the common information
        childNode = node.getChildNode( _T("common") );
        if ( childNode.isEmpty() )
            throw( _T("No 'common' tag found") );
        if ( !childNode.getAttributeText( _T("lineHeight"), value ) )
            throw( _T("No 'lineHeight' attribute found") );
        cgStringParser( value ) >> mLineHeight;
        if ( !childNode.getAttributeText( _T("base"), value ) )
            throw( _T("No 'base' attribute found") );
        cgStringParser( value ) >> mLineBase;
        if ( !childNode.getAttributeText( _T("scaleW"), value ) )
            throw( _T("No 'scaleW' attribute found") );
        cgStringParser( value ) >> mPageWidth;
        if ( !childNode.getAttributeText( _T("scaleH"), value ) )
            throw( _T("No 'scaleH' attribute found") );
        cgStringParser( value ) >> mPageHeight;
        
        // Determine how many font pages there are
        if ( !childNode.getAttributeText( _T("pages"), value ) )
            throw( _T("No page count attribute found within common declaration") );
        cgInt32 pageCount;
        cgStringParser( value ) >> pageCount;
        if ( pageCount == 0 )
            throw( _T("Invalid page count specified within common declaration") );

        // Reserve enough page structures in the font set
        mFontPages.resize( pageCount );

        // Now find all childNode 'page' nodes
        cgInt32 loadedPages = 0;
        cgXMLNode pagesNode = node.getChildNode( _T("pages") );
        for ( cgUInt32 i = 0; ; )
        {
            // Retrieve the next childNode page node
            childNode = pagesNode.getNextChildNode( _T("page"), i );
            if ( childNode.isEmpty() )
                break;

            // Must have an 'id' attribute            
            if ( !childNode.getAttributeText( _T("id"), value ) )
                continue;
            FontSetPage page;
            cgStringParser( value ) >> page.pageId;
            if ( page.pageId >= mFontPages.size() )
                continue;

            // Must have a 'file' attribute
            if ( !childNode.getAttributeText( _T("file"), page.texture ) )
                continue;
            
            // Build a new character type and add this to the font set's page map
            mFontPages[page.pageId] = new FontSetPage( page );
            loadedPages++;

        } // Next available page node

        // No pages loaded?
        if ( loadedPages != pageCount )
            throw( _T("Not all page data was found, or loaded correctly") );

        // Next, all childNode 'char' nodes
        cgXMLNode charactersNode = node.getChildNode( _T("chars") );
        for ( cgUInt32 i = 0; ; )
        {
            // Retrieve the next childNode character node
            childNode = charactersNode.getNextChildNode( _T("char"), i );
            if ( childNode.isEmpty() )
                break;

            // Get 'id' attribute
            if ( !childNode.getAttributeText( _T("id"), value ) )
                continue;
            cgInt32 characterId;
            cgStringParser( value ) >> characterId;

            // Get 'x' attribute
            if ( !childNode.getAttributeText( _T("x"), value ) )
                continue;
            CharDesc character;
            cgStringParser( value ) >> character.x;

            // Get 'y' attribute
            if ( !childNode.getAttributeText( _T("y"), value ) )
                continue;
            cgStringParser( value ) >> character.y;

            // Get 'width' attribute
            if ( !childNode.getAttributeText( _T("width"), value ) )
                continue;
            cgStringParser( value ) >> character.width;

            // Get 'height' attribute
            if ( !childNode.getAttributeText( _T("height"), value ) )
                continue;
            cgStringParser( value ) >> character.height;

            // Get 'xoffset' attribute
            if ( !childNode.getAttributeText( _T("xoffset"), value ) )
                continue;
            cgStringParser( value ) >> character.offsetX;

            // Get 'yoffset' attribute
            if ( !childNode.getAttributeText( _T("yoffset"), value ) )
                continue;
            cgStringParser( value ) >> character.offsetY;

            // Get 'xadvance' attribute
            if ( !childNode.getAttributeText( _T("xadvance"), value ) )
                continue;
            cgStringParser( value ) >> character.advanceX;

            // Get 'page' attribute
            if ( !childNode.getAttributeText( _T("page"), value ) )
                continue;
            cgStringParser( value ) >> character.pageId;

            // Resize the character map array if required
            if ( (characterId + 1) > (cgUInt16)mCharacterMap.size() )
                mCharacterMap.resize( characterId + 1 );

            // Add this to the character map
            mCharacterMap[ characterId ] = new CharDesc( character );

        } // Next available character node

        // No character data loaded?
        if ( mCharacterMap.size() == 0 )
            throw( _T("No character information was found") );

        // Finally, all childNode 'kerning' nodes
        cgXMLNode kerningsNode = node.getChildNode( _T("kernings") );
        for ( cgUInt32 i = 0; ; )
        {
            // Retrieve the next childNode kerning node
            childNode = kerningsNode.getNextChildNode( _T("kerning"), i );
            if ( childNode.isEmpty() == true )
                break;

            // TODO : Retrieve multi-character kerning information.

        } // Next available kerning node

    } // End Try Block

    catch ( const cgTChar * error )
    {
        // Write log information and bail
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("%s : %s when parsing font definition XML.\n"), fontDefinition.c_str(), error );
        return false;

    } // End Catch Block

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initializeFontPages () (Private)
/// <summary>
/// With the font set XML fully parsed, and the complete font description
/// now available, this is called by the text engine in order to construct
/// the font data ready for display / rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFontSet::initializeFontPages( cgRenderDriver * renderDriver, cgInputStream shader )
{
    /*cgBillboardBuffer         * buffer;
    cgRect                      frameBounds;
    
    CharacterArray::iterator    itChar;
    cgInt32                     i, nPageIndex = 0;
    bool                        bResult;*/

    // Iterate through all registered font pages
    PageArray::iterator itPage;
    for ( itPage = mFontPages.begin(); itPage!= mFontPages.end(); ++itPage )
    {
        FontSetPage * page = *itPage;
        if ( !page ) continue;

        // Allocate the billboard buffer for this page
        cgBillboardBuffer * buffer = new cgBillboardBuffer();

        // Prepare to populate the billboard buffer
        if ( !buffer->prepareBuffer( cgBillboardBuffer::ScreenSpace, renderDriver, page->texture, shader ) )
        {
            // Write log information
            cgAppLog::write( cgAppLog::Error, _T("Failed to begin preparing billboard buffer when initializing font '%s'.\n"), mFontName.c_str() );
            
            // Clean up and bail
            delete buffer;
            return false;
        
        } // End if failed to begin buffer preparation

        // Allocate enough 2d billboards to render up to our maximum
        // number of allow characters.
        for ( cgUInt32 i = 0; i < MaxTextCharacters; ++i )
        {
            cgBillboard2D * billboard = new cgBillboard2D();
            if ( buffer->addBillboard( billboard ) < 0 )
            {
                // Write log information
                cgAppLog::write( cgAppLog::Error, _T("Failed to create character billboard %i of %i when initializing font '%s'.\n"), i + 1, MaxTextCharacters, mFontName.c_str() );

                // Clean up and bail
                delete billboard;
                delete buffer;
                return false;
            
            } // End if failed

        } // Next character

        // Now iterate through all character descriptors that link to this page
        // and add their rectangle to the buffer's frame data.
        CharacterArray::iterator itChar;
        for ( itChar = mCharacterMap.begin(); itChar != mCharacterMap.end(); ++itChar )
        {
            CharDesc * character = *itChar;
            if ( !character || character->pageId != page->pageId )
                continue;

            // Build the rectangle that describes the correct portion of the texture
            // for this character.
            cgRect frameBounds;
            frameBounds.left   = character->x;
            frameBounds.right  = character->x + character->width;
            frameBounds.top    = character->y;
            frameBounds.bottom = character->y + character->height;

            // Add the frame to the billboard buffer
            character->frameIndex = buffer->addFrame( 0, frameBounds );

        } // Next character

        // We're finished preparing the buffer
        if ( buffer->endPrepare() == false )
        {
            // Write log information
            cgAppLog::write( cgAppLog::Error, _T("Failed to complete preparation of billboard buffer when initializing font '%s'.\n"), mFontName.c_str() );
            
            // Clean up and bail
            delete buffer;
            return false;
        
        } // End if failed to complete buffer preparation

        // Store the billboard buffer.
        delete page->billboards;
        page->billboards = buffer;
    
    } // Next Page

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgTextMetrics Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTextMetrics () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTextMetrics::cgTextMetrics(  )
{
    // Initialize variables to sensible defaults
    mFontSet          = CG_NULL;
    mMultiline        = false;
    mFirstLine        = -10; // Something out of range of special line codes
    mLastLine         = -10; // Something out of range of special line codes
    mTextLength       = 0;
    mLineHeight       = 0;
    mLineCount        = 0;
    mFlags            = 0;
    mColor            = 0xFFFFFFFF;
    mTextOverflowing  = false;
    mVisibleBounds    = cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1));
    mFullBounds       = cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1));
}

//-----------------------------------------------------------------------------
//  Name : ~cgTextMetrics () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTextMetrics::~cgTextMetrics( )
{
    clear();
}

//-----------------------------------------------------------------------------
//  Name : clear ()
/// <summary>
/// Release memory and clear all computed metrics.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextMetrics::clear()
{
    // Clear vectors
    mTextLines.clear();
    mLineRanges.clear();

    // Clear values
    mFontSet          = CG_NULL;
    mFirstLine        = -10; // Something out of range of special line codes
    mLastLine         = -10; // Something out of range of special line codes
    mMultiline        = false;
    mTextLength       = 0;
    mLineHeight       = 0;
    mLineCount        = 0;
    mFlags            = 0;
    mColor            = 0xFFFFFFFF;
    mTextOverflowing  = false;
    mVisibleBounds    = cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1));
    mFullBounds       = cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1));
}

//-----------------------------------------------------------------------------
//  Name : resolveLocation ()
/// <summary>
/// Given a reference to a specific character, resolve and return the
/// correct TextLine and TextChar objects.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextMetrics::resolveLocation( const MetricRef & location, const TextLine ** lineOut /* = CG_NULL*/, const TextChar ** characterOut /* = CG_NULL */ ) const
{
    const TextChar * character = CG_NULL;
    const TextLine * line = CG_NULL;

    // Be polite and clear variables
    if ( lineOut )
        *lineOut = CG_NULL;
    if ( characterOut )
        *characterOut = CG_NULL;

    // Not a valid reference?
    if ( !isValidLocation( location ) )
        return false;

    // Unable to resolve prior and after codes
    if ( location.line == -1 || location.line == -2 )
        return false;

    // Retrieve the correct line
    TextLineMap::const_iterator itLine = mTextLines.find( location.line );
    line = &itLine->second;

    // If character is in range, return that.
    if ( location.column >= 0 && location.column < (cgInt32)line->characters.size() )
        character = &line->characters[ location.column ];

    // Pass back if requested
    if ( lineOut )
        *lineOut = line;
    if ( characterOut )
        *characterOut = character;

    // Resolved
    return true;
}

//-----------------------------------------------------------------------------
//  Name : isValidLocation ()
/// <summary>
/// Determine if the specified reference actually references a valid
/// entry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextMetrics::isValidLocation( const MetricRef & location ) const
{
    // Special line codes ('prior' or 'after' reference?)
    if ( location.line == -1 || location.line == -2 )
        return true;
    
    // Exists in line map?
    TextLineMap::const_iterator itLine = mTextLines.find( location.line );
    if ( itLine == mTextLines.end() )
        return false;

    // Character reference is valid?
    return ( location.column == -1 || location.column == -2 || ( location.column >= 0 && location.column < (cgInt32)itLine->second.characters.size() ) );
}

//-----------------------------------------------------------------------------
//  Name : getMetrics ()
/// <summary>
/// Retrieve the computed metric array.
/// </summary>
//-----------------------------------------------------------------------------
const cgTextMetrics::TextLineMap & cgTextMetrics::getMetrics( ) const
{
    return mTextLines;
}

//-----------------------------------------------------------------------------
//  Name : getFullBounds ()
/// <summary>
/// Retrieve the rectangle describing the entire text.
/// </summary>
//-----------------------------------------------------------------------------
const cgRect & cgTextMetrics::getFullBounds( ) const
{
    return mFullBounds;
}

//-----------------------------------------------------------------------------
//  Name : getLineCount ()
/// <summary>
/// Retrieve the total number of lines (after wrapping) in the text.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTextMetrics::getLineCount( ) const
{
    return mLineCount;
}

//-----------------------------------------------------------------------------
//  Name : getLineMetrics ()
/// <summary>
/// Retrieve the line metrics for the specified line index.
/// </summary>
//-----------------------------------------------------------------------------
const cgTextMetrics::TextLine & cgTextMetrics::getLineMetrics( cgInt32 lineIndex ) const
{
    TextLineMap::const_iterator itLine = mTextLines.find( lineIndex );
    return itLine->second;
}

//-----------------------------------------------------------------------------
//  Name : getLineHeight ()
/// <summary>
/// Retrieve the height of each line of text.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTextMetrics::getLineHeight( ) const
{
    return mLineHeight;
}

//-----------------------------------------------------------------------------
//  Name : isOverflowing ()
/// <summary>
/// Did the text overflow the original specified rectangle?
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextMetrics::isOverflowing( ) const
{
    return mTextOverflowing;
}

//-----------------------------------------------------------------------------
//  Name : characterFromLocation ()
/// <summary>
/// Resolve the metric reference into a text character index.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgTextMetrics::characterFromLocation( const MetricRef & location ) const
{
    // Process prior / after coded line references
    MetricRef testReference = location;
    if ( testReference.line == -1 ) { testReference.line = mFirstLine; testReference.column = -1; }
    if ( testReference.line == -2 ) { testReference.line = mLastLine; testReference.column = -2; }

    // Valid metric reference?
    if ( isValidLocation( testReference ) == false )
        return -1;

    // Retrieve the line itself
    const TextLine * line = &getLineMetrics( testReference.line );
  
    // Process prior / after coded character references
    if ( testReference.column == -2 )
        testReference.column = (cgInt32)line->characters.size(); // (Last index + 1 to denote after character)

    // Character in range?
    if ( testReference.column >= (cgInt32)line->characters.size() )
        return line->lastCharacter;
    else if ( testReference.column <= 0 )
        return line->firstCharacter;

    // Return the original character index
    return line->characters[ testReference.column ].originalChar;
}

//-----------------------------------------------------------------------------
//  Name : characterFromPoint ()
/// <summary>
/// Given a position that falls within the text, compute the character
/// over which this point is located (and return a metric reference if
/// requested).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgTextMetrics::characterFromPoint( const cgPoint & position, MetricRef * locationOut /* = CG_NULL */ ) const
{
    // Clear structures
    MetricRef location;
    memset( &location, 0, sizeof( MetricRef ) );
    if ( locationOut )
        *locationOut = location;

    // If either the first or last line indices are invalid, we're out of luck... bail
    if ( mFirstLine < 0 || mLastLine < 0 )
        return -1;

    // Retrieve the first and last lines
    const TextLine & firstLine = getLineMetrics( mFirstLine );
    const TextLine & lastLine  = getLineMetrics( mLastLine );

    // Above entire text?
    if ( position.y < mFullBounds.top )
    {
        // Above the first line in the text as a whole. Return the first line data.
        location.line   = mFirstLine;
        location.column = -1;

        // Copy metric reference over if requested
        if ( locationOut )
            *locationOut = location;

        // Return character index
        return firstLine.firstCharacter;

    } // End if above first line
    else if ( position.y > mFullBounds.bottom )
    {
        // Below the last line in the text as a whole. Return the last line data.
        location.line   = mLastLine;
        location.column = -2;

        // Copy metric reference over if requested
        if ( locationOut )
            *locationOut = location;

        // Return character index
        return lastLine.lastCharacter;
    
    } // End if below last line
    
    // Above first line?
    if ( position.y < firstLine.bounds.top )
    {
        // Above the first line in the visible text, return the 'prior' metric reference
        location.line   = -1;
        location.column = -1;

        // Copy metric reference over if requested
        if ( locationOut )
            *locationOut = location;

        // Return character index
        return firstLine.firstCharacter;

    } // End if above first line
    else if ( position.y > lastLine.bounds.bottom )
    {
        // Below the last line in the visible text, return the 'after' metric reference
        location.line   = -2;
        location.column = -2;

        // Copy metric reference over if requested
        if ( locationOut )
            *locationOut = location;

        // Return character index
        return lastLine.lastCharacter;
    
    } // End if below last line
    
    // Iterate through each line
    cgInt32 characterIndex = -1;
    TextLineMap::const_iterator itLine;
    for ( itLine = mTextLines.begin(); itLine != mTextLines.end(); ++itLine )
    {
        const TextLine & line = itLine->second;

        // First, are we within the line rectangle?
        if ( mMultiline == true && (position.y < line.bounds.top || position.y > line.bounds.bottom) )
            continue;

        // Check early out bounding box properties
        if ( line.characters.size() == 0 )
        {
            // Just select first character, there is nothing on this line
            characterIndex  = line.firstCharacter;
            location.line   = itLine->first;
            location.column = -2;
        
        } // End if no characters
        else if ( position.x < line.bounds.left )
        {
            // To the left of the first char
            characterIndex  = line.characters[0].originalChar;
            location.line   = itLine->first;
            location.column = -1;
        
        } // End if left of first char
        else if ( position.x > line.bounds.right )
        {
            // To the right of the last char
            characterIndex  = line.characters[ line.characters.size() - 1 ].originalChar + 1;
            location.line   = itLine->first;
            location.column = -2;
        
        } // End if below last line
        else
        {
            // We are within the line rectangle bounds, check each character
            for ( size_t i = 0, characterCount = line.characters.size(); i < characterCount; ++i )
            {
                const TextChar & character = line.characters[i];

                // Are we within the char rectangle?
                if ( position.x >= character.bounds.left && position.x <= character.bounds.right )
                {
                    // We're within the character rect
                    characterIndex  = character.originalChar;
                    location.line   = itLine->first;
                    location.column = (cgInt32)i;

                    // Over half way across the rectangle?
                    if ( position.x > (character.bounds.left + character.bounds.right) / 2 )
                    {
                        characterIndex++;

                        // Last character?
                        if ( i == line.characters.size() - 1 )
                            location.column = -2;
                        else
                            location.column++;

                    } // End if over half way
                    break;

                } // End if within bounds

            } // Next character

        } // End if

        // Selected a character?
        if ( characterIndex >= 0 )
            break;

    } // Next line

    // Copy metric reference over if requested
    if ( locationOut )
        *locationOut = location;
    
    // Return selected character index
    return characterIndex;
}

//-----------------------------------------------------------------------------
//  Name : characterToLine ()
/// <summary>
/// Given a character index, this method will return the line on which
/// it fell (after wrapping etc.)
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgTextMetrics::characterToLine( cgInt32 characterIndex ) const
{
    for ( size_t i = 0; i < mLineRanges.size(); ++i )
    {
        const LineRange & range = mLineRanges[i];
        if ( characterIndex >= (signed)range.firstCharacter && characterIndex <= (signed)range.lastCharacter )
            return (cgInt32)i;

    } // Next line
    return -1;
}

//-----------------------------------------------------------------------------
//  Name : characterToLocation () (Overload)
/// <summary>
/// Overload for standard characterToLocation method that does not
/// require a isExactMatchOut output value parameter.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextMetrics::characterToLocation( cgInt32 characterIndex, MetricRef & locationOut ) const
{
    bool isExactMatch;
    return characterToLocation( characterIndex, locationOut, isExactMatch );
}

//-----------------------------------------------------------------------------
//  Name : characterToLocation ()
/// <summary>
/// Given a locationOut of a character in the text, compute a metric
/// reference for that specific character.
/// Note : The character may not actually exist any more, and as a result this
/// method will attempt to find the closest surviving character (if any).
/// As a result the method will return true via the isExactMatchOut parameter
/// if it found an exact match, or false if it returned only the closest 
/// match.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextMetrics::characterToLocation( cgInt32 characterIndex, MetricRef & locationOut, bool & isExactMatchOut ) const
{
    // Reset output values
    isExactMatchOut = false;
    memset( &locationOut, 0, sizeof(MetricRef) );

    // If either the first or last line indices are invalid, we're out of luck... bail
    if ( mFirstLine < 0 || mLastLine < 0 )
        return false;

    // Retrieve the first and last lines
    const TextLine & firstLine = getLineMetrics( mFirstLine );
    const TextLine & lastLine  = getLineMetrics( mLastLine );
    
    // Prior to first line?
    if ( characterIndex < firstLine.firstCharacter )
    {
        locationOut.line   = -1;
        locationOut.column = -1;
        return true;
    
    } // End if prior to first line

    // After last line?
    if ( characterIndex > lastLine.lastCharacter )
    {
        locationOut.line   = -2;
        locationOut.column = -2;
        return true;
    
    } // End if after last line

    // Iterate through each line
    TextLineMap::const_iterator itLine;
    for ( itLine = mTextLines.begin(); itLine != mTextLines.end(); ++itLine )
    {
        const TextLine & line = itLine->second;

        // Skip if the character was not part of this line when we processed the text.
        if ( characterIndex < line.firstCharacter || characterIndex > line.lastCharacter )
            continue;

        // If there are no characters here, or it came prior to the first physical
        // character, just return the closest line result.
        if ( line.characters.size() == 0 || characterIndex < line.characters[0].originalChar )
        {
            locationOut.line   = itLine->first;
            locationOut.column = -1;
            return true;

        } // End if no characters found

        // For each character on the line
        for ( size_t i = 0, characterCount = line.characters.size(); i < characterCount; ++i )
        {
            const TextChar & character = line.characters[i];
            
            // Did the character fall between the current and next TextChar items?
            if ( character.originalChar == characterIndex )
            {
                // If this is exactly the character we're looking for
                // just return it.
                locationOut.line   = itLine->first;
                locationOut.column = i;
                isExactMatchOut    = true;
                return true;

            } // End if exact character match with first

            // Also test to see if it falls between this and the next character
            if ( i < line.characters.size() - 1)
            {
                const TextChar & nextCharacter = line.characters[i + 1];
                if ( characterIndex > character.originalChar && characterIndex < nextCharacter.originalChar )
                {
                    // Character did fall between these two, but was not specifically
                    // inserted into the metric for rendering. Return a closest match.
                    locationOut.line   = itLine->first;
                    locationOut.column = i + 1;
                    return true;

                } // End if falls between the two characters

            } // End if additional characters remain

        } // Next character

        // If we got here, then the character was actually part of this line but came after
        // the last inserted character. Return this fact.
        locationOut.line   = itLine->first;
        locationOut.column = -2;
        return true;

    } // Next line

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : computeTextRectangles ()
/// <summary>
/// Compute the list of rectangles which bound all of the characters
/// referenced between the start and end metric references.
/// </summary>
//-----------------------------------------------------------------------------
cgRectArray cgTextMetrics::computeTextRectangles( const MetricRef & firstLocation, const MetricRef & lastLocation ) const
{
    cgRectArray rectangles;

    // Duplicate the references, we may modify them
    MetricRef startRef = firstLocation;
    MetricRef endRef   = lastLocation;
    
    // Process prior / after coded line references
    if ( startRef.line == -1 ) { startRef.line = mFirstLine; startRef.column = -1; }
    if ( startRef.line == -2 ) { startRef.line = mLastLine; startRef.column = -2; }
    if ( endRef.line == -1 ) { endRef.line = mFirstLine; endRef.column = -1; }
    if ( endRef.line == -2 ) { endRef.line = mLastLine; endRef.column = -2; }
    
    // Valid references?
    if ( !isValidLocation( startRef ) || !isValidLocation( endRef ) )
        return rectangles;

    // Process prior / after coded character references
    if ( startRef.column == -1 ) startRef.column = 0;
    if ( startRef.column == -2 ) startRef.column = (cgInt32)getLineMetrics( startRef.line ).characters.size(); // (Last index + 1 to denote after character)
    if ( endRef.column == -1 ) endRef.column = 0;
    if ( endRef.column == -2 ) endRef.column = (cgInt32)getLineMetrics( endRef.line ).characters.size(); // (Last index + 1 to denote after character)
    
    // Single line range, or spans multiple lines?
    if ( startRef.line == endRef.line )
    {
        const TextLine * line = &getLineMetrics( startRef.line );

        // Compute character range
        cgInt32 startChar = startRef.column;
        cgInt32 characterCount = endRef.column - startChar;

        // If selecting full line, just return the line's 'slot'
        if ( characterCount >= (cgInt32)line->characters.size() )
        {
            rectangles.push_back( line->bounds );
            return rectangles;
        
        } // End if full line

        // If there are no full characters to select, just return the degenerate 
        // rectangle that falls at the start of the starting character (useful for caret's etc.).
        // Otherwise, compute the full text rectangle.
        cgRect textBounds = cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1));
        if ( characterCount == 0 )
        {
            // Valid character? Or before / after char.
            if ( startChar >= 0 && startChar < (cgInt32)line->characters.size())
            {
                const cgRect * characterBounds = &line->characters[startChar].bounds;
                if ( characterBounds->left < textBounds.left   ) textBounds.left    = characterBounds->left;
                if ( characterBounds->top  < textBounds.top    ) textBounds.top     = characterBounds->top;
                if ( characterBounds->left > textBounds.right  ) textBounds.right   = characterBounds->left;
                if ( characterBounds->top  > textBounds.bottom ) textBounds.bottom  = characterBounds->top;
                if ( characterBounds->bottom < textBounds.top    ) textBounds.top     = characterBounds->bottom;
                if ( characterBounds->bottom > textBounds.bottom ) textBounds.bottom  = characterBounds->bottom;
        
            } // End if valid character
            else
            {
                const cgRect * lineBounds = &line->bounds;
                if ( lineBounds->right  < textBounds.left   ) textBounds.left    = lineBounds->right;
                if ( lineBounds->bottom < textBounds.top    ) textBounds.top     = lineBounds->bottom;
                if ( lineBounds->right  > textBounds.right  ) textBounds.right   = lineBounds->right;
                if ( lineBounds->bottom > textBounds.bottom ) textBounds.bottom  = lineBounds->bottom;
                if ( lineBounds->top    < textBounds.top    ) textBounds.top     = lineBounds->top;
                if ( lineBounds->top    > textBounds.bottom ) textBounds.bottom  = lineBounds->top;
            
            } // End if after end

            // Add to rectangle list
            rectangles.push_back( textBounds );

        } // End if no full range
        else
        {
            // Build text rectangle
            for ( cgInt32 i = startChar; i < startChar + characterCount; ++i )
            {
                // Valid character? Or before / after char.
                if ( i >= 0 && i < (cgInt32)line->characters.size())
                {
                    const cgRect * characterBounds = &line->characters[i].bounds;
                    if ( characterBounds->left < textBounds.left   ) textBounds.left    = characterBounds->left;
                    if ( characterBounds->top  < textBounds.top    ) textBounds.top     = characterBounds->top;
                    if ( characterBounds->left > textBounds.right  ) textBounds.right   = characterBounds->left;
                    if ( characterBounds->top  > textBounds.bottom ) textBounds.bottom  = characterBounds->top;
                    if ( characterBounds->right  < textBounds.left   ) textBounds.left    = characterBounds->right;
                    if ( characterBounds->bottom < textBounds.top    ) textBounds.top     = characterBounds->bottom;
                    if ( characterBounds->right  > textBounds.right  ) textBounds.right   = characterBounds->right;
                    if ( characterBounds->bottom > textBounds.bottom ) textBounds.bottom  = characterBounds->bottom;
            
                } // End if valid character
                else if ( i < 0 )
                {
                    const cgRect * lineBounds = &line->bounds;
                    if ( lineBounds->left < textBounds.left   ) textBounds.left    = lineBounds->left;
                    if ( lineBounds->top  < textBounds.top    ) textBounds.top     = lineBounds->top;
                    if ( lineBounds->left > textBounds.right  ) textBounds.right   = lineBounds->left;
                    if ( lineBounds->top  > textBounds.bottom ) textBounds.bottom  = lineBounds->top;
                    if ( lineBounds->bottom < textBounds.top    ) textBounds.top     = lineBounds->bottom;
                    if ( lineBounds->bottom > textBounds.bottom ) textBounds.bottom  = lineBounds->bottom;
                
                } // End if prior to start
                else
                {
                    const cgRect * lineBounds = &line->bounds;
                    if ( lineBounds->right  < textBounds.left   ) textBounds.left    = lineBounds->right;
                    if ( lineBounds->bottom < textBounds.top    ) textBounds.top     = lineBounds->bottom;
                    if ( lineBounds->right  > textBounds.right  ) textBounds.right   = lineBounds->right;
                    if ( lineBounds->bottom > textBounds.bottom ) textBounds.bottom  = lineBounds->bottom;
                    if ( lineBounds->top    < textBounds.top    ) textBounds.top     = lineBounds->top;
                    if ( lineBounds->top    > textBounds.bottom ) textBounds.bottom  = lineBounds->top;

                } // End if after end
            
            } // Next character

            // Add to rectangle list
            rectangles.push_back( textBounds );

        } // End if valid character range

        // Just return the single line rectangle
        return rectangles;

    } // End if single line
    else
    {
        // Compute character range for start line
        const TextLine * line = &getLineMetrics( startRef.line );
        cgInt32 startChar = startRef.column;

        // Whole line?
        if ( startChar == 0 )
        {
            // Just add the whole line rectangle
            rectangles.push_back( line->bounds );
        
        } // End if whole line
        else
        {
             // Otherwise we need to select character ranges
            cgRect textBounds = cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1));
            for ( cgInt32 i = startChar; i < (cgInt32)line->characters.size(); ++i )
            {
                const cgRect * characterBounds = &line->characters[i].bounds;
                if ( characterBounds->left < textBounds.left   ) textBounds.left    = characterBounds->left;
                if ( characterBounds->top  < textBounds.top    ) textBounds.top     = characterBounds->top;
                if ( characterBounds->left > textBounds.right  ) textBounds.right   = characterBounds->left;
                if ( characterBounds->top  > textBounds.bottom ) textBounds.bottom  = characterBounds->top;
                if ( characterBounds->right  < textBounds.left   ) textBounds.left    = characterBounds->right;
                if ( characterBounds->bottom < textBounds.top    ) textBounds.top     = characterBounds->bottom;
                if ( characterBounds->right  > textBounds.right  ) textBounds.right   = characterBounds->right;
                if ( characterBounds->bottom > textBounds.bottom ) textBounds.bottom  = characterBounds->bottom;
            
            } // Next character
            rectangles.push_back( textBounds );

        } // Not whole line

        // Select full rectangles for each line in the selected range 
        // BETWEEN the start and end lines (which are processed manually)
        for ( cgInt32 i = startRef.line + 1; i < endRef.line; ++i )
        {
            // Add the line's slot rectangle
            line = &getLineMetrics(  i );
            rectangles.push_back( line->bounds );

        } // Next line

        // Compute character range for end line
        line = &getLineMetrics(  endRef.line );
        cgInt32 characterCount = endRef.column;
        
        // Whole line?
        if ( characterCount >= (cgInt32)line->characters.size() )
        {
            // Just add the whole line rectangle
            rectangles.push_back( line->bounds );
        
        } // End if whole line
        else
        {
             // Otherwise we need to select character ranges
            cgRect textBounds = cgRect( 2147483647, 2147483647, (-2147483647-1), (-2147483647-1));
            for ( cgInt32 i = 0; i < characterCount; ++i )
            {
                const cgRect * characterBounds = &line->characters[i].bounds;
                if ( characterBounds->left < textBounds.left   ) textBounds.left    = characterBounds->left;
                if ( characterBounds->top  < textBounds.top    ) textBounds.top     = characterBounds->top;
                if ( characterBounds->left > textBounds.right  ) textBounds.right   = characterBounds->left;
                if ( characterBounds->top  > textBounds.bottom ) textBounds.bottom  = characterBounds->top;
                if ( characterBounds->right  < textBounds.left   ) textBounds.left    = characterBounds->right;
                if ( characterBounds->bottom < textBounds.top    ) textBounds.top     = characterBounds->bottom;
                if ( characterBounds->right  > textBounds.right  ) textBounds.right   = characterBounds->right;
                if ( characterBounds->bottom > textBounds.bottom ) textBounds.bottom  = characterBounds->bottom;
            
            } // Next character
            rectangles.push_back( textBounds );

        } // Not whole line

        // Return resulting vector
        return rectangles;

    } // End if multiple lines
}