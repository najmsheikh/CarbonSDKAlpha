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
// Name : ArmorRechargeStation.gs                                            //
//                                                                           //
// Desc : Behavior for the armor recharge station.                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../States/GamePlay.gs"
#include_once "Objective.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : ArmorRechargeStation (Class)
// Desc : Behavior for the armor recharge station.
//-----------------------------------------------------------------------------
shared class ArmorRechargeStation : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private GamePlay@           mGamePlayState;         // Top level application state driving gameplay.
    private ObjectNode@         mNode;                  // The node to which we are attached.
    private float               mAvailableCharge;       // Amount of charge available in this station.
    private bool                mIsCharging;            // Are we in the process of charging the player?
    private Objective@          mAssociatedObjective;   // Is there an objective associated with this?
    private LightNode@          mAssociatedLight;
    private MaterialHandle      mAssociatedMaterial;
    private float               mPulseTimer;
    private ColorValue          mOriginalLightColor;
    private ColorValue          mOriginalMaterialColor;

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : PartObjective () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	ArmorRechargeStation( )
    {
        mAvailableCharge = 400;
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
        // Initialize variables.
        mIsCharging  = false;

        // Cache references to required objects.
        @mNode = object;

        // Get the currently active state (GamePlay)
        AppStateManager @ stateManager = getAppStateManager();
        AppState @ state = stateManager.getState( "GamePlay" );
        if ( @state != null )
            @mGamePlayState = cast<GamePlay>( state.getScriptObject() );

        // Get any associated objective.
        Scene @ scene = object.getScene();
        uint objectiveRef = uint(object.getCustomProperty( "charge_objective_refid", 0 ));
        ObjectNode@ node = scene.getObjectNodeById( objectiveRef );
        if ( @node != null )
            @mAssociatedObjective = cast<Objective>(node.getScriptedBehavior(0));

        // Get associated lights / materials
        ResourceManager @ resources = scene.getResourceManager();
        uint lightRef    = uint(object.getCustomProperty( "charge_light_refid", 0 ));
        uint materialRef = uint(object.getCustomProperty( "charge_material_refid", 0 ));
        @mAssociatedLight = cast<LightNode@>(scene.getObjectNodeById( lightRef ) );
        resources.getMaterialFromId( mAssociatedMaterial, materialRef );

        // Cache original light / material colors
        if ( @mAssociatedLight != null )
            mOriginalLightColor = mAssociatedLight.getDiffuseColor();
        StandardMaterial @ material = cast<StandardMaterial>(mAssociatedMaterial.getResource(true));
        if ( @material != null )
            mOriginalMaterialColor = material.getEmissive();
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release references.
        @mGamePlayState = null;
    }

    //-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        if ( @mGamePlayState == null )
            return;

        // Get the player.
        PlayerAgent @ player = mGamePlayState.getPlayer();
        ObjectNode @ playerNode = player.getSceneNode();

        // Is the player nearby, and alive?
        if ( player.isAlive() && vec3Length(playerNode.getPosition() - mNode.getPosition()) < 1.0f )
        {
            // If we weren't previously charging, switch to always updating
            // and skip this first update step to prevent overcharging on the
            // very first step.
            if ( !mIsCharging )
            {
                mNode.setUpdateRate( UpdateRate::Always );
                mIsCharging = true;
                
                // Trigger any associated objective first time in
                if ( @mAssociatedObjective != null )
                    player.setCurrentObjective( mAssociatedObjective.onActivate( playerNode ) );
                @mAssociatedObjective = null;
                return;
            
            } // End if start charging

            // Add armor to the player at a rate of 60 per second.
            float chargeAmount = 60 * elapsedTime;
            
            // Clamp to both available and needed armor.
            if ( chargeAmount > mAvailableCharge )
                chargeAmount = mAvailableCharge;
            float neededCharge = (player.getMaximumArmor() - player.getCurrentArmor());
            if ( chargeAmount > neededCharge )
                chargeAmount = neededCharge;
            
            // Apply charge if necessary
            if ( chargeAmount > 0 )
            {
                player.setArmorCharging( true, false );
                player.setCurrentArmor( player.getCurrentArmor() + chargeAmount );
                mAvailableCharge -= chargeAmount;
            
            } // End if has charge
            else
            {
                // Finished charging armor or out of charge.
                player.setArmorCharging( false, false );
            
            } // End if no charge

            // If there is no charge remaining in the station, de-activate 
            // the light source and the associated material emissive .
            if ( mAvailableCharge <= 0 && @mAssociatedLight != null && mAssociatedLight.isRenderable() )
            {
                // Light source
                mAssociatedLight.showNode( false, false );

                // Emissive
                StandardMaterial @ material = cast<StandardMaterial>(mAssociatedMaterial.getResource(true));
                if ( @material != null )
                {
                    material.setEmissive( ColorValue( 0.0f, 0.0f, 0.0f, mOriginalMaterialColor.a ) );
                    material.setDiffuse( ColorValue( 0.1f, 0.1f, 0.1f, 1.0f ) );
                
                } // End if has material.

            } // End if out of charge
        
        } // End if in range
        else
        {
            // Finish charging as necessary.
            if ( mIsCharging )
            {
                // Disable armor charging, then switch back to a 15 fps update.
                player.setArmorCharging( false, false );
                mNode.setUpdateRate( UpdateRate::FPS15 );

                // We're done charging
                mIsCharging = false;
            
            } // End if was charging

            // Pulse the light source if there is any charge left.
            mPulseTimer += elapsedTime;
            if ( @mAssociatedLight != null && mAvailableCharge > 0 )
            {
                float cycleScale = cos(mPulseTimer * 2.0f);
                ColorValue newColor = mOriginalLightColor;
                newColor.r += newColor.r * cycleScale * 0.5f;
                newColor.g += newColor.r * cycleScale * 0.5f;
                newColor.b += newColor.r * cycleScale * 0.5f;
                mAssociatedLight.setDiffuseColor( newColor );

                // And the associated material
                StandardMaterial @ material = cast<StandardMaterial>(mAssociatedMaterial.getResource(true));
                if ( @material != null )
                {
                    newColor = mOriginalMaterialColor;
                    newColor.r += newColor.r * cycleScale * 0.5f;
                    newColor.g += newColor.r * cycleScale * 0.5f;
                    newColor.b += newColor.r * cycleScale * 0.5f;
                    material.setEmissive( newColor );
                
                } // End if has material

            } // End if has light
        
        } // End if out of range
    }
}