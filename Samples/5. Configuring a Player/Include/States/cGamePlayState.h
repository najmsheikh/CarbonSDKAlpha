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
// Name : cGamePlayState.h                                                   //
//                                                                           //
// Desc : Primary game state used to supply most functionality for the       //
//        demonstration project.                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#if !defined( _CGAMEPLAYSTATE_H_ )
#define _CGAMEPLAYSTATE_H_

//-----------------------------------------------------------------------------
// cGamePlayState Header Includes
//-----------------------------------------------------------------------------

// CGE Includes
#include <Math/cgMathTypes.h>
#include <States/cgAppStateManager.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;
class cgRenderView;
class cgObjectNode;
class cgCameraNode;

//-----------------------------------------------------------------------------
// Globally Unique Type ID(s)
//-----------------------------------------------------------------------------
// {E734A1F1-5D26-41CF-BC96-C396706D99F5}
const cgUID RTID_Sample_GamePlayState = {0xE734A1F1, 0x5D26, 0x41CF, {0xBC, 0x96, 0xC3, 0x96, 0x70, 0x6D, 0x99, 0xF5}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cGamePlayState (Class)
// Desc : Main game state class responsible for providing much of the
//        functionality of the Aspeqt viewer application.
//-----------------------------------------------------------------------------
class cGamePlayState : public cgAppState
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cGamePlayState( const cgString & stateId );
    virtual ~cGamePlayState();

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides gpAppState)
    //-------------------------------------------------------------------------
    // State Activation
    virtual bool            initialize              ( );
    virtual bool            begin                   ( );
    virtual void            end                     ( );

    // State Frame Processing
    virtual void            update                  ( );
    virtual void            render                  ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides gpReferenceTarget)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_Sample_GamePlayState; }
    virtual bool            queryReferenceType      ( const cgUID & Type ) const;

private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                    loadScene               ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgRenderView  * mSceneView;     // Main view into which scene is rendered.
    cgScene       * mScene;         // Main scene being visualized.
    cgObjectNode  * mPlayer;        // Object node used to represent dummy player object.
    cgCameraNode  * mCamera;        // Main scene camera.
    cgVector3       mLastCamPos;    // Last computed position of the camera for smoothing.
    
    // Head bob! :)
    cgFloat         mBobCycle;
    cgVector2       mLastBobOffset;

};

#endif // !_CGAMEPLAYSTATE_H_