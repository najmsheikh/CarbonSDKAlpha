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
// Name : cgUIManager.h                                                      //
//                                                                           //
// Desc : This module houses classes responsible for the storage and         //
//        management of application interface objects.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGUIMANAGER_H_ )
#define _CGE_CGUIMANAGER_H_

//-----------------------------------------------------------------------------
// cgUIManager Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Interface/cgUITypes.h>
#include <System/cgReference.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgTextEngine;
class cgRenderDriver;
class cgUISkin;
class cgUILayer;
class cgUIControl;
class cgUIForm;
class cgUICursorLayer;
class cgBillboardBuffer;
class cgResourceManager;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {ADD4ACB9-38EC-401D-84D8-A6E98B50530B}
const cgUID RTID_UIManager = {0xADD4ACB9, 0x38EC, 0x401D, {0x84, 0xD8, 0xA6, 0xE9, 0x8B, 0x50, 0x53, 0xB}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgUIManager (Class)
/// <summary>
/// Class for the management of interface elements.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUIManager : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgUIManager, cgReference, "UIManager" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgUIForm;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgUIManager( );
    virtual ~cgUIManager( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgUIManager    * getInstance          ( );
    static void             createSingleton      ( );
    static void             destroySingleton     ( );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    initialize          ( cgResourceManager * resourceManager );
    bool                    begin               ( );
    cgResourceManager     * getResourceManager  ( ) { return mResourceManager; }
    cgRenderDriver        * getRenderDriver     ( );
    cgTextEngine          * getTextEngine       ( ) { return mTextEngine; }

    // Interface Skins
    bool                    addSkin             ( cgInputStream definition );
    cgUISkin              * getSkinDefinition   ( const cgString & skinName );
    bool                    selectSkin          ( const cgString & skinName );
    cgUISkin              * getCurrentSkin      ( ) { return mCurrentSkin; }
    cgString                getSkinGlyphLibrary ( ) const;

    // Text Handling
    cgString                addFont             ( cgInputStream definition );
    bool                    selectFont          ( const cgString & fontName );
    bool                    selectDefaultFont   ( );
    bool                    setDefaultFont      ( const cgString & fontName );
    const cgString        & getDefaultFont      ( ) const { return mDefaultFont; }
    
    cgRect                  printText           ( cgInt32 x, cgInt32 y, const cgString & text );
    cgRect                  printText           ( cgInt32 x, cgInt32 y, const cgString & text, cgUInt32 flags );
    cgRect                  printText           ( cgInt32 x, cgInt32 y, const cgString & text, cgUInt32 flags, cgUInt32 color );
    cgRect                  printText           ( cgInt32 x, cgInt32 y, const cgString & text, cgUInt32 flags, cgUInt32 color, cgInt32 kerning );
    cgRect                  printText           ( cgInt32 x, cgInt32 y, const cgString & text, cgUInt32 flags, cgUInt32 color, cgInt32 kerning, cgInt32 lineSpacing );

    cgRect                  printText           ( const cgPoint & position, const cgString & text );
    cgRect                  printText           ( const cgPoint & position, const cgString & text, cgUInt32 flags );
    cgRect                  printText           ( const cgPoint & position, const cgString & text, cgUInt32 flags, cgUInt32 color );
    cgRect                  printText           ( const cgPoint & position, const cgString & text, cgUInt32 flags, cgUInt32 color, cgInt32 kerning );
    cgRect                  printText           ( const cgPoint & position, const cgString & text, cgUInt32 flags, cgUInt32 color, cgInt32 kerning, cgInt32 lineSpacing );

    cgRect                  printText           ( const cgRect & bounds, const cgString & text );
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, cgUInt32 flags );
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, cgUInt32 flags, cgUInt32 color );
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, cgUInt32 flags, cgUInt32 color, cgInt32 kerning );
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, cgUInt32 flags, cgUInt32 color, cgInt32 kerning, cgInt32 lineSpacing );
    
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, const cgPoint & offset );
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, const cgPoint & offset, cgUInt32 flags );
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, const cgPoint & offset, cgUInt32 flags, cgUInt32 color );
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, const cgPoint & offset, cgUInt32 flags, cgUInt32 color, cgInt32 kerning );
    cgRect                  printText           ( const cgRect & bounds, const cgString & text, const cgPoint & offset, cgUInt32 flags, cgUInt32 color, cgInt32 kerning, cgInt32 lineSpacing );

    // Layers
    void                    addLayer            ( cgUILayer * layer );
    void                    removeLayer         ( cgUILayer * layer, bool destroyLayer );
    void                    bringLayerToFront   ( cgUILayer * layer );
    void                    sendLayerToBack     ( cgUILayer * layer );

    // Forms and controls
    void                    setCapture          ( cgUIControl * control );
    cgUIControl           * getCapture          ( ) { return mCapturedControl; }
    void                    setFocus            ( cgUIControl * control );
    cgUIControl           * getFocus            ( ) { return mFocusControl; }
    cgUIForm              * createForm          ( const cgString & type, const cgString & name );
    cgUIForm              * loadForm            ( const cgInputStream & stream, const cgString & name );

    // Images / Glyphs / Icons
    bool                    addImage            ( cgInputStream imageFile, const cgString & referenceName );
    bool                    addImageLibrary     ( cgInputStream atlasFile, const cgString & referenceName );
    bool                    removeImageLibrary  ( const cgString & referenceName );
    bool                    isImageLibraryLoaded( const cgString & referenceName ) const;
    cgSize                  getImageSize        ( const cgString & referenceName ) const;
    cgSize                  getImageSize        ( const cgString & referenceName, const cgString & libraryItem ) const;
    
    void                    drawImage           ( const cgRect & bounds, const cgString & referenceName );
    void                    drawImage           ( const cgRect & bounds, const cgString & referenceName, const cgString & libraryItem );
    void                    drawImage           ( const cgRect & bounds, const cgString & referenceName, const cgString & libraryItem, const cgColorValue & color );
    void                    drawImage           ( const cgRect & bounds, const cgString & referenceName, const cgString & libraryItem, const cgColorValue & color, bool filter );

    void                    drawImage           ( const cgRectF & bounds, const cgString & referenceName );
    void                    drawImage           ( const cgRectF & bounds, const cgString & referenceName, const cgString & libraryItem );
    void                    drawImage           ( const cgRectF & bounds, const cgString & referenceName, const cgString & libraryItem, const cgColorValue & color );
    void                    drawImage           ( const cgRectF & bounds, const cgString & referenceName, const cgString & libraryItem, const cgColorValue & color, bool filter );

    void                    drawImage           ( const cgPoint & offset, const cgString & referenceName );
    void                    drawImage           ( const cgPoint & offset, const cgString & referenceName, const cgString & libraryItem );
    void                    drawImage           ( const cgPoint & offset, const cgString & referenceName, const cgString & libraryItem, const cgColorValue & color );

    void                    drawImage           ( cgImageScaleMode::Base mode, const cgString & referenceName );
    void                    drawImage           ( cgImageScaleMode::Base mode, const cgString & referenceName, const cgString & libraryItem );
    void                    drawImage           ( cgImageScaleMode::Base mode, const cgString & referenceName, const cgString & libraryItem, const cgColorValue & color );

    // Cursors
    void                    selectCursor        ( const cgString & name );

    // Rendering
    void                    render              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UIManager; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    virtual bool            processMessage      ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    CGE_LIST_DECLARE        (cgUIForm*, FormList)
    CGE_UNORDEREDMAP_DECLARE(cgString, cgUISkin*, SkinMap)
    CGE_LIST_DECLARE        (cgUILayer*, LayerList )
    CGE_UNORDEREDMAP_DECLARE(cgString, cgBillboardBuffer*, ImageLibraryMap)

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                    createSystemLayers  ( );
    void                    removeForm          ( cgUIForm * form );
    void                    checkGarbage        ( );
    void                    sendGarbageMessage  ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgResourceManager * mResourceManager;   // The resource manager responsible for all interface resources
    cgTextEngine      * mTextEngine;        // A text engine for this interface, allows us to draw text.
    cgString            mDefaultFont;       // The default font to use for the interface.
    FormList            mForms;             // List of all loaded forms
    LayerList           mLayers;            // List containing all of the interface render layers currently being managed.
    SkinMap             mSkins;             // Map containing all loaded skins keyed by name.
    ImageLibraryMap     mImageLibraries;    // Map containing billboard buffers which provide support for image drawing.
    cgUISkin          * mCurrentSkin;       // The currently selected skin
    bool                mInitialized;       // Has the user interface manager been initialized?
    cgUIControl       * mCapturedControl;   // The control that has currently been "captured" by the mouse.
    cgUIControl       * mFocusControl;      // The control that currently has "focus".
    cgUICursorLayer   * mCursorLayer;       // The system layer containing the cursor.

    // Garbage lists
    FormList            mGarbageForms;      // List of all forms that are due to be unloaded.

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgUIManager * mSingleton;        // Static singleton object instance.
};

#endif // !_CGE_CGUIMANAGER_H_