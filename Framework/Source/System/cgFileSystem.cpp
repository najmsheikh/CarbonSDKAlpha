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
// Name : cgFileSystem.cpp                                                   //
//                                                                           //
// Desc : Classes which provide support for handling resource files,         //
//        directories and data packages.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgFileSystem Module Includes
//-----------------------------------------------------------------------------
#include <System/cgFileSystem.h>
#include <System/cgAppLog.h>
#include <Math/cgChecksum.h>    // computeSHA1

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>            // Warning: Portability
#include <shlobj.h>             // Warning: Portability
#undef WIN32_LEAN_AND_MEAN

// Runtime includes
#include <io.h>                 // For access().
#include <sys/types.h>          // For stat().
#include <sys/stat.h>           // For stat().
#include <algorithm>
#include <tchar.h>

// Remove windows #DEFINE interference
#undef createDirectory

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
// cgFileSystem
bool                        cgFileSystem::mIndexed    = false;
cgString                    cgFileSystem::mRootDirectory;
cgStringArray               cgFileSystem::mPackageFiles;
cgFileSystem::PackageArray  cgFileSystem::mPackages;
cgFileSystem::ProtocolMap   cgFileSystem::mProtocols;

// cgDataPackage
cgByte                      cgDataPackage::mMagicNumber[8]   = { 'C','G','E','P','K','G','V','1' };
cgByte                      cgDataPackage::mIndexStartSig[4] = { 'I','D','X','S' };
cgByte                      cgDataPackage::mIndexEndSig[4]   = { 'I','D','X','E' };

///////////////////////////////////////////////////////////////////////////////
// cgFileSystem Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : setRootDirectory () (Static)
/// <summary>
/// Set the directory that the file system will consider to be its root.
/// When indexing, this will be the base path in which we recurse.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::setRootDirectory( const cgString & strDir )
{
    // Duplicate the data path (trim off any white space)
    mRootDirectory = cgString::trim(strDir);

    // Skip if empty
    if ( mRootDirectory.empty() )
        return true;

    // No trailing slash?
    if ( mRootDirectory.substr(mRootDirectory.length() - 1) != _T("\\") && 
         mRootDirectory.substr(mRootDirectory.length() - 1) != _T("/") )
    {
        // Append a slash to the end of the data path
        mRootDirectory += _T("\\");
    
    } // End if no trailing slash

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addPathProtocol () (Static)
/// <summary>
/// Allows us to map a protocol to a specific directory. A path protocol
/// gives the caller the ability to prepend an identifier to their file
/// name i.e. "sys://Effects/MyEffect.fx" and have it return the
/// relevant mapped path.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::addPathProtocol( const cgString & strProtocol, const cgString & strDir )
{
    // Duplicate the data directory (trim off any white space)
    cgString strNewDir      = cgString::trim(strDir);
    cgString strNewProtocol = cgString::trim(strProtocol);
    
    // Protocol matching is case insensitive, convert to lower case
    strNewProtocol.toLower();

    // Append a slash to the end of the data path if required
    if ( !strNewDir.empty() && 
         !strNewDir.endsWith( _T("\\") ) &&
         !strNewDir.endsWith( _T("/") ) )
        strNewDir.append( _T("\\") );
    
    // Add to the list
    mProtocols[strNewProtocol] = strNewDir;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pathProtocolDefined () (Static)
/// <summary>
/// Determine if a path protocol with the specified name has been defined.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::pathProtocolDefined( const cgString & strProtocol )
{
    // Protocol matching is case insensitive, convert to lower case
    const cgString strKey = cgString::toLower( strProtocol );
    return (mProtocols.find( strKey ) != mProtocols.end());
}

//-----------------------------------------------------------------------------
//  Name : addPackage () (Static)
/// <summary>
/// Add a specific package file to the system for indexing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::addPackage( const cgString & strPackage )
{
    cgDataPackage * pNewPackage = CG_NULL;

    // Normalize the filename
    cgString strPackageFile = getAbsolutePath( resolveFileLocation( strPackage ) );

    // Case-insensitive search etc. Make lower case.
    strPackageFile.toLower();

    // Is this package already in the list?
    cgStringArray::iterator itFile;
    for ( itFile = mPackageFiles.begin(); itFile != mPackageFiles.end(); ++itFile )
    {
        // Already exists?
        if ( *itFile == strPackageFile )
            return true;
    
    } // Next File

    // Check if the specified package file exists
    if ( _taccess( strPackageFile.c_str(), 0 ) != 0 )
        return false;

    // Create a new package object for this file
    pNewPackage = new cgDataPackage( resolveFileLocation( strPackage ) );

    // Determine if the package really is a valid file for us
    if ( !pNewPackage->isValid() )
    {
        delete pNewPackage;
        return false;
    
    } // End if invalid format

    // If the file system has already run its index pass, request that the
    // new package indexes too.
    if ( mIndexed )
    {
        if ( !pNewPackage->index() )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to index package file '%s'. The package is possibly corrupt or incomplete.\n"), strPackage.c_str() );
            delete pNewPackage;
            return false;
        
        } // End if failed
    
    } // End if indexed
    
    // Store package details
    mPackageFiles.push_back( strPackageFile );
    mPackages.push_back( pNewPackage );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : index () (Static)
/// <summary>
/// Index all of the files from the registered packages and optionally
/// the packages found in subdirectories of the root path.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::index( bool bAutoAddPackages /* = false */, const cgString & strPackageExtension /* = _T("pkg") */ )
{
    // First, if requested, recursively search for valid packages to add
    if ( bAutoAddPackages )
        findPackages( mRootDirectory, strPackageExtension );

    // Now index all packages if they are not already
    for ( size_t i = 0; i < mPackages.size(); ++i )
    {
        cgDataPackage * pPackage = mPackages[i];
        if ( pPackage->isIndexed() ) continue;

        // Index the package
        if ( !pPackage->index() )
        {
            // Remove the package from the list
            delete pPackage;
            mPackages.erase( mPackages.begin() + i );
            mPackageFiles.erase( mPackageFiles.begin() + i );
            i--;
        
        } // End if failed to index

    } // Next Package

    // We have been indexed
    mIndexed = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findPackages () (Private, Static, Recursive)
/// <summary>
/// Search through the specified directory tree and add all valid
/// packages.
/// </summary>
//-----------------------------------------------------------------------------
void cgFileSystem::findPackages( const cgString & strSearchPath, const cgString & strPackageExtension )
{
    WIN32_FIND_DATA FindData;
    bool            bExtensionMatch;
    cgString        strSearchString = strSearchPath + _T("*.*");
    
    // Clear FindData
    memset( &FindData, 0, sizeof(WIN32_FIND_DATA));

    // Get the first file in the specified path that matches the wildcard
    HANDLE FindHandle = FindFirstFile( strSearchString.c_str(), &FindData );

    // If we succeeded then lets go.
    if ( FindHandle != INVALID_HANDLE_VALUE )
    {
        do
        {
            // Keep Searching until we run out of files
            strSearchString  = strSearchPath + FindData.cFileName;

            // Check if this is a valid directory
            cgUInt32 FileAttributes = GetFileAttributes( strSearchString.c_str() );
            if ( FileAttributes != 0xFFFFFFFF )
            {
                if ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    // Ensure that this is not current or previous dir.
                    cgString DirName = FindData.cFileName;
                    if (!(DirName == _T(".") || DirName == _T("..")))
                        findPackages( strSearchString + _T("\\"), strPackageExtension );
                
                } // End if directory
                else
                {
                    // Ensure the extension matches if supplied
                    bExtensionMatch = true;
                    if ( !strPackageExtension.empty() )
                    {
                        // Retrieve the extension of the file
                        cgString strExtensionSrc   = getFileNameExtension( strSearchString );
                        cgString strExtensionMatch = strPackageExtension;

                        // Ensure the extension matches if supplied (case insensitive)
                        if ( strExtensionSrc.compare( strExtensionMatch, true ) != 0 )
                            bExtensionMatch = false;

                    } // End if extensions supplied

                    // Add to our list of available packages
                    if ( bExtensionMatch )
                        addPackage( _T("currentdir://") + strSearchString );
                
                } // End if file

            } // End if valid file / dir

        } while ( FindNextFile( FindHandle, &FindData ) );

        // Close up
        FindClose( FindHandle );

    } // End if handle was valid
}

//-----------------------------------------------------------------------------
//  Name : resolveFileLocation() (Static)
/// <summary>
/// Given the specified path/filename, resolve the final full filename.
/// This will be based on either the currently specified root path,
/// or one of the 'path protocol' identifiers if it is included in the
/// filename.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::resolveFileLocation( const cgString & strFile )
{
    cgString::size_type   nSeparator;
    ProtocolMap::iterator itProtocol;
    cgString              strLocation, strProtocol;

    // Case insensitive test please
    strLocation = cgString::toLower( strFile );

    // Includes a path protocol at the start?
    nSeparator = strLocation.find( _T("://") );
    if ( nSeparator != cgString::npos )
    {
        // Extract the protocol
        strProtocol = strLocation.substr( 0, nSeparator );
        strLocation = strFile.substr( nSeparator + 3 );

        // Matching path protocol in our list?
        itProtocol = mProtocols.find( strProtocol );
        if ( itProtocol == mProtocols.end() )
            return strFile;

        // Found matching path protocol
        return itProtocol->second + strLocation;

    } // End if includes protocol
    else if ( strLocation.length() > 2 && strLocation[1] == _T(':') )
    {
        // This is an absolute drive path (i.e. c:\Test.jpg).
        return strFile;
    
    } // End if absolute path

    // No protocol, just return with root path pre-pended
    return mRootDirectory + strFile;    
}

//-----------------------------------------------------------------------------
//  Name : getPackageFile () (Static)
/// <summary>
/// Retrieve an input stream reference for the specified file within
/// any of the indexed packages (if it exists).
/// Note : If two separate packages contain references for the same file, only
/// the first will be returned.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::getPackageFile( const cgString & strPackageReference, cgInputStream & Stream )
{
    // Iterate through registered packages
    for ( size_t i = 0; i < mPackages.size(); ++i )
    {
        cgDataPackage * pPackage = mPackages[i];
        if ( pPackage == CG_NULL ) continue;

        // Request file
        if ( pPackage->getFile( strPackageReference, Stream ) )
            return true;
    
    } // Next Package

    // Reference not found.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : loadStringFromStream () (Static)
/// <summary>
/// Load a string with the contents of the specified file, be that file in a
/// package or in the main file system.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::loadStringFromStream( const cgInputStream & File )
{
    // Open the stream
    cgInputStream Stream = File;
    if ( !Stream.open() )
        return cgString::Empty;

    // Read contents into a string.
    cgChar * pBuffer = new cgChar[(size_t)Stream.getLength()+1];
    Stream.read( pBuffer, (size_t)Stream.getLength() );
    pBuffer[Stream.getLength()] = '\0';

    // Close the stream and set as our string content
    STRING_CONVERT;
    Stream.close();
    cgString strOut = stringConvertA2CT( pBuffer );

    // Finish up
    delete []pBuffer;

    // Done
    return strOut;
}

//-----------------------------------------------------------------------------
//  Name : getAbsolutePath () (Static)
/// <summary>
/// This function will return a path (and file) which is the absolute
/// version of the path specified relative to the current directory.
/// Note : This function is mildly expensive and should be used sparingly.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::getAbsolutePath( const cgString & strPathFile, const cgString & strCurrentDir /* = cgString::Empty */ )
{
    cgString CurrentPath, RelativePath;
    cgTChar  OldCD[MAX_PATH], AbsolutePath[MAX_PATH];
  
    // Trim off any white space
    CurrentPath  = cgString::trim( strCurrentDir );
    RelativePath = cgString::trim( strPathFile );

    // Replace all forward slashes, with back slashes
    CurrentPath.replace( _T('/'), _T('\\') );
    RelativePath.replace( _T('/'), _T('\\') );
    
    // Strip out ANY double back slashes as these are invalid
    CurrentPath.replace( _T("\\\\"), _T("\\") );
    RelativePath.replace( _T("\\\\"), _T("\\"));

    // Change the current directory (if overriding)
    if ( !strCurrentDir.empty() )
    {
        // Backup old current dir
        if (!_tgetcwd( OldCD, MAX_PATH ))
            return cgString::Empty;

        // Set the current directory for expansion
        if (_tchdir(CurrentPath.c_str()) == -1 )
            return cgString::Empty;
    
    } // End if overriding dir

    // Expand the relative path
    _tfullpath( AbsolutePath, RelativePath.c_str(), MAX_PATH );

    // Restore the previous current directory (if overriden)
    if ( !strCurrentDir.empty() )
        _tchdir( OldCD );

    // Return the absolute path
    return cgString( AbsolutePath );
}

//-----------------------------------------------------------------------------
//  Name : getRelativePath () (Static)
/// <summary>
/// This function will return a path (and file) relative to the path 
/// specified in the CurrentDir parameter.
/// Note : This function is mildly expensive and should be used sparingly.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::getRelativePath( const cgString & strPathFile, const cgString & strCurrentDir /* = cgString::Empty */ )
{
    cgString CurrentPath  = strCurrentDir;
    cgString AbsolutePath = strPathFile;
    cgString RelativePath;
    cgString::size_type i; 

    // If the specified current directory is empty, use the actual current dir
    if ( strCurrentDir.empty() )
    {
        cgTChar Buffer[MAX_PATH];
        if (!_tgetcwd( Buffer, MAX_PATH ))
            return cgString::Empty;
        CurrentPath = Buffer;
    
    } // End if no override dir
  
    // Ensure both are the same case
    CurrentPath.trim().toLower();
    AbsolutePath.trim().toLower();

    // Has any length at all after the trim etc
    if ( AbsolutePath.length() == 0 )
        return strPathFile;

    // Replace all forward slashes, with back slashes
    CurrentPath.replace( _T('/'), _T('\\') );
    AbsolutePath.replace( _T('/'), _T('\\') );
    
    // Strip out ANY double back slashes as these are invalid
    CurrentPath.replace( _T("\\\\"), _T("\\"));
    AbsolutePath.replace( _T("\\\\"), _T("\\"));
    
    // Handle names that are on different drives
    if ( CurrentPath[0] != AbsolutePath[0])
    {
        // These are not on the same drive so only absolute will do
        return strPathFile;

    } // End If Not Same Drive

    // Determine how much of both buffers are identical
    for ( i = 0; i < CurrentPath.length() && i < AbsolutePath.length(); i++ )
    {
        if ( CurrentPath[i] != AbsolutePath[i] )
            break;

    } // Next Character
  
    // Did we get to the end of the source??
    if ( i == CurrentPath.length() && (AbsolutePath[i] == _T('\\') || AbsolutePath[i-1] == _T('\\')))
    {
        // The whole source directory name is in the target name so we just trim and return
        // Trim off any leading slash first however.
        if (AbsolutePath[i] == _T('\\'))
            i++;
        return strPathFile.substr( i );

    } // End If

    // The file is not in a child directory of the current so we
    // need to step back the appropriate number of parent directories
    // First we will find out how many leves deeper we are than the source.
    cgString::size_type afMarker = i;
    cgString::size_type levels   = 1;
    while (i < CurrentPath.length())
    {
        // We've found a slash
        if (CurrentPath[i] == _T('\\'))
        {
            // Ensure it's not a trailing slash
            i++;
            if (i != CurrentPath.length())
                levels++;

        } // End If
        i++;
    } // Next Character

    // Move the absolute filename marker back to the start of the dir name that it stopped in
    while ( afMarker > 0 && AbsolutePath[afMarker-1] != _T('\\'))
        afMarker--;

    // Add the appropriate number of "..\"s.
    for (i = 0; i < levels; i++)
        RelativePath += _T("..\\");

    // Copy the rest of the filename into the result string
    RelativePath += strPathFile.substr( afMarker );

    // Return our relative path
    return RelativePath;

}

//-----------------------------------------------------------------------------
//  Name : getDirectoryName () (Static)
/// <summary>
/// Given a full path name, return just the direction portion of it.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::getDirectoryName( const cgString & strPathFile )
{
    cgString::size_type nLastSlash, nLastSlash2;

    // Return the path portion only
    nLastSlash = strPathFile.rfind(_T('\\'));
    nLastSlash2 = strPathFile.rfind(_T('/'));
    if ( nLastSlash == cgString::npos || (nLastSlash2 != cgString::npos && nLastSlash2 > nLastSlash) )
        nLastSlash = nLastSlash2;
    
    // Any slash found?
    if ( nLastSlash != cgString::npos )
        return strPathFile.substr( 0, nLastSlash );
    
    // No path
    return cgString::Empty;
}

//-----------------------------------------------------------------------------
//  Name : getFileName () (Static)
/// <summary>
/// Given a full path name, return just the filename portion of it.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::getFileName( const cgString & strPathFile )
{
    cgString              strFileName = strPathFile;
    cgString::size_type   nLastSlash;

    // Get filename only portion of the specified path
    nLastSlash = strPathFile.rfind(_T('\\'));
    if ( nLastSlash == cgString::npos ) nLastSlash = strPathFile.rfind(_T('/'));
    if ( nLastSlash != cgString::npos ) strFileName = strPathFile.substr( nLastSlash + 1 );

    // Return result
    return strFileName;
}

//-----------------------------------------------------------------------------
//  Name : getFileName () (Static)
/// <summary>
/// Given a full path name, return just the filename portion of it. Optionally
/// the extension can be automatically stripped.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::getFileName( const cgString & strPathFile, bool bStripExtension )
{
    cgString              strFileName = strPathFile;
    cgString::size_type   nLastSlash;

    // Get filename only portion of the specified path
    nLastSlash = strPathFile.rfind(_T('\\'));
    if ( nLastSlash == cgString::npos ) nLastSlash = strPathFile.rfind(_T('/'));
    if ( nLastSlash != cgString::npos ) strFileName = strPathFile.substr( nLastSlash + 1 );

    // Strip extension?
    if ( bStripExtension )
    {
        // Find the portion of the specified file
        cgString::size_type nLastDot = strFileName.rfind(_T('.'));
        if ( nLastDot != cgString::npos )
            strFileName = strFileName.substr( 0, nLastDot );

    } // End if strip extension

    // Return result
    return strFileName;
}

//-----------------------------------------------------------------------------
//  Name : getFileNameExtension () (Static)
/// <summary>
/// Given a full path name, return just the extension portion of it.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::getFileNameExtension( const cgString & strPathFile )
{
    cgString            strFileName;
    cgString::size_type nLastDot;

    // First ensure we only have the file name portion of a full path+file
    // string so that we don't mistake a 'dot' in the path as the extension
    // delimeter.
    strFileName = getFileName( strPathFile );

    // Get extension only portion of the specified file
    nLastDot = strFileName.rfind(_T('.'));
    if ( nLastDot == cgString::npos ) return cgString::Empty;

    // Return only the extension
    return strFileName.substr( nLastDot + 1 );
}

//-----------------------------------------------------------------------------
//  Name : getAppDirectory() (Static)
/// <summary>
/// Retrieve the directory of the currently running application.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgFileSystem::getAppDirectory()
{
    cgTChar strBuffer[MAX_PATH];

    // Retrieve the module name for the process we're running in
    ::GetModuleFileName( CG_NULL, strBuffer, MAX_PATH - 1 );
    
    // Return the direction portion only
    return getDirectoryName( strBuffer ) + _T("\\");
}

//-----------------------------------------------------------------------------
//  Name : fileExists () (Static)
/// <summary>
/// Given a full path name, determine if it exists (and is a file).
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::fileExists( const cgString & strPathFile )
{
    STRING_CONVERT;

    // First, let's see if the referenced item actually exists on disk
    if ( _taccess( strPathFile.c_str(), 0 ) == 0 )
    {
        // Item exists. Is it a file or directory?
        struct stat status;
        stat( stringConvertT2CA(strPathFile.c_str()), &status );

        // Must not indicate directory.
        if ( status.st_mode & S_IFDIR )
            return false;

        // Item is a file
        return true;
    
    } // End if exists

    // File does not exist
    return false;
}

//-----------------------------------------------------------------------------
//  Name : directoryExists () (Static)
/// <summary>
/// Given a full directory name, determine if it exists (and is a directory).
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::directoryExists( const cgString & strDirectory )
{
    STRING_CONVERT;

    // First, let's see if the referenced item actually exists on disk
    if ( _taccess( strDirectory.c_str(), 0 ) == 0 )
    {
        // Item exists. Is it a file or directory?
        struct stat status;
        stat( stringConvertT2CA(strDirectory.c_str()), &status );

        // Must indicate directory.
        if ( status.st_mode & S_IFDIR )
            return true;

        // Item is a file
        return false;
    
    } // End if exists

    // Directory does not exist
    return false;
}

//-----------------------------------------------------------------------------
//  Name : createDirectory () (Static)
/// <summary>
/// Create the specified directory, including any full parent sub-tree as
/// required.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFileSystem::createDirectory( const cgString & strDirectory )
{
    return ( SHCreateDirectoryEx( NULL, strDirectory.c_str(), NULL ) == ERROR_SUCCESS );
}

///////////////////////////////////////////////////////////////////////////////
// cgInputStream Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream()
{
    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( const cgInputStream & Stream )
{
    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Perform necessary copy operation
    *this = Stream;
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( const cgChar * strFile )
{
    // String Conversion
    STRING_CONVERT;

    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Initialize variables to sensible defaults
    setStreamSource( stringConvertA2CT( strFile ) );
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( const cgWChar * strFile )
{
    // String Conversion
    STRING_CONVERT;

    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Initialize variables to sensible defaults
    setStreamSource( cgString( stringConvertW2CT( strFile ) ) );
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( const cgString & strFile )
{
    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Initialize variables to sensible defaults
    setStreamSource( strFile );
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( const cgString & strFile, const cgString & strStreamName )
{
    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Initialize variables to sensible defaults
    setStreamSource( strFile, strStreamName );
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( void * pBuffer, size_t nBufferLength )
{
    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Initialize variables to sensible defaults
    setStreamSource( pBuffer, nBufferLength );
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( void * pBuffer, size_t nBufferLength, const cgString & strStreamName )
{
    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Initialize variables to sensible defaults
    setStreamSource( pBuffer, nBufferLength, strStreamName );
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( const cgInputStream & Container, size_t nOffset, size_t nLength )
{
    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Initialize variables to sensible defaults
    setStreamSource( Container, nOffset, nLength );
}

//-----------------------------------------------------------------------------
//  Name : cgInputStream () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::cgInputStream( const cgInputStream & Container, size_t nOffset, size_t nLength, const cgString & strStreamName )
{
    // Initialize variables to sensible defaults
    mData             = CG_NULL;
    mAccessCount      = 0;
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;

    // Initialize variables to sensible defaults
    setStreamSource( Container, nOffset, nLength, strStreamName );
}

//-----------------------------------------------------------------------------
//  Name : ~cgInputStream () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream::~cgInputStream()
{
    // Clean up.
    reset();
}

//-----------------------------------------------------------------------------
//  Name : reset ()
/// <summary>
/// Clean up and release references to data stream.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputStream::reset( )
{
    // Clean up.
    releaseDataReference();
}

//-----------------------------------------------------------------------------
//  Name : operator= () (const cgInputStream&)
/// <summary>
/// Overloaded assignment operator.
/// </summary>
//-----------------------------------------------------------------------------
cgInputStream & cgInputStream::operator=( const cgInputStream & Stream )
{
    // It's important that we increment the reference count
    // of the input stream first in case it is a self reference
    // or referencing the same data.
    if ( Stream.mData != CG_NULL )
        Stream.mData->referenceCount++;

    // Clean up our own internal data
    releaseDataReference();

    // Share the specified input stream's data pointer
    mData         = Stream.mData;
    mStreamName = Stream.mStreamName;

    // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : setStreamSource ()
/// <summary>
/// Select the source of the data stream. This overload allows you to
/// specify a single file on disk as the source, or optionally part of
/// a container file using the "FileName.Ext(o:x|l:y)" format.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputStream::setStreamSource( const cgString & strFile )
{
    return setStreamSource( strFile, cgString::Empty );
}
bool cgInputStream::setStreamSource( const cgString & strFile, const cgString & strStreamName )
{
    cgString::size_type nMapDetails;
    int                 nOffset, nLength;
    bool                bMappedFile = false;
    cgString            strContainer, strResolvedFile;

    // If the file name is empty, just reset the stream
    if ( strFile.empty() )
    {
        // Invalid file reference
        releaseDataReference();
        return false;
    
    } // End if empty string

    // Parse the file string to see if this is a mapped file reference
    nMapDetails = strFile.rfind( _T("(o:") );
    if ( nMapDetails != cgString::npos )
    {
        // We found initial token of reference details, attempt to extract values
        if ( _stscanf( strFile.substr( nMapDetails ).c_str(), _T("(o:%i|l:%i)"), &nOffset, &nLength ) == 2 )
        {
            strContainer = strFile.substr( 0, nMapDetails );
            bMappedFile  = true;
        
        } // End if extracted both components

    } // End if token found

    if ( bMappedFile )
    {
        // Pass through to mapped file version
        return setStreamSource( strContainer, nOffset, nLength, (strStreamName.empty()) ? strFile : strStreamName );

    } // End if mapped file reference
    else
    {
        // Get the final path/filename for this reference
        strResolvedFile = cgFileSystem::resolveFileLocation( strFile );

        // First, let's see if this file actually exists on disk
        if ( _taccess( strResolvedFile.c_str(), 0 ) == 0 )
        {
            // The file existed. We can reference this as a standard
            // file stream time. First, close and release our reference 
            // to any previous data items.
            releaseDataReference();

            // Allocate a new stream data structure
            allocateDataStructure();

            // Populate StreamData details for standalone file
            mData->type          = cgStreamType::File;
            mData->sourceFile = strResolvedFile;

            // Set the name to be associated with the stream
            mStreamName = (strStreamName.empty()) ? strFile : strStreamName;

        } // End if file exists
        else
        {
            cgInputStream PackagedStream;

            // Since the file didn't exist on the disk, let's attempt
            // to find this file in any opened package.
            if ( cgFileSystem::getPackageFile( strFile, PackagedStream ) )
            {
                // Copy details over
                *this = PackagedStream;
            
            } // End if found in package
            else
            {
                // Invalid file reference
                releaseDataReference();

                // Set the name to be associated with the stream (for error reporting etc.)
                mStreamName = (strStreamName.empty()) ? strFile : strStreamName;
                return false;
            
            } // End if not found in indexed package
            
        } // End if file did not exist

    } // End if standard file

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setStreamSource ()
/// <summary>
/// Select the source of the data stream. This overload allows you to
/// specify a an existing memory buffer.
/// Note : The buffer will NOT be duplicated and the caller must ensure the
/// pointer is valid for the lifetime of the input stream.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputStream::setStreamSource( void * pBuffer, size_t nBufferLength )
{
    return setStreamSource( pBuffer, nBufferLength, cgString::Empty );
}
bool cgInputStream::setStreamSource( void * pBuffer, size_t nBufferLength, const cgString & strStreamName )
{
    // Close and release our reference to previous data items
    releaseDataReference();

    // Allocate a new stream data structure
    allocateDataStructure();

    // Populate StreamData details for memory buffer
    mData->type           = cgStreamType::Memory;
    mData->sourceBuffer  = (cgByte*)pBuffer;
    mData->length        = nBufferLength;
    mStreamName         = strStreamName;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setStreamSource ()
/// <summary>
/// Select the source of the data stream. This overload allows you to
/// specify part of an existing data file.
/// Note : This method accepts an input stream instead of a file name to allow
/// it to resolve any multiple file mapping references. For example 
/// "Meshes/Test.iwf(o:x|l:x)". The IWF file in question may actually be 
/// stored within an existing package and should be resolved further.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputStream::setStreamSource( const cgInputStream & Container, size_t nOffset, size_t nLength )
{
    return setStreamSource( Container, nOffset, nLength, cgString::Empty );
}
bool cgInputStream::setStreamSource( const cgInputStream & Container, size_t nOffset, size_t nLength, const cgString & strStreamName )
{
    // Close and release our reference to previous data items
    releaseDataReference();

    // Where is our container?
    if ( Container.getType() == cgStreamType::File )
    {
        // Allocate a new stream data structure
        allocateDataStructure();

        // Populate StreamData details for mapped file
        mData->type           = cgStreamType::MappedFile;
        mData->sourceFile  = Container.getSourceFile();
        mData->offset        = nOffset;
        mData->length        = nLength;

        // Set the stream name
        if ( strStreamName.empty() )
            mStreamName = cgString::format( _T("%s(o:%i|l:%i)"), Container.getName().c_str(), nOffset, nLength );
        else
            mStreamName = strStreamName;

    } // End if file container
    else if ( Container.getType() == cgStreamType::Memory )
    {
        // Allocate a new stream data structure
        allocateDataStructure();

        // Populate StreamData details for mapped file
        mData->type           = cgStreamType::Memory;
        mData->sourceBuffer  = Container.mData->sourceBuffer + nOffset;
        mData->length        = nLength;

        // Set the stream name
        if ( strStreamName.empty() )
            mStreamName = cgString::format( _T("%s(o:%i|l:%i)"), Container.getName().c_str(), nOffset, nLength );
        else
            mStreamName = strStreamName;

    } // End if memory container
    else if ( Container.getType() == cgStreamType::MappedFile )
    {
        // Allocate a new stream data structure
        allocateDataStructure();

        // Populate StreamData details for mapped file
        mData->type           = cgStreamType::MappedFile;
        mData->sourceFile  = Container.getSourceFile();
        mData->offset        = Container.mData->offset + nOffset;
        mData->length        = nLength;

        // Set the stream name
        if ( strStreamName.empty() )
            mStreamName = cgString::format( _T("%s(o:%i|l:%i)"), Container.getName().c_str(), nOffset, nLength );
        else
            mStreamName = strStreamName;

    } // End if mapped file
    else
    {
        // Set the name to be associated with the stream (for error reporting etc.)
        if ( strStreamName.empty() )
            mStreamName = cgString::format( _T("%s(o:%i|l:%i)"), Container.getName().c_str(), nOffset, nLength );
        else
            mStreamName = strStreamName;
        return false;

    } // End if invalid type

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Retrieve the name associated with this stream.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgInputStream::getName( ) const
{
    return mStreamName;
}

//-----------------------------------------------------------------------------
//  Name : getSourceFile ()
/// <summary>
/// Retrieve the full name of the disk based file that is the source
/// for this stream.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgInputStream::getSourceFile( ) const
{
    if ( mData == CG_NULL )
        return cgString::Empty;
    return mData->sourceFile;
}

//-----------------------------------------------------------------------------
//  Name : sourceExists ()
/// <summary>
/// Determine if the stream source exists or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputStream::sourceExists( ) const
{
    // Any stream source specified?
    if ( mData == CG_NULL )
        return false;

    // If this is a memory stream, return true only if memory buffer is not CG_NULL
    if ( getType() == cgStreamType::Memory )
        return (mData->sourceBuffer != CG_NULL);

    // Otherwise, check for the existence of the source file
    return (_taccess( mData->sourceFile.c_str(), 0 ) == 0);
}

//-----------------------------------------------------------------------------
//  Name : getBuffer ()
/// <summary>
/// Get access to a pointer for this data stream. If the input stream
/// is a file, this method will attempt to map the file into the process
/// address space. If that fails, the file will be loaded into memory
/// in its entirety.
/// </summary>
//-----------------------------------------------------------------------------
cgByte * cgInputStream::getBuffer( size_t & nDataLength, size_t nAccessOffset /* = 0 */, size_t nAccessSize /* = 0 */ )
{
    bool    bMappingFailed = false;
    size_t  nMapViewSize = 0, nMapViewOffset = 0, nMapFileOffset = 0;

    // String conversion
    STRING_CONVERT;

    // Be polite and reset variables
    nDataLength = 0;

    // Anything to access?
    if ( mData == CG_NULL || mData->type == cgStreamType::None )
        return CG_NULL;

    // type dependant operations
    if ( mData->type == cgStreamType::Memory )
    {
        // Compute correct offset / size values if they aren't provided
        if ( nAccessSize == 0 )
            nAccessSize = mData->length;
        if ( nAccessOffset > mData->length )
            nAccessOffset = mData->length;
        if ( nAccessOffset + nAccessSize > mData->length )
            nAccessSize = mData->length - nAccessOffset;

        // If there is nothing to access, bail
        if ( nAccessSize == 0 )
            return CG_NULL;

        // Simplest case, just return our internal memory buffer
        nDataLength = nAccessSize;
        mData->accessCount++;
        mAccessCount++;
        return mData->sourceBuffer + nAccessOffset;
    
    } // End if memory stream
    else if ( mData->type == cgStreamType::File || mData->type == cgStreamType::MappedFile )
    {
        std::ifstream File;
        SYSTEM_INFO   SysInfo;

        // Open file if we haven't previously locked
        if ( mData->accessCount == 0 )
        {
            // Retrive information about the memory granularity of the system
            GetSystemInfo( &SysInfo );

            // Attempt Open the source file
            try
            {
                File.open( stringConvertT2CA( mData->sourceFile.c_str() ), std::ios_base::in | std::ios_base::binary );
                if ( !File.good() ) throw( std::exception( "Failed to open input stream. File does not exist or access was denied." ) );

                // Get the length of the file
                if ( mData->type == cgStreamType::MappedFile )
                {
                    mData->mapLength = mData->length;
                
                } // End if mapped file
                else
                {
                    // Seek to the end and retrieve the file size
                    File.seekg( 0, std::ios::end );
                    mData->mapLength = (size_t)File.tellg();
                    
                }  // End if standard file

                // Close the file, we've finished with it
                File.close();

            } // End try block

            catch ( std::exception & )
            {
                File.close();
                return CG_NULL;

            } // End Catch Block

        } // End if not accessed before

        // Compute correct lock offset / size values if they aren't provided
        if ( nAccessSize == 0 )
            nAccessSize = mData->mapLength;
        if ( nAccessOffset > mData->mapLength )
            nAccessOffset = mData->mapLength;
        if ( nAccessOffset + nAccessSize > mData->mapLength )
            nAccessSize = mData->mapLength - nAccessOffset;

        // If there is nothing to lock, bail
        if ( nAccessSize == 0 )
            return CG_NULL;

        // No call to getBuffer() previously mapped data?
        if ( mData->accessCount == 0 )
        {
            // ToDo: Potentially we should use named mappings here, and attempt to open with 'OpenFileMapping' if one already exists.

            // Attempt to open the file for mapping
            mData->fileHandle = CreateFile( mData->sourceFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, CG_NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, CG_NULL );
            if ( mData->fileHandle == CG_NULL || mData->fileHandle == (HANDLE)INVALID_HANDLE_VALUE )
            {
                DWORD nLastError = GetLastError();

                // Failed to open the file for the mapping. We're going to have to load this file into memory
                bMappingFailed = true;
            
            } // End if failed
            else
            {
                // Compute the correct size of the mapping taking into account memory granularity
                nMapFileOffset = mData->offset % SysInfo.dwAllocationGranularity;
                nMapViewOffset = mData->offset - nMapFileOffset;
                nMapViewSize   = nMapFileOffset + mData->mapLength;

                // Succeeded in opening the file, let's attempt to create a mapping
                mData->mappingHandle = CreateFileMapping( mData->fileHandle, CG_NULL, PAGE_READONLY, 0, 0, CG_NULL );
                if ( mData->mappingHandle == CG_NULL )
                {
                    // Failed to open the file for the mapping. We're going to have to load this file into memory
                    bMappingFailed = true;

                } // End if failed
                else
                {
                    // Attempt to map the file into process address space
                    mData->mappedView = (cgByte*)MapViewOfFile( mData->mappingHandle, FILE_MAP_READ, 0, (cgUInt32)nMapViewOffset, (cgUInt32)nMapViewSize );
                    if ( mData->mappedView == CG_NULL )
                    {
                        // Failed to open the file for the mapping. We're going to have to load this file into memory
                        bMappingFailed = true;
                    }
                    else
                    {
                        // Offset to the correct starting location within the mapped view (where our file begins)
                        mData->mappedFile = ((cgByte*)mData->mappedView) + nMapFileOffset;

                    } // End if map view success

                } // End if mapping create success

            } // End if open success

            // If we failed to map, we're going to need to load into memory
            if ( bMappingFailed )
            {
                // Close handles
                if ( mData->mappingHandle != CG_NULL )
                    CloseHandle( mData->mappingHandle );
                if ( mData->fileHandle != CG_NULL )
                    CloseHandle( mData->fileHandle );
                mData->mappingHandle = CG_NULL;
                mData->fileHandle    = CG_NULL;

                // Notify user via warning.
                cgAppLog::write( cgAppLog::Warning, _T("Failed to map input stream '%s' to process address space for paging. Stream data has been loaded entirely into memory as the only remaining fallback.\n"), getName().c_str() );
    
                try
                {
                    File.open( stringConvertT2CA( mData->sourceFile.c_str() ), std::ios_base::in | std::ios_base::binary );
                    if ( !File.good() ) throw( std::exception( "Failed to open input stream. File does not exist or access was denied." ) );

                    // Seek back to the beginning of the file
                    File.seekg( (cgInt32)mData->offset, std::ios::beg );

                    // Allocate enough memory to store the file
                    mData->memoryBuffer = new cgByte[ mData->mapLength ];

                    // Read the entire contents of the file into this buffer
                    File.read( (cgChar*)mData->memoryBuffer, (std::streamsize)mData->mapLength );

                    // Close the file, we're finished with it
                    File.close();
                
                } // End Try read

                catch ( ... )
                {
                    File.close();
                    if ( mData->memoryBuffer != CG_NULL )
                        delete []mData->memoryBuffer;
                    return CG_NULL;

                } // End catch exception

            } // End if failed to map

        } // End if map for access
        
        // Return the pointer to the mapped data
        if ( mData->memoryBuffer != CG_NULL )
        {
            // Failed to map, loaded into memory instead
            nDataLength = nAccessSize;
            mData->accessCount++;
            mAccessCount++;
            return mData->memoryBuffer + nAccessOffset;

        } // End if memory buffer used
        else if ( mData->mappedFile != CG_NULL )
        {
            // Mapped succesfully
            nDataLength = nAccessSize;
            mData->accessCount++;
            mAccessCount++;
            return mData->mappedFile + nAccessOffset;

        } // End if file mapped succesfully
        else
        {
            return CG_NULL;

        } // Everything failed

    } // End if file stream
    
    // Invalid type (paranoia)
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : releaseBuffer ()
/// <summary>
/// Called in order to notify the input stream that we have finished with
/// one reference to the data pointer.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputStream::releaseBuffer( )
{
    // Is this a no-op?
    if ( mAccessCount == 0 )
        return;

    // We've released a reference
    mAccessCount--;
    mData->accessCount--;

    // Release the lock handles if we were the last one holding them
    if ( mData->accessCount == 0 )
    {
        if ( mData->mappedView != CG_NULL )
            UnmapViewOfFile( mData->mappedView );
        if ( mData->mappingHandle != CG_NULL )
            CloseHandle( mData->mappingHandle );
        if ( mData->fileHandle != CG_NULL )
            CloseHandle( mData->fileHandle );
        if ( mData->memoryBuffer != CG_NULL )
            delete []mData->memoryBuffer;

        // Clear lock variables
        mData->fileHandle          = CG_NULL;
        mData->mappingHandle       = CG_NULL;
        mData->memoryBuffer  = CG_NULL;
        mData->mappedFile    = CG_NULL;
        mData->mappedView    = CG_NULL;
        mData->mapLength     = 0;
    
    } // End if all access completed
}

//-----------------------------------------------------------------------------
//  Name : releaseDataReference () (Private)
/// <summary>
/// Release our reference to the StreamData structure. If we were the 
/// last one holding a pointer, clean up.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputStream::releaseDataReference()
{
    // Close the stream if it was open
    close();

    // Release all of our access references just in case the user did not
    for ( cgUInt32 i = 0; i < mAccessCount; ++i )
        releaseBuffer();

    // Stream data referenced?
    if ( mData != CG_NULL )
    {
        mData->referenceCount--;
        if ( mData->referenceCount == 0 )
        {
            // Delete the information structure, nobody else is referencing
            delete mData;

        } // End if last reference

    } // End if data available
    mData         = CG_NULL;
    mStreamName = cgString::Empty;
}

//-----------------------------------------------------------------------------
//  Name : allocateDataStructure () (Private)
/// <summary>
/// Create and initialize a new StreamData structure for us to use.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputStream::allocateDataStructure()
{
    mData                     = new StreamData();
    mData->type               = cgStreamType::None;
    mData->sourceBuffer      = CG_NULL;
    mData->offset            = 0;
    mData->length            = 0;
    mData->accessCount       = 0;
    mData->referenceCount    = 1; // Important!
    mData->fileHandle              = CG_NULL;
    mData->mappingHandle           = CG_NULL;
    mData->memoryBuffer      = CG_NULL;
    mData->mappedFile        = CG_NULL;
    mData->mappedView        = CG_NULL;
    mData->mapLength         = 0;
}

//-----------------------------------------------------------------------------
//  Name : open ()
/// <summary>
/// Open the specified stream ready for processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputStream::open( )
{
    // String Conversion
    STRING_CONVERT;

    // Close the stream if it is already open
    close();

    if ( getType() == cgStreamType::Memory )
    {
        mStreamOffset     = 0;
        mCurrentPosition       = 0;
        mStreamLength     = mData->length;
        mBufferOffset     = mData->sourceBuffer;
        mBufferedData     = CG_NULL;
    
    } // End if memory buffer
    else if ( getType() == cgStreamType::File )
    {
        try
        {
            // Open the file at the end.
            mFileStream.open( stringConvertT2CA(getSourceFile().c_str()), std::ios_base::in | std::ios_base::binary | std::ios_base::ate );
            if ( !mFileStream.good() ) throw std::exception();

            // Compute stream length / offset
            mStreamLength = mFileStream.tellg();
            mStreamOffset = 0;

            // Allocate the reading buffer (optimization) and set our offset to the beginning.
            mBufferedData = new cgByte[ ReadBufferSize ];
            mFileStream.rdbuf()->pubsetbuf( (char*)mBufferedData, ReadBufferSize );
            
            // Seek back to the beginning
            mFileStream.seekg( 0, std::ios_base::beg );
            mCurrentPosition = 0;

        } // End Try open

        catch (...)
        {
            close();
            return false;

        } // End Catch Block
    
    } // End if file stream
    else if ( getType() == cgStreamType::MappedFile )
    {
        try
        {
            // Open the file.
            mFileStream.open( stringConvertT2CA(getSourceFile().c_str()), std::ios_base::in | std::ios_base::binary );
            if ( !mFileStream.good() ) throw std::exception();

            // Compute stream length / offset
            mStreamLength = mData->length;
            mStreamOffset = mData->offset;
            
            // Allocate the reading buffer (optimization) and set our offset to the beginning.
            mBufferedData = new cgByte[ ReadBufferSize ];
            mFileStream.rdbuf()->pubsetbuf( (char*)mBufferedData, ReadBufferSize );
            
            // Seek to correct starting location
            mFileStream.seekg( (cgInt32)mData->offset, std::ios_base::beg );
            mCurrentPosition = 0;

        } // End Try open

        catch (...)
        {
            close();
            return false;

        } // End Catch Block

    } // End if mapped file
    else
    {
        return false;

    } // End if unknown / no stream type
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : close () (Virtual)
/// <summary>
/// Close the stream, no further processing will occur.
/// </summary>
//-----------------------------------------------------------------------------
void cgInputStream::close( )
{
    if ( !isOpen() )
        return;

    // Close any opened physical file
    if ( mFileStream.is_open() )
        mFileStream.close();

    // Release any allocated memory
    delete []mBufferedData;

    // Clear variables.
    mFileStream.clear();
    mCurrentPosition       = 0;
    mBufferOffset     = CG_NULL;
    mBufferedData     = CG_NULL;
    mStreamLength     = 0;
    mStreamOffset     = 0;
}

//-----------------------------------------------------------------------------
//  Name : computeSHA1 ()
/// <summary>
/// Compute the SHA-1 cryptographic hash for this file. Output array ('Hash'
/// parameter) should be a 5 element array to house the computed 160-bit 
/// message digest.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputStream::computeSHA1( cgUInt32 Hash[] )
{
    cgChecksum::SHA1 Checksum;
    bool bWasOpen = isOpen(), bResult = true;
    cgInt64 nOldPos = 0;
    
    // If the file is already open, backup the position so that we can restore it
    // and then reposition at the start of the stream. Otherwise, open the file.
    if ( bWasOpen )
    {
        nOldPos = getPosition();
        seek( 0, Begin );
    
    } // End if open
    else
    {
        if ( !open( ) )
            return false;
    
    } // End if !open

    // Prepare to recieve message data for SHA1 computation.
    Checksum.beginMessage();

    // Compute the SHA-1 in 4k blocks.
    cgByte Buffer[4096];
    while ( !isEOF() || !bResult )
    {
        size_t nDataRead = read( Buffer, sizeof(Buffer) );
        if ( nDataRead )
        {
            if ( !Checksum.messageData( Buffer, nDataRead ) )
                bResult = false;
        
        } // End if data

    } // Next Block

    // Compute the final digest.
    if ( bResult )
    {
        Checksum.endMessage();
        Checksum.getHash( Hash );
    
    } // End if success

    // Close the file, or reposition back at original position.
    if ( bWasOpen )
        seek( nOldPos, Begin );
    else
        close();

    // Success?
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : read () (Virtual)
/// <summary>
/// Read the amount of data specified from the file into the buffer.
/// </summary>
//-----------------------------------------------------------------------------
size_t cgInputStream::read( void * pBuffer, size_t BufferSize )
{
    // File must be open.
    if ( !isOpen() )
        return 0;

    // Compute offset / size values
    const cgInt64 nDataRemaining  = mStreamLength - mCurrentPosition;
    const cgInt64 nCopyAmount     = __min( nDataRemaining, BufferSize );
    if ( nCopyAmount == 0 )
        return 0;
    
    // Source dependant
    if ( getType() == cgStreamType::Memory )
    {
        // Copy the required amount
        memcpy( pBuffer, mBufferOffset, (size_t)nCopyAmount );

        // Move pointer on by this amount
        mCurrentPosition   += nCopyAmount;
        mBufferOffset += nCopyAmount;

    } // End if memory stream
    else
    {
        mFileStream.read( (cgChar*)pBuffer, std::streamsize(nCopyAmount) );
        mCurrentPosition += nCopyAmount;
    
    } // End if file / memory mapped file

    // Return the amount we copied
    return (size_t)nCopyAmount;
}

//-----------------------------------------------------------------------------
//  Name : readString ()
/// <summary>
/// Read the specified number of ANSI characters from the open stream and
/// return them as a system string.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgInputStream::readString( size_t nLength )
{
    //STRING_CONVERT;
    //std::string Data;
    //Data.resize( nLength );
    //if ( read( &Data[0], nLength ) < nLength )
        //return cgString::Empty;
    //return stringConvertA2CT( Data.c_str() );

    // Read ANSI string (cheap-ish way).
    cgString strOut;
    strOut.resize( nLength );
    for ( size_t i = 0; i < nLength; ++i )
    {
        strOut[i] = 0;
        read( &strOut[i], 1 );
    } // Next char
    return strOut;
}

//-----------------------------------------------------------------------------
//  Name : seek ()
/// <summary>
/// Seek to the relevant location within the stream relative to the 
/// origin specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgInputStream::seek( cgInt64 Offset, SeekOrigin Origin /* = Current */ )
{
    // Bail if file is not open
    if ( !isOpen() )
        return false;

    // Compute offset / size values
    cgInt64 nSeekPos = 0;

    // stdio compatible origin.
    switch ( Origin )
    {
        case Current:
            nSeekPos = mCurrentPosition + Offset;
            break;

        case Begin:
            nSeekPos = Offset;
            break;

        case End:
            nSeekPos = mStreamLength + Offset;
            break;

    } // End switch Origin

    // Clamp the seek position
    if ( nSeekPos > mStreamLength )
        nSeekPos = mStreamLength;
    if ( nSeekPos < 0 )
        nSeekPos = 0;

    // Peform the source dependant seek
    if ( getType() == cgStreamType::Memory )
    {
        mBufferOffset = mData->sourceBuffer + mStreamOffset + nSeekPos;
        
    } // End if Memory
    else
    {
        // Reset eof bit (if set). We might be seeking back from an eof.
        if ( mFileStream.eof() )
            mFileStream.clear( std::ios::eofbit );

        // Seek
        mFileStream.seekg( std::streamoff(mStreamOffset + nSeekPos), std::ios::beg );
    
    } // End if !Memory

    // Update our current position
    mCurrentPosition = nSeekPos;
    
    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDataPackage Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDataPackage () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDataPackage::cgDataPackage( const cgString & strPackageFile )
{
    // Initialize variables to sensible defaults
    mIndexed          = false;
    mPackageFileName    = strPackageFile;
    mIndexTableOffset = 0;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDataPackage () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDataPackage::~cgDataPackage()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : index ()
/// <summary>
/// Index the contents of the package.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDataPackage::index( )
{
    STRING_CONVERT;  // For string conversion macro

    std::ifstream Package;
    cgByte        Signature[4];
    cgUInt32      nFileCount, i;
    cgUInt16      nStringLength;
    std::string   strInputA;

    // Ensure file is valid before we index.
    if ( !isValid() )
        return false;

    // Read the package index
    try
    {
        // Clear out old data
        mFiles.clear();
        mIndexed = false;

        // Open the package
        Package.open( stringConvertT2CA(mPackageFileName.c_str()), std::ios::in | std::ios::binary );
        if ( !Package.good() )
            throw( std::exception( "Failed to open package file. Access denied or file did not exist." ) );

        // Read the package unique identifier.
        Package.seekg( 16, std::ios_base::beg );
        Package.read( (cgChar*)&mPackageUID, 16 );

        // Seek to the end of the file and read the 4 byte index table end signature
        Package.seekg( -4, std::ios_base::end );
        Package.read( (cgChar*)Signature, 4 );
        if ( !Package.good() || memcmp( Signature, mIndexEndSig, 4 ) != 0 )
            throw( std::exception("Failed to access package index table. The package is possibly corrupt or incomplete.") );

        // Also read the 4 byte index table start offset
        Package.seekg( -8, std::ios_base::end );
        Package.read( (cgChar*)&mIndexTableOffset, 4 );
        if ( !Package.good() )
            throw( std::exception("Failed to access package index table. The package is possibly corrupt or incomplete.") );

        // Seek to the start of the index table
        Package.seekg( (cgInt32)mIndexTableOffset, std::ios_base::beg );

        // Ensure we've seeked to the correct place (i.e. index table is valid)
        Package.read( (cgChar*)Signature, 4 );
        if ( !Package.good() || memcmp( Signature, mIndexStartSig, 4 ) != 0 )
            throw( std::exception("Failed to access package index table. The package is possibly corrupt or incomplete.") );

        // Determine how many files are in the package
        Package.read( (cgChar*)&nFileCount, 4 );

        // Read data for each file
        for ( i = 0; i < nFileCount; ++i )
        {
            FileInfo Info;
            
            // Read file path and name
            Package.read( (cgChar*)&nStringLength, 2 );
            if ( nStringLength > 0 )
            {
                strInputA.resize( nStringLength );
                Package.read( &strInputA[0], nStringLength );
                Info.pathName = stringConvertA2CT(strInputA.c_str());

            } // End if path available
            
            Package.read( (cgChar*)&nStringLength, 2 );
            if ( nStringLength > 0 )
            {
                strInputA.resize( nStringLength );
                Package.read( &strInputA[0], nStringLength );
                Info.fileName = stringConvertA2CT(strInputA.c_str());

            } // End if name available

            // Read file parameters
            Package.read( (cgChar*)&Info.flags, 4 );
            Package.read( (cgChar*)&Info.offset, 4 );
            Package.read( (cgChar*)&Info.packagedLength, 4 );
            Package.read( (cgChar*)&Info.originalLength, 4 );
            Package.seekg( 12, std::ios::cur );
            
            // Add this file to the index
            mFiles[ (Info.pathName + Info.fileName).toLower() ] = Info;

        } // Next File

        // Close the package, we're done with it
        Package.close();

        // We're fully indexed
        mIndexed = true;
    
    } // End try and open / read

    catch( ... )
    {
        Package.close();
        return false;

    } // End catch exception

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : appendFile ()
/// <summary>
/// Append the specified file to the end of the package in the specified
/// package directory.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDataPackage::appendFile( const cgString & strFile, const cgString & strPackageReference )
{
    cgUInt32      nVersion, nFlags = 0, nReserved = 0;
    cgUInt32      aVersion[3] = { 1, 0, 0 };
    size_t        nFileLength = 0;
    cgString      strPath, strName;
    FileInfo      Info;
    std::ofstream Package;
    std::ifstream File;

    // String conversion
    STRING_CONVERT;

    try
    {
        // Compute the separate components for the package reference
        strName = cgFileSystem::getFileName( strPackageReference );
        strPath = strPackageReference.substr( 0, strPackageReference.length() - strName.length() );
        strPath.trim();
        if ( strPath.length() > 0 )
        {
            strPath.replace( _T('\\'), _T('/') );
            if ( !strPath.endsWith( _T("/") ) )
                strPath = strPath + _T("/");
            if ( strPath.beginsWith( _T("/") ) )
                strPath = strPath.substr( 1 );
        
        } // End if path specified

        // Open the file that we want to append
        File.open( stringConvertT2CA(strFile.c_str()), std::ios::in | std::ios::binary );
        if ( !File.good() )
            throw( std::exception( "Failed to open source file. Access denied.") );

        // First determine if the package file already exists
        if ( _taccess( mPackageFileName.c_str(), 0 ) != 0 )
        {
            // Package file does not exist yet. Create the file
            // and begin by writing headers.
            Package.open( stringConvertT2CA(mPackageFileName.c_str()), std::ios::out | std::ios::binary );
            if ( !Package.good() )
                throw( std::exception( "Failed to create package file. Access denied." ) );

            // Generate a new unique package identifier.
            mPackageUID = cgUID::generateRandom();

            // Write magic number, version identifier and unique package identifier
            nVersion = ((aVersion[0] & 0xFF) << 24) + ((aVersion[1] & 0xFF) << 16) + (aVersion[2] & 0xFFFF);
            Package.write( (cgChar*)mMagicNumber, 8 );
            Package.write( (cgChar*)&nVersion, 4 );
            Package.write( (cgChar*)&nFlags, 4 );
            Package.write( (cgChar*)&mPackageUID, 16 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&nReserved, 4 );

            // Write blank index block (just in case we bail)
            mIndexTableOffset = (size_t)Package.tellp();
            Package.write( (cgChar*)mIndexStartSig, 4 );
            Package.write( (cgChar*)&nReserved, 4 );
            Package.write( (cgChar*)&mIndexTableOffset, 4 );
            Package.write( (cgChar*)mIndexEndSig, 4 );

            // Assume we're indexed
            mIndexed = true;

        } // End if no existing file
        else
        {
            // It exists, but make sure we can read from and write to it
            if ( _taccess( mPackageFileName.c_str(), 6 ) != 0 )
                throw ( std::exception( "Failed to gain read/write access to package file." ) );

            // Index the file if it hasn't been already
            if ( !mIndexed )
            {
                if ( !index() )
                    throw ( std::exception( "Failed to index package. The file is potentially corrupt or invalid." ) );

            } // End if not indexed yet

            // Ensure file does not already exist in the package.
            if ( mFiles.find( (strPath + strName).toLower() ) != mFiles.end() )
                throw ( std::exception( "A file with the specified reference already exists." ) );

            // Open the file for writing / overwriting
            Package.open( stringConvertT2CA(mPackageFileName.c_str()), std::ios::out | std::ios::in | std::ios::ate | std::ios::binary );
            if ( !Package.good() )
                throw( std::exception( "Failed to open package file. Access denied." ) );

        } // End if package file exists

        // Seek to position just prior to the index table
        Package.seekp( (cgInt32)mIndexTableOffset, std::ios_base::beg );

        // Retrieve the file length
        File.seekg(0, std::ios::end);
        nFileLength = (size_t)File.tellg();
        File.seekg(0, std::ios::beg);

        // Dump the contents of the file into package (if any)
        if ( nFileLength > 0 ) 
            Package << File.rdbuf();
        
        // Construct the index information for this file
        Info.flags             = 0;
        Info.offset            = mIndexTableOffset;
        Info.originalLength    = nFileLength;
        Info.packagedLength    = nFileLength;
        Info.fileName            = strName;
        Info.pathName            = strPath;

        // Add this file to the index
        mFiles[ (Info.pathName + Info.fileName).toLower() ] = Info;
        
        // Write the index
        writeIndex( Package );

        // We're done with both files
        File.close();
        Package.close();

    } // End Try create / write

    catch( std::exception & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to append file '%s' to package '%s'. %s\n"), strPackageReference.c_str(), mPackageFileName.c_str(), e.what() );
        File.close();
        Package.close();
        return false;

    } // End catch exception

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : isIndexed ()
/// <summary>
/// Query method to determine if this package has been indexed yet.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDataPackage::isIndexed( ) const
{
    return mIndexed;
}

//-----------------------------------------------------------------------------
//  Name : isValid ()
/// <summary>
/// Determine if the package file references is valid (if it exists at
/// all).
/// </summary>
//-----------------------------------------------------------------------------
bool cgDataPackage::isValid( ) const
{
    cgByte        MagicNumber[8];
    cgUInt32      nVersion, nMinVersion, nMaxVersion;
    cgUInt32      aMinVersion[3] = { 1, 0, 0 }, aMaxVersion[3] = { 1, 0, 0xFFFF };
    std::ifstream Package;

    // String conversion
    STRING_CONVERT;

    // Determine if the package file is valid
    try
    {
        Package.open( stringConvertT2CA(mPackageFileName.c_str()), std::ios::in | std::ios::binary );
        if ( !Package.good() )
            throw( std::exception( "Failed to open package file. Access denied or file did not exist.") );

        // Read magic number and version identifier
        Package.read( (cgChar*)MagicNumber, 8 );
        Package.read( (cgChar*)&nVersion, 4 );

        // Close the package, we're done with it
        Package.close();

        // Magic number and version match?
        nMinVersion = ((aMinVersion[0] & 0xFF) << 24) + ((aMinVersion[1] & 0xFF) << 16) + (aMinVersion[2] & 0xFFFF);
        nMaxVersion = ((aMaxVersion[0] & 0xFF) << 24) + ((aMaxVersion[1] & 0xFF) << 16) + (aMaxVersion[2] & 0xFFFF);
        if ( memcmp( MagicNumber, mMagicNumber, 8 ) != 0 || nVersion < nMinVersion || nVersion > nMaxVersion )
            return false;
    
    } // End try and open / read

    catch( ... )
    {
        Package.close();
        return false;

    } // End catch exception

    // Valid package!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getFile ()
/// <summary>
/// Retrieve an input stream reference for the specified file within
/// the package (if it exists).
/// </summary>
//-----------------------------------------------------------------------------
bool cgDataPackage::getFile( const cgString & strPackageReference, cgInputStream & Stream )
{
    FileMap::iterator itFile;
    cgString strFile = cgString::toLower( strPackageReference );
    
    // Prepare the string for searching within the index
    strFile.replace( _T('\\'), _T('/') );
    
    // Attempt to find this file
    itFile = mFiles.find( strFile );
    if ( itFile == mFiles.end() )
        return false;

    // We did find the file, construct the input stream
    const FileInfo & Info = itFile->second;
    Stream.setStreamSource( _T("currentdir://") + mPackageFileName, Info.offset, Info.originalLength, strPackageReference );

    // File was found!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : writeIndex () (Private)
/// <summary>
/// Write the index out to the specified output stream.
/// </summary>
//-----------------------------------------------------------------------------
void cgDataPackage::writeIndex( std::ofstream & Package )
{
    FileMap::iterator itFile;
    cgUInt32          nFileCount = (cgUInt32)mFiles.size();
    cgUInt32          nReserved  = 0;
    cgUInt16          nLength;

    // String conversion
    STRING_CONVERT;

    // Update package index table offset.
    mIndexTableOffset = (size_t)Package.tellp();

    // Write the index data.
    Package.write( (cgChar*)mIndexStartSig, 4 );
    Package.write( (cgChar*)&nFileCount, 4 );

    // Iterate through each file
    for ( itFile = mFiles.begin(); itFile != mFiles.end(); ++itFile )
    {
        const FileInfo & Info = itFile->second;
        nLength = (cgUInt16)Info.pathName.length();
        Package.write( (cgChar*)&nLength, 2 );
        Package.write( stringConvertT2CA(Info.pathName.c_str()), nLength );
        nLength = (cgUInt16)Info.fileName.length();
        Package.write( (cgChar*)&nLength, 2 );
        Package.write( stringConvertT2CA(Info.fileName.c_str()), nLength );
        Package.write( (cgChar*)&Info.flags, 4 );
        Package.write( (cgChar*)&Info.offset, 4 );
        Package.write( (cgChar*)&Info.packagedLength, 4 );
        Package.write( (cgChar*)&Info.originalLength, 4 );
        Package.write( (cgChar*)&nReserved, 4 );
        Package.write( (cgChar*)&nReserved, 4 );
        Package.write( (cgChar*)&nReserved, 4 );

    } // Next File

    // Finally write the offset and closing signature
    Package.write( (cgChar*)&mIndexTableOffset, 4 );
    Package.write( (cgChar*)mIndexEndSig, 4 );
}