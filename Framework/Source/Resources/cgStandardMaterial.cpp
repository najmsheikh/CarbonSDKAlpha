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
// Name : cgStandardMaterial.cpp                                             //
//                                                                           //
// Desc : Provides classes which outline the various standard material       //
//        properties that should be applied to individual surfaces within a  //
//        scene. These include properties such as light reflectance,         //
//        samplers and potentially even physical properties (friction etc.)  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgStandardMaterial Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgStandardMaterial.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/cgSurfaceShader.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgSampler.h>
#include <System/cgExceptions.h>

// Preview rendering
#include <System/cgImage.h>
#include <Resources/cgMesh.h>
#include <Resources/cgRenderTarget.h>
#include <World/cgScene.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgMeshObject.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgStandardMaterial::mInsertMaterial;
cgWorldQuery cgStandardMaterial::mInsertSampler;
cgWorldQuery cgStandardMaterial::mDeleteSamplers;
cgWorldQuery cgStandardMaterial::mUpdateName;
cgWorldQuery cgStandardMaterial::mUpdateProperties;
cgWorldQuery cgStandardMaterial::mUpdateDiffuse;
cgWorldQuery cgStandardMaterial::mUpdateSpecular;
cgWorldQuery cgStandardMaterial::mUpdateAmbient;
cgWorldQuery cgStandardMaterial::mUpdateEmissive;
cgWorldQuery cgStandardMaterial::mUpdateMetalness;
cgWorldQuery cgStandardMaterial::mUpdateRimLight;
cgWorldQuery cgStandardMaterial::mUpdateReflection;
cgWorldQuery cgStandardMaterial::mUpdateFresnel;
cgWorldQuery cgStandardMaterial::mUpdateBlending;
cgWorldQuery cgStandardMaterial::mUpdateTransmission;
cgWorldQuery cgStandardMaterial::mUpdateShader;
cgWorldQuery cgStandardMaterial::mUpdatePreview;
cgWorldQuery cgStandardMaterial::mLoadMaterial;
cgWorldQuery cgStandardMaterial::mLoadSamplers;

///////////////////////////////////////////////////////////////////////////////
// cgStandardMaterial Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgStandardMaterial () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgStandardMaterial::cgStandardMaterial( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgMaterial( nReferenceId, pWorld )
{
    // Initialize variables
    mExplicitDiffuseSampler   = CG_NULL;
    mExplicitNormalSampler    = CG_NULL;
    mExplicitOpacitySampler   = CG_NULL;
    mReflectionMode            = cgReflectionMode::None;
    mSpecularOpacityLinked    = true;
    mDiffuseSamplerRegister   = -1;
    mNormalSamplerRegister    = -1;
    mRegisterIndicesCached    = false;
    
    // Default transmission curve is 'LinearDecay'
    mTransmissionCurve.setDescription( cgBezierSpline2::LinearDecay );

    // Cached shader permutation selection.
    mHasEmissiveTexture       = false;
    mHasOpacityTexture        = false;
    mSpecularTextureChannels  = 0;
    mNormalsType              = 1; // NormalMap
    mLightmapsType            = 0; // None

    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::Standard'.
    mComponentTypeId  = (cgUInt32)cgMaterialType::Standard;
}

//-----------------------------------------------------------------------------
//  Name : cgStandardMaterial () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgStandardMaterial::cgStandardMaterial( cgUInt32 nReferenceId, cgWorld * pWorld, cgUInt32 nSourceRefId ) : cgMaterial( nReferenceId, pWorld, nSourceRefId )
{
    // Initialize variables
    mExplicitDiffuseSampler   = CG_NULL;
    mExplicitNormalSampler    = CG_NULL;
    mExplicitOpacitySampler   = CG_NULL;
    mReflectionMode           = cgReflectionMode::None;
    mSpecularOpacityLinked    = true;
    mDiffuseSamplerRegister   = -1;
    mNormalSamplerRegister    = -1;
    mRegisterIndicesCached    = false;

    // Default transmission curve is 'LinearDecay'
    mTransmissionCurve.setDescription( cgBezierSpline2::LinearDecay );

    // Cached shader permutation selection.
    mHasEmissiveTexture       = false;
    mHasOpacityTexture        = false;
    mSpecularTextureChannels  = 0;
    mNormalsType              = 1; // NormalMap
    mLightmapsType            = 0; // None

    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::Standard'.
    mComponentTypeId  = (cgUInt32)cgMaterialType::Standard;
}

//-----------------------------------------------------------------------------
//  Name : cgStandardMaterial () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgStandardMaterial::cgStandardMaterial( cgUInt32 nReferenceId, cgWorld * pWorld, cgStandardMaterial * pInit ) : cgMaterial( nReferenceId, pWorld, pInit )
{
    // Initialize variables to sensible defaults.
    mExplicitDiffuseSampler   = CG_NULL;
    mExplicitNormalSampler    = CG_NULL;
    mExplicitOpacitySampler   = CG_NULL;

    // Cached shader permutation selection.
    mHasEmissiveTexture       = false;
    mHasOpacityTexture        = false;
    mSpecularTextureChannels  = 0;
    mNormalsType              = 1; // NormalMap
    mLightmapsType            = 0; // None
    
    // Material data.
    mTransmissionCurve        = pInit->mTransmissionCurve;
    mMaterialTerms            = pInit->mMaterialTerms;
    mUserProperties           = pInit->mUserProperties;
    mReflectionMode           = pInit->mReflectionMode;
    mSpecularOpacityLinked    = pInit->mSpecularOpacityLinked;
    mDiffuseSamplerRegister   = pInit->mDiffuseSamplerRegister;
    mNormalSamplerRegister    = pInit->mNormalSamplerRegister;
    mRegisterIndicesCached    = pInit->mRegisterIndicesCached;

    // ToDo: 9999 - Clone constants

    // Clone samplers
    for ( size_t i = 0; i < pInit->mSamplers.size(); ++i )
    {
        cgSampler * pNewSampler = mManager->cloneSampler( pWorld, isInternalReference(), pInit->mSamplers[i] );
        
        // We now own a reference to this sampler
        pNewSampler->addReference( this, isInternalReference() );
        
        // Add to our internal list.
        mSamplers.push_back( pNewSampler );
        mNamedSamplers[ pNewSampler->getName() ] = pNewSampler;
        onSamplerAdded( pNewSampler->getName(), pNewSampler );

    } // Next Sampler

    // Local component type as represented in the database is equivalent
    // to the numeric value assigned to 'cgMaterialType::Standard'.
    mComponentTypeId  = (cgUInt32)cgMaterialType::Standard;
}

//-----------------------------------------------------------------------------
//  Name : ~cgStandardMaterial () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgStandardMaterial::~cgStandardMaterial( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgStandardMaterial::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base.
    if ( bDisposeBase == true )
        cgMaterial::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_StandardMaterial )
        return true;

    // Supported by base?
    return cgMaterial::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgStandardMaterial::getDatabaseTable( ) const
{
    return _T("Materials::Standard");
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::prepareQueries()
{
    // Any database supplied?
    if ( mWorld == CG_NULL )
        return;

    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertMaterial.isPrepared() == false )
            mInsertMaterial.prepare( mWorld, _T("INSERT INTO 'Materials::Standard' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21,?22,?23,?24,?25,?26,?27,?28,?29,?30,?31,?32,?33,?34,?35,?36,?37,?38,?39,?40,?41,?42,?43,?44,?45)"), true );  
        if ( mInsertSampler.isPrepared() == false )
            mInsertSampler.prepare( mWorld, _T("INSERT INTO 'Materials::Standard::Samplers' VALUES(NULL,?1,?2)"), true );  
        if ( mDeleteSamplers.isPrepared() == false )
            mDeleteSamplers.prepare( mWorld, _T("DELETE FROM 'Materials::Standard::Samplers' WHERE MaterialId=?1"), true );
        if ( mUpdateName.isPrepared() == false )
            mUpdateName.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET Name=?1 WHERE RefId=?2"), true );  
        if ( mUpdateProperties.isPrepared() == false )
            mUpdateProperties.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET PropertiesHighPart=?1, PropertiesLowPart=?2 WHERE RefId=?3"), true );  
        if ( mUpdateDiffuse.isPrepared() == false )
            mUpdateDiffuse.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET DiffuseReflectanceR=?1,DiffuseReflectanceG=?2,DiffuseReflectanceB=?3 WHERE RefId=?4"), true );  
        if ( mUpdateAmbient.isPrepared() == false )
            mUpdateAmbient.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET AmbientReflectanceR=?1,AmbientReflectanceG=?2,AmbientReflectanceB=?3 WHERE RefId=?4"), true );  
        if ( mUpdateSpecular.isPrepared() == false )
            mUpdateSpecular.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET SpecularReflectanceR=?1,SpecularReflectanceG=?2,SpecularReflectanceB=?3,Gloss=?4 WHERE RefId=?5"), true );  
        if ( mUpdateEmissive.isPrepared() == false )
            mUpdateEmissive.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET EmissiveColorR=?1,EmissiveColorG=?2,EmissiveColorB=?3,EmissiveHDRScalar=?4 WHERE RefId=?5"), true );  
        if ( mUpdateMetalness.isPrepared() == false )
            mUpdateMetalness.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET MetalnessAmount=?1, MetalnessDiffuse=?2, MetalnessSpecular=?3 WHERE RefId=?4"), true );  
        if ( mUpdateRimLight.isPrepared() == false )
            mUpdateRimLight.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET RimIntensity=?1, RimExponent=?2 WHERE RefId=?3"), true );  
        if ( mUpdateReflection.isPrepared() == false )
            mUpdateReflection.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET ReflectionMode=?1, ReflectionIntensity=?2, ReflectionBumpiness=?3, ReflectionMipLevel=?4 WHERE RefId=?5"), true );  
        if ( mUpdateFresnel.isPrepared() == false )
            mUpdateFresnel.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET FresnelDiffuse=?1, FresnelSpecular=?2, FresnelReflection=?3, FresnelOpacity=?4, FresnelExponent=?5 WHERE RefId=?6"), true );  
        if ( mUpdateBlending.isPrepared() == false )
            mUpdateBlending.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET BlendingMode=?1, Opacity=?2, SpecularOpacity=?3, SpecularOpacityLinked=?4, DiffuseOpacityMapStrength=?5, SpecularOpacityMapStrength=?6 WHERE RefId=?7"), true );  
        if ( mUpdateTransmission.isPrepared() == false )
            mUpdateTransmission.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET TransmissionType=?1, TransmissionCurveSize=?2, TransmissionCurve=?3 WHERE RefId=?4"), true );  
        if ( mUpdateShader.isPrepared() == false )
            mUpdateShader.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET ShaderSource=?1 WHERE RefId=?2"), true );  
        if ( mUpdatePreview.isPrepared() == false )
            mUpdatePreview.prepare( mWorld, _T("UPDATE 'Materials::Standard' SET PreviewImage=?1 WHERE RefId=?2"), true ); 
    
    } // End if sandbox

    // Read queries
    if ( mLoadMaterial.isPrepared() == false )
        mLoadMaterial.prepare( mWorld, _T("SELECT * FROM 'Materials::Standard' WHERE RefId=?1"), true );
    if ( mLoadSamplers.isPrepared() == false )
        mLoadSamplers.prepare( mWorld, _T("SELECT * FROM 'Materials::Standard::Samplers' WHERE MaterialId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : serializeMaterial() (Protected)
/// <summary>
/// Write or update the material data stored in the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::serializeMaterial( )
{
    // Bail if we are not allowed to serialize any data
    if ( !shouldSerialize() )
        return true;

    // Bail if there is nothing to do.
    if ( !mDBDirtyFlags && mMaterialSerialized )
        return true;

    // Catch exceptions
    try
    {
        // Start a new transaction
        mWorld->beginTransaction( _T("serializeMaterial") );

        // If material data has yet been inserted, serialize everything.
        // Note: 'Materials' table is part of the core specification
        // and as a result does not need to be created.
        prepareQueries();
        if ( !mMaterialSerialized )
        {
            // Material entry does not exist at all at this stage so insert it.
            mInsertMaterial.bindParameter( 1, mReferenceId );
            mInsertMaterial.bindParameter( 2, mName );
            mInsertMaterial.bindParameter( 3, (cgUInt32)((mProperties >> 32) & 0xFFFFFFFF) ); // High part
            mInsertMaterial.bindParameter( 4, (cgUInt32)(mProperties & 0xFFFFFFFF) ); // Low part

            // Base material reflectance terms.
            const cgMaterialTerms & p = mMaterialTerms;
            mInsertMaterial.bindParameter( 5, p.diffuse.r );
            mInsertMaterial.bindParameter( 6, p.diffuse.g );
            mInsertMaterial.bindParameter( 7, p.diffuse.b );
            mInsertMaterial.bindParameter( 8, p.ambient.r );
            mInsertMaterial.bindParameter( 9, p.ambient.g );
            mInsertMaterial.bindParameter( 10, p.ambient.b );
            mInsertMaterial.bindParameter( 11, p.specular.r );
            mInsertMaterial.bindParameter( 12, p.specular.g );
            mInsertMaterial.bindParameter( 13, p.specular.b );
            mInsertMaterial.bindParameter( 14, p.gloss );
            mInsertMaterial.bindParameter( 15, p.emissive.r );
            mInsertMaterial.bindParameter( 16, p.emissive.g );
            mInsertMaterial.bindParameter( 17, p.emissive.b );
            mInsertMaterial.bindParameter( 18, p.emissiveHDRScale );

            // Extended properties
            mInsertMaterial.bindParameter( 19, p.metalnessAmount );
            mInsertMaterial.bindParameter( 20, p.metalnessDiffuse );
            mInsertMaterial.bindParameter( 21, p.metalnessSpecular );
            mInsertMaterial.bindParameter( 22, p.rimIntensity );
            mInsertMaterial.bindParameter( 23, p.rimExponent );
            mInsertMaterial.bindParameter( 24, (cgUInt32)mReflectionMode );
            mInsertMaterial.bindParameter( 25, p.reflectionIntensity );
            mInsertMaterial.bindParameter( 26, p.reflectionBumpiness );
            mInsertMaterial.bindParameter( 27, p.reflectionMipLevel );
            mInsertMaterial.bindParameter( 28, p.fresnelExponent );
            mInsertMaterial.bindParameter( 29, p.fresnelDiffuse );
            mInsertMaterial.bindParameter( 30, p.fresnelSpecular );
            mInsertMaterial.bindParameter( 31, p.fresnelReflection );
            mInsertMaterial.bindParameter( 32, p.fresnelOpacity );

            // Blending
            cgUInt32 nTransmissionType = mTransmissionCurve.getDescription();
            mInsertMaterial.bindParameter( 33, (cgUInt32)0 ); // ToDo: 9999 - BlendingMode
            mInsertMaterial.bindParameter( 34, p.diffuse.a );
            mInsertMaterial.bindParameter( 35, p.specular.a );
            mInsertMaterial.bindParameter( 36, mSpecularOpacityLinked );
            mInsertMaterial.bindParameter( 37, p.diffuseOpacityMapStrength );
            mInsertMaterial.bindParameter( 38, p.specularOpacityMapStrength );
            mInsertMaterial.bindParameter( 39, (cgUInt32)nTransmissionType );
            if ( nTransmissionType == cgBezierSpline2::Custom )
            {
                mInsertMaterial.bindParameter( 40, mTransmissionCurve.getPointCount() );
                mInsertMaterial.bindParameter( 41, &mTransmissionCurve.getSplinePoints()[0], mTransmissionCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mInsertMaterial.bindParameter( 40, 0 );
                mInsertMaterial.bindParameter( 41, CG_NULL, 0 );
            
            } // End if described

            // Surface shader
            mInsertMaterial.bindParameter( 42, (cgUInt32)0 ); // ShaderFlags: 0 = Auto, 1 = Compiled, 2 = Source
            mInsertMaterial.bindParameter( 43, cgString::Empty );

            // Preview Image
            mInsertMaterial.bindParameter( 44, CG_NULL, 0 );

            // Database ref count (just in case it has already been adjusted)
            mInsertMaterial.bindParameter( 45, mSoftRefCount );

            // Process!
            if ( mInsertMaterial.step( true ) == false )
            {
                cgString strError;
                mInsertMaterial.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to insert data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Parameters and samplers should now be serialized.
            mDBDirtyFlags = SamplersDirty | ParametersDirty;
            
            // All other material properties have been serialized
            mMaterialSerialized = true;
        
        } // End if not yet serialized
        
        // Serialize name
        if ( mDBDirtyFlags & NameDirty )
        {
            // Update name.
            mUpdateName.bindParameter( 1, mName );
            mUpdateName.bindParameter( 2, mReferenceId );
            
            // Process!
            if ( mUpdateName.step( true ) == false )
            {
                cgString strError;
                mUpdateName.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update name data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~NameDirty;

        } // End if name dirty

        // Serialize properties
        if ( mDBDirtyFlags & PropertiesDirty )
        {
            // Update properties.
            mUpdateProperties.bindParameter( 1, (cgUInt32)((mProperties >> 32) & 0xFFFFFFFF) ); // High part
            mUpdateProperties.bindParameter( 2, (cgUInt32)(mProperties & 0xFFFFFFFF) ); // Low part
            mUpdateProperties.bindParameter( 3, mReferenceId );
            
            // Process!
            if ( mUpdateProperties.step( true ) == false )
            {
                cgString strError;
                mUpdateProperties.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update property data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~PropertiesDirty;

        } // End if properties dirty

        // Serialize diffuse
        if ( mDBDirtyFlags & DiffuseDirty )
        {
            // Update diffuse.
            mUpdateDiffuse.bindParameter( 1, mMaterialTerms.diffuse.r );
            mUpdateDiffuse.bindParameter( 2, mMaterialTerms.diffuse.g );
            mUpdateDiffuse.bindParameter( 3, mMaterialTerms.diffuse.b );
            mUpdateDiffuse.bindParameter( 4, mReferenceId );
            
            // Process!
            if ( mUpdateDiffuse.step( true ) == false )
            {
                cgString strError;
                mUpdateDiffuse.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update diffuse reflectance data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~DiffuseDirty;

        } // End if diffuse dirty

        // Serialize specular
        if ( mDBDirtyFlags & SpecularDirty )
        {
            // Update specular.
            mUpdateSpecular.bindParameter( 1, mMaterialTerms.specular.r );
            mUpdateSpecular.bindParameter( 2, mMaterialTerms.specular.g );
            mUpdateSpecular.bindParameter( 3, mMaterialTerms.specular.b );
            mUpdateSpecular.bindParameter( 4, mMaterialTerms.gloss );
            mUpdateSpecular.bindParameter( 5, mReferenceId );
            
            // Process!
            if ( mUpdateSpecular.step( true ) == false )
            {
                cgString strError;
                mUpdateSpecular.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update specular reflectance data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~SpecularDirty;

        } // End if specular dirty

        // Serialize ambient
        if ( mDBDirtyFlags & AmbientDirty )
        {
            // Update ambient.
            mUpdateAmbient.bindParameter( 1, mMaterialTerms.ambient.r );
            mUpdateAmbient.bindParameter( 2, mMaterialTerms.ambient.g );
            mUpdateAmbient.bindParameter( 3, mMaterialTerms.ambient.b );
            mUpdateAmbient.bindParameter( 4, mReferenceId );
            
            // Process!
            if ( mUpdateAmbient.step( true ) == false )
            {
                cgString strError;
                mUpdateAmbient.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update ambient reflectance data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~AmbientDirty;

        } // End if ambient dirty

        // Serialize emissive
        if ( mDBDirtyFlags & EmissiveDirty )
        {
            // Update emissive.
            mUpdateEmissive.bindParameter( 1, mMaterialTerms.emissive.r );
            mUpdateEmissive.bindParameter( 2, mMaterialTerms.emissive.g );
            mUpdateEmissive.bindParameter( 3, mMaterialTerms.emissive.b );
            mUpdateEmissive.bindParameter( 4, mMaterialTerms.emissiveHDRScale );
            mUpdateEmissive.bindParameter( 5, mReferenceId );
            
            // Process!
            if ( mUpdateEmissive.step( true ) == false )
            {
                cgString strError;
                mUpdateEmissive.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update emissive reflectance data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~EmissiveDirty;

        } // End if emissive dirty

        // Serialize blending
        if ( mDBDirtyFlags & BlendingDirty )
        {
            // Update blending.
            mUpdateBlending.bindParameter( 1, (cgUInt32)0 );                  // ToDo: 9999 - BlendingMode
            mUpdateBlending.bindParameter( 2, mMaterialTerms.diffuse.a );    // Opacity
            mUpdateBlending.bindParameter( 3, mMaterialTerms.specular.a );   // Specular opacity
            mUpdateBlending.bindParameter( 4, mSpecularOpacityLinked );
            mUpdateBlending.bindParameter( 5, mMaterialTerms.diffuseOpacityMapStrength );
            mUpdateBlending.bindParameter( 6, mMaterialTerms.specularOpacityMapStrength );
            mUpdateBlending.bindParameter( 7, mReferenceId );
            
            // Process!
            if ( mUpdateBlending.step( true ) == false )
            {
                cgString strError;
                mUpdateBlending.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update blending data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~BlendingDirty;

        } // End if blending dirty

        // Serialize metalness
        if ( mDBDirtyFlags & MetalnessDirty )
        {
            // Update metalness.
            mUpdateMetalness.bindParameter( 1, mMaterialTerms.metalnessAmount );
            mUpdateMetalness.bindParameter( 2, mMaterialTerms.metalnessDiffuse );
            mUpdateMetalness.bindParameter( 3, mMaterialTerms.metalnessSpecular );
            mUpdateMetalness.bindParameter( 4, mReferenceId );
            
            // Process!
            if ( mUpdateMetalness.step( true ) == false )
            {
                cgString strError;
                mUpdateMetalness.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update metalness data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~MetalnessDirty;

        } // End if metalness dirty

        // Serialize rim lighting properties
        if ( mDBDirtyFlags & RimLightDirty )
        {
            // Update rim light.
            mUpdateRimLight.bindParameter( 1, mMaterialTerms.rimIntensity );
            mUpdateRimLight.bindParameter( 2, mMaterialTerms.rimExponent );
            mUpdateRimLight.bindParameter( 3, mReferenceId );
            
            // Process!
            if ( mUpdateRimLight.step( true ) == false )
            {
                cgString strError;
                mUpdateRimLight.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update rim lighting data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~RimLightDirty;

        } // End if rim lighting dirty

        // Serialize reflection properties
        if ( mDBDirtyFlags & ReflectionDirty )
        {
            // Update reflection.
            mUpdateReflection.bindParameter( 1, (cgUInt32)mReflectionMode );
            mUpdateReflection.bindParameter( 2, mMaterialTerms.reflectionIntensity );
            mUpdateReflection.bindParameter( 3, mMaterialTerms.reflectionBumpiness );
            mUpdateReflection.bindParameter( 4, mMaterialTerms.reflectionMipLevel );
            mUpdateReflection.bindParameter( 5, mReferenceId );
            
            // Process!
            if ( mUpdateReflection.step( true ) == false )
            {
                cgString strError;
                mUpdateReflection.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update reflection data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~ReflectionDirty;

        } // End if reflection dirty

        // Serialize fresnel terms
        if ( mDBDirtyFlags & FresnelDirty )
        {
            // Update fresnel terms.
            mUpdateFresnel.bindParameter( 1, mMaterialTerms.fresnelDiffuse );
            mUpdateFresnel.bindParameter( 2, mMaterialTerms.fresnelSpecular );
            mUpdateFresnel.bindParameter( 3, mMaterialTerms.fresnelReflection );
            mUpdateFresnel.bindParameter( 4, mMaterialTerms.fresnelOpacity );
            mUpdateFresnel.bindParameter( 5, mMaterialTerms.fresnelExponent );
            mUpdateFresnel.bindParameter( 6, mReferenceId );
            
            // Process!
            if ( mUpdateFresnel.step( true ) == false )
            {
                cgString strError;
                mUpdateFresnel.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update fresnel term data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~FresnelDirty;

        } // End if fresnel dirty

        // Serialize effect
        if ( mDBDirtyFlags & ShaderFileDirty )
        {
            // Update effect file.
            cgSurfaceShader * pShader = mSurfaceShader.getResource(false);
            if ( pShader )
                mUpdateShader.bindParameter( 1, pShader->getResourceName() );
            else
                mUpdateShader.bindParameter( 1, cgString::Empty );
            mUpdateShader.bindParameter( 2, mReferenceId );
            
            // Process!
            if ( mUpdateShader.step( true ) == false )
            {
                cgString strError;
                mUpdateShader.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update surface shader data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~ShaderFileDirty;

        } // End if shader dirty

        // Serialize samplers
        if ( mDBDirtyFlags & SamplersDirty )
        {
            // Remove old samplers from database.
            mDeleteSamplers.bindParameter( 1, mReferenceId );
            if ( mDeleteSamplers.step( true ) == false )
            {
                cgString strError;
                mDeleteSamplers.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to delete prior sampler data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Insert new samplers into database
            for ( size_t i = 0; i < mSamplers.size(); ++i )
            {
                cgSampler * pSampler = mSamplers[i];
                
                // Insert!
                mInsertSampler.bindParameter( 1, mReferenceId );
                mInsertSampler.bindParameter( 2, pSampler->getReferenceId() );
                if ( mInsertSampler.step( true ) == false )
                {
                    cgString strError;
                    mInsertSampler.getLastError( strError );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to insert sampler data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                } // End if failed

            } // Next Subset

            // Property has been serialized
            mDBDirtyFlags &= ~SamplersDirty;

        } // End if samplers dirty

        // Serialize transmission curve
        if ( mDBDirtyFlags & TransmissionDirty )
        {
            // Compute transmission type.
            cgUInt32 nType = mTransmissionCurve.getDescription();

            // Update curve properties
            mUpdateTransmission.bindParameter( 1, (cgUInt32)nType );
            if ( nType == cgBezierSpline2::Custom )
            {
                mUpdateTransmission.bindParameter( 2, mTransmissionCurve.getPointCount() );
                mUpdateTransmission.bindParameter( 3, &mTransmissionCurve.getSplinePoints()[0], mTransmissionCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mUpdateTransmission.bindParameter( 2, 0 );
                mUpdateTransmission.bindParameter( 3, CG_NULL, 0 );
            
            } // End if described
            mUpdateTransmission.bindParameter( 4, mReferenceId );
            if ( mUpdateTransmission.step( true ) == false )
            {
                cgString strError;
                mUpdateTransmission.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to update transmission curve data for standard material resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Property has been serialized
            mDBDirtyFlags &= ~TransmissionDirty;

        } // End if transmission dirty

        // Commit recorded changes.
        mWorld->commitTransaction( _T("serializeMaterial") );

        // Success!
        return true;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        // Roll back any transaction
        mWorld->rollbackTransaction( _T("serializeMaterial") );
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch

}

//-----------------------------------------------------------------------------
//  Name : loadMaterial ()
/// <summary>
/// Load the specified material from the supplied database file.
/// Note : This function requires you to specify the resource manager into which
/// you would like all additional resources to be loaded (such as
/// textures, etc.) if you are not loading this material directly via the 
/// cgResourceManager::loadMaterial() methods.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::loadMaterial( cgUInt32 nSourceRefId, cgResourceManager * pManager /* = CG_NULL */ )
{
    // Call base class implementation first.
    if ( cgMaterial::loadMaterial( nSourceRefId, pManager ) == false )
        return false;

    // Handle exceptions
    try
    {
        // Load the primary material data entry.
        prepareQueries();
        mLoadMaterial.bindParameter( 1, nSourceRefId );
        if ( mLoadMaterial.step() == false || mLoadMaterial.nextRow() == false )
            throw cgExceptions::ResultException( _T("Failed to retrieve material data. World database has potentially become corrupt."), cgDebugSource() );

        // Retrieve the current 'soft' reference count for this component
        // if we are literally wrapping the database entry.
        if ( nSourceRefId == mReferenceId )
            mLoadMaterial.getColumn( _T("RefCount"), mSoftRefCount );

        // Populate name and register it with the resource manager (do not use
        // the 'SetName()' method as this will trigger premature serialization).
        mLoadMaterial.getColumn( _T("Name"), mName );
        if ( mName.empty() == false )
        {
            cgResourceManager::NameUsageMap & Usage = mManager->getMaterialNameUsage();
            cgString strKey = cgString::toLower( mName );
            cgResourceManager::NameUsageMap::iterator itName = Usage.find( strKey );
            if ( itName != Usage.end() )
                itName->second = itName->second + 1;
            else
                Usage[strKey] = 1;

        } // End if has name

        // Load material grouping information.
        cgUInt32 nPropertyPartHigh = 0, nPropertyPartLow = 0;
        mLoadMaterial.getColumn( _T("PropertiesHighPart"), nPropertyPartHigh );
        mLoadMaterial.getColumn( _T("PropertiesLowPart"), nPropertyPartLow );
        mProperties = (((cgUInt64)nPropertyPartHigh) << 32) | ((cgUInt64)nPropertyPartLow);

        // Base material reflectance terms.
        cgMaterialTerms & p = mMaterialTerms;
        mLoadMaterial.getColumn( _T("DiffuseReflectanceR") , p.diffuse.r );
        mLoadMaterial.getColumn( _T("DiffuseReflectanceG") , p.diffuse.g );
        mLoadMaterial.getColumn( _T("DiffuseReflectanceB") , p.diffuse.b );
        mLoadMaterial.getColumn( _T("AmbientReflectanceR") , p.ambient.r );
        mLoadMaterial.getColumn( _T("AmbientReflectanceG") , p.ambient.g );
        mLoadMaterial.getColumn( _T("AmbientReflectanceB") , p.ambient.b );
        mLoadMaterial.getColumn( _T("SpecularReflectanceR"), p.specular.r );
        mLoadMaterial.getColumn( _T("SpecularReflectanceG"), p.specular.g );
        mLoadMaterial.getColumn( _T("SpecularReflectanceB"), p.specular.b );
        mLoadMaterial.getColumn( _T("Gloss")               , p.gloss );
        mLoadMaterial.getColumn( _T("EmissiveColorR")      , p.emissive.r );
        mLoadMaterial.getColumn( _T("EmissiveColorG")      , p.emissive.g );
        mLoadMaterial.getColumn( _T("EmissiveColorB")      , p.emissive.b );
        mLoadMaterial.getColumn( _T("EmissiveHDRScalar")   , p.emissiveHDRScale );

        // Extended parameters
        mLoadMaterial.getColumn( _T("MetalnessAmount")     , p.metalnessAmount );
        mLoadMaterial.getColumn( _T("MetalnessDiffuse")    , p.metalnessDiffuse );
        mLoadMaterial.getColumn( _T("MetalnessSpecular")   , p.metalnessSpecular );
        mLoadMaterial.getColumn( _T("RimIntensity")        , p.rimIntensity );
        mLoadMaterial.getColumn( _T("RimExponent")         , p.rimExponent );
        mLoadMaterial.getColumn( _T("ReflectionMode")      , (cgUInt32&)mReflectionMode );
        mLoadMaterial.getColumn( _T("ReflectionIntensity") , p.reflectionIntensity );
        mLoadMaterial.getColumn( _T("ReflectionBumpiness") , p.reflectionBumpiness );
        mLoadMaterial.getColumn( _T("ReflectionMipLevel")  , p.reflectionMipLevel );
        mLoadMaterial.getColumn( _T("FresnelExponent")     , p.fresnelExponent );
        mLoadMaterial.getColumn( _T("FresnelDiffuse")      , p.fresnelDiffuse );
        mLoadMaterial.getColumn( _T("FresnelSpecular")     , p.fresnelSpecular );
        mLoadMaterial.getColumn( _T("FresnelReflection")   , p.fresnelReflection );
        mLoadMaterial.getColumn( _T("FresnelOpacity")      , p.fresnelOpacity );

        // Blending
        cgUInt32 nTransmissionType;
        //mLoadMaterial.getColumn( 18, (cgUInt32)0 ); // ToDo: 9999 - BlendingMode
        mLoadMaterial.getColumn( _T("Opacity"), p.diffuse.a );
        mLoadMaterial.getColumn( _T("SpecularOpacity"), p.specular.a );
        mLoadMaterial.getColumn( _T("SpecularOpacityLinked"), mSpecularOpacityLinked );
        mLoadMaterial.getColumn( _T("DiffuseOpacityMapStrength"), p.diffuseOpacityMapStrength );
        mLoadMaterial.getColumn( _T("SpecularOpacityMapStrength"), p.specularOpacityMapStrength );
        mLoadMaterial.getColumn( _T("TransmissionType"), nTransmissionType );

        // Load transmission curve
        if ( nTransmissionType == cgBezierSpline2::Custom )
        {
            cgUInt32 nPointCount, nSize;
            cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
            mLoadMaterial.getColumn( _T("TransmissionCurveSize"), nPointCount );
            mLoadMaterial.getColumn( _T("TransmissionCurve"), (void**)&pPointData, nSize );
            if ( pPointData )
            {
                mTransmissionCurve.clear();
                for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                    mTransmissionCurve.addPoint( pPointData[nPoint] );

            } // End if valid point data

        } // End if custom curve
        else
        {
            mTransmissionCurve.setDescription( (cgBezierSpline2::SplineDescription)nTransmissionType );

        } // End if described

        // Surface Shader
        cgString strShader;
        //mLoadMaterial.getColumn( "ShaderFlags", (cgUInt32)0 ); // ToDo
        mLoadMaterial.getColumn( _T("ShaderSource"), strShader );
        if ( !strShader.empty() )
        {
            if ( !mManager->createSurfaceShader( &mSurfaceShader, strShader, 0, cgDebugSource() ) )
                throw cgExceptions::ResultException( cgString::format( _T("Failed to load referenced surface shader '%s'."), strShader.c_str() ), cgDebugSource() );

        } // End if shader supplied

        // Preview Image
        // mLoadMaterial.getColumn( 22, CG_NULL, 0 ); // ToDo: 9999

        // We're done with the data from the material query
        mLoadMaterial.reset();

        // Now load the sampler data.
        mLoadSamplers.reset();
        mLoadSamplers.bindParameter( 1, nSourceRefId );
        if ( !mLoadSamplers.step() )
            throw cgExceptions::ResultException( _T("Material data contained invalid or corrupt sampler information."), cgDebugSource() );

        // Do we have a surface shader, or should the samplers use the manager default?
        cgSurfaceShaderHandle & hShader = getActiveSurfaceShader();

        // Iterate through each row returned and load each sampler.
        for ( ; mLoadSamplers.nextRow(); )
        {
            cgUInt32 nSamplerId = 0;
            mLoadSamplers.getColumn( _T("SamplerId"), nSamplerId );

            // ToDo: what resource loading flags should be specified?
            // Load the referenced sampler.
            cgSampler * pSampler = mManager->loadSampler( mWorld, hShader, nSamplerId, isInternalReference() );
            if ( !pSampler )
                continue;

            // Add the material as a reference holder (just reconnect,
            // do not adjust the database ref count).
            pSampler->addReference( this, true );

            // Push this sampler to all appropriate lists
            mSamplers.push_back( pSampler );
            mNamedSamplers[ pSampler->getName() ] = pSampler;
            
            // Update cached members
            onSamplerAdded( pSampler->getName(), pSampler );

        } // Next sampler

        // We're done with the sampler data.
        mLoadSamplers.reset();

        // If we are not simply wrapping the original database entry (i.e. the reference
        // identifier of this material resource doesn't match the source identifier)
        // then we potentially need to serialize this data to the database next time 
        // 'serializeMaterial()' is called (assuming we are not an internal resource). 
        // See 'loadResource()' for more information.
        if ( isInternalReference() == false && nSourceRefId != mReferenceId )
        {
            // We are not wrapping. Everything should be serialized.
            mMaterialSerialized   = false;
            mDBDirtyFlags         = AllDirty;

        } // End if not wrapping
        else
        {
            // We are wrapping. Everything is already serialized.
            mMaterialSerialized   = true;
            mDBDirtyFlags         = 0;
        
        } // End if wrapping
            
        // The material is now loaded
        mResourceLoaded = true;
        
        // Success!
        return true;

    } // End Try Block

    catch ( const cgExceptions::ResultException & e )
    {
        // Release any pending read operations.
        mLoadMaterial.reset();
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;

    } // End Catch Block
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// Unload the underlying resource data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::unloadResource( )
{
    cgUInt32 i;

    // Unload child resources
    mDefaultShader.close();

    // Release all samplers we own (disconnect).
    for ( i = 0; i < mSamplers.size(); ++i )
        mSamplers[i]->removeReference(this,true);
    mSamplers.clear();
    mNamedSamplers.clear();

    // Also release all default samplers if they were created.
    mExplicitDiffuseSampler   = CG_NULL;
    mExplicitNormalSampler    = CG_NULL;
    mExplicitOpacitySampler   = CG_NULL;

    // Clear out transmission curve data to save memory.
    mTransmissionCurve.clear();

    // Cached shader permutation selection.
    mHasEmissiveTexture       = false;
    mHasOpacityTexture        = false;
    mSpecularTextureChannels  = 0;
    mNormalsType              = 1;
    mLightmapsType            = 0;

    // Clear remaining variables
    mDiffuseSamplerRegister   = -1;
    mNormalSamplerRegister    = -1;
    mRegisterIndicesCached    = false;
   
    // Call base class implementation last.
    return cgMaterial::unloadResource( );
}

//-----------------------------------------------------------------------------
//  Name : buildPreviewImage ()
/// <summary>
/// Construct the preview image used by the sandbox material editor to display 
/// a representation of this material. An internal scene is provided which is
/// set up with an appropriate render control script should it be required
/// (for instance, object rendering).
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::buildPreviewImage( cgScene * pScene, cgRenderView * pView )
{
    // Create sphere geometry that will be used during the preview image construction.
    cgMeshNode   * pMeshNode   = (cgMeshNode*)pScene->createObjectNode( true, RTID_MeshObject, false );
    cgMeshObject * pMeshObject = (cgMeshObject*)pMeshNode->getReferencedObject();
    if ( pMeshObject->createSphere( 35.0f, 40, 80, false, cgMeshCreateOrigin::Center ) == false )
    {
        pScene->deleteObjectNode( pMeshNode );
        return false;
    
    } // End if failed

    // Apply this material to the mesh
    if ( pMeshObject->setMeshMaterial( cgMaterialHandle(this) ) == false )
    {
        pScene->deleteObjectNode( pMeshNode );
        return false;
    
    } // End if failed

    // Construct a temporary orthographic camera to use for rendering.
    cgSize s = pView->getSize();
    cgCameraNode * pCamera = (cgCameraNode*)pScene->createObjectNode( true, RTID_CameraObject, false );
    pCamera->setProjectionMode( cgProjectionMode::Orthographic );
    pCamera->setProjectionWindow( -(cgFloat)s.width / 2, (cgFloat)s.width / 2, -(cgFloat)s.height / 2, (cgFloat)s.height / 2 );
    pCamera->setNearClip( 1.0f );
    pCamera->setFarClip( (cgFloat)s.width );
    pCamera->lookAt( cgVector3( 10, 10, -((cgFloat)s.width / 2) ), cgVector3( 0, 0, 0 ), cgVector3( 0, 1, 0 ) );

    // Apply new camera.
    cgCameraNode * pOldCamera = pScene->getActiveCamera();
    pScene->setActiveCamera( pCamera );
    
    // Setup mesh's world transformation as required.
    cgTransform t;
    t.translate( pCamera->getYAxis() * 6.0f );
    pMeshNode->setWorldTransform( t );

    // Render the scene to the specified view (don't present).
    cgRenderDriver * pDriver = pScene->getRenderDriver();
    if ( pDriver->beginFrame( false, 0 ) )
    {
        if ( pView->begin() )
        {
            pScene->render();
            pView->end( false );
        
        } // End if RenderView::begin
        pDriver->endFrame( false );
    
    } // End if Driver::beginFrame

    // We're done rendering, finish up
    pScene->setActiveCamera( pOldCamera );
    pScene->deleteObjectNode( pMeshNode );
    pScene->deleteObjectNode( pCamera );

    // Create the image ready to store data.
    if ( mPreviewImage == CG_NULL )
        mPreviewImage = cgImage::createInstance();
    cgToDo( "DX11", "Texture format needs considering more given lack of ARGB in DX10+ (Use GetBestFormat()?)" );
    if ( mPreviewImage->createImage( s.width, s.height, cgBufferFormat::B8G8R8A8, true ) == false )
        return false;

    // Copy the render target data into the supplied image.
    cgRenderTargetHandle hTarget = pView->getViewBuffer();
    cgRenderTarget * pTarget = hTarget.getResource();
    if ( !pTarget || !pTarget->getImageData( *mPreviewImage ) )
        return false;

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : samplerSortPredicate () (Protected, Static)
/// <summary>
/// Predicate function that allows us to sort the sampler array by name.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::samplerSortPredicate( const cgSampler * lhs, const cgSampler * rhs )
{
    return lhs->getName() < rhs->getName();
}

//-----------------------------------------------------------------------------
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::onComponentDeleted( )
{
    // Remove our physical references to any child samplers. Full database update 
    // and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    for ( size_t i = 0; i < mSamplers.size(); ++i )
        mSamplers[i]->removeReference(this,isInternalReference());
    mSamplers.clear();
    mNamedSamplers.clear();
    
    // Call base class implementation last.
    cgMaterial::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
//  Name : compare ()
/// <summary>
/// Performs a deep compare allowing us to perform rapid binary tree 
/// style searches (such as the map in our resource manager).
/// </summary>
//-----------------------------------------------------------------------------
int cgStandardMaterial::compare( const cgMaterial & Material ) const
{
    cgFloat fDifference = 0;
    cgInt   nDifference = 0;

    // Call base class implementation first (checks effect file and material class).
    nDifference = cgMaterial::compare( Material ); 
    if ( nDifference != 0 ) return nDifference;

    // Rapid numeric / handle only tests
    const cgStandardMaterial & m = (const cgStandardMaterial&)Material;
    nDifference = (cgInt)mSamplers.size() - (cgInt)m.mSamplers.size();
    if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;

    // ToDo: 9999 - Blending mode
    nDifference = (cgInt)mReflectionMode - (cgInt)m.mReflectionMode;
    if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
    nDifference = (cgInt)mSpecularOpacityLinked - (cgInt)m.mSpecularOpacityLinked;
    if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
    
    // Test material properties. Diffuse first.
    fDifference = mMaterialTerms.diffuse.r - m.mMaterialTerms.diffuse.r;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.diffuse.g - m.mMaterialTerms.diffuse.g;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.diffuse.b - m.mMaterialTerms.diffuse.b;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.diffuse.a - m.mMaterialTerms.diffuse.a;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Specular
    fDifference = mMaterialTerms.specular.r - m.mMaterialTerms.specular.r;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.specular.g - m.mMaterialTerms.specular.g;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.specular.b - m.mMaterialTerms.specular.b;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.specular.a - m.mMaterialTerms.specular.a;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Ambient
    fDifference = mMaterialTerms.ambient.r - m.mMaterialTerms.ambient.r;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.ambient.g - m.mMaterialTerms.ambient.g;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.ambient.b - m.mMaterialTerms.ambient.b;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.ambient.a - m.mMaterialTerms.ambient.a;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Emissive
    fDifference = mMaterialTerms.emissive.r - m.mMaterialTerms.emissive.r;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.emissive.g - m.mMaterialTerms.emissive.g;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.emissive.b - m.mMaterialTerms.emissive.b;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.emissive.a - m.mMaterialTerms.emissive.a;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Specular / gloss term
    fDifference = m.mMaterialTerms.gloss - mMaterialTerms.gloss;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Metalness
    fDifference = mMaterialTerms.metalnessAmount - m.mMaterialTerms.metalnessAmount;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.metalnessDiffuse - m.mMaterialTerms.metalnessDiffuse;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.metalnessSpecular - m.mMaterialTerms.metalnessSpecular;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Rim lighting
    fDifference = mMaterialTerms.rimExponent - m.mMaterialTerms.rimExponent;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.rimIntensity - m.mMaterialTerms.rimIntensity;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Reflection
    fDifference = mMaterialTerms.reflectionIntensity - m.mMaterialTerms.reflectionIntensity;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.reflectionBumpiness - m.mMaterialTerms.reflectionBumpiness;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.reflectionMipLevel - m.mMaterialTerms.reflectionMipLevel;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Fresnel
    fDifference = mMaterialTerms.fresnelExponent - m.mMaterialTerms.fresnelExponent;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.fresnelDiffuse - m.mMaterialTerms.fresnelDiffuse;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.fresnelSpecular - m.mMaterialTerms.fresnelSpecular;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.fresnelReflection - m.mMaterialTerms.fresnelReflection;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.fresnelOpacity - m.mMaterialTerms.fresnelOpacity;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    /// Emissive HDR scale
    fDifference = mMaterialTerms.emissiveHDRScale - m.mMaterialTerms.emissiveHDRScale;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Transmission / blending terms
    fDifference = mMaterialTerms.diffuseOpacityMapStrength - m.mMaterialTerms.diffuseOpacityMapStrength;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;
    fDifference = mMaterialTerms.specularOpacityMapStrength - m.mMaterialTerms.specularOpacityMapStrength;
    if ( fabsf( fDifference ) > CGE_EPSILON ) return (fDifference < 0) ? -1 : 1;

    // Compare samplers (in name order)
    NameSamplerMap::const_iterator itSamplerSrc, itSamplerCmp;
    for ( itSamplerSrc = mNamedSamplers.begin(), itSamplerCmp = m.mNamedSamplers.begin(); 
          itSamplerSrc != mNamedSamplers.end(); ++itSamplerSrc, ++itSamplerCmp )
    {
        const cgSampler * pSrcSampler = itSamplerSrc->second;
        const cgSampler * pCmpSampler = itSamplerCmp->second;
        nDifference = pSrcSampler->compare( *pCmpSampler );
        if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;

    } // Next Sampler

    // ToDo: Compare user properties

    // ToDo: Compare controllers
    
    // Totally equal
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : setSurfaceShader ()
/// <summary>
/// Load the surface shader file specified for rendering with this material.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::setSurfaceShader( const cgSurfaceShaderHandle & hShader )
{
    // Is this a no-op?
    if ( mSurfaceShader == hShader )
        return true;

    // Call base class implementation
    if ( cgMaterial::setSurfaceShader( hShader ) == false )
        return false;

    // Need to re-query diffuse and normal sampler register indices
    // in case defaults need to be assigned. This happens automatically
    // at the last possible moment (to avoid loading the shader unnecessarily)
    // within 'applySamplers()' if 'mRegisterIndicesCached' is set to false.
    mDiffuseSamplerRegister = -1;
    mNormalSamplerRegister  = -1;
    mRegisterIndicesCached  = false;

    // ToDo: 9999 - Rebuild other samplers and constants to use this new shader.

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getActiveSurfaceShader () (Protected)
/// <summary>
/// Return the currently active surface shader. This may either be the 
/// surface shader currently assigned to the material (if any) or the default
/// resource manager assigned shader.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderHandle & cgStandardMaterial::getActiveSurfaceShader( )
{
    // Do we have a surface shader, or should the we use the manager default?
    cgSurfaceShaderHandle & hShader = (mSurfaceShader.isValid()) ? mSurfaceShader : mDefaultShader;
    if ( !hShader.isValid() )
    {
        mManager->createDefaultSurfaceShader( &mDefaultShader, 0, cgDebugSource() );
        hShader = mDefaultShader;
    
    } // End if invalid shader
    return hShader;
}

//-----------------------------------------------------------------------------
//  Name : addSampler ()
/// <summary>
/// Create the specified sampler and add it to our internal list. Returns the
/// newly created sampler if any.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgStandardMaterial::addSampler( const cgString & strName )
{
    return addSampler( strName, cgInputStream() );
}

//-----------------------------------------------------------------------------
//  Name : addSampler ()
/// <summary>
/// Create the specified sampler and add it to our internal list. Returns the
/// newly created sampler if any. Optionally load the specified texture.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgStandardMaterial::addSampler( const cgString & strName, cgInputStream texture, cgUInt32 textureLoadFlags /* = 0 */, const cgDebugSourceInfo & debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // We must already be associated with a resource manager.
    if ( mManager == CG_NULL )
        return CG_NULL;

    // If the sampler name is invalid, or one with this
    // name already exists, fail.
    if ( strName.empty() == true || mNamedSamplers.find( strName ) != mNamedSamplers.end() )
        return CG_NULL;

    // Do we have a surface shader, or should the samplers use the manager default?
    cgSurfaceShaderHandle & hShader = getActiveSurfaceShader();

    // Create a new sampler of this type (internal or serialized depending on 
    // the properties of this parent material).
    cgSampler * pSampler = CG_NULL;
    if ( isInternalReference() == false )
        pSampler = mManager->createSampler( mWorld, strName, hShader );
    else
        pSampler = mManager->createSampler( strName, hShader );
    if ( pSampler == CG_NULL )
        return CG_NULL;

    // Attempt to load any specified texture.
    if ( texture.sourceExists() )
        pSampler->loadTexture( texture, textureLoadFlags, debugSource );

    // Add the material as a reference holder.
    pSampler->addReference( this, isInternalReference() );

    // Push this sampler to all appropriate lists
    mSamplers.push_back( pSampler );
    mNamedSamplers[ strName ] = pSampler;

    // Update cached members.
    onSamplerAdded( strName, pSampler );

    // ToDo: 9999 - Specular mask packing into diffuse alpha.
    // ToDo: 9999 - Remove BeginPopulate() / EndPopulate()
    // ToDo: 9999 - Figure out how to handle the update of the resource manager's 
    //              material look up table perhaps we can just build it on the fly?.
    // ToDo: 9999 - If alpha blended, set the bTwoSided flag to true and set a default 
    //              light transfer value if not provided. Is this still necessary? Ask Joe.

    // Samplers are now dirty
    mDBDirtyFlags |= SamplersDirty;
    serializeMaterial();

    // Success!
    return pSampler;
}

//-----------------------------------------------------------------------------
//  Name : onSamplerAdded () (Protected)
/// <summary>
/// Update cached member variables based on the newly added sampler.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::onSamplerAdded( const cgString & strName, cgSampler * pSampler )
{
    // Do we have a surface shader, or should the samplers use the manager default?
    cgSurfaceShaderHandle & hShader = getActiveSurfaceShader();

    // If this is one of our required samplers (diffuse, normal, etc.), we need
    // to cache its pointer separately so that we can monitor its state.
    if ( strName == _T("Diffuse") )
        mExplicitDiffuseSampler = pSampler;
    else if ( strName == _T("Normal") )
        mExplicitNormalSampler = pSampler;
    else if ( strName == _T("Opacity") )
        mExplicitOpacitySampler = pSampler;

    // If this is one of the optional system samplers about which 
    // the shader library needs to be informed about, update the
    // cached shader permutation control values here.
    if ( strName == _T("Emissive") )
        mHasEmissiveTexture = true;
    else if ( strName == _T("Opacity") )
        mHasOpacityTexture = true;
    else if ( strName == _T("Specular") )
    {
		// Get the assigned texture to determine its readable channel count.
		mSpecularTextureChannels = 0;
		cgTextureHandle hTexture = pSampler->getTexture();
		if ( hTexture.isValid() )
		{
			cgTexture * pTexture = hTexture.getResourceSilent();
			mSpecularTextureChannels = cgBufferFormatEnum::formatChannelCount( pTexture->getInfo().format );
		
        } // End if valid texture
	
    } // End if Specular
    else if ( strName == _T("Light") )
        mLightmapsType = 1; // 1 = Standard, 2 = RNM
}

//-----------------------------------------------------------------------------
//  Name : removeSampler ()
/// <summary>
/// Remove the specified sampler from this material.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::removeSampler( const cgString & strName )
{
    // If the sampler name is invalid, or one with this
    // name already exists, fail.
    NameSamplerMap::iterator itSampler = mNamedSamplers.find( strName );
    if ( strName.empty() == true || itSampler == mNamedSamplers.end() )
        return false;

    // Remove this sampler from all appropriate lists
    cgSampler * pSampler = itSampler->second;
    mSamplers.erase( std::find(mSamplers.begin(), mSamplers.end(), pSampler) );
    mNamedSamplers.erase( strName );

    // Clean up the sampler.
    pSampler->removeReference( this, isInternalReference() );

    // Update cached members.
    onSamplerRemoved( strName );

    // Samplers are now dirty
    mDBDirtyFlags |= SamplersDirty;
    serializeMaterial();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onSamplerRemoved () (Protected)
/// <summary>
/// Update cached member variables based on the removed sampler.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::onSamplerRemoved( const cgString & strName )
{
    // If this is one of our required samplers (diffuse, normal, etc.), we need
    // to remove the cached pointer used to monitor its state.
    if ( strName == _T("Diffuse") )
        mExplicitDiffuseSampler = CG_NULL;
    else if ( strName == _T("Normal") )
        mExplicitNormalSampler = CG_NULL;
    else if ( strName == _T("Opacity") )
        mExplicitOpacitySampler = CG_NULL;

    // If this is one of the optional system samplers about which 
    // the shader library needs to be informed about, update the
    // cached shader permutation control values here.
    if ( strName == _T("Emissive") )
        mHasEmissiveTexture = false;
    else if ( strName == _T("Opacity") )
        mHasOpacityTexture = false;
    else if ( strName == _T("Specular") )
        mSpecularTextureChannels = 0;
    else if ( strName == _T("Light") )
        mLightmapsType = 0;
}

//-----------------------------------------------------------------------------
//  Name : getSurfaceShader ()
/// <summary>
/// Retrieve the loaded surface shader from the material. Specifying a value of
/// 'false' to the 'bAssignedOnly' parameter will instruct the material to 
/// return its 'default' fallback shader if one is not currently assigned. The
/// default value for this parameter is 'true' meaning only the user assigned
/// shader will be returned.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderHandle cgStandardMaterial::getSurfaceShader( bool bAssignedOnly )
{
    return ( bAssignedOnly ) ? mSurfaceShader : getActiveSurfaceShader( );
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// Apply the necessary material data to the device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::apply( cgRenderDriver * pDriver )
{
    // Apply material properties
    pDriver->setMaterialTerms( mMaterialTerms );
        
    // Apply the material's samplers
    applySamplers();

    // Setup shader permutation controls.
    pDriver->setSystemState( cgSystemState::SampleEmissiveTexture, mHasEmissiveTexture );
    pDriver->setSystemState( cgSystemState::SampleOpacityTexture , mHasOpacityTexture );
    pDriver->setSystemState( cgSystemState::SampleSpecularColor  , mSpecularTextureChannels >= 3 );
	pDriver->setSystemState( cgSystemState::SampleSpecularMask   , mSpecularTextureChannels == 2 );
	pDriver->setSystemState( cgSystemState::SampleGlossTexture   , mSpecularTextureChannels > 0 && mSpecularTextureChannels != 3 );
    pDriver->setSystemState( cgSystemState::LightTextureType     , mLightmapsType );
    pDriver->setSystemState( cgSystemState::NormalSource         , mNormalsType );
    pDriver->setSystemState( cgSystemState::ReflectionMode       , (cgInt32)mReflectionMode );

    // If a diffuse sampler was provided, turn on sampling
    pDriver->setSystemState( cgSystemState::SampleDiffuseTexture, (mExplicitDiffuseSampler != CG_NULL) );
	
    // Opacity is taken from diffuse alpha if they share the same texture.
    if ( mHasOpacityTexture && mExplicitDiffuseSampler )
    {
        cgToDo( "Carbon General", "We can actually perform this test when samplers are added / removed rather than every time." );
        if ( mExplicitDiffuseSampler->getTexture() == mExplicitOpacitySampler->getTexture() )
            pDriver->setSystemState( cgSystemState::OpacityInDiffuse, true );
        else
            pDriver->setSystemState( cgSystemState::OpacityInDiffuse, false );

    } // End if has opacity & diffuse
    else
    {
        // Not shared.
        pDriver->setSystemState( cgSystemState::OpacityInDiffuse, false );
    
    } // End if no opacity | diffuse

    // ToDo: 6767 - Should only be enabled if the artist sets an alpha test threshold
    // and potentially even assigns the alpha tested style, not just whether it has an opacity texture.
    // Turn on alpha testing when opacity texture is to be sampled
	pDriver->setSystemState( cgSystemState::AlphaTest, mHasOpacityTexture );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : reset ()
/// <summary>
/// Reset / restore any applicable data when the material is no longer set to 
/// the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::reset( cgRenderDriver * pDriver )
{
    // Nothing in this implementation
}

//-----------------------------------------------------------------------------
//  Name : applySamplers ()
/// <summary>
/// Apply the necessary samplers to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::applySamplers( )
{
    // Apply each of our managed samplers.
    for ( size_t i = 0; i < mSamplers.size(); ++i )
        mSamplers[i]->apply();

    // Do we need to bind default samplers?
    bool bBindDefaultDiffuse = (!mExplicitDiffuseSampler || !mExplicitDiffuseSampler->isTextureValid());
    bool bBindDefaultNormal  = (!mExplicitNormalSampler || !mExplicitNormalSampler->isTextureValid());

    // Find the sampler indices if they have not already been determined.
    if ( !mRegisterIndicesCached && (bBindDefaultDiffuse || bBindDefaultNormal) )
    {
        cgSurfaceShaderHandle & hShader = getActiveSurfaceShader();
        cgSurfaceShader * pShader = hShader.getResource(true);
        if ( pShader && pShader->isLoaded() )
        {
            mDiffuseSamplerRegister = pShader->findSamplerRegister( _T("Diffuse") );
            mNormalSamplerRegister  = pShader->findSamplerRegister( _T("Normal") );
            mRegisterIndicesCached = true;
        
        } // End if loaded
    
    } // End if !queried

    // A default diffuse sampler is always required. If no diffuse sampler / texture 
    // was available, use the default one available from the resource manager.
    if ( bBindDefaultDiffuse && mDiffuseSamplerRegister >= 0 )
        mManager->getDefaultSampler( cgResourceManager::DefaultDiffuseSampler )->apply( mDiffuseSamplerRegister );
    
    // We also need to do the same for the normal sampler.
    if ( bBindDefaultNormal && mNormalSamplerRegister >= 0 )
        mManager->getDefaultSampler( cgResourceManager::DefaultNormalSampler )->apply( mNormalSamplerRegister );
}

//-----------------------------------------------------------------------------
//  Name : getSamplers ()
/// <summary>
/// Retrieve a list of the samplers defined / utilized by this material.
/// </summary>
//-----------------------------------------------------------------------------
const cgStandardMaterial::SamplerArray & cgStandardMaterial::getSamplers( ) const
{
    return mSamplers;
}

//-----------------------------------------------------------------------------
//  Name : getSamplerByName ()
/// <summary>
/// Retrieve the specified sampler by name. If this sampler does not exist
/// a value of CG_NULL is returned.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgStandardMaterial::getSamplerByName( const cgString & strName )
{
    NameSamplerMap::iterator itSampler = mNamedSamplers.find( strName );
    if ( itSampler == mNamedSamplers.end() )
        return CG_NULL;
    return itSampler->second;
}

//-----------------------------------------------------------------------------
//  Name : setMaterialTerms ()
/// <summary>
/// Set the terms to use for this material. Such terms include lighting
/// reflectance values.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setMaterialTerms( const cgMaterialTerms & Terms )
{
    cgUInt32 nDirtyFlags = 0;

    // Mark relevant material terms as dirty.
    const cgMaterialTerms & p1 = mMaterialTerms, & p2 = Terms;
    nDirtyFlags |= (p1.diffuse.r != p2.diffuse.r) ? DiffuseDirty : 0;
    nDirtyFlags |= (p1.diffuse.g != p2.diffuse.g) ? DiffuseDirty : 0;
    nDirtyFlags |= (p1.diffuse.b != p2.diffuse.b) ? DiffuseDirty : 0;
    nDirtyFlags |= (p1.ambient.r != p2.ambient.r) ? AmbientDirty : 0;
    nDirtyFlags |= (p1.ambient.g != p2.ambient.g) ? AmbientDirty : 0;
    nDirtyFlags |= (p1.ambient.b != p2.ambient.b) ? AmbientDirty : 0;
    nDirtyFlags |= (p1.specular.r != p2.specular.r) ? SpecularDirty : 0;
    nDirtyFlags |= (p1.specular.g != p2.specular.g) ? SpecularDirty : 0;
    nDirtyFlags |= (p1.specular.b != p2.specular.b) ? SpecularDirty : 0;
    nDirtyFlags |= (p1.gloss != p2.gloss) ? SpecularDirty : 0;
    nDirtyFlags |= (p1.emissive.r != p2.emissive.r) ? EmissiveDirty : 0;
    nDirtyFlags |= (p1.emissive.g != p2.emissive.g) ? EmissiveDirty : 0;
    nDirtyFlags |= (p1.emissive.b != p2.emissive.b) ? EmissiveDirty : 0;
    nDirtyFlags |= (p1.emissiveHDRScale != p2.emissiveHDRScale) ? EmissiveDirty : 0;
    nDirtyFlags |= (p1.metalnessAmount != p2.metalnessAmount) ? MetalnessDirty : 0;
    nDirtyFlags |= (p1.metalnessDiffuse != p2.metalnessDiffuse) ? MetalnessDirty : 0;
    nDirtyFlags |= (p1.metalnessSpecular != p2.metalnessSpecular) ? MetalnessDirty : 0;
    nDirtyFlags |= (p1.rimExponent != p2.rimExponent) ? RimLightDirty : 0;
    nDirtyFlags |= (p1.rimIntensity != p2.rimIntensity) ? RimLightDirty : 0;
    nDirtyFlags |= (p1.reflectionIntensity != p2.reflectionIntensity) ? ReflectionDirty : 0;
    nDirtyFlags |= (p1.reflectionBumpiness != p2.reflectionBumpiness) ? ReflectionDirty : 0;
    nDirtyFlags |= (p1.reflectionMipLevel != p2.reflectionMipLevel) ? ReflectionDirty : 0;
    nDirtyFlags |= (p1.fresnelExponent != p2.fresnelExponent) ? FresnelDirty : 0;
    nDirtyFlags |= (p1.fresnelDiffuse != p2.fresnelDiffuse) ? FresnelDirty : 0;
    nDirtyFlags |= (p1.fresnelSpecular != p2.fresnelSpecular) ? FresnelDirty : 0;
    nDirtyFlags |= (p1.fresnelReflection != p2.fresnelReflection) ? FresnelDirty : 0;
    nDirtyFlags |= (p1.fresnelOpacity != p2.fresnelOpacity) ? FresnelDirty : 0;
    nDirtyFlags |= (p1.diffuse.a != p2.diffuse.a) ? BlendingDirty : 0;
    nDirtyFlags |= (p1.specular.a != p2.specular.a) ? BlendingDirty : 0;
    nDirtyFlags |= (p1.diffuseOpacityMapStrength != p2.diffuseOpacityMapStrength) ? BlendingDirty : 0;
    nDirtyFlags |= (p1.specularOpacityMapStrength != p2.specularOpacityMapStrength) ? BlendingDirty : 0;

    // No-op?
    if ( nDirtyFlags == 0 )
        return;

    // Update local member(s)
    mMaterialTerms = Terms;

    // Update the database.
    mDBDirtyFlags |= nDirtyFlags;
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setDiffuse ()
/// <summary>
/// Set the diffuse reflectance / material term.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setDiffuse( const cgColorValue & Color )
{
    // No-op?
    cgColorValue NewColor = cgColorValue( Color.r, Color.g, Color.b, mMaterialTerms.diffuse.a );
    if ( mMaterialTerms.diffuse == NewColor )
        return;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= DiffuseDirty;
    
    // Update local member(s)
    mMaterialTerms.diffuse = NewColor;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setAmbient ()
/// <summary>
/// Set the ambient reflectance / material term.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setAmbient( const cgColorValue & Color )
{
    // No-op?
    cgColorValue NewColor = cgColorValue( Color.r, Color.g, Color.b, 1.0f );
    if ( mMaterialTerms.ambient == NewColor )
        return;
    
    // Update local member(s)
    mMaterialTerms.ambient = NewColor;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= AmbientDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setSpecular ()
/// <summary>
/// Set the specular reflectance / material term.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setSpecular( const cgColorValue & Color )
{
    // No-op?
    cgColorValue NewColor = cgColorValue( Color.r, Color.g, Color.b, mMaterialTerms.specular.a );
    if ( mMaterialTerms.specular == NewColor )
        return;
    
    // Update local member(s)
    mMaterialTerms.specular = NewColor;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= SpecularDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setMetalnessTerms ()
/// <summary>
/// Set the metalness material terms.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setMetalnessTerms( cgFloat fAmount, cgFloat fDiffuseIntensity, cgFloat fSpecularIntensity )
{
    // No-op?
    if ( mMaterialTerms.metalnessAmount == fAmount 
        && mMaterialTerms.metalnessDiffuse == fDiffuseIntensity 
        && mMaterialTerms.metalnessDiffuse == fSpecularIntensity )
        return;
    
    // Update local member(s)
    mMaterialTerms.metalnessAmount   = fAmount;
    mMaterialTerms.metalnessDiffuse  = fDiffuseIntensity;
    mMaterialTerms.metalnessSpecular = fSpecularIntensity;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= MetalnessDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setRimLightTerms ()
/// <summary>
/// Set the rim lighting terms.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setRimLightTerms( cgFloat fAmount, cgFloat fExponent )
{
    // No-op?
    if ( mMaterialTerms.rimIntensity == fAmount 
        && mMaterialTerms.rimExponent == fExponent )
        return;
    
    // Update local member(s)
    mMaterialTerms.rimIntensity = fAmount;
    mMaterialTerms.rimExponent  = fExponent;
    
    // Relevant terms are now dirty.
    mDBDirtyFlags |= RimLightDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setFresnelTerms ()
/// <summary>
/// Set the fresnel terms.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setFresnelTerms( cgFloat fDiffuse, cgFloat fSpecular, cgFloat fReflection, cgFloat fOpacity, cgFloat fExponent )
{
    // No-op?
    if ( mMaterialTerms.fresnelDiffuse == fDiffuse 
        && mMaterialTerms.fresnelSpecular == fSpecular 
        && mMaterialTerms.fresnelReflection == fReflection
        && mMaterialTerms.fresnelOpacity == fOpacity
        && mMaterialTerms.fresnelExponent == fExponent )
        return;
    
    // Update local member(s)
    mMaterialTerms.fresnelDiffuse = fDiffuse;
    mMaterialTerms.fresnelSpecular = fSpecular;
    mMaterialTerms.fresnelReflection = fReflection;
    mMaterialTerms.fresnelOpacity = fOpacity;
    mMaterialTerms.fresnelExponent = fExponent;
    
    // Relevant terms are now dirty.
    mDBDirtyFlags |= FresnelDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setReflectionMode ()
/// <summary>
/// Set the manner in which surface reflections will be handled for geometry
/// utilizing to this material.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setReflectionMode( cgReflectionMode::Base Mode )
{
    // No-op?
    if ( mReflectionMode == Mode )
        return;
    
    // Update local member(s)
    mReflectionMode = Mode;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= ReflectionDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : getReflectionMode ()
/// <summary>
/// retrieve the manner in which surface reflections will be handled for 
/// geometry utilizing this material.
/// </summary>
//-----------------------------------------------------------------------------
cgReflectionMode::Base cgStandardMaterial::getReflectionMode( ) const
{
    return mReflectionMode;
}

//-----------------------------------------------------------------------------
//  Name : setReflectionTerms ()
/// <summary>
/// Set the material reflection terms.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setReflectionTerms( cgFloat fAmount, cgFloat fBumpiness, cgFloat fMipLevel )
{
    // No-op?
    if ( mMaterialTerms.reflectionIntensity == fAmount 
        && mMaterialTerms.reflectionBumpiness == fBumpiness
        && mMaterialTerms.reflectionMipLevel == fMipLevel )
        return;
    
    // Update local member(s)
    mMaterialTerms.reflectionIntensity = fAmount;
    mMaterialTerms.reflectionBumpiness = fBumpiness;
    mMaterialTerms.reflectionMipLevel  = fMipLevel;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= ReflectionDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setGloss ()
/// <summary>
/// Set the gloss / specular power material term.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setGloss( cgFloat fGloss )
{
    // No-op?
    if ( mMaterialTerms.gloss == fGloss )
        return;
    
    // Update local member(s)
    mMaterialTerms.gloss = fGloss;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= SpecularDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setEmissive ()
/// <summary>
/// Set the emissive / self illumination material term.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setEmissive( const cgColorValue & Color )
{
    // No-op?
    cgColorValue NewColor = cgColorValue( Color.r, Color.g, Color.b, 1.0f );
    if ( mMaterialTerms.emissive == NewColor )
        return;
    
    // Update local member(s)
    mMaterialTerms.emissive = NewColor;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= EmissiveDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setEmissiveHDRScale ()
/// <summary>
/// Set the amount by which emissive should be scaled when HDR is enabled.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setEmissiveHDRScale( cgFloat fScale )
{
    // No-op?
    if ( mMaterialTerms.emissiveHDRScale == fScale )
        return;
    
    // Update local member(s)
    mMaterialTerms.emissiveHDRScale = fScale;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= EmissiveDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setOpacity ()
/// <summary>
/// Set the overall opacity level of this material.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setOpacity( cgFloat fOpacity )
{
    // No-op?
    if ( mMaterialTerms.diffuse.a == fOpacity )
        return;
    
    // Update local member(s)
    mMaterialTerms.diffuse.a = fOpacity;

    // If specular opacity linking is enabled, ensure that opacity is
    // updated to match this newly set value.
    if ( mSpecularOpacityLinked == true )
        mMaterialTerms.specular.a = mMaterialTerms.diffuse.a;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= BlendingDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setSpecularOpacity ()
/// <summary>
/// Set the overall opacity level of the specular component for this material.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setSpecularOpacity( cgFloat fOpacity )
{
    // No-op?
    if ( mMaterialTerms.specular.a == fOpacity )
        return;
    
    // Update local member(s)
    mMaterialTerms.specular.a = fOpacity;

    // If specular opacity linking is enabled, ensure that opacity is
    // updated to match this newly set value.
    if ( mSpecularOpacityLinked == true )
        mMaterialTerms.diffuse.a = mMaterialTerms.specular.a;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= BlendingDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setSpecularOpacityLinked()
/// <summary>
/// Enable / disable specular opacity linking. When this is enabled, the 
/// specular and standard opacity terms will always be updated to match 
/// irrespective of which one was updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setSpecularOpacityLinked( bool bLinked )
{
    // No-op?
    if ( mSpecularOpacityLinked == bLinked )
        return;
    
    // Update local member(s)
    mSpecularOpacityLinked = bLinked;

    // If this is being enabled, ensure that specular opacity is set
    // to match the standard opacity (which takes priority initially).
    if ( bLinked == true )
        mMaterialTerms.specular.a = mMaterialTerms.diffuse.a;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= BlendingDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setOpacityMapContributions()
/// <summary>
/// Describes the contribution that the opacity map has for each of the diffuse
/// and specular reflection.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setOpacityMapContributions( cgFloat fDiffuse, cgFloat fSpecular )
{
    // No-op?
    if ( mMaterialTerms.diffuseOpacityMapStrength == fDiffuse
        && mMaterialTerms.specularOpacityMapStrength == fSpecular )
        return;
    
    // Update local member(s)
    mMaterialTerms.diffuseOpacityMapStrength = fDiffuse;
    mMaterialTerms.specularOpacityMapStrength = fSpecular;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= BlendingDirty;
    
    // Update the database.
    if ( serializeMaterial() == false )
        return;
}

//-----------------------------------------------------------------------------
//  Name : setTransmissionCurve()
/// <summary>
/// Set the curve that describes the amount of light which is transmitted 
/// through the surface in relation to its per-pixel opacity.
/// </summary>
//-----------------------------------------------------------------------------
void cgStandardMaterial::setTransmissionCurve( const cgBezierSpline2 & Curve )
{
    // Update local members
    mTransmissionCurve = Curve;

    // Relevant terms are now dirty.
    mDBDirtyFlags |= TransmissionDirty;

    // Update the database.
    if ( !serializeMaterial() )
        return;
}

//-----------------------------------------------------------------------------
//  Name : getTransmissionCurve()
/// <summary>
/// Retrieve the curve that describes the amount of light which is transmitted 
/// through the surface in relation to its per-pixel opacity.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2 & cgStandardMaterial::getTransmissionCurve( ) const
{
    return mTransmissionCurve;
}

//-----------------------------------------------------------------------------
//  Name : isSpecularOpacityLinked()
/// <summary>
/// Determine if the opacity and specular opacity terms are linked.
/// When this is enabled, these two terms will always be set to match
/// irrespective of which of the two was modified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgStandardMaterial::isSpecularOpacityLinked( ) const
{
    return mSpecularOpacityLinked;
}

//-----------------------------------------------------------------------------
//  Name : getMaterialTerms ()
/// <summary>
/// Retrieve the terms for this material. Such terms include lighting 
/// reflectance values, etc.
/// </summary>
//-----------------------------------------------------------------------------
const cgMaterialTerms & cgStandardMaterial::getMaterialTerms( ) const
{
    return mMaterialTerms;
}