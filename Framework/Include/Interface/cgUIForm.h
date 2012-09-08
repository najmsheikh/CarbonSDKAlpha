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
// Name : cgUIForm.h                                                         //
//                                                                           //
// Desc : This module houses a special type of 'Interface Control' known as  //
//        a form. This is like a window which provides the specialized       //
//        functionality required for this type of interface item.            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGUIFORM_H_ )
#define _CGE_CGUIFORM_H_

//-----------------------------------------------------------------------------
// cgUIForm Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Interface/cgUIControl.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgUIControlLayer;
class cgUIManager;
class cgLabelControl;
class cgButtonControl;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {3489C9BC-786A-4576-952B-BCB7DFE50C78}
const cgUID RTID_UIForm = {0x3489C9BC, 0x786A, 0x4576, {0x95, 0x2B, 0xBC, 0xB7, 0xDF, 0xE5, 0xC, 0x78}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgUIForm (Class)
/// <summary>
/// The interface form class is like a window in the Win32 API. It
/// contains all of the logic necessary for managing and rendering
/// the window and all controls on that form.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgUIForm : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgUIForm, cgUIControl, "Form" )

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumeratoes
    //-------------------------------------------------------------------------
    typedef cgUIForm* (*FormAllocFunc)( cgUIControlLayer * );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgUIForm( cgUIControlLayer * layer );
    virtual ~cgUIForm( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static void                 registerType            ( const cgString & typeName, FormAllocFunc func );
    static cgUIForm           * createInstance          ( const cgString & typeName, cgUIControlLayer * layer );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        loadForm                ( cgInputStream formScriptStream, const cgString & name );
    bool                        createForm              ( const cgString & name );
    cgUIManager               * getUIManager            ( ) const { return mUIManager; }
    cgUIControlLayer          * getControlLayer         ( ) const { return mUILayer; }
    cgScript                  * getFormScript           ( ) const { return mFormScriptRes; }
    cgScriptObject            * getFormScriptObject     ( ) const { return mFormScriptObject; }
    void                        setAcceptButton         ( cgButtonControl * button );
    cgButtonControl           * getAcceptButton         ( ) const;
    void                        setCancelButton         ( cgButtonControl * button );
    cgButtonControl           * getCancelButton         ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                onPreCreateForm         ( cgUIFormProperties & properties );
    virtual bool                onCreateForm            ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void                setControlText          ( const cgString & text );
    virtual bool                onMouseMove             ( const cgPoint & position, const cgPointF & offset );
    virtual bool                onMouseButtonDown       ( cgInt32 buttons, const cgPoint & position );
    virtual bool                onMouseButtonUp         ( cgInt32 buttons, const cgPoint & position );
    virtual bool                onKeyPressed            ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual void                onSize                  ( cgInt32 width, cgInt32 height );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_UIForm; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    cgRect                      elementAreaToClientRect ( const cgUIElementArea & area );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUIManager       * mUIManager;         // The interface manager responsible for this form.
    cgUIControlLayer  * mUILayer;           // The interface layer in which this control exists
    cgLabelControl    * mCaption;           // The label control that will render the caption text.
    cgButtonControl   * mCloseButton;       // Form close button that appears in the form caption bar.
    cgButtonControl   * mMinimizeButton;    // Form minimize button that appears in the form caption bar.
    cgButtonControl   * mMaximizeButton;    // Form maximize button that appears in the form caption bar.
    cgScriptHandle      mFormScript;        // Resource handle to the form's execution script.
    cgScript          * mFormScriptRes;     // A cached copy of the actual underlying interface script pointer.
    cgScriptObject    * mFormScriptObject;  // An instance of the form script class being executed.
    cgPoint             mOldCursorPos;      // The previous position of the cursor during a move operation
    cgInt32             mCapturedHandle;    // If we're resizing, this contains the handle that was captured.
    cgUIFormProperties  mProperties;        // The properties for the form, set up by the script.
    cgButtonControl   * mAcceptButton;      // Default 'OK' / 'Accept' button (triggered on enter).
    cgButtonControl   * mCancelButton;      // Default 'Cancel' button (triggered on escape).

private:
    //-------------------------------------------------------------------------
    // Private Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgString, FormAllocFunc, FormAllocTypeMap)

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static FormAllocTypeMap mRegisteredForms;  // All of the form types registered with the system
};

#endif // !_CGE_CGUIFORM_H_