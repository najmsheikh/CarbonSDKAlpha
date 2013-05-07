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
// Name : cgScrollBarControl.h                                               //
//                                                                           //
// Desc : Built in user interface scroll bar control.                        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCROLLBARCONTROL_H_ )
#define _CGE_CGSCROLLBARCONTROL_H_

//-----------------------------------------------------------------------------
// cgScrollBarControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgButtonControl;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {FBCF4B44-8108-4D36-8E2D-0D46A298C65C}
const cgUID RTID_UIScrollBarControl = {0xFBCF4B44, 0x8108, 0x4D36, {0x8E, 0x2D, 0xD, 0x46, 0xA2, 0x98, 0xC6, 0x5C}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgScrollBarControl (Class)
/// <summary>
/// A horizontal or vertical scroll bar control that supplies user
/// controllable numeric value. Also used internally by several controls
/// including the multi-line text box.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScrollBarControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgScrollBarControl, cgUIControl, "ScrollBar" )

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    // UI_ScrollBar_OnValueChange Message Data
    struct UI_ScrollBar_OnValueChangeArgs : public UIEventArgs
    {
        cgFloat value;
        
        // Constructor
        UI_ScrollBar_OnValueChangeArgs( cgFloat _value ) : 
            value(_value) {}

        // Conversion to script argument list
        virtual void toArgumentList( cgScriptArgument::Array & arguments ) const
        {
            arguments.push_back( cgScriptArgument( cgScriptArgumentType::Float, _T("float"), (void*)&value ) );
        }
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgScrollBarControl( bool horizontal = false );
    virtual ~cgScrollBarControl( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgFloat                 getValue            ( ) const;
    cgFloat                 getMinimumValue     ( ) const;
    cgFloat                 getMaximumValue     ( ) const;
    cgFloat                 getSmallStep        ( ) const;
    cgFloat                 getLargeStep        ( ) const;
    cgFloat                 getVisiblePercentage( ) const;
    void                    setValue            ( cgFloat value );
    void                    setMinimumValue     ( cgFloat value );
    void                    setMaximumValue     ( cgFloat value );
    void                    setValueRange       ( cgFloat minimum, cgFloat maximum );
    void                    setSmallStep        ( cgFloat value );
    void                    setLargeStep        ( cgFloat value );
    void                    setStepSizes        ( cgFloat smallStep, cgFloat largeStep );
    void                    setVisiblePercentage( cgFloat percent );
    void                    stepSmall           ( cgInt32 amount );
    void                    stepLarge           ( cgInt32 amount );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            onInitControl       ( );
    virtual void            onSize              ( cgInt32 width, cgInt32 height );
    virtual bool            onMouseWheelScroll  ( cgInt32 delta, const cgPoint & position );
    virtual bool            onMouseButtonDown   ( cgInt32 buttons, const cgPoint & position );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UIScrollBarControl; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    virtual bool            processMessage      ( cgMessage * message );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    updateThumbButton   ( );
    cgRect                  getScrollArea       ( ) const;
    cgRect                  getScrollArea       ( cgControlCoordinateSpace::Base origin ) const;
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool                mHorizontal;        // Should our scroll bar be horizontal instead of vertical?
    cgButtonControl   * mButtonNegative;    // Button which scrolls the bar in the negative direction
    cgButtonControl   * mButtonPositive;    // Button which scrolls the bar in the positive direction
    cgButtonControl   * mThumbButton;       // The sliding thumb track button, used to actually scroll the bar.
    cgPoint             mThumbTrackPos;     // Tracks the position of the cursor when the thumb button is pressed.
    cgFloat             mThumbTrackValue;   // Tracks the value of the scrollbar when the thumb button was first pressed.
    cgFloat             mMinimumValue;      // Minimum scroll bar value.
    cgFloat             mMaximumValue;      // Maximum scroll bar value.
    cgFloat             mValue;             // The current scroll bar value.
    cgFloat             mVisiblePercentage; // Value between 0 and 1 which describes the amount of whatever we are scrolling that is already visible.
    cgFloat             mSmallStep;         // Small step describes how much to shift the value when the buttons are clicked / cursor keys are handled
    cgFloat             mLargeStep;         // Large step describes how much to shift the value when the channel is clicked / page keys are handled
};

#endif // !_CGE_CGSCROLLBARCONTROL_H_