#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgPropertyContainer.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace PropertyContainer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.PropertyContainer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType("PropertyContainer", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgPropertyContainer>( engine );

            // Register the object factories
            BINDSUCCESS( engine->registerObjectBehavior( "PropertyContainer", asBEHAVE_FACTORY, "PropertyContainer@ f()", asFUNCTIONPR(propertyContainerFactory, (), cgPropertyContainer*), asCALL_CDECL) );

            // Register the object methods.
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "bool opEquals(const PropertyContainer &in) const", asMETHODPR(cgPropertyContainer, operator==, (const cgPropertyContainer &) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "bool isPropertyDefined( const String &in) const", asMETHODPR(cgPropertyContainer, isPropertyDefined, (const cgString&) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant & getProperty( const String &in)", asMETHODPR(cgPropertyContainer, getProperty, (const cgString&), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "const Variant & getProperty(const String &in) const", asMETHODPR(cgPropertyContainer, getProperty, (const cgString&) const, const cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, const Variant &in) const", asMETHODPR(cgPropertyContainer, getProperty, (const cgString&, const cgVariant&) const, cgVariant), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "void setProperty(const String &in, const Variant &in)", asMETHODPR(cgPropertyContainer, setProperty, (const cgString&, const cgVariant&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "void removeProperty(const String &in)", asMETHODPR(cgPropertyContainer, removeProperty, (const cgString&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "void clear()", asMETHODPR(cgPropertyContainer, clear, (), void), asCALL_THISCALL) );

            // Wrappers for defaults of various types.
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, int8) const", asFUNCTIONPR(getProperty,( const cgString&, cgInt8, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, uint8) const", asFUNCTIONPR(getProperty,( const cgString&, cgUInt8, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, int) const", asFUNCTIONPR(getProperty,( const cgString&, cgInt32, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, uint) const", asFUNCTIONPR(getProperty,( const cgString&, cgUInt32, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, int64) const", asFUNCTIONPR(getProperty,( const cgString&, cgInt64, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, uint64) const", asFUNCTIONPR(getProperty,( const cgString&, cgUInt64, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, bool) const", asFUNCTIONPR(getProperty,( const cgString&, bool, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, float) const", asFUNCTIONPR(getProperty,( const cgString&, cgFloat, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, double) const", asFUNCTIONPR(getProperty,( const cgString&, cgDouble, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, const String&in) const", asFUNCTIONPR(getProperty,( const cgString&, const cgString&, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, const Vector2&in) const", asFUNCTIONPR(getProperty,( const cgString&, const cgVector2&, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, const Vector3&in) const", asFUNCTIONPR(getProperty,( const cgString&, const cgVector3&, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "Variant getProperty(const String &in, const Vector4&in) const", asFUNCTIONPR(getProperty,( const cgString&, const cgVector4&, cgPropertyContainer* ), cgVariant), asCALL_CDECL_OBJLAST ) );
        }

        //---------------------------------------------------------------------
        //  Name : getProperty ()
        /// <summary>
        /// Wrappers for getProperty() that accept default parameters of
        /// various types.
        /// </summary>
        //---------------------------------------------------------------------
        static cgVariant getProperty( const cgString & name, cgInt8 defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, cgUInt8 defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, cgInt32 defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, cgUInt32 defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, cgInt64 defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, cgUInt64 defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, bool defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, cgFloat defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, cgDouble defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, const cgString & defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, const cgVector2 & defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, const cgVector3 & defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }
        static cgVariant getProperty( const cgString & name, const cgVector4 & defaultValue, cgPropertyContainer * thisPointer )
        {
            return thisPointer->getProperty(name, defaultValue);
        }

        //---------------------------------------------------------------------
        //  Name : propertyContainerFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgPropertyContainer class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgPropertyContainer * propertyContainerFactory( )
        {
            return new cgPropertyContainer();
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::PropertyContainer