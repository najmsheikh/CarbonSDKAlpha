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
// Name : cgSoundEmitterObject.h                                             //
//                                                                           //
// Desc : Contains classes responsible for representing 3D positional sound  //
//        emitters within a scene.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSOUNDEMITTEROBJECT_H_ )
#define _CGE_CGSOUNDEMITTEROBJECT_H_

//-----------------------------------------------------------------------------
// cgSoundEmitterObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {AD888A39-D158-4145-A73F-78B0EFE87D38}
const cgUID RTID_SoundEmitterNode   = {0xAD888A39, 0xD158, 0x4145, {0xA7, 0x3F, 0x78, 0xB0, 0xEF, 0xE8, 0x7D, 0x38}};
// {CADD1C4E-B47B-4807-AC33-8D43E8200865}
const cgUID RTID_SoundEmitterObject = {0xCADD1C4E, 0xB47B, 0x4807, {0xAC, 0x33, 0x8D, 0x43, 0xE8, 0x20, 0x8, 0x65}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSoundEmitterObject (Class)
/// <summary>
/// Class responsible for representing 3D positional sound emitters within a
/// scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSoundEmitterObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSoundEmitterObject, cgWorldObject, "SoundEmitterObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSoundEmitterObject( cgUInt32 referenceId, cgWorld * world );
             cgSoundEmitterObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgSoundEmitterObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgString            & getSourceFile           ( ) const;
    cgFloat                     getDefaultVolume        ( ) const;
    bool                        isStreaming             ( ) const;
    bool                        isLooping               ( ) const;
    bool                        shouldAutoPlay          ( ) const;
    cgFloat                     getInnerRange           ( ) const;
    cgFloat                     getOuterRange           ( ) const;
    bool                        shouldMuteOutsideRange  ( ) const;
    void                        setSourceFile           ( const cgString & fileName );
    void                        setDefaultVolume        ( cgFloat volume );
    void                        enableStreaming         ( bool streaming );
    void                        enableAutoPlay          ( bool autoPlay );
    void                        enableLooping           ( bool looping );
    void                        enableMuteOutsideRange  ( bool mute );
    void                        setInnerRange           ( cgFloat range );
    void                        setOuterRange           ( cgFloat range );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
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
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SoundEmitterObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgString    mSourceFileName;
    bool        mStreamingBuffer;
    cgFloat     mDefaultVolume;
    bool        mAutoPlay;
    bool        mLooping;
    bool        mMuteOutsideRange;
    cgFloat     mOuterRange;
    cgFloat     mInnerRange;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertSoundEmitter;
    static cgWorldQuery     mUpdateProperties;
    static cgWorldQuery     mUpdateRanges;
    static cgWorldQuery     mLoadSoundEmitter;
};

//-----------------------------------------------------------------------------
//  Name : cgSoundEmitterNode (Class)
/// <summary>
/// Custom node type for the sound emitter object. Manages additional
/// properties that may need to be overriden by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSoundEmitterNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSoundEmitterNode, cgObjectNode, "SoundEmitterNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSoundEmitterNode( cgUInt32 referenceId, cgScene * scene );
             cgSoundEmitterNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgSoundEmitterNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        play                    ( );
    void                        stop                    ( );
    bool                        isPlaying               ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                onNodeInit              ( const cgUInt32IndexMap & nodeReferenceRemap );
    virtual bool                setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );
    virtual bool                canScale                ( ) const;
    virtual bool                canRotate               ( ) const;
    virtual bool                getSandboxIconInfo      ( cgCameraNode * camera, const cgSize & viewportSize, cgString & atlasFile, cgString & frameName, cgVector3 & iconOrigin );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool bDisposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setSourceFile( const cgString & fileName )
    {
        ((cgSoundEmitterObject*)mReferencedObject)->setSourceFile( fileName );
    }
    inline void setDefaultVolume( cgFloat volume )
    {
        ((cgSoundEmitterObject*)mReferencedObject)->setDefaultVolume( volume );
    }
    inline void enableStreaming( bool streaming )
    {
        ((cgSoundEmitterObject*)mReferencedObject)->enableStreaming( streaming );
    }
    inline void enableAutoPlay( bool autoPlay )
    {
        ((cgSoundEmitterObject*)mReferencedObject)->enableAutoPlay( autoPlay );
    }
    inline void enableLooping( bool looping )
    {
        ((cgSoundEmitterObject*)mReferencedObject)->enableLooping( looping );
    }
    inline void enableMuteOutsideRange( bool mute )
    {
        ((cgSoundEmitterObject*)mReferencedObject)->enableMuteOutsideRange( mute );
    }
    inline void setInnerRange( cgFloat range )
    {
        ((cgSoundEmitterObject*)mReferencedObject)->setInnerRange( range );
    }
    inline void setOuterRange( cgFloat range )
    {
        ((cgSoundEmitterObject*)mReferencedObject)->setOuterRange( range );
    }

    // Object Property 'Get' Routing
    inline const cgString & getSourceFile( ) const
    {
        return ((cgSoundEmitterObject*)mReferencedObject)->getSourceFile( );
    }
    inline cgFloat getDefaultVolume( ) const
    {
        return ((cgSoundEmitterObject*)mReferencedObject)->getDefaultVolume( );
    }
    inline bool isStreaming( ) const
    {
        return ((cgSoundEmitterObject*)mReferencedObject)->isStreaming( );
    }
    inline bool isLooping( ) const
    {
        return ((cgSoundEmitterObject*)mReferencedObject)->isLooping( );
    }
    inline bool shouldAutoPlay( ) const
    {
        return ((cgSoundEmitterObject*)mReferencedObject)->shouldAutoPlay( );
    }
    inline cgFloat getInnerRange( ) const
    {
        return ((cgSoundEmitterObject*)mReferencedObject)->getInnerRange( );
    }
    inline cgFloat getOuterRange( ) const
    {
        return ((cgSoundEmitterObject*)mReferencedObject)->getOuterRange( );
    }
    inline bool shouldMuteOutsideRange( ) const
    {
        return ((cgSoundEmitterObject*)mReferencedObject)->shouldMuteOutsideRange( );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SoundEmitterNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        reloadBuffer            ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgAudioBufferHandle mAudioBuffer;
};

#endif // !_CGE_CGSOUNDEMITTEROBJECT_H_