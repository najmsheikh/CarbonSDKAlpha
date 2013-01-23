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
#include <Interface/Controls/cgListBoxControl.h>
#include <Interface/cgUIManager.h>
#include <Interface/cgUILayers.h>
#include <Interface/cgUISkin.h>
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
    mSelectedIndex      = -1;
    mDropDownVisible    = false;
    mDropDownLayer      = CG_NULL;

    // Create the drop down list open button
    mDropButton = new cgButtonControl( );
    addChildControl( mDropButton );

    // Create the text box for editable combos
    mTextBox = new cgTextBoxControl( _T("") );
    addChildControl( mTextBox );
    mTextBox->setReadOnly( true );
    mTextBox->setEnabled( false );

    // Create the drop down list control (hidden initially).
    mDropDownList = new cgListBoxControl( );
    mDropDownList->setVisible( false );
}

//-----------------------------------------------------------------------------
//  Name : ~cgComboBoxControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgComboBoxControl::~cgComboBoxControl()
{
    dispose(false);
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgComboBoxControl::dispose( bool disposeBase )
{
    // Release allocated memory
    mItems.clear();

    // Destroy the drop down list layer.
    if ( mDropDownLayer && mUIManager )
        mUIManager->removeLayer( mDropDownLayer, true );
    else if ( mDropDownList )
        mDropDownList->scriptSafeDispose();

    // Clear variables (controls are not owned by us).
    mDropButton         = CG_NULL;
    mTextBox            = CG_NULL;
    mDropDownList       = CG_NULL;
    mDropDownLayer      = CG_NULL;
    mDropDownVisible    = false;
    mSelectedIndex      = -1;

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
bool cgComboBoxControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIComboBoxControl )
        return true;

    // Supported by base?
    return cgUIControl::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : onParentAttach () (Virtual)
/// <summary>
/// This method is called when a control is first attached to a new parent
/// control.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::onParentAttach( cgUIControl * parent )
{
    // Attach drop down list to a custom top level control layer.
    mDropDownLayer = new cgUIControlLayer( mUIManager, cgUILayerType::SystemLayer, -1 );
    mDropDownLayer->attachControl( mDropDownList );
    mDropDownList->setManagementData( mUIManager, mDropDownLayer, CG_NULL, CG_NULL );
    mUIManager->addLayer( mDropDownLayer );

    // Get the currently selected skin
    cgUISkin * pCurrentSkin = mUIManager->getCurrentSkin();
    if ( !pCurrentSkin )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create combo box drop down layer '%s' because no valid skin has been selected.\n"), getName().c_str() );
        return;

    } // End if failed to get skin

    // Begin preparing the layer buffer.
    if ( !mDropDownLayer->prepareLayer( pCurrentSkin->getTextureName() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to begin preparing render buffer for combo box '%s' drop down list.\n"), getName().c_str() );
        return;

    } // End if failed to prepare

    // Add all the required elements from the skin to the control layer in order
    // for it to access frame / frame group data. Uses named frames / groups to allow the
    // information for the elements to be easily retrieved later.
    if ( !pCurrentSkin->prepareControlFrames( mDropDownLayer, cgUIFormStyle::Overlapped ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to populate control layer frameset information for combo box '%s' drop down list.\n"), getName().c_str() );
        return;

    } // End if failed to prepare

    // Build the list box control data
    if ( !mDropDownList->build( ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to build final render data for combo box '%s' drop down list.\n"), getName().c_str() );
        return;

    } // End if failed to build

    // Finalize the billboard buffer preparation
    if ( !mDropDownLayer->endPrepareLayer() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to complete preperation of render data for combo box '%s' drop down list.\n"), getName().c_str() );
        return;

    } // End if failed to prepare
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
    mDropDownList->registerEventHandler( cgSystemMessages::UI_ListBox_OnSelectedIndexChange, getReferenceId() );
    mDropDownList->registerEventHandler( cgSystemMessages::UI_OnLostFocus, getReferenceId() );
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Called in response to a resize event.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    // Pass through to base class first
    cgUIControl::onSize( nWidth, nHeight );

    // Reposition drop down button (ensure it fills entire control area irrespective of the padding)
    cgRect controlArea = getControlArea( cgControlCoordinateSpace::ClientRelative );
    mDropButton->setPosition( controlArea.right - controlArea.height(), controlArea.top );
    mDropButton->setSize( controlArea.height(), controlArea.height() );
    
    // Set text box to fill client area (minus drop button)
    cgRect clientArea = getClientArea( cgControlCoordinateSpace::ControlRelative );
    mTextBox->setSize( mDropButton->getPosition().x - clientArea.left, clientArea.height() );
}

//-----------------------------------------------------------------------------
//  Name : setFont () (Virtual)
/// <summary>
/// Set the font to use when rendering this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::setFont( const cgString & strFont )
{
    // Call base class implementation.
    cgUIControl::setFont( strFont );

    // Pass on to underlying text control.
    mTextBox->setFont( strFont );
    mDropDownList->setFont( strFont );
}

//-----------------------------------------------------------------------------
//  Name : setTextColor () (Virtual)
/// <summary>
/// Set the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::setTextColor( const cgColorValue & Color )
{
    // Call base class implementation
    cgUIControl::setTextColor( Color );
    
    // Pass on to underlying text control.
    mTextBox->setTextColor( Color );
    mDropDownList->setTextColor( Color );
}

//-----------------------------------------------------------------------------
//  Name : addItem()
/// <summary>
/// Add a new string to the list box item array. Returns the index of the
/// newly added item.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgComboBoxControl::addItem( const cgString & value )
{
    // Add the item to the list.
    mItems.push_back( value );

    // Add item to the drop down list too.
    mDropDownList->addItem( value );

    // Return the index of the most recently added item.
    return (cgInt32)mItems.size() - 1;
}

//-----------------------------------------------------------------------------
//  Name : onSelectedIndexChange() (Virtual)
/// <summary>
/// Triggered whenever a new list box item / row is selected in some way.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::onSelectedIndexChange( cgInt32 oldIndex, cgInt32 newIndex )
{
    // Raise the event with registered targets / script
    raiseEvent( cgSystemMessages::UI_ComboBox_OnSelectedIndexChange, &UI_ComboBox_OnSelectedIndexChangeArgs(oldIndex, newIndex) );
}

//-----------------------------------------------------------------------------
//  Name : setSelectedIndex()
/// <summary>
/// Select a new item from the list by supplying a valid index in the range
/// 0 through <list size> - 1. Specifying a value of -1 will deselect the
/// currently selected item.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::setSelectedIndex( cgInt32 index )
{
    // Clamp the index to the last item in the list.
    index = min( (cgInt32)mItems.size() - 1, index );

    // Update the currently selected index.
    cgInt32 oldIndex = mSelectedIndex;
    mSelectedIndex = index;
    if ( mSelectedIndex < -1 )
        mSelectedIndex = -1;

    // Update the contents of the text control.
    mTextBox->setControlText( (mSelectedIndex >= 0) ? mItems[mSelectedIndex] : cgString::Empty );

    // Set the selected index of the list box to match.
    mDropDownList->setSelectedIndex( mSelectedIndex );

    // Raise event.
    onSelectedIndexChange( oldIndex, mSelectedIndex );
}

//-----------------------------------------------------------------------------
//  Name : getSelectedIndex()
/// <summary>
/// Retrieve the index of the currently selected item, or -1 if nothing is
/// currently selected.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgComboBoxControl::getSelectedIndex() const
{
    return mSelectedIndex;
}

//-----------------------------------------------------------------------------
//  Name : showDropDown ()
/// <summary>
/// Show or hide the drop down list overlay control.
/// </summary>
//-----------------------------------------------------------------------------
void cgComboBoxControl::showDropDown( bool show )
{
    mDropDownVisible = show;
    mDropDownList->setVisible( show );

    // Position drop down list under control.
    if ( show )
    {
        cgPoint position = getControlOrigin( cgControlCoordinateSpace::ScreenRelative );
        position.y += getSize().height;
        mDropDownList->setPosition( position );
        cgSize size = getSize();
        size.height = 50;
        mDropDownList->setSize( size );

        // Give the drop down list focus.
        mDropDownList->focus();

        // Make sure currently selected item is visible.
        mDropDownList->scrollToSelection();
    
    } // End if show
}

//-----------------------------------------------------------------------------
//  Name : isDropDownVisible ()
/// <summary>
/// Determine if the drop down list overlay control is currently visible.
/// </summary>
//-----------------------------------------------------------------------------
bool cgComboBoxControl::isDropDownVisible( ) const
{
    return mDropDownVisible;
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgComboBoxControl::processMessage( cgMessage * pMessage )
{
    // Retrieve the reference target
    cgReference * pTarget = cgReferenceManager::getReference( pMessage->fromId );

    // What is the message and from whom?
    if ( pTarget == mDropButton )
    {
        if ( pMessage->messageId == cgSystemMessages::UI_Button_OnClick )
        {
            // User clicked the drop down button to expand the combo.
            if ( mDropDownVisible )
                showDropDown(false);
            else
                showDropDown(true);

            // Processed message
            return true;

        } // End if OnClick
    
    } // End if message from negative button
    else if ( pTarget == mDropDownList )
    {
        if ( mDropDownVisible && pMessage->messageId == cgSystemMessages::UI_ListBox_OnSelectedIndexChange )
        {
            // User selected a new entry in the drop down list.
            cgListBoxControl::UI_ListBox_OnSelectedIndexChangeArgs * pArgs = (cgListBoxControl::UI_ListBox_OnSelectedIndexChangeArgs*)pMessage->messageData;

            // Hide the drop down.
            showDropDown( false );

            // Update selected index
            cgInt32 oldIndex = mSelectedIndex;
            mSelectedIndex = pArgs->newIndex;

            // Update the contents of the text control.
            mTextBox->setControlText( (mSelectedIndex >= 0) ? mItems[mSelectedIndex] : cgString::Empty );

            // Raise event.
            onSelectedIndexChange( oldIndex, mSelectedIndex );

            // Processed message
            return true;

        } // End if OnSelectedIndexChange
        else if ( mDropDownVisible && pMessage->messageId == cgSystemMessages::UI_OnLostFocus )
        {
            // Drop down list lost focus, close it.
            showDropDown(false);
            
            // Processed message
            return true;

        } // End if OnLostFocus
    
    } // End if list changed

    // Message was not processed
    return cgUIControl::processMessage( pMessage );
}