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
// Name : cgNavigationMeshElement.h                                          //
//                                                                           //
// Desc : Class that provides configuration and management of scene          //
//        navigation data, exposed as a scene element type. This provides    //
//        the integration between the application (such as the editing       //
//        environment) and the relevant components of the navigation system. //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNAVIGATIONMESHELEMENT_H_ )
#define _CGE_CGNAVIGATIONMESHELEMENT_H_

//-----------------------------------------------------------------------------
// cgNavigationMeshElement Header Includes
//-----------------------------------------------------------------------------
#include <World/cgSceneElement.h>
#include <Navigation/cgNavigationTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgNavigationMesh;
class cgNavigationAgent;
class cgNavigationHandler;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {DE0FA804-24AF-470C-8556-FB01E2512356}
const cgUID RTID_NavigationMeshElement = { 0xDE0FA804, 0x24AF, 0x470C, { 0x85, 0x56, 0xFB, 0x01, 0xE2, 0x51, 0x23, 0x56 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgNavigationMeshElement (Class)
/// <summary>
/// Class that provides configuration and management of scene navigation data,
/// exposed as a scene element type. This provides the integration between the
/// application (such as the editing environment) and the relevant components
/// of the navigation system.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationMeshElement : public cgSceneElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNavigationMeshElement, cgSceneElement, "NavigationMeshElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationMeshElement( cgUInt32 referenceId, cgScene * scene );
    virtual ~cgNavigationMeshElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgSceneElement     * allocateNew                 ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setAgentRadius              ( cgFloat radius, bool rebuild = false );
    void                        setAgentHeight              ( cgFloat height, bool rebuild = false );
    void                        setAgentMaximumSlope        ( cgFloat degrees, bool rebuild = false );
    void                        setAgentMaximumStepHeight   ( cgFloat height, bool rebuild = false );
    void                        setParameters               ( const cgNavigationMeshCreateParams & params, bool rebuild = false );
    bool                        buildMesh                   ( );
    cgNavigationAgent         * createAgent                 ( const cgNavigationAgentCreateParams & params, const cgVector3 & position );
    const cgNavigationMeshCreateParams& getParameters       ( ) const;
    cgNavigationHandler       * getNavigationHandler        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSceneElement)
    //-------------------------------------------------------------------------
    virtual void                update                      ( cgFloat timeDelta );
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
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_NavigationMeshElement; }
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
    cgNavigationMesh              * mNavMesh;   // Constructed navigation mesh
    cgNavigationHandler           * mHandler;   // Navigation handler associated with this mesh. Performs agent updates as required.
    cgNavigationMeshCreateParams    mParams;    // Navigation mesh generation parameters.
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertMeshParams;
    static cgWorldQuery mUpdateMeshParams;
    static cgWorldQuery mLoadMeshParams;
};

#endif // !_CGE_CGNAVIGATIONMESHELEMENT_H_