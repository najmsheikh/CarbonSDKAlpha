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
// Name : cgWorld.cpp                                                        //
//                                                                           //
// Desc : Provides classes responsible for loading and managing the          //
//        components necessary for the application to update, render and     //
//        interact with the world.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWorld Module Includes
//-----------------------------------------------------------------------------
#include <World/cgWorld.h>
#include <World/cgWorldConfiguration.h>
#include <World/cgWorldObject.h>
#include <World/cgObjectSubElement.h>
#include <World/cgScene.h>
#include <World/cgSceneElement.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgResourceManager.h>
#include <System/cgMessageTypes.h>
#include <System/cgStringUtility.h>
#include <System/cgExceptions.h>
#include <Math/cgMathUtility.h>
#include <SQLite/sqlite3.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgWorld * cgWorld::mSingleton = CG_NULL;

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgExceptions;

///////////////////////////////////////////////////////////////////////////////
// cgWorld Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWorld () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgWorld::cgWorld( ) : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mDatabase           = CG_NULL;
    mStatementBegin     = CG_NULL;
    mStatementCommit    = CG_NULL;
    mStatementRollback  = CG_NULL;
    mConfiguration      = CG_NULL;
    mStreamIsTemporary  = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgWorld () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgWorld::~cgWorld()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgWorld::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Notify anyone interested that the world is disposing.
    onWorldDisposing( &cgWorldEventArgs( this ) );

    // Ensure that the resource manager unloads all resources
    // that relate to this world -- it will shortly no longer exist.
    cgToDo( "Effect Overhaul", "All world components (materials, meshes, etc.) must be released!" );
    /*cgResourceManager * resources = cgResourceManager::getInstance();
    if ( resources )
        resources->ReleaseWorldComponents( this );*/

    // First destroy any active scenes
    for ( size_t i = 0; i < mActiveScenes.size(); ++i )
    {
        // Delete the scene object
        if ( mActiveScenes[i] )
            mActiveScenes[i]->scriptSafeDispose( );

    } // Next scene

    // Dispose of world configuration information.
    delete mConfiguration;

    // Finalize any cached database statements that remain active.
    if ( mStatementBegin )
        sqlite3_finalize( mStatementBegin );
    if ( mStatementCommit )
        sqlite3_finalize( mStatementCommit );
    if ( mStatementRollback )
        sqlite3_finalize( mStatementRollback );

    // Close any database connection that remains open.
    if ( mDatabase )
    {
        if ( sqlite3_close( mDatabase ) == SQLITE_BUSY )
        {
            cgAppLog::write( cgAppLog::Warning | cgAppLog::Debug, _T("Failed to close database because it was marked as busy. Enumerating statements that have not yet been finalized:\n") );

            // List all currently non-finalized statements.
            STRING_CONVERT;
            sqlite3_stmt * testStatement = NULL;
            while ( (testStatement = sqlite3_next_stmt( mDatabase, testStatement )) != CG_NULL )
                cgAppLog::write( cgAppLog::Debug, _T("%s\n"), stringConvertA2CT(sqlite3_sql( testStatement )) );

        } // End if busy
    
    } // End if open

    // Remove our local reference to the database stream.
    cgString databaseFile = mDatabaseStream.getSourceFile().c_str();
    mDatabaseStream.reset();
    mOriginalStream.reset();

    // Remove the temporary world file if necessary.
    if ( mStreamIsTemporary )
        cgFileSystem::deleteFile( databaseFile );

    // Clear variables
    mConfiguration      = CG_NULL;
    mDatabase           = CG_NULL;
    mStatementBegin     = CG_NULL;
    mStatementCommit    = CG_NULL;
    mStatementRollback  = CG_NULL;
    mStreamIsTemporary  = false;
    mActiveScenes.clear();
    mActiveSceneIdMap.clear();
    
    // Dispose of base class(es) if requested.
    if ( disposeBase )
        cgReference::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgWorld * cgWorld::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createSingleton () (Static)
/// <summary>
/// Creates the singleton. You would usually allocate the singleton in
/// the static member definition, however sometimes it's necessary to
/// call for allocation to allow for correct allocation ordering
/// and destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
    {
        // Initialize SQLite subsystems.
        sqlite3_initialize();

        // Create singleton
        mSingleton = new cgWorld();

    } // End if no singleton
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::destroySingleton( )
{
    // Destroy (unless script still referencing)!
    if ( mSingleton != CG_NULL )
    {
        mSingleton->scriptSafeDispose( );

        // Shut down SQLite subsystems.
        sqlite3_shutdown();
    
    } // End if allocated
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_World )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
// Name : executeQuery ()
/// <summary>
/// Execute a query on the database by passing the statement(s) directly in
/// their string form. If requested, the statements can be wrapped in a
/// safe transaction which will automatically rollback in the event of a
/// failure.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::executeQuery( const cgString & statements, bool asOneTransaction )
{
    // If requested, the statement will be automatically wrapped in transaction begin/commit.
    if ( asOneTransaction == true )
        beginTransaction();
    
    // Attempt to execute each component of the statement.
    const cgTChar * queryString = statements.c_str();
    while ( queryString != CG_NULL )
    {
        sqlite3_stmt * statement = CG_NULL;

        // Prepare / compile the next piece of the statement (up to the next semi-colon).
        #if defined(_UNICODE)
        cgInt prepareResult = sqlite3_prepare16_v2( mDatabase, queryString, wcslen(queryString) * sizeof(cgWChar), &statement, (const void**)&queryString );
        #else // _UNICODE
        cgInt prepareResult = sqlite3_prepare_v2( mDatabase, queryString, strlen(queryString) * sizeof(cgChar), &statement, (const void**)&queryString );
        #endif // !_UNICODE
        
        // Bail out of nothing was available to prepare (not an error, we're just done).
        if ( prepareResult == SQLITE_OK && !statement )
            break;

        // If successful, execute the statement.
        cgInt stepResult = SQLITE_DONE;
        if ( prepareResult == SQLITE_OK )
            stepResult = sqlite3_step( statement );

        // Check for preparation or execution errors.
        if ( prepareResult != SQLITE_OK || (stepResult != SQLITE_DONE && stepResult != SQLITE_ROW) )
        {
            STRING_CONVERT;
            cgString error = _T("Failed to prepare or execute statement(s) '") + statements + _T("'. Error: ") + cgString(stringConvertA2CT(sqlite3_errmsg(mDatabase)));

            // Clean up any prepared statement.
            if ( statement )
                sqlite3_finalize( statement );

            // ROLLBACK!
            if ( asOneTransaction == true )
                rollbackTransaction();

            // List all currently non-reset statements.
            sqlite3_stmt * testStatement = NULL;
            while ( (testStatement = sqlite3_next_stmt( mDatabase, testStatement )) != NULL )
            {
                if ( sqlite3_stmt_is_reset( testStatement ) == 0 )
                    cgAppLog::write( cgAppLog::Debug, _T("%s has not been reset.\n"), stringConvertA2CT(sqlite3_sql( testStatement )) );
            
            } // Next active statement

            // Dump error information
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("%s\n"), error.c_str() );
            return false;
        
        } // End if failed

        // Clean up any prepared statement
        if ( statement )
            sqlite3_finalize( statement );

    } // Next section of the queryString.

    // And the final commit.
    if ( asOneTransaction == true )
        commitTransaction();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : beginTransaction ()
/// <summary>
/// Wrap the following queries into a single transaction.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::beginTransaction( )
{
    sqlite3_step( mStatementBegin );
    sqlite3_reset( mStatementBegin );
}

//-----------------------------------------------------------------------------
// Name : beginTransaction ()
/// <summary>
/// Wrap the following queries into a single SAVEPOINT transaction by name.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::beginTransaction( const cgString & savepoint )
{
    cgString queryString = _T("SAVEPOINT '") + cgStringUtility::addSlashes(savepoint) + _T("'");
    executeQuery( queryString, false );
}

//-----------------------------------------------------------------------------
// Name : commitTransaction ()
/// <summary>
/// Commit the prior transaction queries.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::commitTransaction( )
{
    sqlite3_step( mStatementCommit );
    sqlite3_reset( mStatementCommit );
}

//-----------------------------------------------------------------------------
// Name : commitTransaction ()
/// <summary>
/// Commit the prior transaction queries up to the specified SAVEPOINT.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::commitTransaction( const cgString & savepoint )
{
    cgString queryString = _T("RELEASE SAVEPOINT '") + cgStringUtility::addSlashes(savepoint) + _T("'");
    executeQuery( queryString, false );
}

//-----------------------------------------------------------------------------
// Name : rollbackTransaction ()
/// <summary>
/// Rollback the prior transaction queries without commiting.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::rollbackTransaction( )
{
    sqlite3_step( mStatementRollback );
    sqlite3_reset( mStatementRollback );
}

//-----------------------------------------------------------------------------
// Name : rollbackTransaction ()
/// <summary>
/// Rollback the prior transaction queries to the specified SAVEPOINT without 
/// committing.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::rollbackTransaction( const cgString & savepoint )
{
    // Pass through to main method.
    rollbackTransaction( savepoint, false );
}

//-----------------------------------------------------------------------------
// Name : rollbackTransaction ()
/// <summary>
/// Rollback the prior transaction queries to the specified SAVEPOINT without 
/// committing. Optionally the caller can choose to restart the transaction
/// by specifying 'true' the final parameter (default false).
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::rollbackTransaction( const cgString & savepoint, bool restartTransaction )
{
    cgString protectedName = cgStringUtility::addSlashes(savepoint);
    cgString queryString = _T("ROLLBACK TO SAVEPOINT '") + protectedName + _T("'");
    executeQuery( queryString, false );

    // If the user chose not to restart the savepoint transaction, release it.
    if ( restartTransaction == false )
    {
        queryString = _T("RELEASE SAVEPOINT '") + protectedName + _T("'");
        executeQuery( queryString, false );
    
    } // End if !restart
}

//-----------------------------------------------------------------------------
// Name : create () (Virtual)
/// <summary>
/// Create a new empty skeleton world file ready for modification.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::create( cgWorldType::Base type )
{
    // Cannot create a new world if it has already been initialized.
    if ( mDatabase )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to establish connection with a new world database. A prior connection to another database already exists.\n") );
        return false;
    
    } // End if already open

    // Write debug to log.
    cgAppLog::write( cgAppLog::Info, _T("Creating new world database utilizing the v%i.%02i.%04i layout definition.\n"), CGE_WORLD_VERSION, CGE_WORLD_SUBVERSION, CGE_WORLD_REVISION );

    // Retrieve the statements that define the required file layout.
    cgString layout;
    switch ( type )
    {
        case cgWorldType::Master:
            layout = cgFileSystem::loadStringFromStream( _T("sys://Layout/MasterWorld") );
            break;
        default:    
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unknown world file type requested.\n") );
            return false;
    
    } // End switch FileType

    // Valid?
    if ( layout.empty() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to determine file layout for requested world type.\n") );
        return false;
    
    } // End if not found
    
    // Attempt to create this new database.
    STRING_CONVERT;
    cgString temporaryFile = cgFileSystem::getTemporaryFile();
    if ( temporaryFile.empty() || sqlite3_open_v2( stringConvertT2CA(temporaryFile.c_str()), &mDatabase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, CG_NULL ) != SQLITE_OK || (mDatabase == CG_NULL) )
    {
        // Log and fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to create connection with new world database. Access was denied or an out of memory error occurred.\n") );
        return false;
    
    } // End if failed

    // Database was successfully opened. Create tables / indexes / triggers as required.
    // Uses a transaction to improve performance.
    if ( !executeQuery( layout, true ) )
    {
        dispose( false );
        return false;
    
    } // End if failed

    // Use the temporary file as the input stream.
    mOriginalStream     = cgInputStream( temporaryFile );
    mDatabaseStream     = cgInputStream( temporaryFile );
    mStreamIsTemporary  = true;

    // Execute any database post connection logic.
    if ( !postConnect( true ) )
    {
        dispose( false );
        return false;
    
    } // End if failed

    // Create a new world configuration object and instruct it
    // to populate the database with default configuration data.
    mConfiguration = new cgWorldConfiguration( this );
    if ( !mConfiguration->newConfiguration( type ) )
    {
        dispose( false );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : postConnect () (Protected)
/// <summary>
/// Performs all common actions necessary after a new database connection has
/// been established.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::postConnect( bool newWorld )
{
    // Select performance related options.
    executeQuery( _T("PRAGMA main.journal_mode=MEMORY"), false );   // Use in-memory journal
    executeQuery( _T("PRAGMA cache_size=2000"),false );         // 2000 pages (2mb, the default)
    executeQuery( _T("PRAGMA synchronous=OFF"), false );        // Don't wait for file access to complete.
    executeQuery( _T("PRAGMA count_changes=OFF"), false );      // Don't count database changes.
    executeQuery( _T("PRAGMA temp_store=2"), false );           // Temporary tables in memory.
    
    // In sandbox mode, we need to perform some adjustments on the data in the database.
    if ( !newWorld && cgGetSandboxMode() != cgSandboxMode::Disabled )
    {
        cgWorldQuery query;
        cgString fileName = mOriginalStream.getSourceFile();

        // Catch exceptions
        try
        {
            // Before allowing a final export, perform a full scan of the database for all
            // 'path' entries in all tables in the database. These will be converted to
            // relative paths if necessary. In case of failure, run it within a transaction.
            query.prepare( mDatabase, _T("BEGIN") );
            query.step( true );

            // Files will all be relative to original database file.
            cgString databaseDirectory = cgFileSystem::getDirectoryName( fileName );

            // Retrieve a list of all tables in the database.
            cgStringArray tables;
            if ( query.prepare( mDatabase, _T("SELECT name FROM 'SQLITE_MASTER' WHERE type='table'") ) == false || query.step() == false )
            {
                cgString error;
                query.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to select table names from world database. Error: %s"), error.c_str()), cgDebugSource() );
            
            } // End if failed
            while ( query.nextRow() )
            {
                cgString tableName;
                query.getColumn( 0, tableName );
                tables.push_back( tableName );

            } // Next Row

            // For each table, retrieve column information and collect a list
            // of all table.column combinations that have 'path' as their type.
            struct ColumnData
            {
                cgUInt32 table;
                cgString name;
            };
            std::vector<ColumnData> pathColumns;
            for ( size_t i = 0; i < tables.size(); ++i )
            {
                // Retrieve a list of all path columns.
                cgString tableName = tables[i];
                if ( query.prepare( mDatabase, _T("PRAGMA table_info('") + tableName + _T("')") ) == false || query.step() == false )
                {
                    cgString error;
                    query.getLastError( error );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to select column information for world database table %s. Error: %s"), tableName.c_str(), error.c_str()), cgDebugSource() );
                
                } // End if failed
                while ( query.nextRow() )
                {
                    cgString columnType;
                    query.getColumn( _T("type"), columnType );
                    
                    // type is set to 'path'?
                    if ( columnType.compare( _T("path"), true ) == 0 )
                    {
                        // Record column information.
                        ColumnData data;
                        data.table = i;
                        query.getColumn( _T("name"), data.name );
                        pathColumns.push_back( data );
                        
                    } // End if type = 'path'

                } // Next Column

            } // Next table

            // Process all path columns to ensure they contain absolute paths.
            for ( size_t i = 0; i < pathColumns.size(); ++i )
            {
                const ColumnData & data = pathColumns[i];
                const cgTChar * c = data.name.c_str(), * t = tables[data.table].c_str();

                // Build query that allows us to update specified row data.
                cgWorldQuery updateQuery;
                cgString queryString = cgString::format( _T("UPDATE '%s' SET %s=?1 WHERE rowid=?2"), t, c );
                if ( updateQuery.prepare( mDatabase, queryString ) == false )
                {
                    cgString error;
                    updateQuery.getLastError( error );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to prepare row update queryString while generating world database absolute paths. Error: %s"), error.c_str()), cgDebugSource() );
                
                } // End if failed

                // Select all rows where the path column does not contain an absolute style
                // path in the format '<single-letter>:{other-characters}' i.e. "c:\mydata"
                // or a file system path protocol i.e. "sys://"
                cgToDo( "Carbon General", "Rather than using a wildcard for the path protocols, perhaps iterate them (cgFileSystem) and provide them explicitly?" )
                queryString = cgString::format( _T("SELECT rowid,%s FROM '%s' WHERE %s NOT LIKE '_:%%' AND %s NOT LIKE '%%://%%'"), c, t, c, c );
                if ( query.prepare( mDatabase, queryString ) == false || query.step() == false )
                {
                    cgString error;
                    query.getLastError( error );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to select row data while generating world database absolute paths. Error: %s"), error.c_str()), cgDebugSource() );
                
                } // End if failed
                while ( query.nextRow() )
                {
                    cgString currentPath;
                    cgUInt32 rowId;
                    query.getColumn( 0, rowId );
                    query.getColumn( 1, currentPath );

                    // Skip if the path is completely empty.
                    if ( currentPath.empty() )
                        continue;

                    // Correct path.
                    currentPath = cgFileSystem::getAbsolutePath( currentPath, databaseDirectory );

                    // Update row
                    updateQuery.bindParameter( 1, currentPath );
                    updateQuery.bindParameter( 2, rowId );
                    if ( updateQuery.step( true ) == false )
                    {
                        cgString error;
                        updateQuery.getLastError( error );
                        throw cgExceptions::ResultException( cgString::format(_T("Failed to update row data while generating world database absolute paths. Error: %s"), error.c_str()), cgDebugSource() );
                    
                    } // End if failed

                } // Next Column

            } // Next path column

            // Commit changes
            query.prepare( mDatabase, _T("COMMIT") );
            query.step( true );

        } // End try

        catch( const cgExceptions::ResultException & e )
        {
            query.prepare( mDatabase, _T("ROLLBACK") );
            query.step( true );
            cgAppLog::write( cgAppLog::Error, _T("Failed to prepare database for import from file '%s'. %s.\n"), fileName.c_str(), e.toString().c_str() );
            return false;

        } // End catch

    } // End if sandbox

    // Prepare the standard 'BEGIN' transaction queryString we'll be using often.
    sqlite3_stmt * statement = CG_NULL;
    const cgChar * queryString = "BEGIN";
    sqlite3_prepare_v2( mDatabase, queryString, strlen(queryString), &mStatementBegin, NULL );
    if ( !mStatementBegin )
    {
        STRING_CONVERT;
        cgAppLog::write( cgAppLog::Error, _T("Failed to compile common transaction begin queryString. Error: %s\n"), stringConvertA2CT(sqlite3_errmsg(mDatabase)) );
        return false;
    
    } // End if failed

    // Prepare the standard 'COMMIT' transaction queryString we'll be using often.
    queryString = "COMMIT";
    sqlite3_prepare_v2( mDatabase, queryString, strlen(queryString), &mStatementCommit, NULL );
    if ( !mStatementCommit )
    {
        STRING_CONVERT;
        cgAppLog::write( cgAppLog::Error, _T("Failed to compile common transaction commit queryString. Error: %s\n"), stringConvertA2CT(sqlite3_errmsg(mDatabase)) );
        return false;
    
    } // End if failed
    
    // Prepare the standard 'ROLLBACK' transaction queryString we'll be using often.
    queryString = "ROLLBACK";
    sqlite3_prepare_v2( mDatabase, queryString, strlen(queryString), &mStatementRollback, NULL );
    if ( !mStatementRollback )
    {
        STRING_CONVERT;
        cgAppLog::write( cgAppLog::Error, _T("Failed to compile common transaction rollback queryString. Error: %s\n"), stringConvertA2CT(sqlite3_errmsg(mDatabase)) );
        return false;
    
    } // End if failed
       
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : open ()
/// <summary>
/// Establish a connection with the specified world database and import 
/// any necessary initial data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::open( const cgInputStream & stream )
{
    sqlite3 * databaseIn = CG_NULL;
    cgInt     result;

    // Already open?
    if ( mDatabase )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to establish connection with world database '%s'. A prior connection to another database already exists.\n"), stream.getName().c_str() );
        return false;
    
    } // End if already open

    // In sandbox mode, duplicate the database to a temporary file
    // before we open it.
    cgToDo( "Carbon General", "In sandbox mode, if the stream is a mapped file or memory stream, allow opening but add read-only support to the editor." )
    if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
    {
        // In sandbox mode, database must be a file.
        if ( stream.getType() != cgStreamType::File )
        {
            cgAppLog::write( cgAppLog::Error, _T("Attempted to open a non-file based world database in sandbox mode. This behavior is not supported.\n") );
            return false;
        
        } // End if already open

        // Generate a new temporary copy of the source file.
        cgString temporaryFile = cgFileSystem::getTemporaryFile();
        if ( temporaryFile.empty() || !cgFileSystem::copyFile( stream.getSourceFile(), temporaryFile, true ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to create temporary files when establishing a connection with world database '%s'. The disk may be full.\n"), stream.getName().c_str() );
            return false;
        
        } // End if failed

        // Use the temporary file as the new input stream.
        mOriginalStream     = stream;
        mDatabaseStream     = cgInputStream( temporaryFile );
        mStreamIsTemporary  = true;
        
    } // End if sandbox mode
    else
    {
        // Store required details.
        mOriginalStream = stream;
        mDatabaseStream = stream;
    
    } // End if !sandbox

    cgToDo( "Carbon General", "If file does not exist, this steps into the VFS and apparently still succeeds even if the file is not found." )
    
    // Where are we loading from?
    if ( mDatabaseStream.getType() == cgStreamType::File )
    {
        STRING_CONVERT;

        // Loading from file, just pass the name of the file in bypassing the VFS.
        const cgChar * databaseName = stringConvertT2CA( mDatabaseStream.getSourceFile().c_str() );
        if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
            result = sqlite3_open_v2( databaseName, &databaseIn, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, CG_NULL );
        else
            result = sqlite3_open_v2( databaseName, &databaseIn, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, CG_NULL );
    
    } // End if file
    else
    {
        STRING_CONVERT;

        // Register the memory VFS (virtual file system) handler if necessary.
        sqlite3_vfs * fileSystem = cgWorldQuery::registerMemoryVFS( );

        // Pass the pointer to our stream through to the VFS via the application
        // data field in the VFS. Since there is only one such structure, this
        // pointer should be considered valid only during the subsequent open call.
        fileSystem->pAppData = &mDatabaseStream;

        // Open the database using our custom VFS.
        const cgChar * databaseName = stringConvertT2CA( mDatabaseStream.getName().c_str() );
        result = sqlite3_open_v2( databaseName, &databaseIn, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, fileSystem->zName );
        
    } // End if memory / memory mapped

    // Failed to open connection?
    if ( !databaseIn || result != SQLITE_OK )
    {
        // Log and fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to establish connection with world database '%s'. Access was denied or an out of memory error occurred.\n"), stream.getName().c_str() );
        if ( databaseIn )
            sqlite3_close( databaseIn );
        return false;
    
    } // End if failed

    // Database opened successfully. Store the database connection pointer.
    // We either need to use the 'temporary' sandbox database, or the source
    // file database depending on which mode the application is in.
    mDatabase = databaseIn;

    // Success?
    if ( !postConnect(false) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute post-connection processes on world database '%s'. See previous errors for more information.\n"), stream.getName().c_str() );
        dispose( false );
        return false;

    } // End if failed

    // Create a new world configuration object and instruct it load the configuration data 
    // from the database. In sandbox mode, we instruct the configuration object to automatically
    // upgrade the database layout if it does not match the maximum required version.
    mConfiguration = new cgWorldConfiguration( this );
    cgUInt32 requiredVersionMinimum = cgMakeVersion( CGE_WORLD_MIN_VERSION, CGE_WORLD_MIN_SUBVERSION, CGE_WORLD_MIN_REVISION );
    cgUInt32 requiredVersionMaximum = cgMakeVersion( CGE_WORLD_VERSION, CGE_WORLD_SUBVERSION, CGE_WORLD_REVISION );
    cgConfigResult::Base configResult = mConfiguration->loadConfiguration( requiredVersionMinimum, requiredVersionMaximum, (cgGetSandboxMode() == cgSandboxMode::Enabled) );
    if ( configResult == cgConfigResult::Mismatch )
    {
        cgUInt32 actualVersion, actualSubversion, actualRevision;
        cgUInt32 minimumVersion, minimumSubversion, minimumRevision;
        cgUInt32 maximumVersion, maximumSubversion, maximumRevision;
        cgDecomposeVersion( mConfiguration->getVersion(), actualVersion, actualSubversion, actualRevision );
        cgDecomposeVersion( requiredVersionMinimum, minimumVersion, minimumSubversion, minimumRevision );
        cgDecomposeVersion( requiredVersionMaximum, maximumVersion, maximumSubversion, maximumRevision );
        cgString errorNote;
        if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
            errorNote = _T("conversion is required. It may however be possible for the sandbox editing environment to convert the database to a layout that is compatible with this application");
        else
            errorNote = _T("or the conversion process failed");
        cgAppLog::write( cgAppLog::Error, _T("Unable to establish connection with world database in stream '%s'. The database layout version (v%i.%02i.%04i) did not fall within the range supported by this application (v%i.%02i.%04i through v%i.%02i.%04i) or %s. Check with your vendor for more information.\n"),
                         stream.getName().c_str(), actualVersion, actualSubversion, actualRevision, minimumVersion, minimumSubversion, minimumRevision, maximumVersion, maximumSubversion, maximumRevision, errorNote.c_str() );
        dispose( false );
        return false;
    
    } // End if failed
    else if ( configResult == cgConfigResult::Error )
    {
        cgAppLog::write( cgAppLog::Error, _T("Stream '%s' was not recognized as being a valid world database. See previous errors for more information.\n"), stream.getName().c_str() );
        dispose( false );
        return false;
    
    } // End if failed

    // Write debug to log.
    cgUInt32 actualVersion, actualSubversion, actualRevision;
    cgDecomposeVersion( mConfiguration->getVersion(), actualVersion, actualSubversion, actualRevision );
    cgAppLog::write( cgAppLog::Info, _T("Established connection with world database in stream '%s' (Layout: v%i.%02i.%04i).\n"), 
                     stream.getName().c_str(), actualVersion, actualSubversion, actualRevision );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : close ()
/// <summary>
/// Unload all active scenes and close our connection to the world database.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::close( )
{
    // Alias for standard disposal.
    dispose( false );
}

//-----------------------------------------------------------------------------
// Name : save ()
/// <summary>
/// Call in order to serialize the current world to a file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::save( const cgString & fileName )
{
    STRING_CONVERT;
    cgInt result = SQLITE_OK;
    sqlite3 * databaseOut = CG_NULL;

    // Requires FULL sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgWorld::save() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return false;
    
    } // End if !sandbox

    // Overwriting a file?
    bool overwriting = false;
    cgString workingFileName;
    cgString databaseDirectory = cgFileSystem::getDirectoryName( fileName );
    if ( cgFileSystem::fileExists( fileName ) )
    {
        // We are overwriting.
        overwriting = true;

        // Copy the working database file to a temporary file in the 
        // same directory as the destination fie.
        workingFileName = cgFileSystem::getTemporaryFile( databaseDirectory );
        
    } // End if overwriting
    else
    {
        // Just output to the referenced file.
        workingFileName = fileName;

    } // End if !overwriting

    // Copy database file to the specified location.
    if ( !cgFileSystem::copyFile(mDatabaseStream.getSourceFile(), workingFileName, true ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to write world database to the selected directory. The user may not have sufficient permissions. The save operation has been aborted before the destination file was overwritten.\n") );
        return false;
    
    } // End if failed
    
    // Perform a full scan of the database for all 'path' entries in all tables in the 
    // database. These will be converted to relative paths if necessary.
    if ( (result = sqlite3_open( stringConvertT2CA(workingFileName.c_str()), &databaseOut )) == SQLITE_OK )
    {
        // Catch exceptions
        try
        {
            cgWorldQuery query; // Make sure query goes out of scope before database is closed.

            // Retrieve a list of all tables in the database.
            cgStringArray tables;
            if ( query.prepare( databaseOut, _T("SELECT name FROM 'SQLITE_MASTER' WHERE type='table'") ) == false || query.step() == false )
            {
                cgString error;
                query.getLastError( error );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to select table names from world database. Error: %s"), error.c_str()), cgDebugSource() );
            
            } // End if failed
            while ( query.nextRow() )
            {
                cgString tableName;
                query.getColumn( 0, tableName );
                tables.push_back( tableName );

            } // Next Row

            // For each table, retrieve column information and collect a list
            // of all table.column combinations that have 'path' as their type.
            struct ColumnData
            {
                cgUInt32 table;
                cgString name;
            };
            std::vector<ColumnData> pathColumns;
            for ( size_t i = 0; i < tables.size(); ++i )
            {
                // Retrieve a list of all path columns.
                cgString tableName = tables[i];
                if ( query.prepare( databaseOut, _T("PRAGMA table_info('") + tableName + _T("')") ) == false || query.step() == false )
                {
                    cgString error;
                    query.getLastError( error );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to select column information for world database table %s. Error: %s"), tableName.c_str(), error.c_str()), cgDebugSource() );
                
                } // End if failed
                while ( query.nextRow() )
                {
                    cgString columnType;
                    query.getColumn( _T("type"), columnType );
                    
                    // type is set to 'path'?
                    if ( columnType.compare( _T("path"), true ) == 0 )
                    {
                        // Record column information.
                        ColumnData data;
                        data.table = i;
                        query.getColumn( _T("name"), data.name );
                        pathColumns.push_back( data );
                        
                    } // End if type = 'path'

                } // Next Column

            } // Next table

            // Process all path columns to ensure they contain relative paths.
            for ( size_t i = 0; i < pathColumns.size(); ++i )
            {
                const ColumnData & data = pathColumns[i];
                const cgTChar * c = data.name.c_str(), * t = tables[data.table].c_str();

                // Build query that allows us to update specified row data.
                cgWorldQuery updateQuery;
                cgString queryString = cgString::format( _T("UPDATE '%s' SET %s=?1 WHERE rowid=?2"), t, c );
                if ( updateQuery.prepare( databaseOut, queryString ) == false )
                {
                    cgString error;
                    updateQuery.getLastError( error );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to prepare row update queryString while generating world database relative paths. Error: %s"), error.c_str()), cgDebugSource() );
                
                } // End if failed

                // Select all rows where the path column contains an absolute style
                // path in the format '<single-letter>:{other-characters}' i.e. "c:\mydata"
                queryString = cgString::format( _T("SELECT rowid,%s FROM '%s' WHERE %s LIKE '_:%%'"), c, t, c );
                if ( query.prepare( databaseOut, queryString ) == false || query.step() == false )
                {
                    cgString error;
                    query.getLastError( error );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to select row data while generating world database relative paths. Error: %s"), error.c_str()), cgDebugSource() );
                
                } // End if failed
                while ( query.nextRow() )
                {
                    cgString currentPath;
                    cgUInt32 rowId;
                    query.getColumn( 0, rowId );
                    query.getColumn( 1, currentPath );

                    // Correct path.
                    currentPath = cgFileSystem::getRelativePath( currentPath, databaseDirectory );

                    // Update row
                    updateQuery.bindParameter( 1, currentPath );
                    updateQuery.bindParameter( 2, rowId );
                    if ( updateQuery.step( true ) == false )
                    {
                        cgString error;
                        updateQuery.getLastError( error );
                        throw cgExceptions::ResultException( cgString::format(_T("Failed to update row data while generating world database relative paths. Error: %s"), error.c_str()), cgDebugSource() );
                    
                    } // End if failed

                } // Next Column

            } // Next path column

            // Run a final vacuum on the exported database before saving.
            //query.prepare( databaseOut, _T("VACUUM") );
            //query.step( true );

        } // End try

        catch( const cgExceptions::ResultException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to finalize world database records during export to file '%s'. %s.\n"), fileName.c_str(), e.toString().c_str() );

            // Close the database
            sqlite3_close( databaseOut );

            // Delete the working file and then fail.
            cgFileSystem::deleteFile( workingFileName );
            return false;

        } // End catch

        // Close the output database.
        sqlite3_close( databaseOut );

    } // End if success
    else
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to access exported world database in order to finalize data. The user may not have sufficient permissions. The save operation has been aborted before any existing destination file was overwritten.\n") );

        // Close the output database (just in case).
        if ( databaseOut )
            sqlite3_close( databaseOut );

        // Delete the working file and then fail.
        cgFileSystem::deleteFile( workingFileName );
        return false;

    } // End if failed

    // If we are overwriting an existing file, move the temporary file now.
    if ( overwriting )
    {
        if ( !cgFileSystem::moveFile( workingFileName, fileName, true ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to overwrite existing file '%s' while exporting the world database. The user may not have sufficient permissions.\n"), fileName.c_str() );

            // Delete the working file and then fail.
            cgFileSystem::deleteFile( workingFileName );
            return false;

        } // End if move failed

    } // End if overwriting
    
    // Mark all loaded scenes as no longer being dirty.
    for ( size_t i = 0; i < mActiveScenes.size(); ++i )
        mActiveScenes[i]->setDirty( false );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : createScene ()
/// <summary>
/// Create a new scene and insert it into the database if required.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorld::createScene( const cgSceneDescriptor & description )
{
    cgToDo( "Carbon General", "The restriction that we must exist in sandbox mode should be removed. Add support for internal scenes." );

    // Scenes cannot be dynamically created unless we are in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Scenes cannot be dynamically created unless the engine is configured to run in sandbox mode.\n") );
        return 0;
    
    } // End if invalid

    // Insert into the scene database (serialized and / or internal).
    cgUInt32 sceneId = mConfiguration->insertScene( description );
    if ( !sceneId )
        return 0;

    // Notify listeners
    onSceneAdded( &cgSceneUpdateEventArgs( sceneId ) );
        
    // Success!
    return sceneId;
}

//-----------------------------------------------------------------------------
// Name : generateRefId ()
/// <summary>
/// Generate a new unique reference identifier that can be assigned to a 
/// reference object.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorld::generateRefId( bool internalReference )
{
    // If the engine is not in sandbox mode, all reference targets are internal.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        internalReference = true;

    // Internal reference identifier or database?
    if ( internalReference == true )
    {
        // Ask standard reference manager to assign.
        return cgReferenceManager::generateInternalRefId();
    
    } // End if internal
    else
    {
        return mConfiguration->generateRefId();

    } // End if !internal
}

//-----------------------------------------------------------------------------
// Name : componentTablesCreated () (Virtual)
/// <summary>
/// Called in order register the fact that type tables have been created in the
/// world database for the specified component.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::componentTablesCreated( const cgUID & typeIdentifier )
{
    mExistingTypeTables.insert( typeIdentifier );
}

//-----------------------------------------------------------------------------
// Name : componentTablesExist () (Virtual)
/// <summary>
/// Determine if type tables have been created in the world database for the 
/// specified component.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::componentTablesExist( const cgUID & typeIdentifier ) const
{
    return ( mExistingTypeTables.find( typeIdentifier ) != mExistingTypeTables.end() );
}

//-----------------------------------------------------------------------------
// Name : tableExists()
/// <summary>
/// Determine if a data table with the specified name currently exists in the 
/// master world database file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::tableExists( const cgString & name )
{
    static const cgString queryString = _T("SELECT name FROM sqlite_master WHERE type='table' AND name=?1");
    cgWorldQuery query( this, queryString );
    if ( !query.isPrepared() )
        return false;

    // Execute the query and get the result.
    bool result = false;
    query.bindParameter( 1, name );
    if ( query.step( false ) )
        result = query.hasResults();
    
    // Clean up and return the result
    query.reset();
    return result;
}

//-----------------------------------------------------------------------------
// Name : createObject () (Virtual)
/// <summary>
/// Insert information about the specified object into the world database
/// including its type if it doesn't already exist.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgWorld::createObject( bool internalObject, const cgUID & typeIdentifier )
{
    return createObject( internalObject, typeIdentifier, cgCloneMethod::None, CG_NULL );
}

//-----------------------------------------------------------------------------
// Name : createObject () (Virtual)
/// <summary>
/// Insert information about the specified object into the world database
/// including its type if it doesn't already exist. The new object should 
/// represent a clone of the specified initializing object, cloned using the 
/// provided method.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgWorld::createObject( bool internalObject, const cgUID & typeIdentifier, cgCloneMethod::Base initMethod, cgWorldObject * init )
{
    cgWorldObject * object = CG_NULL;
    cgString error;

    // If the engine is not in sandbox mode, all new objects are internal.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        internalObject = true;
    
    // Find the description associated with the underlying object type for this identifier.
    const cgWorldObjectTypeDesc * objectType = mConfiguration->getObjectType( typeIdentifier );
    if ( !objectType ) 
    {
        cgString identifierString = cgStringUtility::toString( typeIdentifier, _T("B") );
        cgAppLog::write( cgAppLog::Error, _T("Unable to create a world object with type identifier '%s' because no matching type was registered by the application prior to opening the world database.\n"), identifierString.c_str() );
        return CG_NULL;
    
    } // End if not found
    
    // Catch Exceptions
    try
    {
        // Ensure we can roll back on failure.
        if ( !internalObject )
            beginTransaction( _T("createObject") );

        // Generate a new reference identifier for this object.
        cgUInt32 referenceId = generateRefId( internalObject );

        // Create new instance (with relevant constructor parameters)
        if ( initMethod == cgCloneMethod::None )
        {
            // No clone, just allocate a new object.
            if ( objectType->objectAllocNew )
                object = objectType->objectAllocNew( typeIdentifier, referenceId, this );
        
        } // End if no clone
        else if ( objectType->objectAllocClone )
        {
            // Clone based on the specified object.
            object = objectType->objectAllocClone( typeIdentifier, referenceId, this, init, initMethod );
        
        } // End if clone

        // Success?
        if ( !object )
            throw ResultException( cgString::format( _T("Unable to create new world object. Failed to create instance for type '%s'."), 
                                                     cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
        
        // Insert type into database if it does not already exist.
        if ( !internalObject )
        {
            if ( !componentTablesExist(typeIdentifier) && !object->createTypeTables( typeIdentifier ) )
                throw ResultException( cgString::format( _T("Failed to create world database type tables for defined object type '%s'."), 
                                                         cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
            if ( !mConfiguration->insertObjectType( typeIdentifier, object->getDatabaseTable() ) )
                throw ResultException( cgString::format( _T("Failed to create world database type description entry for defined object type '%s'."), 
                                                         cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );

        } // End if !internal

        // Allow object to initialize / insert into database if required.
        if ( !object->onComponentCreated( &cgComponentCreatedEventArgs( objectType->localIdentifier, initMethod ) ) )
            throw ResultException( cgString::format( _T("onComponentCreated() returned failure when creating new world object of type '%s'."), 
                                                     cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
        
        // Commit the changes.
        if ( !internalObject )
            commitTransaction( _T("createObject") );

    } // End try

    catch ( const ResultException & e )
    {
        cgAppLog::write( cgAppLog::Warning, _T("%s\n"), e.toString().c_str() );
        if ( !internalObject )
            rollbackTransaction( _T("createObject") );
        if ( object )
            object->deleteReference();
        return CG_NULL;
    
    } // End catch
    
    // Notify whoever is interested that the scene was modified
    // ToDo: 9999 - Do we need a notification?
    // OnObjectAdded( gcnew ObjectUpdatedEventArgs( this, object ) );
    
    // Success?
    return object;
}

//-----------------------------------------------------------------------------
// Name : createObjectSubElement () (Virtual)
/// <summary>
/// Insert information about the specified object sub-element into the world
/// database including its type if it doesn't already exist.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgWorld::createObjectSubElement( bool internalElement, const cgUID & typeIdentifier, cgWorldObject * object )
{
    return createObjectSubElement( internalElement, typeIdentifier, object, CG_NULL );
}

//-----------------------------------------------------------------------------
// Name : createObjectSubElement () (Virtual)
/// <summary>
/// Insert information about the specified object sub-element into the world
/// database including its type if it doesn't already exist. The new element
/// should represent a clone of the specified initializing element.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgWorld::createObjectSubElement( bool internalElement, const cgUID & typeIdentifier, cgWorldObject * object, cgObjectSubElement * init )
{
    cgObjectSubElement * element = CG_NULL;

    // If the engine is not in sandbox mode, all new object elements are internal.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        internalElement = true;
    
    // Find the description associated with the underlying sub-element type for this identifier.
    const cgObjectSubElementTypeDesc * objectSubElementType = mConfiguration->getObjectSubElementType( typeIdentifier );
    if ( !objectSubElementType ) 
    {
        cgString strIdentifier = cgStringUtility::toString( typeIdentifier, _T("B") );
        cgAppLog::write( cgAppLog::Error, _T("Unable to create a world object sub-element with type identifier '%s' because no matching type was registered by the application prior to opening the world database.\n"), strIdentifier.c_str() );
        return CG_NULL;
    
    } // End if not found

    // Catch Exceptions
    try
    {
        // Ensure we can roll back on failure.
        if ( !internalElement )
            beginTransaction( _T("createObjectSubElement") );

        // Generate a new reference identifier for this object sub-element.
        cgUInt32 referenceId = generateRefId( internalElement );

        // Create new instance (with relevant constructor parameters)
        if ( !init )
        {
            // No clone, just allocate a new object sub-element.
            if ( objectSubElementType->elementAllocNew )
                element = objectSubElementType->elementAllocNew( typeIdentifier, referenceId, object );
        
        } // End if no clone
        else if ( objectSubElementType->elementAllocClone )
        {
            // Clone based on the specified object.
            element = objectSubElementType->elementAllocClone( typeIdentifier, referenceId, object, init );
        
        } // End if clone

        // Success?
        if ( !element )
            throw ResultException( cgString::format( _T("Unable to create new object sub-element. Failed to create instance for type '%s'."), 
                                                     cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
        
        // Insert type into database if it does not already exist.
        if ( !internalElement )
        {
            if ( !componentTablesExist(typeIdentifier) && !element->createTypeTables( typeIdentifier ) )
                throw ResultException( cgString::format( _T("Failed to create world database type tables for defined object sub-element type '%s'."), 
                                                         cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
            if ( !mConfiguration->insertObjectSubElementType( typeIdentifier, element->getDatabaseTable() ) )
                throw ResultException( cgString::format( _T("Failed to create world database type description entry for defined object sub-element type '%s'."), 
                                                         cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );

        } // End if !internal

        // Allow object to initialize / insert into database if required.
        if ( !element->onComponentCreated( &cgComponentCreatedEventArgs( objectSubElementType->localIdentifier, (init != CG_NULL) ? cgCloneMethod::Copy : cgCloneMethod::None ) ) )
            throw ResultException( cgString::format( _T("onComponentCreated() returned failure when creating new object sub-element of type '%s'."), 
                                                     cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
        
        // Commit the changes.
        if ( !internalElement )
            commitTransaction( _T("createObjectSubElement") );

    } // End try

    catch ( const ResultException & e )
    {
        cgAppLog::write( cgAppLog::Warning, _T("%s\n"), e.toString().c_str() );
        if ( !internalElement )
            rollbackTransaction( L"createObjectSubElement" );
        if ( element )
            element->deleteReference();
        return CG_NULL;
    
    } // End catch
    
    // Notify whoever is interested that the scene was modified
    // ToDo: 9999 - Do we need a notification?
    // OnObjectAdded( gcnew ObjectUpdatedEventArgs( this, object ) );
    
    // Success?
    return element;
}

//-----------------------------------------------------------------------------
// Name : createSceneElement () (Virtual)
/// <summary>
/// Insert information about the specified scene element into the world 
/// database including its type if it doesn't already exist.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneElement * cgWorld::createSceneElement( bool internalElement, const cgUID & typeIdentifier, cgScene * scene )
{
    cgSceneElement * element = CG_NULL;

    // If the engine is not in sandbox mode, all new scene elements are internal.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        internalElement = true;
    
    // Find the description associated with the underlying element type for this identifier.
    const cgSceneElementTypeDesc * sceneElementType = mConfiguration->getSceneElementType( typeIdentifier );
    if ( !sceneElementType ) 
    {
        cgString identifierString = cgStringUtility::toString( typeIdentifier, _T("B") );
        cgAppLog::write( cgAppLog::Error, _T("Unable to create a scene element with type identifier '%s' because no matching type was registered by the application prior to opening the world database.\n"), identifierString.c_str() );
        return CG_NULL;
    
    } // End if not found

    // Catch Exceptions
    try
    {
        // Ensure we can roll back on failure.
        if ( !internalElement )
            beginTransaction( _T("createSceneElement") );

        // Generate a new reference identifier for this scene element.
        cgUInt32 referenceId = generateRefId( internalElement );

        // Create new instance (with relevant constructor parameters)
        if ( sceneElementType->elementAllocNew )
            element = sceneElementType->elementAllocNew( typeIdentifier, referenceId, scene );
        // NB: If cloning is ever added here, update the 'onComponentCreated()' call to pass 'cgCloneMethod::Copy' where appropriate.
        
        // Success?
        if ( !element )
            throw ResultException( cgString::format( _T("Unable to create new scene element. Failed to create instance for type '%s'."), 
                                                     cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
        
        // Insert type into database if it does not already exist.
        if ( !internalElement )
        {
            if ( !componentTablesExist(typeIdentifier) && !element->createTypeTables( typeIdentifier ) )
                throw ResultException( cgString::format( _T("Failed to create world database type tables for defined scene element type '%s'."), 
                                                         cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
            if ( !mConfiguration->insertSceneElementType( typeIdentifier, element->getDatabaseTable() ) )
                throw ResultException( cgString::format( _T("Failed to create world database type description entry for defined scene element type '%s'."), 
                                                         cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );

        } // End if !internal

        // Allow object to initialize / insert into database if required.
        if ( !element->onComponentCreated( &cgComponentCreatedEventArgs( sceneElementType->localIdentifier, cgCloneMethod::None ) ) )
            throw ResultException( cgString::format( _T("onComponentCreated() returned failure when creating new scene element of type '%s'."), 
                                                     cgStringUtility::toString( typeIdentifier, _T("B") ).c_str() ), cgDebugSource() );
        
        // Commit the changes.
        if ( !internalElement )
            commitTransaction( _T("createSceneElement") );

    } // End try

    catch ( const ResultException & e )
    {
        cgAppLog::write( cgAppLog::Warning, _T("%s\n"), e.toString().c_str() );
        if ( !internalElement )
            rollbackTransaction( L"createSceneElement" );
        if ( element )
            element->deleteReference();
        return CG_NULL;
    
    } // End catch
    
    // Notify whoever is interested that the scene was modified
    // ToDo: 9999 - Do we need a notification?
    // OnObjectAdded( gcnew ObjectUpdatedEventArgs( this, object ) );
    
    // Success?
    return element;
}

//-----------------------------------------------------------------------------
// Name : loadObject () (Virtual)
/// <summary>
/// Recreate a previously existing object, loading it from the database.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgWorld::loadObject( const cgUID & typeIdentifier, cgUInt32 referenceId, cgCloneMethod::Base cloneMethod )
{
    cgWorldObject * newObject = CG_NULL;
    
    // Find the data associated with the underlying object type for this identifier.
    const cgWorldObjectTypeDesc * objectType = mConfiguration->getObjectType( typeIdentifier );
    if ( !objectType )
        return CG_NULL; // Silent fail -- user was notified during type table parsing.
    
    // Catch exceptions
    try
    {
        // Check to see if a reference with this identifier is already resident in memory.
        cgReference * reference = cgReferenceManager::getReference( referenceId );
        if ( reference )
        {
            // *Something* was already resident in memory a matching reference identifier
            // but we don't yet know what it is. If it's not a world object of the requested 
            // type, clearly there is something wrong with the supplied identifier.
            if ( !reference->queryReferenceType( typeIdentifier ) )
            {
                cgString requestIdentifier = cgStringUtility::toString( typeIdentifier, _T("B") );
                cgString foundIdentifier   = cgStringUtility::toString( reference->getReferenceType(), _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested world object '0x%x'. A reference with the specified identifier was found to exist but its type of '%s' did not match the requested object type '%s'."), referenceId, foundIdentifier.c_str(), requestIdentifier.c_str() ), cgDebugSource() );

            } // End if conflicted

            // A *world object* with this identifier already exists in memory. If the user 
            // opted not to clone it, we can just return the existing reference.
            cgWorldObject * existingObject = (cgWorldObject*)reference;
            if ( cloneMethod == cgCloneMethod::None || cloneMethod == cgCloneMethod::ObjectInstance )
                return existingObject;
            
            // Otherwise, we need to clone the existing object using the cloning 
            // method requested.
            if ( !existingObject->clone( cloneMethod, this, true, newObject ) )
                throw ResultException( cgString::format( _T("Unable to load requested world object '0x%x' using the specified cloning method. Refer to earlier errors for more information.\n"), referenceId ), cgDebugSource() );
            
        } // End if already resident
        else
        {
            // There is nothing with this reference identifier already resident in 
            // memory in any part of the engine. So, before we do anything, we need 
            // to generate a new internal identifier for the new object itself if 
            // the user will be ultimately cloning the object and (optionally) its data, 
            cgUInt32 finalReferenceId = referenceId;
            if ( cloneMethod != cgCloneMethod::None && cloneMethod != cgCloneMethod::ObjectInstance )
                finalReferenceId = generateRefId( true );

            // Now allocate a new object of the requested type.
            if ( objectType->objectAllocNew )
                newObject = objectType->objectAllocNew( typeIdentifier, finalReferenceId, this );
            if ( !newObject )
            {
                cgString identifierString = cgStringUtility::toString( typeIdentifier, _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested world object '0x%x'. Failed to create a physical world object instance of type '%s'.\n"), referenceId, identifierString.c_str() ), cgDebugSource() );

            } // End if failed

            // Allow the object to load.
            if ( !newObject->onComponentLoading( &cgComponentLoadingEventArgs( referenceId, objectType->localIdentifier, cloneMethod, CG_NULL ) ) )
            {
                cgString identifierString = cgStringUtility::toString( typeIdentifier, _T("B") );
                throw ResultException( cgString::format( _T("onComponentLoading() returned failure when loading world object '0x%x' of type '%s'."), referenceId, identifierString.c_str() ), cgDebugSource() );

            } // End if failed
                        
            // Notify whoever is interested that the scene was modified
            // ToDo: 9999 - Do we need a notification?
            // OnObjectAdded( gcnew ObjectUpdatedEventArgs( this, object ) );

        } // End if not resident
    
        // Success!
        return newObject;

    } // End try

    catch ( const ResultException & e )
    {
        if ( newObject )
            newObject->deleteReference();
        cgAppLog::write( cgAppLog::Warning, _T("%s\n"), e.toString().c_str() );
        return CG_NULL;

    } // End catch
}

//-----------------------------------------------------------------------------
// Name : loadObjectSubElement () (Virtual)
/// <summary>
/// Recreate a previously existing object sub-element, loading it from the
/// database.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgWorld::loadObjectSubElement( const cgUID & typeIdentifier, cgUInt32 referenceId, cgWorldObject * object, cgCloneMethod::Base cloneMethod )
{
    cgObjectSubElement * newElement = CG_NULL;
    
    // Find the data associated with the underlying object sub-element type for this identifier.
    const cgObjectSubElementTypeDesc * objectSubElementType = mConfiguration->getObjectSubElementType( typeIdentifier );
    if ( !objectSubElementType )
        return CG_NULL; // Silent fail -- user was notified during type table parsing.
    
    // Catch exceptions
    try
    {
        // Check to see if a reference with this identifier is already resident in memory.
        cgReference * reference = cgReferenceManager::getReference( referenceId );
        if ( reference )
        {
            // *Something* was already resident in memory a matching reference identifier
            // but we don't yet know what it is. If it's not an object sub-element of the 
            // requested type, clearly there is something wrong with the supplied identifier.
            if ( !reference->queryReferenceType( typeIdentifier ) )
            {
                cgString requestIdentifier = cgStringUtility::toString( typeIdentifier, _T("B") );
                cgString foundIdentifier   = cgStringUtility::toString( reference->getReferenceType(), _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested object sub-element '0x%x'. A reference with the specified identifier was found to exist but its type of '%s' did not match the requested object sub-element type '%s'."), referenceId, foundIdentifier.c_str(), requestIdentifier.c_str() ), cgDebugSource() );

            } // End if conflicted

            // An *object sub-element* with this identifier already exists in memory. If the user 
            // opted not to clone it, we can just return the existing reference.
            cgObjectSubElement * existingElement = (cgObjectSubElement*)reference;
            if ( cloneMethod == cgCloneMethod::None || cloneMethod == cgCloneMethod::ObjectInstance )
                return existingElement;

            // Otherwise, we need to clone the existing element using the cloning 
            // method requested.
            if ( !existingElement->clone( object, true, newElement ) )
                throw ResultException( cgString::format( _T("Unable to load requested object sub-element '0x%x' using the specified cloning method. Refer to earlier errors for more information.\n"), referenceId ), cgDebugSource() );

        } // End if already resident
        else
        {
            // There is nothing with this reference identifier already resident in memory
            // in any part of the engine. So, before we do anything, we need to  generate 
            // a new internal identifier for the new element itself if the user will be 
            // ultimately cloning the object and (optionally) its data, 
            cgUInt32 finalReferenceId = referenceId;
            if ( cloneMethod != cgCloneMethod::None && cloneMethod != cgCloneMethod::ObjectInstance )
                finalReferenceId = generateRefId( true );

            // Now allocate a new object sub-element of the requested type.
            if ( objectSubElementType->elementAllocNew )
                newElement= objectSubElementType->elementAllocNew( typeIdentifier, finalReferenceId, object );
            if ( !newElement )
            {
                cgString identifierString = cgStringUtility::toString( typeIdentifier, _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested object sub-element '0x%x'. Failed to create a physical object sub-element instance of type '%s'.\n"), referenceId, identifierString.c_str() ), cgDebugSource() );

            } // End if failed

            // Allow the object to load.
            if ( !newElement->onComponentLoading( &cgComponentLoadingEventArgs( referenceId, objectSubElementType->localIdentifier, cloneMethod, CG_NULL ) ) )
            {
                cgString identifierString = cgStringUtility::toString( typeIdentifier, _T("B") );
                throw ResultException( cgString::format( _T("onComponentLoading() returned failure when loading object sub-element '0x%x' of type '%s'."), referenceId, identifierString.c_str() ), cgDebugSource() );

            } // End if failed

        } // End if not resident
            
        // Success!
        return newElement;

    } // End try

    catch ( const ResultException & e )
    {
        if ( newElement )
            newElement->deleteReference();
        cgAppLog::write( cgAppLog::Warning, _T("%s\n"), e.toString().c_str() );
        return CG_NULL;

    } // End catch
}

//-----------------------------------------------------------------------------
// Name : loadSceneElement () (Virtual)
/// <summary>
/// Recreate a previously existing scene element, loading it from the database.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneElement * cgWorld::loadSceneElement( const cgUID & typeIdentifier, cgUInt32 referenceId, cgScene * scene )
{
    cgSceneElement * newElement = CG_NULL;
    
    // Find the data associated with the underlying scene element type for this identifier.
    const cgSceneElementTypeDesc * sceneElementType = mConfiguration->getSceneElementType( typeIdentifier );
    if ( !sceneElementType )
        return CG_NULL; // Silent fail -- user was notified during type table parsing.
    
    // Catch exceptions
    try
    {
        // Check to see if a reference with this identifier is already resident in memory.
        cgReference * reference = cgReferenceManager::getReference( referenceId );
        if ( reference )
        {
            // *Something* was already resident in memory a matching reference identifier
            // but we don't yet know what it is. If it's not a scene element of the requested
            // type, clearly there is something wrong with the supplied identifier.
            if ( !reference->queryReferenceType( typeIdentifier ) )
            {
                cgString requestIdentifier = cgStringUtility::toString( typeIdentifier, _T("B") );
                cgString foundIdentifier   = cgStringUtility::toString( reference->getReferenceType(), _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested scene element '0x%x'. A reference with the specified identifier was found to exist but its type of '%s' did not match the requested scene element type '%s'."), referenceId, foundIdentifier.c_str(), requestIdentifier.c_str() ), cgDebugSource() );

            } // End if conflicted

            // A *scene element* with this identifier already exists in memory. 
            // We can just return the existing reference.
            return (cgSceneElement*)reference;

        } // End if already resident
        else
        {
            // There is nothing with this reference identifier already resident in memory
            // in any part of the engine so allocate new scene element of the requested type.
            if ( sceneElementType->elementAllocNew )
                newElement = sceneElementType->elementAllocNew( typeIdentifier, referenceId, scene );
            if ( !newElement )
            {
                cgString identifierString = cgStringUtility::toString( typeIdentifier, _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested scene element '0x%x'. Failed to create a physical scene element instance of type '%s'.\n"), referenceId, identifierString.c_str() ), cgDebugSource() );

            } // End if failed

            // Allow the object to load.
            if ( !newElement->onComponentLoading( &cgComponentLoadingEventArgs( referenceId, sceneElementType->localIdentifier, cgCloneMethod::None, CG_NULL ) ) )
            {
                cgString identifierString = cgStringUtility::toString( typeIdentifier, _T("B") );
                throw ResultException( cgString::format( _T("onComponentLoading() returned failure when loading scene element '0x%x' of type '%s'."), referenceId, identifierString.c_str() ), cgDebugSource() );

            } // End if failed

        } // End if not resident
            
        // Success!
        return newElement;

    } // End try

    catch ( const ResultException & e )
    {
        if ( newElement )
            newElement->deleteReference();
        cgAppLog::write( cgAppLog::Warning, _T("%s\n"), e.toString().c_str() );
        return CG_NULL;

    } // End catch
}

//-----------------------------------------------------------------------------
//  Name : loadScene ()
/// <summary>
/// Allocate and load the specified scene by identifier. This will activate
/// a scene such that all required data will be loaded / begin to load
/// and it can now be updated/rendered.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgWorld::loadScene( cgUInt32 sceneId )
{
    // Is this scene already loaded?
    if ( getLoadedSceneById( sceneId ) )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Specified scene (Id:0x%x) has already been loaded. You are not permitted to load a scene multiple times.\n"), sceneId );
        return CG_NULL;

    } // End if scene is not already active

    // Retrieve the scene descriptor
    const cgSceneDescriptor * description = getSceneDescriptorById( sceneId );
    if ( !description )
    {
        cgAppLog::write( cgAppLog::Error, _T("Specified scene (Id:0x%x) could not be found within the loaded environment definition.\n"), sceneId );
        return CG_NULL;

    } // End if scene is not already active

    // Dump debug information
    cgAppLog::write( cgAppLog::Debug, _T("Loading scene '%s'.\n"), description->name.c_str() );
    
    // Allocate the scene
    cgScene * newScene = new cgScene( this, description );

    // Notify whoever is interested that this scene is about to be loaded.
    onSceneLoading( &cgSceneLoadEventArgs( newScene ) );

    // Begin loading the scene
    if ( !newScene->load() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Scene '%s' failed to complete the loading process. See any previous errors for more information where available.\n"), description->name.c_str() );

        // Notify subscribers that the load failed.
        onSceneLoadFailed( &cgSceneLoadEventArgs( newScene ) );

        // Clean up and bail
        newScene->scriptSafeDispose( );
        return CG_NULL;
    
    } // End if failed to begin loading

    // Loading was a success!
    onSceneLoaded( &cgSceneLoadEventArgs( newScene ) );

    // Add to the active scene map
    mActiveScenes.push_back( newScene );
    mActiveSceneIdMap[ sceneId ] = newScene;

    // Success!!
    return newScene;
}

//-----------------------------------------------------------------------------
//  Name : unloadScene ()
/// <summary>
/// Unload the loaded scene from the scene manager. This overloaded
/// method allows the scene to be specified by name.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::unloadScene( cgUInt32 sceneId )
{
    // Does the scene exist in our loaded scene database?
    cgScene * scene = getLoadedSceneById( sceneId );
    if ( !scene ) return;

    // Notify anyone interested that the scene is being unloaded.
    onSceneUnloading( &cgSceneLoadEventArgs( scene ) );

    // Remove from the loaded scene list
    SceneArray::iterator itScene = std::find( mActiveScenes.begin(), mActiveScenes.end(), scene );
    if ( itScene != mActiveScenes.end() )
        mActiveScenes.erase( itScene );
    mActiveSceneIdMap.erase( sceneId );

    // Cleanup the scene but pay attention to script
    // reference count on delete.
    scene->scriptSafeDispose( );
}

//-----------------------------------------------------------------------------
//  Name : unloadScene ()
/// <summary>
/// Unload the loaded scene from the scene manager. This overloaded
/// method allows the scene to be specified by it's pointer.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::unloadScene( cgScene * scene )
{
    // Paranoia check
    if ( !scene )
        return;

    // Notify anyone interested that the scene is being unloaded.
    onSceneUnloading( &cgSceneLoadEventArgs( scene ) );

    // Remove from the loaded scene list
    SceneArray::iterator itScene = std::find( mActiveScenes.begin(), mActiveScenes.end(), scene );
    if ( itScene != mActiveScenes.end() )
        mActiveScenes.erase( itScene );
    mActiveSceneIdMap.erase( scene->getSceneId() );

    // Cleanup the scene but pay attention to script
    // reference count on delete.
    scene->scriptSafeDispose( );
}

//-----------------------------------------------------------------------------
//  Name : getSceneCount ()
/// <summary>
/// Retrieve the total number of scenes contained within the loaded
/// environment definition.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorld::getSceneCount( ) const
{
    cgAssert(mConfiguration);
    return mConfiguration->getSceneCount();
}

//-----------------------------------------------------------------------------
//  Name : getLoadedSceneCount ()
/// <summary>
/// Retrieve the total number of scenes that are currently loaded / active.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorld::getLoadedSceneCount( ) const
{
    return (cgUInt32)mActiveScenes.size();
}

//-----------------------------------------------------------------------------
//  Name : getLoadedScene ()
/// <summary>
/// Retrieve a reference to the loaded scene indicated by the supplied index.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgWorld::getLoadedScene( cgUInt32 index ) const
{
    return mActiveScenes[index];
}

//-----------------------------------------------------------------------------
//  Name : getSceneDescriptor ()
/// <summary>
/// Retrieve the physical descriptor structure for a scene as it was
/// loaded from the environment definition file. This particular
/// method allows you to retrieve a descriptor by index.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneDescriptor * cgWorld::getSceneDescriptor( cgUInt32 index ) const
{
    cgAssert(mConfiguration);
    return mConfiguration->getSceneDescriptor( index );
}

//-----------------------------------------------------------------------------
//  Name : getSceneDescriptorByName ()
/// <summary>
/// Retrieve the physical descriptor structure for a scene as it was
/// loaded from the world definition file. This particular
/// method allows you to retrieve a descriptor using the name of the 
/// scene.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneDescriptor * cgWorld::getSceneDescriptorByName( const cgString & sceneName ) const
{
    cgAssert(mConfiguration);
    return mConfiguration->getSceneDescriptorByName( sceneName );
}

//-----------------------------------------------------------------------------
//  Name : getSceneDescriptorByName ()
/// <summary>
/// Retrieve the physical descriptor structure for a scene as it was
/// loaded from the world definition file. This particular
/// method allows you to retrieve a descriptor using the unique identifier
/// of the scene.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneDescriptor * cgWorld::getSceneDescriptorById( cgUInt32 sceneId ) const
{
    cgAssert(mConfiguration);
    return mConfiguration->getSceneDescriptorById( sceneId );
}

//-----------------------------------------------------------------------------
//  Name : updateSceneDescriptorById ()
/// <summary>
/// Update the scene descriptor for the scene with the specified identifier.
/// This method will automatically update both the database side description
/// (if necessary) as well as updating any loaded scene with the new 
/// information.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::updateSceneDescriptorById( cgUInt32 sceneId, const cgSceneDescriptor & sceneDescriptor )
{
    cgToDo( "Carbon General", "The restriction that we must exist in sandbox mode should be removed. Add support for internal scenes." );

    // Scene descriptors cannot be adjusted unless we are in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Scene descriptors cannot be updated unless the engine is configured to run in sandbox mode.\n") );
        return false;

    } // End if invalid

    cgAssert(mConfiguration);
    if ( !mConfiguration->updateSceneDescriptorById( sceneId, sceneDescriptor ) )
        return false;

    // If a scene with this id is currently loaded, make sure it is updated with the
    // new information.
    // ToDo: Probably better if this happens through a method so that the scene can take
    // some action where it is necessary to do so.
    cgScene * scene = getLoadedSceneById( sceneId );
    if ( scene )
    {
        scene->mSceneDescriptor = *getSceneDescriptorById( sceneId );

        // Mark scene as dirty.
        scene->setDirty( true );
    
    } // End if loaded

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getLoadedSceneByName ()
/// <summary>
/// If a scene with the specified name has been loaded, you can retrieve 
/// it using this method.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgWorld::getLoadedSceneByName( const cgString & sceneName ) const
{
    // Find the associated descriptor in order to get the scene identifier.
    const cgSceneDescriptor * description = getSceneDescriptorByName( sceneName );
    if ( !description )
        return CG_NULL;

    // Retrieve the loaded scene based on the unique integer identifier.
    return getLoadedSceneById( description->sceneId );
}

//-----------------------------------------------------------------------------
//  Name : getLoadedSceneById ()
/// <summary>
/// If a scene with the specified identifier has been loaded, you can retrieve 
/// it using this method.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgWorld::getLoadedSceneById( cgUInt32 sceneId ) const
{
    // Find any loaded scene with the specified name
    SceneMap::const_iterator itScene = mActiveSceneIdMap.find( sceneId );
    if ( itScene == mActiveSceneIdMap.end() )
        return CG_NULL;

    // Return the scene pointer
    return itScene->second;
}

//-----------------------------------------------------------------------------
//  Name : isSceneLoaded ()
/// <summary>
/// Determine if the specified scene is currently loaded / active.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorld::isSceneLoaded( cgUInt32 sceneId ) const
{
    // Find any loaded scene with the specified name
    return ( mActiveSceneIdMap.find( sceneId ) != mActiveSceneIdMap.end() );
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Instructs the world to issue an 'update' command to all currently loaded
/// scenes.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::update( )
{
    // Iterate through active scene's and issue allow them to update.
    SceneMap::iterator itScene;
    for ( itScene = mActiveSceneIdMap.begin(); itScene != mActiveSceneIdMap.end();  )
    {
        // Increment iterator before we trigger the update in case
        // the scene gets unloaded (and the active scene list modified)
        // whilst we're processing.
        cgScene * pScene = itScene->second;
        ++itScene;
        
        // Issue the update command.
        pScene->update();
    
    } // Next Scene
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseConnection( ) (Private)
/// <summary>
/// Retrieve the internal SQLite database connection.
/// </summary>
//-----------------------------------------------------------------------------
sqlite3 * cgWorld::getDatabaseConnection( )
{
    return mDatabase;
}

//-----------------------------------------------------------------------------
//  Name : getConfiguration( )
/// <summary>
/// Retrieve the global world configuration object. The configuration object
/// describes properties relating to defined world object types, layout version 
/// number, scene descriptors, user defined material property identifiers, etc.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldConfiguration * cgWorld::getConfiguration( ) const
{
    return mConfiguration;
}

//-----------------------------------------------------------------------------
//  Name : getRenderDriver ()
/// <summary>
/// Simply retrieve the render driver through which the world will be
/// rendered.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver * cgWorld::getRenderDriver( ) const
{
    // Worlds always currently use the main singleton render driver.
    return cgRenderDriver::getInstance();
}

//-----------------------------------------------------------------------------
//  Name : getResourceManager ()
/// <summary>
/// Simply retrieve the resource manager that will manage the world's
/// resources.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager * cgWorld::getResourceManager( ) const
{
    // Worlds always currently use the main singleton resource manager.
    return cgResourceManager::getInstance();
}

//-----------------------------------------------------------------------------
//  Name : onWorldDisposing () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the world is about to 
/// begin disposing data in order to notify all listeners.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::onWorldDisposing( cgWorldEventArgs * e )
{
    // Trigger 'onWorldDisposing' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgWorldEventListener*)(*itListener))->onWorldDisposing( e );
    
    // Build the message for anyone listening via messaging system
    cgMessage message;
    message.messageId      = cgSystemMessages::World_Disposing;
    message.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_World, &message );
}

//-----------------------------------------------------------------------------
//  Name : onSceneAdded () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when a new scene is added to 
/// the world in order to notify all listeners.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::onSceneAdded( cgSceneUpdateEventArgs * e )
{
    // Trigger 'onSceneAdded' of all listeners(duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgWorldEventListener*)(*itListener))->onSceneAdded( e );

    // Build the message for anyone listening via messaging system
    cgMessage message;
    message.messageId      = cgSystemMessages::World_SceneAdded;
    message.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_World, &message );
}

//-----------------------------------------------------------------------------
//  Name : onSceneLoading () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when a scene is about to be 
/// loaded in order to notify all listeners.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::onSceneLoading( cgSceneLoadEventArgs * e )
{
    // Trigger 'onSceneLoading' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgWorldEventListener*)(*itListener))->onSceneLoading( e );

    // Build the message for anyone listening via messaging system
    cgMessage message;
    message.messageId      = cgSystemMessages::World_SceneLoading;
    message.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_World, &message );
}

//-----------------------------------------------------------------------------
//  Name : onSceneLoadFailed () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the loading of a scene 
/// failed in order to notify all listeners.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::onSceneLoadFailed( cgSceneLoadEventArgs * e )
{
    // Trigger 'onSceneLoadFailed' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgWorldEventListener*)(*itListener))->onSceneLoadFailed( e );

    // Build the message for anyone listening via messaging system
    cgMessage message;
    message.messageId      = cgSystemMessages::World_SceneLoadFailed;
    message.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_World, &message );
}

//-----------------------------------------------------------------------------
//  Name : onSceneLoaded () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the loading of a scene 
/// succeeded in order to notify all listeners.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::onSceneLoaded( cgSceneLoadEventArgs * e )
{
    // Trigger 'onSceneLoaded' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgWorldEventListener*)(*itListener))->onSceneLoaded( e );

    // Build the message for anyone listening via messaging system
    cgMessage message;
    message.messageId      = cgSystemMessages::World_SceneLoaded;
    message.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_World, &message );
}

//-----------------------------------------------------------------------------
//  Name : onSceneUnloading () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when a scene is about to be 
/// unloaded in order to notify all listeners.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorld::onSceneUnloading( cgSceneLoadEventArgs * e )
{
    // Trigger 'onSceneUnloading' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgWorldEventListener*)(*itListener))->onSceneUnloading( e );

    // Build the message for anyone listening via messaging system
    cgMessage message;
    message.messageId      = cgSystemMessages::World_SceneUnloading;
    message.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_World, &message );
}