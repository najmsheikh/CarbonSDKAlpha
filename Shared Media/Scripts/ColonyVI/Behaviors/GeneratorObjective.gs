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
// Name : GeneratorObjective.gs                                              //
//                                                                           //
// Desc : Objective that must be activated by the player to progress.        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "Objective.gsh"
#include_once "../API/Agent.gsh"

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------
const int TurretRefId = 0xDF5;

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : GeneratorObjective (Class)
// Desc : Objective that must be activated by the player to progress.
//-----------------------------------------------------------------------------
shared class GeneratorObjective : Objective
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private uint                    mCoverRefId;        // Reference ID of the cover that drops down when the coolant supply is shut down
    private ObjectNode@             mCover;             // Cached reference to the cover object node
    private uint                    mLightRefId;        // Reference ID of the light source associated with this generator.
    private LightNode@              mLight;             // Cached reference to the light.
    private uint                    mSteam1RefId;       // Reference ID of the steam particle emitter that is enabled coolant supply is shut down
    private ParticleEmitterNode@    mSteam1;            // Cached reference to the emitter.
    private uint                    mSteam2RefId;       // Reference ID of the steam particle emitter that is enabled coolant supply is shut down
    private ParticleEmitterNode@    mSteam2;            // Cached reference to the emitter.
    private float                   mSlideAmount;       // Amount that the cover has been moved so far.
    private ColorValue              mOriginalDiffuse;
    private ColorValue              mOriginalSpecular;

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : GeneratorObjective () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	GeneratorObjective( )
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
        // Get properties
        mCoverRefId  = uint(object.getCustomProperty( "cover_refid", 0 ) );
        mSteam1RefId = uint(object.getCustomProperty( "emitter1_refid", 0 ) );
        mSteam2RefId = uint(object.getCustomProperty( "emitter2_refid", 0 ) );
        mLightRefId  = uint(object.getCustomProperty( "light_refid", 0 ) );
        
        // Cache references to objects
        Scene @ scene = object.getScene();
        if ( mCoverRefId != 0 )
            @mCover = scene.getObjectNodeById( mCoverRefId );
        if ( mLightRefId != 0 )
            @mLight = cast<LightNode>(scene.getObjectNodeById( mLightRefId ));
        if ( mSteam1RefId != 0 )
            @mSteam1 = cast<ParticleEmitterNode>(scene.getObjectNodeById( mSteam1RefId ) );
        if ( mSteam2RefId != 0 )
            @mSteam2 = cast<ParticleEmitterNode>(scene.getObjectNodeById( mSteam2RefId ) );

        // Store original light color values
        if ( @mLight != null )
        {
            mOriginalDiffuse  = mLight.getDiffuseColor();
            mOriginalSpecular = mLight.getSpecularColor();

        } // End if has light

        // Call base class implementation last.
        Objective::onAttach( object );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release our references.
        @mSteam1 = null;
        @mSteam2 = null;
        @mCover  = null;
        @mLight  = null;

        // Call base class implementation last
        Objective::onDetach( object );
    }
    
    //-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Slide the cover down.
        if ( @mCover != null )
        {
            const float CoverMaxDistance = 1.67f;

            // Compute amount to open.
            float moveAmount = elapsedTime * 0.5f;
            mSlideAmount += moveAmount;
            if ( mSlideAmount >  CoverMaxDistance )
            {
                moveAmount -= (mSlideAmount - CoverMaxDistance);
                mSlideAmount = CoverMaxDistance;
            
            } // End if overshoots

            // Adjust cover
            mCover.moveLocal( 0, -moveAmount, 0 );

            // Dim light source
            if ( @mLight != null )
            {
                float scale = 1.0f - ((mSlideAmount / CoverMaxDistance) * 0.5f);

                // Diminish diffuse
                ColorValue color = mOriginalDiffuse;
                color.r *= scale;
                color.g *= scale;
                color.b *= scale;
                mLight.setDiffuseColor( color );

                // Diminish specular
                color = mOriginalSpecular;
                color.r *= scale;
                color.g *= scale;
                color.b *= scale;
                mLight.setSpecularColor( color );

            } // End if has light

            // Done sliding?
            if ( mSlideAmount >= CoverMaxDistance )
                mNode.setUpdateRate( UpdateRate::Never );

        } // End if has cover
        else
            mNode.setUpdateRate( UpdateRate::Never );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (Objective)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : onActivate () (Event)
	// Desc : This objective was activated. Returns new objective (if any)
	//-------------------------------------------------------------------------
    Objective @ onActivate( ObjectNode @ issuer )
    {
        // Trigger this gameplay sequence.
        mGamePlayState.setCurrentGameSequence( mSequenceIdentifier );

        // Enable updates for this objective so we can slide the cover down.
        if ( @mCover != null )
            mNode.setUpdateRate( UpdateRate::Always );
        mSlideAmount = 0;

        // Enable updates to particle emitter.
        if ( @mSteam1 != null )
        {
            mSteam1.setUpdateRate( UpdateRate::Always );
            mSteam1.enableLayerEmission( 0, true );
        
        } // End if has steam
        if ( @mSteam2 != null )
        {
            mSteam2.setUpdateRate( UpdateRate::Always );
            mSteam2.enableLayerEmission( 0, true );
        
        } // End if has steam

        // Get the next objective.
        if ( mNextObjectiveId != 0 )
        {
            Scene @ scene = mNode.getScene();
            ObjectNode @ objectiveNode = scene.getObjectNodeById( mNextObjectiveId );
            if ( objectiveNode.getBehaviorCount() > 0 )
            {
                Objective @ nextObjective = cast<Objective>(objectiveNode.getScriptedBehavior(0));
                if ( mSequenceIdentifier == "trigger_generator_4" )
                {
                    // Kill the turret.
                    ObjectNode@ turretNode = scene.getObjectNodeById( TurretRefId );
                    if ( @turretNode != null )
                    {
                        Agent @ turretAgent = cast<Agent>(turretNode.getScriptedBehavior(0));
                        if ( @turretAgent != null )
                            turretAgent.kill();

                    } // End if found turret

                    // Trigger the next objective (elevator doors will close).
                    nextObjective.onActivate( issuer );
                
                } // End if final trigger
                return nextObjective;

            } // End if has script
        
        } // End if has other objective

        // No new objective
        return null;
    }
}