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
// File: SimpleScatter.cpp                                                   //
//                                                                           //
// Desc: Simple example utility plugin. Displays a form, and adds some       //
//       objects to the currently active scene (if any).                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2014 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// SimpleScatter Module Includes
//-----------------------------------------------------------------------------
#include "Utility Plugins/SimpleScatter.h"
#include "frmSimpleScatter.h"

// CGE Includes
#include <World/cgScene.h>
#include <World/Objects/cgMeshObject.h>
#include <World/Objects/cgPointLight.h>
#include <Math/cgRandom.h>
#include <System/cgThreading.h>

// CarbonForge Includes
#include <cfCommon.h>
#include <cfProgressHandler.h>
#include <cfWorldDoc.h>

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace System;
using namespace System::Windows::Forms;
using namespace CarbonForge;
using namespace CarbonForge::CustomUtilities;

///////////////////////////////////////////////////////////////////////////////
// SimpleScatter Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : SimpleScatter() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
SimpleScatter::SimpleScatter( cfWorldDoc * document ) : cfBaseUtility( document )
{
    // Initialize variables to sensible defaults
    m_pScene    = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : ~SimpleScatter() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
SimpleScatter::~SimpleScatter( )
{
}

//-----------------------------------------------------------------------------
//  Name : Allocate() (Static)
/// <summary>
/// Allocate a utility of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cfBaseUtility * SimpleScatter::Allocate( const cgUID & type, cfWorldDoc * document )
{
    return new SimpleScatter( document );
}

//-----------------------------------------------------------------------------
// Name : Launch () (Virtual)
/// <summary>
/// Launch the utility and allow it to process as required. Note: Utility class
/// will be destroyed once this method returns so any persistent data should
/// be static.
/// </summary>
//-----------------------------------------------------------------------------
bool SimpleScatter::Launch( )
{
    // Get any active scene.
    cgScene * scene = (m_pDocument) ? m_pDocument->GetActiveScene() : CG_NULL;
    if ( !scene )
    {
        MessageBox::Show( L"There is no active scene.\r\n\r\nPlease open and/or select a valid scene and try again", L"No Active Scene", MessageBoxButtons::OK, MessageBoxIcon::Warning );
        return false;
    
    } // End if no landscape

    // Display the scatter options.
    frmSimpleScatter ^ form = gcnew frmSimpleScatter( this );
    DialogResult result = form->ShowDialog( Form::ActiveForm );

    // Did the user indicate that it was OK to start the process?
    if ( result == DialogResult::OK )
    {
        // Store the specified options.
        m_pScene        = scene;
        m_nObjectType   = form->ObjectType;
        m_nObjectCount  = form->ObjectCount;

        // Begin processing.
        return StartThreadedProcess();
    
    } // End if OK
    else
        return true;

    // Some other failure?
    return false;
}

//-----------------------------------------------------------------------------
// Name : UtilityThread() (Protected, Virtual)
/// <summary>
/// Thread method (non-static) responsible for processing utility functionality 
/// without locking the UI. This is the method that derived classes should
/// implement in order to add their processing behavior.
/// </summary>
//-----------------------------------------------------------------------------
bool SimpleScatter::UtilityThread( cgThread * thread, cfProgressHandler * progress )
{
    try
    {
        cgRandom::ParkMiller random(true);

        // Update progress.
        progress->SetTotalProgress( 0, 1 );
        progress->SetOperationProgress( L"Scattering Objects", 0, 1 );
        progress->SetTaskProgress( L"Creating object...", 0, m_nObjectCount );

        // What object type are we creating?
        cgUID objectType = RTID_MeshObject;
        if ( m_nObjectType == 2 )
            objectType = RTID_PointLightObject;

        // Create objects.
        for ( int i = 0; i < m_nObjectCount; ++i )
        {
            // Create new object
            cgObjectNode * node = m_pScene->createObjectNode( false, objectType, true );
            if ( node )
            {
                // Object specific setup.
                switch ( m_nObjectType )
                {
                    case 0: // Cube
                    {
                        cgMeshNode * mesh = static_cast<cgMeshNode*>(node);
                        mesh->createBox( 10.0f, 10.0f, 10.0f, 1, 1, 1, false, cgMeshCreateOrigin::Center );

                    } // End cube
                    break;
                
                    case 1: // Sphere
                    {
                        cgMeshNode * mesh = static_cast<cgMeshNode*>(node);
                        mesh->createSphere( 5.0f, 10, 20, false, cgMeshCreateOrigin::Center );
                
                    } // End sphere
                    break;

                    case 2: // Point light
                    {
                        cgPointLightNode * light = static_cast<cgPointLightNode*>(node);
                        light->setOuterRange( 10.0f );
                        light->setDiffuseColor( cgColorValue( (cgFloat)random.next(0,1), (cgFloat)random.next(0,1), (cgFloat)random.next(0,1), 1 ) );
                        light->setSpecularColor( 0xFF444444 );
                        light->setDiffuseHDRScale( 3.0f );
                        light->setSpecularHDRScale( 2.0f );

                    } // End light
                    break;

                } // End switch type

                // Compute a random position.
                cgVector3 pos( (cgFloat)random.next(-100, 100), 0, (cgFloat)random.next(-100, 100) );
                node->setPosition( pos );

            } // End if valid

            // Update progress
            progress->SetTaskProgress( i+1, m_nObjectCount );

            // Wait / terminate?
            thread->suspendedWait();
            if ( thread->terminateRequested() )
                return true;
        
        } // Next object

        // Update progress
        progress->SetTotalProgress( 1, 1 );
        progress->SetOperationProgress( L"Done!", 1, 1 );
        progress->SetTaskProgress( L"Done!", 1.0f );

    } // End Try Block

    catch ( System::Exception ^ e )
    {
        cgAppLog::write( cgAppLog::Error, cfAPI::ToNative( e->ToString() + L"\n" ).c_str() );
        return false;
    
    } // End catch block
    catch ( ... )
    {
        cgAppLog::write( cgAppLog::Error, _T("An unknown error occurred during terrain export.\n") );
        return false;
    
    } // End catch block*/

    // Success!
    return true;
}