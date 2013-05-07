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
// File : Carbon.h                                                           //
//                                                                           //
// Desc : Common header that includes access to commonly needed components   //
//        of the core framework.                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CARBON_H_ )
#define _CGE_CARBON_H_

// Core Includes
#include <cgAPI.h>
#include <cgBase.h>
#include <cgBaseTypes.h>
#include <cgConfig.h>

// Animation
#include <Animation\cgAnimationController.h>
#include <Animation\cgAnimationTarget.h>
#include <Animation\cgAnimationTypes.h>

// Audio
#include <Audio\cgAudioDriver.h>
#include <Audio\cgAudioTypes.h>

// Input
#include <Input\cgInputDriver.h>
#include <Input\cgInputTypes.h>

// User Interface
#include <Interface\cgTextEngine.h>
#include <Interface\cgUIControl.h>
#include <Interface\cgUIForm.h>
#include <Interface\cgUILayers.h>
#include <Interface\cgUIManager.h>
#include <Interface\cgUISkin.h>
#include <Interface\cgUITypes.h>
#include <Interface\Controls\cgButtonControl.h>
#include <Interface\Controls\cgComboBoxControl.h>
#include <Interface\Controls\cgGroupBoxControl.h>
#include <Interface\Controls\cgImageBoxControl.h>
#include <Interface\Controls\cgLabelControl.h>
#include <Interface\Controls\cgListBoxControl.h>
#include <Interface\Controls\cgScrollBarControl.h>
#include <Interface\Controls\cgTextBoxControl.h>

// Math Utilities
#include <Math\cgBezierSpline.h>
#include <Math\cgBoundingBox.h>
#include <Math\cgChecksum.h>
#include <Math\cgCollision.h>
#include <Math\cgEulerAngles.h>
#include <Math\cgExtrudedBoundingBox.h>
#include <Math\cgFrustum.h>
#include <Math\cgLeastSquares.h>
#include <Math\cgMathTypes.h>
#include <Math\cgMathUtility.h>
#include <Math\cgMatrix.h>
#include <Math\cgPlane.h>
#include <Math\cgPolynomial.h>
#include <Math\cgQuaternion.h>
#include <Math\cgRandom.h>
#include <Math\cgTransform.h>
#include <Math\cgVector.h>

// Networking
#include <Network\cgBroadcast.h>
#include <Network\cgBroadcastClient.h>

// Physics
#include <Physics\cgPhysicsBody.h>
#include <Physics\cgPhysicsController.h>
#include <Physics\cgPhysicsEngine.h>
#include <Physics\cgPhysicsEntity.h>
#include <Physics\cgPhysicsJoint.h>
#include <Physics\cgPhysicsShape.h>
#include <Physics\cgPhysicsTypes.h>
#include <Physics\cgPhysicsWorld.h>
#include <Physics\Bodies\cgRigidBody.h>
#include <Physics\Controllers\cgCharacterController.h>
#include <Physics\Controllers\cgRagdollController.h>
#include <Physics\Joints\cgBallJoint.h>
#include <Physics\Joints\cgFixedAxisJoint.h>
#include <Physics\Joints\cgHingeJoint.h>
#include <Physics\Joints\cgKinematicControllerJoint.h>
#include <Physics\Shapes\cgBoxShape.h>
#include <Physics\Shapes\cgCapsuleShape.h>
#include <Physics\Shapes\cgConeShape.h>
#include <Physics\Shapes\cgConvexHullShape.h>
#include <Physics\Shapes\cgConvexShape.h>
#include <Physics\Shapes\cgCylinderShape.h>
#include <Physics\Shapes\cgMeshShape.h>
#include <Physics\Shapes\cgNullShape.h>
#include <Physics\Shapes\cgSphereShape.h>

// Navigation
#include <Navigation/cgNavigationMesh.h>
#include <Navigation/cgNavigationTile.h>
#include <Navigation/cgNavigationHandler.h>
#include <Navigation/cgNavigationAgent.h>
#include <Navigation/cgNavigationTypes.h>

// Rendering
#include <Rendering\cgBillboardBuffer.h>
#include <Rendering\cgImageProcessor.h>
#include <Rendering\cgObjectRenderContext.h>
#include <Rendering\cgObjectRenderQueue.h>
#include <Rendering\cgParticleEmitter.h>
#include <Rendering\cgRenderDriver.h>
#include <Rendering\cgRenderingCapabilities.h>
#include <Rendering\cgRenderingTypes.h>
#include <Rendering\cgResampleChain.h>
#include <Rendering\cgSampler.h>
#include <Rendering\cgVertexFormats.h>
#include <Rendering\Processors\cgAntialiasProcessor.h>
#include <Rendering\Processors\cgAtmosphericsProcessor.h>
#include <Rendering\Processors\cgDepthOfFieldProcessor.h>
#include <Rendering\Processors\cgGlareProcessor.h>
#include <Rendering\Processors\cgMotionBlurProcessor.h>
#include <Rendering\Processors\cgSSAOProcessor.h>
#include <Rendering\Processors\cgToneMapProcessor.h>

// Resources & Resource Management
#include <Resources\cgAnimationSet.h>
#include <Resources\cgAudioBuffer.h>
#include <Resources\cgBufferFormatEnum.h>
#include <Resources\cgConstantBuffer.h>
#include <Resources\cgDepthStencilTarget.h>
#include <Resources\cgHardwareShaders.h>
#include <Resources\cgHeightMap.h>
#include <Resources\cgIndexBuffer.h>
#include <Resources\cgLandscapeLayerMaterial.h>
#include <Resources\cgMaterial.h>
#include <Resources\cgMesh.h>
#include <Resources\cgRenderTarget.h>
#include <Resources\cgResource.h>
#include <Resources\cgResourceHandles.h>
#include <Resources\cgResourceManager.h>
#include <Resources\cgResourceTypes.h>
#include <Resources\cgScript.h>
#include <Resources\cgStandardMaterial.h>
#include <Resources\cgStateBlocks.h>
#include <Resources\cgSurfaceShader.h>
#include <Resources\cgSurfaceShaderScript.h>
#include <Resources\cgTexture.h>
#include <Resources\cgTexturePool.h>
#include <Resources\cgVertexBuffer.h>

// Scripting
#include <Scripting\cgBindingUtils.h>
#include <Scripting\cgScriptEngine.h>
#include <Scripting\cgScriptInterop.h>
#include <Scripting\cgScriptLibrary.h>
#include <Scripting\cgScriptPackage.h>
#include <Scripting\cgScriptPreprocessor.h>

// Application States
#include <States\cgAppStateManager.h>

// System Level Functionality
#include <System\cgApplication.h>
#include <System\cgAppLog.h>
#include <System\cgAppWindow.h>
#include <System\cgCursor.h>
#include <System\cgEventDispatcher.h>
#include <System\cgExceptions.h>
#include <System\cgFileSystem.h>
#include <System\cgFilterExpression.h>
#include <System\cgImage.h>
#include <System\cgMessageTypes.h>
#include <System\cgPoolAllocator.h>
#include <System\cgProfiler.h>
#include <System\cgPropertyContainer.h>
#include <System\cgReference.h>
#include <System\cgReferenceManager.h>
#include <System\cgString.h>
#include <System\cgStringUtility.h>
#include <System\cgThreading.h>
#include <System\cgTimer.h>
#include <System\cgUID.h>
#include <System\cgVariant.h>
#include <System\cgXML.h>

// Worlds, Scenes & Objects
#include <World\cgInterestAreaSet.h>
#include <World\cgLandscape.h>
#include <World\cgLandscapeTypes.h>
#include <World\cgObjectBehavior.h>
#include <World\cgObjectNode.h>
#include <World\cgObjectSubElement.h>
#include <World\cgScene.h>
#include <World\cgSceneCell.h>
#include <World\cgSceneController.h>
#include <World\cgSceneElement.h>
#include <World\cgSphereTree.h>
#include <World\cgBSPVisTree.h>
#include <World\cgSpatialTree.h>
#include <World\cgVisibilitySet.h>
#include <World\cgWorld.h>
#include <World\cgWorldComponent.h>
#include <World\cgWorldConfiguration.h>
#include <World\cgWorldObject.h>
#include <World\cgWorldQuery.h>
#include <World\cgWorldResourceComponent.h>
#include <World\cgWorldTypes.h>
#include <World\Elements\cgSkyElement.h>
#include <World\Elements\cgNavigationMeshElement.h>
#include <World\Elements\cgBSPVisTreeElement.h>
#include <World\Lighting\cgLightingManager.h>
#include <World\Lighting\cgLightingTypes.h>
#include <World\Lighting\cgRadianceGrid.h>
#include <World\Lighting\cgShadowGenerator.h>
#include <World\Objects\cgActor.h>
#include <World\Objects\cgBoneObject.h>
#include <World\Objects\cgCameraObject.h>
#include <World\Objects\cgDirectionalLight.h>
#include <World\Objects\cgDummyObject.h>
#include <World\Objects\cgFixedAxisJointObject.h>
#include <World\Objects\cgGroupObject.h>
#include <World\Objects\cgHemisphereLight.h>
#include <World\Objects\cgHingeJointObject.h>
#include <World\Objects\cgJointObject.h>
#include <World\Objects\cgKinematicControllerJointObject.h>
#include <World\Objects\cgLightObject.h>
#include <World\Objects\cgMeshObject.h>
#include <World\Objects\cgParticleEmitterObject.h>
#include <World\Objects\cgPointLight.h>
#include <World\Objects\cgProjectorLight.h>
#include <World\Objects\cgSkinObject.h>
#include <World\Objects\cgSoundEmitterObject.h>
#include <World\Objects\cgSpatialTreeObject.h>
#include <World\Objects\cgSplineObject.h>
#include <World\Objects\cgSpotLight.h>
#include <World\Objects\cgTargetObject.h>
#include <World\Objects\cgNavigationWaypoint.h>
#include <World\Objects\cgNavigationPatrolPoint.h>
#include <World\Objects\Elements\cgAnimationSetElement.h>
#include <World\Objects\Elements\cgBoxCollisionShapeElement.h>
#include <World\Objects\Elements\cgCapsuleCollisionShapeElement.h>
#include <World\Objects\Elements\cgCollisionShapeElement.h>
#include <World\Objects\Elements\cgConeCollisionShapeElement.h>
#include <World\Objects\Elements\cgCylinderCollisionShapeElement.h>
#include <World\Objects\Elements\cgHullCollisionShapeElement.h>
#include <World\Objects\Elements\cgSphereCollisionShapeElement.h>

#endif // !_CGE_CARBON_H_