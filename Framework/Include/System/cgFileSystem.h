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
// Name : cgFileSystem.h                                                     //
//                                                                           //
// Desc : Classes which provide support for handling resource files,         //
//        directories and data packages.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGFILESYSTEM_H_ )
#define _CGE_CGFILESYSTEM_H_

//-----------------------------------------------------------------------------
// cgFileSystem Header Includes
//-----------------------------------------------------------------------------
#include <cgAPI.h>
#include <cgBaseTypes.h>
#include <fstream>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgDataPackage;
class cgInputStream;

//-----------------------------------------------------------------------------
// Global Enumerations
//-----------------------------------------------------------------------------
namespace cgStreamType
{
    enum Base
    {
        None = 0,                       // No stream specified
        File,                           // Standard local file on disk
        RemoteFile,                     // File that exists on a remote server (i.e. HTTP) * Not currently supported
        MappedFile,                     // Part of an existing local file given an offset / length
        Memory                          // Memory buffer.
    };

}; // End Namespace : cgStreamType

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgFileSystem (Static Class)
/// <summary>
/// Class that contains our core file system handling functionality.
/// Access is via entirely static methods to provide support application
/// wide.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFileSystem
{
public:
    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static bool     setRootDirectory    ( const cgString & directoryName );
    static bool     addPathProtocol     ( const cgString & protocol, const cgString & directoryName );
    static bool     addPackage          ( const cgString & packageFile );
    static bool     index               ( bool autoAddPackages = false, const cgString & packageExtension = _T("pkg") );
    static cgString resolveFileLocation ( const cgString & fileName );
    static bool     getPackageFile      ( const cgString & packageReference, cgInputStream & stream );
    
    // Utility methods
    static cgString loadStringFromStream( const cgInputStream & stream );
    static cgString getAbsolutePath     ( const cgString & pathFile, const cgString & currentDirectory = cgString::Empty );
    static cgString getRelativePath     ( const cgString & pathFile, const cgString & currentDirectory = cgString::Empty );
    static cgString getFileNameExtension( const cgString & pathFile );
    static cgString getFileName         ( const cgString & pathFile );
    static cgString getFileName         ( const cgString & pathFile, bool stripExtension );
    static cgString getDirectoryName    ( const cgString & pathFile );
    static cgString getAppDirectory     ( );
    static bool     fileExists          ( const cgString & pathFile );
    static bool     directoryExists     ( const cgString & directoryName );
    static bool     createDirectory     ( const cgString & directoryName );
    static bool     pathProtocolDefined ( const cgString & protocol );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgString, cgString, ProtocolMap)
    CGE_VECTOR_DECLARE      (cgDataPackage*, PackageArray)
    
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static void     findPackages        ( const cgString & searchPath, const cgString & packageExtension );

    //-------------------------------------------------------------------------
    // Private Static Variables.
    //-------------------------------------------------------------------------
    static bool             mIndexed;           // True if the file system has been indexed.
    static cgString         mRootDirectory;     // The main file system root directory.
    static cgStringArray    mPackageFiles;      // List of all of the package files that should be indexed.
    static PackageArray     mPackages;          // Vector containing the actual instantiated package objects.
    static ProtocolMap      mProtocols;         // A map containing all of the protocols (i.e. sys) and the directory that they link to.
};

//-----------------------------------------------------------------------------
//  Name : cgInputStream (Class)
/// <summary>
/// Core file system class which provides largely seamless access to data
/// irrespective of the location of the source (i.e. file, offset within
/// an existing file, or memory buffer).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgInputStream
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    enum SeekOrigin
    {
        Current = 0,
        Begin   = 1,
        End     = 2
    
    }; // End enum SeekOrigin

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgInputStream( );
             cgInputStream( const cgInputStream & stream );
             cgInputStream( const cgChar * fileName );
             cgInputStream( const cgWChar * fileName );
             cgInputStream( const cgString & fileName );
             cgInputStream( const cgString & fileName, const cgString & streamReferenceName );
             cgInputStream( void * buffer, size_t bufferLength );
             cgInputStream( void * buffer, size_t bufferLength, const cgString & streamRefenceName );
             cgInputStream( const cgInputStream & containerStream, size_t offset, size_t length );
             cgInputStream( const cgInputStream & containerStream, size_t offset, size_t length, const cgString & streamReferenceName );
    virtual ~cgInputStream( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // Stream selection.
    bool                setStreamSource ( const cgString & fileName );
    bool                setStreamSource ( const cgString & fileName, const cgString & streamReferenceName );
    bool                setStreamSource ( void * buffer, size_t bufferLength );
    bool                setStreamSource ( void * buffer, size_t bufferLength, const cgString & streamReferenceName );
    bool                setStreamSource ( const cgInputStream & containerStream, size_t offset, size_t length );
    bool                setStreamSource ( const cgInputStream & containerStream, size_t offset, size_t length, const cgString & streamReferenceName );
    void                reset           ( );

    // File like access.
    bool                open            ( );
    void                close           ( );
    size_t              read            ( void * buffer, size_t bufferLength );
    cgString            readString      ( size_t length );
    bool                seek            ( cgInt64 offset, SeekOrigin origin = Current );
    
    // Pointer based access.
    cgByte            * getBuffer       ( size_t & dataLength, size_t accessOffset = 0, size_t accessSize = 0 );
    void                releaseBuffer   ( );

    // Query Methods
    cgString            getName         ( ) const;
    cgString            getSourceFile   ( ) const;
    bool                sourceExists    ( ) const;

    // Hashing
    bool                computeSHA1     ( cgUInt32 hash[] );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : readByte ()
    /// <summary>
    /// Read an 8 bit unsigned byte (integer) from the open stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgByte readByte( )
    {
        cgByte data;
        if ( read( &data, 1 ) < 1 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : readInt8 ()
    /// <summary>
    /// Read an 8 bit signed integer from the open stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgInt8 readInt8( )
    {
        cgInt8 data;
        if ( read( &data, 1 ) < 1 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : readUInt8 ()
    /// <summary>
    /// Read an 8 bit unsigned integer from the open stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt8 readUInt8( )
    {
        cgUInt8 data;
        if ( read( &data, 1 ) < 1 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : readInt16 ()
    /// <summary>
    /// Read a 16 bit signed integer from the open stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgInt16 readInt16( )
    {
        cgInt16 data;
        if ( read( &data, 2 ) < 2 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : readUInt16 ()
    /// <summary>
    /// Read a 16 bit unsigned integer from the open stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt16 readUInt16( )
    {
        cgUInt16 data;
        if ( read( &data, 2 ) < 2 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : readInt32 ()
    /// <summary>
    /// Read a 32 bit signed integer from the open stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgInt32 readInt32( )
    {
        cgInt32 data;
        if ( read( &data, 4 ) < 4 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : readUInt32 ()
    /// <summary>
    /// Read a 32 bit unsigned integer from the open stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32 readUInt32( )
    {
        cgUInt32 data;
        if ( read( &data, 4 ) < 4 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : readSingle ()
    /// <summary>
    /// Read a 32 bit, single precision, floating point value from the open 
    /// stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgFloat readSingle( )
    {
        cgFloat data;
        if ( read( &data, 4 ) < 4 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : readDouble()
    /// <summary>
    /// Read a 64 bit, double precision, floating point value from the open 
    /// stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgDouble readDouble( )
    {
        cgDouble data;
        if ( read( &data, 8 ) < 8 )
            return 0;
        return data;
    }

    //-------------------------------------------------------------------------
    //  Name : getPosition () (Virtual)
    /// <summary>
    /// Retrieve the current position within the stream in bytes.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgInt64 getPosition( ) const
    {
        return mCurrentPosition;
    }

    //-------------------------------------------------------------------------
    //  Name : getLength ()
    /// <summary>
    /// Retrieve the total length of the stream in bytes.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgInt64 getLength( ) const
    {
        return mStreamLength;
    }

    //-------------------------------------------------------------------------
    //  Name : getType ()
    /// <summary>
    /// Determine the type of this input stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgStreamType::Base getType( ) const
    {
        if ( mData == CG_NULL )
            return cgStreamType::None;
        return mData->type;
    }

    //-------------------------------------------------------------------------
    //  Name : isOpen ()
    /// <summary>
    /// Determine if the stream is currently open.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isOpen( ) const
    {
        return ( mStreamLength != 0 );
    }

    //-------------------------------------------------------------------------
    //  Name : isEOF ()
    /// <summary>
    /// Determine if we have reached (or exceeded) the end of the stream.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isEOF( ) const
    {
        return ( mStreamLength == 0 || mCurrentPosition >= mStreamLength );
    }

    //-------------------------------------------------------------------------
    // Public Operator Overloads
    //-------------------------------------------------------------------------
    cgInputStream & operator= ( const cgInputStream & Stream );

private:
    //-------------------------------------------------------------------------
    // Private Constants
    //-------------------------------------------------------------------------
    static const size_t ReadBufferSize = 1024;

    //-------------------------------------------------------------------------
    // Private Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    // Internal, reference counted shared stream data structure
    struct StreamData
    {
        cgStreamType::Base  type;               // What type of stream is this.
        cgString            sourceFile;         // The source file (or container in the case of mapped file)
        cgByte            * sourceBuffer;       // The source memory buffer (when type == Memory).
        size_t              offset;             // Offset within the container when type == MappedFile
        size_t              length;             // The length to read from within the container when type == MappedFile, or length of memory buffer when type == Memory
        cgUInt32            accessCount;        // Number of references which currently maintain access to stream via getBuffer
        cgUInt32            referenceCount;     // Number of InputStream objects referencing this data structure

        // Memory mapped Lock() data.
        void              * fileHandle;         // Handle to the opened file
        void              * mappingHandle;      // Handle to the page mapping
        void              * mappedView;         // Pointer to the starting address of the mapped file view
        cgByte            * mappedFile;         // Actual base pointer to the mapped file offset to the correct location (if succesful)
        cgByte            * memoryBuffer;       // If we're forced to load into memory during getBuffer() call, this is the buffer allocation
        size_t              mapLength;          // The length (in bytes) of the mapping we're maintaining.
    };

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    void                releaseDataReference    ( );
    void                allocateDataStructure   ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    StreamData    * mData;              // Pointer to the information structure associated with this tream (Potentially Shared)
    cgUInt32        mAccessCount;       // Number of access references we are actually holding to the data.
    cgString        mStreamName;        // Original name of the stream source (valid or otherwise).
    
    std::ifstream   mFileStream;        // The stream used for reading from file data.
    cgInt64         mCurrentPosition;   // Current position within the above opened stream (faster than tellg() in some circumstances).
    cgInt64         mStreamOffset;      // The offset within the data that the stream starts
    cgInt64         mStreamLength;      // Cached copy of the stream length.
    cgByte        * mBufferOffset;      // Contains current offset for reading for memory streams.
    cgByte        * mBufferedData;      // When reading from file, this contains a buffered section of the file for improved reading performance.

};

//-----------------------------------------------------------------------------
//  Name : cgDataPackage (Class)
/// <summary>
/// Provides support for packaging multiple data files within one single
/// file on disk.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDataPackage
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDataPackage( const cgString & packageFile );
    virtual ~cgDataPackage( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool    isValid         ( ) const;
    bool    isIndexed       ( ) const;
    bool    index           ( );
    bool    appendFile      ( const cgString & fileName, const cgString & packageReference );
    bool    getFile         ( const cgString & packageReference, cgInputStream & stream );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    struct FileInfo
    {
        cgString    pathName;           // The package path in which the file is stored
        cgString    fileName;           // The filename only portion of the reference.
        cgUInt32    flags;              // Any flag bits set for this file.
        size_t      offset;             // Offset (in bytes) within the package to get to this file.
        size_t      packagedLength;     // The length (in bytes) of the file within the package
        size_t      originalLength;     // The original length (in bytes) of the file before being added to the package.

    }; // End Struct FileInfo
    CGE_MAP_DECLARE(cgString, FileInfo, FileMap)
    
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    void    writeIndex      ( std::ofstream & package );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    bool            mIndexed;           // Has this package been indexed yet.
    cgString        mPackageFileName;   // The path and filename of the package file itself.
    FileMap         mFiles;             // Map containing information about each file in the package
    size_t          mIndexTableOffset;  // The offset within the package that the index table exists
    cgUID           mPackageUID;        // Unique identifier of the package (generated on creation).

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgByte   mMagicNumber[8];    // The magic number we should see in packages
    static cgByte   mIndexStartSig[4];  // The signature signifying the start of the index block
    static cgByte   mIndexEndSig[4];    // The signature signifying the end of the index block
};

#endif // !_CGE_CGFILESYSTEM_H_