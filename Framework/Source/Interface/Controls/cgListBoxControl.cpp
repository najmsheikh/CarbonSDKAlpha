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
// Name : cgListBoxControl.cpp                                               //
//                                                                           //
// Desc : Built in user interface list box control.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgListBoxControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgListBoxControl.h>
#include <Interface/Controls/cgScrollBarControl.h>
#include <System/cgMessageTypes.h>

///////////////////////////////////////////////////////////////////////////////
// cgListBoxControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgListBoxControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgListBoxControl::cgListBoxControl( ) : cgUIControl( Complex, _T("ControlFrame") )
{
    // Initialize variables to sensible defaults
    mVerticalScrollAmount   = 0;

    // Create the vertical scroll bar that will be used to scroll the list.
    mVerticalScrollBar = new cgScrollBarControl( );
    addChildControl( mVerticalScrollBar );

    // We need to capture value change messages from the scroll bar
    mVerticalScrollBar->registerEventHandler( cgSystemMessages::UI_ScrollBar_OnValueChange, getReferenceId() );
}

//-----------------------------------------------------------------------------
//  Name : ~cgListBoxControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgListBoxControl::~cgListBoxControl()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgListBoxControl::dispose( bool disposeBase )
{
    // Release allocated memory
    mItems.clear();

    // Clear variables (controls are not owned by us).
    mVerticalScrollBar = CG_NULL;

    // Dispose of base if required.
    if ( disposeBase )
        cgUIControl::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgListBoxControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIListBoxControl )
        return true;

    // Supported by base?
    return cgUIControl::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : addItem()
/// <summary>
/// Add a new string to the list box item array. Returns the index of the
/// newly added item.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgListBoxControl::addItem( const cgString & value )
{
    mItems.push_back( value );
    return (cgInt32)mItems.size() - 1;
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Triggered whenever the layout of the control has been recomputed.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::onSize( cgInt32 width, cgInt32 height )
{
    // Call base class implementation
    cgUIControl::onSize( width, height );

    // Reposition scroll bar (ensure it fills entire client area irrespective of the padding)
    /*cgRect clientArea = getClientArea();
    mVerticalScrollBar->setPosition( ((clientArea.right - clientArea.left) - 15) + mPadding.right, -mPadding.top );
    mVerticalScrollBar->setSize( 15, (clientArea.bottom - clientArea.top) + mPadding.top + mPadding.bottom );
    mVerticalScrollBar->setVisible( true ); // Always visible in list boxes.*/

    // Reposition scroll bar (ensure it fills entire control area irrespective of the padding)
    cgRect controlArea = getControlArea( cgControlCoordinateSpace::ClientRelative );
    mVerticalScrollBar->setPosition( controlArea.right - 15, controlArea.top );
    mVerticalScrollBar->setSize( 15, controlArea.height() );
    mVerticalScrollBar->setVisible( true ); // Always visible in list boxes.
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgListBoxControl::processMessage( cgMessage * message )
{
    // Retrieve the message source
    cgReference * source = cgReferenceManager::getReference( message->fromId );

    // Was this our vertical scroll bar message?
    if ( source == mVerticalScrollBar && message->messageId == cgSystemMessages::UI_ScrollBar_OnValueChange )
    {
        cgScrollBarControl::UI_ScrollBar_OnValueChangeArgs * arguments = (cgScrollBarControl::UI_ScrollBar_OnValueChangeArgs*)message->messageData;

        // Store vertical scroll value
        cgInt32 oldScrollAmount = mVerticalScrollAmount;
        mVerticalScrollAmount = (cgInt32)arguments->value;

        /*// Recompute the text metrics to take into account this new offset if it is different
        if ( oldScrollAmount != mVerticalScrollAmount )
            computeTextMetrics();*/

        // Processed message
        return true;

    } // End if message from negative button

    // Message was not processed
    return cgUIControl::processMessage( message );
}