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
// Name : cgXML.h                                                            //
//                                                                           //
// Desc : Utility classes that provide XML read/write support.               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGXML_H_ )
#define _CGE_CGXML_H_

//-----------------------------------------------------------------------------
// cgXML Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Namespaces
//-----------------------------------------------------------------------------
namespace cgXMLError
{
    enum Result
    {
        Success = 0,
        AlreadyOpen,
        AccessDenied,
        EndOfFile,
        NoDocumentTag,
        NoTagName,
        MismatchedQuote,
        MismatchedTag,
        UnexpectedToken,
        NoBinaryData,
        TruncatedData,
        BufferTooSmall,
        IllegalBase64Char
    };

} // End Namespace : cgXMLError

namespace cgXMLTokens
{
    enum Type
    {
        TagOpen = 0,
        TagClose,
        TagShortClose,
        TagStart,
        TagEnd,
        TagShort,
        Declaration,
        Equals,
        Text,
        QuotedText,
        Comment
    };

} // End Namespace : cgXMLTokens

namespace cgXMLBinaryEncoding
{
    enum Type
    {
        Base64 = 0
    };

}; // End Namespace : cgXMLBinaryEncoding

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgXMLDocument (Class)
/// <summary>
/// Top level XML document class through which hierarchical XML content
/// is accessed.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgXMLDocument
{
    //-------------------------------------------------------------------------
	// Friend List
	//-------------------------------------------------------------------------
    friend class cgXMLNode;
    friend class cgXMLAttribute;

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
     cgXMLDocument( );
     cgXMLDocument( const cgXMLDocument & doc );
    ~cgXMLDocument( );
    
    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    cgXMLError::Result  open                ( const cgInputStream & stream, const cgString & documentTag = _T(""), bool write = false );
    void                close               ( );
    const cgXMLNode   & getDocumentNode     ( ) const;

    //-------------------------------------------------------------------------
	// Public Static Methods
	//-------------------------------------------------------------------------
    static cgString     getErrorDescription ( cgXMLError::Result result );

    //-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    cgXMLDocument     & operator=           ( const cgXMLDocument & doc );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    // Internal, reference counted shared stream data structure.
    // This is managed separately to ensure the stream stays live
    // until everyone (including child nodes) have finished with it.
    struct CGE_API StreamData
    {
        cgInputStream   stream;             // stream from which node data is being read.
        cgUInt32        referenceCount;    // Number of node objects referencing this data structure.
        
        // Constructor
        StreamData( )
        {
            referenceCount = 1; // Important!
        }

        // Destructor
        ~StreamData( )
        {
            // We're the last reference. Close the stream.
            stream.close();
        }

        // Reference counting
        void addRef()
        {
            referenceCount++;
        }
        void release()
        {
            referenceCount--;
            if ( referenceCount == 0 )
                delete this;
        }
    
    }; // End struct StreamData
    
    //-------------------------------------------------------------------------
	// Private Methods
	//-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    StreamData    * mStreamData;    // Pointer to the information structure associated with this stream (potentially shared).
    cgXMLNode     * mDocumentNode;  // Pointer to the root document node.
};

//-----------------------------------------------------------------------------
//  Name : cgXMLAttribute (Class)
/// <summary>
/// An attribute value item associated with a given node.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgXMLAttribute
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
     cgXMLAttribute( );
     cgXMLAttribute( const cgXMLAttribute & attribute );
    ~cgXMLAttribute( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    bool                isEmpty             ( ) const;
    bool                isOfType            ( const cgString & type ) const;
    const cgString    & getName             ( ) const;
    const cgString    & getText             ( ) const;
    void                setName             ( const cgString & name );
    void                setText             ( const cgString & value );

    //-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    cgXMLAttribute    & operator=           ( const cgXMLAttribute & attribute );

    //-------------------------------------------------------------------------
	// Public Static Variables
	//-------------------------------------------------------------------------
    static const cgXMLAttribute Empty;

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    // Internal, reference counted shared attribute data structure
    struct CGE_API AttributeData
    {
        cgString                    name;               // Name of the attribute.
        cgString                    text;               // Text associated with the attribute.
        cgUInt32                    referenceCount;     // Number of node objects referencing this data structure
        cgXMLDocument::StreamData * streamData;         // Pointer to the stream information structure associated with the document.

        // Constructor
        AttributeData( )
        {
            streamData     = CG_NULL;
            referenceCount = 1;    // Important!
        }

        // Destructor
        ~AttributeData( )
        {
            if ( streamData != CG_NULL )
                streamData->release();
        }

        // Reference counting
        void addRef()
        {
            referenceCount++;
        }
        void release()
        {
            referenceCount--;
            if ( referenceCount == 0 )
                delete this;
        }
    
    }; // End struct AttributeData

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    AttributeData * mData;  // Pointer to the information structure associated with this attribute (potentially shared).
};

//-----------------------------------------------------------------------------
//  Name : cgXMLNode (Class)
/// <summary>
/// Individual node in the XML hierarchy.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgXMLNode
{
    //-------------------------------------------------------------------------
	// Friend List
	//-------------------------------------------------------------------------
    friend cgXMLDocument;

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
     cgXMLNode( );
     cgXMLNode( const cgXMLNode & node );
    ~cgXMLNode( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    bool                isEmpty             ( ) const;
    bool                isOfType            ( const cgString & type ) const;
    const cgString    & getName             ( ) const;
    cgString            getText             ( );
    cgUInt32            getChildNodeCount   ( ) const;
    const cgXMLNode   & getChildNode        ( cgUInt32 index ) const;
    const cgXMLNode   & getChildNode        ( const cgString & name ) const;
    const cgXMLNode   & getNextChildNode    ( const cgString & name, cgUInt32 & startIndex ) const;
    void                setName             ( const cgString & name );
    bool                definesAttribute    ( const cgString & attributeName ) const;
    bool                getAttributeText    ( const cgString & attributeName, cgString & textOut ) const;
    const cgString    & getAttributeText    ( const cgString & attributeName ) const;
    cgXMLError::Result  processBinary       ( cgXMLBinaryEncoding::Type Encoding, cgUInt32 & nDataSize, void * pDataOut );

    //-------------------------------------------------------------------------
	// Public Operators
	//-------------------------------------------------------------------------
    cgXMLNode         & operator=           ( const cgXMLNode & node );

    //-------------------------------------------------------------------------
	// Public Static Variables
	//-------------------------------------------------------------------------
    static const cgXMLNode Empty;

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    CGE_ARRAY_DECLARE(cgXMLNode, NodeArray)
    CGE_ARRAY_DECLARE(cgXMLAttribute, AttributeArray)
    CGE_MAP_DECLARE  (cgString, size_t, AttributeMap)
    CGE_MAP_DECLARE  (cgString, size_t, NodeMap)

    // We try to avoid maintaining text data internally in an
    // attempt to save memory. Instead, information about its
    // location in the file is stored and read on demand.
    struct TextReference
    {
        cgInt64     location;
        cgUInt32    length;
        cgString    data;
        bool        cached;
    
    }; // End Struct : TextReference
    CGE_ARRAY_DECLARE(TextReference, TextArray)
    
    // Internal, reference counted shared node data structure
    struct CGE_API NodeData
    {
        cgString                    name;               // Name of the node / element.
        NodeArray                   childNodes;         // Array of child nodes.
        NodeMap                     childNodeLUT;       // Provides a rapid lookup table for the first node with a given name.
        AttributeArray              childAttributes;    // Array of child attributes.
        AttributeMap                childAttributeLUT;  // Provides a rapid lookup table for the first attribute with a given name.
        TextArray                   childText;          // Array of child text fields.
        cgUInt32                    referenceCount;     // Number of node objects referencing this data structure
        cgXMLDocument::StreamData * streamData;         // Pointer to the stream information structure associated with the document.

        // Constructor
        NodeData( )
        {
            streamData     = CG_NULL;
            referenceCount = 1;    // Important!
        }

        // Destructor
        ~NodeData( )
        {
            if ( streamData != CG_NULL )
                streamData->release();
        }

        // Reference counting
        void addRef()
        {
            referenceCount++;
        }
        void release()
        {
            referenceCount--;
            if ( referenceCount == 0 )
                delete this;
        }
    
    }; // End struct NodeData

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    cgXMLError::Result  parseElement        ( cgXMLDocument::StreamData * stream, cgXMLNode * parentNode );
    cgXMLError::Result  getNextToken        ( bool insideTag, bool processText, cgXMLTokens::Type & tokenType, cgString & data, cgInt64 & tokenStart );
    bool                getNextNonWhitespace( cgTChar & c );
    
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    NodeData  * mData;  // Pointer to the information structure associated with this node (potentially shared).
};

#endif // !_CGE_CGXML_H_