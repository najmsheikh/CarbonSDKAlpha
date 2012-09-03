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
// File : cgWorldResourceComponent.h                                         //
//                                                                           //
// Desc : Contains a specialized base class, similar to cgWorldComponent,    //
//        from which all database driven resources should derive. Such       //
//        components might include meshes and other data sources that wish   //
//        to participate in data instancing.                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_WORLDRESOURCECOMPONENT_H_ )
#define _CGE_WORLDRESOURCECOMPONENT_H_

//-----------------------------------------------------------------------------
// cgWorldComponent Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResource.h>
#include <World/cgWorldTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class  cgWorld;
class  cgWorldQuery;
struct cgComponentModifiedEventArgs;
struct cgComponentCreatedEventArgs;
struct cgComponentLoadingEventArgs;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {A0371E05-CF09-4314-BB34-06A3B8B36DB9}
const cgUID RTID_WorldResourceComponent = {0xA0371E05, 0xCF09, 0x4314, {0xBB, 0x34, 0x06, 0xA3, 0xB8, 0xB3, 0x6D, 0xB9}};

//-----------------------------------------------------------------------------
// Name : cgWorldResourceComponent (Base Class)
/// <summary>
/// The basic representation of all resource components that make up the world.
/// Typical derived classes would be meshes or other data source types.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWorldResourceComponent : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgWorldResourceComponent, cgResource, "WorldResourceComponent" );

public:        
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWorldResourceComponent( cgUInt32 referenceId, cgWorld * world );
             cgWorldResourceComponent( cgUInt32 referenceId, cgWorld * world, cgWorldResourceComponent * init );
    virtual ~cgWorldResourceComponent( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgWorld                           * getParentWorld      ( ) const;
    cgUInt32                            getLocalTypeId      ( ) const;
    bool                                shouldSerialize     ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    // Property access.
    virtual cgString                    getDatabaseTable    ( ) const = 0;
    
    // Database management.
    virtual bool                        createTypeTables    ( const cgUID & typeIdentifier );

    // Event issuers.
    virtual void                        onComponentModified ( cgComponentModifiedEventArgs * e );
    virtual bool                        onComponentCreated  ( cgComponentCreatedEventArgs * e );
    virtual bool                        onComponentLoading  ( cgComponentLoadingEventArgs * e );
    virtual void                        onComponentDeleted  ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual void                        onReferenceAdded    ( cgReference * holder, cgInt32 holderReferences, bool reconnecting );
    virtual void                        onReferenceRemoved  ( cgReference * holder, cgInt32 holderReferences, bool disconnecting );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID               & getReferenceType    ( ) const { return RTID_WorldResourceComponent; }
    virtual bool                        queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                        dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgWorld   * mWorld;                 // Parent world to which this component belongs.
    cgUInt32    mComponentTypeId;       // The integer type identifier of the component as it exists in the database.
    bool        mInternalComponent;     // Is this component for use only internally, i.e. it does not exist in the database?
    
}; // End Class cgWorldResourceComponent

#endif // !_CGE_WORLDRESOURCECOMPONENT_H_