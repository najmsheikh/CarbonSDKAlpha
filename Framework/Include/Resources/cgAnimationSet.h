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
// Name : cgAnimationSet.h                                                   //
//                                                                           //
// Desc : Contains resource data for an individual animation set, providing  //
//        animated transformation, event and property information where      //
//        available.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGANIMATIONSET_H_ )
#define _CGE_CGANIMATIONSET_H_

//-----------------------------------------------------------------------------
// cgAnimationSet Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldResourceComponent.h>
#include <World/cgWorldQuery.h>
#include <Animation/cgAnimationTypes.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgVector3;
class cgMatrix;
class cgQuaternion;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {FE5A5527-F03B-4924-85A1-B5657D798E1F}
const cgUID RTID_AnimationSetResource = {0xFE5A5527, 0xF03B, 0x4924, {0x85, 0xA1, 0xB5, 0x65, 0x7D, 0x79, 0x8E, 0x1F}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAnimationSet (Class)
/// <summary>
/// A named collection of animation data which can be applied to the
/// controller and swapped / merged with other animation sets at runtime.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAnimationSet : public cgWorldResourceComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAnimationSet, cgWorldResourceComponent, "AnimationSet" )

public:
    //-------------------------------------------------------------------------
	// Public Typdefs, Structures and Enumerations
	//-------------------------------------------------------------------------
    struct CGE_API TargetData
    {
        cgAnimationTargetController   * scaleController;
        cgAnimationTargetController   * rotationController;
        cgAnimationTargetController   * translationController;
        cgUInt32                        databaseId;

        // Constructor
        TargetData() :
            scaleController( CG_NULL ), rotationController(CG_NULL),
            translationController(CG_NULL), databaseId(0) {}

    };
    CGE_UNORDEREDMAP_DECLARE(cgString, TargetData, TargetDataMap)

    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgAnimationSet( cgUInt32 referenceId, cgWorld * world );
             cgAnimationSet( cgUInt32 referenceId, cgWorld * world, cgFloat frameRate );
             cgAnimationSet( cgUInt32 referenceId, cgWorld * world, cgUInt32 sourceRefId );
             cgAnimationSet( cgUInt32 referenceId, cgWorld * world, cgAnimationSet * init );
             cgAnimationSet( cgUInt32 referenceId, cgWorld * world, cgAnimationSet * init, const cgRange & frameRange );
	virtual ~cgAnimationSet( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    bool                    suspendSerialization( bool flush = false );
    bool                    resumeSerialization ( bool flush = true );
    bool                    loadSet             ( cgUInt32 sourceRefId, cgResourceManager * resourceManager = CG_NULL );
    bool                    isEmpty             ( ) const;
    void                    setName             ( const cgString & name );
    const cgString        & getName             ( ) const;
    cgFloat                 getFrameRate        ( ) const;
    cgRange                 getFrameRange       ( ) const;
    const TargetDataMap   & getTargetData       ( ) const;
    const TargetData      * getTargetData       ( const cgString & targetId ) const;
    TargetData            * getTargetData       ( const cgString & targetId );
    TargetData            * getTargetData       ( const cgString & targetId, bool createTargetData );
    bool                    getSRT              ( cgDouble framePosition, cgAnimationPlaybackMode::Base mode, const cgString & targetId, cgInt32 firstFrame, cgInt32 lastFrame, cgVector3 & scale, cgQuaternion & rotation, cgVector3 & translation );
    bool                    getSRT              ( cgDouble framePosition, cgAnimationPlaybackMode::Base mode, const cgString & targetId, cgInt32 firstFrame, cgInt32 lastFrame, cgAnimationTarget * defaultsTarget, cgVector3 & scale, cgQuaternion & rotation, cgVector3 & translation );
    void                    addScaleKey         ( cgInt32 frame, const cgString & targetId, const cgVector3 & scale );
    void                    addRotationKey      ( cgInt32 frame, const cgString & targetId, const cgQuaternion & rotation );
    void                    addTranslationKey   ( cgInt32 frame, const cgString & targetId, const cgVector3 & translation );
    void                    addSRTKey           ( cgInt32 frame, const cgString & targetId, const cgVector3 & scale, const cgQuaternion & rotation, const cgVector3 & translation );
    void                    addMatrixKey        ( cgInt32 frame, const cgString & targetId, const cgMatrix & transform );
    cgInt32                 computeFrameIndex   ( cgDouble position );
    void                    targetDataUpdated   ( bool recomputeRange );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_AnimationSetResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldResourceComponent)
    //-------------------------------------------------------------------------
    virtual cgString        getDatabaseTable    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    prepareQueries      ( );
    bool                    serializeSet        ( );

    //-------------------------------------------------------------------------
    // Protected Static Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 NameDirty       = 0x1;
    static const cgUInt32 FrameRateDirty  = 0x2;
    static const cgUInt32 TargetDataDirty = 0x3;
    static const cgUInt32 AllDirty        = 0xFFFFFFFF;

    //-------------------------------------------------------------------------
	// Protected Variables
	//-------------------------------------------------------------------------
    /// <summary>Friendly name of the animation set.</summary>
    cgString            mName;
    /// <summary>A map containing the animation data for a given animation target (by identifier).</summary>
    TargetDataMap       mTargetData;
    /// <summary>Lowest frame recorded in this animation set.</summary>
    cgInt32             mFirstFrame;
    /// <summary>Highest frame recorded in this animation set.</summary>
    cgInt32             mLastFrame;
    /// <summary>Playback rate of this animation set. For information purposes only.</summary>
    cgFloat             mFramesPerSecond;

    // Loading & Serialization.
    /// <summary>Reference identifier of the animation set source data from which we should load (if any).</summary>
    cgUInt32            mSourceRefId;
    /// <summary>Has data for this animation set been serialized yet? (i.e. the initial insert).</summary>
    bool                mSetSerialized;
    /// <summary>Which components of the animation set are dirty?.</summary>
    cgUInt32            mDBDirtyFlags;
    /// <summary>Is serialization suspended?</summary>
    bool                mSuspendSerialization;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertSet;
    static cgWorldQuery mInsertTargetData;
    static cgWorldQuery mUpdateFrameRate;
    static cgWorldQuery mUpdateName;
    static cgWorldQuery mLoadSet;
    static cgWorldQuery mLoadTargetData;
    static cgWorldQuery mLoadTargetControllers;
};

#endif // !_CGE_CGANIMATIONSET_H_