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
// Name : cgSkyElement.h                                                     //
//                                                                           //
// Desc : Class that provides configuration and management of scene          //
//        sky rendering data, exposed as a scene element type. This provides //
//        the integration between the application (such as the editing       //
//        environment) and the relevant components of the scene renderer.    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSKYELEMENT_H_ )
#define _CGE_CGSKYELEMENT_H_

//-----------------------------------------------------------------------------
// cgSkyElement Header Includes
//-----------------------------------------------------------------------------
#include <World/cgSceneElement.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgSampler;
struct cgSamplerStateDesc;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {290340C3-D200-4AFD-AA4E-BEB67717D0D8}
const cgUID RTID_SkyElement = { 0x290340C3, 0xD200, 0x4AFD, { 0xAA, 0x4E, 0xBE, 0xB6, 0x77, 0x17, 0xD0, 0xD8 } };

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
namespace cgSkyElementType
{
    enum Base
    {
        SkyBox  = 0
    };

} // End Namespace : cgSkyElementType

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSkyElement (Class)
/// <summary>
/// Class that provides configuration and management of scene sky rendering 
/// data, exposed as a scene element type. This provides the integration 
/// between the application (such as the editing environment) and the relevant
/// components of the scene renderer.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSkyElement : public cgSceneElement
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSkyElement, cgSceneElement, "SkyElement" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSkyElement( cgUInt32 referenceId, cgScene * scene );
    virtual ~cgSkyElement( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgSceneElement     * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgSampler                 * getBaseSampler          ( );
    cgFloat                     getBaseHDRScale         ( ) const;
    cgSkyElementType::Base      getSkyType              ( ) const;
    bool                        setBaseSampler          ( cgSampler * sourceSampler );
    bool                        setBaseSampler          ( cgInputStream texture, const cgSamplerStateDesc & states );
    void                        setBaseHDRScale         ( cgFloat value );
    void                        setSkyType              ( cgSkyElementType::Base type );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSceneElement)
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual cgString            getDatabaseTable        ( ) const;
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual void                onComponentDeleted      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SkyElement; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSkyElementType::Base  mType;          // Sky rendering type.
    cgSampler             * mBaseSampler;   // Base texture sampler (i.e., cube texture if skybox).
    cgFloat                 mBaseHDRScale;  // Intensity scalar to apply when rendering with HDR enabled.
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertSkyElement;
    static cgWorldQuery mDeleteSkyElement;
    static cgWorldQuery mUpdateSkyProperties;
    static cgWorldQuery mLoadSkyElement;
};

#endif // !_CGE_CGSKYELEMENT_H_