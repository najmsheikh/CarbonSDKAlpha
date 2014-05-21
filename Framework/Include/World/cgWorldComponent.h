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
// File : cgWorldComponent.h                                                 //
//                                                                           //
// Desc : Contains a base class from which all component pieces of the world //
//        should derive. Such components might include objects, data sources //
//        that wish to participate in data instancing (such as mesh /        //
//        geometry data used to represent skins, standard meshes or spatial  //
//        trees for instance) and so on.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWORLDCOMPONENT_H_ )
#define _CGE_CGWORLDCOMPONENT_H_

//-----------------------------------------------------------------------------
// cgWorldComponent Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgReference.h>
#include <World/cgWorldTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgWorld;
class cgWorldQuery;
class cgWorldComponent;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {E9B38216-67A4-424D-ABF4-D1CADD1E3B15}
const cgUID RTID_WorldComponent = {0xE9B38216, 0x67A4, 0x424D, {0xAB, 0xF4, 0xD1, 0xCA, 0xDD, 0x1E, 0x3B, 0x15}};

//-----------------------------------------------------------------------------
// Event Argument Definitions
//-----------------------------------------------------------------------------
struct CGE_API cgComponentCreatedEventArgs
{
    cgComponentCreatedEventArgs( cgUInt32 _componentTypeId, cgCloneMethod::Base _cloneMethod ) :
        componentTypeId(_componentTypeId), cloneMethod(_cloneMethod) {}
    cgUInt32            componentTypeId;
    cgCloneMethod::Base cloneMethod;
};

struct CGE_API cgComponentLoadingEventArgs
{
    // Constructor
    cgComponentLoadingEventArgs( cgUInt32 _sourceRefId, cgUInt32 _componentTypeId, cgCloneMethod::Base _cloneMethod, cgWorldQuery * _componentData ) :
        sourceRefId(_sourceRefId), componentTypeId(_componentTypeId), componentData(_componentData),
        cloneMethod(_cloneMethod) {}

    // Public Members
    cgUInt32            sourceRefId;
    cgUInt32            componentTypeId;
    cgCloneMethod::Base cloneMethod;
    cgWorldQuery      * componentData;
};

struct CGE_API cgComponentModifiedEventArgs
{
    cgComponentModifiedEventArgs( const cgString & _context ) :
        context(_context) {}
    cgComponentModifiedEventArgs( const cgString & _context, const cgVariant & _argument ) :
        context(_context), argument(_argument) {}
    cgString  context;
    cgVariant argument;
};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgWorldComponentEventListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever world component events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWorldComponentEventListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgWorldComponentEventListener, cgEventListener, "WorldComponentEventListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onComponentModified ( cgReference * sender, cgComponentModifiedEventArgs * e ) {};
    virtual void    onComponentCreated  ( cgReference * sender, cgComponentCreatedEventArgs * e ) {};
    virtual void    onComponentLoading  ( cgReference * sender, cgComponentLoadingEventArgs * e ) {};
    virtual void    onComponentDeleted  ( cgReference * sender ) {};

private:
    virtual void    onComponentModified ( cgComponentModifiedEventArgs * e ) {};
    virtual void    onComponentCreated  ( cgComponentCreatedEventArgs * e ) {};
    virtual void    onComponentLoading  ( cgComponentLoadingEventArgs * e ) {};
    virtual void    onComponentDeleted  ( ) {};
};

//-----------------------------------------------------------------------------
// Name : cgWorldComponent (Base Class)
/// <summary>
/// The basic representation of all components that make up the world.
/// Typical derived classes would be object or data source types.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWorldComponent : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgWorldComponent, cgReference, "WorldComponent" );

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWorldComponent( cgUInt32 referenceId, cgWorld * world );
             cgWorldComponent( cgUInt32 referenceId, cgWorld * world, cgWorldComponent * init );
    virtual ~cgWorldComponent( );

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
    virtual const cgUID               & getReferenceType    ( ) const { return RTID_WorldComponent; }
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
    
}; // End Class cgWorldComponent

#endif // !_CGE_CGWORLDCOMPONENT_H_