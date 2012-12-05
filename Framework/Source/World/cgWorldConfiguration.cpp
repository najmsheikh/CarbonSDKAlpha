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
// File : cgWorldConfiguration.cpp                                           //
//                                                                           //
// Desc : Class that maintains and updates information about how the         //
//        currently loaded world was created and configured. Such            //
//        information includes version information, reference identifier     //
//        details, registered object types, material property identifiers,   //
//        and more.                                                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWorldConfiguration Module Includes
//-----------------------------------------------------------------------------
#include <World/cgWorldConfiguration.h>
#include <World/cgWorld.h>
#include <World/cgObjectSubElement.h>
#include <World/cgScene.h>
#include <World/cgSceneElement.h>
#include <System/cgStringUtility.h>

cgToDo( "Carbon General", "Consider creation of new world that is not database driven?" )

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
// World queries
cgWorldQuery cgWorldConfiguration::mIncrementNextRefId;
cgWorldQuery cgWorldConfiguration::mGetObjectType;
cgWorldQuery cgWorldConfiguration::mGetObjectSubElementType;
cgWorldQuery cgWorldConfiguration::mGetSceneElementType;
cgWorldQuery cgWorldConfiguration::mInsertObjectType;
cgWorldQuery cgWorldConfiguration::mInsertObjectSubElementType;
cgWorldQuery cgWorldConfiguration::mInsertSceneElementType;
cgWorldQuery cgWorldConfiguration::mInsertMaterialProperty;
cgWorldQuery cgWorldConfiguration::mInsertRenderClass;
cgWorldQuery cgWorldConfiguration::mInsertScene;
cgWorldQuery cgWorldConfiguration::mUpdateScene;

///////////////////////////////////////////////////////////////////////////////
// cgWorldConfiguration Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWorldConfiguration () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldConfiguration::cgWorldConfiguration( cgWorld * world )
{
    // Initialize variables to sensible defaults
    mWorld     = world;
    mWorldType = cgWorldType::Master;
    mVersion   = 0;
}

//-----------------------------------------------------------------------------
//  Name : ~cgWorld () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldConfiguration::~cgWorldConfiguration()
{
    // Destroy any allocated scene descriptors
    for ( SceneDescriptorArray::iterator itDesc = mSceneDescriptors.begin(); itDesc != mSceneDescriptors.end(); ++itDesc )
        delete *itDesc;
    mSceneDescriptors.clear();
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldConfiguration::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertScene.isPrepared() )
            mInsertScene.prepare( mWorld, _T("INSERT INTO 'Scenes' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17)"), true );
        if ( !mUpdateScene.isPrepared() )
            mUpdateScene.prepare( mWorld, _T("UPDATE 'Scenes' SET Type=?1, FriendlyName=?2, EditorName=?3, Description=?4, Flags=?5, RenderControlScript=?6, DistanceDisplayUnits=?6, ")
                                          _T("BoundsMinX=?8, BoundsMinY=?9, BoundsMinZ=?10, BoundsMaxX=?11, BoundsMaxY=?12, BoundsMaxZ=?13, ")
                                          _T("CellDimensionsX=?14, CellDimensionsY=?15, CellDimensionsZ=?16, LandscapeId=?17 WHERE SceneId=?18"), true );
        if ( !mInsertObjectType.isPrepared() )
            mInsertObjectType.prepare( mWorld, _T("INSERT INTO 'ObjectTypes' VALUES(NULL,?1,?2,?3)"), true );
        if ( !mInsertObjectSubElementType.isPrepared() )
            mInsertObjectSubElementType.prepare( mWorld, _T("INSERT INTO 'ObjectSubElementTypes' VALUES(NULL,?1,?2,?3)"), true );
        if ( !mInsertSceneElementType.isPrepared() )
            mInsertSceneElementType.prepare( mWorld, _T("INSERT INTO 'SceneElementTypes' VALUES(NULL,?1,?2,?3)"), true );
        if ( !mInsertMaterialProperty.isPrepared() )
            mInsertMaterialProperty.prepare( mWorld, _T("INSERT INTO 'Configuration::MaterialProperties' VALUES(NULL,?1,?2,?3,?4)"), true );
        if ( !mInsertRenderClass.isPrepared() )
            mInsertRenderClass.prepare( mWorld, _T("INSERT INTO 'Configuration::RenderClasses' VALUES(NULL,?1,?2)"), true );
        if ( !mIncrementNextRefId.isPrepared() )
            mIncrementNextRefId.prepare( mWorld, _T("SELECT NextRefId FROM 'Configuration' WHERE Type='Primary';")
                                                 _T("UPDATE 'Configuration' SET NextRefId=NextRefId+1 WHERE Type='Primary'"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mGetObjectType.isPrepared() )
        mGetObjectType.prepare( mWorld, _T("SELECT * FROM 'ObjectTypes' WHERE Identifier=?1 LIMIT 1"), true );
    if ( !mGetObjectSubElementType.isPrepared() )
        mGetObjectSubElementType.prepare( mWorld, _T("SELECT * FROM 'ObjectSubElementTypes' WHERE Identifier=?1 LIMIT 1") );
    if ( !mGetSceneElementType.isPrepared() )
        mGetSceneElementType.prepare( mWorld, _T("SELECT * FROM 'SceneElementTypes' WHERE Identifier=?1 LIMIT 1") );
}

//-----------------------------------------------------------------------------
// Name : newConfiguration () (Protected)
/// <summary>
/// Create a new 'default' world file configuration ready for modification.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::newConfiguration( cgWorldType::Base type )
{
    // Store new world details.
    mWorldType = type;
    mVersion  = cgMakeVersion( CGE_WORLD_VERSION, CGE_WORLD_SUBVERSION, CGE_WORLD_REVISION );

    // Create the required file configuration row using the defaults.
    cgStringParser queryString;
    queryString << _T("INSERT INTO 'Configuration' VALUES('Primary',");
    queryString << (cgInt32)type;
    queryString << _T(",");
    queryString << CGE_WORLD_VERSION;
    queryString << _T(",");
    queryString << CGE_WORLD_SUBVERSION;
    queryString << _T(",");
    queryString << CGE_WORLD_REVISION;
    queryString << _T(",1)");
    if ( !mWorld->executeQuery( queryString.str(), false ) )
        return false;

    // Insert default material types
    if ( !insertMaterialProperty( _T("Forward Rendered"), _T("ForwardRendered"),
                                  _T("Geometry should be rendered (and lit) using forward rendering techniques allowing for increased rendering customization."),
                                  0 ) ||
         !insertMaterialProperty( _T("Alpha Tested"), _T("AlphaTested"),
                                  _T("Material and per-pixel alpha / opacity values should be considered as a means for clipping pixels to be classified as transparent."),
                                  1 ) || 
         !insertMaterialProperty( _T("Alpha Blended"), _T("AlphaBlended"),
                                  _T("Material and per-pixel alpha / opacity values should be considered as a means for blending pixels with the existing contents of the frame buffer."),
                                  2 ) ||
         !insertMaterialProperty( _T("Refractive"), _T("Refractive"),
                                  _T("Surfaces to which this material is applied should be considered refractive. This will cause the existing contents of the frame buffer below the surface to 'warp' based on the configured refraction settings."),
                                  3 ) ||
         !insertMaterialProperty( _T("Reflective"), _T("Reflective"),
                                  _T("Surfaces to which this material is applied should be considered reflective. Real-time or pre-computed reflections (environment maps) will be applied based on the configured reflection settings."),
                                  4 ) ||
         !insertMaterialProperty( _T("Pre-generated Lighting"), _T("Prelit"),
                                  _T("Lighting data has been pre-computed in advance and applied to the surface in the form of a light-map."),
                                  5 ) )
        return false;

    // Insert default render classes
    if ( !insertRenderClass( _T("Default"), _T("Default render class. Usually reserved for standard, non-special effect related objects such as meshes.") ) ||
         !insertRenderClass( _T("Transparent"), _T("Transparent / translucent meshes.") ) ||
         !insertRenderClass( _T("Effects"), _T("Special effect related objects such as particle emitters and billboards.") ) )
        return false;

    // Populate our local object type lists with the list of types registered
    // by the application at start-up.
    mObjectTypes = cgWorldObject::getRegisteredTypes();
    mObjectSubElementTypes = cgObjectSubElement::getRegisteredTypes();
    mSceneElementTypes = cgSceneElement::getRegisteredTypes();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : loadConfiguration () (Protected)
/// <summary>
/// Load the current world configuration from the recently opened world 
/// database file.
/// </summary>
//-----------------------------------------------------------------------------
cgConfigResult::Base cgWorldConfiguration::loadConfiguration( cgUInt32 minSupportedVersion /* = 0 */, cgUInt32 maxSupportedVersion /* = 0xFFFFFFFF */, bool autoUpgrade /* = false */ )
{
    // Load the primary configuration row.
    cgString error;
    cgWorldQuery query( mWorld, _T("SELECT * FROM 'Configuration' WHERE Type='Primary' LIMIT 1") );
    if ( query.getLastError( error ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create world database query for primary configuration entry. Error: %s.\n"), error.c_str() );
        return cgConfigResult::Error;
    
    } // End if failed.

    // Execute the query and step to the first row.
    if ( !query.step() || !query.nextRow() )
    {
        if ( query.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to query world database for primary configuration entry. Error: %s.\n"), error.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to query world database for primary configuration entry. The world database has potentially become corrupt.\n") );
        query.reset();
        return cgConfigResult::Error;
    
    } // End if failed.

    // Update our local members
    cgInt32 fileType, version, subversion, revision;
    query.getColumn( _T("FileType"), fileType );
    query.getColumn( _T("Version"), version );
    query.getColumn( _T("Subversion"), subversion );
    query.getColumn( _T("Revision"), revision );
    mWorldType = (cgWorldType::Base)fileType;

    // We're done with the read data, release it.
    query.reset();

    // Build the version number and check to see if it falls within
    // the required version range.
    mVersion = cgMakeVersion( version, subversion, revision );
    if ( mVersion < minSupportedVersion || mVersion > maxSupportedVersion )
        return cgConfigResult::Mismatch;

    // If layout is not the latest version, check to see if it 
    // can be / needs to be converted.
    mLayoutStatus = cgWorldDatabaseStatus::Valid;
    if ( mVersion < maxSupportedVersion )
    {
        // Check to see if a conversion file exists (this indicates that an upgrade is REQUIRED)
        cgString conversionFile = cgString::format( _T("sys://Layout/Conversion/%x_to_%x.dat"), mVersion, maxSupportedVersion );
        bool conversionRequired = cgFileSystem::fileExists( cgFileSystem::resolveFileLocation( conversionFile ) );

        // Upgrade?
        mLayoutStatus = cgWorldDatabaseStatus::LegacyLayout;
        if ( autoUpgrade && conversionRequired )
        {
            cgUInt32 newVersion, newSubversion, newRevision;
            cgDecomposeVersion( maxSupportedVersion, newVersion, newSubversion, newRevision );
            cgAppLog::write( cgAppLog::Info, _T("Attempting to upgrade layout of world database from v%i.%02i.%04i to v%i.%02i.%04i.\n"), 
                             version, subversion, revision, newVersion, newSubversion, newRevision );

            // Run the upgrade script
            cgString layout = cgFileSystem::loadStringFromStream( conversionFile );
            if ( !mWorld->executeQuery( layout, true ) )
                return cgConfigResult::Error;

            // Update version number and mark as upgraded
            mVersion = maxSupportedVersion;
            mLayoutStatus = cgWorldDatabaseStatus::LayoutUpdated;

        } // End if auto upgrade
        else if ( conversionRequired )
        {
            // Just fail.
            return cgConfigResult::Mismatch;

        } // End if cannot upgrade

    } // End if not latest version
    
    // Build local object type table.
    if ( !loadObjectTypeTable( ) )
        return cgConfigResult::Error;

    // Build local object sub-element type table.
    if ( !loadObjectSubElementTypeTable( ) )
        return cgConfigResult::Error;

    // Build local scene element type table.
    if ( !loadSceneElementTypeTable( ) )
        return cgConfigResult::Error;

    // Build a list of scene descriptors from the database.
    if ( !loadSceneTable( ) )
        return cgConfigResult::Error;

    // Build a list of the user-defined material properties.
    if ( !loadMaterialPropertiesTable( ) )
        return cgConfigResult::Error;

    // Build a list of the user-defined object render classes.
    if ( !loadRenderClassTable( ) )
        return cgConfigResult::Error;

    // Success!
    return cgConfigResult::Valid;
}

//-----------------------------------------------------------------------------
//  Name : loadObjectTypeTable () (Protected)
/// <summary>
/// Build the local object type table by combining the list of known
/// registered object types defined by the application, and those defined
/// within the database. This new type list is specific to the opened
/// world.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::loadObjectTypeTable( )
{
    // Populate our local object type list with the list of types registered
    // by the application at start-up.
    mObjectTypes = cgWorldObject::getRegisteredTypes();

    // Query object types defined within the database.
    cgString error;
    cgWorldQuery query( mWorld, _T("SELECT * FROM ObjectTypes") );
    if ( query.getLastError( error ) == true )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to query world database for defined object types. Error: %s.\n"), error.c_str() );
        return false;
    
    } // End if failed.

    // Execute the query.
    if ( query.step() == true )
    {
        // Retrieve results.
        while( query.nextRow() )
        {
            cgUID    globalIdentifier;
            cgString identifierString, name;

            // Retrieve the global type identifier string associated with this entry
            // and convert it into an actual cgUID that we can use.
            query.getColumn( _T("Identifier"), identifierString );
            query.getColumn( _T("Name"), name );
            cgStringUtility::tryParse( identifierString, globalIdentifier );
            
            // Find this entry in the list of registered types if it exists.
            cgWorldObjectTypeDesc::Map::iterator itType = mObjectTypes.find( globalIdentifier );
            if ( itType == mObjectTypes.end() )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("The world database defined an unknown object type '%s' (%s). Objects of this type will be ignored during import.\n"), identifierString.c_str(), name.c_str() );
                continue;

            } // End if not found

            // Populate the remaining fields of the local object type structure.
            cgWorldObjectTypeDesc & typeDescription = itType->second;
            query.getColumn( _T("ObjectTypeId"), typeDescription.localIdentifier );
            typeDescription.name = name;

            // Add to the 'local->global' type identifier look up table.
            mObjectTypeLUT[ typeDescription.localIdentifier ] = typeDescription.globalIdentifier;

            // Mark component's type table as existing.
            mWorld->componentTablesCreated( typeDescription.globalIdentifier );
            
        } // Next Result
        query.reset();

    } // End if success

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadObjectSubElementTypeTable () (Protected)
/// <summary>
/// Build the local object sub-element type table by combining the list of
/// known registered object types defined by the application, and those defined
/// within the database. This new type list is specific to the opened
/// world.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::loadObjectSubElementTypeTable( )
{
    // Populate our local object type list with the list of types registered
    // by the application at start-up.
    mObjectSubElementTypes = cgObjectSubElement::getRegisteredTypes();

    // Query object sub-element types defined within the database.
    cgString error;
    cgWorldQuery query( mWorld, _T("SELECT * FROM ObjectSubElementTypes") );
    if ( query.getLastError( error ) == true )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to query world database for defined object sub-element types. Error: %s.\n"), error.c_str() );
        return false;
    
    } // End if failed.

    // Execute the query.
    if ( query.step() == true )
    {
        // Retrieve results.
        while( query.nextRow() )
        {
            cgUID    globalIdentifier;
            cgString identifierString, name;

            // Retrieve the global type identifier string associated with this entry
            // and convert it into an actual cgUID that we can use.
            query.getColumn( _T("Identifier"), identifierString );
            query.getColumn( _T("Name"), name );
            cgStringUtility::tryParse( identifierString, globalIdentifier );
            
            // Find this entry in our local type table if it exists.
            cgObjectSubElementTypeDesc::Map::iterator itType = mObjectSubElementTypes.find( globalIdentifier );
            if ( itType == mObjectSubElementTypes.end() )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("The world database defined an unknown object sub-element type '%s' (%s). Objects of this type will be ignored during import.\n"), identifierString.c_str(), name.c_str() );
                continue;

            } // End if not found

            // Populate the remaining fields of our local object sub-element type structure.
            cgObjectSubElementTypeDesc & typeDescription = itType->second;
            query.getColumn( _T("ObjectSubElementTypeId"), typeDescription.localIdentifier );
            typeDescription.name = name;

            // Add to the 'local->global' type identifier look up table.
            mObjectSubElementTypeLUT[ typeDescription.localIdentifier ] = typeDescription.globalIdentifier;

            // Mark component's type table as existing.
            mWorld->componentTablesCreated( typeDescription.globalIdentifier );
            
        } // Next Result
        query.reset();

    } // End if success

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadSceneElementTypeTable () (Protected)
/// <summary>
/// Build the local scene element type table by combining the list of known 
/// registered element types defined by the application, and those defined
/// within the database. This new type list is specific to the opened
/// world.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::loadSceneElementTypeTable( )
{
    // Populate our local element type list with the list of types registered
    // by the application at start-up.
    mSceneElementTypes = cgSceneElement::getRegisteredTypes();

    // Query scene element types defined within the database.
    cgString error;
    cgWorldQuery query( mWorld, _T("SELECT * FROM SceneElementTypes") );
    if ( query.getLastError( error ) == true )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to query world database for defined scene element types. Error: %s.\n"), error.c_str() );
        return false;
    
    } // End if failed.

    // Execute the query.
    if ( query.step() == true )
    {
        // Retrieve results.
        while( query.nextRow() )
        {
            cgUID    globalIdentifier;
            cgString identifierString, name;

            // Retrieve the global type identifier string associated with this entry
            // and convert it into an actual cgUID that we can use.
            query.getColumn( _T("Identifier"), identifierString );
            query.getColumn( _T("Name"), name );
            cgStringUtility::tryParse( identifierString, globalIdentifier );
            
            // Find this entry in our local type table if it exists.
            cgSceneElementTypeDesc::Map::iterator itType = mSceneElementTypes.find( globalIdentifier );
            if ( itType == mSceneElementTypes.end() )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("The world database defined an unknown scene element type '%s' (%s). Elements of this type will be ignored during import.\n"), identifierString.c_str(), name.c_str() );
                continue;

            } // End if not found

            // Populate the remaining fields of our local scene element type structure.
            cgSceneElementTypeDesc & typeDescription = itType->second;
            query.getColumn( _T("SceneElementTypeId"), typeDescription.localIdentifier );
            typeDescription.name = name;

            // Add to the 'local->global' type identifier look up table.
            mSceneElementTypeLUT[ typeDescription.localIdentifier ] = typeDescription.globalIdentifier;

            // Mark component's type table as existing.
            mWorld->componentTablesCreated( typeDescription.globalIdentifier );
            
        } // Next Result
        query.reset();

    } // End if success

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadSceneTable () (Private)
/// <summary>
/// Build the local list of scene descriptors populated from the
/// currently opened world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::loadSceneTable( )
{
    // Now we need to load information about each scene. Issue a custom query for this.
    cgString error;
    cgWorldQuery query( mWorld, _T("SELECT * FROM Scenes") );
    if ( query.getLastError( error ) == true )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to query world database for available scenes. Error: %s.\n"), error.c_str() );
        return false;
    
    } // End if failed.

    // Execute the query.
    if ( query.step() == true )
    {
        // Retrieve results.
        while( query.nextRow() )
        {
            // ToDo: move this logic into the descriptor itself.
            cgSceneDescriptor * sceneDescriptor = new cgSceneDescriptor();
            query.getColumn( _T("SceneId"), sceneDescriptor->sceneId );
            query.getColumn( _T("SceneType"), (cgUInt32&)sceneDescriptor->type );
            query.getColumn( _T("FriendlyName"), sceneDescriptor->friendlyName );
            query.getColumn( _T("EditorName"), sceneDescriptor->name );
            query.getColumn( _T("Description"), sceneDescriptor->description );
            query.getColumn( _T("Flags"), sceneDescriptor->flags );
            query.getColumn( _T("RenderControlScript"), sceneDescriptor->renderControl );
            query.getColumn( _T("DistanceDisplayUnits"), (cgUInt32&)sceneDescriptor->distanceDisplayUnits );
            query.getColumn( _T("BoundsMinX"), sceneDescriptor->sceneBounds.min.x );
            query.getColumn( _T("BoundsMinY"), sceneDescriptor->sceneBounds.min.y );
            query.getColumn( _T("BoundsMinZ"), sceneDescriptor->sceneBounds.min.z );
            query.getColumn( _T("BoundsMaxX"), sceneDescriptor->sceneBounds.max.x );
            query.getColumn( _T("BoundsMaxY"), sceneDescriptor->sceneBounds.max.y );
            query.getColumn( _T("BoundsMaxZ"), sceneDescriptor->sceneBounds.max.z );
            query.getColumn( _T("CellDimensionsX"), sceneDescriptor->cellDimensions.x );
            query.getColumn( _T("CellDimensionsY"), sceneDescriptor->cellDimensions.y );
            query.getColumn( _T("CellDimensionsZ"), sceneDescriptor->cellDimensions.z );
            query.getColumn( _T("LandscapeId"), sceneDescriptor->landscapeId );

            // Ignore if a scene with this name already exists.
            if ( mSceneDescriptorLUT.find( sceneDescriptor->name ) != mSceneDescriptorLUT.end() )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("A scene with the name '%s' already exists and has been ignored.\n"), sceneDescriptor->name.c_str() );
                continue;
            
            } // End if exists

            // Add the descriptor to our internal lists.
            mSceneDescriptors.push_back( sceneDescriptor );
            mSceneDescriptorLUT[ sceneDescriptor->name ] = mSceneDescriptors.size() - 1;
            
        } // Next Result
        query.reset();

    } // End if success

    // Any scene's found?
    if ( mSceneDescriptors.empty() )
    {
        cgAppLog::write( cgAppLog::Error, _T("World database did not contain any valid scenes or the scene descriptor query failed.\n") );
        return false;
    
    } // End if no scenes

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadMaterialPropertiesTable () (Private)
/// <summary>
/// Build the local list of material properties populated from the
/// currently opened world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::loadMaterialPropertiesTable( )
{
    // Now we need to load information about each scene. Issue a custom query for this.
    cgString error;
    cgWorldQuery query( mWorld, _T("SELECT * FROM 'Configuration::MaterialProperties'") );
    if ( query.getLastError( error ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to query world database for configured material properties. Error: %s.\n"), error.c_str() );
        return false;
    
    } // End if failed.

    // Execute the query.
    if ( query.step() )
    {
        // Retrieve results.
        while( query.nextRow() )
        {
            // Read property data.
            MaterialPropertyDesc propertyDescriptor;
            query.getColumn( _T("PropertyId"), propertyDescriptor.propertyId );
            query.getColumn( _T("Name"), propertyDescriptor.name );
            query.getColumn( _T("Identifier"), propertyDescriptor.identifier );
            query.getColumn( _T("Description"), propertyDescriptor.description );
            query.getColumn( _T("BitOffset"), propertyDescriptor.bitOffset );

            // Ignore if a property with this name already exists.
            bool isDuplicate = false;
            for ( size_t i = 0; i < mMaterialProperties.size(); ++i )
            {
                if ( mMaterialProperties[i].identifier == propertyDescriptor.identifier )
                {
                    cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("A user configured material property with the identifier '%s' already exists. All duplicate properties will be ignored when reading from '%s'.\n"), propertyDescriptor.identifier.c_str() );
                    isDuplicate = true;
                    break;

                } // End if duplicate
            
            } // End if exists

            // Add the descriptor to our internal lists.
            if ( !isDuplicate )
            {
                mMaterialProperties.push_back( propertyDescriptor );
                cgFilterExpression::Identifier filterIdentifier;
                filterIdentifier.name  = propertyDescriptor.identifier;
                filterIdentifier.value = (cgUInt64)1 << (cgUInt64)propertyDescriptor.bitOffset;
                mMaterialPropertyIdentifiers.push_back( filterIdentifier );

            } // End if !duplicated
            
        } // Next Result
        query.reset();

    } // End if success

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadRenderClassTable () (Private)
/// <summary>
/// Build the local list of object render class identifiers populated from the
/// currently opened world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::loadRenderClassTable( )
{
    // Now we need to load information about each scene. Issue a custom query for this.
    cgString error;
    cgWorldQuery query( mWorld, _T("SELECT * FROM 'Configuration::RenderClasses'") );
    if ( query.getLastError( error ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to query world database for configured material properties. Error: %s.\n"), error.c_str() );
        return false;
    
    } // End if failed.

    // Execute the query.
    if ( query.step() )
    {
        // Retrieve results.
        while( query.nextRow() )
        {
            // Read property data.
            RenderClassDesc classDescriptor;
            query.getColumn( _T("ClassId"), classDescriptor.classId );
            query.getColumn( _T("Identifier"), classDescriptor.identifier );
            query.getColumn( _T("Description"), classDescriptor.description );
            
            // Ignore if a class with this name already exists.
            bool isDuplicate = false;
            for ( size_t i = 0; i < mRenderClasses.size(); ++i )
            {
                if ( mRenderClasses[i].identifier == classDescriptor.identifier )
                {
                    cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("A user configured render class with the identifier '%s' already exists. All duplicate classes will be ignored while reading from '%s'.\n"), classDescriptor.identifier.c_str() );
                    isDuplicate = true;
                    break;

                } // End if duplicate
            
            } // End if exists

            // Add the descriptor to our internal lists.
            if ( !isDuplicate )
            {
                mRenderClassLUT[ classDescriptor.identifier ] = mRenderClasses.size();
                mRenderClasses.push_back( classDescriptor );
                
            } // End if !duplicated
            
        } // Next Result
        query.reset();

    } // End if success

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : insertMaterialProperty () (Protected)
/// <summary>
/// Insert the specified user customizable material property descriptor into 
/// the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::insertMaterialProperty( const cgString & name, const cgString & identifier, const cgString & description, cgInt bitOffset )
{
    // Build the property insert query.
    prepareQueries();
    mInsertMaterialProperty.bindParameter( 1, name );
    mInsertMaterialProperty.bindParameter( 2, identifier );
    mInsertMaterialProperty.bindParameter( 3, description );
    mInsertMaterialProperty.bindParameter( 4, bitOffset );

    // Execute the query.
    if ( !mInsertMaterialProperty.step( true ) )
    {
        cgString error;
        if ( mInsertMaterialProperty.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new user defined material property entry '%s' into the world database. Error: %s.\n"), error.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new user defined material property entry '%s' into the world database.\n") );
        return false;

    } // End if insert failed

    // Insert into the resident container.
    mMaterialProperties.resize( mMaterialProperties.size() + 1 );
    MaterialPropertyDesc & propertyDescriptor = mMaterialProperties.back();
    propertyDescriptor.propertyId  = mInsertMaterialProperty.getLastInsertId();
    propertyDescriptor.name        = name;
    propertyDescriptor.identifier  = identifier;
    propertyDescriptor.description = description;
    propertyDescriptor.bitOffset   = bitOffset;

    // Insert into the pre-cached identifier list.
    cgFilterExpression::Identifier filterIdentifier;
    filterIdentifier.name  = propertyDescriptor.identifier;
    filterIdentifier.value = (cgUInt64)1 << (cgUInt64)propertyDescriptor.bitOffset;
    mMaterialPropertyIdentifiers.push_back( filterIdentifier );

    // Success!
    return true;

}

//-----------------------------------------------------------------------------
//  Name : insertRenderClass () (Protected)
/// <summary>
/// Insert the specified user customizable object render class descriptor into 
/// the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::insertRenderClass( const cgString & identifier, const cgString & description )
{
    // Build the property insert query.
    prepareQueries();
    mInsertRenderClass.bindParameter( 1, identifier );
    mInsertRenderClass.bindParameter( 2, description );

    // Execute the query.
    if ( !mInsertRenderClass.step( true ) )
    {
        cgString error;
        if ( mInsertRenderClass.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new user defined object render class entry '%s' into the world database. Error: %s.\n"), error.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new user defined object render class entry '%s' into the world database.\n") );
        return false;

    } // End if insert failed

    // Insert into the resident container.
    mRenderClasses.resize( mRenderClasses.size() + 1 );
    RenderClassDesc & classDescriptor = mRenderClasses.back();
    classDescriptor.classId     = mInsertRenderClass.getLastInsertId();
    classDescriptor.identifier  = identifier;
    classDescriptor.description = description;
    
    // Insert into the pre-cached identifier list.
    mRenderClassLUT[identifier] = mRenderClasses.size() -1;
    
    // Success!
    return true;

}

//-----------------------------------------------------------------------------
// Name : insertScene () (Protected)
/// <summary>
/// Insert the specified scene descripter into the database if required. 
/// Returns 0 on failure.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorldConfiguration::insertScene( const cgSceneDescriptor & description )
{
    // Build the scene insert query.
    prepareQueries();
    mInsertScene.bindParameter( 1, (cgUInt32)description.type );
    mInsertScene.bindParameter( 2, cgString::Empty ); // FriendlyName
    mInsertScene.bindParameter( 3, description.name );
    mInsertScene.bindParameter( 4, description.description );
    mInsertScene.bindParameter( 5, description.flags );
    mInsertScene.bindParameter( 6, description.renderControl );
    mInsertScene.bindParameter( 7, (cgUInt16)description.distanceDisplayUnits );
    mInsertScene.bindParameter( 8, description.sceneBounds.min.x );
    mInsertScene.bindParameter( 9, description.sceneBounds.min.y );
    mInsertScene.bindParameter( 10, description.sceneBounds.min.z );
    mInsertScene.bindParameter( 11, description.sceneBounds.max.x );
    mInsertScene.bindParameter( 12, description.sceneBounds.max.y );
    mInsertScene.bindParameter( 13, description.sceneBounds.max.z );
    mInsertScene.bindParameter( 14, description.cellDimensions.x );
    mInsertScene.bindParameter( 15, description.cellDimensions.y );
    mInsertScene.bindParameter( 16, description.cellDimensions.z );
    mInsertScene.bindParameter( 17, description.landscapeId );

    // Execute the query.
    if ( !mInsertScene.step( true ) )
    {
        cgString error;
        if ( mInsertScene.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new scene '%s' into the world database. Error: %s.\n"), description.name.c_str(), error.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new scene '%s' into the world database.\n"), description.name.c_str() );
        return 0;

    } // End if insert failed
        
    // Retrieve the row identifier of the new scene.
    cgUInt32 sceneId = mInsertScene.getLastInsertId();

    // Create a new descriptor for this scene.
    cgSceneDescriptor * sceneDescriptor = new cgSceneDescriptor( description );
    sceneDescriptor->sceneId = sceneId;
    mSceneDescriptors.push_back( sceneDescriptor );

    // Add to the name based lookup table if one does not already exist 
    // for a scene with this name.
    if ( mSceneDescriptorLUT.find(sceneDescriptor->name) == mSceneDescriptorLUT.end() )
        mSceneDescriptorLUT[sceneDescriptor->name] = mSceneDescriptors.size() - 1;

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
cgUInt32 cgWorldConfiguration::generateRefId( )
{
    // Prepare statements if necessary.
    if ( !mIncrementNextRefId.isPrepared() )
        prepareQueries();

    // First statement retrieves the current 'next' identifier.
    if ( !mIncrementNextRefId.step() )
    {
        cgString error;
        mIncrementNextRefId.reset();
        if ( mIncrementNextRefId.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve next world reference identifier. Error: %s.\n"), error.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve next world reference identifier.\n") );

    } // End if failed

    // Retrieve the result.
    cgUInt32 nextId = 0;
    mIncrementNextRefId.getColumn( 0, nextId );

    // ToDo: Paranoia - warn / error if nextId gets >= 0x80000000
    // (reserved for internal reference identifiers)
    
    // Second statement increments.
    if ( !mIncrementNextRefId.step( true ) )
    {
        cgString error;
        if ( mIncrementNextRefId.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to increment next world reference identifier. Error: %s.\n"), error.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to increment next world reference identifier.\n") );

    } // End if failed

    // Return result.
    return nextId;
}

//-----------------------------------------------------------------------------
// Name : insertObjectType () (Protected)
/// <summary>
/// Insert the specified object type into the world configuration table(s).
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::insertObjectType( const cgUID & typeIdentifier, const cgString & databaseTable )
{
    // Find the data associated with the underlying object type for this identifier.
    cgWorldObjectTypeDesc::Map::iterator itType = mObjectTypes.find( typeIdentifier );
    if ( itType == mObjectTypes.end() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Attempted to create a new world database type entry for an object type that has not been registered by the application.\n") );
        return false;
    
    } // End if not previously inserted

    // Already exists in the database?
    if ( itType->second.localIdentifier > 0 )
        return true;
            
    // Insert information about this type into the database.
    prepareQueries();
    cgString identifierString = cgStringUtility::toString(typeIdentifier,_T("B"));
    mInsertObjectType.bindParameter( 1, identifierString );
    mInsertObjectType.bindParameter( 2, itType->second.name );
    mInsertObjectType.bindParameter( 3, databaseTable );
    if ( !mInsertObjectType.step( true ) )
    {
        cgString error;
        if ( mInsertObjectType.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert defined object type entry '%s' into world database. Error: %s.\n"), identifierString.c_str(), error.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert defined object type entry '%s' into world database.\n"), identifierString.c_str() );
        return false;
    
    } // End if failed

    // Retrieve last inserted identifier. This is the database 'local' type identifier for this object type.
    itType->second.localIdentifier = mInsertObjectType.getLastInsertId();

    // Add to the 'local->global' type identifier look up table.
    mObjectTypeLUT[ itType->second.localIdentifier ] = typeIdentifier;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : insertObjectSubElementType () (Protected)
/// <summary>
/// Insert the specified object sub-element type into the world configuration 
/// table(s).
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::insertObjectSubElementType( const cgUID & typeIdentifier, const cgString & databaseTable )
{
    // Find the data associated with the underlying object type for this identifier.
    cgObjectSubElementTypeDesc::Map::iterator itType = mObjectSubElementTypes.find( typeIdentifier );
    if ( itType == mObjectSubElementTypes.end() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Attempted to create a new world database type entry for an object sub-element type that has not been registered by the application.\n") );
        return false;
    
    } // End if not previously inserted

    // Already exists in the database?
    if ( itType->second.localIdentifier > 0 )
        return true;
            
    // Insert information about this type into the database.
    prepareQueries();
    cgString identifierString = cgStringUtility::toString(typeIdentifier,_T("B"));
    mInsertObjectSubElementType.bindParameter( 1, identifierString );
    mInsertObjectSubElementType.bindParameter( 2, itType->second.name );
    mInsertObjectSubElementType.bindParameter( 3, databaseTable );
    if ( !mInsertObjectSubElementType.step( true ) )
    {
        cgString strError;
        if ( mInsertObjectSubElementType.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert defined object sub-element type data '%s' into world database. Error: %s.\n"), identifierString.c_str(), strError.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert defined object sub-element type data '%s' into world database.\n"), identifierString.c_str() );
        return false;
    
    } // End if failed
    
    // Retrieve last inserted identifier. This is the database 'local' type identifier for this element type.
    itType->second.localIdentifier = mInsertObjectSubElementType.getLastInsertId();

    // Add to the 'local->global' type identifier look up table.
    mObjectSubElementTypeLUT[ itType->second.localIdentifier ] = typeIdentifier;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : insertSceneElementType () (Protected)
/// <summary>
/// Insert the specified scene element type into the world configuration 
/// table(s).
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::insertSceneElementType( const cgUID & typeIdentifier, const cgString & databaseTable )
{
    // Find the data associated with the underlying element type for this identifier.
    cgSceneElementTypeDesc::Map::iterator itType = mSceneElementTypes.find( typeIdentifier );
    if ( itType == mSceneElementTypes.end() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Attempted to create a new world database type entry for a scene element type that has not been registered by the application.\n") );
        return false;
    
    } // End if not previously inserted

    // Already exists in the database?
    if ( itType->second.localIdentifier > 0 )
        return true;
            
    // Insert information about this type into the database.
    prepareQueries();
    cgString identifierString = cgStringUtility::toString(typeIdentifier,_T("B"));
    mInsertSceneElementType.bindParameter( 1, identifierString );
    mInsertSceneElementType.bindParameter( 2, itType->second.name );
    mInsertSceneElementType.bindParameter( 3, databaseTable );
    if ( !mInsertSceneElementType.step( true ) )
    {
        cgString strError;
        if ( mInsertSceneElementType.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert defined scene element type data '%s' into world database. Error: %s.\n"), identifierString.c_str(), strError.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert defined scene element type data '%s' into world database.\n"), identifierString.c_str() );
        return false;
    
    } // End if failed
    
    // Retrieve last inserted identifier. This is the database 'local' type identifier for this element type.
    itType->second.localIdentifier = mInsertSceneElementType.getLastInsertId();

    // Add to the 'local->global' type identifier look up table.
    mSceneElementTypeLUT[ itType->second.localIdentifier ] = typeIdentifier;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSceneCount ()
/// <summary>
/// Retrieve the total number of scenes contained within the loaded
/// world configuration.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorldConfiguration::getSceneCount( ) const
{
    return (cgUInt32)mSceneDescriptors.size();
}

//-----------------------------------------------------------------------------
//  Name : getSceneDescriptor ()
/// <summary>
/// Retrieve the physical descriptor structure for a scene as it was
/// described by the world configuration. This particular method allows the
/// caller to retrieve a descriptor by index.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneDescriptor * cgWorldConfiguration::getSceneDescriptor( cgUInt32 index ) const
{
    // Out of range?
    if ( index >= mSceneDescriptors.size() )
        return CG_NULL;

    // Return the descriptor at this location
    return mSceneDescriptors[ index ];
}

//-----------------------------------------------------------------------------
//  Name : getSceneDescriptorByName ()
/// <summary>
/// Retrieve the physical descriptor structure for a scene as it was
/// described by the world configuration. This particular method allows the
/// caller to retrieve a descriptor using the name of the scene.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneDescriptor * cgWorldConfiguration::getSceneDescriptorByName( const cgString & sceneName ) const
{
    // Descriptor with this name exists?
    SceneDescriptorLUT::const_iterator itDesc = mSceneDescriptorLUT.find( sceneName );
    if ( itDesc == mSceneDescriptorLUT.end() )
        return CG_NULL;

    // Return descriptor pointer
    return mSceneDescriptors[itDesc->second];
}

//-----------------------------------------------------------------------------
//  Name : getSceneDescriptorById ()
/// <summary>
/// Retrieve the physical descriptor structure for a scene as it was described
/// by the world configuration. This particular method allows the caller to 
/// retrieve a descriptor using the unique identifier of the scene.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneDescriptor * cgWorldConfiguration::getSceneDescriptorById( cgUInt32 sceneId ) const
{
    for ( size_t i = 0 ; i < mSceneDescriptors.size(); ++i )
    {
        if ( mSceneDescriptors[i]->sceneId == sceneId )
            return mSceneDescriptors[i];
    
    } // Next Descriptor
    
    // Nothing found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : updateSceneDescriptorById ()
/// <summary>
/// Update the scene descriptor for the scene with the specified identifier.
/// Will also update the database where appropriate.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldConfiguration::updateSceneDescriptorById( cgUInt32 sceneId, const cgSceneDescriptor & description )
{
    // First get the current descriptor for this scene (if one exists).
    size_t sceneIndex = size_t(-1);
    for ( size_t i = 0 ; i < mSceneDescriptors.size(); ++i )
    {
        if ( mSceneDescriptors[i]->sceneId == sceneId )
        {
            sceneIndex = i;
            break;
        
        } // End if match

    } // Next Descriptor

    // Cannot update a scene that does not currently exist.
    if ( sceneIndex == size_t(-1) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to update scene descriptor because no existing scene with the id '0x%x' could be found.\n"), sceneId );
        return false;
    
    } // End if no existing scene

    // Build the update query.
    prepareQueries();
    mUpdateScene.bindParameter( 1, (cgUInt32)description.type );
    mUpdateScene.bindParameter( 2, cgString::Empty ); // FriendlyName
    mUpdateScene.bindParameter( 3, description.name );
    mUpdateScene.bindParameter( 4, description.description );
    mUpdateScene.bindParameter( 5, description.flags );
    mUpdateScene.bindParameter( 6, description.renderControl );
    mUpdateScene.bindParameter( 7, (cgUInt16)description.distanceDisplayUnits );
    mUpdateScene.bindParameter( 8, description.sceneBounds.min.x );
    mUpdateScene.bindParameter( 9, description.sceneBounds.min.y );
    mUpdateScene.bindParameter( 10, description.sceneBounds.min.z );
    mUpdateScene.bindParameter( 11, description.sceneBounds.max.x );
    mUpdateScene.bindParameter( 12, description.sceneBounds.max.y );
    mUpdateScene.bindParameter( 13, description.sceneBounds.max.z );
    mUpdateScene.bindParameter( 14, description.cellDimensions.x );
    mUpdateScene.bindParameter( 15, description.cellDimensions.y );
    mUpdateScene.bindParameter( 16, description.cellDimensions.z );
    mUpdateScene.bindParameter( 17, description.landscapeId );
    mUpdateScene.bindParameter( 18, sceneId );

    // Execute the query.
    if ( !mUpdateScene.step( true ) )
    {
        cgString error;
        if ( mUpdateScene.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to update the descriptor for scene '%s' (0x%x). Error: %s.\n"), description.name.c_str(), sceneId, error.c_str() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to update the descriptor for scene '%s' (0x%x).\n"), description.name.c_str(), sceneId );
        return false;

    } // End if insert failed

    // Update the internal descriptor in the descriptor table.
    *mSceneDescriptors[sceneIndex] = description;

    // Scene Id cannot be modified. Overwrite whatever was provided with the original.
    mSceneDescriptors[sceneIndex]->sceneId = sceneId;

    // If the old and new names are different, regenerate the descriptor 
    // name lookup table.
    if ( mSceneDescriptors[sceneIndex]->name != description.name )
    {
        mSceneDescriptorLUT.clear();
        for ( size_t i = 0; i < mSceneDescriptors.size(); ++i )
        {
            // Ignore if a scene with this name already exists.
            const cgSceneDescriptor * sceneDescriptor = mSceneDescriptors[i];
            if ( mSceneDescriptorLUT.find( sceneDescriptor->name ) != mSceneDescriptorLUT.end() )
                continue;

            // Add the descriptor index to the LUT
            mSceneDescriptorLUT[ sceneDescriptor->name ] = i;

        } // Next scene

    } // End if changed

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getParentWorld ()
/// <summary>
/// Retrieve the world to which this configuration applies.
/// </summary>
//-----------------------------------------------------------------------------
cgWorld * cgWorldConfiguration::getParentWorld( ) const
{
    return mWorld;
}

//-----------------------------------------------------------------------------
//  Name : getObjectType ()
/// <summary>
/// Retrieve details for the specified object type based on its global 
/// identifier.
/// </summary>
//-----------------------------------------------------------------------------
const cgWorldObjectTypeDesc * cgWorldConfiguration::getObjectType( const cgUID & globalIdentifier ) const
{
    cgWorldObjectTypeDesc::Map::const_iterator itType = mObjectTypes.find( globalIdentifier );
    if ( itType == mObjectTypes.end() )
        return CG_NULL;
    return &itType->second;
}

//-----------------------------------------------------------------------------
//  Name : getObjectTypeByLocalId ()
/// <summary>
/// Retrieve details for the specified object type based on its local (database)
/// identifier.
/// </summary>
//-----------------------------------------------------------------------------
const cgWorldObjectTypeDesc * cgWorldConfiguration::getObjectTypeByLocalId( cgUInt32 localIdentifier ) const
{
    ComponentTypeLUT::const_iterator itType = mObjectTypeLUT.find( localIdentifier );
    if ( itType == mObjectTypeLUT.end() )
        return CG_NULL;
    return getObjectType( itType->second );
}

//-----------------------------------------------------------------------------
//  Name : getObjectSubElementType ()
/// <summary>
/// Retrieve details for the specified object sub-element type based on its
/// global identifier.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectSubElementTypeDesc * cgWorldConfiguration::getObjectSubElementType( const cgUID & globalIdentifier ) const
{
    cgObjectSubElementTypeDesc::Map::const_iterator itType = mObjectSubElementTypes.find( globalIdentifier );
    if ( itType == mObjectSubElementTypes.end() )
        return CG_NULL;
    return &itType->second;
}

//-----------------------------------------------------------------------------
//  Name : getObjectSubElementTypeByLocalId ()
/// <summary>
/// Retrieve details for the specified object sub-element type based on its
/// local (database) identifier.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectSubElementTypeDesc * cgWorldConfiguration::getObjectSubElementTypeByLocalId( cgUInt32 localIdentifier ) const
{
    ComponentTypeLUT::const_iterator itType = mObjectSubElementTypeLUT.find( localIdentifier );
    if ( itType == mObjectSubElementTypeLUT.end() )
        return CG_NULL;
    return getObjectSubElementType( itType->second );
}

//-----------------------------------------------------------------------------
//  Name : getSceneElementType ()
/// <summary>
/// Retrieve details for the specified scene element type based on its global
/// identifier.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneElementTypeDesc * cgWorldConfiguration::getSceneElementType( const cgUID & globalIdentifier ) const
{
    cgSceneElementTypeDesc::Map::const_iterator itType = mSceneElementTypes.find( globalIdentifier );
    if ( itType == mSceneElementTypes.end() )
        return CG_NULL;
    return &itType->second;
}

//-----------------------------------------------------------------------------
//  Name : getSceneElementTypeByLocalId ()
/// <summary>
/// Retrieve details for the specified scene element type based on its local 
/// (database) identifier.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneElementTypeDesc * cgWorldConfiguration::getSceneElementTypeByLocalId( cgUInt32 localIdentifier ) const
{
    ComponentTypeLUT::const_iterator itType = mSceneElementTypeLUT.find( localIdentifier );
    if ( itType == mSceneElementTypeLUT.end() )
        return CG_NULL;
    return getSceneElementType( itType->second );
}

//-----------------------------------------------------------------------------
//  Name : getVersion ()
/// <summary>
/// Get the database layout version code as specified within the opened world
/// database. Use 'cgDecomposeVersion()' to separate the version code into its
/// 'major' version, subversion and revision components.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorldConfiguration::getVersion( ) const
{
    return mVersion;
}

//-----------------------------------------------------------------------------
//  Name : getLayoutStatus ()
/// <summary>
/// When the database configuration is loaded, version testing is performed
/// and the layout of the database is potentially updated. The layout status
/// returned by this method indicates the action (if any) that was taken during
/// this process when the configuration was first loaded.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldDatabaseStatus::Base cgWorldConfiguration::getLayoutStatus() const
{
    return mLayoutStatus;
}

//-----------------------------------------------------------------------------
//  Name : getMaterialPropertyIdentifiers ()
/// <summary>
/// Retrieve the pre-constructed list of identifiers (compatible with 
/// cgFilterExpression) for the various user defined material properties types.
/// </summary>
//-----------------------------------------------------------------------------
const cgFilterExpression::IdentifierArray & cgWorldConfiguration::getMaterialPropertyIdentifiers( ) const
{
    return mMaterialPropertyIdentifiers;
}

//-----------------------------------------------------------------------------
//  Name : getMaterialProperties ()
/// <summary>
/// Retrieve the pre-constructed list of the various user defined material 
/// properties types / styles.
/// </summary>
//-----------------------------------------------------------------------------
const cgWorldConfiguration::MaterialPropertyArray & cgWorldConfiguration::getMaterialProperties( ) const
{
    return mMaterialProperties;
}

//-----------------------------------------------------------------------------
//  Name : getRenderClassId ()
/// <summary>
/// Retrieve the integer identifier associated with the specified user defined
/// render class name string.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorldConfiguration::getRenderClassId( const cgString & className ) const
{
    RenderClassLUT::const_iterator itClass = mRenderClassLUT.find( className );
    if ( itClass == mRenderClassLUT.end() )
        return 0;
    return mRenderClasses[itClass->second].classId;
}