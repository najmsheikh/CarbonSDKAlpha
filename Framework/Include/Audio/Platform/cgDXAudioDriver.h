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
// Name : cgDXAudioDriver.h                                                  //
//                                                                           //
// Desc : The main device through which we may load / play sound effects and //
//        music (DirectX class.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDXAUDIODRIVER_H_ )
#define _CGE_CGDXAUDIODRIVER_H_

//-----------------------------------------------------------------------------
// cgAudioDriver Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Audio/cgAudioDriver.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct IDirectSound8;
struct IDirectSound3DListener;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {F52645B8-3BB0-43A3-9217-8A7B38D60676}
const cgUID RTID_DXAudioDriver = {0xF52645B8, 0x3BB0, 0x43A3, {0x92, 0x17, 0x8A, 0x7B, 0x38, 0xD6, 0x6, 0x76}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDXAudioDriver (Class)
/// <summary>
/// Device through which sound and music is played and managed.
/// </summary>
//-----------------------------------------------------------------------------
class cgDXAudioDriver : public cgAudioDriver
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDXAudioDriver( );
    virtual ~cgDXAudioDriver( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgAudioDriver)
    //-------------------------------------------------------------------------
    virtual bool                    initialize              ( cgResourceManager * resourceManager, cgAppWindow * focusWindow );

    // Configuration
    virtual cgConfigResult::Base    loadConfig              ( const cgString & fileName );
    virtual cgConfigResult::Base    loadDefaultConfig       ( );
    virtual bool                    saveConfig              ( const cgString & fileName );

    // 3D Audio
    virtual void                    set3DWorldScale         ( cgFloat unitsPerMeter );
    virtual void                    set3DListenerTransform  ( const cgTransform & t );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // Internal Methods
    IDirectSound8                 * getDirectSound          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_AudioDriver; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    IDirectSound3DListener        * get3DListenerInterface  ( );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                    apply3DSettings         ( );
    
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    IDirectSound8             * mDS;            // Main DirectSound object
    IDirectSound3DListener    * m3DListener;    // Cached reference to the 3D listener interface.
};

#endif // !_CGE_CGDXAUDIODRIVER_H_