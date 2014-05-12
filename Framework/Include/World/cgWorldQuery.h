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
// File : cgWorldQuery.h                                                     //
//                                                                           //
// Desc : Provides support for running queries against the main world        //
//        database. This includes the selection of existing data from the    //
//        database as well as inserting and updating data.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWORLDQUERY_H_ )
#define _CGE_CGWORLDQUERY_H_

//-----------------------------------------------------------------------------
// cgWorldQuery Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorld.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct sqlite3;
struct sqlite3_stmt;
struct sqlite3_vfs;
struct sqlite3_file;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgWorldQuery (Class)
// Provides support for running queries against the main world database. This 
// includes the selection of existing data from the database as well as 
// inserting and updating data.
//-----------------------------------------------------------------------------
class CGE_API cgWorldQuery : public cgWorldEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgWorldQuery, cgEventListener, "WorldQuery" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWorldQuery( );
             cgWorldQuery( cgWorld * world, const cgString & statements );
             cgWorldQuery( sqlite3 * database, const cgString & statements );
    virtual ~cgWorldQuery( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static sqlite3_vfs* registerMemoryVFS   ( );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool            prepare         ( cgWorld * world, const cgString & statements, bool verbose = false );
    bool            prepare         ( sqlite3 * database, const cgString & statements, bool verbose = false );
    void            unprepare       ( );
    bool            step            ( );
    bool            step            ( bool autoReset );
    void            stepAll         ( );
	bool            reset           ( );
    bool            reset           ( bool resetErrorState );
    bool            resetCurrent    ( );
    bool            nextRow         ( );

    // Retrieve column data.
    bool            getColumn       ( const cgString & index, bool & value );
    bool            getColumn       ( cgInt16 column, bool & value );
    bool            getColumn       ( const cgString & index, cgUInt & value );
    bool            getColumn       ( cgInt16 column, cgUInt & value );
    bool            getColumn       ( const cgString & index, cgInt & value );
    bool            getColumn       ( cgInt16 column, cgInt & value );
    bool            getColumn       ( const cgString & index, cgUInt32 & value );
    bool            getColumn       ( cgInt16 column, cgUInt32 & value );
    bool            getColumn       ( const cgString & index, cgInt32 & value );
    bool            getColumn       ( cgInt16 column, cgInt32 & value );
    bool            getColumn       ( const cgString & index, cgInt16 & value );
    bool            getColumn       ( cgInt16 column, cgInt16 & value );
    bool            getColumn       ( const cgString & index, cgUInt16 & value );
    bool            getColumn       ( cgInt16 column, cgUInt16 & value );
    bool            getColumn       ( const cgString & index, cgFloat & value );
    bool            getColumn       ( cgInt16 column, cgFloat & value );
    bool            getColumn       ( const cgString & index, cgDouble & value );
    bool            getColumn       ( cgInt16 column, cgDouble & value );
    bool            getColumn       ( const cgString & index, cgString & value );
    bool            getColumn       ( cgInt16 column, cgString & value );
    bool            getColumn       ( const cgString & index, void ** blobOut, cgUInt32 & sizeOut );
    bool            getColumn       ( cgInt16 column, void ** blobOut, cgUInt32 & sizeOut );
    
    // Bind parameters
    bool            bindParameter   ( cgInt16 index, const cgString & value );
    bool            bindParameter   ( cgInt16 index, bool value );
    bool            bindParameter   ( cgInt16 index, cgFloat value );
    bool            bindParameter   ( cgInt16 index, cgDouble value );
    bool            bindParameter   ( cgInt16 index, cgUInt value );
    bool            bindParameter   ( cgInt16 index, cgInt value );
    bool            bindParameter   ( cgInt16 index, cgUInt32 value );
    bool            bindParameter   ( cgInt16 index, cgInt32 value );
    bool            bindParameter   ( cgInt16 index, cgUInt16 value );
    bool            bindParameter   ( cgInt16 index, cgInt16 value );
    bool            bindParameter   ( cgInt16 index, const void * blob, cgUInt32 size );
    
    // Information
    bool            hasResults      ( ) const;
    cgInt32         getLastInsertId ( ) const;
    bool            getLastError    ( ) const;
    bool            getLastError    ( cgString & messageOut ) const;
    bool            isPrepared      ( ) const;
    cgWorld       * getWorld        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldEventListener)
    //-------------------------------------------------------------------------
    virtual void    onWorldDisposing( cgWorldEventArgs * e );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgString, cgInt16, ResultColumnMap)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void        setErrorState   ( bool state, bool verbose = false );

    //-------------------------------------------------------------------------
    // Protected Static Functions
    //-------------------------------------------------------------------------
    // VFS integration for sqlite in-memory database processing.
    static cgInt            memoryDBVFSOpen                     ( sqlite3_vfs*, const cgChar * zName, sqlite3_file*, cgInt flags, cgInt *pOutFlags );
    static cgInt            memoryDBVFSDelete                   ( sqlite3_vfs*, const cgChar * zName, cgInt syncDir );
    static cgInt            memoryDBVFSAccess                   ( sqlite3_vfs*, const cgChar * zName, cgInt flags, cgInt *pResOut);
    static cgInt            memoryDBVFSFullPathname             ( sqlite3_vfs*, const cgChar * zName, cgInt nOut, cgChar *zOut );
    static void           * memoryDBVFSDlOpen                   ( sqlite3_vfs*, const cgChar * zFilename );
    static void             memoryDBVFSDlError                  ( sqlite3_vfs*, cgInt nByte, cgChar * zErrMsg );
    static void          (* memoryDBVFSDlSym                    ( sqlite3_vfs*, void *, const cgChar * zSymbol ))(void);
    static void             memoryDBVFSDlClose                  ( sqlite3_vfs*, void * );
    static cgInt            memoryDBVFSRandomness               ( sqlite3_vfs*, cgInt nByte, cgChar * zOut );
    static cgInt            memoryDBVFSSleep                    ( sqlite3_vfs*, cgInt microseconds );
    static cgInt            memoryDBVFSCurrentTime              ( sqlite3_vfs*, cgDouble * );

    // FILE integration for sqlite in-memory database processing.
    static cgInt            memoryDBFileClose                   ( sqlite3_file* );
    static cgInt            memoryDBFileRead                    ( sqlite3_file*, void*, cgInt iAmt, cgInt64 iOfst );
    static cgInt            memoryDBFileWrite                   ( sqlite3_file*, const void*, cgInt iAmt, cgInt64 iOfst );
    static cgInt            memoryDBFileTruncate                ( sqlite3_file*, cgInt64 size );
    static cgInt            memoryDBFileSync                    ( sqlite3_file*, cgInt flags );
    static cgInt            memoryDBFileSize                    ( sqlite3_file*, cgInt64 *pSize );
    static cgInt            memoryDBFileLock                    ( sqlite3_file*, cgInt );
    static cgInt            memoryDBFileUnlock                  ( sqlite3_file*, cgInt );
    static cgInt            memoryDBFileCheckReservedLock       ( sqlite3_file*, cgInt *pResOut );
    static cgInt            memoryDBFileControl                 ( sqlite3_file*, cgInt op, void *pArg );
    static cgInt            memoryDBFileSectorSize              ( sqlite3_file* );
    static cgInt            memoryDBFileDeviceCharacteristics   ( sqlite3_file* );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgWorld           * mWorld;             // The world on which the query is being run.
    sqlite3           * mDatabase;          // Database connection object.
    sqlite3_stmt     ** mStatements;        // List of prepared statements for this query.
    cgInt16             mStatementCount;    // Number of statements in this query.
    cgInt16             mCurrentStatement;  // This describes the statement due for execution.
    bool                mHasResults;        // Any results available to return?
    cgString            mLastError;         // Cached copy of the last error in the case it was unprepared.
    bool                mFirstRowCached;    // Has the first row of the result set been cached during the first call to 'step()'?
    bool                mErrorOccurred;     // Did an error occur since the getLastError() method was last called?
    ResultColumnMap     mResultColumns;     // Map of result set column names and their corresponding column index.
};

#endif // !_CGE_CGWORLDQUERY_H_