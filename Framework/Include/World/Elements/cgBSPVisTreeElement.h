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
// Name : cgBSPVisTreeElement.h                                              //
//                                                                           //
// Desc : Class that provides configuration and management of BSP based      //
//        scene visibility data, exposed as a scene element type. This       //
//        provides the integration between the application (such as the      //
//        editing environment) and the relevant components of the navigation //
//        system.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBSPVISTREEELEMENT_H_ )
#define _CGE_CGBSPVISTREEELEMENT_H_

//-----------------------------------------------------------------------------
// cgBSPVisTreeElement Header Includes
//-----------------------------------------------------------------------------
#include <World/cgSceneElement.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBSPTree;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {78DD8B02-34A7-4AE1-8DDD-38BEB2267906}
const cgUID RTID_BSPVisTreeElement = { 0x78dd8b02, 0x34a7, 0x4ae1, { 0x8d, 0xdd, 0x38, 0xbe, 0xb2, 0x26, 0x79, 0x6 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBSPVisTreeElement (Class)
/// <summary>
/// Class that provides configuration and management of BSP based scene 
/// visibility data, exposed as a scene element type. This provides the 
/// integration between the application (such as the editing environment) and 
/// the relevant components of the navigation system.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBSPVisTreeElement : public cgSceneElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBSPVisTreeElement, cgSceneElement, "BSPVisTreeElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBSPVisTreeElement( cgUInt32 referenceId, cgScene * scene );
    virtual ~cgBSPVisTreeElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgSceneElement     * allocateNew                 ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        addOcclusionGeometryId      ( cgUInt32 meshNodeRefId );
    void                        clearOcclusionGeometry      ( );
    const cgUInt32Array       & getOcclusionGeometryIds     ( ) const;
    bool                        buildTree                   ( cgBSPTree * tree );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSceneElement)
    //-------------------------------------------------------------------------
    virtual void                sandboxRender               ( cgUInt32 flags, cgCameraNode * camera );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual cgString            getDatabaseTable            ( ) const;
    virtual bool                onComponentCreated          ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading          ( cgComponentLoadingEventArgs * e );
    virtual void                onComponentDeleted          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_BSPVisTreeElement; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries              ( );
    bool                        insertComponentData         ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32Array   mOcclusionGeometryIds;  // Reference identifiers of the meshes that contain the occlusion geometry.
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertCompileParams;
    static cgWorldQuery mInsertGeometryRef;
    static cgWorldQuery mClearGeometryRefs;
    static cgWorldQuery mUpdateCompileParams;
    static cgWorldQuery mLoadCompileParams;
    static cgWorldQuery mLoadGeometryRefs;
};

#endif // !_CGE_CGBSPVISTREEELEMENT_H_