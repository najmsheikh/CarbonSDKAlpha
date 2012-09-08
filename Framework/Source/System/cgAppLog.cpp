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
// Name : cgAppLog.cpp                                                       //
//                                                                           //
// Desc : These classes provide support for logging events within the        //
//        engine and host application. Custom 'LogOutput' classes can be     //
//        derived to route messages to alternate destinations (such as an    //
//        in-game console or stdout console window.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAppLog Module Includes
//-----------------------------------------------------------------------------
#include <System/cgAppLog.h>
#include <Resources/cgScript.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    // Warning: Portability
#undef WIN32_LEAN_AND_MEAN

// Standard library includes
#include <iostream>
#include <algorithm>
#include <stdarg.h>
#include <tchar.h>
#include <time.h>

//-----------------------------------------------------------------------------
// Static Member Variable Definitions
//-----------------------------------------------------------------------------
cgAppLog::LogOutputArray    cgAppLog::mOutputChannels;
bool                        cgAppLog::mLogging = false;

///////////////////////////////////////////////////////////////////////////////
// cgAppLog Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : beginLogging () (Static)
/// <summary>
/// Begin the logging process to the specified file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppLog::beginLogging( const cgString & strFile /* = _T("") */ )
{
    // Already logging?
    if ( mLogging == true ) return false;

    // If a file was specified, load it
    if ( !strFile.empty() )
    {
        cgLogOutput * pLogFile = new cgLogOutputFile( strFile );
        if ( pLogFile->isOpen() )
        {
            // Add to our list of output channels
            if ( !registerOutput( pLogFile ) )
            {
                delete pLogFile;
                return false;
            
            } // End if failed to register

        } // End if opened
        else
        {
            // Failed to open the log channel, clean up
            delete pLogFile;
        
        } // End if not open
    
    } // End if log file specified

    // Instruct all output channels to write their header
    for ( size_t i = 0; i < mOutputChannels.size(); ++i )
    {
        cgLogOutput * pOutput = mOutputChannels[i];

        // Output to this channel
        if ( pOutput ) pOutput->writeHeader( );

    } // Next Output Item

    // We're now logging
    mLogging = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endLogging () (Static)
/// <summary>
/// End the logging process.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppLog::endLogging( )
{
    // We are no longer logging
    mLogging = false;

    // Instruct all output channels to write their footer and release it
    for ( size_t i = 0; i < mOutputChannels.size(); ++i )
    {
        cgLogOutput * pOutput = mOutputChannels[i];

        // Output to this channel
        if ( pOutput )
        {
            pOutput->writeFooter( );
            delete pOutput;
        
        } // End if valid output

    } // Next Output Item

    // Clear the list
    mOutputChannels.clear();
}

//-----------------------------------------------------------------------------
//  Name : write () (Static)
/// <summary>
/// Write data to the log
/// </summary>
//-----------------------------------------------------------------------------
void cgAppLog::write( cgUInt32 nType, const cgTChar * lpszFormat, ... )
{
    cgString strBuffer;

    // Validate parameters
    if ( lpszFormat == CG_NULL || _tcslen(lpszFormat) == 0 || mOutputChannels.size() == 0 )
        return;

    // Begin processing the variable argument list
    va_list	ap;
    va_start(ap, lpszFormat);

    // Build the string
    strBuffer = cgString::format( lpszFormat, ap );

    // Finished building string
    va_end(ap);

    // Pass through to all output channels as the requested event
    for ( size_t i = 0; i < mOutputChannels.size(); ++i )
    {
        cgLogOutput * pOutput = mOutputChannels[i];

        // Output to this channel
        if ( pOutput ) pOutput->write( nType, strBuffer );

    } // Next Output Item
}

//-----------------------------------------------------------------------------
//  Name : write () (Static)
/// <summary>
/// Write data to the log
/// </summary>
//-----------------------------------------------------------------------------
void cgAppLog::write( const cgTChar * lpszFormat, ... )
{
    cgString strBuffer;

    // Validate parameters
    if ( lpszFormat == CG_NULL || _tcslen(lpszFormat) == 0 || mOutputChannels.size() == 0 )
        return;

    // Begin processing the variable argument list
    va_list	ap;
    va_start(ap, lpszFormat);

    // Build the string
    strBuffer = cgString::format( lpszFormat, ap );

    // Finished building string
    va_end(ap);

    // Pass through to all output channels as specified event
    for ( size_t i = 0; i < mOutputChannels.size(); ++i )
    {
        cgLogOutput * pOutput = mOutputChannels[i];

        // Output to this channel
        if ( pOutput ) pOutput->write( Normal, strBuffer );

    } // Next Output Item

}

//-----------------------------------------------------------------------------
//  Name : writeSeparator () (Static)
/// <summary>
/// Write a simple separator rule to the log
/// </summary>
//-----------------------------------------------------------------------------
void cgAppLog::writeSeparator( cgUInt32 nType /* = Normal */ )
{
    // Write a simple horizontal line
    for ( size_t i = 0; i < mOutputChannels.size(); ++i )
    {
        cgLogOutput * pOutput = mOutputChannels[i];

        // Output to this channel
        if ( pOutput ) pOutput->writeSeparator( nType );

    } // Next Output Item
}

//-----------------------------------------------------------------------------
//  Name : registerOutput () (Static)
/// <summary>
/// Add an 'cgLogOutput' derived listener to the system for log output
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppLog::registerOutput( cgLogOutput * pOutput )
{
    // Validate parameter
    if ( pOutput == CG_NULL )
        return false;

    try 
    {
        // Attempt to add to the vector
        mOutputChannels.push_back( pOutput );

    } // End Try Block
    catch( ... )
    {
        return false;
    } // End Catch Block

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : removeOutput () (Static)
/// <summary>
/// Remove the specified output item from the list if applicable.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppLog::removeOutput( cgLogOutput * pOutput, bool bDelete /* = true */ )
{
    LogOutputArray::iterator itItem = mOutputChannels.begin();

    // Find this in the vector and remove it
    for ( ; itItem != mOutputChannels.end(); )
    {
        itItem = std::find( mOutputChannels.begin(), mOutputChannels.end(), pOutput );
        if ( itItem != mOutputChannels.end() )
        {
            cgLogOutput * pOutput = *itItem;
            if ( pOutput != CG_NULL && bDelete == true )
                delete pOutput;
            mOutputChannels.erase( itItem );
        
        } // End if found the item
    
    } // Next search pass
}

//-----------------------------------------------------------------------------
//  Name : getOutputCount () (Static)
/// <summary>
/// Retrieve the number of output items registered.
/// </summary>
//-----------------------------------------------------------------------------
size_t cgAppLog::getOutputCount( )
{
    return mOutputChannels.size();
}

//-----------------------------------------------------------------------------
//  Name : getOutput () (Static)
/// <summary>
/// Retrieve an actual output object.
/// </summary>
//-----------------------------------------------------------------------------
cgLogOutput * cgAppLog::getOutput( cgUInt32 nIndex )
{
    if ( nIndex >= getOutputCount() ) return CG_NULL;
    return mOutputChannels[ nIndex ];
}

///////////////////////////////////////////////////////////////////////////////
// cgLogOutputFile Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLogOutputFile() (Constructor)
/// <summary>
/// cgLogOutputFile Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgLogOutputFile::cgLogOutputFile( const cgString & strLogFile, const cgString & strAppTitle, const cgString & strLogTitle )
{
    // Store copy of passed variables
    mApplicationTitle = strAppTitle;
    mLogTitle = strLogTitle;

    // Select the correct path for file icons
    if ( cgFileSystem::pathProtocolDefined( _T("sys" ) ) )
        mIconPath = cgFileSystem::resolveFileLocation( _T("sys://Icons/") );
    else
        mIconPath = cgFileSystem::resolveFileLocation( _T("System/Icons/") );

    try
    {
        // Convert filename to ANSI
        STRING_CONVERT;
        const cgChar * strFileName = stringConvertT2CA(strLogFile.c_str());

        // Attempt to open the file (write, text)
        mFile.open( strFileName );
    
    } // End Try Block
    catch( ... )
    {
        return;
    } // End Catch Block
}

//-----------------------------------------------------------------------------
//  Name : cgLogOutputFile () (Destructor)
/// <summary>
/// cgLogOutputFile Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgLogOutputFile::~cgLogOutputFile()
{
    // Close the file if it is open
    if ( isOpen() )
        mFile.close();
}

//-----------------------------------------------------------------------------
//  Name : write()
/// <summary>
/// Called by the cgAppLog class passing in any message that was written.
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputFile::write( cgUInt32 nType, const cgString & strMessage )
{
    cgTChar  strDate[128], strTime[128];
    cgString strType = _T("&nbsp;"), strOutput;

    // Validate requirements
    if ( !isOpen() ) return;

    // Replace all required HTML entities
    strOutput = cgString::replace( strMessage, _T("<"), _T("&lt;") );
    strOutput.replace( _T("<"), _T("&lt;") );
    strOutput.replace( _T(">"), _T("&gt;") );
    strOutput.replace( _T("\n"), _T("<BR/>\n") );

    // Replace custom tag values.
    strOutput.replace( _T("[["), _T("<") );
    strOutput.replace( _T("]]"), _T(">") );

    // Build prepend string based on type
    if ( nType == cgAppLog::Debug )
        strType = _T("<img src='") + mIconPath + _T("debug_filter.png' width='16' height='16' title='Debug' alt='Debug' align='absmiddle'>");
    else if ( nType & cgAppLog::Warning )
        strType = _T("<img src='") + mIconPath + _T("warning_filter.png' width='16' height='16' title='Warning' alt='Warning' align='absmiddle'>");
    else if ( nType & cgAppLog::Error )
        strType = _T("<img src='") + mIconPath + _T("error_filter.png' width='16' height='16' title='Error' alt='Error' align='absmiddle'>");
    else if ( nType & cgAppLog::Info )
        strType = _T("<img src='") + mIconPath + _T("info_filter.png' width='16' height='16' title='Info' alt='Info' align='absmiddle'>");
    else if ( nType & cgAppLog::Internal )
        strType = _T("&nbsp;");

    // Get Time and Date
    _tzset();
    _tstrdate( strDate );
    _tstrtime( strTime );

    // Build HTML for message
    mFile << _T("  <tr>\n")
              _T("    <td valign='top'><span class='log_date'>") << strTime << _T("</span></td>\n")
              _T("    <td valign='top'>") << strType << _T("</td>\n")
              _T("    <td><span class='log_message'>") << strOutput << _T("</span></td>\n")
              _T("  </tr>\n");
    
    // Flush file (in case a crash occurs)
    mFile.flush();
}

//-----------------------------------------------------------------------------
//  Name : writeSeparator()
/// <summary>
/// Called by the cgAppLog class in order for us to write a horizontal
/// separator to the output stream.
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputFile::writeSeparator( cgUInt32 nType )
{
    // Validate requirements
    if ( !isOpen() ) return;

    // Output rule to the file
    mFile << _T("<tr><td colspan=\"3\"><hr/></td></tr>\n");
    
    // Flush file (in case a crash occurs)
    mFile.flush();
}

//-----------------------------------------------------------------------------
//  Name : isOpen()
/// <summary>
/// Called by various sources to determine if this output stream is
/// currently open for writing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLogOutputFile::isOpen() const
{
    // FixMe: STLPort incompatibility
    return true; //mFile.is_open();
}

//-----------------------------------------------------------------------------
//  Name : writeHeader()
/// <summary>
/// Called by the cgAppLog class in order for us to write out the header
/// information (i.e. the information that appears at the top of the log)
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputFile::writeHeader(  )
{
    cgTChar strDate[128], strTime[128];

    // Validate requirements
    if ( !isOpen() ) return;

    // Build standard HTML header data.
    mFile << _T("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0 Transitional//EN'>\n")
              _T("<HTML>\n")
              _T("<HEAD>\n")
              _T("<TITLE>") << mLogTitle << _T("</TITLE>\n")
              _T("<LINK REL='SHORTCUT ICON' HREF='http://www.gameinstitute.com/favicon.ico'>\n")
              _T("<STYLE type='text/css'>\n")
              _T("<!--\n")
              _T("body,td,th {\n")
              _T("  font-family: Verdana, Arial, Helvetica, sans-serif;\n")
              _T("  font-size: 9pt;\n")
              _T("  color: #000000;\n")
              _T("}\n")
              _T("pre {\n")
              _T("  font-size: 9pt;\n")
              _T("}\n")
              _T("a:link {\n")
              _T("	color: #7e9e51;\n")
              _T("}\n")
              _T("a:visited {\n")
              _T("  color: #798767;\n")
              _T("}\n")
              _T("a:hover {\n")
              _T("  color: #FF0000;\n")
              _T("}\n")
              _T("a:active {\n")
              _T("  color: #FF0000;\n")
              _T("}\n")
              _T(".app_title {\n")
              _T("	font-family: Verdana, Arial, Helvetica, sans-serif;\n")
              _T("	font-size: 24pt;\n")
              _T("	font-weight: bold;\n")
              _T("	color: #990000;\n")
              _T("	font-style: normal;\n")
              _T("}\n")
              _T(".log_title {\n")
              _T("	font-family: Verdana, Arial, Helvetica, sans-serif;\n")
              _T("	font-size: 16pt;\n")
              _T("	font-weight: bold;\n")
              _T("}\n")
              _T(".section_title {\n")
              _T("	font-size: 10pt;\n")
              _T("	font-style: italic;\n")
              _T("	margin-left: 10px;\n")
              _T("}\n")
              _T(".process_title {\n")
              _T("	font-size: 12pt;\n")
              _T("	font-weight: bold;\n")
              _T("	border: 1px solid #999999;\n")
              _T("	padding: 2px;\n")
              _T("}\n")
              _T(".success {color: #990000}\n")
              _T(".table_style {\n")
              _T("	margin-left: 10px;\n")
              _T("	border: 1px dotted #999999;\n")
              _T("}\n")
              _T(".log_date {\n")
              _T("	font-family: 'Courier New', Courier, monospace;\n")
              _T("	font-weight: normal;\n")
              _T("	font-size: 10pt;\n")
              _T("}\n")
              _T(".log_message {\n")
              _T("	font-family: 'Courier New', Courier, monospace;\n")
              _T("	font-weight: normal;\n")
              _T("	font-size: 10pt;\n")
              _T("}\n")
              _T("-->\n")
              _T("</STYLE>\n")
              _T("</HEAD>\n")
              _T("<body>\n")
              _T("<div style='width:800'>\n")
              _T("<div align='justify'><span class='app_title'>") << mApplicationTitle << _T(" </span><span class='log_title'>- Information Log</span></div>\n")
              _T("<hr/>\n")
              _T("<p align='justify'>If you have any problems with this application, please post a copy of this file along with a copy of the file <em>&quot;&lt;installation-path&gt;\\System\\Config\\config.ini&quot;</em> and a detailed description of your problem to the relevant support forum. Please include a list of any relevant hardware or software configurations that may have been reported incorrectly within this debug output in addition to any other information you believe may be in some way related to the fault that occured.</p>\n");

    // Flush file (in case a crash occurs)
    mFile.flush();

    // Now detect hardware configurations
    detectHardware();

    // Get Time and Date
    _tzset();
    _tstrdate( strDate );
    _tstrtime( strTime );

    // Begin logging
    mFile << _T("<p class='process_title'><img src='") + mIconPath + _T("log.png' width='16' height='16' align='absmiddle'> Log Session Opened<span class='log_date'> (") + cgString(strDate) + _T(" ") + cgString(strTime) + _T(")</span></p>\n")
              _T("<table width='789' border='0' cellpadding='2' cellspacing='0' class='table_style'>\n");

    // Flush file (in case a crash occurs)
    mFile.flush();
}

//-----------------------------------------------------------------------------
//  Name : detectHardware () (Private)
/// <summary>
/// Detect the end-users hardware and dump that information out to the
/// debug log.
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputFile::detectHardware( )
{
    cgString        strRootPath = _T("a:\\");
    cgString        strFreePhysicalMem, strTotalPhysicalMem, strPercentPhysicalMem;
    cgString        strFreePagingMem, strTotalPagingMem, strPercentPagingMem;
    MEMORYSTATUS    MemInfo;

    // Write formatted info
    mFile << _T("<p class='process_title'><img src='") << mIconPath << _T("computer.png' width='16' height='16' align='absmiddle'> Detecting Hardware Configuration</p>\n")
              _T("<p class='section_title'><img src='") << mIconPath << _T("memory.png' width='16' height='16' align='absmiddle'> Detecting Memory Configuration... <span class='success'>Success</span></p>\n");

    // Flush file (in case a crash occurs)
    mFile.flush();

    // Use GlobalMemoryStatus to get memory details
    memset( &MemInfo, 0, sizeof( MEMORYSTATUS ) );
	MemInfo.dwLength = sizeof( MEMORYSTATUS );
	GlobalMemoryStatus( &MemInfo );

    // Compute the memory info
    strFreePhysicalMem      = cgString::format( _T("%i Kb\n"), (MemInfo.dwAvailPhys / 1024) );
    strTotalPhysicalMem     = cgString::format( _T("%i Kb\n"), (MemInfo.dwTotalPhys / 1024) );
    strFreePagingMem        = cgString::format( _T("%i Kb\n"), (MemInfo.dwAvailPageFile / 1024) );
    strTotalPagingMem       = cgString::format( _T("%i Kb\n"), (MemInfo.dwTotalPageFile / 1024) );
    strPercentPhysicalMem   = cgString::format( _T("%i%%"), 100 - (int)(((cgFloat)MemInfo.dwAvailPhys / (cgFloat)MemInfo.dwTotalPhys) * 100.0f) );
    strPercentPagingMem     = cgString::format( _T("%i%%"), 100 - (int)(((cgFloat)MemInfo.dwAvailPageFile / (cgFloat)MemInfo.dwTotalPageFile ) * 100.0f) );

    // Write the info out to disk
    mFile << _T("<table width='400' border='0' cellpadding='2' cellspacing='0' class='table_style'>\n")
              _T("  <tr>\n")
              _T("    <td><strong>Type</strong></td>\n")
              _T("    <td><strong>Free</strong></td>\n")
              _T("    <td><strong>Total</strong></td>\n")
              _T("    <td><strong>In Use </strong></td>\n")
              _T("  </tr>\n")
              _T("  <tr>\n")
              _T("    <td><img src='") << mIconPath << _T("memory.png' width='16' height='16' align='absmiddle'> Physical Memory</td>\n")
              _T("    <td>") << strFreePhysicalMem << _T("</td>\n")
              _T("    <td>") << strTotalPhysicalMem << _T("</td>\n")
              _T("    <td>") << strPercentPhysicalMem << _T("</td>\n")
              _T("  </tr>\n")
              _T("  <tr>\n")
              _T("    <td><img src='") << mIconPath << _T("removable_drive.png' width='16' height='16' align='absmiddle'> Paging Memory</td>\n")
              _T("    <td>") << strFreePagingMem << _T("</td>\n")
              _T("    <td>") << strTotalPagingMem << _T("</td>\n")
              _T("    <td>") << strPercentPagingMem << _T("</td>\n")
              _T("  </tr>\n")
              _T("</table>\n");

    // Write header for dumping drive info
    mFile << _T("<p class='section_title'><img src='") << mIconPath << _T("removable_drive.png' width='16' height='16' align='absmiddle'> Detecting Drive Configuration... <span class='success'>Success</span></p>\n")
              _T("<table width='789' border='0' cellpadding='2' cellspacing='0' class='table_style'>\n")
              _T("  <tr>\n")
              _T("    <td><strong>Letter</strong></td>\n")
              _T("    <td><strong>Type</strong></td>\n")
              _T("    <td><strong>Label</strong></td>\n")
              _T("    <td><strong>Serial</strong></td>\n")
              _T("    <td><strong>Free</strong></td>\n")
              _T("    <td><strong>Total</strong></td>\n")
              _T("    <td><strong>In Use</strong></td>\n")
              _T("  </tr>\n");

    // Flush file (in case a crash occurs)
    mFile.flush();
    
    // Loop through drives from c to z
    for ( cgChar c = 'C'; c < 'Z'; ++c )
    {
        // Update Root Path
        strRootPath[0] = (cgTChar)c;

        // Get Drive Type
        cgUInt32 DriveType = GetDriveType( strRootPath.c_str() );
        
        // Skip invalid drive types
        if ( DriveType == DRIVE_NO_ROOT_DIR ) continue;

        // Begin to build string
        cgString strDriveType;
        bool     bGetInfo = true;
        
        // Add Drive Type
        switch ( DriveType )
        {
            case DRIVE_UNKNOWN:
                strDriveType = _T("<img src='") + mIconPath + _T("fixed_drive.png' width='16' height='16' align='absmiddle'> Unknown Drive Type");
                bGetInfo     = false;
                break;

            case DRIVE_REMOVABLE:
                strDriveType = _T("<img src='") + mIconPath + _T("removable_drive.png' width='16' height='16' align='absmiddle'> Removable Disk Drive");
                break;

            case DRIVE_FIXED:
                strDriveType = _T("<img src='") + mIconPath + _T("fixed_drive.png' width='16' height='16' align='absmiddle'> Fixed Disk Drive");
                break;

            case DRIVE_REMOTE:
                strDriveType = _T("<img src='") + mIconPath + _T("network_drive.png' width='16' height='16' align='absmiddle'> Remote Network Drive");
                bGetInfo     = false;
                break;

            case DRIVE_CDROM:
                strDriveType = _T("<img src='") + mIconPath + _T("cddvd_drive.png' width='16' height='16' align='absmiddle'> CD/DVD Drive");
                bGetInfo     = false;
                break;

            case DRIVE_RAMDISK:
                strDriveType = _T("<img src='") + mIconPath + _T("memory.png' width='16' height='16' align='absmiddle'> RAM Drive");
                break;

            default:
                continue;
        
        } // End Drive Type Switch

        // Output Drive Info
        mFile << _T("  <tr>\n")
                  _T("    <td>") << strRootPath << _T("</td>\n")
                  _T("    <td>") << strDriveType << _T("</td>\n");

        // Get the volume label
        cgTChar  NameBuffer[128];
        cgUInt32 nSerialNumber;
        cgString strSerialNumber;
        if ( GetVolumeInformation( strRootPath.c_str(), NameBuffer, 127, &nSerialNumber, CG_NULL, CG_NULL, CG_NULL, 0 ) )
        {
            // If volume has no label, print that information
            if ( _tcslen( NameBuffer ) == 0 ) _tcscpy( NameBuffer, _T("Unnamed Volume") );

            // Get string representation of serial number
            strSerialNumber = cgString::format( _T("0x%x"), nSerialNumber );

            // Output drive label / serial
            mFile << _T("    <td>") << NameBuffer << _T("</td>\n")
                      _T("    <td>") << strSerialNumber << _T("</td>\n");
            
        } // End if got volume label etc
        else
        {
            // No volume info available, so do not attempt to get space
            bGetInfo = false;

            // Output drive label / serial
            mFile << _T("    <td>n/a</td>\n")
                      _T("    <td>n/a</td>\n");

        } // End if no volume info

        // Retrieve the drive info
        cgInt64 TotalSpace, FreeSpace;
        if ( bGetInfo && GetDiskFreeSpaceEx( strRootPath.c_str(), CG_NULL, (PULARGE_INTEGER)&TotalSpace, (PULARGE_INTEGER)&FreeSpace ) )
        {
            cgString strFreeSpace, strTotalSpace, strPercentSpace;
            
            // Get drive space information
            strFreeSpace    = cgString::format( _T("%.0f Kb"), (cgDouble)(FreeSpace / 1024) );
            strTotalSpace   = cgString::format( _T("%.0f Kb"), (cgDouble)(TotalSpace / 1024) );
            strPercentSpace = cgString::format( _T("%i%%"), 100 - (int)(((cgFloat)FreeSpace / (cgFloat)TotalSpace) * 100.0f) );
            
            // Write space info
            mFile << _T("    <td>") << strFreeSpace << _T("</td>\n")
                      _T("    <td>") << strTotalSpace << _T("</td>\n")
                      _T("    <td>") << strPercentSpace << _T("</td>\n")
                      _T("  </tr>");
            
        
        } // End if got disk free space
        else
        {
            // Write blank rows info
            mFile << _T("    <td>n/a</td>\n")
                      _T("    <td>n/a</td>\n")
                      _T("    <td>n/a</td>\n")
                      _T("  </tr>");
        
        } // End if no free space info

    } // Next Drive Letter

    // Finish up and flush
    mFile << _T("</table>");
    mFile.flush();

}

//-----------------------------------------------------------------------------
//  Name : writeFooter ()
/// <summary>
/// Called by the cgAppLog class in order for us to write out the footer
/// information (i.e. the information that appears at the bottom of the 
/// log)
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputFile::writeFooter(  )
{
    cgTChar strDate[128], strTime[128];

    // Validate requirements
    if ( !isOpen() ) return;

    // Get Time and Date
    _tzset();
    _tstrdate( strDate );
    _tstrtime( strTime );

    // Build standard HTML footer data.
    mFile << _T("</table>\n")
              _T("<p class='process_title'>Log Session Closed<span class='log_date'> (") << strDate << _T(" ") << strTime << _T(")</span></p>\n")
              _T("</div>\n")
              _T("</body>\n")
              _T("</HTML>\n");
    
    // Flush file (in case a crash occurs)
    mFile.flush();
}

///////////////////////////////////////////////////////////////////////////////
// cgLogOutputStd Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLogOutputStd() (Constructor)
/// <summary>
/// cgLogOutputStd Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgLogOutputStd::cgLogOutputStd( bool bOpenConsole /* = false */ )
{
    // Initialize variables
    mConsoleOwned = false;

    // Should we open a console window and redirect output to it?
    if ( bOpenConsole )
    {
        AllocConsole();
        freopen("CONOUT$", "a", stdout);
        mConsoleOwned = true;
    
    } // End if open console window
}

//-----------------------------------------------------------------------------
//  Name : cgLogOutputStd () (Destructor)
/// <summary>
/// cgLogOutputStd Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgLogOutputStd::~cgLogOutputStd()
{
    // Did we open a console window?
    if ( mConsoleOwned )
    {
        FreeConsole();
        mConsoleOwned = false;

    } // End if open console window
}

//-----------------------------------------------------------------------------
//  Name : write()
/// <summary>
/// Called by the cgAppLog class passing in any message that was written.
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputStd::write( cgUInt32 nType, const cgString & strMessage )
{
    cgString strType = _T("  "), strPadding = _T("  ");

    // Build prepend string based on type
    if ( nType == cgAppLog::Debug )
    {
        strType    = _T("> ");
        strPadding = _T("  ");
    }
    else if ( nType & cgAppLog::Warning )
    {
        strType    = _T("# Warning : ");
        strPadding = _T("  ");
    }
    else if ( nType & cgAppLog::Error )
    {
        strType    = _T("! Error : ");
        strPadding = _T("  ");
    }
    else if ( nType & cgAppLog::Info )
    {
        strType    = _T("* ");
        strPadding = _T("  ");
    }
    else if ( nType & cgAppLog::Internal )
    {
        strType    = _T("");
        strPadding = _T("");
    }

    // Just print the message to the standard output stream.
    #if defined( UNICODE ) || defined(_UNICODE)
        std::wcout << cgString::wordWrap( strType + strMessage, getOutputWidth(), strPadding );
    #else
        std::cout << cgString::wordWrap( strType + strMessage, getOutputWidth(), strPadding );
    #endif
}

//-----------------------------------------------------------------------------
//  Name : writeSeparator()
/// <summary>
/// Called by the cgAppLog class in order for us to write a horizontal
/// separator to the output stream.
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputStd::writeSeparator( cgUInt32 nType )
{
    // Just print a series of hyphens to the standard output stream.
    #if defined( UNICODE ) || defined(_UNICODE)
        std::wcout << cgString( getOutputWidth(), _T('-') ) << std::endl;
    #else
        std::cout << cgString( getOutputWidth(), _T('-') ) << std::endl;
    #endif
}

//-----------------------------------------------------------------------------
//  Name : writeHeader()
/// <summary>
/// Called by the cgAppLog class in order for us to write out the header
/// information (i.e. the information that appears at the top of the log)
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputStd::writeHeader(  )
{
    // Nothing in this implementation
}

//-----------------------------------------------------------------------------
//  Name : writeFooter ()
/// <summary>
/// Called by the cgAppLog class in order for us to write out the footer
/// information (i.e. the information that appears at the bottom of the 
/// log)
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputStd::writeFooter(  )
{
    // Nothing in this implementation
}

///////////////////////////////////////////////////////////////////////////////
// cgLogOutputScript Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLogOutputScript() (Constructor)
/// <summary>
/// cgLogOutputScript Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgLogOutputScript::cgLogOutputScript( const cgString & strHeaderMethod, const cgString & strFooterMethod, const cgString & strWriteMethod, const cgString & strSeparatorMethod, cgScriptObject * pScriptObject )
{
    // Initialize variables
    mScriptObject         = pScriptObject;
    mWriteMethod        = strWriteMethod;
    mSeparatorMethod    = strSeparatorMethod;
    mHeaderMethod       = strHeaderMethod;
    mFooterMethod       = strFooterMethod;

    // Hold a reference to the script object
    if ( pScriptObject )
        pScriptObject->addRef();
}

//-----------------------------------------------------------------------------
//  Name : cgLogOutputScript () (Destructor)
/// <summary>
/// cgLogOutputScript Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgLogOutputScript::~cgLogOutputScript()
{
    // Release references.
    if ( mScriptObject )
        mScriptObject->release();

    // Clear variables
    mScriptObject = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : write()
/// <summary>
/// Called by the cgAppLog class passing in any message that was written.
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputScript::write( cgUInt32 nType, const cgString & strMessage )
{
    // Any method?
    if ( mWriteMethod.empty() )
        return;

    // We're calling object methods.
    try
    {
        cgScriptArgument::Array ScriptArgs;
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&nType ) );
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"), (void*)&strMessage ) );
        mScriptObject->executeMethodVoid( mWriteMethod, ScriptArgs, false );
        
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute %s() method in '%s'. The engine reported the following error: %s.\n"), mWriteMethod.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
    
    } // End catch exception
}

//-----------------------------------------------------------------------------
//  Name : writeSeparator()
/// <summary>
/// Called by the cgAppLog class in order for us to write a horizontal
/// separator to the output stream.
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputScript::writeSeparator( cgUInt32 nType )
{
    // Any method?
    if ( mSeparatorMethod.empty() )
        return;

    // We're calling object methods.
    try
    {
        cgScriptArgument::Array ScriptArgs;
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&nType ) );
        mScriptObject->executeMethodVoid( mSeparatorMethod, ScriptArgs, false );
        
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute %s() method in '%s'. The engine reported the following error: %s.\n"), mWriteMethod.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
    
    } // End catch exception
}

//-----------------------------------------------------------------------------
//  Name : writeHeader()
/// <summary>
/// Called by the cgAppLog class in order for us to write out the header
/// information (i.e. the information that appears at the top of the log)
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputScript::writeHeader(  )
{
    // Any method?
    if ( mHeaderMethod.empty() )
        return;

    // We're calling object methods.
    try
    {
        cgScriptArgument::Array ScriptArgs;
        mScriptObject->executeMethodVoid( mHeaderMethod, ScriptArgs, false );
        
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute %s() method in '%s'. The engine reported the following error: %s.\n"), mWriteMethod.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
    
    } // End catch exception
}

//-----------------------------------------------------------------------------
//  Name : writeFooter ()
/// <summary>
/// Called by the cgAppLog class in order for us to write out the footer
/// information (i.e. the information that appears at the bottom of the 
/// log)
/// </summary>
//-----------------------------------------------------------------------------
void cgLogOutputScript::writeFooter(  )
{
    // Any method?
    if ( mFooterMethod.empty() )
        return;

    // We're calling object methods.
    try
    {
        cgScriptArgument::Array ScriptArgs;
        mScriptObject->executeMethodVoid( mFooterMethod, ScriptArgs, false );
        
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute %s() method in '%s'. The engine reported the following error: %s.\n"), mWriteMethod.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
    
    } // End catch exception
}