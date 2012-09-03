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
// Name : cgWorld.h                                                          //
//                                                                           //
// Desc : Provides classes responsible for loading and managing the          //
//        components necessary for the application to update, render and     //
//        interact with the world.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWORLD_H_ )
#define _CGE_CGWORLD_H_

//-----------------------------------------------------------------------------
// cgWorld Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldTypes.h>
#include <System/cgReference.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//----------------------------------------------------------------------------
class  cgSceneDescriptor;
class  cgRenderDriver;
class  cgResourceManager;
class  cgWorldObject;
class  cgObjectNode;
class  cgScene;
class  cgWorld;
class  cgWorldConfiguration;
class  cgTransform;
struct sqlite3;
struct sqlite3_stmt;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {7DC2CBE0-4F07-42C8-A008-F981C335DBE9}
const cgUID RTID_World = {0x7DC2CBE0, 0x4F07, 0x42C8, {0xA0, 0x8, 0xF9, 0x81, 0xC3, 0x35, 0xDB, 0xE9}};

//-----------------------------------------------------------------------------
// Event Argument Definitions
//-----------------------------------------------------------------------------
struct CGE_API cgWorldEventArgs
{
    cgWorldEventArgs( cgWorld * _world ) { world = _world; }
    cgWorld * world;

}; // End Struct cgWorldEventArgs

struct CGE_API cgSceneUpdateEventArgs
{
    cgSceneUpdateEventArgs( cgUInt32 _sceneId ) { sceneId = _sceneId; }
    cgUInt32 sceneId;

}; // End Struct cgSceneUpdateEventArgs

struct CGE_API cgSceneLoadEventArgs
{
    cgSceneLoadEventArgs( cgScene * _scene ) { scene = _scene; }
    cgScene * scene;

}; // End Struct cgSceneLoadEventArgs

//-----------------------------------------------------------------------------
// Main Class Declarations
//----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgWorldEventListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever world events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWorldEventListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgWorldEventListener, cgEventListener, "WorldEventListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onWorldDisposing    ( cgWorldEventArgs * e ) {};
    virtual void    onSceneAdded        ( cgSceneUpdateEventArgs * e ) {};
    virtual void    onSceneLoading      ( cgSceneLoadEventArgs * e ) {};
    virtual void    onSceneLoadFailed   ( cgSceneLoadEventArgs * e ) {};
    virtual void    onSceneLoaded       ( cgSceneLoadEventArgs * e ) {};
    virtual void    onSceneUnloading    ( cgSceneLoadEventArgs * e ) {};
};

//-----------------------------------------------------------------------------
//  Name : cgWorld (Class)
/// <summary>
/// The main class responsible for loading and managing a world. 
/// A world is essentially a collection of scenes which make up
/// a game levels or paritioned set of game data (main game vs cutscene
/// for instance).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWorld : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgWorld, cgReference, "World" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgScene;
    friend class cgWorldQuery;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWorld( );
    virtual ~cgWorld( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorld            * getInstance                 ( );
    static void                 createSingleton             ( );
    static void                 destroySingleton            ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        open                        ( const cgInputStream & stream );
    bool                        create                      ( cgWorldType::Base type );
    bool                        save                        ( const cgString & fileName );
    cgUInt32                    createScene                 ( const cgSceneDescriptor & description );
    cgScene                   * loadScene                   ( cgUInt32 sceneId );
    void                        unloadScene                 ( cgUInt32 sceneId );
    void                        unloadScene                 ( cgScene * scene );
    void                        update                      ( );
    cgWorldObject             * createObject                ( bool internalObject, const cgUID & typeIdentifier );
    cgWorldObject             * createObject                ( bool internalObject, const cgUID & typeIdentifier, cgCloneMethod::Base initMethod, cgWorldObject * init );
    cgObjectSubElement        * createObjectSubElement      ( bool internalElement, const cgUID & typeIdentifier, cgWorldObject * object );
    cgObjectSubElement        * createObjectSubElement      ( bool internalElement, const cgUID & typeIdentifier, cgWorldObject * object, cgObjectSubElement * init );
    cgWorldObject             * loadObject                  ( const cgUID & typeIdentifier, cgUInt32 objectRefId, cgCloneMethod::Base cloneMethod );
    cgObjectSubElement        * loadObjectSubElement        ( const cgUID & typeIdentifier, cgUInt32 objectSubElementRefId, cgWorldObject * object, cgCloneMethod::Base cloneMethod );
    cgUInt32                    generateRefId               ( bool internalReference );
    cgWorldConfiguration      * getConfiguration            ( ) const;
    cgUInt32                    getSceneCount               ( ) const;
    const cgSceneDescriptor   * getSceneDescriptor          ( cgUInt32 index ) const;
    const cgSceneDescriptor   * getSceneDescriptorById      ( cgUInt32 sceneId ) const;
    const cgSceneDescriptor   * getSceneDescriptorByName    ( const cgString & sceneName ) const;
    bool                        updateSceneDescriptorById   ( cgUInt32 sceneId, const cgSceneDescriptor & sceneDescriptor );
    cgScene                   * getLoadedSceneByName        ( const cgString & sceneName ) const;
    cgScene                   * getLoadedSceneById          ( cgUInt32 sceneId ) const;
    cgRenderDriver            * getRenderDriver             ( ) const;
    cgResourceManager         * getResourceManager          ( ) const;
    bool                        isSceneLoaded               ( cgUInt32 sceneId ) const;
    
    // Database methods
    void                        componentTablesCreated      ( const cgUID & typeIdentifier );
    bool                        componentTablesExist        ( const cgUID & typeIdentifier ) const;
    bool                        executeQuery                ( const cgString & statements, bool asOneTransaction );
    void                        beginTransaction            ( );
    void                        beginTransaction            ( const cgString & savepoint );
    void                        commitTransaction           ( );
    void                        commitTransaction           ( const cgString & savepoint );
    void                        rollbackTransaction         ( );
    void                        rollbackTransaction         ( const cgString & savepoint );
    void                        rollbackTransaction         ( const cgString & savepoint, bool restartTransaction );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_World; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgUInt32, cgScene*, SceneMap)
    CGE_UNORDEREDSET_DECLARE(cgUID, ComponentTypeTableSet)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                        buildObjectTypeTable            ( );
    bool                        buildObjectSubElementTypeTable  ( );
    bool                        buildSceneTable                 ( );
    sqlite3                   * getDatabaseConnection           ( );
    bool                        postConnect                     ( );
    bool                        postDatabaseExport              ( const cgString & fileName, sqlite3 * databaseOut );
    bool                        postDatabaseImport              ( const cgString & fileName, sqlite3 * databaseIn );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    // Event Virtuals
    virtual void                onWorldDisposing            ( cgWorldEventArgs * e );
    virtual void                onSceneAdded                ( cgSceneUpdateEventArgs * e );
    virtual void                onSceneLoading              ( cgSceneLoadEventArgs * e );
    virtual void                onSceneLoadFailed           ( cgSceneLoadEventArgs * e );
    virtual void                onSceneLoaded               ( cgSceneLoadEventArgs * e );
    virtual void                onSceneUnloading            ( cgSceneLoadEventArgs * e );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgWorldConfiguration          * mConfiguration;             // Maintains and manages updates for all world configuration tables.
    SceneMap                        mActiveScenes;              // Map containing all loaded scenes, keyed on name.

    // File database management.
    cgInputStream                   mDatabaseStream;            // The stream of the database to which we are connected.
    sqlite3                       * mDatabase;                  // The main connection maintained with the world file database.
    sqlite3_stmt                  * mStatementBegin;            // Cached SQL "BEGIN" statement.
    sqlite3_stmt                  * mStatementCommit;           // Cached SQL "COMMIT" statement.
    sqlite3_stmt                  * mStatementRollback;         // Cached SQL "ROLLBACK" statement.
    ComponentTypeTableSet           mExistingTypeTables;        // All component type tables that have been created in the database.

private:
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgWorld * mSingleton;    // Static singleton object instance.
};

#endif // !_CGE_CGWORLD_H_