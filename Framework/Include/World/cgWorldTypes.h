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
// Name : cgWorldTypes.h                                                     //
//                                                                           //
// Desc : Common system file that defines various world types and common     //
//        enumerations.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWORLDTYPES_H_ )
#define _CGE_CGWORLDTYPES_H_

//-----------------------------------------------------------------------------
// cgWorldTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgWorld;
class cgWorldObject;
class cgScene;
class cgObjectNode;
class cgObjectSubElement;
class cgTransform;

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
namespace cgVisibilitySearchFlags
{
    enum Base
    {
        MustRender          = 0x1,
        MustCastShadows     = 0x2,
        CollectMaterials    = 0x100
    };

}; // End Namespace : cgVisibilitySearchFlags

// Describes the stage at which certain scene processes have been
// or are to be performed (i.e. a given light precomputed its lighting, etc.)
namespace cgSceneProcessStage
{
    enum Base
    {
        None          = 0,
        Precomputed   = 1,
        Runtime       = 2,
        Both          = 3
    };

}; // End Namespace : cgSceneProcessStage

// Defines the specific rendering process that is currently being undertaken and
// allows the script (for instance) to take the appropriate action.
namespace cgSceneRenderContext
{
    enum Base
    {
        Runtime         = 0,
        SandboxRender   = 1,
        SandboxMaterial = 2

    };

}; // End Namespace : cgSceneRenderContext

// Contains items which describe how often to perform an update (i.e. on an object)
namespace cgUpdateRate
{
    enum Base
    {
        Never = 0,
        Always,
        FPS1,
        FPS2,
        FPS5,
        FPS10,
        FPS15,
        FPS20,
        FPS30,
        FPS40,
        FPS50,
        FPS60,
        FPS100,
        FPS120,
        NTSC,      // 29.97fps
        PAL,       // 25fps
        Count      // Number of supported interval types
    }; 

}; // End Namespace : cgUpdateRate

// Describes the type of world file being opened.
namespace cgWorldType
{
    enum Base
    {
        Master = 0,
        Merge,
        Data
    };

}; // End Namespace : cgWorldType

// Describes the type of the scene file being managed.
namespace cgSceneType
{
    enum Base
    {
        Standard = 0,
        Exterior,
        Construct
    };

}; // End Namespace : cgSceneType

// Describes the type of the node being managed.
namespace cgNodeType
{
    enum Base
    {
        Persistent = 0,
        Transient  = 1,
        Distant    = 2
    };

}; // End Namespace : cgNodeType

// Describes the method that can be used for cloning objects / nodes.
namespace cgCloneMethod
{
    enum Base
    {
        None = 0,
        Copy,
        ObjectInstance,
        DataInstance
    };

}; // End Namespace : cgCloneMethod

// Describes in which space an operation should occur (i.e. world or local)
namespace cgOperationSpace
{
    enum Base
    {
        World = 0,
        Local

    };

}; // End Namespace : cgOperationSpace

// Specifies the projection mode of the camera, viewport or process (orthgraphic / perspective)
namespace cgProjectionMode
{
    enum Base
    {
        Perspective     = 0,
        Orthographic    = 1
    
    };

}; // End Namespace : cgProjectionMode

// Describes how manipulations in the position of a target node will be interpreted.
namespace cgNodeTargetMethod
{
    enum Base
    {
        NoTarget    = 0,    // Destroy target.
        XAxis       = 1,    // Link to X axis.
        YAxis       = 2,    // Link to Y axis.
        ZAxis       = 3,    // Link to Z axis.
        Unlocked    = 4,    // Do not update.
        Relative    = 5     // Axis defined by target and node's relative location.
    };

}; // End Namespace : cgNodeTargetMethod

namespace cgTransformMethod
{
    enum Base
    {
        Standard        = 0,    // Adjust transforms using normal behavior.
        PivotOnly       = 1,    // Adjust only the pivot transform, leaving the object in place.
        ObjectOnly      = 2,    // Adjust only the object transform, leaving the pivot in place.
        NoChildUpdate   = 3     // Do not transform hierarchy children to match.
    };

}; // End Namespace : cgTransformMethod

namespace cgTransformSource
{
    enum Base
    {
        Standard            = 0,
        Dynamics            = 1,
        TransformResolve    = 2
    };

}; // End Namespace : cgTransformSource

// Operations that need to be performed to fully resolve an object after updates have occurred.
namespace cgDeferredUpdateFlags
{
    enum Base
    {
        BoundingBox              = 0x1,         // World space bounding box needs to be re-computed
        OwnershipStatus          = 0x2,         // Node needs to be inserted into the relevant spatial tree / cell grid
        Transforms               = 0x4,         // World space transforms must be recomputed.
        All                      = 0x7FFFFFFF
    };

}; // End Namespace : cgDeferredUpdateFlags

//-----------------------------------------------------------------------------
// Common Global Constants
//-----------------------------------------------------------------------------
// System defined object sub-element category identifiers.
// {FF337EAC-C68E-4C21-8445-3A5B042A82EF}
const cgUID OSECID_CollisionShapes = {0xFF337EAC, 0xC68E, 0x4C21, {0x84, 0x45, 0x3A, 0x5B, 0x4, 0x2A, 0x82, 0xEF}};

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct cgObjectSubElementDesc
{
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE      (cgObjectSubElementDesc, Array)
    CGE_UNORDEREDMAP_DECLARE(cgUID, cgObjectSubElementDesc, Map)

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    /// <summary>Unique reference type identifier for this element.</summary>
    cgUID       identifier;
    /// <summary>Friendly name for the sub element.</summary>
    cgString    name;
    
}; // End Struct : cgObjectSubElementDesc

struct cgObjectSubElementCategory
{
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE      (cgObjectSubElementCategory, Array)
    CGE_UNORDEREDMAP_DECLARE(cgUID, cgObjectSubElementCategory, Map)

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    /// <summary>Unique identifier for this category.</summary>
    cgUID                           identifier;
    /// <summary>Friendly name for the sub element category.</summary>
    cgString                        name;
    /// <summary>Resource type identifiers for all supported types in this category (for this object).</summary>
    cgObjectSubElementDesc::Map     supportedTypes;
    /// <summary>New sub-elements in this category can be created.</summary>
    bool                            canCreate;
    /// <summary>Existing sub-elements in this category can be deleted.</summary>
    bool                            canDelete;
    /// <summary>Items in this category can be enumerated (requested) through the parent object.</summary>
    bool                            canEnumerate;
    
}; // End Struct : cgObjectSubElementCategory

// Structure responsible for storing information about registered object types.
struct cgWorldObjectTypeDesc
{
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    // Function pointers
    typedef cgWorldObject * (*ObjectAllocNewFunc)( const cgUID &, cgUInt32, cgWorld * );
    typedef cgWorldObject * (*ObjectAllocCloneFunc)( const cgUID &, cgUInt32, cgWorld *, cgWorldObject *, cgCloneMethod::Base );
    typedef cgObjectNode  * (*NodeAllocNewFunc)( const cgUID &, cgUInt32, cgScene * );
    typedef cgObjectNode  * (*NodeAllocCloneFunc)( const cgUID &, cgUInt32, cgScene *, cgObjectNode*, cgCloneMethod::Base, const cgTransform& );
    
    // Containers
    CGE_UNORDEREDMAP_DECLARE(cgUID, cgWorldObjectTypeDesc, Map)

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    /// <summary>Friendly name of the type used for debug purposes only.</summary>
    cgString                name;               
    /// <summary>Globally unique identifier for this object type.</summary>
    cgUID                   globalIdentifier;
    /// <summary>Integer identifier of the type as it exists in the world database.</summary>
    cgUInt32                localIdentifier;
    /// <summary>Allocation function for the creation of a new object.</summary>
    ObjectAllocNewFunc      objectAllocNew;
    /// <summary>Allocation function for the creation of a cloned object.</summary>
    ObjectAllocCloneFunc    objectAllocClone;
    /// <summary>Allocation function for the creation of a new node used to contain the object.</summary>
    NodeAllocNewFunc        nodeAllocNew;
    /// <summary>Allocation function for the creation of a cloned node used to contain the object.</summary>
    NodeAllocCloneFunc      nodeAllocClone;

}; // End Struct cgWorldObjectTypeDesc

// Structure responsible for storing information about registered object sub-element types.
struct cgObjectSubElementTypeDesc
{
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    // Function pointers
    typedef cgObjectSubElement* (*AllocNewFunc)( const cgUID &, cgUInt32, cgWorldObject * );
    typedef cgObjectSubElement* (*AllocCloneFunc)( const cgUID &, cgUInt32, cgWorldObject *, cgObjectSubElement * );
    
    // Containers
    CGE_UNORDEREDMAP_DECLARE(cgUID, cgObjectSubElementTypeDesc, Map)

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    /// <summary>Friendly name of the type used for debug purposes only.</summary>
    cgString        name;
    /// <summary>Globally unique identifier for this object sub-element type.</summary>
    cgUID           globalIdentifier;
    /// <summary>Integer identifier of the type as it exists in the database.</summary>
    cgUInt32        localIdentifier;
    /// <summary>Allocation function for the creation of a new object sub element.</summary>
    AllocNewFunc    elementAllocNew;
    /// <summary>Allocation function for the creation of a cloned object sub element.</summary>
    AllocCloneFunc  elementAllocClone;

}; // End Struct cgObjectSubElementTypeDesc

#endif // !_CGE_CGWORLDTYPES_H_