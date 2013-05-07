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
// Name : cgUITypes.h                                                        //
//                                                                           //
// Desc : Common header file that defines various UI types and common enum   //
//        results, flags, etc.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_UITYPES_H_ )
#define _CGE_UITYPES_H_

//-----------------------------------------------------------------------------
// cgUITypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgCursor;

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
// Flags which can be specified when drawing text
namespace cgTextFlags
{
    enum Base
    {
        ClipRectangle           = 0x1,          // Instructs the DrawText function to clip the text to the specified rectangle
        Multiline               = 0x2,          // Allow text to span multiple lines, will perform word wrapping (warn: performance implications).
        AlignCenter             = 0x10,         // Horizontally align the text to the center of the specified rectangle
        AlignRight              = 0x20,         // Horizontally align the text to the right of the specified rectangle
        VAlignCenter            = 0x40,         // Vertically align the text to the center of the specified rectangle
        VAlignBottom            = 0x80,         // Vertically align the text to the bottom of the specified rectangle
        AllowFormatCode         = 0x100         // Formatting tags are allowed (i.e. [c=#ffff0000]red text[/c])
    };

}; // End Namespace : cgTextFlags

// Select the space in which coordinates should be expressed
namespace cgControlCoordinateSpace
{
    enum Base
    {
        ClientRelative = 0,
        ControlRelative,
        ScreenRelative
    };

}; // End Namespace : cgControlCoordinateSpace

// Select the mode in which an image should be automatically drawn (see cgUIManager::DrawImage)
namespace cgImageScaleMode
{
    enum Base
    {
        None,           // No offset or resizing will occur.
        Center,         // Center the image in the viewport.
        Stretch,        // Stretch the image to fill the viewport.
        Fit,            // Maintain aspect ratio such that the entire image fits on the screen.
        FitU,           // Maintain aspect ratio such that the width of the image fills the screen.
        FitV            // Maintain aspect ratio such that the height of the image fills the screen.
    };

}; // End Namespace : cgImageScaleMode

// The type of this given layer describes how it is managed.
namespace cgUILayerType
{
    enum Base
    {
        UserLayer       = 0,    // Standard user layer to which forms / widgets can be attached
        SystemLayer     = 1     // A system layer cannot be arranged, and exists always at the specified depth
    };

}; // End Namespace : cgUILayerType

// Constants for specifying the individual handle regions that may be available for an element
namespace cgUIHandleType
{
    enum Base
    {
        N         = 0,
        NE        = 1,
        E         = 2,
        SE        = 3,
        S         = 4,
        SW        = 5,
        W         = 6,
        NW        = 7,
        Invalid   = 8
    };

}; // End Namespace : cgUIHandleType

// Describes the various different styles of form that can be used.
namespace cgUIFormStyle
{
    enum Base
    {
        Overlapped      = 0,       // Standard window type with caption bar.
    };

}; // End Namespace : cgUIFormStyle

namespace cgHorizontalAlignment
{
    enum Base
    {
        Left    = 0,
        Center  = 1,
        Right   = 2,
    
    };

}; // End Namespace : cgHorizontalAlignment

namespace cgVerticalAlignment
{
    enum Base
    {
        Top     = 0,
        Middle  = 1,
        Bottom  = 2,
    
    };

}; // End Namespace : cgVerticalAlignment

namespace cgTextTruncationMode
{
    enum Base
    {
        None     = 0,
        Ellipsis = 1,
    };

}; // End Namespace : cgTextTruncationMode

namespace cgDockMode
{
    enum Base
    {
        None    = 0,
        Left    = 1,
        Right   = 2,
        Top     = 3,
        Bottom  = 4,
        Fill    = 5
    };

}; // End Namespace : cgDockMode

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
// Describes an individual skin element within the skin texture.
struct cgUISkinElement
{
    // Typedefs
    CGE_MAP_DECLARE(cgString, cgUISkinElement, Map) // ToDo: unordered_map?
    
    // Variables
    cgString        name;               // Name of this element.
    cgRect          bounds;             // The rectangle within the skin texture for this element.
    cgPointArray    activeRegion;       // The active region allows you to mask out the pieces of the element you're interested in.
    cgPointArray    handleRegions[8];   // Handle regions can be provided to describe areas of the element used for sizing.

    // Reset method to allow for post-load validation
    void reset()
    {
        // Clear vars
        name.clear();
        bounds = cgRect(0,0,0,0);
        activeRegion.clear();
        for ( size_t i = 0; i < 8; ++i )
            handleRegions[i].clear();
    
    } // End Method reset

}; // End Struct : cgUISkinElement

// Contains extended rectangle information that allows each component to be aligned to a specific edge.
struct cgUIElementArea
{
    cgRect  bounds;
    cgInt32 align[4];

    // Reset method to allow for post-load validation
    void reset()
    {
        // Clear vars
        bounds = cgRect( 0, 0, 0, 0 );
        memset( align, 0, 4 * sizeof(cgInt32) );
    
    } // End Method reset

}; // End Struct cgUIElementArea

// The description for an individual cursor type (i.e. "Arrow", "SizeNS" etc.)
struct cgUICursorType
{
    // Typedefs
    CGE_MAP_DECLARE(cgString, cgUICursorType, Map) // ToDo: unordered_map?
    CGE_VECTOR_DECLARE( cgCursor*, PlatformCursorArray )

    // Variables
    cgString                name;       // Name associated with this cursor type.
    cgPoint                 hotPoint;   // Offset in the billboard about which the cursor will be centered.
    cgRectArray             frames;     // List of animation frame rectangles for this cursor type
    bool                    animated;   // Is this cursor type animated?
    bool                    loop;       // Should the animation loop?
    cgFloat                 duration;   // How long should the animation last.
    PlatformCursorArray     platformCursors;

    // Reset method to allow for post-load validation
    void reset()
    {
        // Clear vars
        hotPoint.x = 0;
        hotPoint.y = 0;
        animated   = false;
        loop       = false;
        duration   = 0.0f;
        frames.clear();

    } // End Method Reset

}; // End Struct : cgUICursorType

// Contains the definition dictating how the cursor is to be displayed.
struct cgUICursorDesc
{
    // Variables
    cgString            texture;    // The texture to use for the skin's cursor
    cgUICursorType::Map types;      // Map containing all supported cursor types keyed by name

    // Reset method to allow for post-load validation
    void Reset()
    {
        // Clear vars
        texture = _T("");
        types.clear();
    
    } // End Method reset

}; // End Struct : cgUICursorDesc

// Allows the caller to configure the properties of the form as it is created.
struct cgUIFormProperties
{
    cgUIFormStyle::Base style;
    bool                sizable;
    bool                movable;
    bool                canMinimize;
    bool                canMaximize;
    bool                canClose;

}; // End Struct FormProperties

// Contains the skin definition for an individual style of a form (i.e. Overlapped)
struct CGE_API cgUIFormStyleDesc
{
    // Variables
    cgSize                  minimumSize;
    cgSize                  maximumSize;
    cgUIElementArea         icon;
    cgUIElementArea         caption;
    cgUIElementArea         buttonMinimize;
    cgUIElementArea         buttonMaximize;
    cgUIElementArea         buttonRestore;
    cgUIElementArea         buttonClose;
    cgUISkinElement::Map    elements;
    bool                    sizable;
    bool                    hasIconArea;
    bool                    hasCaptionArea;
    bool                    hasSizeControls;
    bool                    styleAvailable;

    // Reset method to allow for post-load validation
    void reset()
    {
        // Clear vars
        minimumSize.width   = -1;
        minimumSize.height  = -1;
        maximumSize.width   = -1;
        maximumSize.height  = -1;
        styleAvailable      = false;
        sizable             = false;
        hasIconArea         = false;
        hasCaptionArea      = false;
        hasSizeControls     = false;

        // Reset child structures
        icon.reset();
        caption.reset();
        buttonMinimize.reset();
        buttonMaximize.reset();
        buttonRestore.reset();
        buttonClose.reset();
        elements.clear();
    
    } // End Method reset

    // Constructor
    cgUIFormStyleDesc() { reset(); }

}; // End Struct cgUIFormStyleDesc

#endif // !_CGE_UITYPES_H_