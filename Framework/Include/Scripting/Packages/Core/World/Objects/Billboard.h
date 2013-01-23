#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgBillboardObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Billboard
{
    //-------------------------------------------------------------------------
    // Name : registerNodeMethods ()
    // Desc : Register the base cgBillboardNode's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerNodeMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::ObjectNode::registerNodeMethods<type>( engine, typeName );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( const SizeF &in )", asMETHODPR(type, setSize, ( const cgSizeF& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const SizeF & getSize( ) const", asMETHODPR(type, getSize, ( ) const, const cgSizeF& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setScale( const Vector2 &in )", asMETHODPR(type, setScale, ( const cgVector2& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Vector2 & getScale( ) const", asMETHODPR(type, getScale, ( ) const, const cgVector2& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setColor( uint )", asMETHODPR(type, setColor, ( cgUInt32 ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "uint getColor( ) const", asMETHODPR(type, getColor, ( ) const, cgUInt32 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRotation( float )", asMETHODPR(type, setRotation, ( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getRotation( ) const", asMETHODPR(type, getRotation, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setHDRScale( float )", asMETHODPR(type, setHDRScale, ( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getHDRScale( ) const", asMETHODPR(type, getHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void enableBillboardAlignment( bool )", asMETHODPR(type, enableBillboardAlignment, ( bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isBillboardAligned( ) const", asMETHODPR(type, isBillboardAligned, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "BillboardBuffer@+ getBuffer( )", asMETHODPR(type, getBuffer, ( ), cgBillboardBuffer* ), asCALL_THISCALL) );
    
    } // End Method registerNodeMethods<>

    //-------------------------------------------------------------------------
    // Name : registerObjectMethods ()
    // Desc : Register the base cgBillboardObject's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerObjectMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::WorldObject::registerObjectMethods<type>( engine, typeName );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( const SizeF &in )", asMETHODPR(type, setSize, ( const cgSizeF& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const SizeF & getSize( ) const", asMETHODPR(type, getSize, ( ) const, const cgSizeF& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setScale( const Vector2 &in )", asMETHODPR(type, setScale, ( const cgVector2& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Vector2 & getScale( ) const", asMETHODPR(type, getScale, ( ) const, const cgVector2& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setColor( uint )", asMETHODPR(type, setColor, ( cgUInt32 ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "uint getColor( ) const", asMETHODPR(type, getColor, ( ) const, cgUInt32 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRotation( float )", asMETHODPR(type, setRotation, ( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getRotation( ) const", asMETHODPR(type, getRotation, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setHDRScale( float )", asMETHODPR(type, setHDRScale, ( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getHDRScale( ) const", asMETHODPR(type, getHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void enableBillboardAlignment( bool )", asMETHODPR(type, enableBillboardAlignment, ( bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isBillboardAligned( ) const", asMETHODPR(type, isBillboardAligned, ( ) const, bool ), asCALL_THISCALL) );

    } // End Method registerObjectMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Billboard" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "BillboardObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "BillboardNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgBillboardObject>( engine );
            registerHandleBehaviors<cgBillboardNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_BillboardObject", (void*)&RTID_BillboardObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_BillboardNode", (void*)&RTID_BillboardNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgBillboardObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgBillboardObject>( engine, "BillboardObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgBillboardNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgBillboardNode>( engine, "BillboardNode" );            
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Billboard