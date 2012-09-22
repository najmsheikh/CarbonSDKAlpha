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
// Name : cgSplineObject.h                                                   //
//                                                                           //
// Desc: Contains classes that allow for the design and placement of three   //
//       dimensional bezier splines. Shape of these splines can be designed  //
//       by manipulating individual control points. Can be used to provide   //
//       pathing / waypoint information, smooth animation, lofting or        //
//       collision data, etc.                                                //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSPLINEOBJECT_H_ )
#define _CGE_CGSPLINEOBJECT_H_

//-----------------------------------------------------------------------------
// cgSplineObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>
#include <Math/cgBezierSpline.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {7E35636A-8A2C-4FF2-9A64-510D034DF8BF}
const cgUID RTID_SplineNode   = {0x7E35636A, 0x8A2C, 0x4FF2, {0x9A, 0x64, 0x51, 0xD, 0x3, 0x4D, 0xF8, 0xBF}};
// {5584BF28-FBD8-4FDA-B6ED-47C159B3D95C}
const cgUID RTID_SplineObject = {0x5584BF28, 0xFBD8, 0x4FDA, {0xB6, 0xED, 0x47, 0xC1, 0x59, 0xB3, 0xD9, 0x5C}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSplineObject (Class)
/// <summary>
/// A simple spline object. Can be used to provide pathing / waypoint
/// information, smooth animation, lofting or collision data, etc.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSplineObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSplineObject, cgWorldObject, "SplineObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSplineObject( cgUInt32 referenceId, cgWorld * world );
             cgSplineObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgSplineObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgBezierSpline3     & getSplineData           ( ) const;
    void                        setSplineData           ( const cgBezierSpline3 & data );
    cgUInt32                    getDisplaySteps         ( ) const;
    void                        setDisplaySteps         ( cgUInt32 steps );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distanceOut );
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SplineObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );
    void                        updateBoundingBox       ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBezierSpline3 mSpline;
    cgBoundingBox   mBounds;
    cgUInt32        mDisplayInterpolationSteps;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertSpline;
    static cgWorldQuery     mUpdateSplineData;
    static cgWorldQuery     mUpdateDisplayOptions;
    static cgWorldQuery     mLoadSpline;
};

//-----------------------------------------------------------------------------
//  Name : cgSplineNode (Class)
/// <summary>
/// Custom node type for the spline object. Manages additional properties
/// that may need to be override by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSplineNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSplineNode, cgObjectNode, "SplineNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSplineNode( cgUInt32 referenceId, cgScene * scene );
             cgSplineNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgSplineNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setSplineData( const cgBezierSpline3 & data )
    {
        ((cgSplineObject*)mReferencedObject)->setSplineData( data );
    }
    inline void setDisplaySteps( cgUInt32 steps )
    {
        ((cgSplineObject*)mReferencedObject)->setDisplaySteps( steps );
    }

    // Object Property 'Get' Routing
    inline const cgBezierSpline3 & getSplineData( ) const
    {
        return ((cgSplineObject*)mReferencedObject)->getSplineData( );
    }
    inline cgUInt32 getDisplaySteps( ) const
    {
        return ((cgSplineObject*)mReferencedObject)->getDisplaySteps( );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SplineNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
};

#endif // !_CGE_CGSPLINEOBJECT_H_