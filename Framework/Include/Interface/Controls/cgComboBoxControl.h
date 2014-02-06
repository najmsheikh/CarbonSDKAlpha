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
// Name : cgComboBoxControl.h                                                //
//                                                                           //
// Desc : Built in user interface combo box control.                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCOMBOBOXCONTROL_H_ )
#define _CGE_CGCOMBOBOXCONTROL_H_

//-----------------------------------------------------------------------------
// cgComboBoxControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgButtonControl;
class cgTextBoxControl;
class cgListBoxControl;
class cgUIControlLayer;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {640ADE73-5881-41A3-9B74-87B1D99498CC}
const cgUID RTID_UIComboBoxControl  = {0x640ADE73, 0x5881, 0x41A3, {0x9B, 0x74, 0x87, 0xB1, 0xD9, 0x94, 0x98, 0xCC}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgComboBoxControl (Class)
/// <summary>
/// An interface combo box that can be attached to a form or as a child of
/// any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgComboBoxControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgComboBoxControl, cgUIControl, "ComboBox" )

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    // UI_ComboBox_OnSelectedIndexChangeArgs Message Data
    struct UI_ComboBox_OnSelectedIndexChangeArgs : public UIEventArgs
    {
        cgInt32 oldIndex;
        cgInt32 newIndex;
        
        // Constructor
        UI_ComboBox_OnSelectedIndexChangeArgs( cgInt32 _oldIndex, cgInt32 _newIndex ) : 
            oldIndex(_oldIndex), newIndex(_newIndex) {}

        // Conversion to script argument list
        virtual void toArgumentList( cgScriptArgument::Array & arguments ) const
        {
            arguments.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&oldIndex ) );
            arguments.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&newIndex ) );
        }
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgComboBoxControl( );
    virtual ~cgComboBoxControl( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgInt32                 addItem                 ( const cgString & value );
    void                    clear                   ( );
    void                    setSelectedIndex        ( cgInt32 index );
    cgInt32                 getSelectedIndex        ( ) const;
    void                    showDropDown            ( bool show );
    bool                    isDropDownVisible       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            onSelectedIndexChange   ( cgInt32 oldIndex, cgInt32 newIndex );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            onInitControl           ( );
    virtual void            onSize                  ( cgInt32 width, cgInt32 height );
    virtual void            onParentAttach          ( cgUIControl * parent );
    virtual void            setFont                 ( const cgString & fontName );
    virtual void            setTextColor            ( const cgColorValue & color );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_UIComboBoxControl; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;
    virtual bool            processMessage          ( cgMessage * pMessage );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgButtonControl   * mDropButton;        // The button which opens the combo box dropdown list
    cgTextBoxControl  * mTextBox;           // The text box for the (potentially editable) "text" portion of the combo
    cgListBoxControl  * mDropDownList;      // The list box to display.
    cgUIControlLayer  * mDropDownLayer;     // Control layer on which the drop down list is displayed.
    bool                mDropDownVisible;   // Is the drop down list currently visible?
    cgInt32             mSelectedIndex;     // The index of the currently selected item.
    cgStringArray       mItems;             // List of items represented by this list box.
};

#endif // !_CGE_CGCOMBOBOXCONTROL_H_