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
// Name : DestructibleLight.gs                                               //
//                                                                           //
// Desc : Behavior associated with destructible light sources.               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : DestructibleLight (Class)
// Desc : Behavior associated with destructible light sources.
//-----------------------------------------------------------------------------
shared class DestructibleLight : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private MeshNode@       mMesh;      // The node to which we are attached.
    private LightNode@      mLight;     // Child light source to control.
    private bool            mActive;
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : DestructibleLight () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	DestructibleLight( )
    {
    }

    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedObjectBehavior)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : onAttach () (Event)
	// Desc : Called by the application when the behavior is first attached to
	//        an object and can initialize.
	//-------------------------------------------------------------------------
	void onAttach( ObjectNode @ object )
	{
        // Initialize variables
        @mMesh  = cast<MeshNode>(object);
        @mLight = cast<LightNode>(mMesh.findChildOfType( RTID_LightObject ));
        mActive = true;
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release our references.
        @mMesh  = null;
        @mLight = null;
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{

	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : deactivate ()
	// Desc : Deactivate this light source.
	//-------------------------------------------------------------------------
    void deactivate( )
    {
        // No-op?
        if ( !mActive )
            return;

        // Turn off the light.
        Scene @ scene = mMesh.getScene();
        if ( @mLight != null )
        {
            //mLight.showNode( false, false );
            mLight.setDiffuseColor( mLight.getDiffuseColor() * 0.4f );
            mLight.setSpecularColor( ColorValue( 0, 0, 0, 0 ) );
            mLight.setShadowStage( SceneProcessStage::None );
        
        } // End if 

        // Replace any emissive material with its non emissive variant.
        // First, get the identifiers of the materials we want to swap.
        uint litMaterialId   = mMesh.getCustomProperties().getProperty( "material_illuminated", 0 );
        uint unlitMaterialId = mMesh.getCustomProperties().getProperty( "material_deluminated", 0 );

        // Get the lit material (should already be resident).
        MaterialHandle litMaterial, unlitMaterial;
        ResourceManager @ resources = scene.getResourceManager();
        if ( resources.getMaterialFromId( litMaterial, litMaterialId ) )
        {
            // Get/load the unlit material.
            if ( resources.loadMaterial( unlitMaterial, scene.getParentWorld(), MaterialType::Standard, unlitMaterialId, false, 0, DebugSource() ) )
            {
                // Swap!
                mMesh.getMesh().getResource(true).replaceMaterial( litMaterial, unlitMaterial );
            
            } // End if found unlit

        } // End if found lit

        //  Light is no longer active.
        mActive = false;
    }

} // End Class DestructibleLight