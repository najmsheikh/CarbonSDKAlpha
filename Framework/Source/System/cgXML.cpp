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
// Name : cgXML.cpp                                                          //
//                                                                           //
// Desc : Utility classes that provide XML read/write support.               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgXML Module Includes
//-----------------------------------------------------------------------------
#include <System/cgXML.h>
#include <System/cgStringUtility.h>

//-----------------------------------------------------------------------------
// Static Member Definitions.
//-----------------------------------------------------------------------------
const cgXMLNode      cgXMLNode::Empty;
const cgXMLAttribute cgXMLAttribute::Empty;

//-----------------------------------------------------------------------------
// Module Local Variables
//-----------------------------------------------------------------------------
namespace
{
    const cgChar PartialWordChar = '=';
    const cgByte DecodeTable[] = {
        99,98,98,98,98,98,98,98,98,97,  97,98,98,97,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //00 -29
        98,98,97,98,98,98,98,98,98,98,  98,98,98,62,98,98,98,63,52,53,  54,55,56,57,58,59,60,61,98,98,  //30 -59
        98,96,98,98,98, 0, 1, 2, 3, 4,   5, 6, 7, 8, 9,10,11,12,13,14,  15,16,17,18,19,20,21,22,23,24,  //60 -89
        25,98,98,98,98,98,98,26,27,28,  29,30,31,32,33,34,35,36,37,38,  39,40,41,42,43,44,45,46,47,48,  //90 -119
        49,50,51,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //120 -149
        98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //150 -179
        98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //180 -209
        98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98,98,98,98,98,  //210 -239
        98,98,98,98,98,98,98,98,98,98,  98,98,98,98,98,98                                               //240 -255
    };
}; 

///////////////////////////////////////////////////////////////////////////////
// cgXMLDocument Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgXMLDocument () (Constructor)
/// <summary>
/// cgXMLDocument Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgXMLDocument::cgXMLDocument()
{
    // Initialize variables
    mStreamData = CG_NULL;
    mDocumentNode    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgXMLDocument () (Constructor)
/// <summary>
/// cgXMLDocument Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgXMLDocument::cgXMLDocument( const cgXMLDocument & Doc )
{
    // Initialize variables
    mStreamData = CG_NULL;
    mDocumentNode    = CG_NULL;

    // Perform necessary copy operation
    *this = Doc;
}

//-----------------------------------------------------------------------------
//  Name : ~cgXMLDocument () (Destructor)
/// <summary>
/// cgXMLDocument Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgXMLDocument::~cgXMLDocument()
{
    // Destroy our document node.
    delete mDocumentNode;

    // Close our reference to any open stream.
    if ( mStreamData != CG_NULL )
        mStreamData->release();

    // Clear variables
    mStreamData = CG_NULL;
    mDocumentNode    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (const cgXMLDocument&)
/// <summary>
/// Overloaded assignment operator.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLDocument & cgXMLDocument::operator=( const cgXMLDocument & Doc )
{
    // It's important that we increment the reference count
    // of the input document's stream first in case it is a 
    // self reference or referencing the same data.
    if ( Doc.mStreamData != CG_NULL )
        Doc.mStreamData->addRef();

    // Clean up our own internal data
    if ( mStreamData != CG_NULL )
        mStreamData->release();
    delete mDocumentNode;
    mDocumentNode = CG_NULL;

    // Share the specified input documents's stream pointer
    mStreamData = Doc.mStreamData;

    // Duplicate its document node.
    if ( Doc.mDocumentNode != NULL )
        mDocumentNode = new cgXMLNode( *Doc.mDocumentNode );

    // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (const cgXMLDocument&)
/// <summary>
/// Overloaded assignment operator.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgXMLDocument::getErrorDescription( cgXMLError::Result result )
{
    switch (result)
    {
        case cgXMLError::Success:
            return _T("The operation completed successfully.");
        case cgXMLError::AlreadyOpen:
            return _T("The specified document or stream was already open.");
        case cgXMLError::AccessDenied:
            return _T("Access to the specified document or stream was denied.");
        case cgXMLError::EndOfFile:
            return _T("Unexpected end of file encountered.");
        case cgXMLError::NoDocumentTag:
            return _T("The required document tag was not found in the specified document or stream.");
        case cgXMLError::NoTagName:
            return _T("A tag with no name was encountered.");
        case cgXMLError::MismatchedQuote:
            return _T("Unable to find a matching quote character. Unable to distinguish the end of encountered string data.");
        case cgXMLError::MismatchedTag:
            return _T("A mismatched tag was encountered. The name of the tag close did not match the opening tag name.");
        case cgXMLError::UnexpectedToken:
            return _T("An unexpected token was encountered when parsing the specified document or stream.");
        case cgXMLError::NoBinaryData:
            return _T("No binary data was found when parsing encoded text node.");
        case cgXMLError::TruncatedData:
            return _T("Amount of binary data written to the output buffer was less than the requested size.");
        case cgXMLError::BufferTooSmall:
            return _T("The output buffer was found to be too small when decoding binary data.");
        case cgXMLError::IllegalBase64Char:
            return _T("Base64 binary data decoder encountered a character that it was unable to interpret.");

    } // End Switch Result

    // Unknown
    return _T("An unknown error has occurred.");
}

//-----------------------------------------------------------------------------
//  Name : getDocumentNode ()
/// <summary>
/// Get the root document node loaded from the XML stream.
/// </summary>
//-----------------------------------------------------------------------------
const cgXMLNode & cgXMLDocument::getDocumentNode( ) const
{
    if ( mDocumentNode == CG_NULL )
        return cgXMLNode::Empty;
    return *mDocumentNode;
}

//-----------------------------------------------------------------------------
//  Name : open ()
/// <summary>
/// Open the specified stream for read / write.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLError::Result cgXMLDocument::open( const cgInputStream & Stream, const cgString & strDocumentTag /* = _T("") */, bool bWrite /* = false */ )
{
    cgXMLError::Result Result;

    // ToDo: "Write" support.
    // ToDo: Alternate encoding support (assumes ASCII currently).

    // Already open?
    if ( mStreamData != CG_NULL )
        return cgXMLError::AlreadyOpen;

    // Destroy any prior document data.
    if ( mDocumentNode != CG_NULL )
        delete mDocumentNode;
    mDocumentNode = CG_NULL;

    // Create a new referencable stream data item. This allows
    // duplicated documents AND child node objects to continue
    // to reference the opened stream even if the original
    // document is destroyed.
    mStreamData = new StreamData();
    mStreamData->stream = Stream;

    // Attempt to open the specified stream.
    if ( mStreamData->stream.open() == false )
    {
        mStreamData->release();
        mStreamData = CG_NULL;
        return cgXMLError::AccessDenied;
    
    } // End if failed

    // Parse the document.
    cgXMLNode RootNode;
    Result = RootNode.parseElement( mStreamData, CG_NULL );
    
    // Clean up if this failed.
    if ( Result != cgXMLError::Success )
    {
        delete mDocumentNode;
        mDocumentNode = CG_NULL;
        close();
    
    } // End if failed
    else
    {
        // Find the requested document node
        if ( strDocumentTag.empty() == false )
        {
            mDocumentNode = new cgXMLNode( RootNode.getChildNode( strDocumentTag ) );
            if ( mDocumentNode->isEmpty() == true )
            {
                Result = cgXMLError::NoDocumentTag;
                delete mDocumentNode;
                mDocumentNode = CG_NULL;
                close();
            
            } // End if not found.
        
        } // End if requested root
        else
            mDocumentNode = new cgXMLNode( RootNode );

    } // End if success
    
    // Success!
    return Result;
}

//-----------------------------------------------------------------------------
//  Name : close ()
/// <summary>
/// Close our reference to the stream.
/// Note : Nodes may still reference and therefore the stream itself may remain 
/// open until those nodes go out of scope or are destroyed.
/// </summary>
//-----------------------------------------------------------------------------
void cgXMLDocument::close( )
{
    // Close our reference to any stream data.
    if ( mStreamData != CG_NULL )
        mStreamData->release();
    mStreamData = CG_NULL;
}

///////////////////////////////////////////////////////////////////////////////
// cgXMLNode Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgXMLNode () (Constructor)
/// <summary>
/// cgXMLNode Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgXMLNode::cgXMLNode()
{
    // Initialize variables
    mData = CG_NULL;
}


//-----------------------------------------------------------------------------
//  Name : cgXMLNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLNode::cgXMLNode( const cgXMLNode & Node )
{
    // Initialize variables to sensible defaults
    mData = CG_NULL;
    
    // Perform necessary copy operation
    *this = Node;
}

//-----------------------------------------------------------------------------
//  Name : ~cgXMLNode () (Destructor)
/// <summary>
/// cgXMLNode Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgXMLNode::~cgXMLNode()
{
    // Close our reference to any node data.
    if ( mData != CG_NULL )
        mData->release();

    // Clear variables
    mData = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (const cgXMLNode&)
/// <summary>
/// Overloaded assignment operator.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLNode & cgXMLNode::operator=( const cgXMLNode & Node )
{
    // It's important that we increment the reference count
    // of the input node's data first in case it is a self
    // reference or referencing the same data.
    if ( Node.mData != CG_NULL )
        Node.mData->addRef();

    // Clean up our own internal data
    if ( mData != CG_NULL )
        mData->release();

    // Share the specified input node's data pointer
    mData = Node.mData;

    // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : parseElement () (Private)
/// <summary>
/// Parse the next element in the stream as appropriate.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLError::Result cgXMLNode::parseElement( cgXMLDocument::StreamData * pStream, cgXMLNode * pParentNode )
{
    bool     bAttribute     = false;
    bool     bSeparated     = false;
    bool     bInsideTag     = false;
    bool     bIsDeclaration = false;

    // Add references to data as appropriate
    if ( pStream != CG_NULL )
        pStream->addRef();

    // Build node data entry.
    if ( mData == CG_NULL )
        mData = new NodeData();

    // Release old stream (if necessary) and replace.
    if ( mData->streamData != CG_NULL )
        mData->streamData->release();
    mData->streamData = pStream;
    
    // Keep processing until we run out of data.
    for ( ; ; )
    {
        cgInt64            nTokenStart;
        cgString           strData;
        cgXMLTokens::Type  TokenType;

        // Get the next token in the document.
        cgXMLError::Result Result = getNextToken( bInsideTag, bInsideTag, TokenType, strData, nTokenStart );

        // Currently processing the interior of a tag for attributes or not?
        if ( bInsideTag == false )
        {
            // If we hit an EOF, we can just bail and let our parent determine 
            // if this was valid for them unless we're the root.
            if ( Result == cgXMLError::EndOfFile )
                return (pParentNode == CG_NULL) ? cgXMLError::Success : Result;

            // If the result was anything else, we need to bail immediately.
            if ( Result != cgXMLError::Success )
                return Result;

            // What token did we encounter?
            switch ( TokenType )
            {
                case cgXMLTokens::Declaration:
                case cgXMLTokens::TagStart:
                case cgXMLTokens::TagOpen:
                case cgXMLTokens::TagShort:
                {
                    // Tag must have name.
                    if ( strData.empty() == true )
                        return cgXMLError::NoTagName;

                    // A new child node has been encountered. Create
                    // a place in our child node array for it.
                    size_t nChild = mData->childNodes.size();
                    mData->childNodes.resize( nChild + 1 );
                    cgXMLNode & ChildNode = mData->childNodes[ nChild ];

                    // Store child node data
                    ChildNode.setName( strData );
                    // ToDo: Others

                    // Cache in LUT if it doesn't already exist.
                    cgString strKey = cgString::toLower( strData );
                    if ( mData->childNodeLUT.find( strKey ) == mData->childNodeLUT.end() )
                        mData->childNodeLUT[ strKey ] = nChild;

                    // Allow child node to parse immediately if this was a full start tag.
                    if ( TokenType == cgXMLTokens::TagStart )
                    {
                        // Parse children.
                        cgXMLError::Result ChildResult = ChildNode.parseElement( mData->streamData, this );
                        
                        // If the child encountered an EOF, this to us is classed as 
                        // a mismatched start tag.
                        if ( ChildResult == cgXMLError::EndOfFile )
                            ChildResult = cgXMLError::MismatchedTag;

                        // Failed?
                        if ( ChildResult != cgXMLError::Success )
                            return ChildResult;

                    } // End if TagOpen
                    else if ( TokenType == cgXMLTokens::TagShort )
                    {
                        // We're done with this node, return to parent.
                        return cgXMLError::Success;
                    
                    } // End if TagShort
                    else
                    {
                        // Process tag interior for attributes
                        bInsideTag     = true;
                        bIsDeclaration = (TokenType == cgXMLTokens::Declaration);
                    
                    } // End if TagStart | TagDeclaration
                    
                    // Done
                    break;

                } // End case TagStart
                case cgXMLTokens::TagEnd:
                {
                    // We're done with this node, return to parent.
                    return cgXMLError::Success;

                } // End case TagClose
                case cgXMLTokens::Text:
                {
                    // Record text element.
                    TextReference ref;
                    ref.cached  = false;
                    ref.length   = (cgUInt32)(pStream->stream.getPosition() - nTokenStart);
                    ref.location = nTokenStart;
                    mData->childText.push_back( ref );

                    // Done.
                    break;

                } // End case Text
                default:
                    return cgXMLError::UnexpectedToken;

            } // End switch TokenType

        } // End if outside tag
        else
        {
            size_t nCurrentNode = mData->childNodes.size() - 1;
            cgXMLNode & CurrentNode = mData->childNodes[ nCurrentNode ];

            // If we hit an EOF, then there is a mismatched starting tag.
            if ( Result == cgXMLError::EndOfFile )
                return cgXMLError::MismatchedTag;

            // If the result was anything else, we need to bail immediately.
            if ( Result != cgXMLError::Success )
                return Result;

            // Are we reading an attribute right now?
            if ( bAttribute == false )
            {                
                // What token did we encounter?
                switch ( TokenType )
                {
                    case cgXMLTokens::Text:
                    {
                        // ToDo: use an 'AddAttribute()'?

                        // A new child attribute has been encountered. Create
                        // a place in our child attribute array for it.
                        size_t nChildAttr = CurrentNode.mData->childAttributes.size();
                        CurrentNode.mData->childAttributes.resize( nChildAttr + 1 );
                        cgXMLAttribute & ChildAttribute = CurrentNode.mData->childAttributes[ nChildAttr ];

                        // Store child attribute data
                        ChildAttribute.setName( strData );

                        // Cache in LUT if it doesn't already exist.
                        cgString strKey = cgString::toLower( strData );
                        if ( CurrentNode.mData->childAttributeLUT.find( strKey ) == CurrentNode.mData->childAttributeLUT.end() )
                            CurrentNode.mData->childAttributeLUT[ strKey ] = nChildAttr;

                        // This is an attribute.
                        bAttribute = true;
                        bSeparated = false;
                        break;
                    
                    } // End Case Text
                    case cgXMLTokens::TagShortClose:
                    case cgXMLTokens::TagClose:
                    {
                        // We are no longer inside a tag.
                        bInsideTag = false;

                        // Allow child node to parse if applicable.
                        if ( (TokenType != cgXMLTokens::TagShortClose) )
                        {
                            cgXMLError::Result ChildResult = CurrentNode.parseElement( mData->streamData, this );
                            
                            // If the child encountered an EOF, this to us is classed as 
                            // a mismatched start tag.
                            if ( ChildResult == cgXMLError::EndOfFile )
                                ChildResult = cgXMLError::MismatchedTag;

                            // Failed?
                            if ( ChildResult != cgXMLError::Success )
                                return ChildResult;

                        } // End if !Declaration
                        
                        // Done
                        break;

                    } // End case TagEnd
                    default:
                        return cgXMLError::UnexpectedToken;

                } // End switch TokenType

            } // End if !Attribute
            else
            {
                // What token did we encounter?
                switch ( TokenType )
                {
                    case cgXMLTokens::Equals:

                        // Separator discovered.
                        bSeparated = true;
                        break;

                    case cgXMLTokens::QuotedText:
                    {

                        // Must be separated by equals
                        if ( bSeparated == false )
                            return cgXMLError::UnexpectedToken;

                        // Store the processed text.
                        size_t nChild = CurrentNode.mData->childAttributes.size() - 1;
                        cgXMLAttribute & ChildAttribute = CurrentNode.mData->childAttributes[ nChild ];
                        ChildAttribute.setText( strData );

                        // We're done reading this attribute.
                        bAttribute = false;
                        break;
                    
                    } // End case QuotedText
                    default:
                        return cgXMLError::UnexpectedToken;

                } // End switch TokenType

            } // End if Attribute

        } // End if inside tag

    } // Next iteration

    // Success!
    return cgXMLError::Success;
}

//-----------------------------------------------------------------------------
//  Name : getNextToken () (Private)
/// <summary>
/// Retrieve the next valid token in the stream.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLError::Result cgXMLNode::getNextToken( bool bInsideTag, bool bProcessText, cgXMLTokens::Type & TokenType, cgString & strData, cgInt64 & nTokenStart )
{
    cgTChar c;
    cgInputStream & Stream = mData->streamData->stream;

    // Keep searching until we're done
    bool bDone = false;
    while ( bDone == false )
    {
        // Assume we're done unless someone resets
        bDone = true;

        // First, skip to the next non-whitespace character (bail on EOF).
        if ( getNextNonWhitespace( c ) == false )
            return cgXMLError::EndOfFile;

        // Be polite and clear output parameter.
        strData.clear();
        
        // Record the starting location for the token (remember that we
        // have already consumed one character in the call to getNextNonWhitespace()).
        nTokenStart = Stream.getPosition() - 1;
    
        // White type of token is this?
        if ( bInsideTag == true )
        {
            switch ( c )
            {
                case _T('?'): // For declaration close
                case _T('/'):
                {
                    cgTChar cTest = (cgTChar)0;

                    // This is the potentially the closing of a short tag
                    // (i.e. <Test/>). Check if this is the case
                    if ( Stream.read( &cTest, 1 ) == 0 )
                        return cgXMLError::EndOfFile;

                    // What type?
                    switch ( cTest )
                    {
                        case _T('>'):
                            // This is a short close tag.
                            TokenType = cgXMLTokens::TagShortClose;
                            break;
                        
                        default:
                            // Nothing that we need be concerned about.
                            // Treat it like standard text.
                            TokenType = cgXMLTokens::Text;
                            
                            // Append character(s) to token data.
                            if ( bProcessText == true )
                            {
                                strData += c;
                                strData += cTest;
                            
                            } // End if collect text

                            break;

                    } // End Switch cTest

                    // Done
                    break;
                
                } // End Case '/' | '?'
                case _T('>'):
                    // Simply closing an open tag.
                    TokenType = cgXMLTokens::TagClose;
                    break;

                case _T('='):
                    // Equals for attribute values.
                    TokenType = cgXMLTokens::Equals;
                    break;

                case _T('\''):
                case _T('\"'):
                {
                    size_t  nCount      = 0;
                    cgTChar cQuote      = c;
                    bool    bFoundClose = false;

                    // Handle quoted text.
                    TokenType = cgXMLTokens::QuotedText;

                    // Read the data and find a matching closing quote.
                    for ( ; nCount = Stream.read( &c, 1 ); )
                    {
                        // Found the closing quote, or perhaps an invalid character?
                        if ( c == cQuote )
                        {
                            bFoundClose = true;
                            break;
                        
                        } // End if matching close
                        else if ( c == _T('<') )
                        {
                            // Not valid in string data!
                            break;
                        
                        } // End if invalid '<'
                        else if ( c == _T('>') )
                        {
                            // Not valid in string data!
                            break;
                        
                        } // End if invalid '>'
                        
                        // Append character to token data.
                        strData += c;
                        
                    } // Next Character

                    // Out of data?
                    if ( nCount == 0 )
                        return cgXMLError::EndOfFile;

                    // If we didn't find a closing quote, the file is malformed.
                    if ( bFoundClose == false )
                        return cgXMLError::MismatchedQuote;
                    
                    // Done.
                    break;

                } // End case Quote
                default:
                    // Text data.
                    TokenType = cgXMLTokens::Text;
                    
                    // Append character to token data.
                    if ( bProcessText == true )
                        strData += c;
                    break;
            
            } // End switch c

        } // End if bInsideTag
        else
        {
            switch ( c )
            {
                case _T('<'):
                    
                    // This is the opening of a tag. Check what type of tag
                    // it is (i.e. start '<Tag>' or end '</Tag>' or possibly
                    // a declaration '<?' or comment '<!--')
                    if ( Stream.read( &c, 1 ) == 0 )
                        return cgXMLError::EndOfFile;

                    // What type?
                    switch ( c )
                    {
                        case _T('!'):
                        {
                            // Possible comment (<!-- -->).
                            cgTChar cTest1 = (cgTChar)0;
                            cgTChar cTest2 = (cgTChar)0;
                            if ( Stream.read( &cTest1, 1 ) == 0 )
                                return cgXMLError::EndOfFile;
                            if ( Stream.read( &cTest2, 1 ) == 0 )
                                return cgXMLError::EndOfFile;
                            if ( cTest1 == _T('-') && cTest2 == _T('-') )
                                TokenType = cgXMLTokens::Comment;
                            else
                            {
                                // The is the opening of a start tag.
                                TokenType = cgXMLTokens::TagOpen;

                                // Store the characters we read from the stream (prematurely) as
                                // the first characters of the token data (i.e. the tag name).
                                strData += c;
                                strData += cTest1;
                                strData += cTest2;
                            
                            } // End if not comment
                            break;
                        
                        } // End Case !
                        case _T('/'):
                            // This is a closing tag.
                            TokenType = cgXMLTokens::TagEnd;
                            break;

                        case _T('?'):
                            // This is a declaration tag.
                            TokenType = cgXMLTokens::Declaration;
                            break;

                        case _T('>'):
                            // Closed immediately (classed as a completed tag but will throw an error later).
                            TokenType = cgXMLTokens::TagStart;
                            break;

                        default:
                            // The is the opening of a start tag.
                            TokenType = cgXMLTokens::TagOpen;

                            // Store the character we read from the stream (prematurely) as
                            // the first character of the token data (i.e. the tag name).
                            strData += c;
                            break;

                    } // End switch
                    break;

                default:
                    // Text data.
                    TokenType = cgXMLTokens::Text;
                    
                    // Append character to token data.
                    if ( bProcessText == true )
                        strData += c;
                    break;
            
            } // End switch c

        } // End if !bInsideTag

        // Read remaining characters as token data if necessary.
        switch ( TokenType )
        {
            case cgXMLTokens::TagOpen:
            case cgXMLTokens::Declaration:
            {
                size_t nCount;

                // Keep reading until the first invalid char.
                for ( ; nCount = Stream.read( &c, 1 ); )
                {
                    if ( c == _T('>') )
                    {
                        // Completed starting tag.
                        TokenType = cgXMLTokens::TagStart;
                        break;
                    
                    } // End if completed
                    else if ( c == _T(' ') || c == _T('\t') || c == _T('\n') || c == _T('\r') )
                    {
                        // Whitespace
                        break;
                    
                    } // End if close or whitespace
                    else if ( c == _T('/') )
                    {
                        cgTChar cNext;

                        // Possily a short tag (i.e. <MyTag/>)?
                        if ( (nCount = Stream.read( &cNext, 1 )) == 0 )
                            break;
                        if ( cNext == _T('>') )
                        {
                            // This was a short tag.
                            TokenType = cgXMLTokens::TagShort;
                            break;
                        
                        } // End if close
                        else
                        {
                            // This was just a continuation of the text.
                            strData += c;
                            strData += cNext;

                        } // End if other

                    } // End if possible short tag
                    else
                    {
                        // Continuation of the text.
                        strData += c;
                    
                    } // End if text
                    
                } // Next Character

                // Unexpected EOF?
                if ( nCount == 0 )
                    return cgXMLError::EndOfFile;

                // Done
                break;

            } // End case TagOpen || Declaration
            case cgXMLTokens::TagEnd:
            {
                size_t nCount;

                // Keep reading until we find the close.
                for ( ; nCount = Stream.read( &c, 1 ); )
                {
                    if ( c == _T('>') )
                    {
                        break;

                    } // End if found the close
                    else
                    {
                        // Continuation of the text.
                        strData += c;
                    
                    } // End if text
                    
                } // Next Character

                // Unexpected EOF?
                if ( nCount == 0 )
                    return cgXMLError::EndOfFile;

                // Done
                break;

            } // End case TagEnd
            case cgXMLTokens::Comment:
            {
                size_t nCount;
                
                // Keep reading until we find the closing comment tag
                for ( ; nCount = Stream.read( &c, 1 ); )
                {
                    if ( c == _T('-') )
                    {
                        // Possible comment close (-->).
                        cgTChar cTest1 = (cgTChar)0;
                        cgTChar cTest2 = (cgTChar)0;
                        if ( Stream.read( &cTest1, 1 ) == 0 )
                            return cgXMLError::EndOfFile;
                        if ( Stream.read( &cTest2, 1 ) == 0 )
                            return cgXMLError::EndOfFile;
                        if ( cTest1 == _T('-') && cTest2 == _T('>') )
                            break;
                        
                    } // End if start of close

                } // Next Character

                // Unexpected EOF?
                if ( nCount == 0 )
                    return cgXMLError::EndOfFile;

                // Comment discarded, search again for next valid token.
                bDone = false;

                // Done
                break;

            } // End case Comment
            case cgXMLTokens::Text:
            {
                size_t nCount;

                // Keep reading until we find the close.
                for ( ; nCount = Stream.read( &c, 1 ); )
                {
                    if ( c == _T('<') || (bInsideTag == true && (c == _T('>') || c == _T('='))) )
                    {
                        Stream.seek( -1 );
                        break;

                    } // End if found the close
                    else
                    {
                        // Continuation of the text.
                        if ( bProcessText == true )
                            strData += c;
                    
                    } // End if text
                    
                } // Next Character

                // Unexpected EOF?
                if ( nCount == 0 )
                    return cgXMLError::EndOfFile;

                // Done
                break;
                
            } // End case text

        } // End switch TokenType

    } // Next Iteration

    // We succeeded
    return cgXMLError::Success;
}

//-----------------------------------------------------------------------------
//  Name : getNextNonWhitespace () (Private)
/// <summary>
/// Retrieve the next valid non whitespace character in the stream.
/// </summary>
//-----------------------------------------------------------------------------
bool cgXMLNode::getNextNonWhitespace( cgTChar & c )
{
    cgInputStream & Stream = mData->streamData->stream;

    // Clear character to ensure that we zero out
    // any byte that is not relevant to the input character.
    c = (cgTChar)0;

    // Keep searching until we find a non whitespace character.
    size_t nCount;
    for ( ; nCount = Stream.read( &c, 1 ); )
    {
        // If this is not white space, break and return valid result.
        if ( c != _T(' ') && c != _T('\t') && c != _T('\n') && c != _T('\r') )
            break;

    } // Next character

    // Found next valid character?
    return (nCount != 0);
}

//-----------------------------------------------------------------------------
//  Name : isOfType ()
/// <summary>
/// Determine if this node is of the specified type (has a matching name)
/// </summary>
//-----------------------------------------------------------------------------
bool cgXMLNode::isOfType( const cgString & strType ) const
{
    if ( isEmpty() == true )
        return false;
    return (getName().compare(strType, true ) == 0);
}

//-----------------------------------------------------------------------------
//  Name : isEmpty ()
/// <summary>
/// Determine if this node is empty (i.e. contains no data).
/// </summary>
//-----------------------------------------------------------------------------
bool cgXMLNode::isEmpty( ) const
{
    return (mData == CG_NULL);
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Retrieve the name of this node (i.e. the tag name <Name></Name>)
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgXMLNode::getName( ) const
{
    static const cgString strEmpty;
    return ( mData == CG_NULL ) ? strEmpty : mData->name;
}

//-----------------------------------------------------------------------------
//  Name : setName ()
/// <summary>
/// Set the name of this node (i.e. the tag name <Name></Name>)
/// </summary>
//-----------------------------------------------------------------------------
void cgXMLNode::setName( const cgString & strName )
{
    if ( mData == CG_NULL )
        mData = new NodeData();
    mData->name = strName;
}

//-----------------------------------------------------------------------------
//  Name : getChildNodeCount ()
/// <summary>
/// Retrieve the number of child nodes associated with this one.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgXMLNode::getChildNodeCount( ) const
{
    return ( mData == CG_NULL ) ? 0 : (cgUInt32)mData->childNodes.size();
}

//-----------------------------------------------------------------------------
//  Name : getChildNode ()
/// <summary>
/// Retrieve the child node at the specified location in the array of
/// children.
/// </summary>
//-----------------------------------------------------------------------------
const cgXMLNode & cgXMLNode::getChildNode( cgUInt32 nIndex ) const
{
    if ( mData == CG_NULL || nIndex >= mData->childNodes.size() )
        return Empty;
    else
        return mData->childNodes[nIndex];
}

//-----------------------------------------------------------------------------
//  Name : getChildNode ()
/// <summary>
/// Retrieve the first child node with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
const cgXMLNode & cgXMLNode::getChildNode( const cgString & strTag ) const
{
    if ( mData == CG_NULL )
        return Empty;

    // Rapid lookup for first node with the specified name.
    cgString strKey = cgString::toLower( strTag );
    NodeMap::iterator itNode = mData->childNodeLUT.find( strKey );
    if ( itNode != mData->childNodeLUT.end() )
        return mData->childNodes[itNode->second];
    
    // Nothing found.
    return Empty;
}

//-----------------------------------------------------------------------------
//  Name : getNextChildNode ()
/// <summary>
/// Retrieve the next child node with the specified name, starting with
/// the specified index. The selected child index will be passed back out
/// via the same nStartIndex paramter ready for the next iteration
/// of a continuing loop.
/// </summary>
//-----------------------------------------------------------------------------
const cgXMLNode & cgXMLNode::getNextChildNode( const cgString & strTag, cgUInt32 & nStartIndex ) const
{
    if ( mData == CG_NULL || nStartIndex >= mData->childNodes.size() )
        return Empty;
    
    // Search for next item.
    size_t nChildren = mData->childNodes.size();
    for ( size_t i = nStartIndex; i < nChildren; ++i )
    {
        if ( mData->childNodes[i].getName().compare( strTag, true ) == 0 )
        {
            nStartIndex = (cgUInt32)i + 1;
            return mData->childNodes[i];

        } // End if match

    } // Next Child

    // None found.
    nStartIndex = (cgUInt32)mData->childNodes.size();
    return Empty;
}

//-----------------------------------------------------------------------------
//  Name : definesAttribute ()
/// <summary>
/// Determine if an attribute with the specified name is defined.
/// </summary>
//-----------------------------------------------------------------------------
bool cgXMLNode::definesAttribute( const cgString & strAttribute ) const
{
    if ( mData == CG_NULL )
        return false;

    // Rapid lookup for first attribute with the specified name.
    cgString strKey = cgString::toLower( strAttribute );
    AttributeMap::iterator itAttribute = mData->childAttributeLUT.find( strKey );
    return ( itAttribute != mData->childAttributeLUT.end() );
}

//-----------------------------------------------------------------------------
//  Name : getAttributeText ()
/// <summary>
/// Retrieve the text associated with the specified attribute. This
/// overload provides error information by returning a true/false result
/// if the attribute could not be found, with the value returned via the
/// strText output parameter.
/// </summary>
//-----------------------------------------------------------------------------
bool cgXMLNode::getAttributeText( const cgString & strAttribute, cgString & strText ) const
{
    // Be polite and clear the output parameter
    strText = _T("");
    
    // Empty attribute?
    if ( mData == CG_NULL )
        return false;

    // Rapid lookup for first attribute with the specified name.
    cgString strKey = cgString::toLower( strAttribute );
    AttributeMap::iterator itAttribute = mData->childAttributeLUT.find( strKey );
    if ( itAttribute == mData->childAttributeLUT.end() )
        return false;

    // Return attribute data.
    strText = mData->childAttributes[itAttribute->second].getText();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getAttributeText ()
/// <summary>
/// Retrieve the text associated with the specified attribute. This
/// overload returns the text directly via the return value and will
/// return an empty string on failure.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgXMLNode::getAttributeText( const cgString & strAttribute ) const
{
    static const cgString strEmpty;
    
    // Empty attribute?
    if ( mData == CG_NULL )
        return strEmpty;

    // Rapid lookup for first attribute with the specified name.
    cgString strKey = cgString::toLower( strAttribute );
    AttributeMap::iterator itAttribute = mData->childAttributeLUT.find( strKey );
    if ( itAttribute == mData->childAttributeLUT.end() )
        return strEmpty;

    // Return attribute data.
    return mData->childAttributes[itAttribute->second].getText();
}

//-----------------------------------------------------------------------------
//  Name : getText ()
/// <summary>
/// Retrieve the text associated with this node.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgXMLNode::getText( )
{
    if ( mData == CG_NULL || mData->childText.size() == 0 )
        return cgString();

    // If the text data has already been cached, retrieve it.
    TextReference & ref = mData->childText[0];
    if ( ref.cached == true )
        return ref.data;
    
    // Anything to do?
    if ( ref.length == 0 )
        return cgString();

    // Otherwise, read it from the stream (slight portability warning;
    // string data is not required to be contiguous, in which case this
    // may fail. On all platforms we are developing for however this
    // should not be a problem).
    std::string strData;
    strData.resize( ref.length );
    cgInputStream & Stream = mData->streamData->stream;
    Stream.seek( ref.location, cgInputStream::Begin );
    Stream.read( &strData[0], ref.length );

    // Convert to unicode if required
    #if defined( UNICODE ) || defined( _UNICODE )
        STRING_CONVERT;
        return cgString(stringConvertA2CT( strData.c_str() ));
    #else // UNICODE
        return strData;
    #endif // !UNICODE
}

//-----------------------------------------------------------------------------
//  Name : processBinary ()
/// <summary>
/// Process the text of this node as binary data, decoding using the
/// specified approach where necessary.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLError::Result cgXMLNode::processBinary( cgXMLBinaryEncoding::Type Encoding, cgUInt32 & nDataSize, void * pDataOut )
{
    if ( mData == CG_NULL || mData->childText.size() == 0 )
        return cgXMLError::NoBinaryData;
    
    // Anything to do (length is required to be a multiple of 4)?
    TextReference & ref = mData->childText[0];
    if ( ref.length == 0 )
        return cgXMLError::NoBinaryData;

    // Clear output parameters where necessary
    if ( pDataOut == NULL )
        nDataSize = 0;

    // Seek to the relevant location in the stream.
    cgInputStream & Stream = mData->streamData->stream;
    Stream.seek( ref.location, cgInputStream::Begin );
    
    // Process until we're out of data.
    cgByte nDecodeByte1, nDecodeByte2, nChar;
    cgUInt32 nIndexOut = 0;
    for ( cgUInt32 i = 0; i < ref.length; i+=4 )
    {
        // Read next character
        Stream.read( &nChar, 1 );
        do { nDecodeByte1 = DecodeTable[ nChar ]; } while ( nDecodeByte1 == 97 );
        switch ( nDecodeByte1 )
        {
            case 98:
                return cgXMLError::IllegalBase64Char;
            case 99:
                return cgXMLError::Success;
            case 96:
                if ( pDataOut == NULL || nIndexOut == nDataSize )
                    return cgXMLError::Success;
                else
                    return cgXMLError::TruncatedData;
        
        } // End switch nDecodeByte1

        // Read next character
        Stream.read( &nChar, 1 );
        do { nDecodeByte2 = DecodeTable[ nChar ]; } while ( nDecodeByte2 == 97 );
        switch ( nDecodeByte2 )
        {
            case 96:
            case 99:
                return cgXMLError::TruncatedData;

        } // End switch nDecodeByte2

        // Out of data already?
        if ( pDataOut != NULL && nIndexOut == nDataSize )
            return cgXMLError::BufferTooSmall;
        
        // Store the first byte.
        if ( pDataOut != NULL )
            ((cgByte*)pDataOut)[nIndexOut++] = (nDecodeByte1 << 2) | ((nDecodeByte2 >> 4) & 0x3);
        else
            nDataSize++;

        // Read next character
        Stream.read( &nChar, 1 );
        do { nDecodeByte1 = DecodeTable[ nChar ]; } while ( nDecodeByte1 == 97 );
        switch ( nDecodeByte1 )
        {
            case 99:
                return cgXMLError::TruncatedData;
            case 96:
                if ( pDataOut == NULL || nIndexOut == nDataSize )
                    return cgXMLError::Success;
                else
                    return cgXMLError::TruncatedData;
            default:
                if ( pDataOut != NULL && nIndexOut == nDataSize )
                    return cgXMLError::BufferTooSmall;

        } // End switch nDecodeByte1

        // Store second byte
        if ( pDataOut != NULL )
            ((cgByte*)pDataOut)[nIndexOut++] = ((nDecodeByte2 << 4) & 0xF0) | ((nDecodeByte1 >> 2) & 0xF );
        else
            nDataSize++;

        // Read next character
        Stream.read( &nChar, 1 );
        do { nDecodeByte2 = DecodeTable[ nChar ]; } while ( nDecodeByte2 == 97 );
        switch ( nDecodeByte2 )
        {
            case 99:
                return cgXMLError::TruncatedData;
            case 96:
                if ( pDataOut == NULL || nIndexOut == nDataSize )
                    return cgXMLError::Success;
                else
                    return cgXMLError::TruncatedData;
            default:
                if ( pDataOut != NULL && nIndexOut == nDataSize )
                    return cgXMLError::BufferTooSmall;

        } // End switch nDecodeByte2

        // Store third byte
        if ( pDataOut != NULL )
            ((cgByte*)pDataOut)[nIndexOut++] = ((nDecodeByte1 << 6) & 0xC0) | nDecodeByte2;
        else
            nDataSize++;
        
    } // Next byte

    // Success!
    return cgXMLError::Success;
}

///////////////////////////////////////////////////////////////////////////////
// cgXMLAttribute Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgXMLAttribute () (Constructor)
/// <summary>
/// cgXMLAttribute Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgXMLAttribute::cgXMLAttribute()
{
    // Initialize variables
    mData = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgXMLAttribute () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLAttribute::cgXMLAttribute( const cgXMLAttribute & Attribute )
{
    // Initialize variables to sensible defaults
    mData = CG_NULL;
    
    // Perform necessary copy operation
    *this = Attribute;
}

//-----------------------------------------------------------------------------
//  Name : ~cgXMLAttribute () (Destructor)
/// <summary>
/// cgXMLAttribute Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgXMLAttribute::~cgXMLAttribute()
{
    // Close our reference to any node data.
    if ( mData != CG_NULL )
        mData->release();

    // Clear variables
    mData = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : operator= () (const cgXMLAttribute&)
/// <summary>
/// Overloaded assignment operator.
/// </summary>
//-----------------------------------------------------------------------------
cgXMLAttribute & cgXMLAttribute::operator=( const cgXMLAttribute & Attribute )
{
    // It's important that we increment the reference count
    // of the input attribute's data first in case it is a self
    // reference or referencing the same data.
    if ( Attribute.mData != CG_NULL )
        Attribute.mData->addRef();

    // Clean up our own internal data
    if ( mData != CG_NULL )
        mData->release();

    // Share the specified input attributes's data pointer
    mData = Attribute.mData;

    // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : isOfType ()
/// <summary>
/// Determine if this attribute is of the specified type (has a matching
/// name)
/// </summary>
//-----------------------------------------------------------------------------
bool cgXMLAttribute::isOfType( const cgString & strType ) const
{
    if ( isEmpty() == true )
        return false;
    return (getName().compare( strType, true ) == 0);
}

//-----------------------------------------------------------------------------
//  Name : isEmpty ()
/// <summary>
/// Determine if this attribute is empty (i.e. contains no data).
/// </summary>
//-----------------------------------------------------------------------------
bool cgXMLAttribute::isEmpty( ) const
{
    return (mData == CG_NULL);
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Retrieve the name of this attribute (i.e. the name="text")
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgXMLAttribute::getName( ) const
{
    static const cgString strEmpty;
    return ( mData == CG_NULL ) ? strEmpty : mData->name;
}

//-----------------------------------------------------------------------------
//  Name : getText ()
/// <summary>
/// Retrieve the text associated with this attribute.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgXMLAttribute::getText( ) const
{
    static const cgString strEmpty;
    return ( mData == CG_NULL ) ? strEmpty : mData->text;
}

//-----------------------------------------------------------------------------
//  Name : setName ()
/// <summary>
/// Set the name of this attribute (i.e. the name="text")
/// </summary>
//-----------------------------------------------------------------------------
void cgXMLAttribute::setName( const cgString & strName )
{
    if ( mData == CG_NULL )
        mData = new AttributeData();
    mData->name = strName;
}

//-----------------------------------------------------------------------------
//  Name : setText ()
/// <summary>
/// Set the text of this attribute (i.e. the name="text")
/// </summary>
//-----------------------------------------------------------------------------
void cgXMLAttribute::setText( const cgString & strText )
{
    if ( mData == CG_NULL )
        mData = new AttributeData();
    mData->text = strText;
}