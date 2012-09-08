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
// Name : cgTexture.h                                                        //
//                                                                           //
// Desc : Contains classes responsible for loading and managing texture      //
//        resource data.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGTEXTURE_H_ )
#define _CGE_CGTEXTURE_H_

//-----------------------------------------------------------------------------
// cgTexture Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResource.h>
#include <Scripting/cgScriptInterop.h>

// Media / video decoder
// ToDo: Move this to a "cleaner" location so that application can
// also access it (i.e. CMainMenuState)
#include "../../Lib/giEXCore/giEXCore.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgEvent;
class cgRenderDriver;
class cgImage;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {365D928D-B996-418F-B83C-54C2BDEA9342}
const cgUID RTID_TextureResource = {0x365D928D, 0xB996, 0x418F, {0xB8, 0x3C, 0x54, 0xC2, 0xBD, 0xEA, 0x93, 0x42}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgTexture (Class)
/// <summary>
/// Wrapper for managing texture resources.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTexture : public cgResource, public giMediaListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgTexture, cgResource, "Texture" )

public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    // Configuration information for media / video playback.
    struct MediaConfig
    {
        cgFloat     mipmapUpdateRate;   // Update mip levels lower than the top-most at this rate in frames per second.
    };

    /// <summary>Used to select / identify a specific face of a cube map.</summary>
    enum CubeFace
    {
        PositiveX = 0,
        NegativeX = 1,
        PositiveY = 2,
        NegativeY = 3,
        PositiveZ = 4,
        NegativeZ = 5
    
    }; // End enum CubeFace

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgTexture( cgUInt32 referenceId, const cgInputStream & stream, cgRenderDriver * driver, cgInt32 mipLevels = -1 );
             cgTexture( cgUInt32 referenceId, const cgImageInfo & description );
    virtual ~cgTexture( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgTexture      * createInstance          ( cgUInt32 referenceId, const cgInputStream & stream, cgRenderDriver * driver, cgInt32 mipLevels = -1 );
    static cgTexture      * createInstance          ( cgUInt32 referenceId, const cgImageInfo & description );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgImageInfo     & getInfo                 ( ) const;
    cgSize                  getSize                 ( ) const;
    void                    configureMedia          ( const MediaConfig & config );
    CubeFace                getCurrentCubeFace      ( ) const;
    void                    setCurrentCubeFace      ( CubeFace face );
    bool                    supportsLinearSampling  ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            update                  ( );
    virtual void          * lock                    ( cgUInt32 & pitch, cgUInt32 flags ) = 0;
    virtual void          * lock                    ( const cgRect & bounds, cgUInt32 & pitch, cgUInt32 flags ) = 0;
    virtual void            unlock                  ( bool updateMips = false ) = 0;
    virtual bool            getImageData            ( cgImage & imageOut ) = 0;
    virtual bool            updateMipLevels         ( ) = 0;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgResource)
    //-------------------------------------------------------------------------
    virtual bool            loadResource            ( );
    virtual bool            unloadResource          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (giMediaListener)
    //-------------------------------------------------------------------------
    virtual giEXResult      onNewVideoFrame         ( void * data, cgUInt32 size );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_TextureResource; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    // Describes where the texture is being loaded from
    enum TextureSource
    {
        Source_Stream               = 0,            // Texture is being loaded from the specified input stream.
        Source_Created              = 1,            // Texture is being created on request.
    };

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    TextureSource           mSource;                // The source from which the texture will be generated.
    cgImageInfo             mInfo;                  // Information about the texture
    cgInputStream           mInputStream;           // The source for the texture.
    cgInt32                 mMipLevels;             // The total number of mip levels to generate
    bool                    mAutoGenMips;           // Auto generation of mips should occur (for created texture types).
    CubeFace                mCurrentCubeFace;       // The currently selected cube face to use when reading / manipulating cube faces.
    bool                    mLocked;                // Is the buffer locked?
    void                  * mLockedBuffer;          // Pointer to the locked buffer data area
    CubeFace                mLockedCubeFace;        // The cube face that is currently locked (if at all)
    giMediaDecoder        * mMediaDecoder;          // Used for decoding supported video types.
    MediaConfig             mMediaConfig;           // Configuration for media playback.
    cgEvent               * mVideoSyncEvent;        // Signal / event used to inform main thread that new frame of video is available for display.
    cgDouble                mLastMipUpdateTime;     // Last time at which mip levels were re-generated during video playback.
    bool                    mMipsDirty;             // Are mip-maps dirty after video frame update?    
};

#endif // !_CGE_CGTEXTURE_H_