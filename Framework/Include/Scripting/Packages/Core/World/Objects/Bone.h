#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgBoneObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Bone
{
    //-------------------------------------------------------------------------
    // Name : registerNodeMethods ()
    // Desc : Register the base cgGroupNode's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerNodeMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::ObjectNode::registerNodeMethods<type>( engine, typeName );

        // Register methods.
        //BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( float )", asMETHODPR(type, setSize, ( cgFloat ), void), asCALL_THISCALL) );
        //BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSize( ) const", asMETHODPR(type, getSize, ( ) const, cgFloat), asCALL_THISCALL) );

    } // End Method registerNodeMethods<>

    //-------------------------------------------------------------------------
    // Name : registerObjectMethods ()
    // Desc : Register the base cgGroupObject's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerObjectMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::WorldObject::registerObjectMethods<type>( engine, typeName );

        // Register methods.
        //BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( float )", asMETHODPR(type, setSize, ( cgFloat ), void), asCALL_THISCALL) );
        //BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSize( ) const", asMETHODPR(type, getSize, ( ) const, cgFloat), asCALL_THISCALL) );

    } // End Method registerObjectMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Bone" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "BoneObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "BoneNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgBoneObject>( engine );
            registerHandleBehaviors<cgBoneNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_BoneObject", (void*)&RTID_BoneObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_BoneNode", (void*)&RTID_BoneNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgBoneObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgBoneObject>( engine, "BoneObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgBoneNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgBoneNode>( engine, "BoneNode" );            
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Bone