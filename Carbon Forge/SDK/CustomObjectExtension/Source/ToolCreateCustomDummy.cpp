//---------------------------------------------------------------------------//
//         ____           _                   _____                          //
//        / ___|__ _ _ __| |__   ___  _ __   |  ___|__  _ __ __ _  ___       //
//       | |   / _` | '__| '_ \ / _ \| '_ \  | |_ / _ \| '__/ _` |/ _ \      //
//       | |__| (_| | |  | |_) | (_) | | | | |  _| (_) | | | (_| |  __/      //
//        \____\__,_|_|  |_.__/ \___/|_| |_| |_|  \___/|_|  \__, |\___|      //
//                   Game Institute - Carbon Engine Sandbox |___/            //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// File: ToolCreateCustomDummy.cpp                                           //
//                                                                           //
// Desc: Classes responsible for creating a simple dummy frame and adding it //
//       to the scene based on user interaction with the render viewports.   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// ToolCreateCustomDummy Module Includes
//-----------------------------------------------------------------------------
#include "ToolCreateCustomDummy.h"
#include "CustomDummy.h"

// Carbon Forge Includes
#include <cfRenderViewport.h>

// CGE Includes
#include <World/cgScene.h>

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace CarbonForge;

///////////////////////////////////////////////////////////////////////////////
// ToolCreateCustomDummy Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : ToolCreateCustomDummy() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
ToolCreateCustomDummy::ToolCreateCustomDummy(  )
{
    // Initialize variables to sensible defaults
    m_bCreating     = false;
    m_nCurrentStep  = 0;
    m_pDummy        = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : ~ToolCreateCustomDummy() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
ToolCreateCustomDummy::~ToolCreateCustomDummy( )
{
}

//-----------------------------------------------------------------------------
//  Name : Allocate() (Static)
/// <summary>
/// Allocate a tool mode of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cfBaseToolMode * ToolCreateCustomDummy::Allocate( )
{
    return new ToolCreateCustomDummy( );
}

//-----------------------------------------------------------------------------
// Name : OnMouseDown()
/// <summary>
/// Called in order to notify the selected tool that a mouse down event was
/// triggered in the specified viewport. Note: tool should return true if the
/// event was handled and should override the viewport behavior.
/// </summary>
//-----------------------------------------------------------------------------
bool ToolCreateCustomDummy::OnMouseDown( cgUInt8 nButtons, const cgPoint & Position, cgUInt32 nModifiers, cfRenderViewport * pViewport )
{
    // Left button to create / progress. Right button cancels.
    if ( nButtons & cgMouseButtons::Left )
    {
        // Creating a new object, or continuing with previous?
        if ( !m_bCreating )
        {
            // First, compute the world space "origin" for the dummy based on
            // the position of the cursor when the click occured.
            if ( !pViewport->ViewportToWorld( Position, m_vOrigin ) )
                return false;

            // Begin the dummy creation process. First allocate an empty dummy object node
            // and add it to the document so that it can be rendered during creation.
            cgScene * pScene = pViewport->GetParentScene();
            if ( !(m_pDummy = (CustomDummyNode*)pScene->createObjectNode( false, RTID_CustomDummyObject, true ) ) )
                return false;
            
            // Setup for the following creation steps
            m_bCreating     = true;
            m_bDragged      = false;
            m_nCurrentStep  = 0;
            m_pDummy->setSize( 0.0f );

            // Viewport "capture" status should be locked to ensure we don't process
            // other viewports until the creation process is complete.
            pViewport->SetCaptureLocked( true );

            // Object should be the only item now selected
            pScene->clearSelection();
            m_pDummy->setSelected( true );

        } // End if creating new mesh
        else
        {
            // We are continuing an existing object.
            switch ( m_nCurrentStep )
            {
                case 0: // After initial click (Size)
                    
                    // Creation process is complete
                    m_bCreating = false;
                    m_pDummy    = CG_NULL;

                    // Viewport "capture" status can be released.
                    pViewport->SetCaptureLocked( false );

                    // Update selection property dialog to reflect final values.
                    pViewport->ShowSceneSelectionProperties();
                    break;

            } // End switch Step

        } // End if continuing existing

        // We handled the click
        return true;

    } // End if left button
    
    // We did not handle
    return false;
}

//-----------------------------------------------------------------------------
// Name : OnMouseMove()
/// <summary>
/// Called in order to notify the selected tool that a mouse move event was
/// triggered in the specified viewport. Note: tool should return true if the
/// event was handled and should override the viewport behavior.
/// </summary>
//-----------------------------------------------------------------------------
bool ToolCreateCustomDummy::OnMouseMove( cgUInt8 nButtons, const cgPoint & Position, const cgPoint & PrevCursorPos, cgUInt32 nModifiers, cfRenderViewport * pViewport, bool & bWrapCursor )
{
    // Started creating an object?
    if ( m_bCreating )
    {
        switch ( m_nCurrentStep )
        {
            case 0: // After initial click (Size)

                // Cursor was moved
                m_bDragged = true;
                
                // Retrieve world space grid position under the cursor
                if ( !pViewport->ViewportToWorld( Position, m_vBaseEnd ) )
                    return false;

                // Compute the size of the dummy based on these values
                m_pDummy->setSize( cgVector3::length( (m_vBaseEnd - m_vOrigin)) * 2.0f );

                // Generate the new object matrix for the node.
                m_pDummy->setWorldTransform( pViewport->GetToViewMatrix() );
                m_pDummy->setPosition( m_vOrigin );

                // Notify whoever is listening that the selection was modified.
                cgScene * pScene = pViewport->GetParentScene();
                pScene->onModifySelection( &cgSceneEventArgs( pScene ) );
                pViewport->RefreshSceneSelectionProperties( );
                break;

        } // End switch Step

        // We handled the mouse event
        return true;

    } // End if creating mesh

    // We did not handle
    return false;
}

//-----------------------------------------------------------------------------
// Name : OnMouseUp()
/// <summary>
/// Called in order to notify the selected tool that a mouse up event was
/// triggered in the specified viewport. Note: tool should return true if the
/// event was handled and should override the viewport behavior.
/// </summary>
//-----------------------------------------------------------------------------
bool ToolCreateCustomDummy::OnMouseUp( cgUInt8 nButtons, const cgPoint & Position, const cgPoint & PrevCursorPos, cgUInt32 nModifiers, cfRenderViewport * pViewport )
{
    // Started creating an object?
    if ( m_bCreating )
    {
        switch ( m_nCurrentStep )
        {
            case 0: // After initial click (Size)

                // If the user dragged the mouse (instead of click and release)
                // perform second click operation automatically on mouse up.
                if ( m_bDragged )
                {
                    // Creation process is complete
                    m_bCreating = false;
                    m_pDummy    = CG_NULL;

                    // Viewport "capture" status can be released.
                    pViewport->SetCaptureLocked( false );

                    // Update selection property dialog to reflect final values.
                    pViewport->ShowSceneSelectionProperties( );
                    
                } // End if cursor dragged
                break;

        } // End switch Step

        // We handled the mouse event
        return true;

    } // End if creating mesh

    // We did not handle
    return false;
}
