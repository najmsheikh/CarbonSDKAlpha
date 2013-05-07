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
// Name : cgUIControl.cpp                                                    //
//                                                                           //
// Desc : This module contains all classes responsible for display and       //
//        construction of user interface controls such as buttons, list      //
//        boxes, check boxes etc.                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgUIControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>
#include <Interface/cgUIManager.h>
#include <Interface/cgUILayers.h>
#include <Interface/cgUIForm.h>
#include <Resources/cgScript.h>
#include <Rendering/cgBillboardBuffer.h>
#include <System/cgMessageTypes.h>

// ToDo: Add TabStop / TabIndex support.
// ToDo: Tighten up the 'Focus Control' concept and find a nice way to show control with current focus.
// ToDo: Add 'CanGainFocus' support (defaults to true)
// ToDo: When a form script is destroyed, all event handlers still referencing it must be removed!
// ToDo: All controls should implement dispose method.

///////////////////////////////////////////////////////////////////////////////
// cgUIControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgUIControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUIControl::cgUIControl( ControlMode Mode, const cgString & strSkinElementName ) : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mUILayer            = CG_NULL;
    mUIManager          = CG_NULL;
    mParent             = CG_NULL;
    mRootForm           = CG_NULL;
    mDockMode           = cgDockMode::None;
    mControlMode        = Mode;
    mPosition           = cgPoint(0,0);
    mMinimumSize        = cgSize(0,0);
    mMaximumSize        = cgSize(0,0);
    mSize               = cgSize(500,500);
    mSkinElement        = strSkinElementName;
    mBillboard          = CG_NULL;
    mCurrentState       = State_Active;
    mCurrentRenderMode  = RenderMode_Normal;
    mBackgroundOpacity  = 1.0f;
    mVisible            = true;
    mParentVisible      = true;
    mEnabled            = true;
    mParentEnabled      = true;
    mCanGainFocus       = true;
    mParentCanGainFocus = true;
    mBuilt              = false;
    mControlTextColor   = cgColorValue( 1, 1, 1, 1 );

    // Set default padding
    mPadding            = cgRect( 0, 0, 0, 0 );
    
    // Reset structures
    mControlStateInfo[State_Active].reset();
    mControlStateInfo[State_Inactive].reset();
}

//-----------------------------------------------------------------------------
//  Name : ~cgUIControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUIControl::~cgUIControl()
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
void cgUIControl::dispose( bool bDisposeBase )
{
    ControlIndexMap::iterator itElement;
    ControlList::iterator     itControl;
    EventHandlerMap::iterator itEventHandler;
    cgInt32                   i, j, k;

    // Clear the captured / focus control if we're the one captured
    if ( mUIManager && mUIManager->getCapture() == this )
        mUIManager->setCapture( CG_NULL );
    if ( mUIManager && mUIManager->getFocus() == this )
        mUIManager->setFocus( CG_NULL );

    // Release all event handlers
    for ( itEventHandler = mEventHandlers.begin(); itEventHandler != mEventHandlers.end(); ++itEventHandler )
    {
        EventHandler & Handler = itEventHandler->second;
        if ( Handler.scriptObject )
            Handler.scriptObject->release();
    
    } // Next handler
    mEventHandlers.clear();

    // Release all elements responsible for rendering this control
    for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
    {
        if ( itElement->second )
            itElement->second->scriptSafeDispose();

    } // Next element
    mControlElements.clear();

    // Release all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        if ( *itControl )
            (*itControl)->scriptSafeDispose();

    } // Next child control
    mChildren.clear();

    // Clear variables
    mUILayer            = CG_NULL;
    mUIManager          = CG_NULL;
    mParent             = CG_NULL;
    mPosition           = cgPoint(0,0);
    mSize               = cgSize(500,500);
    mBillboard          = CG_NULL;
    mRootForm           = CG_NULL;

    // Release any required elements of the state structures
    for ( i = 0; i < 2; ++i )
    {
        // Iterate through each render mode
        for ( j = 0; j < 4; ++j )
        {
            RenderFrameDesc & Desc = mControlStateInfo[i].renderModes[j];
            
            // Release the active region object
            if ( Desc.activeRegion )
                DeleteObject( (HRGN)Desc.activeRegion );

            // Release any handle region objects
            for ( k = 0; k < 8; ++k )
            {
                if ( Desc.handleRegions[k] )
                    DeleteObject( (HRGN)Desc.handleRegions[k] );

            } // Next Handle Region

        } // Next Render Mode

    } // Next State

    // Reset structures
    mControlStateInfo[State_Active].reset();
    mControlStateInfo[State_Inactive].reset();

    // Dispose of base if required.
    if ( bDisposeBase )
        cgReference::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIControl )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : build() (Protected)
/// <summary>
/// Build the control's render data after loading, or after selecting a
/// new skin for instance.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::build( )
{
    ControlList::iterator     itControl;
    ControlIndexMap::iterator itElement;

    // Construct the control elements
    if ( mControlMode == Simple )
    {
        // Any previous billboard is no longer valid
        mBillboard = CG_NULL;

        // If the control specified no element name, there is no render data
        if ( !mSkinElement.empty() && !buildSimple( ) )
            return false;

    } // End if simple control construction
    else
    {
        // Release all elements responsible for rendering this control
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            if ( itElement->second )
                itElement->second->scriptSafeDispose();

        } // Next element
        mControlElements.clear();

        // Build this complex control type
        if ( !buildComplex( ) )
            return false;

    } // End if complex control construction

    // Compute the layout of all of the control elements
    recomputeLayout();

    // Construct all children.
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Build child control
        if ( !pControl->build( ) )
            return false;

    } // Next child control

    // Control has been constructed.
    mBuilt = true;

    // Trigger the OnInit method for this control (after children have been built)
    onInitControl( );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : buildSimple() (Protected)
/// <summary>
/// Construct this control using the 'Simple' control logic (i.e. this
/// is the core level, and contains the actual billboard).
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::buildSimple( )
{
    cgString strName;
    cgInt16  nFrameIndex;

    // Retrieve billboard buffer
    cgBillboardBuffer * pBillboards = getControlLayer()->getLayerBuffer();

    // Active states first
    ControlStateDesc * pDesc = &mControlStateInfo[State_Active];

    // Reset the structure
    pDesc->reset();

    // Retrieve the active - normal state (this is the only one that is absolutely required).
    strName     = mSkinElement;
    nFrameIndex = pBillboards->getFrameIndex( 0, strName );
    if ( nFrameIndex < 0 ) return false;

    // Store details
    pDesc->renderModes[RenderMode_Normal].frameIndex = nFrameIndex;
    constructRegions( pDesc->renderModes[RenderMode_Normal], strName );

    // Retrieve the active - hover state
    strName     = mSkinElement + _T(".Hover");
    nFrameIndex = pBillboards->getFrameIndex( 0, strName );
    if ( nFrameIndex >= 0 )
    {
        pDesc->renderModes[RenderMode_Hover].frameIndex = nFrameIndex;
        constructRegions( pDesc->renderModes[RenderMode_Hover], strName );
    
    } // End if element exists
    
    // Retrieve the active - pressed state
    strName     = mSkinElement + _T(".Pressed");
    nFrameIndex = pBillboards->getFrameIndex( 0, strName );
    if ( nFrameIndex >= 0 )
    {
        pDesc->renderModes[RenderMode_Pressed].frameIndex = nFrameIndex;
        constructRegions( pDesc->renderModes[RenderMode_Pressed], strName );
    
    } // End if element exists

    // Retrieve the active - disabled state
    strName     = mSkinElement + _T(".Disabled");
    nFrameIndex = pBillboards->getFrameIndex( 0, strName );
    if ( nFrameIndex >= 0 )
    {
        pDesc->renderModes[RenderMode_Disabled].frameIndex = nFrameIndex;
        constructRegions( pDesc->renderModes[RenderMode_Disabled], strName );
    
    } // End if element exists
    
    // Inactive states now
    pDesc = &mControlStateInfo[State_Inactive];

    // Reset the structure
    pDesc->reset();

    // Retrieve the active - hover state
    strName     = _T("Inactive::") + mSkinElement;
    nFrameIndex = pBillboards->getFrameIndex( 0, strName );
    if ( nFrameIndex >= 0 )
    {
        pDesc->renderModes[RenderMode_Normal].frameIndex = nFrameIndex;
        constructRegions( pDesc->renderModes[RenderMode_Normal], strName );
    
    } // End if element exists

    // Retrieve the active - hover state
    strName     = _T("Inactive::") + mSkinElement + _T(".Hover");
    nFrameIndex = pBillboards->getFrameIndex( 0, strName );
    if ( nFrameIndex >= 0 )
    {
        pDesc->renderModes[RenderMode_Hover].frameIndex = nFrameIndex;
        constructRegions( pDesc->renderModes[RenderMode_Hover], strName );
    
    } // End if element exists
    
    // Retrieve the active - pressed state
    strName     = _T("Inactive::") + mSkinElement + _T(".Pressed");
    nFrameIndex = pBillboards->getFrameIndex( 0, strName );
    if ( nFrameIndex >= 0 )
    {
        pDesc->renderModes[RenderMode_Pressed].frameIndex = nFrameIndex;
        constructRegions( pDesc->renderModes[RenderMode_Pressed], strName );
    
    } // End if element exists

    // Retrieve the active - disabled state
    strName     = _T("Inactive::") + mSkinElement + _T(".Disabled");
    nFrameIndex = pBillboards->getFrameIndex( 0, strName );
    if ( nFrameIndex >= 0 )
    {
        pDesc->renderModes[RenderMode_Disabled].frameIndex = nFrameIndex;
        constructRegions( pDesc->renderModes[RenderMode_Disabled], strName );
    
    } // End if element exists

    // Create a new billboard and add it to the buffer
    mBillboard = new cgBillboard2D( );
    if ( pBillboards->addBillboard( mBillboard ) < 0 )
        return false;
    
    // Set the billboard's properties (have it auto-size when setting the frame).
    cgPoint ptCurrent = getControlOrigin( cgControlCoordinateSpace::ScreenRelative );
    mBillboard->setPosition( (cgFloat)ptCurrent.x, (cgFloat)ptCurrent.y, 0 );
    mBillboard->setVisible( isVisible( ) );
    mBillboard->setColor( cgColorValue( 1, 1, 1, mBackgroundOpacity ) );
    if ( isEnabled() )
        mBillboard->setFrame( mControlStateInfo[State_Active].renderModes[RenderMode_Normal].frameIndex, true );
    else
        mBillboard->setFrame( mControlStateInfo[State_Active].renderModes[RenderMode_Disabled].frameIndex, true );
    
    // Our initial size should match the billboard frame
    mSize.width  = (cgInt32)mBillboard->getSize().width;
    mSize.height = (cgInt32)mBillboard->getSize().height;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : constructRegions() (Protected)
/// <summary>
/// If this element has any region points, build the region information
/// to allow us to perform more accurate 'pointInControl' detection etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::constructRegions( RenderFrameDesc & Desc, const cgString & strElementName )
{
    cgInt32 i;

    // Get the skin element from the layer
    const cgUISkinElement * pElement = getControlLayer()->getSkinElement( strElementName );
    if ( !pElement ) return;

    // Store the element pointer in the frame descriptor to allow
    // other parts of the system to query original values later.
    Desc.originalElement = pElement;

    // Does the element have any region data at all?
    if ( pElement->activeRegion.size() > 0 )
        Desc.activeRegion = (void*)::CreatePolygonRgn( (POINT*)&pElement->activeRegion[0], (int)pElement->activeRegion.size(), WINDING );

    // Build any handle rehion which may have been defined
    for ( i = 0; i < 8; ++i )
    {
        // Does the element have any handle region data?
        if ( pElement->handleRegions[i].size() > 0 )
            Desc.handleRegions[i] = (void*)::CreatePolygonRgn( (POINT*)&(pElement->handleRegions[i])[0], (int)pElement->handleRegions[i].size(), WINDING );

    } // Next Handle Region
}

//-----------------------------------------------------------------------------
//  Name : buildComplex() (Virtual, Protected)
/// <summary>
/// Construct this control using the 'Complex' control logic (i.e. this
/// is a high level control type which owns several 'Simple' controls
/// which make up the background with sizable borders etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::buildComplex( )
{
    // Define element names to allow us to do this in a loop
    static const cgString ElementNames[17] = { _T("BorderTopLeft"), _T("BorderTopBegin"), _T("BorderTopFiller"),
                                               _T("BorderTopEnd"), _T("BorderTopRight"), _T("BorderLeftBegin"),
                                               _T("BorderLeftFiller"), _T("BorderLeftEnd"), _T("BorderRightBegin"),
                                               _T("BorderRightFiller"), _T("BorderRightEnd"), _T("BorderBottomLeft"),
                                               _T("BorderBottomBegin"), _T("BorderBottomFiller"), _T("BorderBottomEnd"),
                                               _T("BorderBottomRight"), _T("Background") };

    // Retrieve billboard buffer
    cgBillboardBuffer * pBillboards = getControlLayer()->getLayerBuffer();

    // Loop through each of the element types, and allocate (or not) as required
    for ( cgInt i = 0; i < 17; ++i )
    {
        // Construct the element 'frame' name as it will appear in the buffer.
        cgString strElementName = mSkinElement + _T(".") + ElementNames[i];

        // If the frame exists, build the new control element
        if ( pBillboards->getFrameIndex( 0, strElementName ) >= 0 )
        {
            // Allocate a new control and store it in our map
            cgUIControl * pControl = new cgUIControl( Simple, strElementName );
            pControl->setManagementData( mUIManager, mUILayer, this, mRootForm );
            pControl->setParentVisible( isVisible() );
            pControl->setParentEnabled( isEnabled() );
            pControl->setParentCanGainFocus( canGainFocus() );
            pControl->setBackgroundOpacity( getBackgroundOpacity() );
            mControlElements[ i ] = pControl;

            // Allow the control to build
            if ( !pControl->build( ) )
                return false;

        } // End if frame exists

    } // Next Element

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : recomputeLayout() (Protected)
/// <summary>
/// If the control is resized, this function will recompute all of the
/// positions and sizes of any control elements.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::recomputeLayout( )
{
    // Ensure element size is 0 initially. When populating, this allows
    // us to only populate the relevant axis size where applicable.
    cgSize ElementSizes[17];
    memset( ElementSizes, 0, 17 * sizeof(cgSize) );

    // If this is not a complex control, process the billboard only
    if ( mControlMode != Complex )
    {
        // Set the size of the billboard if any stored here
        if ( mBillboard )
        {
            cgPoint ptCurrent = getControlOrigin( cgControlCoordinateSpace::ScreenRelative );
            mBillboard->setPosition( (cgFloat)ptCurrent.x, (cgFloat)ptCurrent.y, 0 );
            mBillboard->setSize( cgSizeF( (cgFloat)mSize.width, (cgFloat)mSize.height ) );
            mBillboard->update();
        
        } // End if billboard exists

        // Trigger the onSize event for registered subscribers
        onSize( mSize.width, mSize.height );
        return;

    } // End if simple control

    // Loop through each of the element types, and allocate (or not) as required
    for ( cgInt i = 0; i < 17; ++i )
    {
        // Retrieve the child control element
        cgUIControl * pControl = getControlElement( (ControlElements)i );
        if ( !pControl ) continue;

        // Retrieve the size of the control for later comparison
        ElementSizes[i] = pControl->getSize();

    } // Next Element
    
    // Compute the size of the filler elements
    ElementSizes[ BorderTopFiller ].width    = mSize.width - (ElementSizes[ BorderTopLeft ].width + ElementSizes[ BorderTopBegin ].width + 
                                               ElementSizes[ BorderTopEnd ].width + ElementSizes[ BorderTopRight ].width );

    ElementSizes[ BorderBottomFiller ].width = mSize.width - (ElementSizes[ BorderBottomLeft ].width + ElementSizes[ BorderBottomBegin ].width + 
                                               ElementSizes[ BorderBottomEnd ].width + ElementSizes[ BorderBottomRight ].width );

    ElementSizes[ BorderLeftFiller ].height  = mSize.height - (ElementSizes[ BorderTopLeft ].height + ElementSizes[ BorderLeftBegin ].height + 
                                               ElementSizes[ BorderLeftEnd ].height + ElementSizes[ BorderBottomLeft ].height );

    ElementSizes[ BorderRightFiller ].height = mSize.height - (ElementSizes[ BorderTopRight ].height + ElementSizes[ BorderRightBegin ].height + 
                                               ElementSizes[ BorderRightEnd ].height + ElementSizes[ BorderBottomRight ].height );

    ElementSizes[ Background ].width         = mSize.width - (ElementSizes[ BorderTopLeft ].width + ElementSizes[ BorderTopRight ].width);
    ElementSizes[ Background ].height        = mSize.height - (ElementSizes[ BorderTopLeft ].height + ElementSizes[ BorderBottomLeft ].height);


    // Elements are relative to this control's client space like any other control.
    // Set our origin to the top left of our control relative to client space.
    cgPoint ptControlOrigin = getControlOrigin( cgControlCoordinateSpace::ClientRelative );
    
    // BorderTopLeft
    cgPoint ptCurrent = ptControlOrigin;
    cgUIControl * pControl  = getControlElement( BorderTopLeft );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderTopBegin
    ptCurrent.x += ElementSizes[ BorderTopLeft ].width;
    pControl     = getControlElement( BorderTopBegin );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderTopFiller
    ptCurrent.x += ElementSizes[ BorderTopBegin ].width;
    pControl     = getControlElement( BorderTopFiller );
    if ( pControl )
    {
        pControl->setPosition( ptCurrent );
        pControl->setSize( ElementSizes[ BorderTopFiller ] );

    } // End if control available

    // BorderTopEnd
    ptCurrent.x += ElementSizes[ BorderTopFiller ].width;
    pControl     = getControlElement( BorderTopEnd );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderTopRight
    ptCurrent.x += ElementSizes[ BorderTopEnd ].width;
    pControl     = getControlElement( BorderTopRight );
    if ( pControl )
        pControl->setPosition( ptCurrent );
    
    // BorderLeftBegin
    ptCurrent.x = ptControlOrigin.x;
    ptCurrent.y = ptControlOrigin.y + ElementSizes[ BorderTopLeft ].height;
    pControl    = getControlElement( BorderLeftBegin );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderLeftFiller
    ptCurrent.y += ElementSizes[ BorderLeftBegin ].height;
    pControl     = getControlElement( BorderLeftFiller );
    if ( pControl )
    {
        pControl->setPosition( ptCurrent );
        pControl->setSize( ElementSizes[ BorderLeftFiller ] );

    } // End if control available

    // BorderLeftEnd
    ptCurrent.y += ElementSizes[ BorderLeftFiller ].height;
    pControl     = getControlElement( BorderLeftEnd );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // Background
    ptCurrent.x = ptControlOrigin.x + ElementSizes[ BorderTopLeft ].width;
    ptCurrent.y = ptControlOrigin.y + ElementSizes[ BorderTopLeft ].height;
    pControl    = getControlElement( Background );
    if ( pControl )
    {
        pControl->setPosition( ptCurrent );
        pControl->setSize( ElementSizes[ Background ] );

    } // End if control available

    // BorderRightBegin
    ptCurrent.x = ptControlOrigin.x + (mSize.width - ElementSizes[ BorderTopRight ].width);
    ptCurrent.y = ptControlOrigin.y + ElementSizes[ BorderTopRight ].height;
    pControl    = getControlElement( BorderRightBegin );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderRightFiller
    ptCurrent.y += ElementSizes[ BorderRightBegin ].height;
    pControl     = getControlElement( BorderRightFiller );
    if ( pControl )
    {
        pControl->setPosition( ptCurrent );
        pControl->setSize( ElementSizes[ BorderRightFiller ] );

    } // End if control available

    // BorderRightEnd
    ptCurrent.y += ElementSizes[ BorderRightFiller ].height;
    pControl     = getControlElement( BorderRightEnd );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderBottomLeft
    ptCurrent.x = ptControlOrigin.x;
    ptCurrent.y = ptControlOrigin.y + (mSize.height - ElementSizes[ BorderBottomLeft ].height);
    pControl    = getControlElement( BorderBottomLeft );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderBottomBegin
    ptCurrent.x += ElementSizes[ BorderBottomLeft ].width;
    pControl     = getControlElement( BorderBottomBegin );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderBottomFiller
    ptCurrent.x += ElementSizes[ BorderBottomBegin ].width;
    pControl     = getControlElement( BorderBottomFiller );
    if ( pControl )
    {
        pControl->setPosition( ptCurrent );
        pControl->setSize( ElementSizes[ BorderBottomFiller ] );

    } // End if control available

    // BorderBottomEnd
    ptCurrent.x += ElementSizes[ BorderBottomFiller ].width;
    pControl     = getControlElement( BorderBottomEnd );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // BorderBottomRight
    ptCurrent.x += ElementSizes[ BorderBottomEnd ].width;
    pControl     = getControlElement( BorderBottomRight );
    if ( pControl )
        pControl->setPosition( ptCurrent );

    // Trigger the onSize event for registered subscribers
    onSize( mSize.width, mSize.height );
}

//-----------------------------------------------------------------------------
//  Name : updateScreenElements () (Protected, Recursive)
/// <summary>
/// Update the positions of every screen-space billboard that exists at or
/// below this control in the hierarchy. Although the parent relative position
/// of the control may not have changed, any change to position above this
/// control in the hierarchy will require that the screen space position of
/// any associated billboard be updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::updateScreenElements( )
{
    // Pass through to all child controls first.
    ControlList::iterator itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;

        // Force child to recompute its position
        pControl->updateScreenElements( );

    } // Next child control

    // Reposition the control elements if it has any
    if ( mControlMode == Complex )
    {
        ControlIndexMap::iterator itElement;
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            cgUIControl * pControl = itElement->second;
            if ( !pControl ) continue;

            // Force element to recompute its position
            pControl->updateScreenElements( );

        } // Next element

    } // End if complex control

    // Set the size of the billboard if any stored here
    if ( mBillboard )
    {
        // Update the billboard
        cgPoint ptCurrent = getControlOrigin( cgControlCoordinateSpace::ScreenRelative );
        mBillboard->setPosition( (cgFloat)ptCurrent.x, (cgFloat)ptCurrent.y, 0 );
        mBillboard->update();

    } // End if billboard exists
}

//-----------------------------------------------------------------------------
//  Name : renderSecondary () (Virtual)
/// <summary>
/// Render any secondary elements for this control. This would include
/// items such as the text for any controls.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::renderSecondary( )
{
    ControlList::iterator itControl;

    // Allow control elements to render secondary elements if they have the need to
    // (i.e. they may have been replaced with physical controls, such as with the group
    // box control replacing BorderTopBegin with a label control).
    if ( mControlMode == Complex )
    {
        for ( cgInt32 i = 0; i < 17; ++i )
        {
            // Allow this control element to render secondary elements if desired
            cgUIControl * pControl = getControlElement( (ControlElements)i);
            if ( pControl ) pControl->renderSecondary();            
        
        } // Next Element

    } // End if complex control

    // Pass through to all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Render child
        pControl->renderSecondary( );

    } // Next child control
}

//-----------------------------------------------------------------------------
//  Name : hasControlElement () (Protected)
/// <summary>
/// Determine if the specified control element is contained in the skin
/// definition for this control (i.e. does it have a BorderTopBegin?).
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::hasControlElement( ControlElements Element ) const
{
    return (mControlElements.find( Element ) != mControlElements.end());
}

//-----------------------------------------------------------------------------
//  Name : getControlElement () (Protected)
/// <summary>
/// Retrieve the control element. If it does not exist, just return CG_NULL.
/// </summary>
//-----------------------------------------------------------------------------
cgUIControl * cgUIControl::getControlElement( ControlElements Element )
{
    ControlIndexMap::iterator itElement;
    itElement = mControlElements.find( Element );
    if ( itElement == mControlElements.end() ) return CG_NULL;
    return itElement->second;
}

//-----------------------------------------------------------------------------
//  Name : getControlElement () (Protected, Overload)
/// <summary>
/// Retrieve the control element. If it does not exist, just return CG_NULL.
/// Note : Provides const pointer.
/// </summary>
//-----------------------------------------------------------------------------
const cgUIControl * cgUIControl::getControlElement( ControlElements Element ) const
{
    ControlIndexMap::const_iterator itElement;
    itElement = mControlElements.find( Element );
    if ( itElement == mControlElements.end() ) return CG_NULL;
    return itElement->second;
}

//-----------------------------------------------------------------------------
//  Name : setSize () (Virtual)
/// <summary>
/// Set the size of this control in screen space pixels.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setSize( const cgSize & Size )
{
    setSize( Size.width, Size.height );
}

//-----------------------------------------------------------------------------
//  Name : setSize () (Virtual)
/// <summary>
/// Set the size of this control in screen space pixels.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setSize( cgInt32 nWidth, cgInt32 nHeight )
{
    // Ignore input for axes disallowed by docking mode.
    if ( mDockMode == cgDockMode::Fill )
        return;
    if ( mDockMode == cgDockMode::Top || mDockMode == cgDockMode::Bottom )
        nWidth = mSize.width;
    if ( mDockMode == cgDockMode::Left || mDockMode == cgDockMode::Right )
        nHeight = mSize.height;
    
    // Compute size change.
    cgInt32 nDeltaX = nWidth  - mSize.width;
    cgInt32 nDeltaY = nHeight - mSize.height;
    
    // Prevent size from being smaller than that required for 
    // this control's client area (prevent area <= 0)
    cgRect rcClient = getClientArea();
    if ( ((rcClient.right - rcClient.left) + nDeltaX) < 1 )
        nDeltaX = -((rcClient.right - rcClient.left) - 1);
    if ( ((rcClient.bottom - rcClient.top) + nDeltaY) < 1 )
        nDeltaY = -((rcClient.bottom - rcClient.top) - 1);
    
    // Compute final size
    cgSize NewSize = mSize;
    NewSize.width  += nDeltaX;
    NewSize.height += nDeltaY;

    // Clamp to the minimum size (always).
    NewSize.width  = max( mMinimumSize.width, NewSize.width );
    NewSize.height = max( mMinimumSize.height, NewSize.height );

    // Optionally clamp to the maximum size
    if ( mMaximumSize.width > 0 )
        NewSize.width = min( mMaximumSize.width, NewSize.width );
    if ( mMaximumSize.height > 0 )
        NewSize.height = min( mMaximumSize.height, NewSize.height );

    // Skip if this is a no-op
    if ( NewSize == mSize )
        return;

    // Update the current size of the control only at this stage.
    mSize = NewSize;

    // Reposition and resize all of the control elements that
    // make up this complex control (i.e. all of the borders etc).
    // Note: This will automatically call the onSize handler.
    recomputeLayout();
}

//-----------------------------------------------------------------------------
//  Name : setMinimumSize () (Virtual)
/// <summary>
/// Set the minimum allowable size of this control in screen space pixels
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setMinimumSize( const cgSize & Size )
{
    setMinimumSize( Size.width, Size.height );
}

//-----------------------------------------------------------------------------
//  Name : setMinimumSize () (Virtual)
/// <summary>
/// Set the minimum allowable size of this control in screen space pixels
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setMinimumSize( cgInt32 nWidth, cgInt32 nHeight )
{
    mMinimumSize.width = nWidth;
    mMinimumSize.height = nHeight;

    // Update maximum size if minimum size is smaller (and has not been disabled)
    cgSize MaximumSize = mMaximumSize;
    if ( (MaximumSize.width > 0) && (mMaximumSize.width < nWidth) )
        MaximumSize.width = nWidth;
    if ( (MaximumSize.height > 0) && (mMaximumSize.height < nHeight) )
        MaximumSize.height = nHeight;

    // Update required?
    if ( MaximumSize.width != mMaximumSize.width || MaximumSize.height != mMaximumSize.height )
        setMaximumSize( MaximumSize );

    // Trigger a resize if the current size is smaller than this.
    if ( mSize.width < nWidth || mSize.height < nHeight )
        setSize( max( nWidth, mSize.width ), max( nHeight, mSize.height ) );
}

//-----------------------------------------------------------------------------
//  Name : setMaximumSize () (Virtual)
/// <summary>
/// Set the maximum allowable size of this control in screen space pixels
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setMaximumSize( const cgSize & Size )
{
    setMaximumSize( Size.width, Size.height );
}

//-----------------------------------------------------------------------------
//  Name : setMaximumSize () (Virtual)
/// <summary>
/// Set the maximum allowable size of this control in screen space pixels
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setMaximumSize( cgInt32 nWidth, cgInt32 nHeight )
{
    mMaximumSize.width = nWidth;
    mMaximumSize.height = nHeight;

    // Update minimum size if maximum size is smaller (and has not been disabled)
    cgSize MinimumSize = mMinimumSize;
    if ( (nWidth > 0) && (mMinimumSize.width > nWidth) )
        MinimumSize.width = nWidth;
    if ( (nHeight > 0) && (mMinimumSize.height > nHeight) )
        MinimumSize.height = nHeight;

    // Update required?
    if ( MinimumSize.width != mMinimumSize.width || MinimumSize.height != mMinimumSize.height )
        setMinimumSize( MinimumSize );

    // Trigger a resize if the current size is larger than this.
    if ( mSize.width > nWidth || mSize.height > nHeight )
        setSize( min( nWidth, mSize.width ), min( nHeight, mSize.height ) );
}

//-----------------------------------------------------------------------------
//  Name : setPadding () (Virtual)
/// <summary>
/// Set the *interior* margin (padding) to apply to the client area which 
/// will allow controls to be inset.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setPadding( const cgRect & Padding )
{
    setPadding( Padding.left, Padding.top, Padding.right, Padding.bottom );
}

//-----------------------------------------------------------------------------
//  Name : setPadding () (Virtual)
/// <summary>
/// Set the *interior* margin (padding) to apply to the client area which 
/// will allow controls to be inset.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setPadding( cgInt32 nLeft, cgInt32 nTop, cgInt32 nRight, cgInt32 nBottom )
{
    mPadding = cgRect( nLeft, nTop, nRight, nBottom );

    // We need to recompute this control's layout because child
    // elements are specified relative to client space.
    if ( mBuilt )
        recomputeLayout();
}

//-----------------------------------------------------------------------------
//  Name : setPosition () (Virtual)
/// <summary>
/// Set the position of this control, relative to the origin of the parent
/// control's client area origin.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setPosition( const cgPoint & Position )
{
    setPosition( Position.x, Position.y );
}

//-----------------------------------------------------------------------------
//  Name : setPosition () (Virtual)
/// <summary>
/// Set the position of this control, relative to its parent control, 
/// in screen space pixels.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setPosition( cgInt32 nX, cgInt32 nY )
{
    // If control is docked in 'fill' mode, ignore.
    if ( mDockMode == cgDockMode::Fill )
        return;
    
    // Store the new position components depending on docking constraints
    cgPoint newPosition = mPosition;
    if ( mDockMode == cgDockMode::None || mDockMode == cgDockMode::Top || mDockMode == cgDockMode::Bottom )
        newPosition.x = nX;
    if ( mDockMode == cgDockMode::None || mDockMode == cgDockMode::Left || mDockMode == cgDockMode::Right )
        newPosition.y = nY;

    // Was this a no-op?
    if ( newPosition == mPosition )
        return;
    mPosition = newPosition;

    // Update the final position of all child controls and billboards.
    updateScreenElements();
}

//-----------------------------------------------------------------------------
//  Name : setTextColor ()
/// <summary>
/// Set the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setTextColor( const cgColorValue & Color )
{
    mControlTextColor = Color;
}

//-----------------------------------------------------------------------------
//  Name : getTextColor ()
/// <summary>
/// Retreive the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgUIControl::getTextColor( ) const
{
    return mControlTextColor;
}

//-----------------------------------------------------------------------------
//  Name : setFont () (Virtual)
/// <summary>
/// Set the font to use when rendering this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setFont( const cgString & strFont )
{
    mFontName = strFont;
}

//-----------------------------------------------------------------------------
//  Name : getFont ()
/// <summary>
/// Retreive the the font in use when rendering this control.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgUIControl::getFont( ) const
{
    // Select the appropriate font.
    if ( !mFontName.empty() )
    {
        return mFontName;

    } // End if explicit font
    else
    {
        // Otherwise, use the parent's font.
        if ( mParent )
            return mParent->getFont();
        else if ( mRootForm && mRootForm != this )
            return mRootForm->getFont();

    } // End if default font

    // No font available
    return _T("");
}

//-----------------------------------------------------------------------------
//  Name : loadControlText ()
/// <summary>
/// Utility function to load the control text from the specified stream.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::loadControlText( cgInputStream Stream )
{
    STRING_CONVERT;

    // Open the stream
    if ( !Stream.open() )
        return false;

    // Read contents into a string.
    cgChar * pBuffer = new cgChar[(size_t)Stream.getLength()+1];
    Stream.read( pBuffer, (size_t)Stream.getLength() );
    pBuffer[Stream.getLength()] = '\0';

    // Close the stream and set this as our control text.
    Stream.close();
    setControlText( stringConvertA2CT( pBuffer ) );

    // Finish up
    delete []pBuffer;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : move () (Virtual)
/// <summary>
/// Move this control, relative to its parent control, by an amount
/// specified in in screen space pixels.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::move( cgInt32 nX, cgInt32 nY )
{
    // Update the position of the control
    setPosition( mPosition.x + nX, mPosition.y + nY );
}

//-----------------------------------------------------------------------------
//  Name : addChildControl ()
/// <summary>
/// Attach a control as a child of this control (i.e. button on a form).
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::addChildControl( cgUIControl * pChild )
{
    // Validate Requirements
    if ( !pChild )
        return false;

    // If the parent is already set, it's already been added somewhere
    if ( pChild->mParent )
        return false;

    // Set the child's parent control and manager objects
    pChild->setManagementData( mUIManager, mUILayer, this, mRootForm );
    pChild->setParentVisible( isVisible() );
    pChild->setParentEnabled( isEnabled() );
    pChild->setParentCanGainFocus( canGainFocus() );

    // Allow child to perform any necessary operations.
    pChild->onParentAttach( this );

    // Add this to the child control list
    mChildren.push_back( pChild );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setManagementData () (Protected, Recursive)
/// <summary>
/// Set all of the data required for the control to function. This will
/// also recurse into any already attached children and populate them.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setManagementData( cgUIManager * pManager, cgUIControlLayer * pLayer, cgUIControl * pParent, cgUIForm * pRootForm )
{
    ControlList::iterator   itControl;

    // Store details
    mUIManager        = pManager;
    mUILayer          = pLayer;
    mParent           = pParent;
    mRootForm         = pRootForm;
    
    // Pass through to all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Set child data
        pControl->setManagementData( pManager, pLayer, this, pRootForm );

    } // Next child control
}

//-----------------------------------------------------------------------------
//  Name : getControlArea ()
/// <summary>
/// Calculate the rectangle that describes the area of this control 
/// (including its borders) relative to the specified origin.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIControl::getControlArea( cgControlCoordinateSpace::Base Origin ) const
{
    cgPoint ptOrigin;
    cgRect  rcControl;
    
    // Compute actual control rectangle
    rcControl = cgRect( 0, 0, mSize.width, mSize.height );

    // Which origin would the caller like the rectangle relative to?
    switch ( Origin )
    {
        case cgControlCoordinateSpace::ClientRelative:
            // Return rectangle data relative to the client origin
            ptOrigin          = getControlOrigin( cgControlCoordinateSpace::ClientRelative );
            rcControl.left   += ptOrigin.x;
            rcControl.right  += ptOrigin.x;
            rcControl.top    += ptOrigin.y;
            rcControl.bottom += ptOrigin.y;
            break;

        case cgControlCoordinateSpace::ScreenRelative:
            // Return rectangle data relative to the screen origin
            ptOrigin          = getControlOrigin( cgControlCoordinateSpace::ScreenRelative );
            rcControl.left   += ptOrigin.x;
            rcControl.right  += ptOrigin.x;
            rcControl.top    += ptOrigin.y;
            rcControl.bottom += ptOrigin.y;
            break;

    } // End Switch Origin

    // Return the rectangle
    return rcControl;
}

//-----------------------------------------------------------------------------
//  Name : getControlOrigin ()
/// <summary>
/// Simply calculate the top left point of the control area in relation
/// to the specified origin.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgUIControl::getControlOrigin( cgControlCoordinateSpace::Base Origin ) const
{
    cgPoint ptOrigin(0,0);

    // Which origin would the caller like the data relative to?
    switch ( Origin )
    {
        case cgControlCoordinateSpace::ClientRelative:
            // Return origin data relative to the client origin
            ptOrigin   = getClientOrigin( cgControlCoordinateSpace::ControlRelative );
            ptOrigin.x = -ptOrigin.x;
            ptOrigin.y = -ptOrigin.y;
            break;

        case cgControlCoordinateSpace::ScreenRelative:
            // Return origin data relative to the screen origin
            ptOrigin = controlToScreen( ptOrigin );
            break;

    } // End Switch Origin
    
    // Return the final position
    return ptOrigin;
}

//-----------------------------------------------------------------------------
//  Name : getClientArea ()
/// <summary>
/// Calculate the rectangle that describes the interior of this control 
/// (minus its borders) relative to the specified origin.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIControl::getClientArea( cgControlCoordinateSpace::Base Origin ) const
{
    cgPoint ptOrigin;
    cgRect  rcClient;
 
    // Set to size of client area initially
    rcClient = cgRect( 0, 0, mSize.width - (mPadding.left + mPadding.right), mSize.height - (mPadding.top + mPadding.bottom) );

    // Is this a complex control?
    if ( mControlMode == Complex )
    {
        // Get the required elements
        const cgUISkinElement * pTopLeft     = getControlLayer()->getSkinElement( mSkinElement + _T(".BorderTopLeft") );
        const cgUISkinElement * pBottomRight = getControlLayer()->getSkinElement( mSkinElement + _T(".BorderBottomRight") );
        
        // Remove size of top and left borders
        if ( pTopLeft )
        {
            rcClient.right  -= pTopLeft->bounds.right  - pTopLeft->bounds.left;
            rcClient.bottom -= pTopLeft->bounds.bottom - pTopLeft->bounds.top;
        
        } // End if pTopLeft

        // Remove size of bottom and right borders
        if ( pBottomRight )
        {
            rcClient.right  -= pBottomRight->bounds.right  - pBottomRight->bounds.left;
            rcClient.bottom -= pBottomRight->bounds.bottom - pBottomRight->bounds.top;
        
        } // End if pBottomRight

    } // End if complex control

    // Which origin would the caller like the rectangle relative to?
    switch ( Origin )
    {
        case cgControlCoordinateSpace::ControlRelative:
            // Return rectangle data relative to the control origin
            ptOrigin         = getClientOrigin( cgControlCoordinateSpace::ControlRelative );
            rcClient.left   += ptOrigin.x;
            rcClient.right  += ptOrigin.x;
            rcClient.top    += ptOrigin.y;
            rcClient.bottom += ptOrigin.y;
            break;

        case cgControlCoordinateSpace::ScreenRelative:
            // Return rectangle data relative to the screen origin
            ptOrigin         = getClientOrigin( cgControlCoordinateSpace::ScreenRelative );
            rcClient.left   += ptOrigin.x;
            rcClient.right  += ptOrigin.x;
            rcClient.top    += ptOrigin.y;
            rcClient.bottom += ptOrigin.y;
            break;

    } // End Switch Origin
    
    // Return the final rectangle
    return rcClient;
}

//-----------------------------------------------------------------------------
//  Name : getClientSize ()
/// <summary>
/// Calculate the size of the interior area of this control (minus its borders)
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgUIControl::getClientSize( ) const
{
    // ToDo: Potentially cache this size.

    // Set to full size of control including borders but minus padding initially
    cgSize clientSize( mSize.width - (mPadding.left + mPadding.right), mSize.height - (mPadding.top + mPadding.bottom) );

    // Is this a complex control?
    if ( mControlMode == Complex )
    {
        // Get the required elements
        const cgUISkinElement * pTopLeft     = getControlLayer()->getSkinElement( mSkinElement + _T(".BorderTopLeft") );
        const cgUISkinElement * pBottomRight = getControlLayer()->getSkinElement( mSkinElement + _T(".BorderBottomRight") );
        
        // Remove size of top and left borders
        if ( pTopLeft )
        {
            clientSize.width  -= pTopLeft->bounds.width();
            clientSize.height -= pTopLeft->bounds.height();
        
        } // End if pTopLeft

        // Remove size of bottom and right borders
        if ( pBottomRight )
        {
            clientSize.width  -= pBottomRight->bounds.width();
            clientSize.height -= pBottomRight->bounds.height();
        
        } // End if pBottomRight

    } // End if complex control
    
    // Return the final size
    return clientSize;
}

//-----------------------------------------------------------------------------
//  Name : getClientOrigin ()
/// <summary>
/// Simply calculate the top left point of the client area in relation
/// to the specified origin.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgUIControl::getClientOrigin( cgControlCoordinateSpace::Base Origin ) const
{
    cgPoint ptOrigin(0,0);
    cgSize  TopLeftSize( mPadding.left, mPadding.top );

    // Is this a complex control?
    if ( mControlMode == Complex )
    {
        // Get the required elements
        const cgUISkinElement * pTopLeft = getControlLayer()->getSkinElement( mSkinElement + _T(".BorderTopLeft") );
        
        // Compute top left position
        if ( pTopLeft )
        {
            TopLeftSize.width  += pTopLeft->bounds.right  - pTopLeft->bounds.left;
            TopLeftSize.height += pTopLeft->bounds.bottom - pTopLeft->bounds.top;

        } // End if pTopLeft

    } // End if complex control

    // Which origin would the caller like the data relative to?
    switch ( Origin )
    {
        case cgControlCoordinateSpace::ControlRelative:
            // Return origin data relative to the control origin
            ptOrigin.x = TopLeftSize.width;
            ptOrigin.y = TopLeftSize.height;
            break;

        case cgControlCoordinateSpace::ScreenRelative:
            // Return origin data relative to the screen origin
            ptOrigin = controlToScreen( (cgPoint&)TopLeftSize );
            break;

    } // End Switch Origin
    
    // Return the final position
    return ptOrigin;
}

//-----------------------------------------------------------------------------
//  Name : getControlArea () (Overload)
/// <summary>
/// Calculate the rectangle that describes the area of this control 
/// (including its borders) relative to the control origin.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIControl::getControlArea( ) const
{
    // Return the rectangle
    return getControlArea( cgControlCoordinateSpace::ControlRelative );
}

//-----------------------------------------------------------------------------
//  Name : getControlOrigin () (Overload)
/// <summary>
/// Simply calculate the top left point of the control area in relation
/// to the control origin.
/// Note : This will effectively always return a position of (0,0) but is
/// provided for the sake of completeness.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgUIControl::getControlOrigin( ) const
{
    // Return the final position
    return cgPoint(0,0);
}

//-----------------------------------------------------------------------------
//  Name : getClientArea () (Overload)
/// <summary>
/// Calculate the rectangle that describes the interior of this control 
/// (minus its borders) relative to the client origin.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIControl::getClientArea( ) const
{
    // Return the final rectangle
    return getClientArea( cgControlCoordinateSpace::ClientRelative );
}

//-----------------------------------------------------------------------------
//  Name : getClientOrigin () (Overload)
/// <summary>
/// Simply calculate the top left point of the client area in relation
/// to the client origin.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgUIControl::getClientOrigin( ) const
{
    // Return the final position
    return getClientOrigin( cgControlCoordinateSpace::ClientRelative );
}

//-----------------------------------------------------------------------------
//  Name : pointInControl ()
/// <summary>
/// Test to see if the specified screen space point is within the control.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::pointInControl( const cgPoint & ptTest ) const
{
    // Is this a complex or simple control?
    if ( mControlMode == Complex )
    {
        ControlIndexMap::const_iterator itElement;
        const cgUIControl             * pControl;

        // If this is a complex control, we need to query the elements
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            pControl = itElement->second;
            if ( !pControl ) continue;

            // Ask the child element to test the point
            if ( pControl->pointInControl( ptTest ) )
                return true;

        } // Next element

    } // End if complex control type
    else
    {
        // Retrieve the current render frame description
        const RenderFrameDesc * pDesc = getCurrentRenderFrame();

        // If this has any region, test that, otherwise just use the control area.
        if ( pDesc->activeRegion )
        {
            // Convert the screen space point into the space of the control
            cgPoint ptControl = screenToControl( ptTest );

            // We need to scale the test position in order to ensure that we test
            // the region correctly if the element has been resized (i.e. the filler 
            // in the control borders.) This prevents us from having to constantly 
            // rebuild the region data.
            cgInt32 nElementWidth  = pDesc->originalElement->bounds.right - pDesc->originalElement->bounds.left;
            cgInt32 nElementHeight = pDesc->originalElement->bounds.bottom - pDesc->originalElement->bounds.top;
            ptControl.x = (cgInt32)(((cgFloat)ptControl.x / (cgFloat)mSize.width) * (cgFloat)nElementWidth);
            ptControl.y = (cgInt32)(((cgFloat)ptControl.y / (cgFloat)mSize.height) * (cgFloat)nElementHeight);
            
            // Test against the region
            return ( ::PtInRegion( (HRGN)pDesc->activeRegion, ptControl.x, ptControl.y ) == TRUE );

        } // End if has an active region
        else
        {
            // Just test against the control's screen space bounding box.
            return getControlArea( cgControlCoordinateSpace::ScreenRelative ).containsPoint( ptTest );
        
        } // End if no active region

    } // End if simple control type

    // Not within the control area
    return false;
}

//-----------------------------------------------------------------------------
//  Name : clientToScreen ()
/// <summary>
/// Convert a point in the client space of this control, into screen
/// space.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgUIControl::clientToScreen( const cgPoint & ptClient ) const
{
    cgPoint ptCurrent = ptClient;

    // Add on our client to control space, and control to parent space offsets
    cgPoint ptOrigin = getClientOrigin( cgControlCoordinateSpace::ControlRelative );
    ptCurrent.x += ptOrigin.x + mPosition.x;
    ptCurrent.y += ptOrigin.y + mPosition.y;

    // Ask parent to do the same
    if ( mParent )
        return mParent->clientToScreen( ptCurrent );

    // If no parent, just return the final value
    return ptCurrent;
}

//-----------------------------------------------------------------------------
//  Name : controlToScreen ()
/// <summary>
/// Convert a point in the space of this control, into screen space.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgUIControl::controlToScreen( const cgPoint & ptControl ) const
{
    cgPoint ptCurrent = ptControl;

    // Add on our parent space offsets
    ptCurrent.x += mPosition.x;
    ptCurrent.y += mPosition.y;

    // Ask parent to do the same although this is now in the client space
    if ( mParent )
        return mParent->clientToScreen( ptCurrent );

    // If no parent, just return the final value
    return ptCurrent;
}

//-----------------------------------------------------------------------------
//  Name : screenToClient ()
/// <summary>
/// Convert a point in screen space, into the client space of this 
/// control.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgUIControl::screenToClient( const cgPoint & ptScreen ) const
{
    cgPoint ptCurrent = ptScreen;

    // Subtract our client to control space, and control to parent space offsets
    cgPoint ptOrigin = getClientOrigin( cgControlCoordinateSpace::ControlRelative );
    ptCurrent.x -= ptOrigin.x + mPosition.x;
    ptCurrent.y -= ptOrigin.y + mPosition.y;

    // Ask parent to do the same
    if ( mParent )
        return mParent->screenToClient( ptCurrent );

    // If no parent, just return the final value
    return ptCurrent;
}

//-----------------------------------------------------------------------------
//  Name : screenToControl ()
/// <summary>
/// Convert a point in screen space, into the space of this control
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgUIControl::screenToControl( const cgPoint & ptScreen ) const
{
    cgPoint ptCurrent = ptScreen;

    // Subtract our parent space offsets
    ptCurrent.x -= mPosition.x;
    ptCurrent.y -= mPosition.y;

    // Ask parent to do the same although this is now in the client space
    if ( mParent )
        return mParent->screenToClient( ptCurrent );

    // If no parent, just return the final value
    return ptCurrent;
}

//-----------------------------------------------------------------------------
//  Name : setRenderMode ()
/// <summary>
/// Sets the current rendering mode of the control (i.e. hover/pressed)
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setRenderMode( ControlRenderMode Mode )
{
    ControlIndexMap::iterator itElement;

    // Store the current render mode
    mCurrentRenderMode = Mode;

    // Set the render mode of any control element
    for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
    {
        if ( itElement->second )
            itElement->second->setRenderMode( Mode );

    } // Next element

    // Billboard stored here?
    if ( mBillboard )
    {
        const RenderFrameDesc * pDesc = getCurrentRenderFrame();

        // Set the frame information
        mBillboard->setFrame( pDesc->frameIndex );
        mBillboard->update();

    } // End if billboard here
}

//-----------------------------------------------------------------------------
//  Name : onLostFocus() (Virtual)
/// <summary>
/// Triggered whenever the control loses user input focus.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::onLostFocus( )
{
    // Raise the associated event
    raiseEvent( cgSystemMessages::UI_OnLostFocus, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : onGainFocus () (Virtual)
/// <summary>
/// Triggered whenever the control gains user input focus.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::onGainFocus( )
{
    // Raise the associated event
    raiseEvent( cgSystemMessages::UI_OnGainFocus, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : onInitControl () (Virtual)
/// <summary>
/// Triggered whenever the control has been initialized.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::onInitControl( )
{
    // Raise the onInitControl event
    raiseEvent( cgSystemMessages::UI_OnInitControl, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : onMouseMove ()
/// <summary>
/// Triggered whenever the mouse moves.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;
   
    // Pass through to all child controls
    ControlList::iterator itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Notify child
        if ( pControl->onMouseMove( Position, Offset ) )
            return true;

    } // Next child control

    // No child processed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Pass through to all child controls (important that we process from bottom to top)
    ControlList::iterator itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Notify child
        if ( pControl->onMouseButtonDown( nButtons, Position ) )
            return true;

    } // Next child control

    // No child processed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonUp () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is released.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() && mUIManager->getCapture() != this )
        return false;

    // Pass through to all child controls
    ControlList::iterator   itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Notify child
        if ( pControl->onMouseButtonUp( nButtons, Position ) )
            return true;

    } // Next child control

    // No child processed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseWheelScroll () (Virtual)
/// <summary>
/// This method is called whenever the mouse wheel is scrolled.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::onMouseWheelScroll( cgInt32 nDelta, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Pass through to all child controls
    ControlList::iterator   itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Notify child
        if ( pControl->onMouseWheelScroll( nDelta, Position ) )
            return true;

    } // Next child control

    // No child processed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onKeyDown () (Virtual)
/// <summary>
/// This method is called whenever a key is first pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::onKeyDown( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;
    
    // Pass through to all child controls (important that we process from bottom to top)
    ControlList::iterator itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Notify child
        if ( pControl->onKeyDown( nKeyCode, nModifiers ) )
            return true;

    } // Next child control

    // No child processed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onKeyUp () (Virtual)
/// <summary>
/// This method is called whenever a key is released.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::onKeyUp( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Pass through to all child controls (important that we process from bottom to top)
    ControlList::iterator itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Notify child
        if ( pControl->onKeyUp( nKeyCode, nModifiers ) )
            return true;

    } // Next child control

    // No child processed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onKeyPressed () (Virtual)
/// <summary>
/// This method is called whenever a key is pressed, and subsequent times
/// if the key is held taking into account repeat delay and rate.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::onKeyPressed( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Pass through to all child controls (important that we process from bottom to top)
    ControlList::iterator itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Notify child
        if ( pControl->onKeyPressed( nKeyCode, nModifiers ) )
            return true;

    } // Next child control

    // No child processed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Triggered whenever the control is resized.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    // Raise the onSize event
    raiseEvent( cgSystemMessages::UI_OnSize, &UI_OnSizeArgs( nWidth, nHeight ) );
}

//-----------------------------------------------------------------------------
//  Name : onParentAttach () (Virtual)
/// <summary>
/// This method is called when a control is first attached to a new parent
/// control.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::onParentAttach( cgUIControl * parent )
{
    // Nothing in base implementation
}

//-----------------------------------------------------------------------------
//  Name : onScreenLayoutChange () (Virtual)
/// <summary>
/// This method is called whenever the render driver reports that the layout
/// of the screen may have changes (its size, etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::onScreenLayoutChange( )
{
    // Raise the event on the top level control first.
    raiseEvent( cgSystemMessages::UI_OnScreenLayoutChange, CG_NULL );

    // Pass through to all child controls from top to bottom
    ControlList::iterator itControl;
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        cgUIControl * pControl = *itControl;
        if ( !pControl ) continue;
        
        // Notify child
        pControl->onScreenLayoutChange( );

    } // Next child control
}

//-----------------------------------------------------------------------------
//  Name : registerEventHandler ()
/// <summary>
/// This function is used to register an event handler that should
/// receive notifications via the reference messaging system for a
/// specific event (i.e. UI_Button_OnClick) for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::registerEventHandler( cgUInt32 nUIMessage, cgInt32 nReferenceId )
{
    EventHandler Handler;

    // Construct event handler data
    Handler.referenceId  = nReferenceId;
    Handler.func         = CG_NULL;
    Handler.context      = CG_NULL;
    Handler.scriptObject = CG_NULL;

    // Store this event handler
    mEventHandlers[ nUIMessage ] = Handler;
}

//-----------------------------------------------------------------------------
//  Name : registerEventHandler ()
/// <summary>
/// This function is used to register an event handler that should
/// receive notifications via a direct callback for a specific event 
/// (i.e. UI_Button_OnClick) for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::registerEventHandler( cgUInt32 nUIMessage, UIEventCallback pFunction, void * pContext )
{
    EventHandler Handler;

    // Construct event handler data
    Handler.referenceId    = 0;
    Handler.func       = pFunction;
    Handler.context        = pContext;
    Handler.scriptObject   = CG_NULL;

    // Store this event handler
    mEventHandlers[ nUIMessage ] = Handler;
}

//-----------------------------------------------------------------------------
//  Name : registerEventHandler ()
/// <summary>
/// This function is used to register an event handler that should
/// receive notifications via a script based callback for a specific event 
/// (i.e. UI_Button_OnClick) for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::registerEventHandler( cgUInt32 nUIMessage, const cgString & strScriptHandler, cgScriptObject * pScriptObject )
{
    EventHandler Handler;

    cgToDo( "Carbon General", "Support multiple handlers per message, or make sure we release the prior scriptObject if one existed (in all 'registerEventHandler' methods -- they can all overwrite an existing one)." );

    // Construct event handler data
    Handler.referenceId    = 0;
    Handler.func       = CG_NULL;
    Handler.context        = CG_NULL;
    Handler.scriptHandler   = strScriptHandler;
    Handler.scriptObject   = pScriptObject;

    // Add reference to held objects.
    if ( Handler.scriptObject )
        Handler.scriptObject->addRef();

    // Store this event handler
    mEventHandlers[ nUIMessage ] = Handler;
}

//-----------------------------------------------------------------------------
//  Name : raiseEvent ()
/// <summary>
/// This method can be called in order to raise an event of the specified
/// type and pass the message on to the registered event target.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::raiseEvent( cgUInt32 nUIMessage, UIEventArgs * pData )
{
    // Any event handler registered for this message type?
    EventHandlerMap::iterator itTarget = mEventHandlers.find( nUIMessage );
    if ( itTarget != mEventHandlers.end() )
    {
        EventHandler & Handler = itTarget->second;

        // Sending via messaging system or direct callback?
        if ( Handler.referenceId > 0 )
        {
            cgMessage  Msg;
            
            // Build the message that should be sent out
            Msg.messageId   = nUIMessage;
            Msg.messageData = pData;

            // Send this message to the target
            cgReferenceManager::sendMessageTo( getReferenceId(), Handler.referenceId, &Msg );

        } // End if message
        else if ( Handler.scriptObject )
        {
            // Get arguments from data structure if provided
            cgScriptArgument::Array aScriptArgs;
            if ( pData )
                pData->toArgumentList( aScriptArgs );

            // Control should be pushed onto the front of the argument list
            aScriptArgs.insert( aScriptArgs.begin(), cgScriptArgument( cgScriptArgumentType::Object, _T("UIControl@+"), this ) );
            
            // Execute the function
            try
            {
                Handler.scriptObject->executeMethodVoid( Handler.scriptHandler, aScriptArgs );

            } // End try execute
            
            catch ( cgScriptInterop::Exceptions::ExecuteException & e )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to execute %s() event method in '%s'. The engine reported the following error: %s.\n"), Handler.scriptHandler.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
                return;

            } // End catch exception

        } // End if script callback
        else if ( Handler.func )
        {
            // Make the call.
            Handler.func( this, pData, Handler.context );

        } // End if C-callback

    } // End if target found
}

//-----------------------------------------------------------------------------
//  Name : getCurrentRenderFrame () (Protected)
/// <summary>
/// Retrieve the render frame information for this control based on the
/// current state and render mode.
/// </summary>
//-----------------------------------------------------------------------------
const cgUIControl::RenderFrameDesc * cgUIControl::getCurrentRenderFrame( ) const
{
    const RenderFrameDesc * pDesc = &mControlStateInfo[ mCurrentState ].renderModes[ mCurrentRenderMode ];

    // Select the normal rendering mode if the one selected is not valid
    if ( !pDesc->isValid() )
        pDesc = &mControlStateInfo[mCurrentState].renderModes[RenderMode_Normal];

    // Return the current mode
    return pDesc;
}

//-----------------------------------------------------------------------------
//  Name : pointOverHandle () (Protected)
/// <summary>
/// Determine if the specified screen point is currently over any handle
/// region defined by the interface skin.
/// </summary>
//-----------------------------------------------------------------------------
cgUIHandleType::Base cgUIControl::pointOverHandle( const cgPoint & ptScreen ) const
{
    cgUIHandleType::Base Type;
    cgInt32 i;

    // Is this a complex or simple control?
    if ( mControlMode == Complex )
    {
        ControlIndexMap::const_iterator itElement;
        const cgUIControl             * pControl;

        // If this is a complex control, we need to query the elements
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            if ( !(pControl = itElement->second) )
                continue;
            
            // Ask the child element to test the point
            Type = pControl->pointOverHandle( ptScreen );
            if ( Type != cgUIHandleType::Invalid )
                return Type;

        } // Next element

    } // End if complex control type
    else
    {
        // Retrieve the current render frame description
        const RenderFrameDesc * pDesc = getCurrentRenderFrame();

        // Convert the screen space point into the space of the control
        cgPoint ptControl = screenToControl( ptScreen );

        // We need to scale the position in order to ensure that we test
        // the region correctly if the element has been resized (i.e. the filler 
        // in the control borders.) This prevents us from having to constantly 
        // rebuild the region data.
        cgInt32 nElementWidth  = pDesc->originalElement->bounds.right - pDesc->originalElement->bounds.left;
        cgInt32 nElementHeight = pDesc->originalElement->bounds.bottom - pDesc->originalElement->bounds.top;
        ptControl.x = (cgInt32)(((cgFloat)ptControl.x / (cgFloat)mSize.width) * (cgFloat)nElementWidth);
        ptControl.y = (cgInt32)(((cgFloat)ptControl.y / (cgFloat)mSize.height) * (cgFloat)nElementHeight);

        // Test all of the 8 handle region types that may have been defined for this control
        for ( i = 0; i < 8; ++i )
        {
            // If this region is defined, test it
            if ( pDesc->handleRegions[i] )
            {
                // Test against the region
                if ( ::PtInRegion( (HRGN)pDesc->handleRegions[i], ptControl.x, ptControl.y ) == TRUE )
                    return (cgUIHandleType::Base)i;

            } // End if has a handle region

        } // Next Region

    } // End if simple control type

    // Not over any handle
    return cgUIHandleType::Invalid;
}

//-----------------------------------------------------------------------------
//  Name : isVisible ()
/// <summary>
/// Determine if this control is considered visible. This will include
/// the parent visibility status by default.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::isVisible( ) const
{
    return mVisible && mParentVisible;
}

//-----------------------------------------------------------------------------
//  Name : isVisible ()
/// <summary>
/// Determine if this control is considered visible optionally including
/// the parent visibility status.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::isVisible( bool bIncludeParentState ) const
{
    if (bIncludeParentState)
        return mVisible && mParentVisible;
    else
        return mVisible;
}

//-----------------------------------------------------------------------------
//  Name : canGainFocus ()
/// <summary>
/// Determine if this control can currently gain focus. This will include
/// the parent focus enabled status by default.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::canGainFocus( ) const
{
    return mCanGainFocus && mParentCanGainFocus;
}

//-----------------------------------------------------------------------------
//  Name : canGainFocus ()
/// <summary>
/// Determine if this control can currently gain focus. This will include
/// the parent focus enabled status by default.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::canGainFocus( bool bIncludeParentState ) const
{
    if (bIncludeParentState)
        return mCanGainFocus && mParentCanGainFocus;
    else
        return mCanGainFocus;
}

//-----------------------------------------------------------------------------
//  Name : isEnabled ()
/// <summary>
/// Determine if this control is considered enabled. This will include
/// the parent enabled status by default.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::isEnabled( ) const
{
    return mEnabled && mParentEnabled;
}

//-----------------------------------------------------------------------------
//  Name : isEnabled ()
/// <summary>
/// Determine if this control is considered enabled optionally including
/// the parent enabled status.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControl::isEnabled( bool bIncludeParentState ) const
{
    if (bIncludeParentState )
        return mEnabled && mParentEnabled;
    else
        return mEnabled;
}

//-----------------------------------------------------------------------------
//  Name : setVisible () (Virtual)
/// <summary>
/// Hide / show this control and all it's children.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setVisible( bool bVisible )
{
    ControlList::iterator     itControl;
    ControlIndexMap::iterator itElement;
    cgUIControl             * pControl;

    // Is this a no-op?
    if ( bVisible == mVisible )
        return;

    // Update our visibility status
    mVisible = bVisible;

    // Pass through to all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        pControl = *itControl;
        if ( !pControl ) continue;
        
        // Set child's "parent" visibility status
        pControl->setParentVisible( isVisible() );

    } // Next child control

    // Hide/show the control elements if it has any
    if ( mControlMode == Complex )
    {
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            pControl = itElement->second;
            if ( !pControl ) continue;

            // Set visibility status
            pControl->setParentVisible( isVisible() );

        } // Next element
    
    } // End if complex control

    // Set the visible status of the billboard if any stored here
    if ( mBillboard )
    {
        // Update the billboard
        mBillboard->setVisible( isVisible( ) );
        mBillboard->update();
    
    } // End if billboard exists
}

//-----------------------------------------------------------------------------
//  Name : setParentVisible () (Protected)
/// <summary>
/// Parent visibility and child visibility are treated differently.
/// Because the visibility of the parent should not affect the original
/// visibility of the child, this is tracked separately.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setParentVisible( bool bVisible )
{
    ControlList::iterator     itControl;
    ControlIndexMap::iterator itElement;
    cgUIControl             * pControl;

    // Is this a no-op?
    if ( bVisible == mParentVisible )
        return;

    // Update our parent visibility status
    mParentVisible = bVisible;

    // If parent is being shown again, and we were already invisible
    // just bail.
    if ( bVisible && !mVisible )
        return;

    // Pass through to all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        pControl = *itControl;
        if ( !pControl ) continue;
        
        // Set child's "parent" visibility status
        pControl->setParentVisible( isVisible() );

    } // Next child control

    // Hide/show the control elements if it has any
    if ( mControlMode == Complex )
    {
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            pControl = itElement->second;
            if ( !pControl ) continue;

            // Set visibility status
            pControl->setParentVisible( isVisible() );

        } // Next element
    
    } // End if complex control

    // Set the visible status of the billboard if any stored here
    if ( mBillboard )
    {
        // Update the billboard
        mBillboard->setVisible( isVisible( ) );
        mBillboard->update();
    
    } // End if billboard exists
}

//-----------------------------------------------------------------------------
//  Name : setCanGainFocus ()
/// <summary>
/// Set the flag that dictates whether or not this control can receive input
/// focus.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setCanGainFocus( bool bCanGainFocus )
{
    ControlList::iterator     itControl;
    ControlIndexMap::iterator itElement;
    cgUIControl             * pControl;

    // Is this a no-op?
    if ( bCanGainFocus == mCanGainFocus )
        return;

    // Update our internal status
    mCanGainFocus = bCanGainFocus;

    // Pass through to all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        pControl = *itControl;
        if ( !pControl ) continue;
        
        // Set child's "parent" focus gain status
        pControl->setParentCanGainFocus( bCanGainFocus );

    } // Next child control

    // Update the control elements if it has any
    if ( mControlMode == Complex )
    {
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            pControl = itElement->second;
            if ( !pControl ) continue;
            pControl->setParentCanGainFocus( bCanGainFocus );

        } // Next element
    
    } // End if complex control
}

//-----------------------------------------------------------------------------
//  Name : setParentCanGainFocus () (Protected)
/// <summary>
/// Parent status and child status are treated differently.
/// Because the status of the parent should not affect the original
/// status of the child, this is tracked separately.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setParentCanGainFocus( bool bCanGainFocus )
{
    ControlList::iterator     itControl;
    ControlIndexMap::iterator itElement;
    cgUIControl             * pControl;

    // Is this a no-op?
    if ( bCanGainFocus == mParentCanGainFocus )
        return;

    // Update our parent status
    mParentCanGainFocus = bCanGainFocus;

    // If parent is being enabled again, and we were already disabled
    // just bail.
    if ( bCanGainFocus && !mCanGainFocus )
        return;

    // Pass through to all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        pControl = *itControl;
        if ( !pControl ) continue;
        
        // Set child's "parent" focus gain status
        pControl->setParentCanGainFocus( bCanGainFocus );

    } // Next child control

    // Update state of the control elements if it has any
    if ( mControlMode == Complex )
    {
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            pControl = itElement->second;
            if ( !pControl ) continue;
            pControl->setParentCanGainFocus( bCanGainFocus );

        } // Next element
    
    } // End if complex control
}

//-----------------------------------------------------------------------------
//  Name : setEnabled ()
/// <summary>
/// Enable / Disable this control and all it's children.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setEnabled( bool bEnabled )
{
    ControlList::iterator     itControl;
    ControlIndexMap::iterator itElement;
    cgUIControl             * pControl;

    // Is this a no-op?
    if ( bEnabled == mEnabled )
        return;

    // Update our enabled status
    mEnabled = bEnabled;

    // Pass through to all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        pControl = *itControl;
        if ( !pControl ) continue;
        
        // Set child's "parent" enabled status
        pControl->setParentEnabled( bEnabled );

    } // Next child control

    // Enable / disable the control elements if it has any
    if ( mControlMode == Complex )
    {
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            pControl = itElement->second;
            if ( !pControl ) continue;

            // Set enabled status
            pControl->setParentEnabled( bEnabled );

        } // Next element
    
    } // End if complex control
    else
    {
        // Set our mode to disabled state
        setRenderMode( RenderMode_Disabled );
    
    } // End if simple control
}

//-----------------------------------------------------------------------------
//  Name : setParentEnabled () (Protected)
/// <summary>
/// Parent status and child status are treated differently.
/// Because the status of the parent should not affect the original
/// status of the child, this is tracked separately.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setParentEnabled( bool bEnabled )
{
    ControlList::iterator     itControl;
    ControlIndexMap::iterator itElement;
    cgUIControl             * pControl;

    // Is this a no-op?
    if ( bEnabled == mParentEnabled )
        return;

    // Update our parent status
    mParentEnabled = bEnabled;

    // If parent is being enabled again, and we were already disabled
    // just bail.
    if ( bEnabled && !mEnabled )
        return;

    // Pass through to all child controls
    for ( itControl = mChildren.begin(); itControl != mChildren.end(); ++itControl )
    {
        pControl = *itControl;
        if ( !pControl ) continue;
        
        // Set child's "parent" enabled status
        pControl->setParentEnabled( bEnabled );

    } // Next child control

    // Enable/disable the control elements if it has any
    if ( mControlMode == Complex )
    {
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            pControl = itElement->second;
            if ( !pControl ) continue;

            // Set visibility status
            pControl->setParentEnabled( bEnabled );

        } // Next element
    
    } // End if complex control
    else
    {
        // Set our mode to disabled state
        setRenderMode( RenderMode_Disabled );
    
    } // End if simple control
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundOpacity ()
/// <summary>
/// Set the level of opacity for the background elements of this control
/// 0 = completely transparent, 1 = completely opaque.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setBackgroundOpacity( cgFloat fAlpha )
{
    // Update local value
    mBackgroundOpacity = fAlpha;

    // Set opacity level for child control elements if it has any
    if ( mControlMode == Complex )
    {
        ControlIndexMap::iterator itElement;
        for ( itElement = mControlElements.begin(); itElement != mControlElements.end(); ++itElement )
        {
            cgUIControl * pControl = itElement->second;
            if ( !pControl )
                continue;

            // Set opacity status
            pControl->setBackgroundOpacity( fAlpha );

        } // Next element
    
    } // End if complex control
    else if ( mBillboard )
    {
        cgColorValue CurrentColor = mBillboard->getColor();
        CurrentColor.a = fAlpha;
        mBillboard->setColor( CurrentColor );
        mBillboard->update();

    } // End if !Complex
}

//-----------------------------------------------------------------------------
//  Name : getBackgroundOpacity ()
/// <summary>
/// Get the level of opacity for the background elements of this control
/// 0 = completely transparent, 1 = completely opaque.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgUIControl::getBackgroundOpacity( ) const
{
    return mBackgroundOpacity;
}

//-----------------------------------------------------------------------------
//  Name : setDockMode ()
/// <summary>
/// Set the docking mode used in the automatic position / sizing of controls
/// during layout computation.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::setDockMode( cgDockMode::Base mode )
{
    mDockMode = mode;
}

//-----------------------------------------------------------------------------
//  Name : getDockMode ()
/// <summary>
/// Get the docking mode used in the automatic position / sizing of controls
/// during layout computation.
/// </summary>
//-----------------------------------------------------------------------------
cgDockMode::Base cgUIControl::getDockMode( ) const
{
    return mDockMode;
}

//-----------------------------------------------------------------------------
//  Name : focus ()
/// <summary>
/// This control gains focus.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControl::focus( )
{
    if ( mUIManager )
        mUIManager->setFocus( this );
}