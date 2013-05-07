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
// Name : cgUIControl.h                                                      //
//                                                                           //
// Desc : This module contains the base class from which all user interface  //
//        controls such as buttons, list boxes, check boxes, etc. derive in  //
//        order to provide much of the fundamental and common functionality. //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGUICONTROL_H_ )
#define _CGE_CGUICONTROL_H_

//-----------------------------------------------------------------------------
// cgUIControl Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Interface/cgUITypes.h>
#include <System/cgReference.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBillboard2D;
class cgUIForm;
class cgUIManager;
class cgUIControlLayer;
class cgScriptObject;
class cgScript;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {DED4AE72-104F-465A-BE1B-09FE0002F096}
const cgUID RTID_UIControl = {0xDED4AE72, 0x104F, 0x465A, {0xBE, 0x1B, 0x9, 0xFE, 0x0, 0x2, 0xF0, 0x96}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgUIControl (Class)
/// <summary>
/// The base class responsible for providing the majority of the core
/// user interface control support. Individual control classes (i.e.
/// cgButtonControl) derive from this class and extend it in order to add
/// specific support for a particular type of control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUIControl : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgUIControl, cgReference, "UIControl" )

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    enum ControlMode
    {
        Simple = 0,
        Complex
    };

    enum ControlRenderMode
    {
        RenderMode_Normal      = 0,
        RenderMode_Hover       = 1,
        RenderMode_Pressed     = 2,
        RenderMode_Disabled    = 3
    };

    // UIEventArgs base structure
    struct UIEventArgs : public cgScriptCompatibleStruct
    {
    };

    // UI_OnSize Message Data
    struct UI_OnSizeArgs : public UIEventArgs
    {
        cgInt32 width;
        cgInt32 height;
        
        // Constructor
        UI_OnSizeArgs( cgInt32 _width, cgInt32 _height ) : 
            width(_width), height(_height) {}

        // Conversion to script argument list
        virtual void toArgumentList( cgScriptArgument::Array & args ) const
        {
            args.reserve(2);
            args.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&width ) );
            args.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&height ) );
        }

    };

    // UI_OnMouseMove Message Data
    struct UI_OnMouseMoveArgs : public UIEventArgs
    {
        cgPoint     position;
        cgPointF    offset;
        
        // Constructor
        UI_OnMouseMoveArgs( const cgPoint & _position, const cgPointF & _offset ) : 
            position(_position), offset(_offset) {}

        // Conversion to script argument list
        virtual void toArgumentList( cgScriptArgument::Array & args ) const
        {
            args.reserve(2);
            args.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const Point&"), (void*)&position ) );
            args.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const PointF&"), (void*)&offset ) );
        }
    };

    // UI_OnMouseDown/UI_OnMouseUp Message Data
    typedef struct UI_MouseButtonArgs : public UIEventArgs
    {
        cgInt32 buttons;
        cgPoint position;
        
        // Constructor
        UI_MouseButtonArgs( cgInt32 _buttons, const cgPoint & _position ) : 
            buttons(_buttons), position(_position) {}

        // Conversion to script argument list
        virtual void toArgumentList( cgScriptArgument::Array & args ) const
        {
            args.reserve(2);
            args.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&buttons ) );
            args.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const Point&"), (void*)&position ) );
        }
    
    } UI_OnMouseButtonDownArgs, UI_OnMouseButtonUpArgs;

    // UI_OnMouseWheelScroll Message Data
    struct UI_OnMouseWheelScrollArgs : public UIEventArgs
    {
        cgInt32 delta;
        cgPoint position;
        
        // Constructor
        UI_OnMouseWheelScrollArgs( cgInt32 _delta, const cgPoint & _position ) : 
            delta(_delta), position(_position) {}

        // Conversion to script argument list
        virtual void toArgumentList( cgScriptArgument::Array & args ) const
        {
            args.reserve(2);
            args.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&delta ) );
            args.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const Point&"), (void*)&position ) );
        }
    };

    // UI_OnKeyDown/UI_OnKeyUp/UI_OnKeyPressed Message Data
    typedef struct UI_KeyEventArgs : public UIEventArgs
    {
        cgInt32  keyCode;
        cgUInt32 modifiers;
        
        // Constructor
        UI_KeyEventArgs( cgInt32 _keyCode, cgUInt32 _modifiers ) : 
            keyCode(_keyCode), modifiers(_modifiers) {}

        // Conversion to script argument list
        virtual void toArgumentList( cgScriptArgument::Array & args ) const
        {
            args.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&keyCode ) );
            args.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("uint"), (void*)&modifiers ) );
        }
    
    } UI_OnKeyDownArgs, UI_OnKeyUpArgs, UI_OnKeyPressedArgs;

    // Callback function pointer type for raising events directly
    // rather than using the messaging system.
    typedef bool   (*UIEventCallback)( cgUIControl * sender, void * args, void * context );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgUIControl( ControlMode mode, const cgString & skinElementName );
    virtual ~cgUIControl( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_UIControl; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgUIForm          * getRootForm         ( ) const { return mRootForm; }
    cgUIControlLayer  * getControlLayer     ( ) const { return mUILayer; }
    cgUIManager       * getUIManager        ( ) const { return mUIManager; }
    const cgString    & getName             ( ) const { return mControlName; }
    void                setName             ( const cgString & name ) { mControlName = name; }
    bool                addChildControl     ( cgUIControl * child );
    cgRect              getControlArea      ( ) const;
    cgPoint             getControlOrigin    ( ) const;
    cgRect              getClientArea       ( ) const;
    cgSize              getClientSize       ( ) const;
    cgPoint             getClientOrigin     ( ) const;
    cgRect              getControlArea      ( cgControlCoordinateSpace::Base origin ) const;
    cgPoint             getControlOrigin    ( cgControlCoordinateSpace::Base origin ) const;
    cgRect              getClientArea       ( cgControlCoordinateSpace::Base origin ) const;
    cgPoint             getClientOrigin     ( cgControlCoordinateSpace::Base origin ) const;
    cgPoint             clientToScreen      ( const cgPoint & point ) const;
    cgPoint             controlToScreen     ( const cgPoint & point ) const;
    cgPoint             screenToClient      ( const cgPoint & point ) const;
    cgPoint             screenToControl     ( const cgPoint & point ) const;
    bool                pointInControl      ( const cgPoint & point ) const;
    bool                loadControlText     ( cgInputStream stream );
    const cgString    & getControlText      ( ) const { return mControlText; }
    const cgSize      & getSize             ( ) const { return mSize; }
    const cgSize      & getMinimumSize      ( ) const { return mMinimumSize; }
    const cgSize      & getMaximumSize      ( ) const { return mMaximumSize; }
    const cgPoint     & getPosition         ( ) const { return mPosition; }
    const cgRect      & getPadding          ( ) const { return mPadding; }
    bool                canGainFocus        ( ) const;
    bool                canGainFocus        ( bool includeParentState ) const;
    void                setCanGainFocus     ( bool enabled );
    bool                isVisible           ( ) const;
    bool                isVisible           ( bool includeParentState ) const;
    bool                isEnabled           ( ) const;
    bool                isEnabled           ( bool includeParentState ) const;
    void                setEnabled          ( bool enabled );
    cgString            getFont             ( ) const;
    const cgColorValue& getTextColor        ( ) const;
    void                setBackgroundOpacity( cgFloat opacity );
    cgFloat             getBackgroundOpacity( ) const;
    void                setDockMode         ( cgDockMode::Base mode );
    cgDockMode::Base    getDockMode         ( ) const;
    void                focus               ( );

    // Message registration.
    void                registerEventHandler( cgUInt32 message, const cgString & scriptHandler, cgScriptObject * scriptObject );
    void                registerEventHandler( cgUInt32 message, cgInt32 referenceId );
    void                registerEventHandler( cgUInt32 message, UIEventCallback func, void * context );
    void                raiseEvent          ( cgUInt32 message, UIEventArgs * data );

    // System methods.
    void                setManagementData       ( cgUIManager * manager, cgUIControlLayer * layer, cgUIControl * parent, cgUIForm * rootForm );
    void                setParentEnabled        ( bool enabled );
    void                setParentCanGainFocus   ( bool enabled );
    void                setRenderMode           ( ControlRenderMode mode );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool        build               ( );
    virtual void        setControlText      ( const cgString & text ) { mControlText = text; }
    virtual void        setFont             ( const cgString & name );
    virtual void        setTextColor        ( const cgColorValue & color );
    virtual void        setMinimumSize      ( const cgSize & size );
    virtual void        setMinimumSize      ( cgInt32 width, cgInt32 height );
    virtual void        setMaximumSize      ( const cgSize & size );
    virtual void        setMaximumSize      ( cgInt32 width, cgInt32 height );
    virtual void        setPadding          ( const cgRect & padding );
    virtual void        setPadding          ( cgInt32 left, cgInt32 top, cgInt32 right, cgInt32 bottom );
    virtual void        setSize             ( const cgSize & size );
    virtual void        setSize             ( cgInt32 width, cgInt32 height );
    virtual void        setPosition         ( const cgPoint & position );
    virtual void        setPosition         ( cgInt32 x, cgInt32 y );
    virtual void        setVisible          ( bool visible );
    virtual void        setParentVisible    ( bool visible );
    virtual void        move                ( cgInt32 x, cgInt32 y );
    virtual void        renderSecondary     ( );
    virtual void        onInitControl       ( );
    virtual bool        onMouseMove         ( const cgPoint & position, const cgPointF & offset );
    virtual bool        onMouseButtonDown   ( cgInt32 buttons, const cgPoint & position );
    virtual bool        onMouseButtonUp     ( cgInt32 buttons, const cgPoint & position );
    virtual bool        onMouseWheelScroll  ( cgInt32 delta, const cgPoint & position );
    virtual bool        onKeyDown           ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual bool        onKeyUp             ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual bool        onKeyPressed        ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual void        onLostFocus         ( );
    virtual void        onGainFocus         ( );
    virtual void        onSize              ( cgInt32 width, cgInt32 height );
    virtual void        onScreenLayoutChange( );
    virtual void        onParentAttach      ( cgUIControl * parent );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    enum ControlElements
    {
        BorderTopLeft = 0,
        BorderTopBegin,
        BorderTopFiller,
        BorderTopEnd,
        BorderTopRight,
        BorderLeftBegin,
        BorderLeftFiller,
        BorderLeftEnd,
        BorderRightBegin,
        BorderRightFiller,
        BorderRightEnd,
        BorderBottomLeft,
        BorderBottomBegin,
        BorderBottomFiller,
        BorderBottomEnd,
        BorderBottomRight,
        Background
    };

    enum ControlState
    {
        State_Active   = 0,
        State_Inactive = 1
    };

    // Contains the frame and group indices that should be set for this control state
    struct RenderFrameDesc
    {
        cgInt16                 frameIndex;
        const cgUISkinElement * originalElement;
        void                  * activeRegion;
        void                  * handleRegions[8];

        // reset method to allow for post-load validation
        void reset()
        {
            // Clear vars
            frameIndex         = -1;
            originalElement    = CG_NULL;
            activeRegion       = CG_NULL;
            memset( handleRegions, 0, 8 * sizeof(void*) );
        
        } // End Method reset

        // isValid method used to determine if structure details are valid
        bool isValid() const
        {
            return (frameIndex >= 0);
        
        } // End Method isValid

    }; // End Struct RenderFrameDesc

    // Stores details about the billboard frames to use for each control state.
    struct ControlStateDesc
    {
        RenderFrameDesc renderModes[4];

        // reset method to allow for post-load validation
        void reset()
        {
            // Clear vars
            renderModes[RenderMode_Normal].reset();
            renderModes[RenderMode_Hover].reset();
            renderModes[RenderMode_Pressed].reset();
            renderModes[RenderMode_Disabled].reset();
        
        } // End Method reset

    }; // End Struct ControlStateDesc

    // Details regarding all registered event handlers
    struct EventHandler
    {
        // Messaging
        cgUInt32          referenceId;      // Send via message to this reference target,
        
        // Callback function
        UIEventCallback   func;             // call this function directly...
        void            * context;          // passing this context,

        // Script handler
        cgString          scriptHandler;    // or call this script function...
        cgScriptObject  * scriptObject;     // on this script object.

    }; // End Struct EventHandler
    
    CGE_MAP_DECLARE (cgUInt32, cgUIControl*, ControlIndexMap) // ToDo: unordered_map?
    CGE_LIST_DECLARE(cgUIControl*, ControlList)
    CGE_MAP_DECLARE (cgUInt32, EventHandler, EventHandlerMap) // ToDo: unordered_map?
        
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                        buildComplex            ( );
    bool                        buildSimple             ( );
    bool                        hasControlElement       ( ControlElements element ) const;
    cgUIControl               * getControlElement       ( ControlElements element );
    const cgUIControl         * getControlElement       ( ControlElements element ) const;
    void                        constructRegions        ( RenderFrameDesc & desc, const cgString & elementName );
    const RenderFrameDesc     * getCurrentRenderFrame   ( ) const;
    cgUIHandleType::Base        pointOverHandle         ( const cgPoint & screenPoint ) const;
    void                        recomputeLayout         ( );
    void                        updateScreenElements    ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUIManager       * mUIManager;             // The UI manager responsible for this control.
    cgUIControlLayer  * mUILayer;               // The interface layer in which this control exists.
    cgUIForm          * mRootForm;              // The root form of which this control is ultimately a child.
    cgUIControl       * mParent;                // The parent of this control.
    cgDockMode::Base    mDockMode;              // Automatic control docking mode. Used to aid in the automatic layout of controls.
    cgPoint             mPosition;              // Describes the current position of the interface control relative to its parent's client space.
    cgSize              mSize;                  // Describes the current size of the interface control.
    cgSize              mMinimumSize;           // Describes the minimum size allowed for the interface control.
    cgSize              mMaximumSize;           // Describes the maximum size allowed for the interface control.
    cgRect              mPadding;               // Describes an interior "margin" (padding) to apply to the client area which will allow controls to be inset.
    cgString            mSkinElement;           // The name of the skin element we are initializing our render data from
    ControlMode         mControlMode;           // The mode of the control, either simple (i.e. just a background element) or complex (separated border elements etc)
    ControlIndexMap     mControlElements;       // Contains a list of all of the core elements which are used to render the control
    ControlList         mChildren;              // arguments of all child controls.
    ControlStateDesc    mControlStateInfo[2];   // Contains the information required to render the control for each state (active, inactive etc.)
    ControlState        mCurrentState;          // What is the current state of the control (active / inactive)
    ControlRenderMode   mCurrentRenderMode;     // The currently selected render mode.
    cgBillboard2D     * mBillboard;             // The actual billboard for a 'Simple' interface control.
    cgString            mControlName;           // The name of the control
    cgString            mControlText;           // Text (if used) associated with the control
    EventHandlerMap     mEventHandlers;         // arguments of subscribed event handlers for any registered event in this control
    bool                mVisible;               // Is the control visible?
    bool                mParentVisible;         // Is the parent of this control visible?
    bool                mEnabled;               // Is the control enabled?
    bool                mParentEnabled;         // Is the parent of this control enabled?
    bool                mCanGainFocus;          // Can this control gain focus right now?
    bool                mParentCanGainFocus;    // Can the parent of this control gain focus right now?
    cgString            mFontName;              // Font to use for rendering this control
    bool                mBuilt;                 // Has the control been built yet?
    cgFloat             mBackgroundOpacity;     // Opacity level for the control background.
    cgColorValue        mControlTextColor;      // Set the default color of any text rendered for this control.
};

#endif // !_CGE_CGUICONTROL_H_