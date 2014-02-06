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
// Name : cgUISkin.cpp                                                       //
//                                                                           //
// Desc : Contains classes resposible for the loading, processing and        //
//        management of the definition data for interface skins. A skin      //
//        is a collection of image and layout data which allows users to     //
//        customize the look and feel of the interfaces rendered within this //
//        system.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgUISkin Module Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUISkin.h>
#include <Interface/cgUILayers.h>
#include <Rendering/cgBillboardBuffer.h>
#include <Resources/cgTexture.h>
#include <System/cgImage.h>
#include <System/cgCursor.h>
#include <System/cgStringUtility.h>
#include <System/cgXML.h>

///////////////////////////////////////////////////////////////////////////////
// cgUISkin Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgUISkin () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUISkin::cgUISkin( )
{
    // Initialize variables to sensible defaults
    mSkinName       = _T("");
    mTextureFile    = _T("");
    mGlyphAtlas     = _T("");
    
    // Reset structures
    mOverlappedFormStyle.reset();
    mCursorDefinition.Reset();
    mControlConfig.reset();

    // Populate the valid elements list
    mValidFormElements.push_back( _T("Form.BorderTopLeft") );
    mValidFormElements.push_back( _T("Form.BorderTopBegin") );
    mValidFormElements.push_back( _T("Form.BorderTopFiller") );
    mValidFormElements.push_back( _T("Form.BorderTopEnd") );
    mValidFormElements.push_back( _T("Form.BorderTopRight") );
    mValidFormElements.push_back( _T("Form.BorderLeftBegin") );
    mValidFormElements.push_back( _T("Form.BorderLeftFiller") );
    mValidFormElements.push_back( _T("Form.BorderLeftEnd") );
    mValidFormElements.push_back( _T("Form.BorderRightBegin") );
    mValidFormElements.push_back( _T("Form.BorderRightFiller") );
    mValidFormElements.push_back( _T("Form.BorderRightEnd") );
    mValidFormElements.push_back( _T("Form.BorderBottomLeft") );
    mValidFormElements.push_back( _T("Form.BorderBottomBegin") );
    mValidFormElements.push_back( _T("Form.BorderBottomFiller") );
    mValidFormElements.push_back( _T("Form.BorderBottomEnd") );
    mValidFormElements.push_back( _T("Form.BorderBottomRight") );
    mValidFormElements.push_back( _T("Form.Background") );
    mValidFormElements.push_back( _T("Form.ButtonClose") );
    mValidFormElements.push_back( _T("Form.ButtonStandaloneClose") );
    mValidFormElements.push_back( _T("Form.ButtonMinimize") );
    mValidFormElements.push_back( _T("Form.ButtonMaximize") );
    mValidFormElements.push_back( _T("Form.ButtonRestore") );

    // Button elements
    mValidControlElements.push_back( _T("Button.BorderTopLeft") );
    mValidControlElements.push_back( _T("Button.BorderTopBegin") );
    mValidControlElements.push_back( _T("Button.BorderTopFiller") );
    mValidControlElements.push_back( _T("Button.BorderTopEnd") );
    mValidControlElements.push_back( _T("Button.BorderTopRight") );
    mValidControlElements.push_back( _T("Button.BorderLeftBegin") );
    mValidControlElements.push_back( _T("Button.BorderLeftFiller") );
    mValidControlElements.push_back( _T("Button.BorderLeftEnd") );
    mValidControlElements.push_back( _T("Button.BorderRightBegin") );
    mValidControlElements.push_back( _T("Button.BorderRightFiller") );
    mValidControlElements.push_back( _T("Button.BorderRightEnd") );
    mValidControlElements.push_back( _T("Button.BorderBottomLeft") );
    mValidControlElements.push_back( _T("Button.BorderBottomBegin") );
    mValidControlElements.push_back( _T("Button.BorderBottomFiller") );
    mValidControlElements.push_back( _T("Button.BorderBottomEnd") );
    mValidControlElements.push_back( _T("Button.BorderBottomRight") );
    mValidControlElements.push_back( _T("Button.Background") );

    // CheckBox elements
    mValidControlElements.push_back( _T("CheckBoxChecked") );
    mValidControlElements.push_back( _T("CheckBoxUnchecked") );

    // ControlFrame elements
    mValidControlElements.push_back( _T("ControlFrame.BorderTopLeft") );
    mValidControlElements.push_back( _T("ControlFrame.BorderTopBegin") );
    mValidControlElements.push_back( _T("ControlFrame.BorderTopFiller") );
    mValidControlElements.push_back( _T("ControlFrame.BorderTopEnd") );
    mValidControlElements.push_back( _T("ControlFrame.BorderTopRight") );
    mValidControlElements.push_back( _T("ControlFrame.BorderLeftBegin") );
    mValidControlElements.push_back( _T("ControlFrame.BorderLeftFiller") );
    mValidControlElements.push_back( _T("ControlFrame.BorderLeftEnd") );
    mValidControlElements.push_back( _T("ControlFrame.BorderRightBegin") );
    mValidControlElements.push_back( _T("ControlFrame.BorderRightFiller") );
    mValidControlElements.push_back( _T("ControlFrame.BorderRightEnd") );
    mValidControlElements.push_back( _T("ControlFrame.BorderBottomLeft") );
    mValidControlElements.push_back( _T("ControlFrame.BorderBottomBegin") );
    mValidControlElements.push_back( _T("ControlFrame.BorderBottomFiller") );
    mValidControlElements.push_back( _T("ControlFrame.BorderBottomEnd") );
    mValidControlElements.push_back( _T("ControlFrame.BorderBottomRight") );
    mValidControlElements.push_back( _T("ControlFrame.Background") );

    // GroupFrame elements
    mValidControlElements.push_back( _T("GroupFrame.BorderTopLeft") );
    mValidControlElements.push_back( _T("GroupFrame.BorderTopBegin") );
    mValidControlElements.push_back( _T("GroupFrame.BorderTopFiller") );
    mValidControlElements.push_back( _T("GroupFrame.BorderTopEnd") );
    mValidControlElements.push_back( _T("GroupFrame.BorderTopRight") );
    mValidControlElements.push_back( _T("GroupFrame.BorderLeftBegin") );
    mValidControlElements.push_back( _T("GroupFrame.BorderLeftFiller") );
    mValidControlElements.push_back( _T("GroupFrame.BorderLeftEnd") );
    mValidControlElements.push_back( _T("GroupFrame.BorderRightBegin") );
    mValidControlElements.push_back( _T("GroupFrame.BorderRightFiller") );
    mValidControlElements.push_back( _T("GroupFrame.BorderRightEnd") );
    mValidControlElements.push_back( _T("GroupFrame.BorderBottomLeft") );
    mValidControlElements.push_back( _T("GroupFrame.BorderBottomBegin") );
    mValidControlElements.push_back( _T("GroupFrame.BorderBottomFiller") );
    mValidControlElements.push_back( _T("GroupFrame.BorderBottomEnd") );
    mValidControlElements.push_back( _T("GroupFrame.BorderBottomRight") );
    mValidControlElements.push_back( _T("GroupFrame.Background") );

    // ScrollBarFrame.Vertical elements
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderTopLeft") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderTopBegin") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderTopFiller") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderTopEnd") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderTopRight") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderLeftBegin") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderLeftFiller") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderLeftEnd") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderRightBegin") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderRightFiller") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderRightEnd") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderBottomLeft") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderBottomBegin") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderBottomFiller") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderBottomEnd") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.BorderBottomRight") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Vertical.Background") );

    // ScrollBarFrame.Horizontal elements
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderTopLeft") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderTopBegin") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderTopFiller") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderTopEnd") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderTopRight") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderLeftBegin") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderLeftFiller") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderLeftEnd") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderRightBegin") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderRightFiller") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderRightEnd") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderBottomLeft") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderBottomBegin") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderBottomFiller") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderBottomEnd") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.BorderBottomRight") );
    mValidControlElements.push_back( _T("ScrollBarFrame.Horizontal.Background") );
}

//-----------------------------------------------------------------------------
//  Name : ~cgUISkin () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUISkin::~cgUISkin()
{
    // Release any allocated cursors.
    cgUICursorType::Map::iterator itType;
    cgUICursorType::Map & Types = mCursorDefinition.types;
    for ( itType = Types.begin(); itType != Types.end(); ++itType )
    {
        cgUICursorType & Type = itType->second;
        for ( size_t i = 0; i < Type.platformCursors.size(); ++i )
        {
            cgCursor * pCursor = Type.platformCursors[i];
            if ( pCursor )
                pCursor->removeReference( CG_NULL );
        
        } // Next cursor

    } // Next cursor type
}

//-----------------------------------------------------------------------------
//  Name : loadDefinition()
/// <summary>
/// Load and parse the skin definition XML file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUISkin::loadDefinition( cgInputStream Definition )
{
    cgXMLError::Result  Result;
    cgXMLDocument       xmlDocument;

    // Write debug to log.
    cgAppLog::write( cgAppLog::Debug, _T("Parsing skin definition from file '%s'.\n"), Definition.getName().c_str() );

    // Open and parse the XML file
    Result = xmlDocument.open( Definition, _T("SkinDefinition") );

    // Any error parsing XML file?
    if ( Result != cgXMLError::Success )
    {
        if ( Result == cgXMLError::NoDocumentTag )
        {
            // Write to the registered output streams
            cgAppLog::write( cgAppLog::Error, _T("%s : Document tag 'SkinDefinition' not found when parsing XML.\n"), Definition.getName().c_str() );
        
        } // End if first tag not found
        else
        {
            // Write to the registered output streams
            // ToDo: Support line / column
            cgAppLog::write( cgAppLog::Error, _T("%s(%d,%d) : %s\n"), Definition.getName().c_str(),
                             0, 0, cgXMLDocument::getErrorDescription( Result ).c_str() );
        
        } // End if parse error

        return false;
    
    } // End if failed to parse

    // Attempt to parse the loaded data
    if ( !parseSkinDefinition( xmlDocument.getDocumentNode(), Definition.getName() ) )
        return false;

    // Process any region information loaded from the skin
    processRegions( );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseSkinDefinition () (Private)
/// <summary>
/// Once the XML data has been loaded, this method is responsible for
/// parsing that data and setting up the internal structures based on the
/// information specified in the XML font definition file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUISkin::parseSkinDefinition( const cgXMLNode & xNode, const cgString & strDefinitionFile )
{
    cgXMLNode xChild;
    
    try
    {
        // Retrieve the skin name
        xChild = xNode.getChildNode( _T("Name") );
        if ( xChild.isEmpty() ) throw _T("No 'Name' tag found");
        mSkinName = xChild.getText();

        // Retrieve the glyph image atlas to use for the skin (relative to the definition file).
        xChild = xNode.getChildNode( _T("Glyphs") );
        // ToDo: Test that this is reliable.
        cgString baseDirectory = cgFileSystem::getDirectoryName( strDefinitionFile );
        if ( !xChild.isEmpty() && !xChild.getText().empty() )
            mGlyphAtlas = baseDirectory + _T("/") + xChild.getText();

        // Retrieve the texture to use for the skin (relative to the definition file).
        xChild = xNode.getChildNode( _T("Texture") );
        if ( xChild.isEmpty() ) throw _T("No 'Texture' tag found");
        // ToDo: Test that this is reliable.
        mTextureFile = baseDirectory + _T("/") + xChild.getText();

        // Retrieve the cursor to use for the skin
        xChild = xNode.getChildNode( _T("Cursor") );
        if ( xChild.isEmpty() ) throw _T("No 'Cursor' tag found");
        parseCursor( xChild, baseDirectory, mCursorDefinition );

        // Retrieve the 'form' child and parse it
        xChild = xNode.getChildNode( _T("Form") );
        if ( xChild.isEmpty() ) throw _T("No 'Form' tag found");
        parseForm( xChild );

        // Retrieve the 'controls' child and parse it
        xChild = xNode.getChildNode( _T("Controls") );
        if ( xChild.isEmpty() ) throw _T("No 'Controls' tag found");
        parseControls( xChild, mControlElements, mControlConfig );

    } // End Try Block

    catch ( const cgString & error )
    {
        // Write log information and bail
        cgAppLog::write( cgAppLog::Error, _T("%s : %s when parsing skin definition XML.\n"), strDefinitionFile.c_str(), error.c_str() );
        return false;

    } // End Catch Block
    catch ( const cgTChar * error )
    {
        // Write log information and bail
        cgAppLog::write( cgAppLog::Error, _T("%s : %s when parsing skin definition XML.\n"), strDefinitionFile.c_str(), error );
        return false;

    } // End Catch Block

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseForm () (Private)
/// <summary>
/// Parses all chidren of the 'Form' node.
/// </summary>
//-----------------------------------------------------------------------------
void cgUISkin::parseForm( const cgXMLNode & xNode )
{
    cgXMLNode xChild;

    // Retrieve the 'Overlapped' child and parse it
    xChild = xNode.getChildNode( _T("Overlapped") );
    if ( !xChild.isEmpty() )
        parseFormStyle( xChild, mOverlappedFormStyle );
}

//-----------------------------------------------------------------------------
//  Name : parseElementArea () (Private)
/// <summary>
/// Parses an individual area definition (rectangle + alignment settings)
/// </summary>
//-----------------------------------------------------------------------------
bool cgUISkin::parseElementArea( const cgXMLNode & xNode, cgUIElementArea & Area )
{
    // Input node was empty?
    if ( xNode.isEmpty() )
        return false;

    // Reset the structure
    Area.reset();

    // Contains all required attributes?
    cgString strRect, strAlign;
    if ( !xNode.getAttributeText( _T("rect"), strRect ) || 
         !xNode.getAttributeText( _T("align"), strAlign ) )
        return false;

    // Parse the area rectangle values
    if ( !cgStringUtility::tryParse( strRect, Area.bounds ) )
        return false;
    
    // Parse the alignment values
    cgStringArray aTokens;
    if ( !cgStringUtility::tokenize( strAlign, aTokens, _T(",") ) || aTokens.size() != 4 )
        return false;
    cgStringParser( aTokens[0] ) >> Area.align[0];
    cgStringParser( aTokens[1] ) >> Area.align[1];
    cgStringParser( aTokens[2] ) >> Area.align[2];
    cgStringParser( aTokens[3] ) >> Area.align[3];
    
    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : parseRegion () (Private)
/// <summary>
/// Parses the data required to populate a region structure.
/// </summary>
//-----------------------------------------------------------------------------
void cgUISkin::parseRegion( const cgXMLNode & xNode, cgPointArray & Region )
{
    cgXMLNode xChild;

    // Clear the array
    Region.clear();

    // Get all child points
    for ( cgUInt32 i = 0; ; )
    {
        // Retrieve the next child point node
        xChild = xNode.getNextChildNode( _T("Point"), i );
        if ( xChild.isEmpty() ) break;

        // Extract the point data
        cgPoint ptRegion;
        if ( !cgStringUtility::tryParse( xChild.getText(), ptRegion ) )
            continue;
        
        // Add it to the region for the element
        Region.push_back( ptRegion );

    } // Next point
}

//-----------------------------------------------------------------------------
//  Name : parseCursor () (Private)
/// <summary>
/// Parses all chidren of the 'Cursor' node.
/// </summary>
//-----------------------------------------------------------------------------
void cgUISkin::parseCursor( const cgXMLNode & xNode, const cgString & strBaseDirectory, cgUICursorDesc & Desc )
{
    cgXMLNode  xChild, xType, xFrames, xPlayback;
    cgString   strValue;

    // Reset the description structure
    Desc.Reset();

    // Retrieve the texture to use for the cursor
    xChild = xNode.getChildNode( _T("Texture") );
    if ( xChild.isEmpty() ) throw _T("No 'Texture' tag found within 'Cursor' node");
    // ToDo: test that this is reliable
    Desc.texture = strBaseDirectory + _T("/") + xChild.getText();

     // Find all cursor types
    for ( cgUInt32 i = 0; ; )
    {
        cgUICursorType Type;

        // Retrieve the next child type node
        xType = xNode.getNextChildNode( _T("Type"), i );
        if ( xType.isEmpty() )
            break;

        // Reset the type strucure
        Type.reset();

        // Get the type name
        if ( !xType.getAttributeText( _T("name"), Type.name ) )
            continue;
        
        // Is this cusror type animated?
        if ( !xType.getAttributeText( _T("animated"), strValue ) )
            continue;
        cgStringParser( strValue ) >> Type.animated;

        // hotPoint tag is optional, just retrieve if available, otherwise
        // <0,0> will be used.
        xChild = xType.getChildNode( _T("HotPoint") );
        if ( !xChild.isEmpty() )
            cgStringUtility::tryParse( xChild.getText(), Type.hotPoint );

        // If this is animated, it will have a frames child node, otherwise it will contain a single rect
        if ( Type.animated )
        {
            // Retrieve the frames node containing the animation rectangles
            xFrames = xType.getChildNode( _T("Frames") );
            if ( xFrames.isEmpty() )
                throw _T("Cursor type '") + Type.name + _T("' is listed as animated but contains no frame definitions");
            
            // Retrieve all frame rectangles
            for ( cgUInt32 j = 0; ; )
            {
                cgRect rcFrame;

                // Retrieve the next child rectangle node
                xChild = xFrames.getNextChildNode( _T("Rect"), j );
                if ( xChild.isEmpty() )
                    break;

                // Extract the point data
                cgStringUtility::tryParse( xChild.getText(), rcFrame );

                // Add it to the active region for the element
                Type.frames.push_back( rcFrame );

            } // Next point

            // Retrieve the playback descriptor for this animated cursor
            xPlayback = xType.getChildNode( _T("Playback") );
            if ( xPlayback.isEmpty() )
                throw _T("Cursor type '") + Type.name + _T("' is listed as animated but contains no playback descriptor");

            // Retrieve playback parameters
            xChild = xPlayback.getChildNode( _T("Duration") );
            if ( xChild.isEmpty() )
                throw _T("Cursor type '") + Type.name + _T("' is listed as animated but contains no animation duration");
            cgStringParser( xChild.getText() ) >> Type.duration;

            xChild = xPlayback.getChildNode( _T("Loop") );
            if ( xChild.isEmpty() )
                cgStringParser( xChild.getText() ) >> Type.loop;

        } // End if animated
        else
        {
            cgRect rcFrame;

            // Simply retrieve the single rectangle
            xChild = xType.getChildNode( _T("Rect") );
            if ( xChild.isEmpty() )
                throw _T("Cursor type '") + Type.name + _T("' is listed as static but contains no child rectangle");
            
            // Extract the rectangle data
            cgStringUtility::tryParse( xChild.getText(), rcFrame );

            // Add it to the active region for the element
            Type.frames.push_back( rcFrame );

        } // End if static / single frame

        // Add the cursor type to the definition
        Desc.types[ Type.name ] = Type;

    } // Next cursor type
}

//-----------------------------------------------------------------------------
//  Name : parseFormStyle () (Private)
/// <summary>
/// Parses the information for a given form node (i.e. overlapped, frame
/// etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgUISkin::parseFormStyle( const cgXMLNode & xNode, cgUIFormStyleDesc & Style )
{
    cgXMLNode xProperties, xElements, xChild;
    bool      bValue;

    // Reset the style structure
    Style.reset();

    // Retrieve the 'Properties' child and parse it
    xProperties = xNode.getChildNode( _T("Properties") );
    if ( !xProperties.isEmpty() )
    {
        xChild = xProperties.getChildNode( _T("MinSize") );
        if ( !xChild.isEmpty() )
            cgStringUtility::tryParse( xChild.getText(), Style.minimumSize );

        xChild = xProperties.getChildNode( _T("MaxSize") );
        if ( !xChild.isEmpty() )
            cgStringUtility::tryParse( xChild.getText(), Style.maximumSize );

        xChild = xProperties.getChildNode( _T("Sizable") );
        if ( !xChild.isEmpty() )
            Style.sizable = cgStringUtility::parseBool( xChild.getText() );

        // Has an IconArea node?
        xChild = xProperties.getChildNode( _T("IconArea") );
        Style.hasIconArea = parseElementArea( xChild, Style.icon );
        
        // Has a CaptionArea node?
        xChild = xProperties.getChildNode( _T("CaptionArea") );
        Style.hasCaptionArea = parseElementArea( xChild, Style.caption );
        
        // Has a ControlButtons node?
        xElements = xProperties.getChildNode( _T("ControlButtons") );
        if ( xElements.isEmpty() != false )
            throw _T("No 'ControlButtons' tag found");

        // First retrieve the close button node, this is actually the only required item
        xChild = xElements.getChildNode( _T("Close") );
        if ( !parseElementArea( xChild, Style.buttonClose ) )
            throw _T("No valid 'Close' child found within the 'ControlButtons' node");
        
        // Retrieve other nodes, only enable if all three elements are available
        bValue  = true;
        bValue &= parseElementArea( xElements.getChildNode( _T("Maximize") ), Style.buttonMaximize );
        bValue &= parseElementArea( xElements.getChildNode( _T("Minimize") ), Style.buttonMinimize );
        bValue &= parseElementArea( xElements.getChildNode( _T("Restore") ), Style.buttonRestore );
        bValue &= parseElementArea( xElements.getChildNode( _T("Close") ), Style.buttonClose );
        Style.hasSizeControls = bValue;
        
    } // End if has properties structure

    // Retrieve the 'Elements' child and parse it
    xElements = xNode.getChildNode( _T("Elements") );
    if ( xElements.isEmpty() )
        throw _T("No 'Elements' tag found");

    // Parse each element
    for ( cgUInt32 i = 0; ; )
    {
        cgUISkinElement Item;

        // Reset the item to allow it to be correctly validated
        Item.reset();

        // Retrieve the next child element node
        xChild = xElements.getNextChildNode( _T("Element"), i );
        if ( xChild.isEmpty() )
            break;

        // Parse the element
        if ( !parseElement( xChild, Item ) )
            continue;

        // Store this in the element map
        Style.elements[ Item.name ] = Item;

    } // Next element

    // This style is now available
    Style.styleAvailable = true;
}

//-----------------------------------------------------------------------------
//  Name : parseControls () (Private)
/// <summary>
/// Parses the node containing element definitions for controls (buttons
/// checkboxes etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgUISkin::parseControls( const cgXMLNode & xNode, cgUISkinElement::Map & Elements, ControlConfig & Config )
{
    cgXMLNode xConfig, xControl, xConfigKey, xElements, xChild;

    // Clear config structure to ensure that defaults are avaiable
    Config.reset();

    // Any configuration node?
    xConfig = xNode.getChildNode( _T("Configuration") );
    if ( !xConfig.isEmpty() )
    {
        // Has textbox config?
        xControl = xConfig.getChildNode( _T("TextBox") );
        if ( !xControl.isEmpty() )
        {
            // Caret config
            xConfigKey = xControl.getChildNode( _T("Caret") );
            if ( !xConfigKey.isEmpty() )
            {
                xChild = xConfigKey.getChildNode( _T("Color") );
                if ( !xChild.isEmpty() )
                {
                    cgColorValue & Color = Config.textBox.caretColor;
                    cgStringUtility::tryParse( xChild.getText(), Color );
                
                } // End if found color

                xChild = xConfigKey.getChildNode( _T("BlinkSpeed") );
                if ( !xChild.isEmpty() )
                    cgStringParser( xChild.getText() ) >> Config.textBox.caretBlinkSpeed;
                
            } // End if Caret

            // Selection config
            xConfigKey = xControl.getChildNode( _T("Selection") );
            if ( !xConfigKey.isEmpty() )
            {
                xChild = xConfigKey.getChildNode( _T("Color") );
                if ( !xChild.isEmpty() )
                {
                    cgColorValue & Color = Config.textBox.selectionColor;
                    cgStringUtility::tryParse( xChild.getText(), Color );
                
                } // End if found color

            } // End if Selection

        } // End if textBox config

        // Has listbox config?
        xControl = xConfig.getChildNode( _T("ListBox") );
        if ( !xControl.isEmpty() )
        {
            // Selection config
            xConfigKey = xControl.getChildNode( _T("Selection") );
            if ( !xConfigKey.isEmpty() )
            {
                xChild = xConfigKey.getChildNode( _T("Color") );
                if ( !xChild.isEmpty() )
                {
                    cgColorValue & Color = Config.listBox.selectionColor;
                    cgStringUtility::tryParse( xChild.getText(), Color );
                
                } // End if found color

            } // End if Selection

        } // End if listBox config

    } // End if found configuration node

    // Clear the array to ensure that we're starting with a fresh data set
    Elements.clear();

    // Retrieve the 'Elements' child and parse it
    xElements = xNode.getChildNode( _T("Elements") );
    if ( xElements.isEmpty() )
        throw _T("No 'Elements' tag found");

    // Parse each element
    for ( cgUInt32 i = 0; ; )
    {
        cgUISkinElement Item;

        // Reset the item to allow it to be correctly validated
        Item.reset();

        // Retrieve the next child element node
        xChild = xElements.getNextChildNode( _T("Element"), i );
        if ( xChild.isEmpty() )
            break;

        // Parse the element
        if ( !parseElement( xChild, Item ) )
            continue;

        // Store this in the element map
        Elements[ Item.name ] = Item;

    } // Next element
}

//-----------------------------------------------------------------------------
//  Name : parseElement() (Private)
/// <summary>
/// Parses the information for a given element node.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUISkin::parseElement( const cgXMLNode & xNode, cgUISkinElement & Desc )
{
    cgXMLNode xChild;
    cgString  strType, strValue;
    cgPoint   Offset, Size;
    
    // Reset the item to allow it to be correctly validated
    Desc.reset();

    // Get all attributes
    if ( !xNode.getAttributeText( _T("name"), Desc.name ) )
        return false;
    if ( !xNode.getAttributeText( _T("x"), strValue ) )
        return false;
    cgStringParser( strValue ) >> Offset.x;
    if ( !xNode.getAttributeText( _T("y"), strValue ) )
        return false;
    cgStringParser( strValue ) >> Offset.y;
    if ( !xNode.getAttributeText( _T("width"), strValue ) )
        return false;
    cgStringParser( strValue ) >> Size.x;
    if ( !xNode.getAttributeText( _T("height"), strValue ) )
        return false;
    cgStringParser( strValue ) >> Size.y;

    // Build the element rectangle
    Desc.bounds = cgRect( Offset.x, Offset.y, Offset.x + Size.x, Offset.y + Size.y );

    // Does this element have an active region?
    xChild = xNode.getChildNode( _T("ActiveRegion") );
    if ( !xChild.isEmpty() )
        parseRegion( xChild, Desc.activeRegion );

    // Any handle regions?
    for ( cgUInt32 i = 0; ; )
    {
        // Retrieve the next HandleRegion node.
        xChild = xNode.getNextChildNode( _T("HandleRegion"), i );
        if ( xChild.isEmpty() )
            break;

        // What type of handle region?
        if ( !xChild.getAttributeText( _T("type"), strType ) )
            continue;

        // Parse into the correct region
        strType.toLower();
        if ( strType == _T("north") )
            parseRegion( xChild, Desc.handleRegions[ cgUIHandleType::N ] );
        else if ( strType == _T("northeast") )
            parseRegion( xChild, Desc.handleRegions[ cgUIHandleType::NE ] );
        else if ( strType == _T("east") )
            parseRegion( xChild, Desc.handleRegions[ cgUIHandleType::E ] );
        else if ( strType == _T("southeast") )
            parseRegion( xChild, Desc.handleRegions[ cgUIHandleType::SE ] );
        else if ( strType == _T("south") )
            parseRegion( xChild, Desc.handleRegions[ cgUIHandleType::S ] );
        else if ( strType == _T("southwest") )
            parseRegion( xChild, Desc.handleRegions[ cgUIHandleType::SW ] );
        else if ( strType == _T("west") )
            parseRegion( xChild, Desc.handleRegions[ cgUIHandleType::W ] );
        else if ( strType == _T("northwest") )
            parseRegion( xChild, Desc.handleRegions[ cgUIHandleType::NW ] );

    } // Next handle region

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : prepareControlFrames()
/// <summary>
/// Prepare all of the billboard data based on the information contained 
/// in the skin definition for forms and controls etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUISkin::prepareControlFrames( cgUIControlLayer * pLayer, cgUIFormStyle::Base Style )
{
    cgUIFormStyleDesc     * pStyleDesc = CG_NULL;
    cgUISkinElement::Map  * pElements = CG_NULL;
    cgInt16                 nActiveGroup = -1, nInactiveGroup = -1;
    cgUInt32                i;

    // Find the correct form style type
    switch ( Style )
    {
        case cgUIFormStyle::Overlapped:
            pStyleDesc = &mOverlappedFormStyle;
            break;

    } // End Switch Style

    // Form style exists?
    if ( !pStyleDesc || !pStyleDesc->styleAvailable )
        return false;

    // Add the form elements for the selected style
    pElements = &((cgUISkinElement::Map&)pStyleDesc->elements);

    // The active form elements
    for ( i = 0; i < mValidFormElements.size(); ++i )
        addElementFrame( pLayer, *pElements, mValidFormElements[i] );
    
    // The inactive form elements
    for ( i = 0; i < mValidFormElements.size(); ++i )
        addElementFrame( pLayer, *pElements, _T("Inactive::") + mValidFormElements[i] );
    
    // Now add any defined control elements
    pElements = &((cgUISkinElement::Map&)mControlElements);

    // The active control elements
    for ( i = 0; i < mValidControlElements.size(); ++i )
        addElementFrame( pLayer, *pElements, mValidControlElements[i] );
    
    // The inactive control elements
    for ( i = 0; i < mValidControlElements.size(); ++i )
        addElementFrame( pLayer, *pElements, _T("Inactive::") + mValidControlElements[i] );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : prepareCursorFrames()
/// <summary>
/// Prepare all of the billboard data based on the information contained 
/// in the skin definition for the cursor.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUISkin::prepareCursorFrames( cgBillboardBuffer * pBuffer, cgRenderDriver * pRenderDriver )
{
    cgInt16  nGroupIndex = 0;
    cgUInt32 i;

    // Get access to required systems.
    cgResourceManager * pResources = pRenderDriver->getResourceManager();

    // Generate platform specific cursor data from the provided cursor
    // texture file to use when cursor emulation is not in use.
    cgImage * pCursorImage = CG_NULL;
    cgTextureHandle hTexture = pBuffer->getTexture();
    cgTexture * pTexture = hTexture.getResource();
    if ( pTexture && pTexture->isLoaded() )
    {
        if ( ( pCursorImage = cgImage::createInstance( pTexture->getSize().width, pTexture->getSize().height, cgBufferFormat::B8G8R8A8, false ) ) )
        {
            if ( !pTexture->getImageData( *pCursorImage ) )
            {
                delete pCursorImage;
                pCursorImage = CG_NULL;

            } // End if failed

        } // End if created
    
    } // End if loaded
    
    // Iterate through each defined cursor type
    cgUICursorType::Map::iterator itType;
    cgUICursorType::Map & Types = mCursorDefinition.types;
    for ( itType = Types.begin(); itType != Types.end(); ++itType )
    {
        cgUICursorType & Type = itType->second;
        
        // Add the frame group for this cursor type
        nGroupIndex = pBuffer->addFrameGroup( itType->first );

        // Add all of the frames defined in the cursor definition
        for ( i = 0; i < Type.frames.size(); ++i )
            pBuffer->addFrame( nGroupIndex, Type.frames[i] );

        // If hardware cursors are enabled, build the platform
        // compatible cursor resources here for each frame.
        if ( pCursorImage )
        {
            for ( i = 0; i < Type.frames.size(); ++i )
            {
                cgCursor * pCursor = cgCursor::createInstance( Type.hotPoint, Type.frames[i], *pCursorImage );
                if ( pCursor )
                    pCursor->addReference( CG_NULL );
                Type.platformCursors.push_back( pCursor );

            } // Next frame

            // We're done with the cursor.
            delete pCursorImage;
            pCursorImage = NULL;
        
        } // End if hardware cursor

    } // Next cursor type

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addElementFrame() (Private)
/// <summary>
/// If the specified element exists in the element map, registers the
/// information with the control layer.
/// </summary>
//-----------------------------------------------------------------------------
void cgUISkin::addElementFrame( cgUIControlLayer * pLayer, const cgUISkinElement::Map & Elements, const cgString & strElementName )
{
    static const cgString      Modes[4] = { _T(""), _T(".Hover"), _T(".Pressed"), _T(".Disabled") };
    cgString                   strFullElementName;
    int                        i;

    // Loop through for each of the 4 supported modes for any element
    for ( i = 0; i < 4; ++i )
    {
        // Build the full element name used as the key from the element map
        strFullElementName = strElementName + Modes[i];

        // First find the element referenced by the element name
        cgUISkinElement::Map::const_iterator itElement;
        itElement = Elements.find( strFullElementName );
        
        // If we couldn't find it, this is an optional element, so just continue as if nothing went wrong
        if ( itElement == Elements.end() ) continue;

        // Add this to the control layer
        pLayer->registerSkinElement( itElement->second, strFullElementName );

    } // Next Item
}

//-----------------------------------------------------------------------------
//  Name : getFormStyleDefinition()
/// <summary>
/// Retrieve the loaded definition for the specified form style.
/// </summary>
//-----------------------------------------------------------------------------
const cgUIFormStyleDesc & cgUISkin::getFormStyleDefinition( cgUIFormStyle::Base Type ) const
{
    static const cgUIFormStyleDesc Empty;
    switch ( Type )
    {
        case cgUIFormStyle::Overlapped:
            return mOverlappedFormStyle;
    
    } // End Switch Type

    // Invalid type
    return Empty;
}

//-----------------------------------------------------------------------------
//  Name : getFormElement() (Private)
/// <summary>
/// Retrieve the specified element definition from the required form type.
/// </summary>
//-----------------------------------------------------------------------------
cgUISkinElement * cgUISkin::getFormElement( cgUIFormStyle::Base Style, const cgString & strElementName )
{
    cgUISkinElement::Map::iterator itElement;
    cgUIFormStyleDesc * pDesc = CG_NULL;

    // What form style?
    switch ( Style )
    {
        case cgUIFormStyle::Overlapped:
            pDesc = &mOverlappedFormStyle;
            break;
    
    } // End Switch Style

    // Valid style specified?
    if ( !pDesc )
        return CG_NULL;

    // Find the element referenced by the element name
    itElement = pDesc->elements.find( strElementName );
    if ( itElement == pDesc->elements.end() )
        return CG_NULL;

    // Return the element
    return &itElement->second;
}

//-----------------------------------------------------------------------------
//  Name : getControlElement() (Private)
/// <summary>
/// Retrieve the specified control element definition.
/// </summary>
//-----------------------------------------------------------------------------
cgUISkinElement * cgUISkin::getControlElement( const cgString & strElementName )
{
    cgUISkinElement::Map::iterator itElement;

    // Find the element referenced by the element name
    itElement = mControlElements.find( strElementName );
    if ( itElement == mControlElements.end() )
        return CG_NULL;

    // Return the element
    return &itElement->second;
}

//-----------------------------------------------------------------------------
//  Name : processRegions() (Private)
/// <summary>
/// After the skin definition has been loaded in, several elements may
/// contain region information. This function first copys any regions
/// into paired elements if one was missing (i.e. Form.BorderTopLeft had
/// an active region, but Inactive::Form.BorderTopLeft did not)
/// </summary>
//-----------------------------------------------------------------------------
void cgUISkin::processRegions( )
{
    cgUInt32                i, j, k, l;
    cgUISkinElement       * pPrimaryElement, * pSecondaryElement;
    static const cgString   Modes[4] = { _T(""), _T(".Hover"), _T(".Pressed"), _T(".Disabled") };
    static const cgString   Groups[2] = { _T(""), _T("Inactive::") };

    // Iterate through all supported form elements first of all
    for ( i = 0; i < mValidFormElements.size(); ++i )
    {
        // Retrieve the primary element we'll duplicate regions from
        pPrimaryElement = getFormElement( cgUIFormStyle::Overlapped, Groups[0] + mValidFormElements[i] );
        if ( !pPrimaryElement ) continue;

        // Copy to the primary item for all other groups
        for ( l = 1; l < 2; ++l )
        {
            // Retrieve the next secondary element
            pSecondaryElement = getFormElement( cgUIFormStyle::Overlapped, Groups[l] + mValidFormElements[i] );
            if ( !pSecondaryElement ) continue;
            
            // Duplicate this to any secondary element if it has no region
            if ( pSecondaryElement->activeRegion.size() == 0 )
                pSecondaryElement->activeRegion = pPrimaryElement->activeRegion;

            // Copy all handle regions specified if required
            for ( k = 0; k < 8; ++k )
            {
                if ( pSecondaryElement->handleRegions[k].size() == 0 )
                    pSecondaryElement->handleRegions[k] = pPrimaryElement->handleRegions[k];

            } // Next Handle

        } // Next group

        // For each group (active / inactive etc)
        for ( l = 0; l < 2; ++l )
        {
            // Retrieve the primary element we'll duplicate regions from
            pPrimaryElement = getFormElement( cgUIFormStyle::Overlapped, Groups[l] + mValidFormElements[i] );
            if ( !pPrimaryElement ) continue;

            // Each of the other modes
            for ( j = 1; j < 4; ++j )
            {
                // Retrieve the next secondary element
                pSecondaryElement = getFormElement( cgUIFormStyle::Overlapped, Groups[l] + mValidFormElements[i] + Modes[j] );
                if ( !pSecondaryElement ) continue;
                
                // Duplicate this to any secondary element if it has no region
                if ( pSecondaryElement->activeRegion.size() == 0 )
                    pSecondaryElement->activeRegion = pPrimaryElement->activeRegion;

                // Copy all handle regions specified if required
                for ( k = 0; k < 8; ++k )
                {
                    if ( pSecondaryElement->handleRegions[k].size() == 0 )
                        pSecondaryElement->handleRegions[k] = pPrimaryElement->handleRegions[k];

                } // Next Handle

            } // Next mode

        } // Next Group
        
    } // Next Skin Element

    // Now loop through the supported control elements
    for ( i = 0; i < mValidControlElements.size(); ++i )
    {
        // Retrieve the primary element we'll duplicate regions from
        pPrimaryElement = getControlElement( Groups[0] + mValidControlElements[i] );
        if ( !pPrimaryElement ) continue;

        // Copy to the primary item for all other groups
        for ( l = 1; l < 2; ++l )
        {
            // Retrieve the next secondary element
            pSecondaryElement = getControlElement( Groups[l] + mValidControlElements[i] );
            if ( !pSecondaryElement ) continue;
            
            // Duplicate this to any secondary element if it has no region
            if ( pSecondaryElement->activeRegion.size() == 0 )
                pSecondaryElement->activeRegion = pPrimaryElement->activeRegion;

            // Copy all handle regions specified if required
            for ( k = 0; k < 8; ++k )
            {
                if ( pSecondaryElement->handleRegions[k].size() == 0 )
                    pSecondaryElement->handleRegions[k] = pPrimaryElement->handleRegions[k];

            } // Next Handle

        } // Next group

        // For each group (active / inactive etc)
        for ( l = 0; l < 2; ++l )
        {
            // Retrieve the primary element we'll duplicate regions from
            pPrimaryElement = getControlElement( Groups[l] + mValidControlElements[i] );
            if ( !pPrimaryElement ) continue;

            // Each of the other modes
            for ( j = 1; j < 4; ++j )
            {
                // Retrieve the next secondary element
                pSecondaryElement = getControlElement( Groups[l] + mValidControlElements[i] + Modes[j] );
                if ( !pSecondaryElement ) continue;
                
                // Duplicate this to any secondary element if it has no region
                if ( pSecondaryElement->activeRegion.size() == 0 )
                    pSecondaryElement->activeRegion = pPrimaryElement->activeRegion;

                // Copy all handle regions specified if required
                for ( k = 0; k < 8; ++k )
                {
                    if ( pSecondaryElement->handleRegions[k].size() == 0 )
                        pSecondaryElement->handleRegions[k] = pPrimaryElement->handleRegions[k];

                } // Next Handle

            } // Next mode

        } // Next Group

    } // Next Skin Element
}