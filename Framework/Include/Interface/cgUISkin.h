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
// Name : cgUISkin.h                                                         //
//                                                                           //
// Desc : Contains classes resposible for the loading, processing and        //
//        management of the definition data for interface skins. A skin      //
//        is a collection of image and layout data which allows users to     //
//        customize the look and feel of the interfaces rendered within this //
//        system.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGUISKIN_H_ )
#define _CGE_CGUISKIN_H_

//-----------------------------------------------------------------------------
// cgUISkin Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Interface/cgUITypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgUIControlLayer;
class cgBillboardBuffer;
class cgXMLNode;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgUISkin (Class)
/// <summary>
/// Contains all information about an individual skin which can be
/// applied to various elements and controls on the user interface.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUISkin
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    
    // Settings for the textbox control type
    struct TextBoxConfig
    {
        cgColorValue    caretColor;         // The color used to render the caret
        cgUInt32        caretBlinkSpeed;    // The speed at which the caret blinks
        cgColorValue    selectionColor;     // The color used to render the selection rectangle.

        // Reset method to allow for post-load validation
        void reset()
        {
            // Clear vars
            caretColor      = cgColorValue( 0, 0, 0, 1.0f );
            caretBlinkSpeed = 500; // 500ms
            selectionColor  = cgColorValue( 0.019f, 0.566f, 0.945f, 1.0f );
        
        } // End Method reset

    }; // End Struct TextBoxConfig

    // Settings for the listbox control type
    struct ListBoxConfig
    {
        cgColorValue    selectionColor;     // The color used to render the selection rectangle.

        // Reset method to allow for post-load validation
        void reset()
        {
            // Clear vars
            selectionColor  = cgColorValue( 0.019f, 0.566f, 0.945f, 1.0f );
        
        } // End Method reset

    }; // End Struct ListBoxConfig

    // Contains the various configuration settings for control types
    struct ControlConfig
    {
        TextBoxConfig   textBox;    // Configuration options for text box control.
        ListBoxConfig   listBox;    // Configuration options for list box control.
        
        // Reset method to allow for post-load validation
        void reset()
        {
            // Clear vars
            textBox.reset();
            listBox.reset();
        
        } // End Method reset

    }; // End Struct ControlConfig

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgUISkin( );
    virtual ~cgUISkin( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        loadDefinition          ( cgInputStream definition );
    const cgString            & getName                 ( ) const { return mSkinName; }
    const cgString            & getTextureName          ( ) const { return mTextureFile; }
    const cgString            & getGlyphDefinition      ( ) const { return mGlyphAtlas; }
    bool                        prepareControlFrames    ( cgUIControlLayer * layer, cgUIFormStyle::Base style );
    bool                        prepareCursorFrames     ( cgBillboardBuffer * buffer );
    const cgUICursorDesc      & getCursorDefinition     ( ) const { return mCursorDefinition; }
    const cgUIFormStyleDesc   & getFormStyleDefinition  ( cgUIFormStyle::Base type ) const;
    const ControlConfig       & getControlConfig        ( ) const { return mControlConfig; }

private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                    parseSkinDefinition     ( const cgXMLNode & node, const cgString & definitionFile );
    void                    parseCursor             ( const cgXMLNode & node, const cgString & baseDirectory, cgUICursorDesc & desc );
    void                    parseForm               ( const cgXMLNode & node );
    void                    parseControls           ( const cgXMLNode & node, cgUISkinElement::Map & elements, ControlConfig & config );
    bool                    parseElement            ( const cgXMLNode & node, cgUISkinElement & elementDesc );
    void                    parseFormStyle          ( const cgXMLNode & node, cgUIFormStyleDesc & styleDesc );
    bool                    parseElementArea        ( const cgXMLNode & node, cgUIElementArea & area );
    void                    parseRegion             ( const cgXMLNode & node, cgPointArray & region );
    void                    addElementFrame         ( cgUIControlLayer * layer, const cgUISkinElement::Map & elements, const cgString & elementName );
    void                    processRegions          ( );
    cgUISkinElement       * getFormElement          ( cgUIFormStyle::Base style, const cgString & elementName );
    cgUISkinElement       * getControlElement       ( const cgString & elementName );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgString                mSkinName;              // The name of this skin
    cgString                mTextureFile;           // Filename of the texture which contains the skin image data.
    cgString                mGlyphAtlas;            // The atlas definition for any associated glyphs.
    cgUISkinElement::Map    mControlElements;       // Element definitions for common controls (i.e. buttons etc).
    cgUIFormStyleDesc       mOverlappedFormStyle;   // Definition for overlapped form style (with caption etc.)
    cgUICursorDesc          mCursorDefinition;      // The definition parameters for the mouse cursor associated with this skin.
    cgStringArray           mValidFormElements;     // Contains a list of all valid form element types supported by the skinning system.
    cgStringArray           mValidControlElements;  // Contains a list of all valid control element types supported by the skinning system.
    ControlConfig           mControlConfig;         // List of configuration options for various defined control types.
};

#endif // !_CGE_CGUISKIN_H_