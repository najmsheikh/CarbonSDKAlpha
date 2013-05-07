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
// Name : cgUIControls.cpp                                                   //
//                                                                           //
// Desc : Built in user interface group box / control container.             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgGroupBoxControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgGroupBoxControl.h>
#include <Interface/Controls/cgLabelControl.h>

///////////////////////////////////////////////////////////////////////////////
// cgGroupBoxControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgGroupBoxControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupBoxControl::cgGroupBoxControl( ) : cgUIControl( Complex, _T("GroupFrame") )
{
    // Create the group box label
    mGroupLabel = CG_NULL;
    mOldBorder  = CG_NULL;

    // Set default padding.
    setPadding( 3, 5, 3, 3 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgGroupBoxControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupBoxControl::~cgGroupBoxControl()
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
void cgGroupBoxControl::dispose( bool bDisposeBase )
{
    // Replace old BorderTopBegin element with its original
    if ( mGroupLabel )
    {
        if ( mOldBorder )
        {
            mControlElements[ BorderTopBegin ] = mOldBorder;
            mOldBorder = CG_NULL;
        
        } // End if had previous
        else
        {
            mControlElements.erase( BorderTopBegin );
        
        } // End if none prior

        mGroupLabel->scriptSafeDispose();
        mGroupLabel = CG_NULL;
    
    } // End if already replaced

    // Pass through to base class if required.
    if ( bDisposeBase )
        cgUIControl::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGroupBoxControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIGroupBoxControl )
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
void cgGroupBoxControl::onInitControl( )
{
    // Update the group box label (where necessary)
    updateGroupLabel();
}

//-----------------------------------------------------------------------------
//  Name : setControlText () (Virtual)
/// <summary>
/// Update the common text string for the control (i.e. Caption for a
/// form, button text for a button etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupBoxControl::setControlText( const cgString & strText )
{
    // Skip if this is a no-op.
    if ( strText == mControlText )
        return;

    // Call base class implementation first
    cgUIControl::setControlText( strText );

    // If we have not yet been initialized, leave it at this point for now.
    if ( !mBuilt )
        return;

    // Otherwise we can update the group box label immediately.
    updateGroupLabel();
}

//-----------------------------------------------------------------------------
//  Name : updateGroupLabel () (Protected)
/// <summary>
/// Set, remove or update the group's label.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupBoxControl::updateGroupLabel( )
{
    // Replace old BorderTopBegin element with its original
    if ( mGroupLabel )
    {
        if ( mOldBorder )
        {
            mControlElements[ BorderTopBegin ] = mOldBorder;
            mOldBorder = CG_NULL;
        
        } // End if had previous
        else
        {
            mControlElements.erase( BorderTopBegin );
        
        } // End if none prior
    
    } // End if already replaced

    // Anything being added?
    if ( !mControlText.empty() )
    {
        // Destroy prior label
        if ( mGroupLabel )
            mGroupLabel->scriptSafeDispose();

        // Create a new label control.
        mGroupLabel = new cgLabelControl();

        // Replace BorderTopBegin element with label.
        mOldBorder = getControlElement( BorderTopBegin );
        mControlElements[ BorderTopBegin ] = mGroupLabel;

        // Set the label's correct management data.
        mGroupLabel->setManagementData( mUIManager, mUILayer, this, mRootForm );
        mGroupLabel->setParentVisible( isVisible() );
        mGroupLabel->setParentEnabled( isEnabled() );

        // Set initial left / right padding for size computation.
        cgRect Padding = mGroupLabel->getPadding();
        mGroupLabel->setPadding( 4, Padding.top, 4, Padding.bottom );
        
        // Allow the label to auto size based on the text set.
        mGroupLabel->setAutoSize( true );
        mGroupLabel->setControlText( mControlText );

        // Position in the center of the border by forcing negative padding.
        Padding = mGroupLabel->getPadding();
        const cgSize & Size = mGroupLabel->getSize();
        mGroupLabel->setPadding( Padding.left, (Padding.top - (Size.height/2)) + 1, Padding.right, (Padding.bottom + (Size.height/2)) - 1 );

    } // End if text supplied
    else
    {
        // Destroy group label if any existed previously.
        if ( mGroupLabel )
            mGroupLabel->scriptSafeDispose();
        mGroupLabel = CG_NULL;

    } // End if no text

    // Recompute this control's layout
    recomputeLayout();
}