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
// Name : cgUILayers.h                                                       //
//                                                                           //
// Desc : These classes are responsible for managing and rendering data      //
//        for an individual layer on the user interface. Each layer is       //
//        is essentially a slice of the interface depth that contains an     //
//        individual item such as a form or widget. System layers also exist //
//        which display system specific items such as the cursor.            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGUILAYERS_H_ )
#define _CGE_CGUILAYERS_H_

//-----------------------------------------------------------------------------
// cgUILayers Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>
#include <Interface/cgUITypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBillboardBuffer;
class cgBillboard2D;
class cgUIManager;
class cgUIControl;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgUILayer (Class)
/// <summary>
/// An interface layer contains the physical render data for each unique
/// 'depth slice' of the interface. For instance, each form will be on
/// a unique layer. The layer order can be swapped dynamically to allow
/// forms, widgets or other elements to be arranged correctly.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUILayer : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgUILayer, "UILayer" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgUILayer( cgUIManager * manager, cgUILayerType::Base type, cgInt32 layerDepth = -1 );
    virtual ~cgUILayer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgUILayerType::Base getLayerType        ( ) const { return mLayerType; }
    cgBillboardBuffer * getLayerBuffer      ( ) { return mBillboards; }
    bool                prepareLayer        ( const cgInputStream & textureFileStream );
    bool                endPrepareLayer     ( );
    cgUIManager       * getUIManager        ( ) { return mUIManager; }
    void                bringToFront        ( );
    void                sendToBack          ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void        render              ( );
    virtual bool        onMouseMove         ( const cgPoint & position, const cgPointF & offset ) { return false; }
    virtual bool        onMouseButtonDown   ( cgInt32 buttons, const cgPoint & position ) { return false; }
    virtual bool        onMouseButtonUp     ( cgInt32 buttons, const cgPoint & position ) { return false; }
    virtual bool        onMouseWheelScroll  ( cgInt32 delta, const cgPoint & position ) { return false; }
    virtual bool        onKeyDown           ( cgInt32 keyCode, cgUInt32 modifiers ) { return false; }
    virtual bool        onKeyUp             ( cgInt32 keyCode, cgUInt32 modifiers ) { return false; }
    virtual bool        onKeyPressed        ( cgInt32 keyCode, cgUInt32 modifiers ) { return false; }

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    void                dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUIManager       * mUIManager;     // The parent interface manager we belong to.
    cgBillboardBuffer * mBillboards;    // The billboard data for this layer
    cgInt32             mLayerDepth;    // Depth of this current layer (larger the number, higher the order)
    cgUILayerType::Base mLayerType;     // Type of this layer (i.e. System or User).
};

//-----------------------------------------------------------------------------
//  Name : cgUIControlLayer (Class)
/// <summary>
/// This is a layer which can contain a single item derived from
/// cgUIControl (such as a form).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUIControlLayer : public cgUILayer
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgUIControlLayer, cgUILayer, "ControlLayer" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgUIControlLayer( cgUIManager * manager, cgUILayerType::Base type, cgInt32 layerDepth = -1 );
    virtual ~cgUIControlLayer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        registerSkinElement ( const cgUISkinElement & element, const cgString & elementName );
    void                        attachControl       ( cgUIControl * control );
    const cgUISkinElement     * getSkinElement      ( const cgString & name );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUILayer)
    //-------------------------------------------------------------------------
    virtual void                render              ( );
    virtual bool                onMouseMove         ( const cgPoint & position, const cgPointF & offset );
    virtual bool                onMouseButtonDown   ( cgInt32 buttons, const cgPoint & position );
    virtual bool                onMouseButtonUp     ( cgInt32 buttons, const cgPoint & position );
    virtual bool                onMouseWheelScroll  ( cgInt32 delta, const cgPoint & position );
    virtual bool                onKeyDown           ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual bool                onKeyUp             ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual bool                onKeyPressed        ( cgInt32 keyCode, cgUInt32 modifiers );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    void                        dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUIControl           * mControl;       // The control attached to this layer.
    cgUISkinElement::Map    mSkinElements;  // All relevant elements loaded from the selected skin.
};

//-----------------------------------------------------------------------------
//  Name : cgUICursorLayer (Class)
/// <summary>
/// A system layer which controls the rendering of the system cursor.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUICursorLayer : public cgUILayer
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgUICursorLayer, cgUILayer, "CursorLayer" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgUICursorLayer( cgUIManager * manager );
    virtual ~cgUICursorLayer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                initialize      ( );
    void                selectCursor    ( const cgString & name );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUILayer)
    //-------------------------------------------------------------------------
    virtual void        render          ( );
    virtual bool        onMouseMove     ( const cgPoint & position, const cgPointF & offset );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    void                dispose         ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBillboard2D             * mCursor;            // Billboard containing the cursor
    cgPoint                     mCursorOffset;      // Amount to offset cursor billboard from the mouse position (hot point)
    const cgUICursorType      * mCurrentType;       // The currently selected cursor type.
    cgString                    mNextCursor;        // The name of the cursor to use the next time we render
    cgString                    mCurrentCursor;     // The name of the currently selected cursor (i.e. the last time we rendered).
    cgInt16                     mCurrentFrame;      // Current frame to select from the billboard group for an animating cursor
    cgFloat                     mAnimBeginTime;     // When did the animation start?
};

#endif // !_CGE_CGUILAYERS_H_