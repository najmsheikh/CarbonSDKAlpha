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
// Name : cgAnimationTypes.h                                                 //
//                                                                           //
// Desc : Houses declarations and classes for various miscellaneous          //
//        animation related types, defines and macros.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGANIMATIONTYPES_H_ )
#define _CGE_CGANIMATIONTYPES_H_

//-----------------------------------------------------------------------------
// cgAnimationSet Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldQuery.h>
#include <Math/cgBezierSpline.h>
#include <Math/cgEulerAngles.h>
#include <Math/cgMathUtility.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgWorld;
class cgEulerAngles;
class cgQuaternion;

//-----------------------------------------------------------------------------
// Global Enumerations
//-----------------------------------------------------------------------------
namespace cgAnimationTargetControllerType
{
    enum Base
    {
        PositionXYZ = 0,
        ScaleXYZ,
        UniformScale,
        EulerAngles,
        Quaternion
    };

}; // End Namespace : cgAnimationTargetControllerType

namespace cgAnimationChannelDataType
{
    enum Base
    {
        BezierSpline = 0,
        CustomKeys
    };

}; // End Namespace : cgAnimationChannelDataType

namespace cgAnimationPlaybackMode
{
    enum Base
    {
        Loop = 0,
        PlayOnce
    };

}; // End Namespace : cgAnimationPlaybackMode

//-----------------------------------------------------------------------------
// Global Structures
//-----------------------------------------------------------------------------
struct CGE_API cgAnimationTrackDesc
{
    cgAnimationPlaybackMode::Base playbackMode; // Method used to playback the animation.
    cgDouble                      position;     // Current position of this track in seconds.
    cgDouble                      length;       // Computed length of the animation track in seconds. This member is populated by the animation controller.
    cgFloat                       weight;       // Current blending weight of this track.
    cgFloat                       speed;        // Rate at which the track should play.
    cgInt32                       firstFrame;   // Minimum key frame index to limit the animation set sampler.
    cgInt32                       lastFrame;    // Maximum key frame index to limit the animation set sampler.
    bool                          enabled;      // Track is currently playing back (will still contribute even when false, just will not advance)

    // Constructor
    cgAnimationTrackDesc() :
        playbackMode(cgAnimationPlaybackMode::Loop), position(0), length(0), speed(1),
        weight(1), firstFrame(0x7FFFFFFF), lastFrame(0x7FFFFFFF), enabled(true) {}

}; // End Struct : cgAnimationTrackDesc

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAnimationChannel (Class)
/// <summary>
/// Base class for an individual data channel of an animation target 
/// "controller". For instance, this might define the 'X' component data for 
/// the 'Scale' controller of a single bone in a character.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAnimationChannel
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgAnimationChannel( ) : 
      databaseId( 0 ), dirty(false) {}

    //-------------------------------------------------------------------------
	// Public Inline Static Methods
	//-------------------------------------------------------------------------
    inline static cgInt32 integerFrameIndex( cgFloat f )
    {
        // Round to closest integer frame index.
        return cgMathUtility::integerFloor( (f>=0) ? f + 0.5f : f - 0.5f );
    }

    //-------------------------------------------------------------------------
	// Public Members
	//-------------------------------------------------------------------------
    cgUInt32    databaseId;
    bool        dirty;

protected:
    //-------------------------------------------------------------------------
	// Protected Methods
	//-------------------------------------------------------------------------
    void        prepareQueries( cgWorld * world );

    //-------------------------------------------------------------------------
	// Protected Static Variables
	//-------------------------------------------------------------------------
    static cgWorldQuery mInsertChannelData;
    static cgWorldQuery mUpdateChannelData;
    static cgWorldQuery mLoadChannelData;
};

//-----------------------------------------------------------------------------
//  Name : cgFloatCurveAnimationChannel (Class)
/// <summary>
/// An implementation of the base 'cgAnimationChannel' which is capable of 
/// describing a single floating point value over time as a bezier spline.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFloatCurveAnimationChannel : public cgAnimationChannel
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgFloatCurveAnimationChannel( ) :
        data( cgBezierSpline2::SplinePointArray() ) {}

    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------
    bool        serialize   ( cgUInt32 targetControllerId, const cgString & channelIdentifier, cgWorld * world );
    bool        deserialize ( cgWorldQuery & channelQuery, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut );
    void        clear       ( );
    void        addLinearKey( cgInt32 frame, cgFloat value );
    
    //-----------------------------------------------------------------------------
    // Public Inline Methods
    //-----------------------------------------------------------------------------
    inline bool isEmpty     ( ) const { return (data.getPointCount() == 0); }

    //-------------------------------------------------------------------------
	// Public Variables
	//-------------------------------------------------------------------------
    cgBezierSpline2 data;
};

//-----------------------------------------------------------------------------
//  Name : cgQuaternionAnimationChannel (Class)
/// <summary>
/// An implementation of the base 'cgAnimationChannel' which is capable of 
/// describing the value of a single quaternion over time as linear piecewise 
/// data (simple key frames).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgQuaternionAnimationChannel : public cgAnimationChannel
{
public:
    //-------------------------------------------------------------------------
	// Public Typedefs, Structures & Enumerations
	//-------------------------------------------------------------------------
    // Key frame for the storage of a quaternion
    struct QuaternionKeyFrame
    {
        cgInt32    frame;       // Time in frame / ticks for this keyframe
        cgFloat    value[4];    // The quaternion key data.
    };
    CGE_VECTOR_DECLARE(QuaternionKeyFrame, QuaternionKeyArray)

    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgQuaternionAnimationChannel( ) {}

    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------
    bool        serialize   ( cgUInt32 targetControllerId, const cgString & channelIdentifier, cgWorld * world );
    bool        deserialize ( cgWorldQuery & channelQuery, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut );
    void        clear       ( );
    void        addKey      ( cgInt32 frame, const cgQuaternion & value );
    
    //-----------------------------------------------------------------------------
    // Public Inline Methods
    //-----------------------------------------------------------------------------
    inline bool isEmpty     ( ) const { return data.empty(); }
    
    //-------------------------------------------------------------------------
	// Public Variables
	//-------------------------------------------------------------------------
    QuaternionKeyArray data;
};

//-----------------------------------------------------------------------------
//  Name : cgAnimationTargetController (Class)
/// <summary>
/// Base 'class' for an individual controller for an animation target. A target
/// controller commonly defines a collection of data channels 
/// (cgAnimationChannel), and a mechanism for evaluating them at a given 
/// time. For instance, this controller might define the 'Scale' values for 
/// a specific animation set.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAnimationTargetController : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgAnimationTargetController, "AnimationTargetController" )

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgAnimationTargetController( );
	virtual ~cgAnimationTargetController( );

    //-------------------------------------------------------------------------
	// Public Static Methods
	//-------------------------------------------------------------------------
    static cgAnimationTargetController* createInstance( cgAnimationTargetControllerType::Base Type );

    //-------------------------------------------------------------------------
	// Public Virtual Methods
	//-------------------------------------------------------------------------
    virtual bool    serialize           ( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize );
    virtual bool    deserialize         ( cgWorldQuery & controllerQuery, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut, void *& customDataOut, cgUInt32 & customDataSizeOut );
    virtual bool    deserializeChannel  ( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut ) { return true; }

    //-------------------------------------------------------------------------
	// Public Pure Virtual Methods
	//-------------------------------------------------------------------------
    virtual cgAnimationTargetControllerType::Base getControllerType     ( ) const = 0;
    virtual const cgStringArray                 & getSupportedChannels  ( ) const = 0;

protected:
    //-------------------------------------------------------------------------
	// Protected Methods
	//-------------------------------------------------------------------------
    void            prepareQueries      ( cgWorld * world );

    //-------------------------------------------------------------------------
	// Protected Variables
	//-------------------------------------------------------------------------
    cgUInt32    mDatabaseId;

    //-------------------------------------------------------------------------
	// Protected Static Variables
	//-------------------------------------------------------------------------
    static cgWorldQuery mInsertController;
    static cgWorldQuery mLoadChannelData;
};

//-----------------------------------------------------------------------------
//  Name : cgPositionXYZTargetController (Class)
/// <summary>
/// An implementation of the base 'cgAnimationTargetController' class that
/// allows the application to represent a three component (XYZ) position vector
/// as a series of curves for the purposes of animation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPositionXYZTargetController : public cgAnimationTargetController
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPositionXYZTargetController, cgAnimationTargetController, "PositionXYZTargetController" )

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgPositionXYZTargetController( );
             cgPositionXYZTargetController( const cgPositionXYZTargetController & init, const cgRange & frameRange );
	virtual ~cgPositionXYZTargetController( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                                    evaluate            ( cgDouble position, cgVector3 & p, const cgVector3 & default );
    void                                    addLinearKey        ( cgInt32 frame, const cgVector3 & value );
    const cgFloatCurveAnimationChannel    & getAnimationChannel ( cgUInt32 index ) const;
    cgFloatCurveAnimationChannel          & getAnimationChannel ( cgUInt32 index );
    
    //-------------------------------------------------------------------------
	// Public Inline Methods
	//-------------------------------------------------------------------------
    inline bool isEmpty( bool anyChannelEmpty = false ) const
    {
        if ( anyChannelEmpty )
            return mCurves[0].isEmpty() || mCurves[1].isEmpty() || mCurves[2].isEmpty();
        else
            return mCurves[0].isEmpty() && mCurves[1].isEmpty() && mCurves[2].isEmpty();
    }

    //-------------------------------------------------------------------------
	// Public Virtual Methods (Overrides cgAnimationTargetController)
	//-------------------------------------------------------------------------
    virtual bool    serialize           ( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize );
    virtual bool    deserializeChannel  ( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut );

    // Pure virtual type descriptions
    virtual cgAnimationTargetControllerType::Base getControllerType     ( ) const { return cgAnimationTargetControllerType::PositionXYZ; }
    virtual const cgStringArray                 & getSupportedChannels  ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Members
	//-------------------------------------------------------------------------
    cgFloatCurveAnimationChannel    mCurves[3];
};

//-----------------------------------------------------------------------------
//  Name : cgScaleXYZTargetController (Class)
/// <summary>
/// An implementation of the base 'cgAnimationTargetController' class that
/// allows the application to represent a three component (XYZ) scale vector
/// as a series of curves for the purposes of animation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScaleXYZTargetController : public cgAnimationTargetController
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgScaleXYZTargetController, cgAnimationTargetController, "ScaleXYZTargetController" )

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgScaleXYZTargetController( );
             cgScaleXYZTargetController( const cgScaleXYZTargetController & init, const cgRange & frameRange );
	virtual ~cgScaleXYZTargetController( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                                    evaluate            ( cgDouble position, cgVector3 & s, const cgVector3 & default );
    void                                    addLinearKey        ( cgInt32 frame, const cgVector3 & value );
    const cgFloatCurveAnimationChannel    & getAnimationChannel ( cgUInt32 index ) const;
    cgFloatCurveAnimationChannel          & getAnimationChannel ( cgUInt32 index );
    
    //-------------------------------------------------------------------------
	// Public Inline Methods
	//-------------------------------------------------------------------------
    inline bool isEmpty( bool anyChannelEmpty = false ) const
    {
        if ( anyChannelEmpty )
            return mCurves[0].isEmpty() || mCurves[1].isEmpty() || mCurves[2].isEmpty();
        else
            return mCurves[0].isEmpty() && mCurves[1].isEmpty() && mCurves[2].isEmpty();
    }

    //-------------------------------------------------------------------------
	// Public Virtual Methods (Overrides cgAnimationTargetController)
	//-------------------------------------------------------------------------
    virtual bool    serialize           ( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize );
    virtual bool    deserializeChannel  ( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut );

    // Pure virtual type descriptions
    virtual cgAnimationTargetControllerType::Base getControllerType     ( ) const { return cgAnimationTargetControllerType::ScaleXYZ; }
    virtual const cgStringArray                 & getSupportedChannels  ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose         ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Members
	//-------------------------------------------------------------------------
    cgFloatCurveAnimationChannel    mCurves[3];
};

//-----------------------------------------------------------------------------
//  Name : cgUniformScaleTargetController (Class)
/// <summary>
/// An implementation of the base 'cgAnimationTargetController' class that
/// allows the application to represent a single component uniform scale value
/// as a curve for the purposes of animation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUniformScaleTargetController : public cgAnimationTargetController
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgUniformScaleTargetController, cgAnimationTargetController, "UniformScaleTargetController" )

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgUniformScaleTargetController( );
             cgUniformScaleTargetController( const cgUniformScaleTargetController & init, const cgRange & frameRange );
	virtual ~cgUniformScaleTargetController( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                                evaluate            ( cgDouble position, cgFloat & s, cgFloat default );
    void                                addLinearKey        ( cgInt32 frame, cgFloat value );
    const cgFloatCurveAnimationChannel& getAnimationChannel ( ) const;
    cgFloatCurveAnimationChannel      & getAnimationChannel ( );
    
    //-------------------------------------------------------------------------
	// Public Inline Methods
	//-------------------------------------------------------------------------
    inline bool isEmpty( bool anyChannelEmpty = false ) const
    {
        return mCurve.isEmpty();
    }

    //-------------------------------------------------------------------------
	// Public Virtual Methods (Overrides cgAnimationTargetController)
	//-------------------------------------------------------------------------
    virtual bool    serialize           ( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize );
    virtual bool    deserializeChannel  ( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut );

    // Pure virtual type descriptions
    virtual cgAnimationTargetControllerType::Base getControllerType     ( ) const { return cgAnimationTargetControllerType::UniformScale; }
    virtual const cgStringArray                 & getSupportedChannels  ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Members
	//-------------------------------------------------------------------------
    cgFloatCurveAnimationChannel    mCurve;
};

//-----------------------------------------------------------------------------
//  Name : cgQuaternionTargetController (Class)
/// <summary>
/// An implementation of the base 'cgAnimationTargetController' class that
/// allows the application to represent a quaternion as a series of key frames
/// for the purposes of animation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgQuaternionTargetController : public cgAnimationTargetController
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgQuaternionTargetController, cgAnimationTargetController, "QuaternionTargetController" )

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgQuaternionTargetController( );
             cgQuaternionTargetController( const cgQuaternionTargetController & init, const cgRange & frameRange );
	virtual ~cgQuaternionTargetController( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                                evaluate            ( cgDouble position, cgQuaternion & q, const cgQuaternion & default );
    void                                addKey              ( cgInt32 frame, const cgQuaternion & value );
    const cgQuaternionAnimationChannel& getAnimationChannel ( ) const;
    cgQuaternionAnimationChannel      & getAnimationChannel ( );
    
    //-------------------------------------------------------------------------
	// Public Inline Methods
	//-------------------------------------------------------------------------
    inline bool isEmpty( bool anyChannelEmpty = false ) const
    {
        return mKeyFrames.isEmpty();
    }

    //-------------------------------------------------------------------------
	// Public Virtual Methods (Overrides cgAnimationTargetController)
	//-------------------------------------------------------------------------
    virtual bool    serialize           ( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize );
    virtual bool    deserializeChannel  ( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut );

    // Pure virtual type descriptions
    virtual cgAnimationTargetControllerType::Base getControllerType     ( ) const { return cgAnimationTargetControllerType::Quaternion; }
    virtual const cgStringArray                 & getSupportedChannels  ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Members
	//-------------------------------------------------------------------------
    cgQuaternionAnimationChannel    mKeyFrames;
};

//-----------------------------------------------------------------------------
//  Name : cgEulerAnglesTargetController (Class)
/// <summary>
/// An implementation of the base 'cgAnimationTargetController' class that
/// allows the application to represent three separate euler angles (XYZ in
/// degrees) as a series of curves for the purpose of animation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgEulerAnglesTargetController : public cgAnimationTargetController
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgEulerAnglesTargetController, cgAnimationTargetController, "EulerAnglesTargetController" )

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgEulerAnglesTargetController( cgEulerAnglesOrder::Base order = cgEulerAnglesOrder::YXZ );
             cgEulerAnglesTargetController( const cgEulerAnglesTargetController & init, const cgRange & frameRange );
	virtual ~cgEulerAnglesTargetController( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                                    evaluate            ( cgDouble position, cgEulerAngles & a, const cgEulerAngles & default );
    void                                    evaluate            ( cgDouble position, cgEulerAngles & a, const cgQuaternion & default );
    void                                    addLinearKey        ( cgInt32 frame, const cgEulerAngles & value );
    void                                    addLinearKey        ( cgInt32 frame, const cgQuaternion & value );
    const cgFloatCurveAnimationChannel    & getAnimationChannel ( cgUInt32 index ) const;
    cgFloatCurveAnimationChannel          & getAnimationChannel ( cgUInt32 index );
    
    //-------------------------------------------------------------------------
	// Public Inline Methods
	//-------------------------------------------------------------------------
    inline bool isEmpty( bool anyChannelEmpty = false ) const
    {
        if ( anyChannelEmpty )
            return mCurves[0].isEmpty() || mCurves[1].isEmpty() || mCurves[2].isEmpty();
        else
            return mCurves[0].isEmpty() && mCurves[1].isEmpty() && mCurves[2].isEmpty();
    }
    
    inline cgEulerAnglesOrder::Base getRotationOrder( ) const
    {
        return mRotationOrder;
    }

    //-------------------------------------------------------------------------
	// Public Virtual Methods (Overrides cgAnimationTargetController)
	//-------------------------------------------------------------------------
    virtual bool    serialize           ( cgUInt32 targetDataId, const cgString & controllerIdentifier, cgWorld * world, void * customData, cgUInt32 customDataSize );
    virtual bool    deserialize         ( cgWorldQuery & controllerQuery, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut, void *& customDataOut, cgUInt32 & customDataSizeOut );
    virtual bool    deserializeChannel  ( cgWorldQuery & channelQuery, cgInt32 channelIndex, bool cloning, cgInt32 & minFrameOut, cgInt32 & maxFrameOut );

    // Pure virtual type descriptions
    virtual cgAnimationTargetControllerType::Base getControllerType     ( ) const { return cgAnimationTargetControllerType::EulerAngles; }
    virtual const cgStringArray                 & getSupportedChannels  ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Members
	//-------------------------------------------------------------------------
    cgFloatCurveAnimationChannel    mCurves[3];
    cgEulerAnglesOrder::Base        mRotationOrder;
};

#endif // !_CGE_CGANIMATIONTYPES_H_