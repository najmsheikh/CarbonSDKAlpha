#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgCameraObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Camera
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Camera" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "CameraObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "CameraNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgCameraObject>( engine );
            registerHandleBehaviors<cgCameraNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_CameraObject", (void*)&RTID_CameraObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_CameraNode", (void*)&RTID_CameraNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgCameraObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register base class object methods
            Core::World::Objects::WorldObject::registerObjectMethods<cgCameraObject>( engine, "CameraObject" );

            // Register methods.
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "void setProjectionMode( ProjectionMode )", asMETHODPR(cgCameraObject, setProjectionMode, ( cgProjectionMode::Base ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "void setFOV( float )", asMETHODPR(cgCameraObject, setFOV, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "void setProjectionWindow( float, float, float, float )", asMETHODPR(cgCameraObject, setProjectionWindow, ( cgFloat, cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "void setNearClip( float )", asMETHODPR(cgCameraObject, setNearClip, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "void setFarClip( float )", asMETHODPR(cgCameraObject, setFarClip, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "void setZoomFactor( float )", asMETHODPR(cgCameraObject, setZoomFactor, ( cgFloat ), void), asCALL_THISCALL) );
            
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "ProjectionMode getProjectionMode( ) const", asMETHODPR(cgCameraObject, getProjectionMode, ( ) const, cgProjectionMode::Base), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "float getFOV( ) const", asMETHODPR(cgCameraObject, getFOV, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "void getProjectionWindow( float &inout, float &inout, float &inout, float &inout) const", asMETHODPR(cgCameraObject, getProjectionWindow, ( cgFloat&, cgFloat&, cgFloat&, cgFloat& ) const, void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "float getNearClip( ) const", asMETHODPR(cgCameraObject, getNearClip, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "float getFarClip( ) const", asMETHODPR(cgCameraObject, getFarClip, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraObject", "float getZoomFactor( ) const", asMETHODPR(cgCameraObject, getZoomFactor, ( ) const, cgFloat), asCALL_THISCALL) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgCameraNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register base class object methods
            Core::World::Objects::ObjectNode::registerNodeMethods<cgCameraNode>( engine, "CameraNode" );

            // Register methods.
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setAspectRatio( float, bool )", asMETHODPR(cgCameraNode, setAspectRatio, ( cgFloat, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float getAspectRatio( ) const", asMETHODPR(cgCameraNode, getAspectRatio, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "bool isFrustumLocked( ) const", asMETHODPR(cgCameraNode, isFrustumLocked, ( ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "bool isAspectLocked( ) const", asMETHODPR(cgCameraNode, isAspectLocked, ( ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void lockFrustum( bool )", asMETHODPR(cgCameraNode, lockFrustum, ( bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setInterlaceField( int )", asMETHODPR(cgCameraNode, setInterlaceField, ( cgUInt32 ), void), asCALL_THISCALL) );

            // ToDo: const cgFrustum           & GetFrustum              ( );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "const Matrix & getProjectionMatrix( )", asMETHODPR(cgCameraNode, getProjectionMatrix, ( ), const cgMatrix&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "const Matrix & getViewMatrix( )", asMETHODPR(cgCameraNode, getViewMatrix, ( ), const cgMatrix&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "const Matrix & getPreviousProjectionMatrix( )", asMETHODPR(cgCameraNode, getPreviousProjectionMatrix, ( ) const, const cgMatrix&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "const Matrix & getPreviousViewMatrix( )", asMETHODPR(cgCameraNode, getPreviousViewMatrix, ( ) const, const cgMatrix&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void recordCurrentMatrices( )", asMETHODPR(cgCameraNode, recordCurrentMatrices, ( ), void), asCALL_THISCALL) );
            
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void applyVisibility( )", asMETHODPR(cgCameraNode, applyVisibility, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void computeVisibility( uint, bool )", asMETHODPR(cgCameraNode, computeVisibility, ( cgUInt32, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "VisibilitySet @+ getVisibilitySet( )", asMETHODPR(cgCameraNode, getVisibilitySet, ( ), cgVisibilitySet*), asCALL_THISCALL) );

            // ToDo: cgVolumeQuery::Class        boundsInFrustum         ( const cgBoundingBox & AABB, const cgMatrix * mtxWorld = CG_NULL, cgUInt8 * FrustumBits = CG_NULL, cgInt8 * LastOutside = CG_NULL );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "bool viewportToRay( const Size &in, const Vector2 &in, Vector3 &inout, Vector3 &inout )", asMETHODPR(cgCameraNode, viewportToRay, ( const cgSize&, const cgVector2&, cgVector3&, cgVector3& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "bool viewportToWorld( const Size &in, const Vector2 &in, const Plane &in, Vector3 &inout )", asMETHODPR(cgCameraNode, viewportToWorld, ( const cgSize&, const cgVector2&, const cgPlane&, cgVector3& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "bool viewportToMajorAxis( const Size &in, const Vector2 &in, const Vector3 &in, Vector3 &inout, Vector3 &inout )", asMETHODPR(cgCameraNode, viewportToMajorAxis, ( const cgSize&, const cgVector2&, const cgVector3&, cgVector3&, cgVector3& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "bool viewportToMajorAxis( const Size &in, const Vector2 &in, const Vector3 &in, const Vector3 &in, Vector3 &inout, Vector3 &inout )", asMETHODPR(cgCameraNode, viewportToMajorAxis, ( const cgSize&, const cgVector2&, const cgVector3&, const cgVector3&, cgVector3&, cgVector3& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "bool viewportToCamera( const Size &in, const Vector3 &in, Vector3 &inout )", asMETHODPR(cgCameraNode, viewportToCamera, ( const cgSize&, const cgVector3&, cgVector3& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "bool worldToViewport( const Size &in, const Vector3 &in, Vector3 &inout, bool, bool, bool )", asMETHODPR(cgCameraNode, worldToViewport, ( const cgSize&, const cgVector3&, cgVector3&, bool, bool, bool ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float estimateZoomFactor( const Size &in, const Plane &in )", asMETHODPR(cgCameraNode, estimateZoomFactor, ( const cgSize&, const cgPlane& ), cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float estimateZoomFactor( const Size &in, const Vector3 &in )", asMETHODPR(cgCameraNode, estimateZoomFactor, ( const cgSize&, const cgVector3& ), cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float estimateZoomFactor( const Size &in, const Plane &in, float )", asMETHODPR(cgCameraNode, estimateZoomFactor, ( const cgSize&, const cgPlane&, cgFloat ), cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float estimateZoomFactor( const Size &in, const Vector3 &in, float )", asMETHODPR(cgCameraNode, estimateZoomFactor, ( const cgSize&, const cgVector3&, cgFloat ), cgFloat), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setProjectionMode( ProjectionMode )", asMETHODPR(cgCameraNode, setProjectionMode, ( cgProjectionMode::Base ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setFOV( float )", asMETHODPR(cgCameraNode, setFOV, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setProjectionWindow( float, float, float, float )", asMETHODPR(cgCameraNode, setProjectionWindow, ( cgFloat, cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setNearClip( float )", asMETHODPR(cgCameraNode, setNearClip, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setFarClip( float )", asMETHODPR(cgCameraNode, setFarClip, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setZoomFactor( float )", asMETHODPR(cgCameraNode, setZoomFactor, ( cgFloat ), void), asCALL_THISCALL) );
            
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "ProjectionMode getProjectionMode( ) const", asMETHODPR(cgCameraNode, getProjectionMode, ( ) const, cgProjectionMode::Base), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float getFOV( ) const", asMETHODPR(cgCameraNode, getFOV, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void getProjectionWindow( float &inout, float &inout, float &inout, float &inout) const", asMETHODPR(cgCameraNode, getProjectionWindow, ( cgFloat&, cgFloat&, cgFloat&, cgFloat& ) const, void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float getNearClip( ) const", asMETHODPR(cgCameraNode, getNearClip, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float getFarClip( ) const", asMETHODPR(cgCameraNode, getFarClip, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "float getZoomFactor( ) const", asMETHODPR(cgCameraNode, getZoomFactor, ( ) const, cgFloat), asCALL_THISCALL) );

			BINDSUCCESS( engine->registerObjectMethod( "CameraNode", "void setJitterAA( const Vector2 & in )", asMETHODPR(cgCameraNode, setJitterAA, ( const cgVector2& ), void), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Camera