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
// File : cgWorldQuery.cpp                                                   //
//                                                                           //
// Desc : Provides support for running queries against the main world        //
//        database. This includes the selection of existing data from the    //
//        database as well as inserting and updating data.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWorldQuery Module Includes
//-----------------------------------------------------------------------------
#include <World/cgWorldQuery.h>
#include <World/cgWorld.h>
#include <SQLite/sqlite3.h>

//-----------------------------------------------------------------------------
// Module Local Structures
//-----------------------------------------------------------------------------
namespace
{
    // Custom file structure used to integrate sqlite3 with our
    // in-memory database stream.
    struct MemoryDBFile
    {
        sqlite3_file    VFSFile;
        cgInputStream * pStream;
        cgInt           Flags;
        cgByte        * pBuffer;
        size_t          BufferSize;
    
    }; // End Struct MemoryDBFile

} // End Unnamed Namespace

///////////////////////////////////////////////////////////////////////////////
// cgWorldQuery Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWorldQuery() (Constructor)
/// <summary>
/// Class constructor.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldQuery::cgWorldQuery( )
{
    // Initialize variables to sensible defaults
    mWorld            = CG_NULL;
    mDatabase         = CG_NULL;
    mStatements      = CG_NULL;
    mCurrentStatement = -1;
    mStatementCount       = 0;
    mHasResults       = false;
    mFirstRowCached   = false;
    mErrorOccurred    = false;
}

//-----------------------------------------------------------------------------
//  Name : cgWorldQuery() (Constructor)
/// <summary>
/// Class constructor.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldQuery::cgWorldQuery( cgWorld * pWorld, const cgString & strStatements )
{
    // Initialize variables to sensible defaults
    mWorld            = CG_NULL;
    mDatabase         = CG_NULL;
    mStatements      = CG_NULL;
    mCurrentStatement = -1;
    mStatementCount       = 0;
    mHasResults       = false;
    mFirstRowCached   = false;
    mErrorOccurred    = false;

    // Pass through to preparation.
    prepare( pWorld, strStatements );
}

//-----------------------------------------------------------------------------
//  Name : cgWorldQuery() (Constructor)
/// <summary>
/// Class constructor.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldQuery::cgWorldQuery( sqlite3 * pDatabase, const cgString & strStatements )
{
    // Initialize variables to sensible defaults
    mWorld            = CG_NULL;
    mDatabase         = CG_NULL;
    mStatements      = CG_NULL;
    mCurrentStatement = -1;
    mStatementCount       = 0;
    mHasResults       = false;
    mFirstRowCached   = false;
    mErrorOccurred    = false;

    // Pass through to preparation.
    prepare( pDatabase, strStatements );
}

//-----------------------------------------------------------------------------
//  Name : ~cgWorldQuery () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldQuery::~cgWorldQuery( )
{
    // Clean up
    unprepare();
}

//-----------------------------------------------------------------------------
//  Name : prepare( )
/// <summary>
/// Prepare the query statements ready for execution (with 'step').
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::prepare( cgWorld * pWorld, const cgString & strStatements, bool bVerbose /* = false */ )
{
    // Clear any prior statements.
    unprepare( );

    // Reset error information
    setErrorState( false );

    // Store required information.
    mWorld    = pWorld;
    mDatabase = pWorld->getDatabaseConnection();

    // Attempt to prepare each component of the statement.
    const cgTChar * lpszSQL = strStatements.c_str();
    while ( lpszSQL != NULL )
    {
        sqlite3_stmt * pStatement = NULL;

        // Prepare / compile the next piece of the statement (up to the next semi-colon).
        #if defined(UNICODE) || defined(_UNICODE)
            int nPrepareResult = sqlite3_prepare16_v2( mDatabase, lpszSQL, (int)(wcslen(lpszSQL) * sizeof(cgWChar)), &pStatement, (const void**)&lpszSQL );
        #else // UNICODE
            int nPrepareResult = sqlite3_prepare_v2( mDatabase, lpszSQL, (int)(strlen(lpszSQL) * sizeof(cgChar)), &pStatement, (const void**)&lpszSQL );
        #endif // !UNICODE
        
        // Bail out of nothing was available to prepare (not an error, we're just done).
        if ( nPrepareResult == SQLITE_OK && pStatement == NULL )
            break;

        // An error occured?
        if ( nPrepareResult != SQLITE_OK )
        {
            // Unroll preparation.
            setErrorState( true, bVerbose );
            unprepare();
            return false;

        } // End if failed

        // Add this new statement to the list.
        sqlite3_stmt ** ppStatements = new sqlite3_stmt*[mStatementCount+1];
        if ( mStatements != NULL )
        {
            memcpy( ppStatements, mStatements, mStatementCount * sizeof(sqlite3_stmt*) );
            delete []mStatements;
        
        } // End if allocated
        mStatements = ppStatements;
        mStatements[ mStatementCount++ ] = pStatement;

    } // Next section of the statement.

    // Register ourselves with the parent world's event mechanism
    // so that we know when it is being disposed and the database
    // connection closed.
    if ( mWorld != CG_NULL )
        mWorld->registerEventListener( this );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : prepare( )
/// <summary>
/// Prepare the query statements ready for execution (with 'step').
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::prepare( sqlite3 * pDatabase, const cgString & strStatements, bool bVerbose /* = false */ )
{
    // Clear any prior statements.
    unprepare( );

    // Reset error information
    setErrorState( false );

    // Store required information.
    mWorld    = CG_NULL;
    mDatabase = pDatabase;

    // Attempt to prepare each component of the statement.
    const cgTChar * lpszSQL = strStatements.c_str();
    while ( lpszSQL != NULL )
    {
        sqlite3_stmt * pStatement = NULL;

        // Prepare / compile the next piece of the statement (up to the next semi-colon).
        #if defined(UNICODE) || defined(_UNICODE)
            int nPrepareResult = sqlite3_prepare16_v2( mDatabase, lpszSQL, (int)(wcslen(lpszSQL) * sizeof(cgWChar)), &pStatement, (const void**)&lpszSQL );
        #else // UNICODE
            int nPrepareResult = sqlite3_prepare_v2( mDatabase, lpszSQL, (int)(strlen(lpszSQL) * sizeof(cgChar)), &pStatement, (const void**)&lpszSQL );
        #endif // !UNICODE
        
        // Bail out of nothing was available to prepare (not an error, we're just done).
        if ( nPrepareResult == SQLITE_OK && pStatement == NULL )
            break;

        // An error occured?
        if ( nPrepareResult != SQLITE_OK )
        {
            // Unroll preparation.
            setErrorState( true, bVerbose );
            unprepare();
            return false;

        } // End if failed

        // Add this new statement to the list.
        sqlite3_stmt ** ppStatements = new sqlite3_stmt*[mStatementCount+1];
        if ( mStatements != NULL )
        {
            memcpy( ppStatements, mStatements, mStatementCount * sizeof(sqlite3_stmt*) );
            delete []mStatements;
        
        } // End if allocated
        mStatements = ppStatements;
        mStatements[ mStatementCount++ ] = pStatement;

    } // Next section of the statement.

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unprepare( )
/// <summary>
/// Dispose of any information already contained within this query ready 
/// for a new preparation. 
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldQuery::unprepare( )
{
    // Unregister ourselves from the parent world's event mechanism.
    if ( mWorld != CG_NULL )
        mWorld->unregisterEventListener( this );

    // Clear any prepared statements.
    if ( mStatements != NULL )
    {
        for ( int i = 0; i < mStatementCount; ++i )
        {
            if ( mStatements[i] != NULL )
                sqlite3_finalize( mStatements[i] );
        
        } // Next Statement
        delete []mStatements;

    } // End if prepared

    // Clear result column cache.
    mResultColumns.clear();
    
    // Clear variables
    mWorld            = CG_NULL;
    mDatabase         = CG_NULL;
    mStatements      = CG_NULL;
    mStatementCount       = 0;
    mCurrentStatement = -1;
    mHasResults       = false;
    mFirstRowCached   = false;
}

//-----------------------------------------------------------------------------
//  Name : step( )
/// <summary>
/// Step the execution of the query to the next compiled statement. 
/// Returns false if there are no more statements to execute. Specify
/// 'true' to the bAutoReset parameter in order to automatically call
/// the reset() method upon completion.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::step( bool bAutoReset )
{
    // Perform regular step.
    bool bResult = step();
    
    // Always reset, even if the step failed
    // but do not clear error result.
    if ( bAutoReset == true )
        reset( false );
    
    // Return result
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : step( )
/// <summary>
/// Step the execution of the query to the next compiled statement. 
/// Returns false if there are no more statements to execute.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::step( )
{
    // Reset error status
    setErrorState( false );

    // Any connection?
    if ( mDatabase == NULL )
    {
        setErrorState( true );
        return false;
    
    } // End if not connected

    // Anything to execute (not an error, just will not step)?
    if ( mStatementCount == 0 )
        return false;
    
    // Move on to the next statement.
    mCurrentStatement++;

    // Anything more to execute?
    if ( mCurrentStatement >= mStatementCount )
    {
        // Prevent endless increment (limit to 1 past the last element)
        if ( mCurrentStatement > mStatementCount )
            mCurrentStatement--;
        
        // Set error status
        setErrorState( true );
        mLastError = _T("No further statements are available for execution. Make sure that the query has been correctly reset.");
        return false;
    
    } // End if fully executed

    // No results yet.
    mHasResults = false;
    mFirstRowCached = false;
    mResultColumns.clear();
    
    // Execute the next statement.
    int nStepResult = sqlite3_step( mStatements[mCurrentStatement] );
    switch ( nStepResult )
    {
        case SQLITE_ROW:
            mHasResults = true;
            mFirstRowCached = true;
            return true;
        case SQLITE_DONE:
            return true;
        default:
            setErrorState( true );
            return false;

    } // End switch result
}

//-----------------------------------------------------------------------------
//  Name : stepAll( )
/// <summary>
/// Execute all statements.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldQuery::stepAll( )
{
    // Continue to step until we're out of statements.
    while ( step() == true )
    {
    }
}

//-----------------------------------------------------------------------------
//  Name : nextRow( )
/// <summary>
/// Get the first/next row in the set of results from the most recently
/// executed statement. Returns false when no more results are available.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::nextRow( )
{
    // Reset error status
    setErrorState( false );

    // Anything to return ?
    if ( mHasResults == false )
        return false;

    // First row already cached by initial step?
    sqlite3_stmt * pStatement = mStatements[mCurrentStatement];
    if ( mFirstRowCached == true )
    {
        cgInt16 nColumnCount = (cgInt16)sqlite3_column_count( pStatement );

        // Reconstruct result column data.
        mResultColumns.clear();
        for ( cgInt16 i = 0; i < nColumnCount; ++i )
        {
            #if defined(UNICODE) || defined(_UNICODE)
                mResultColumns[cgString((const cgWChar*)sqlite3_column_name16( pStatement, i ))] = i;
            #else // UNICODE
                mResultColumns[cgString((const cgChar*)sqlite3_column_name( pStatement, i ))] = i;
            #endif // !UNICODE
        
        } // Next Column

        // Caller can now read.
        mFirstRowCached = false;
        return true;
    
    } // End if already retrieved

    // Otherwise we need to step the current statement and get the next row.
    int nStepResult = sqlite3_step( pStatement );
    switch ( nStepResult )
    {
        case SQLITE_ROW:
            // More results available
            return true;

        case SQLITE_DONE:
            // No more results are available
            mHasResults = false;
            mResultColumns.clear();
            return false;

        default:
            // An error occured.
            setErrorState( true );
            return false;

    } // End switch result
}

//-----------------------------------------------------------------------------
//  Name : reset( )
/// <summary>
/// Reset the entire query so it can be run again.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::reset( bool bClearErrorState /* = true */ )
{
    // Reset error status
    if ( bClearErrorState == true )
        setErrorState( false );

    // Any connection?
    if ( mDatabase == NULL )
    {
        setErrorState( true );
        return false;
    
    } // End if not connected

    // Anything to reset (not an error)?
    if ( mStatementCount == 0 )
        return true;

    // Reset all statements.
    for ( int i = 0; i < mStatementCount; ++i )
    {
        sqlite3_reset( mStatements[ i ] );
        sqlite3_clear_bindings( mStatements[ i ] );
    
    } // Next Statement

    // Start at the beginning once again.
    mCurrentStatement = -1;
    mHasResults       = false;
    mFirstRowCached   = false;
    mResultColumns.clear();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : resetCurrent( )
/// <summary>
/// Reset the current step only in order to execute again.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::resetCurrent( )
{
    // Reset error status
    setErrorState( false );

    // Any connection?
    if ( mDatabase == NULL )
    {
        setErrorState( true );
        return false;
    
    } // End if not connected

    // Anything to reset (not an error)?
    if ( mStatementCount == 0 || mCurrentStatement < 0 )
        return true;

    // Reset the last executed statement.
    if ( mCurrentStatement >= 0 && mCurrentStatement < mStatementCount )
    {
        sqlite3_reset( mStatements[ mCurrentStatement ] );
        sqlite3_clear_bindings( mStatements[ mCurrentStatement ] );
    
    } // End if active statement

    // Move back so that this statement can be stepped again.
    mCurrentStatement--;
    mHasResults       = false;
    mFirstRowCached   = false;
    mResultColumns.clear();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getColumn(*)
/// <summary>
/// Retrieve data associated with the specified column by name.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::getColumn( const cgString & strIndex, cgUInt & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = (cgUInt)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgUInt & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = (cgUInt)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, cgInt & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgInt & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, cgUInt32 & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = (cgUInt32)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgUInt32 & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = (cgUInt32)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, bool & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = (sqlite3_column_int( mStatements[mCurrentStatement], nColumn ) != 0);
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, bool & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = (sqlite3_column_int( mStatements[mCurrentStatement], nColumn ) != 0);
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, cgInt32 & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = (cgInt32)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgInt32 & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = (cgInt32)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, cgUInt16 & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = (cgUInt16)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgUInt16 & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = (cgUInt16)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, cgInt16 & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = (cgInt16)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgInt16 & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = (cgInt16)sqlite3_column_int( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, cgFloat & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = (cgFloat)sqlite3_column_double( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgFloat & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = (cgFloat)sqlite3_column_double( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, cgDouble & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    Value = (cgDouble)sqlite3_column_double( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgDouble & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    Value = (cgDouble)sqlite3_column_double( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, cgString & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    const cgTChar * pData = CG_NULL;
    #if defined(UNICODE) || defined(_UNICODE)
        pData = (const cgWChar*)sqlite3_column_text16( mStatements[mCurrentStatement], nColumn );
    #else // UNICODE
        pData = (const cgChar*)sqlite3_column_text( mStatements[mCurrentStatement], nColumn );
    #endif // !UNICODE

    // Pass data back out.
    if ( pData == CG_NULL )
        Value.clear();
    else
        Value = pData;

    // Success!
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, cgString & Value )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    const cgTChar * pData = CG_NULL;
    #if defined(UNICODE) || defined(_UNICODE)
        pData = (const cgWChar*)sqlite3_column_text16( mStatements[mCurrentStatement], nColumn );
    #else // UNICODE
        pData = (const cgChar*)sqlite3_column_text( mStatements[mCurrentStatement], nColumn );
    #endif // !UNICODE

    // Pass data back out.
    if ( pData == CG_NULL )
        Value.clear();
    else
        Value = pData;

    // Success!
    return true;
}
bool cgWorldQuery::getColumn( const cgString & strIndex, void ** ppBlobOut, cgUInt32 & nSizeOut )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;

    // Get column by name.
    cgInt16 nColumn = -1;
    ResultColumnMap::iterator itColumn = mResultColumns.find( strIndex );
    if ( itColumn == mResultColumns.end() )
        return false;
    else
        nColumn = itColumn->second;
    
    // Retrieve the column data.
    nSizeOut = sqlite3_column_bytes( mStatements[mCurrentStatement], nColumn );
    if ( ppBlobOut != CG_NULL )
        *ppBlobOut = (void*)sqlite3_column_blob( mStatements[mCurrentStatement], nColumn );
    return true;
}
bool cgWorldQuery::getColumn( cgInt16 nColumn, void ** ppBlobOut, cgUInt32 & nSizeOut )
{
    // Reset error status
    setErrorState( false );

    // Anything to return?
    if ( mHasResults == false )
        return false;
    
    // Retrieve the column data.
    nSizeOut = sqlite3_column_bytes( mStatements[mCurrentStatement], nColumn );
    if ( ppBlobOut != CG_NULL )
        *ppBlobOut = (void*)sqlite3_column_blob( mStatements[mCurrentStatement], nColumn );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : bindParameter(*)
/// <summary>
/// Bind data to any parameter prepared within this statement.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::bindParameter( cgInt16 nIndex, const cgString & Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    bool bResult = true;
    #if defined(UNICODE) || defined(_UNICODE)
        bResult = ( sqlite3_bind_text16( mStatements[mCurrentStatement + 1], nIndex, Value.c_str(), -1, SQLITE_TRANSIENT ) == SQLITE_OK );
    #else // UNICODE
        bResult = ( sqlite3_bind_text( mStatements[mCurrentStatement + 1], nIndex, Value.c_str(), -1, SQLITE_TRANSIENT ) == SQLITE_OK );
    #endif // !UNICODE
    
    // Success?
    if ( bResult == false )
    {
        setErrorState( true );
        return false;
    
    } // End if failed
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, bool Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_int( mStatements[mCurrentStatement + 1], nIndex, (cgInt32)Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, cgFloat Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_double( mStatements[mCurrentStatement + 1], nIndex, (cgDouble)Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, cgDouble Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_double( mStatements[mCurrentStatement + 1], nIndex, Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, cgUInt32 Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_int( mStatements[mCurrentStatement + 1], nIndex, (cgInt32)Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, cgInt32 Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_int( mStatements[mCurrentStatement + 1], nIndex, Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, cgUInt16 Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_int( mStatements[mCurrentStatement + 1], nIndex, Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, cgInt16 Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_int( mStatements[mCurrentStatement + 1], nIndex, Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, cgUInt Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_int( mStatements[mCurrentStatement + 1], nIndex, (int)Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, cgInt Value )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_int( mStatements[mCurrentStatement + 1], nIndex, Value ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}
bool cgWorldQuery::bindParameter( cgInt16 nIndex, const void * pBlob, cgUInt32 nSize )
{
    // Reset error status
    setErrorState( false );

    // We're binding to the NEXT executing statement.
    if ( mCurrentStatement >= mStatementCount - 1 )
        return false;

    // Bind the value.
    if ( sqlite3_bind_blob( mStatements[mCurrentStatement + 1], nIndex, pBlob, nSize, SQLITE_TRANSIENT ) != SQLITE_OK )
    {
        setErrorState( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onWorldDisposing () (Virtual)
/// <summary>
/// Called when the world is about to begin disposing data.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldQuery::onWorldDisposing( cgWorldEventArgs * e )
{
    unprepare();
}

//-----------------------------------------------------------------------------
//  Name : hasResults ()
/// <summary>
/// Determine if there were any column results available after the 
/// previously executed statement (after stepping the query). 
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::hasResults( ) const
{
    return mHasResults;
}

//-----------------------------------------------------------------------------
//  Name : getLastInsertId ()
/// <summary>
/// Get the last row identifier that was generated as a result of an insert 
/// operation.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgWorldQuery::getLastInsertId( ) const
{
    if ( mDatabase == NULL )
        return 0;
    return (cgInt32)sqlite3_last_insert_rowid(mDatabase);
}

//-----------------------------------------------------------------------------
//  Name : isPrepared ()
/// <summary>
/// Has the query been prepared (may become unprepared automatically if 
/// the parent world object to which this is attached gets disposed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::isPrepared( ) const
{
    return ( mDatabase != NULL );
}


//-----------------------------------------------------------------------------
//  Name : getWorld( )
/// <summary>
/// Retrieve the world on which this query is operating (if any).
/// </summary>
//-----------------------------------------------------------------------------
cgWorld * cgWorldQuery::getWorld( ) const
{
    return mWorld;
}

//-----------------------------------------------------------------------------
//  Name : getLastError( )
/// <summary>
/// Did an error occur during the last query method called?
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::getLastError( ) const
{
    return mErrorOccurred;
}

//-----------------------------------------------------------------------------
//  Name : getLastError( )
/// <summary>
/// Get the message of the last error that occurred if any.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldQuery::getLastError( cgString & strMessage ) const
{
    strMessage = mLastError;
    return mErrorOccurred;
}

//-----------------------------------------------------------------------------
//  Name : setErrorState( ) (Protected)
/// <summary>
/// Set the error status information available to the application
/// through the 'getLastError()' method.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldQuery::setErrorState( bool bState, bool bVerbose /* = false */ )
{
    // Is this a no-op?
    if ( bState == mErrorOccurred )
        return;

    // Update cached response.
    mErrorOccurred = bState;
    mLastError.clear();

    // Update error message if applicable.
    if ( bState == true )
    {
        if ( mDatabase == CG_NULL )
        {
            mLastError = _T("No database connection.");
        
        } // End if no database
        else
        {
            #if defined(UNICODE) || defined(_UNICODE)
                mLastError = (const cgWChar*)sqlite3_errmsg16(mDatabase);
            #else // UNICODE
                mLastError = (const cgChar*)sqlite3_errmsg(mDatabase);
            #endif // !UNICODE
        
        } // End if connected

        // Debug print error?
        if ( bVerbose == true )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("SQL Error: %s\n"), mLastError.c_str() );

    } // End if in error state
}

//-----------------------------------------------------------------------------
//  Name : registerMemoryVFS( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
sqlite3_vfs * cgWorldQuery::registerMemoryVFS( )
{
    // Define our VFS callback descriptor
    static sqlite3_vfs Desc =
    {
        1,                                      // iVersion
        sizeof( MemoryDBFile ),                 // szOsFile
        256,                                    // mxPathname
        CG_NULL,                                // pNext
        "CGEMEMDBVFS_v1.0",                     // zName
        CG_NULL,                                // pAppData
        cgWorldQuery::memoryDBVFSOpen,          // xOpen
        cgWorldQuery::memoryDBVFSDelete,        // xDelete
        cgWorldQuery::memoryDBVFSAccess,        // xAccess
        cgWorldQuery::memoryDBVFSFullPathname,  // xFullPathname
        cgWorldQuery::memoryDBVFSDlOpen,        // xDlOpen
        cgWorldQuery::memoryDBVFSDlError,       // xDlError
        cgWorldQuery::memoryDBVFSDlSym,         // xDlSym
        cgWorldQuery::memoryDBVFSDlClose,       // xDlClose
        cgWorldQuery::memoryDBVFSRandomness,    // xRandomness
        cgWorldQuery::memoryDBVFSSleep,         // xSleep
        cgWorldQuery::memoryDBVFSCurrentTime,   // xCurrentTime
        CG_NULL                                 // xGetLastError
    };

    // Check to see if have previously yet registered our in-memory database VFS
    if ( sqlite3_vfs_find( Desc.zName ) == CG_NULL )
    {
        // Not previously registered, so register it now.
        sqlite3_vfs_register( &Desc, 0 );

    } // End if not registered

    // Return the description structure
    return &Desc;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSOpen( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBVFSOpen( sqlite3_vfs * pVFS, const cgChar * zName, sqlite3_file * pFile, cgInt flags, cgInt *pOutFlags )
{
    // Set up the file structure.
    MemoryDBFile * pMemFile = (MemoryDBFile*)pFile;
    pMemFile->Flags         = flags;
    pMemFile->pStream       = CG_NULL;
    pMemFile->pBuffer       = CG_NULL;
    pMemFile->BufferSize    = 0;

    // Select correct callback methods.
    static const sqlite3_io_methods IODesc =
    {
        1,                                              // iVersion
        cgWorldQuery::memoryDBFileClose,                // xClose
        cgWorldQuery::memoryDBFileRead,                 // xRead
        cgWorldQuery::memoryDBFileWrite,                // xWrite
        cgWorldQuery::memoryDBFileTruncate,             // xTruncate
        cgWorldQuery::memoryDBFileSync,                 // xSync
        cgWorldQuery::memoryDBFileSize,                 // xFileSize
        cgWorldQuery::memoryDBFileLock,                 // xLock
        cgWorldQuery::memoryDBFileUnlock,               // xUnlock
        cgWorldQuery::memoryDBFileCheckReservedLock,    // xCheckReservedLock
        cgWorldQuery::memoryDBFileControl,              // xFileControl
        cgWorldQuery::memoryDBFileSectorSize,           // xSectorSize
        cgWorldQuery::memoryDBFileDeviceCharacteristics // xDeviceCharacteristics
    };
    pMemFile->VFSFile.pMethods = &IODesc;

    // Open the file if this is the main database and not a journal.
    if ( (flags & SQLITE_OPEN_MAIN_DB) != 0 )
    {
        // sqlite3 allocates and deallocates the file structure using malloc / free.
        // As a result, member classes in the structure will not be constructed /
        // destructed appropriately. Create a heap allocated stream object instead.
        pMemFile->pStream = new cgInputStream( *(cgInputStream*)(pVFS->pAppData) );
        pMemFile->pBuffer = pMemFile->pStream->getBuffer( pMemFile->BufferSize );
    
    } // End if main DB
    
    // Success!
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSDelete( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBVFSDelete( sqlite3_vfs*, const cgChar * zName, cgInt syncDir )
{
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSAccess( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBVFSAccess( sqlite3_vfs*, const cgChar * zName, cgInt flags, cgInt *pResOut)
{
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSFullPathname( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBVFSFullPathname( sqlite3_vfs*, const cgChar * zName, cgInt nOut, cgChar *zOut )
{
    strncpy( zOut, zName, nOut );
	zOut[ nOut - 1 ] = '\0';
	return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSDlOpen( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void * cgWorldQuery::memoryDBVFSDlOpen( sqlite3_vfs*, const cgChar * zFilename )
{
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSDlError( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldQuery::memoryDBVFSDlError( sqlite3_vfs*, cgInt nByte, cgChar * zErrMsg )
{
    // Nothing in this implementation
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSDlSym( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void (* cgWorldQuery::memoryDBVFSDlSym( sqlite3_vfs*, void *, const cgChar * zSymbol ))(void)
{
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSDlClose( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldQuery::memoryDBVFSDlClose( sqlite3_vfs*, void * )
{
    // Nothing in this implementation
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSRandomness( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBVFSRandomness( sqlite3_vfs*, cgInt nByte, cgChar * zOut )
{
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSSleep( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBVFSSleep( sqlite3_vfs*, cgInt microseconds )
{
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBVFSCurrentTime( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBVFSCurrentTime( sqlite3_vfs*, cgDouble * )
{
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileClose( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileClose( sqlite3_file * pFile )
{
    MemoryDBFile * pMemFile = (MemoryDBFile*)pFile;

    // Is this the main database file, or perhaps a journal?
    if ( (pMemFile->Flags & SQLITE_OPEN_MAIN_DB) != 0 )
    {
        delete pMemFile->pStream;
        pMemFile->pBuffer = CG_NULL;
        pMemFile->BufferSize = 0;
    
    } // End if opened
    
    // Success!
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileRead( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileRead( sqlite3_file * pFile, void * pBuffer, cgInt iAmt, cgInt64 iOfst )
{
    MemoryDBFile * pMemFile = (MemoryDBFile*)pFile;

    // Check for overflow
    if ( (iOfst + iAmt) > pMemFile->BufferSize )
        return SQLITE_IOERR_SHORT_READ;
    
    // Copy data out
    memcpy( pBuffer, pMemFile->pBuffer + iOfst, iAmt );

    // Success!
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileWrite( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileWrite( sqlite3_file * pFile, const void * pBuffer, cgInt iAmt, cgInt64 iOfst )
{
    // Writing is not supported.
    return SQLITE_IOERR_ACCESS;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileTruncate( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileTruncate( sqlite3_file * pFile, cgInt64 size )
{
    // Writing is not supported
    return SQLITE_IOERR_ACCESS;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileSync( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileSync( sqlite3_file*, cgInt flags )
{
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileSize( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileSize( sqlite3_file * pFile, cgInt64 *pSize )
{
    MemoryDBFile * pMemFile = (MemoryDBFile*)pFile;
    *pSize = (cgInt64)pMemFile->BufferSize;
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileLock( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileLock( sqlite3_file*, cgInt )
{
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileUnlock( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileUnlock( sqlite3_file*, cgInt )
{
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileCheckReservedLock( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileCheckReservedLock( sqlite3_file*, cgInt *pResOut )
{
    *pResOut = 0;
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileControl( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileControl( sqlite3_file* file, cgInt op, void *pArg )
{
    switch ( op )
    {
        case SQLITE_FCNTL_LOCKSTATE:
            // Not required unless in test mode (which we are not)
            break;

        case SQLITE_GET_LOCKPROXYFILE:
            // Unsupported
            break;

        case SQLITE_SET_LOCKPROXYFILE:
            // Unsupported
            break;

        case SQLITE_LAST_ERRNO:
            // Unsupported
            break;

        case SQLITE_FCNTL_SIZE_HINT:
            // VFS does not support writing, so size hint is not required.
            break;

        case SQLITE_FCNTL_CHUNK_SIZE:
            // VFS does not support writing, so chunk size alteration is not required.
            break;

        case SQLITE_FCNTL_FILE_POINTER:
            *((sqlite3_file**)pArg) = file;
            break;

        case SQLITE_FCNTL_SYNC_OMITTED:
            // Used to inform us about PRAGMA Synchronous=OFF scenario.
            break;

        case SQLITE_FCNTL_WIN32_AV_RETRY:
            // Configure automatic retry counts for file IO.
            break;

        case SQLITE_FCNTL_PERSIST_WAL:
            // VFS does not support writing, so configuring write ahead log is not required.
            break;

        case SQLITE_FCNTL_OVERWRITE:
            // VFS does not support writing, so signaling a database overwrite is not required.
            break;
        
        case SQLITE_FCNTL_VFSNAME:
            // Unsupported
            break;

        case SQLITE_FCNTL_POWERSAFE_OVERWRITE:
            // VFS does not support writing, so configuring power safe overwriting is not required.
            break;

        case SQLITE_FCNTL_PRAGMA:
            // Let sqlite handle this pragma with SQLITE_NOTFOUND,
            // or handle it ourselves and return SQLITE_OK;
            //return SQLITE_NOTFOUND;
            break;

    } // End switch op

    // Default behavior
    return SQLITE_OK;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileSectorSize( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileSectorSize( sqlite3_file* )
{
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : memoryDBFileDeviceCharacteristics( ) (Protected, Static)
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgWorldQuery::memoryDBFileDeviceCharacteristics( sqlite3_file* )
{
    return 0;
}