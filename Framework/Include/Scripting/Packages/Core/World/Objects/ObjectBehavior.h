#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgObjectBehavior.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace ObjectBehavior
{
    //-------------------------------------------------------------------------
    // Name : registerObjectBehaviorMethods ()
    // Desc : Register the base cgObjectBehavior class methods. Can be called
    //        by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerObjectBehaviorMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::Input::InputListener::registerListenerMethods<type>( engine, typeName );

        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod(typeName, "ObjectNode @+ getParentObject( )", asMETHODPR(type,getParentObject,(),cgObjectNode*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void registerAsInputListener( )", asMETHODPR(type,registerAsInputListener,(),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void unregisterAsInputListener( )", asMETHODPR(type,unregisterAsInputListener,(),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void registerAsPhysicsListener( )", asMETHODPR(type,registerAsPhysicsListener,(),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void unregisterAsPhysicsListener( )", asMETHODPR(type,unregisterAsPhysicsListener,(),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onUpdate( float )", asMETHODPR(type,onUpdate,( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onAttach( ObjectNode@+ )", asMETHODPR(type,onAttach,( cgObjectNode* ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onDetach( ObjectNode@+ )", asMETHODPR(type,onDetach,( cgObjectNode* ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void hitByObject( ObjectNode @+, const Vector3 &in, const Vector3 &in )", asMETHODPR(type,hitByObject,( cgObjectNode*, const cgVector3&, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void objectHit( ObjectNode @+, const Vector3 &in, const Vector3 &in )", asMETHODPR(type,objectHit,( cgObjectNode*, const cgVector3&, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool supportsInputChannels( ) const", asMETHODPR(type,supportsInputChannels,( ) const, bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "IScriptedObjectBehavior@ getScriptObject( )", asFUNCTIONPR(getScriptObject,( type* ), asIScriptObject*), asCALL_CDECL_OBJLAST ) );

    } // End Method registerObjectBehaviorMethods<>

    //-------------------------------------------------------------------------
    // Name : getScriptObject ()
    // Desc : Wrapper function that returns a reference to the script based
    //        'IScriptedObjectBehavior' interface rather than the C++ side 
    //        'cgScriptObject' that is returned by the 'cgObjectBehavior' 
    //        native method of the same name.
    //-------------------------------------------------------------------------
    template <class type>
    asIScriptObject * getScriptObject( type* thisPointer )
    {
        cgScriptObject * object = thisPointer->getScriptObject();
        if ( object && object->getInternalObject() )
        {
            object->getInternalObject()->AddRef();
            return object->getInternalObject();
        
        } // End if valid
        return CG_NULL;
    }

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.ObjectBehavior" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Object Types
            BINDSUCCESS( engine->registerObjectType("ObjectBehavior", 0, asOBJ_REF ) );

            // Interface Types
            BINDSUCCESS( engine->registerInterface("IScriptedObjectBehavior" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgObjectBehavior (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgObjectBehavior>( engine );
            
            // Register the object methods for the base class (template method in header)
            registerObjectBehaviorMethods<cgObjectBehavior>( engine, "ObjectBehavior" );

            ///////////////////////////////////////////////////////////////////////
            // IScriptedObjectBehavior (Interface)
            ///////////////////////////////////////////////////////////////////////

            // Register the /required/ interface methods
            // ToDo: Are any actually required?
            /*BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onAttach()" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onDetach()" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onUpdate( float )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onKeyDown( int, uint )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onKeyUp( int, uint )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onKeyPressed( int, uint )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onMouseMove( const Point &in, const Vector2 &in )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onMouseButtonDown( int, const Point &in )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onMouseButtonUp( int, const Point &in )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onMouseWheelScroll( int, const Point &in )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onHitByObject( ObjectNode@+, const Vector3 &in, const Vector3 &in )" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptedObjectBehavior", "void onObjectHit( ObjectNode@+, const Vector3 &in, const Vector3 &in )" ) );*/

        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::ObjectBehavior