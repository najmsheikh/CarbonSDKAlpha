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
// Name : cgImageBoxControl.cpp                                              //
//                                                                           //
// Desc : Built in user interface image box control.                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgImageBoxControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgImageBoxControl.h>
#include <Interface/cgUIManager.h>
#include <Interface/cgUIForm.h>

// ToDo: Remove these comments once completed.
// ToDo: Image box needs scrollbars for when no scale mode is used and image is larger than box.

///////////////////////////////////////////////////////////////////////////////
// cgImageBoxControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgImageBoxControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgImageBoxControl::cgImageBoxControl( ) : cgUIControl( Simple, _T("") )
{
    // Initialize variables to sensible defaults
    mImageReference    = _T("");
    mLibrary     = _T("");
    mLibraryItem = _T("");
    mScaleMode      = cgImageScaleMode::Stretch;
    autoSize         = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageBoxControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgImageBoxControl::~cgImageBoxControl()
{
    // Release allocated memory
    // Clear variables
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgImageBoxControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIImageBoxControl )
        return true;

    // Supported by base?
    return cgUIControl::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : renderSecondary () (Virtual)
/// <summary>
/// Render any secondary elements for this control. This would include
/// items such as the text for any controls.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageBoxControl::renderSecondary( )
{
    cgString strLibrary;
    cgRect   rcClient = getClientArea( cgControlCoordinateSpace::ScreenRelative );
    
    // No image currently set or invisible?
    if ( !isVisible() || mImageReference.empty() )
        return;

    // Get access to interface manager
    cgUIManager    * pManager = mRootForm->getUIManager();
    cgRenderDriver * pDriver  = pManager->getRenderDriver();

    // Pulling from system glyph library?
    if ( mLibrary.empty() )
        strLibrary = pManager->getSkinGlyphLibrary();
    else
        strLibrary = mLibrary;

    // What rendering mode are we using?
    if ( mScaleMode == cgImageScaleMode::Stretch )
    {
        // Draw the specified image to fill the client area
        pManager->drawImage( rcClient, strLibrary, mLibraryItem );
    
    } // End if stretch to fill
    else if ( mScaleMode == cgImageScaleMode::None )
    {
        // Enable clipping of the image to client area.
        pDriver->setScissorRect( &rcClient );

        // Draw at it's original size
        cgPoint ptPosition( rcClient.left, rcClient.top );
        pManager->drawImage( ptPosition, strLibrary, mLibraryItem );

        // Turn clipping off again
        pDriver->setScissorRect( CG_NULL );
    
    } // End if original size

    cgToDo( "User Interface", "Support other image draw modes!" );
 
    // Call base class implementation
    cgUIControl::renderSecondary();
}

//-----------------------------------------------------------------------------
//  Name : setImage ()
/// <summary>
/// Allows external code / scripts to select the image that should be
/// rendered inside this control.
/// Note : Reference is in the format "LIBRARY::ITEM".
/// </summary>
//-----------------------------------------------------------------------------
bool cgImageBoxControl::setImage( const cgString & strImageRef )
{
    cgSize Size;

    // Validate requirements
    if ( !mRootForm )
        return false;

    // ToDo: Should really be done with image library instead of perhaps a texture / resource handle?

    // Get access to manager
    cgUIManager * pManager = mRootForm->getUIManager();

    // Clear current image
    mImageReference    = _T("");
    mLibrary     = _T("");
    mLibraryItem = _T("");

    // Any new reference specified?
    if ( strImageRef.empty() )
        return true;

    // Attempt to tokenize with the namespace delimeter
    size_t nDelim = strImageRef.find( _T("::") );
    if ( nDelim != cgString::npos )
    {
        // Namespace delimeter specified, extract left and right
        // components.
        mLibrary     = cgString::trim(strImageRef.substr( 0, nDelim ));
        mLibraryItem = cgString::trim(strImageRef.substr( nDelim + 2 ));

    } // End if namespace delimeter found
    else
    {
        // No item reference, just select as the library
        mLibrary     = strImageRef;

    } // End if no delimeter
        
    // If this is not a system glyph, does the library exist?
    if ( !mLibrary.empty() )
    {
        // Any library by this name?
        if ( !pManager->isImageLibraryLoaded( mLibrary ) )
        {
            // Attempt to load this into the library
            if ( !pManager->addImage( mLibrary, mLibrary ) )
                return false;

        } // End if no image library

    } // End if not system glyph

    // Success! Store the specified reference
    mImageReference = strImageRef;

    // Get the size of the specified image (will return a degenerate size
    // if the image didn't exist)
    Size = getImageSize();
    if ( Size.width == 0 && Size.height == 0 )
    {
        // Image didn't actually exist
        mImageReference = _T("");
        return false;
    
    } // End if degenerate

    // Should auto size control?
    if ( autoSize )
        setSize( Size );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getImageSize ()
/// <summary>
/// Retrieve the original size of the selected image.
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgImageBoxControl::getImageSize( ) const
{
    cgString      strLibrary;
    cgUIManager * pManager = mRootForm->getUIManager();

    // No image currently set?
    if ( mImageReference.empty() )
        return cgSize(0,0);
    
    // Pulling from system glyph library?
    if ( mLibrary.empty() )
        strLibrary = pManager->getSkinGlyphLibrary();
    else
        strLibrary = mLibrary;

    // Return the size of the specified image
    return pManager->getImageSize( strLibrary, mLibraryItem );
}

//-----------------------------------------------------------------------------
//  Name : getImage ()
/// <summary>
/// Return the currently set image reference.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgImageBoxControl::getImage( ) const
{
    return mImageReference;
}

//-----------------------------------------------------------------------------
//  Name : setScaleMode ()
/// <summary>
/// Set the mode that describes how the image box will scale the image.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageBoxControl::setScaleMode( cgImageScaleMode::Base Mode )
{
    // Is this a no-op?
    if ( mScaleMode == Mode )
        return;

    // Store updated mode
    mScaleMode = Mode;
}

//-----------------------------------------------------------------------------
//  Name : getScaleMode ()
/// <summary>
/// Return the currently set mode that the image box is using to scale
/// images.
/// </summary>
//-----------------------------------------------------------------------------
cgImageScaleMode::Base cgImageBoxControl::getScaleMode( ) const
{
    return mScaleMode;
}