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
// File : cgWorldConfiguration.h                                             //
//                                                                           //
// Desc : Class that maintains and updates information about how the         //
//        currently loaded world was created and configured. Such            //
//        information includes version information, reference identifier     //
//        details, registered object types, material property identifiers,   //
//        and more.                                                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWORLDCONFIGURATION_H_ )
#define _CGE_CGWORLDCONFIGURATION_H_

//-----------------------------------------------------------------------------
// cgWorldConfiguration Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldQuery.h>
#include <System/cgFilterExpression.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgWorld;
class cgSceneDescriptor;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgWorldConfiguration (Class)
/// <summary>
/// Class that maintains and updates information about how the currently loaded 
/// world was created and configured. Such information includes version 
/// information, reference identifier details, registered object types and
/// material property identifiers, and more.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWorldConfiguration
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgWorld;

public:
    //-------------------------------------------------------------------------
    // Public Structures & Typedefs
    //-------------------------------------------------------------------------
    // Structure responsible for storing information about user defined material property bits.
    struct MaterialPropertyDesc
    {
        cgUInt32    propertyId;     // Database record identifier.
        cgString    name;           // Friendly name of this material property (i.e. the one displayed in the UI).
        cgString    identifier;     // String identifier for this property used in "code" for material filtering.
        cgString    description;    // Description of this property (what it does, and how it is to be used).
        cgInt       bitOffset;      // Property value bit (0-63).
    };
    CGE_ARRAY_DECLARE(MaterialPropertyDesc, MaterialPropertyArray)

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgWorldConfiguration( cgWorld * world );
    ~cgWorldConfiguration( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgWorld                                   * getParentWorld                  ( ) const;
    cgUInt32                                    getVersion                      ( ) const;
    cgUInt32                                    getSceneCount                   ( ) const;
    cgWorldDatabaseStatus::Base                 getLayoutStatus                 ( ) const;
    const cgSceneDescriptor                   * getSceneDescriptor              ( cgUInt32 index ) const;
    const cgSceneDescriptor                   * getSceneDescriptorById          ( cgUInt32 sceneId ) const;
    const cgSceneDescriptor                   * getSceneDescriptorByName        ( const cgString & sceneName ) const;
    const cgWorldObjectTypeDesc               * getObjectType                   ( const cgUID & globalIdentifier ) const;
    const cgWorldObjectTypeDesc               * getObjectTypeByLocalId          ( cgUInt32 localIdentifier ) const;
    const cgObjectSubElementTypeDesc          * getObjectSubElementType         ( const cgUID & globalIdentifier ) const;
    const cgObjectSubElementTypeDesc          * getObjectSubElementTypeByLocalId( cgUInt32 localIdentifier ) const;
    const cgSceneElementTypeDesc              * getSceneElementType             ( const cgUID & globalIdentifier ) const;
    const cgSceneElementTypeDesc              * getSceneElementTypeByLocalId    ( cgUInt32 localIdentifier ) const;
    const cgFilterExpression::IdentifierArray & getMaterialPropertyIdentifiers  ( ) const;
    const MaterialPropertyArray               & getMaterialProperties           ( ) const;
    cgUInt32                                    getRenderClassId                ( const cgString & className ) const;
    bool                                        updateSceneDescriptorById       ( cgUInt32 sceneId, const cgSceneDescriptor & sceneDescriptor );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    // Structure responsible for storing information about user defined render classes.
    struct RenderClassDesc
    {
        cgUInt32    classId;        // Database record identifier.
        cgString    identifier;     // String identifier for this render class used in "code" for object filtering.
        cgString    description;    // Description of this property (what it does, and how it is to be used).
    };

    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_ARRAY_DECLARE       (RenderClassDesc, RenderClassArray)
    CGE_ARRAY_DECLARE       (cgSceneDescriptor*, SceneDescriptorArray)
    CGE_UNORDEREDMAP_DECLARE(cgString, size_t, SceneDescriptorLUT)
    CGE_UNORDEREDMAP_DECLARE(cgUInt32, cgUID, ComponentTypeLUT )
    CGE_UNORDEREDMAP_DECLARE(cgString, cgUInt32, RenderClassLUT )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    // cgWorld interface
    cgUInt32                generateRefId                   ( );
    bool                    newConfiguration                ( cgWorldType::Base type );
    cgConfigResult::Base    loadConfiguration               ( cgUInt32 minSupportedVersion = 0, cgUInt32 maxSupportedVersion = 0xFFFFFFFF, bool autoUpgrade = false );
    cgUInt32                insertScene                     ( const cgSceneDescriptor & description );
    bool                    insertMaterialProperty          ( const cgString & name, const cgString & identifier, const cgString & description, cgInt bitOffset );
    bool                    insertRenderClass               ( const cgString & identifier, const cgString & description );
    bool                    insertObjectType                ( const cgUID & typeIdentifier, const cgString & databaseTable );
    bool                    insertObjectSubElementType      ( const cgUID & typeIdentifier, const cgString & databaseTable );
    bool                    insertSceneElementType          ( const cgUID & typeIdentifier, const cgString & databaseTable );
	bool					removeScene						( cgUInt32 sceneId );

    // Internal
    bool                    loadSceneTable                  ( );
    bool                    loadObjectTypeTable             ( );
    bool                    loadObjectSubElementTypeTable   ( );
    bool                    loadSceneElementTypeTable       ( );
    bool                    loadMaterialPropertiesTable     ( );
    bool                    loadRenderClassTable            ( );
    void                    prepareQueries                  ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgWorld                           * mWorld;                         // Parent world described by this configuration.
    cgWorldType::Base                   mWorldType;                     // The type of world file opened / being managed.
    cgUInt32                            mVersion;                       // World database version number.
    cgWorldDatabaseStatus::Base         mLayoutStatus;                  // Was the database layout upgraded during configuration load?

    cgWorldObjectTypeDesc::Map          mObjectTypes;                   // Local copy of the type descriptor map with database specific entries completed.
    ComponentTypeLUT                    mObjectTypeLUT;                 // Look up table that allows us to convert the local 'integer' type identifier into its UID counterpart.
    cgObjectSubElementTypeDesc::Map     mObjectSubElementTypes;         // Local copy of the sub-element type descriptor map with database specific entries completed.
    ComponentTypeLUT                    mObjectSubElementTypeLUT;       // Look up table that allows us to convert the local 'integer' type identifier into its UID counterpart.
    cgSceneElementTypeDesc::Map         mSceneElementTypes;             // Local copy of the scene element type descriptor map with database specific entries completed.
    ComponentTypeLUT                    mSceneElementTypeLUT;           // Look up table that allows us to convert the local 'integer' type identifier into its UID counterpart.

    SceneDescriptorArray                mSceneDescriptors;              // Vector containing pointers to all loaded scene descriptors.
    SceneDescriptorLUT                  mSceneDescriptorLUT;            // Indexes into above array based on the name of the scene.

    MaterialPropertyArray               mMaterialProperties;            // Array of all user defined material properties (alpha tested, forward rendered, refractive, etc.)
    cgFilterExpression::IdentifierArray mMaterialPropertyIdentifiers;   // Pre-constructed list of identifiers (compatible with cgFilterExpression) for the various defined material properties types.

    RenderClassArray                    mRenderClasses;                 // Array of all user defined render class identifiers.
    RenderClassLUT                      mRenderClassLUT;                // Look up table for configured object render class identifiers to elements in the above array.

private:
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgWorldQuery         mIncrementNextRefId;            // Increment the next unique reference identifier value stored in the configuration table.
    static cgWorldQuery         mGetObjectType;                 // Select information about the specified object from 'ObjectTypes' table.
    static cgWorldQuery         mGetObjectSubElementType;       // Select information about the specified object sub-element from 'ObjectSubElementTypes' table.
    static cgWorldQuery         mGetSceneElementType;           // Select information about the specified scene element from 'SceneElementTypes' table.
    static cgWorldQuery         mInsertObjectType;              // Insert new information about an object into the 'ObjectTypes' table.
    static cgWorldQuery         mInsertObjectSubElementType;    // Insert new information about an object sub-element into the 'ObjectSubElementTypes' table.
    static cgWorldQuery         mInsertSceneElementType;        // Insert new information about a scene element into the 'SceneElementTypes' table.
    static cgWorldQuery         mInsertMaterialProperty;        // Insert new information about a user-defined material property into the 'Configuration::MaterialProperties' table.
    static cgWorldQuery         mInsertRenderClass;             // Insert new information about a user-defined object render class into the 'Configuration::RenderClasses' table.
    static cgWorldQuery         mInsertScene;                   // Insert a new scene descriptor into the database.
	static cgWorldQuery         mDeleteScene;                   // Remove the specified scene descriptor from the database.
    static cgWorldQuery         mUpdateScene;                   // Update an existing scene descriptor within the database.

}; // End Class cgWorldConfiguration

#endif // !_CGE_CGWORLDCONFIGURATION_H_