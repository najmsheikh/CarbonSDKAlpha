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
// Name : cgUILayers.cpp                                                     //
//                                                                           //
// Desc : These classes are responsible for managing and rendering data      //
//        for an individual layer on the user interface. Each layer is       //
//        is essentially a slice of the interface depth that contains an     //
//        individual item such as a form or widget. System layers also exist //
//        which display system specific items such as the cursor.            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgUILayers Module Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUILayers.h>
#include <Interface/cgUIManager.h>
#include <Interface/cgUIControl.h>
#include <Interface/cgUISkin.h>
#include <Input/cgInputDriver.h>
#include <Rendering/cgBillboardBuffer.h>

///////////////////////////////////////////////////////////////////////////////
// cgUILayer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgUILayer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUILayer::cgUILayer( cgUIManager * pManager, cgUILayerType::Base Type, cgInt32 nLayerDepth /* = -1 */ )
{
    // Initialize variables to sensible defaults
    mUIManager  = pManager;
    mBillboards = new cgBillboardBuffer();
    mLayerDepth = nLayerDepth;
    mLayerType  = Type;
}

//-----------------------------------------------------------------------------
//  Name : ~cgUILayer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUILayer::~cgUILayer()
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
void cgUILayer::dispose( bool bDisposeBase )
{
    // Release allocated memory
    if ( mBillboards )
        delete mBillboards;

    // Clear variables
    mBillboards = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : render () (Virtual)
/// <summary>
/// Render the contents of this layer.
/// </summary>
//-----------------------------------------------------------------------------
void cgUILayer::render( )
{
    // Render the contents of the billboard buffer
    mBillboards->render();
}

//-----------------------------------------------------------------------------
//  Name : bringToFront ()
/// <summary>
/// Move the interface layer to the front of its class of layers.
/// </summary>
//-----------------------------------------------------------------------------
void cgUILayer::bringToFront( )
{
    if ( mUIManager )
        mUIManager->bringLayerToFront( this );
}

//-----------------------------------------------------------------------------
//  Name : sendToBack ()
/// <summary>
/// Move the interface layer to the back of its class of layers.
/// </summary>
//-----------------------------------------------------------------------------
void cgUILayer::sendToBack( )
{
    if ( mUIManager )
        mUIManager->sendLayerToBack( this );
}

//-----------------------------------------------------------------------------
//  Name : prepareLayer ()
/// <summary>
/// Begin preparations for populating the billboard buffer for this layer
/// </summary>
//-----------------------------------------------------------------------------
bool cgUILayer::prepareLayer( const cgInputStream & TextureFileStream )
{
    return mBillboards->prepareBuffer( cgBillboardBuffer::ScreenSpace, mUIManager->getRenderDriver(), TextureFileStream );
}

//-----------------------------------------------------------------------------
//  Name : endPrepareLayer ()
/// <summary>
/// Finishing preparing the layer buffer
/// </summary>
//-----------------------------------------------------------------------------
bool cgUILayer::endPrepareLayer( )
{
    return mBillboards->endPrepare();
}

///////////////////////////////////////////////////////////////////////////////
// cgUIControlLayer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgUIControlLayer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUIControlLayer::cgUIControlLayer( cgUIManager * pManager, cgUILayerType::Base Type, cgInt32 nLayerDepth /* = -1 */ ) : cgUILayer( pManager, Type, nLayerDepth )
{
    // Initialize variables to sensible defaults
    mControl = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgUIControlLayer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUIControlLayer::~cgUIControlLayer()
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
void cgUIControlLayer::dispose( bool bDisposeBase )
{
    // Release the control
    if ( mControl )
        mControl->scriptSafeDispose();
    
    // Clear variables
    mControl = CG_NULL;

    // Call base class implementation
    if ( bDisposeBase )
        cgUILayer::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : attachControl ()
/// <summary>
/// Attach the specified control to this layer.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControlLayer::attachControl( cgUIControl * pControl )
{
    // Store the control
    mControl = pControl;
}

//-----------------------------------------------------------------------------
//  Name : registerSkinElement() (Protected)
/// <summary>
/// Given information about an individual skin element, this function
/// will register that information with the layer's billboard buffer
/// as well as storing the information for child controls to use.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::registerSkinElement( const cgUISkinElement & Element, const cgString & strElementName )
{
    cgInt16  nGroupIndex = -1;
    cgString strFullElementName;

    // Validate requirements
    if ( !mBillboards )
        return false;

    // Add this to the billboard buffer
    if ( mBillboards->addFrame( 0, Element.bounds, strElementName ) < 0 )
        return false;

    // Also add this element to the map for lookup later
    mSkinElements[ strElementName ] = Element;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSkinElement() (Protected)
/// <summary>
/// Retrieve the information for the specified skin element.
/// </summary>
//-----------------------------------------------------------------------------
const cgUISkinElement * cgUIControlLayer::getSkinElement( const cgString & strName )
{
    // Does the element exist in the map?
    cgUISkinElement::Map::iterator itElement = mSkinElements.find( strName );
    if ( itElement == mSkinElements.end() )
        return CG_NULL;

    // If it does, return it
    return &itElement->second;
}

//-----------------------------------------------------------------------------
//  Name : render () (Virtual)
/// <summary>
/// Render the contents of this layer.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIControlLayer::render( )
{
    // Render the contents of the billboard buffer
    cgUILayer::render();

    // Allow child control to render secondary elements
    if ( mControl )
        mControl->renderSecondary();
}

//-----------------------------------------------------------------------------
//  Name : onMouseMove () (Virtual)
/// <summary>
/// This method is called whenever the mouse has moved.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    // Pass the message on to the attached control
    if ( mControl )
        return mControl->onMouseMove( Position, Offset );
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
{
    // Pass the message on to the attached control
    if ( mControl )
        return mControl->onMouseButtonDown( nButtons, Position );
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonUp () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is released.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
{
    // Pass the message on to the attached control
    if ( mControl )
        return mControl->onMouseButtonUp( nButtons, Position );
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseWheelScroll () (Virtual)
/// <summary>
/// This method is called whenever the mouse wheel is scrolled.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::onMouseWheelScroll( cgInt32 nDelta, const cgPoint & Position )
{
    // Pass the message on to the attached control
    if ( mControl )
        return mControl->onMouseWheelScroll( nDelta, Position );
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onKeyDown () (Virtual)
/// <summary>
/// This method is called whenever a key is first pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::onKeyDown( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Pass the message on to the attached control
    if ( mControl )
        return mControl->onKeyDown( nKeyCode, nModifiers );
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onKeyUp () (Virtual)
/// <summary>
/// This method is called whenever a key is released.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::onKeyUp( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Pass the message on to the attached control
    if ( mControl )
        return mControl->onKeyUp( nKeyCode, nModifiers );
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onKeyPressed () (Virtual)
/// <summary>
/// This method is called whenever a key is pressed, and subsequent times
/// if the key is held taking into account repeat delay and rate.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::onKeyPressed( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Pass the message on to the attached control
    if ( mControl )
        return mControl->onKeyPressed( nKeyCode, nModifiers );
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onScreenLayoutChange () (Virtual)
/// <summary>
/// This method is called whenever the render driver reports that the layout
/// of the screen may have changes (its size, etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIControlLayer::onScreenLayoutChange( )
{
    // Pass the message on to the attached control
    if ( mControl )
        mControl->onScreenLayoutChange( );

    // Always continue processing (everyone should receive).
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// cgUICursorLayer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgUICursorLayer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUICursorLayer::cgUICursorLayer( cgUIManager * pManager ) : cgUILayer( pManager, cgUILayerType::SystemLayer )
{
    // Initialize variables to sensible defaults
    mCursor         = CG_NULL;
    mCurrentType    = CG_NULL;
    mCursorOffset.x = 0;
    mCursorOffset.y = 0;
    mCurrentFrame   = 0;
    mCursorVisible  = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgUICursorLayer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUICursorLayer::~cgUICursorLayer()
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
void cgUICursorLayer::dispose( bool bDisposeBase )
{
    // Clear variables
    mCursor         = CG_NULL;
    mCurrentType    = CG_NULL;
    mCursorOffset.x = 0;
    mCursorOffset.y = 0;
    mCurrentFrame   = 0;
    mCursorVisible  = false;

    // Call base class implementation
    if ( bDisposeBase )
        cgUILayer::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the cursor layer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUICursorLayer::initialize( )
{
    cgUISkin * pCurrentSkin = mUIManager->getCurrentSkin();
    if ( !pCurrentSkin )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to load system cursor because no valid skin has been selected.\n") );
        return false;

    } // End if failed to load script

    // Retrieve the cursor definition and store required values
    const cgUICursorDesc & Desc = pCurrentSkin->getCursorDefinition();

    // Begin preparing the layer buffer.
    if ( prepareLayer( Desc.texture ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to begin preparing system cursor layer.\n") );
        return false;

    } // End if failed to prepare

    // Add the frame information for the cursor
    cgRenderDriver * pDriver = getUIManager()->getRenderDriver();
    if ( pCurrentSkin->prepareCursorFrames( mBillboards, pDriver ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to populate render buffer frameset for cursor layer.\n") );
        return false;
    
    } // End if failed to add frames
    
    // Create the cursor billboard
    mCursor = new cgBillboard2D();
    mBillboards->addBillboard( mCursor );
    mCursor->setPosition( cgVector3( 0, 0, 0 ) );
    mCursor->setFrameGroup( 0 );
    mCursor->setFrame( 0, true );
    mCursor->setVisible( false );
    mCursor->update();
    mCursorDirty = false;
    mCursorVisible = false;

     // Finalize the billboard buffer preparation
    if ( endPrepareLayer() == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to complete preperation of render data for system cursor layer.\n") );
        return false;

    } // End if failed to prepare

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : showCursor () (Virtual)
/// <summary>
/// Show / hide the emulated cursor.
/// </summary>
//-----------------------------------------------------------------------------
void cgUICursorLayer::showCursor( bool show )
{
    // No-op?
    if ( mCursorVisible == show )
        return;
    mCursorVisible = show;

    // Show / hide the cursor and update.
    if ( mCursor )
    {
        mCursor->setVisible( (mCurrentType) ? show : false );
        mCursor->update();
        mCursorDirty = false;
    
    } // End if dirty
}

//-----------------------------------------------------------------------------
//  Name : onMouseMove () (Virtual)
/// <summary>
/// This method is called whenever the mouse has moved.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUICursorLayer::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    // Validate requirements
    if ( !mCursor )
        return false;

    // Set the position of the billboard
    mCursor->setPosition( cgVector3( (cgFloat)(Position.x + mCursorOffset.x), (cgFloat)(Position.y + mCursorOffset.y), 0 ) );
    mCursorDirty = true;

    // This layer should never halt the mouse move message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : selectCursor ()
/// <summary>
/// Call this method to select the current cursor to use from the
/// cursor definition.
/// </summary>
//-----------------------------------------------------------------------------
void cgUICursorLayer::selectCursor( const cgUICursorType * type, cgUInt32 frame )
{
    if ( type != mCurrentType )
    {
        // Data has been updated.
        mCurrentType  = type;
        mCurrentFrame = (cgInt16)frame;
        
        // Update the position of the billboard in case the cursor offset changes
        cgVector3 CursorPos = mCursor->getPosition();
        if ( mCurrentType )
        {
            CursorPos += cgVector3( (-(cgFloat)mCurrentType->hotPoint.x) - (cgFloat)mCursorOffset.x, (-(cgFloat)mCurrentType->hotPoint.y) - (cgFloat)mCursorOffset.y, 0 ); 
            mCursorOffset.x = -mCurrentType->hotPoint.x;
            mCursorOffset.y = -mCurrentType->hotPoint.y;

            // Select the correct frame data for the billboard
            mCursor->setFrameGroup( mBillboards->getFrameGroupIndex( mCurrentType->name ) );
            mCursor->setFrame( (cgInt16)frame, true );
            mCursor->setVisible( (mCursorVisible) ? true : false );
            mCursorDirty  = true;

        } // End if selected type
        else
        {
            CursorPos += cgVector3( -(cgFloat)mCursorOffset.x, -(cgFloat)mCursorOffset.y, 0 ); 
            mCursorOffset = cgPoint(0,0);
            mCursor->setFrameGroup( 0 );
            mCursor->setFrame( 0, true );
            mCursor->setVisible( false );
            mCursorDirty = true;
        
        } // End if hiding.
        mCursor->setPosition( CursorPos );
    
    } // End if type changing
    else if ( frame != mCurrentFrame )
    {
        mCurrentFrame = (cgInt16)frame;
        mCursor->setFrame( mCurrentFrame, false );
        mCursorDirty  = true;
        
    } // End if frame changing
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Allow the cursor billboard to be updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgUICursorLayer::update( )
{
    if ( mCursor && mCursor->getVisible() && mCursorDirty )
    {
        mCursor->update();
        mCursorDirty = false;
    
    } // End if dirty
}

//-----------------------------------------------------------------------------
//  Name : render () (Virtual)
/// <summary>
/// Render the contents of this layer.
/// </summary>
//-----------------------------------------------------------------------------
void cgUICursorLayer::render( )
{
    // Render the contents of the billboard buffer
    if ( mCursor->getVisible() && cgInputDriver::getInstance()->getMouseMode() == cgMouseHandlerMode::Cursor )
        mBillboards->render();
}