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
// Name : cgComboBoxControl.cpp                                              //
//                                                                           //
// Desc : Built in user interface combo box control.                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgComboBoxControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgComboBoxControl.h>
#include <Interface/Controls/cgButtonControl.h>
#include <Interface/Controls/cgTextBoxControl.h>
#include <System/cgMessageTypes.h>

///////////////////////////////////////////////////////////////////////////////
// cgComboBoxControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgComboBoxControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgComboBoxControl::cgComboBoxControl( ) : cgUIControl( Complex, _T("ControlFrame") )
{
    // Initialize variables to sensible defaults

    // Create the drop down list open button
    mDropButton = new cgButtonControl( );
    addChildControl( mDropButton );

    // Create the text box for editable combos
    mTextBox = new cgTextBoxControl( _T("") );
    addChildControl( mTextBox );
}

//-----------------------------------------------------------------------------
//  Name : ~cgComboBoxControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgComboBoxControl::~cgComboBoxControl()
{
    // Release allocated memory
    // Clear variables
    // Note : We don't release the child controls because we don't directly own 
    //        them. These will be cleaned up automatically elsewhere.
    mDropButton = CG_NULL;
    mTextBox    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgComboBoxControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIComboBoxControl )
        return true;

    // Supported by base?
    return cgUIControl::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : onInitControl () (Virtual)
/// <summary>
/// Triggered whenever the control has been initialized.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::onInitControl( )
{
    // Call base class implementation
    cgUIControl::onInitControl();

    // Set the image for the drop down button
    mDropButton->setImage( _T("::DownArrow") );

    // Register this as the target for any applicable events raised by child controls
    mDropButton->registerEventHandler( cgSystemMessages::UI_Button_OnClick, getReferenceId() );
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Called in response to a resize event.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    cgRect rcClient;

    // Pass through to base class first
    cgUIControl::onSize( nWidth, nHeight );

    // Retrieve client area rectangle so we can corretly position the child controls
    rcClient = getClientArea( cgControlCoordinateSpace::ClientRelative );

    // Set position of drop down button to right hand side of box
    cgUInt32 nClientHeight = (rcClient.bottom-rcClient.top);
    mDropButton->setPosition( rcClient.right - nClientHeight, rcClient.top );

    // Set size such that it is square
    mDropButton->setSize( nClientHeight, nClientHeight );

    // Set text box to fill client area (minus drop button)
    mTextBox->setSize( ((rcClient.right - nClientHeight)-rcClient.left), nClientHeight );
}

//-----------------------------------------------------------------------------
//  Name : setControlText () (Virtual)
/// <summary>
/// Update the common text string for the control (i.e. Caption for a
/// form, button text for a button etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::setControlText( const cgString & strText )
{
    // Call base class implementation
    cgUIControl::setControlText( strText );

    // Update the child text box control with the combo text
    mTextBox->setControlText( strText );
}