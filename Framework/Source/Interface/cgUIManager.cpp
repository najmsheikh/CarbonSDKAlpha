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
// Name : cgUIManager.cpp                                                    //
//                                                                           //
// Desc : This module houses classes responsible for the storage and         //
//        management of application interface objects such as dialogs,       //
//        controls and widgets.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgUIManager Module Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIManager.h>
#include <Interface/cgUILayers.h>
#include <Interface/cgUISkin.h>
#include <Interface/cgUIForm.h>
#include <Interface/cgUITypes.h>
#include <Interface/cgTextEngine.h>
#include <Input/cgInputDriver.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/cgBillboardBuffer.h>
#include <Rendering/cgRenderDriver.h>
#include <Input/cgInputTypes.h>
#include <System/cgMessageTypes.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgUIManager * cgUIManager::mSingleton = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgUIManager Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgUIManager () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUIManager::cgUIManager() : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mResourceManager = CG_NULL;
    mTextEngine      = CG_NULL;
    mCurrentSkin     = CG_NULL;
    mCapturedControl = CG_NULL;
    mFocusControl    = CG_NULL;
    mCursorLayer     = CG_NULL;
    mInitialized     = false;

    // Register us with the mouse and keyboard input messaging groups
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_MouseInput );
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_KeyboardInput );

    // Listen out for device reset events.
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_RenderDriver );
}

//-----------------------------------------------------------------------------
//  Name : ~cgUIManager () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUIManager::~cgUIManager()
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
void cgUIManager::dispose( bool bDisposeBase )
{
    SkinMap::iterator         itSkin;
    LayerList::iterator       itLayer;
    ImageLibraryMap::iterator itLibrary;

    // Iterate through and release all layers
    while ( (itLayer = mLayers.begin()) != mLayers.end() )
    {
        (*itLayer)->scriptSafeDispose();
        mLayers.erase( itLayer );
    
    } // End if found layer

    // Iterate through and release all loaded skins
    for ( itSkin = mSkins.begin(); itSkin != mSkins.end(); ++itSkin )
        delete itSkin->second;

    // Iterate through and release all loaded image libraries
    for ( itLibrary = mImageLibraries.begin(); itLibrary != mImageLibraries.end(); ++itLibrary )
        delete itLibrary->second;

    // Clear all lists
    mLayers.clear();
    mSkins.clear();
    mForms.clear();
    mGarbageForms.clear();
    mImageLibraries.clear();

    // Release allocated memory
    delete mTextEngine;

    // Clear variables
    mCursorLayer     = CG_NULL;
    mResourceManager = CG_NULL;
    mTextEngine      = CG_NULL;
    mCurrentSkin     = CG_NULL;
    mCapturedControl = CG_NULL;
    mFocusControl    = CG_NULL;
    mInitialized     = false;

    // Call base class implementation if required.
    if ( bDisposeBase == true )
        cgReference::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgUIManager * cgUIManager::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createSingleton () (Static)
/// <summary>
/// Creates the singleton. You would usually allocate the singleton in
/// the static member definition, however sometimes it's necessary to
/// call for allocation to allow for correct allocation ordering
/// and destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
        mSingleton = new cgUIManager();
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::destroySingleton( )
{
    // Destroy!
    delete mSingleton;
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the interface manager ready for processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::initialize( cgResourceManager * pResourceManager )
{
    // Validate requirements
    if ( mInitialized == true )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Cannot initialize the user interface system a second time after it has previously been initialized.\n"));
        return false;
    
    } // End if no resource manager

    // Validate requirements
    if ( pResourceManager == CG_NULL )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Cannot initialize the user interface without specifying a valid resource manager!\n"));
        return false;
    
    } // End if no resource manager

    // Store references
    mResourceManager = pResourceManager;

    // Allocate a text engine
    mTextEngine = new cgTextEngine( getRenderDriver() );

    // We are now initialized
    mInitialized = true;

    // Begin the garbage collection process
    sendGarbageMessage();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : begin()
/// <summary>
/// Once all skins have been added, this method should be called in order
/// to have the interface manager begin processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::begin( )
{
    if ( mInitialized == false )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The user interface system must be initialized before it can be started with a call to 'begin()'.\n"));
        return false;
    
    } // End if not yet initialized

    // Create the top level system layers (includes the layer on which the cursor is rendered)
    if ( createSystemLayers() == false )
        return false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createSystemLayers () (Private)
/// <summary>
/// Creates the system layers required for the user interface. These
/// include top level layers such as the one on which the cursor is
/// rendered.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::createSystemLayers( )
{
    // Create the cursor layer
    mCursorLayer = new cgUICursorLayer( this );

    // Initialize the cursor layer
    if ( mCursorLayer->initialize( ) == false )
    {
        delete mCursorLayer;
        mCursorLayer =  CG_NULL;
        return false;
    
    } // End if failed to init layer

     // Add this to the layer list
    mLayers.push_back( mCursorLayer );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createForm ()
/// <summary>
/// Create a form ready for display.
/// </summary>
//-----------------------------------------------------------------------------
cgUIForm * cgUIManager::createForm( const cgString & strFormType, const cgString & strFormName )
{
    // Allocate a new control layer to attach the form to
    cgUIControlLayer * pLayer = new cgUIControlLayer( this, cgUILayerType::UserLayer, -1 );

    // Allocate the form object
    cgUIForm * pForm = cgUIForm::createInstance( strFormType, pLayer );

    // Initialize the form
    if ( !pForm->createForm( strFormName ) )
    {
        delete pLayer;
        delete pForm;
        return CG_NULL;
    
    } // End if failed to load

    // Attach the form to the layer
    pLayer->attachControl( pForm );

    // Add to the main form list
    mForms.push_back( pForm );

    // Also add the layer to the layer list
    mLayers.push_front( pLayer );

    // Success!
    return pForm;
}

//-----------------------------------------------------------------------------
//  Name : loadForm ()
/// <summary>
/// Load a form from a script ready for display.
/// </summary>
//-----------------------------------------------------------------------------
cgUIForm * cgUIManager::loadForm( const cgInputStream & Stream, const cgString & strFormName )
{
    // Allocate a new control layer to attach the form to
    cgUIControlLayer * pLayer = new cgUIControlLayer( this, cgUILayerType::UserLayer, -1 );

    // Allocate the form object
    cgUIForm * pForm = new cgUIForm( pLayer );

    // Initialize the form
    if ( !pForm->loadForm( Stream, strFormName ) )
    {
        delete pLayer;
        delete pForm;
        return CG_NULL;
    
    } // End if failed to load

    // Attach the form to the layer
    pLayer->attachControl( pForm );

    // Add to the main form list
    mForms.push_back( pForm );

    // Also add the layer to the layer list
    mLayers.push_front( pLayer );

    // Success!
    return pForm;
}

//-----------------------------------------------------------------------------
//  Name : removeForm ()
/// <summary>
/// Destroy the specified form and remove it from the manager.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::removeForm( cgUIForm * pForm )
{
    // Find the form in the form list first and remove it.
    FormList::iterator itForm = std::find( mForms.begin(), mForms.end(), pForm );
    if ( itForm != mForms.end() )
        mForms.erase( itForm );
    
    // Now find the form's layer in the layer list and destroy. This
    // will automatically destroy the form itself.
    cgUILayer * pLayer = pForm->getControlLayer();
    LayerList::iterator itLayer = std::find( mLayers.begin(), mLayers.end(), pLayer );
    if ( itLayer != mLayers.end() )
        mLayers.erase( itLayer );
    pLayer->scriptSafeDispose();
}

//-----------------------------------------------------------------------------
//  Name : addLayer ()
/// <summary>
/// Add a new UI layer to the system for rendering / management.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::addLayer( cgUILayer * pLayer )
{
    if ( pLayer->getLayerType() == cgUILayerType::SystemLayer )
    {
        // Insert below current system layers.
        LayerList::iterator itLayer;
        for ( itLayer = mLayers.begin(); itLayer != mLayers.end(); ++itLayer )
        {
            // If this is the first system layer, we have found our top most item
            if ( (*itLayer)->getLayerType() == cgUILayerType::SystemLayer )
                 break;

        } // Next Layer

        // Did we find a system layer?
        if ( itLayer == mLayers.end() )
        {
            // We didn't find one, just insert back at the end
            mLayers.push_back( pLayer );
        
        } // End if no
        else
        {
            // First non-system layer found, insert just prior to this
            mLayers.insert( itLayer, pLayer );

        } // End if yes
    
    } // End if system
    else
        mLayers.push_front( pLayer );
}

//-----------------------------------------------------------------------------
//  Name : removeLayer ()
/// <summary>
/// Remove the specified UI layer from the system. The layer can optionally be
/// destroyed.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::removeLayer( cgUILayer * pLayer, bool destroyLayer )
{
    LayerList::iterator itLayer = std::find( mLayers.begin(), mLayers.end(), pLayer );
    if ( itLayer != mLayers.end() )
    {
        mLayers.erase( itLayer );
        if ( destroyLayer )
            pLayer->scriptSafeDispose();
    
    } // End if found
}

//-----------------------------------------------------------------------------
//  Name : bringLayerToFront ()
/// <summary>
/// Move the interface layer to the front of its class of layers.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::bringLayerToFront( cgUILayer * pLayer )
{
    cgUILayer          * pTestLayer;
    LayerList::iterator  itLayer;
    
    // Do nothing if this is a system layer
    if ( pLayer->getLayerType() == cgUILayerType::SystemLayer )
        return;

    // First, remove this layer from the list.
    mLayers.remove( pLayer );

    // Now, find the top most (non system) layer in its class
    for ( itLayer = mLayers.begin(); itLayer != mLayers.end(); ++itLayer )
    {
        // Retrieve the layer we're testing against
        pTestLayer = *itLayer;

        // If this is the first system layer, we have found our top most item
        if ( pTestLayer->getLayerType() == cgUILayerType::SystemLayer )
             break;

    } // Next Layer

    // Did we find a system layer?
    if ( itLayer == mLayers.end() )
    {
        // We didn't find one, just insert back at the end
        mLayers.push_back( pLayer );
    
    } // End if no
    else
    {
        // First non-system layer found, insert just prior to this
        mLayers.insert( itLayer, pLayer );

    } // End if yes

}

//-----------------------------------------------------------------------------
//  Name : sendLayerToBack ()
/// <summary>
/// Move the interface layer to the back of its class of layers.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::sendLayerToBack( cgUILayer * pLayer )
{
    // Do nothing if this is a system layer
    if ( pLayer->getLayerType() == cgUILayerType::SystemLayer )
        return;

    // First, remove this layer from the list.
    mLayers.remove( pLayer );

    // Just insert at the head, no other layer types will affect this at the moment
    mLayers.push_front( pLayer );
}

//-----------------------------------------------------------------------------
//  Name : addSkin()
/// <summary>
/// Load a skin into the interface manager based on the XML skin definition
/// file specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::addSkin( cgInputStream Definition )
{
    cgString strSkinKey, strGlyphDefinition;

    // Allocate a new skin object
    cgUISkin * pSkin = new cgUISkin( );

    // Attempt to load the definition
    if ( !pSkin->loadDefinition( Definition ) )
    {
        // Clean up and bail (any relevant errors will have 
        // been logged during the loadDefinition call)
        delete pSkin;
        return false;

    } // End if failed

    // Generate a lower case version of the skin name to ensure that
    // the skin selection by name is case insensitive.
    strSkinKey = cgString::toLower(pSkin->getName());

    // A skin already exists with this name?
    if ( mSkins.find( strSkinKey ) != mSkins.end() )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s : A skin with duplicate name '%s' was found when parsing skin definition XML.\n"), Definition.getName().c_str(), pSkin->getName().c_str() );

        // Clean up and bail
        delete pSkin;
        return false;
    
    } // End if already exists

    // Load glyph library if available
    strGlyphDefinition = pSkin->getGlyphDefinition();
    if ( strGlyphDefinition.empty() == false )
    {
        // Load the glyph image library
        if ( addImageLibrary( strGlyphDefinition, strSkinKey + _T("_SystemGlyphs") ) == false )
        {
            delete pSkin;
            return false;

        } // End if failed

    } // End if glyph definition specified

    // Store in our list of available skins
    mSkins[ strSkinKey ] = pSkin;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : selectSkin()
/// <summary>
/// Select the skin that you would like all skin aware interface elements
/// to use.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::selectSkin( const cgString &strSkinName )
{
    mCurrentSkin = getSkinDefinition( strSkinName );
    return ( mCurrentSkin != CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : getSkinDefinition()
/// <summary>
/// Retrieve the definition object for the skin with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgUISkin * cgUIManager::getSkinDefinition( const cgString &strSkinName )
{
    cgString          strSkinKey;
    SkinMap::iterator itSkin;

    // Generate a lower case version of the skin name to ensure that
    // the skin selection by name is case insensitive.
    strSkinKey = cgString::toLower(strSkinName);

    // Attempt to find the skin by name
    itSkin = mSkins.find( strSkinKey );
    if ( itSkin == mSkins.end() ) return CG_NULL;

    // Return the referenced skin
    return itSkin->second;
}

//-----------------------------------------------------------------------------
//  Name : getSkinGlyphLibrary ()
/// <summary>
/// If a skin is currently selected, return the image library reference 
/// name for it's glyph images.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgUIManager::getSkinGlyphLibrary( ) const
{
    cgString strLibrary;

    // No skin selected?
    if ( mCurrentSkin == CG_NULL )
        return strLibrary;

    // Otherwise build it from the name as a whole
    strLibrary = cgString::toLower(mCurrentSkin->getName());
    strLibrary += _T("_SystemGlyphs");

    // Return the library reference name
    return strLibrary;
}

//-----------------------------------------------------------------------------
//  Name : addFont ()
/// <summary>
/// Load a font set into the text engine based on the XML font definition
/// file specified.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgUIManager::addFont( cgInputStream Definition )
{
    if ( !mTextEngine ) return false;
    cgString strFontName = mTextEngine->addFont( Definition );
    if ( strFontName.empty() == true )
        return strFontName;

    // Success!
    return strFontName;
}

//-----------------------------------------------------------------------------
//  Name : selectFont ()
/// <summary>
/// Select the font that the application would like to use the next time
/// 'DrawText' is called.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::selectFont( const cgString & strFontName )
{
    // Validate requirements
    if ( !mTextEngine )
        return false;
    
    // If the string is empty, select the default font
    if ( strFontName.empty() == true )
        return selectDefaultFont();

    // Otherwise select the specified font.
    return mTextEngine->setCurrentFont( strFontName );
}

//-----------------------------------------------------------------------------
//  Name : selectDefaultFont ()
/// <summary>
/// Select the default font into the text engine ready for drawing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::selectDefaultFont(  )
{
    if ( !mTextEngine )
        return false;
    return mTextEngine->setCurrentFont( mDefaultFont );
}

//-----------------------------------------------------------------------------
//  Name : setDefaultFont ()
/// <summary>
/// Set the default font that you would like to be used whenever no
/// specific font is specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::setDefaultFont( const cgString & strFontName  )
{
    // Validate requirements
    if ( !mTextEngine )
        return false;

    // Can the specified font be selected?
    if ( mTextEngine->setCurrentFont( strFontName ) == false )
        return false;

    // Store the default font
    mDefaultFont = strFontName;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( cgInt32 x, cgInt32 y, const cgString & strText )
{
    return printText( cgPoint(x,y), strText, 0, 0xFFFFFFFF, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( cgInt32 x, cgInt32 y, const cgString & strText, cgUInt32 nFlags )
{
    return printText( cgPoint(x,y), strText, nFlags, 0xFFFFFFFF, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( cgInt32 x, cgInt32 y, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor )
{
    return printText( cgPoint(x,y), strText, nFlags, nColor, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( cgInt32 x, cgInt32 y, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor, cgInt32 nKerning )
{
    return printText( cgPoint(x,y), strText, nFlags, nColor, nKerning, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( cgInt32 x, cgInt32 y, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor, cgInt32 nKerning, cgInt32 nLineSpacing )
{
    return printText( cgPoint(x,y), strText, nFlags, nColor, nKerning, nLineSpacing );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgPoint & ptScreen, const cgString & strText )
{
    return printText( ptScreen, strText, 0, 0xFFFFFFFF, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgPoint & ptScreen, const cgString & strText, cgUInt32 nFlags )
{
    return printText( ptScreen, strText, nFlags, 0xFFFFFFFF, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgPoint & ptScreen, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor )
{
    return printText( ptScreen, strText, nFlags, nColor, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgPoint & ptScreen, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor, cgInt32 nKerning )
{
    return printText( ptScreen, strText, nFlags, nColor, nKerning, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a single position in the screen which will be
/// used as the top left origin from which to draw text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgPoint & ptScreen, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor, cgInt32 nKerning, cgInt32 nLineSpacing )
{
    // Validate requirements
    if ( mTextEngine == CG_NULL )
        return cgRect(0,0,0,0);

    // Get access to required systems
    cgRenderDriver * pDriver = getRenderDriver();
    cgSize ScreenSize = pDriver->getScreenSize();

    // Set the text rendering properties
    cgInt32 oldKerning = mTextEngine->getKerning();
    cgInt32 oldLineSpacing = mTextEngine->getLineSpacing();
    cgUInt32 oldColor = mTextEngine->getColor();
    mTextEngine->setKerning( nKerning );
    mTextEngine->setLineSpacing( nLineSpacing );
    mTextEngine->setColor( nColor );

    // Render the text
    cgRect rcText( ptScreen.x, ptScreen.y, ScreenSize.width, ScreenSize.height );
    cgRect rcResult = mTextEngine->printText( rcText, nFlags, strText );

    // Restore old settings
    mTextEngine->setKerning( oldKerning );
    mTextEngine->setLineSpacing( oldLineSpacing );
    mTextEngine->setColor( oldColor );

    // Return the area of the screen we rendered to.
    return rcResult;
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text in addition to supplying a rendering offset.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, const cgPoint & ptOffset )
{
    return printText( rcScreen, strText, ptOffset, 0, 0xFFFFFFFF, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text in addition to supplying a rendering offset.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, const cgPoint & ptOffset, cgUInt32 nFlags )
{
    return printText( rcScreen, strText, ptOffset, nFlags, 0xFFFFFFFF, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text in addition to supplying a rendering offset.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, const cgPoint & ptOffset, cgUInt32 nFlags, cgUInt32 nColor )
{
    return printText( rcScreen, strText, ptOffset, nFlags, nColor, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text in addition to supplying a rendering offset.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, const cgPoint & ptOffset, cgUInt32 nFlags, cgUInt32 nColor, cgInt32 nKerning )
{
    return printText( rcScreen, strText, ptOffset, nFlags, nColor, nKerning, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text in addition to supplying a rendering offset.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, const cgPoint & ptOffset, cgUInt32 nFlags, cgUInt32 nColor, cgInt32 nKerning, cgInt32 nLineSpacing )
{
    // Validate requirements
    if ( mTextEngine == CG_NULL )
        return cgRect(0,0,0,0);

    // Set the text rendering properties
    cgInt32 oldKerning = mTextEngine->getKerning();
    cgInt32 oldLineSpacing = mTextEngine->getLineSpacing();
    cgUInt32 oldColor = mTextEngine->getColor();
    mTextEngine->setKerning( nKerning );
    mTextEngine->setLineSpacing( nLineSpacing );
    mTextEngine->setColor( nColor );

    // Render the text
    cgRect rcResult = mTextEngine->printText( rcScreen, nFlags, strText, ptOffset );

    // Restore old settings
    mTextEngine->setKerning( oldKerning );
    mTextEngine->setLineSpacing( oldLineSpacing );
    mTextEngine->setColor( oldColor );

    // Return the area of the screen we rendered to.
    return rcResult;
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText )
{
    return printText( rcScreen, strText, 0, 0xFFFFFFFF, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, cgUInt32 nFlags )
{
    return printText( rcScreen, strText, nFlags, 0xFFFFFFFF, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor )
{
    return printText( rcScreen, strText, nFlags, nColor, 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor, cgInt32 nKerning )
{
    return printText( rcScreen, strText, nFlags, nColor, nKerning, 0 );
}

//-----------------------------------------------------------------------------
//  Name : printText ()
/// <summary>
/// Actually draw the text to the frame buffer.
/// Note : This overload accepts a full rectangle and can be used for multiline
/// or clipped text.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIManager::printText( const cgRect & rcScreen, const cgString & strText, cgUInt32 nFlags, cgUInt32 nColor, cgInt32 nKerning, cgInt32 nLineSpacing )
{
    // Validate requirements
    if ( mTextEngine == CG_NULL )
        return cgRect(0,0,0,0);

    // Set the text rendering properties
    cgInt32 oldKerning = mTextEngine->getKerning();
    cgInt32 oldLineSpacing = mTextEngine->getLineSpacing();
    cgUInt32 oldColor = mTextEngine->getColor();
    mTextEngine->setKerning( nKerning );
    mTextEngine->setLineSpacing( nLineSpacing );
    mTextEngine->setColor( nColor );

    // Render the text
    cgRect rcResult = mTextEngine->printText( rcScreen, nFlags, strText );

    // Restore old settings
    mTextEngine->setKerning( oldKerning );
    mTextEngine->setLineSpacing( oldLineSpacing );
    mTextEngine->setColor( oldColor );

    // Return the area of the screen we rendered to.
    return rcResult;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Draw all currently active user interface components.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::render( )
{
    cgUILayer         * pLayer = CG_NULL;
    LayerList::iterator itLayer;
    
    // Iterate through each layer and render them in order
    for ( itLayer = mLayers.begin(); itLayer != mLayers.end(); ++itLayer )
    {
        pLayer = *itLayer;
        if ( pLayer == CG_NULL ) continue;

        // Render the layer data itself
        pLayer->render();

    } // Next Layer
}

//-----------------------------------------------------------------------------
//  Name : getRenderDriver ()
/// <summary>
/// Retrieve the render driver through which the interface is being
/// rendered.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver * cgUIManager::getRenderDriver( )
{
    // Validate requirements
    if ( mResourceManager == CG_NULL )
        return CG_NULL;

    // Return the render driver to which the resource manager is attached
    return mResourceManager->getRenderDriver();
}

//-----------------------------------------------------------------------------
//  Name : setCapture ()
/// <summary>
/// Set the control that is currently "captured" by the mouse.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::setCapture( cgUIControl * pControl )
{
    mCapturedControl = pControl;
}

//-----------------------------------------------------------------------------
//  Name : setFocus ()
/// <summary>
/// Set the control that currently has focus.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::setFocus( cgUIControl * pControl )
{
    // Is this a no-op?
    if ( pControl == mFocusControl )
        return;

    // If specified control cannot gain focus, skip.
    if ( pControl && !pControl->canGainFocus() )
        return;

    // Notify the prior control that it lost focus.
    if ( mFocusControl )
        mFocusControl->onLostFocus();

    // Swap focus control.
    mFocusControl = pControl;

    // Notify the new control that it gained focus.
    if ( mFocusControl )
        mFocusControl->onGainFocus();
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIManager )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : checkGarbage () (Private)
/// <summary>
/// Check the contents of the garbage list.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::checkGarbage( )
{
    FormList::iterator itForm;
    for ( itForm = mGarbageForms.begin(); itForm != mGarbageForms.end(); ++itForm )
    {
        // Perform the full close operation.
        removeForm( (*itForm) );
    
    } // Next form
    mGarbageForms.clear();
}

//-----------------------------------------------------------------------------
//  Name : sendGarbageMessage () (Private)
/// <summary>
/// Sends the garbage collection message to ourselves.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::sendGarbageMessage( )
{
    // Initialize message structure
    cgMessage Msg;
    Msg.messageId = cgSystemMessages::UI_CollectGarbage;

    // Send the message to ourselves with a miniscule delay so that we clean up on the next message poll
    cgReferenceManager::sendMessageTo( getReferenceId(), getReferenceId(), &Msg, 0.00001f );
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::processMessage( cgMessage * pMessage )
{
    cgUILayer                 * pLayer;
    LayerList::reverse_iterator itLayer;

    // Ignore messages when in direct processing mode.
    if ( cgInputDriver::getInstance()->getMouseMode() == cgMouseHandlerMode::Direct )
        return false;

    // What type of message is this?
    switch ( pMessage->messageId )
    {
        case cgSystemMessages::UI_CollectGarbage:
        {
            // Check for garbage that needs to be unloaded.
            checkGarbage();

            // Send the garbage collect message again if we aren't in the process of
            // being unregistered.
            if ( !(pMessage->fromId == getReferenceId() && pMessage->sourceUnregistered) )
                sendGarbageMessage();
        
        } // End case UI_CollectGarbage
        case cgSystemMessages::RenderDriver_ScreenLayoutChange:
        {
            // Iterate through each layer and notify them from the top layer down
            for ( itLayer = mLayers.rbegin(); itLayer != mLayers.rend(); ++itLayer )
            {
                pLayer = *itLayer;
                if ( pLayer == CG_NULL ) continue;

                // Notify layout change
                if ( pLayer->onScreenLayoutChange( ) == true )
                    break;

            } // Next Layer

            // This message was processed by us
            return true;

        } // End Case RenderDriver_ScreenLayoutChange
        case cgSystemMessages::InputDriver_MouseMoved:
        {
            const cgMouseMoveEventArgs * pData = (cgMouseMoveEventArgs*)pMessage->messageData;

            // Iterate through each layer and notify them from the top layer down
            for ( itLayer = mLayers.rbegin(); itLayer != mLayers.rend(); ++itLayer )
            {
                pLayer = *itLayer;
                if ( pLayer == CG_NULL ) continue;

                // Notify mouse moved
                if ( pLayer->onMouseMove( pData->position, pData->offset ) == true )
                    break;

            } // Next Layer

            // This message was processed by us
            return true;

        } // End Case InputDriver_MouseMoved

        case cgSystemMessages::InputDriver_MouseButtonDown:
        {
            const cgMouseButtonEventArgs * pData = (cgMouseButtonEventArgs*)pMessage->messageData;

            // Iterate through each layer and notify them from the top layer down
            for ( itLayer = mLayers.rbegin(); itLayer != mLayers.rend(); ++itLayer )
            {
                pLayer = *itLayer;
                if ( pLayer == CG_NULL ) continue;

                // Notify mouse down
                if ( pLayer->onMouseButtonDown( pData->buttons, pData->position ) == true )
                    break;

            } // Next Layer

            // This message was processed by us
            return true;

        } // End Case InputDriver_MouseButtonDown

        case cgSystemMessages::InputDriver_MouseButtonUp:
        {
            const cgMouseButtonEventArgs * pData = (cgMouseButtonEventArgs*)pMessage->messageData;

            // Iterate through each layer and notify them from the top layer down
            for ( itLayer = mLayers.rbegin(); itLayer != mLayers.rend(); ++itLayer )
            {
                if ( (pLayer = *itLayer) == CG_NULL )
                    continue;

                // Notify mouse up
                bool bProcessed = pLayer->onMouseButtonUp( pData->buttons, pData->position );
                
                // Also send a zero offset mouse movement to ensure hover state gets updated, etc.
                if ( bProcessed == true )
                    pLayer->onMouseMove( pData->position, cgPointF( 0, 0 ) );

                // Did this layer handle the message?
                if ( bProcessed == true )
                    break;

            } // Next Layer

            // This message was processed by us
            return true;

        } // End Case InputDriver_MouseButtonUp

        case cgSystemMessages::InputDriver_MouseWheelScroll:
        {
            const cgMouseWheelScrollEventArgs * pData = (cgMouseWheelScrollEventArgs*)pMessage->messageData;

            // Iterate through each layer and notify them from the top layer down
            for ( itLayer = mLayers.rbegin(); itLayer != mLayers.rend(); ++itLayer )
            {
                pLayer = *itLayer;
                if ( pLayer == CG_NULL ) continue;

                // Notify mouse moved
                if ( pLayer->onMouseWheelScroll( pData->delta, pData->position ) == true )
                    break;

            } // Next Layer

            // This message was processed by us
            return true;

        } // End Case InputDriver_MouseWheelScroll

        case cgSystemMessages::InputDriver_KeyDown:
        {
            const cgKeyEventArgs * pData = (cgKeyEventArgs*)pMessage->messageData;

            // Iterate through each layer and notify them from the top layer down
            for ( itLayer = mLayers.rbegin(); itLayer != mLayers.rend(); ++itLayer )
            {
                pLayer = *itLayer;
                if ( pLayer == CG_NULL ) continue;

                // Notify mouse moved
                if ( pLayer->onKeyDown( pData->keyCode, pData->modifiers ) == true )
                    break;

            } // Next Layer

            // This message was processed by us
            return true;

        } // End Case InputDriver_KeyDown

        case cgSystemMessages::InputDriver_KeyUp:
        {
            const cgKeyEventArgs * pData = (cgKeyEventArgs*)pMessage->messageData;

            // Iterate through each layer and notify them from the top layer down
            for ( itLayer = mLayers.rbegin(); itLayer != mLayers.rend(); ++itLayer )
            {
                pLayer = *itLayer;
                if ( pLayer == CG_NULL ) continue;

                // Notify mouse moved
                if ( pLayer->onKeyUp( pData->keyCode, pData->modifiers ) == true )
                    break;

            } // Next Layer

            // This message was processed by us
            return true;

        } // End Case InputDriver_KeyUp

        case cgSystemMessages::InputDriver_KeyPressed:
        {
            const cgKeyEventArgs * pData = (cgKeyEventArgs*)pMessage->messageData;

            // Iterate through each layer and notify them from the top layer down
            for ( itLayer = mLayers.rbegin(); itLayer != mLayers.rend(); ++itLayer )
            {
                pLayer = *itLayer;
                if ( pLayer == CG_NULL ) continue;

                // Notify mouse moved
                if ( pLayer->onKeyPressed( pData->keyCode, pData->modifiers ) == true )
                    break;

            } // Next Layer

            // This message was processed by us
            return true;

        } // End Case InputDriver_KeyPressed

    } // End Switch nMessageId

    // No message processed
    return cgReference::processMessage( pMessage );
}

//-----------------------------------------------------------------------------
//  Name : selectCursor ()
/// <summary>
/// Call this method to select the current cursor to use from the
/// cursor definition.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::selectCursor( const cgString &strCursor )
{
    if ( mCursorLayer != CG_NULL )
        mCursorLayer->selectCursor( strCursor );
}

//-----------------------------------------------------------------------------
//  Name : addImage ()
/// <summary>
/// Add a single image file to the interface system ready for rendering
/// through the 'drawImage' method.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::addImage( cgInputStream ImageFile, const cgString & strReferenceName )
{
    cgBillboardBuffer * pBuffer = CG_NULL;

    // Does the specified library already exist?
    if ( mImageLibraries.find( strReferenceName ) != mImageLibraries.end() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to add image '%s' to the interface manager because an image or library with the same reference name ('%s') already existed.\n"), ImageFile.getName().c_str(), strReferenceName.c_str() );
        return false;
    
    } // End if already exists

    // Allocate a new billboard buffer and prepare it with a single frame
    // of the full size of the specified image.
    pBuffer = new cgBillboardBuffer();
    if ( pBuffer->prepareBuffer( cgBillboardBuffer::ScreenSpace, getRenderDriver(), ImageFile ) == false )
    {
        delete pBuffer;
        return false;
    
    } // End if failed to prepare

    // Add a single 2D billboard for us to use when drawing
    pBuffer->addBillboard( new cgBillboard2D( ) );

    // Finish preparing
    pBuffer->endPrepare();

    // Add to our list of image libraries
    mImageLibraries[ strReferenceName ] = pBuffer;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addImageLibrary ()
/// <summary>
/// Add a series of images, packed into a single image, based on the
/// image atlas definition file specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::addImageLibrary( cgInputStream AtlasFile, const cgString & strReferenceName )
{
    cgBillboardBuffer * pBuffer = CG_NULL;

    // Does the specified library already exist?
    if ( mImageLibraries.find( strReferenceName ) != mImageLibraries.end() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to add image data from atlas '%s' to the interface manager because an image or library with the same reference name ('%s') already existed.\n"), AtlasFile.getName().c_str(), strReferenceName.c_str() );
        return false;
    
    } // End if already exists

    // Allocate a new billboard buffer and prepare it with the frames
    // specified in the image atlas.
    pBuffer = new cgBillboardBuffer();
    if ( pBuffer->prepareBufferFromAtlas( cgBillboardBuffer::ScreenSpace, getRenderDriver(), AtlasFile ) == false )
    {
        delete pBuffer;
        return false;
    
    } // End if failed to prepare

    // Add a single 2D billboard for us to use when drawing
    pBuffer->addBillboard( new cgBillboard2D() );

    // Finish preparing
    pBuffer->endPrepare();

    // Add to our list of image libraries
    mImageLibraries[ strReferenceName ] = pBuffer;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getImageSize ()
/// <summary>
/// Retrieve the original size of the specified image.
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgUIManager::getImageSize( const cgString & strReferenceName ) const
{
    return getImageSize( strReferenceName, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getImageSize ()
/// <summary>
/// Retrieve the original size of the specified image.
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgUIManager::getImageSize( const cgString & strReferenceName, const cgString & strLibraryItem ) const
{
    const cgBillboardBuffer            * pBuffer     = CG_NULL;
    const cgBillboardBuffer::FrameDesc * pDesc       = CG_NULL;
    cgInt16                              nFrameIndex = 0;
    cgSize                               ImageSize   = cgSize(0,0);
    ImageLibraryMap::const_iterator      itLibrary;

    // Find the specified image library
    itLibrary = mImageLibraries.find( strReferenceName );
    if ( itLibrary == mImageLibraries.end() )
        return ImageSize;

    // Retrieve the billboard buffer so we have easy access
    pBuffer = itLibrary->second;

    // Was a library item specified?
    if ( strLibraryItem.empty() == false )
    {
        nFrameIndex = pBuffer->getFrameIndex( 0, strLibraryItem );
        if ( nFrameIndex < 0 )
            return ImageSize;

    } // End if library item specified

    // Get the description of the specified item
    pDesc = pBuffer->getFrameData( 0, nFrameIndex );
    if ( pDesc == CG_NULL )
        return ImageSize;
    
    // Return the size
    ImageSize.width  = pDesc->bounds.right - pDesc->bounds.left;
    ImageSize.height = pDesc->bounds.bottom - pDesc->bounds.top;
    return ImageSize;
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the image using one of the automatic drawing placement modes.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( cgImageScaleMode::Base Mode, const cgString & strReferenceName )
{
    drawImage( Mode, strReferenceName, cgString::Empty, cgColorValue(1,1,1,1) );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the image using one of the automatic drawing placement modes.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( cgImageScaleMode::Base Mode, const cgString & strReferenceName, const cgString & strLibraryItem )
{
    drawImage( Mode, strReferenceName, strLibraryItem, cgColorValue(1,1,1,1) );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the image using one of the automatic drawing placement modes.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( cgImageScaleMode::Base Mode, const cgString & strReferenceName, const cgString & strLibraryItem, const cgColorValue & Color )
{
    cgRect rcImage;
    cgSize ScreenSize, ImageSize, NewImageSize;
    bool   bFilter = false;

    // Get access to required systems
    cgRenderDriver * pDriver = getRenderDriver();

    // Collect selected image and screen size
    ImageSize  = getImageSize( strReferenceName, strLibraryItem );
    ScreenSize = pDriver->getScreenSize();
    if ( ImageSize.width == 0 && ImageSize.height == 0 )
        return;

    cgToDo( "User Interface", "Use render view and viewport values!" );

    // ToDo : Support Center & None

    // Compute the correct image rectangle
    switch ( Mode )
    {
        case cgImageScaleMode::Stretch:
            // Compute full screen rectangle.
            rcImage.left   = 0;
            rcImage.top    = 0;
            rcImage.right  = ScreenSize.width;
            rcImage.bottom = ScreenSize.height;
            bFilter        = true;
            break;

        case cgImageScaleMode::Fit:
            // Ensure entire image fits based on largest axis.
            if ( ImageSize.width > ImageSize.height )
            {
                // Compute the new aspect correct image size
                NewImageSize.width  = ScreenSize.width;
                NewImageSize.height = (cgInt32)((cgFloat)ImageSize.height * ((cgFloat)ScreenSize.width / (cgFloat)ImageSize.width));
            
            } // End if width largest
            else
            {
                // Compute the new aspect correct image size
                NewImageSize.width  = (cgInt32)((cgFloat)ImageSize.width * ((cgFloat)ScreenSize.height / (cgFloat)ImageSize.height));
                NewImageSize.height = ScreenSize.height;
            
            } // End if height largest

            // Now compute required rectangle
            rcImage.top     = (ScreenSize.height - NewImageSize.height) / 2;
            rcImage.bottom  = rcImage.top + NewImageSize.height;
            rcImage.left    = (ScreenSize.width - NewImageSize.width) / 2;
            rcImage.right   = rcImage.left + NewImageSize.width;
            bFilter         = true;
            break; 

        case cgImageScaleMode::FitV:
            // Compute the new aspect correct image size
            NewImageSize.width  = (cgInt32)((cgFloat)ImageSize.width * ((cgFloat)ScreenSize.height / (cgFloat)ImageSize.height));
            NewImageSize.height = ScreenSize.height;

            // Now compute required rectangle
            rcImage.top     = 0;
            rcImage.bottom  = NewImageSize.height;
            rcImage.left    = (ScreenSize.width - NewImageSize.width) / 2;
            rcImage.right   = rcImage.left + NewImageSize.width;
            bFilter         = true;
            break;

        case cgImageScaleMode::FitU:
            // Compute the new aspect correct image size
            NewImageSize.width  = ScreenSize.width;
            NewImageSize.height = (cgInt32)((cgFloat)ImageSize.height * ((cgFloat)ScreenSize.width / (cgFloat)ImageSize.width));

            // Now compute required rectangle
            rcImage.left    = 0;
            rcImage.right   = NewImageSize.height;
            rcImage.top     = (ScreenSize.height - NewImageSize.height) / 2;
            rcImage.bottom  = rcImage.top + NewImageSize.height;
            bFilter         = true;
            break;

        default:
            return;
    
    } // End switch Mode

    // Draw the image using the new rectangle
    drawImage( rcImage, strReferenceName, strLibraryItem, Color, bFilter );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the image in its original size, offset to the specified location
/// on the screen.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgPoint & ptOffset, const cgString & strReferenceName )
{
    cgSize Size = getImageSize( strReferenceName, cgString::Empty );

    // No such image?
    if ( Size.width == 0 || Size.height == 0 )
        return;

    // Draw the image
    cgRect rcImage( ptOffset.x, ptOffset.y, ptOffset.x + Size.width, ptOffset.y + Size.height );
    drawImage( rcImage, strReferenceName, cgString::Empty, cgColorValue(1,1,1,1), false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the image in its original size, offset to the specified location
/// on the screen.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgPoint & ptOffset, const cgString & strReferenceName, const cgString & strLibraryItem )
{
    cgSize Size = getImageSize( strReferenceName, strLibraryItem );

    // No such image?
    if ( Size.width == 0 || Size.height == 0 )
        return;

    // Draw the image
    cgRect rcImage( ptOffset.x, ptOffset.y, ptOffset.x + Size.width, ptOffset.y + Size.height );
    drawImage( rcImage, strReferenceName, strLibraryItem, cgColorValue(1,1,1,1), false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the image in its original size, offset to the specified location
/// on the screen.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgPoint & ptOffset, const cgString & strReferenceName, const cgString & strLibraryItem, const cgColorValue & Color )
{
    cgSize Size = getImageSize( strReferenceName, strLibraryItem );

    // No such image?
    if ( Size.width == 0 || Size.height == 0 )
        return;

    // Draw the image
    cgRect rcImage( ptOffset.x, ptOffset.y, ptOffset.x + Size.width, ptOffset.y + Size.height );
    drawImage( rcImage, strReferenceName, strLibraryItem, Color, false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the specified image to the display.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgRect & rcImage, const cgString & strReferenceName )
{
    drawImage( cgRectF( (cgFloat)rcImage.left, (cgFloat)rcImage.top, (cgFloat)rcImage.right, (cgFloat)rcImage.bottom ),
               strReferenceName, cgString::Empty, cgColorValue(1,1,1,1), false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the specified image to the display.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgRect & rcImage, const cgString & strReferenceName, const cgString & strLibraryItem )
{
    drawImage( cgRectF( (cgFloat)rcImage.left, (cgFloat)rcImage.top, (cgFloat)rcImage.right, (cgFloat)rcImage.bottom ),
               strReferenceName, strLibraryItem, cgColorValue(1,1,1,1), false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the specified image to the display.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgRect & rcImage, const cgString & strReferenceName, const cgString & strLibraryItem, const cgColorValue & Color )
{
    drawImage( cgRectF( (cgFloat)rcImage.left, (cgFloat)rcImage.top, (cgFloat)rcImage.right, (cgFloat)rcImage.bottom ),
               strReferenceName, strLibraryItem, Color, false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the specified image to the display.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgRect & rcImage, const cgString & strReferenceName, const cgString & strLibraryItem, const cgColorValue & Color, bool bFilter )
{
    drawImage( cgRectF( (cgFloat)rcImage.left, (cgFloat)rcImage.top, (cgFloat)rcImage.right, (cgFloat)rcImage.bottom ),
               strReferenceName, strLibraryItem, Color, bFilter );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the specified image to the display.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgRectF & rcImage, const cgString & strReferenceName )
{
    drawImage( rcImage, strReferenceName, cgString::Empty, cgColorValue(1,1,1,1), false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the specified image to the display.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgRectF & rcImage, const cgString & strReferenceName, const cgString & strLibraryItem )
{
    drawImage( rcImage, strReferenceName, strLibraryItem, cgColorValue(1,1,1,1), false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the specified image to the display.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgRectF & rcImage, const cgString & strReferenceName, const cgString & strLibraryItem, const cgColorValue & Color )
{
    drawImage( rcImage, strReferenceName, strLibraryItem, Color, false );
}

//-----------------------------------------------------------------------------
//  Name : drawImage ()
/// <summary>
/// Draw the specified image to the display.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIManager::drawImage( const cgRectF & rcImage, const cgString & strReferenceName, const cgString & strLibraryItem, const cgColorValue & Color, bool bFilter )
{
    cgBillboardBuffer       * pBuffer     = CG_NULL;
    cgBillboard             * pBillboard  = CG_NULL;
    cgInt16                   nFrameIndex = 0;
    ImageLibraryMap::iterator itLibrary;

    // Find the specified image library
    itLibrary = mImageLibraries.find( strReferenceName );
    if ( itLibrary == mImageLibraries.end() )
        return;

    // Retrieve the billboard buffer so we have easy access
    pBuffer = itLibrary->second;

    // Was a library item specified?
    if ( strLibraryItem.empty() == false )
    {
        nFrameIndex = pBuffer->getFrameIndex( 0, strLibraryItem );
        if ( nFrameIndex < 0 )
            return;

    } // End if library item specified

    // Retrieve the billboard we're going to use for rendering
    pBillboard = pBuffer->getBillboard( 0 );
    if ( pBillboard == CG_NULL )
        return;

    // Set the billboard properties
    pBillboard->setFrame( nFrameIndex );
    pBillboard->setPosition( rcImage.left, rcImage.top, 0 );
    pBillboard->setSize( (rcImage.right - rcImage.left), (rcImage.bottom - rcImage.top) );
    pBillboard->setColor( Color );
    pBillboard->update();

    // Allow the billboard buffer to render
    pBuffer->render( 0, -1, (bFilter == false) );
}

//-----------------------------------------------------------------------------
//  Name : removeImageLibrary ()
/// <summary>
/// Remove an image library from memory.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::removeImageLibrary( const cgString & strReferenceName )
{
    ImageLibraryMap::iterator itLibrary;

    // Find the specified image library
    itLibrary = mImageLibraries.find( strReferenceName );
    if ( itLibrary == mImageLibraries.end() )
        return false;

    // Delete the billboard buffer
    if ( itLibrary->second != CG_NULL )
        delete itLibrary->second;
    
    // Remove this from the map
    mImageLibraries.erase( itLibrary );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : isImageLibraryLoaded ()
/// <summary>
/// Determine if an image library is already resident.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIManager::isImageLibraryLoaded( const cgString & strReferenceName ) const
{
    return (mImageLibraries.find( strReferenceName ) != mImageLibraries.end() );
}