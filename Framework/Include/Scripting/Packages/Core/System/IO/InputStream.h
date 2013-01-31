#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgFileSystem.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace IO {

// Package declaration
namespace InputStream
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.IO.InputStream" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "InputStream", sizeof(cgInputStream), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgInputStream (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the object management behaviors.
            BINDSUCCESS( engine->registerObjectBehavior( "InputStream", asBEHAVE_CONSTRUCT,  "void f()", asFUNCTIONPR(constructInputStream,(cgInputStream*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "InputStream", asBEHAVE_CONSTRUCT,  "void f( const InputStream &in )", asFUNCTIONPR(constructInputStream,(const cgInputStream&,cgInputStream*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "InputStream", asBEHAVE_CONSTRUCT,  "void f( const String &in )", asFUNCTIONPR(constructInputStream,(const cgString&,cgInputStream*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "InputStream", asBEHAVE_CONSTRUCT,  "void f( const String &in, const String &in )", asFUNCTIONPR(constructInputStream,(const cgString&,const cgString&,cgInputStream*),void), asCALL_CDECL_OBJLAST) );
            // ToDo: Following two constructors actually uses size_t... Make sure we register the correct 32bit vs 64 bit types
            BINDSUCCESS( engine->registerObjectBehavior( "InputStream", asBEHAVE_CONSTRUCT,  "void f( const InputStream &in, uint, uint)", asFUNCTIONPR(constructInputStream,( const cgInputStream &, size_t, size_t, cgInputStream*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "InputStream", asBEHAVE_CONSTRUCT,  "void f( const InputStream &in, uint, uint, const String &in)", asFUNCTIONPR(constructInputStream,( const cgInputStream &, size_t, size_t, const cgString&, cgInputStream*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "InputStream", asBEHAVE_DESTRUCT,   "void f()", asFUNCTIONPR(destructInputStream, (cgInputStream*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "InputStream &opAssign(const InputStream &in)", asMETHODPR(cgInputStream, operator=, (const cgInputStream&), cgInputStream&), asCALL_THISCALL) );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool setStreamSource( const String &in )", asMETHODPR(cgInputStream, setStreamSource, (const cgString&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool setStreamSource( const String &in, const String &in )", asMETHODPR(cgInputStream, setStreamSource, (const cgString&, const cgString& ), bool), asCALL_THISCALL) );
            // ToDo: SetStreamSource( void*, size_t) not included due to direct pointer access
            // ToDo: Following two methods actually uses size_t... Make sure we register the correct 32bit vs 64 bit types
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool setStreamSource( const InputStream &in, uint, uint )", asMETHODPR(cgInputStream, setStreamSource, (const cgInputStream &, size_t, size_t ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool setStreamSource( const InputStream &in, uint, uint, const String &in )", asMETHODPR(cgInputStream, setStreamSource, (const cgInputStream &, size_t, size_t, const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "void reset()", asMETHODPR(cgInputStream, reset, (), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool open( )", asMETHODPR(cgInputStream, open, (), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "void close( )", asMETHODPR(cgInputStream, close, (), void), asCALL_THISCALL) );
            // ToDo: Disabled currently due to direct pointer access.
            // ToDo: Following method actually uses size_t... Make sure we register the correct 32bit vs 64 bit types
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "InputStream", "uint read( void *, uint )", asMETHODPR(cgInputStream, read, ( void*, size_t ), size_t), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool seek( int64, SeekOrigin )", asMETHODPR(cgInputStream, seek, ( cgInt64, cgInputStream::SeekOrigin ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "int64 getPosition( ) const", asMETHODPR(cgInputStream, getPosition, ( ) const, cgInt64), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "int64 getLength( ) const", asMETHODPR(cgInputStream, getLength, ( ) const, cgInt64), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool isOpen( ) const", asMETHODPR(cgInputStream, isOpen, ( ) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool isEof( ) const", asMETHODPR(cgInputStream, isEOF, ( ) const, bool ), asCALL_THISCALL) );
            // ToDo: GetBuffer() / ReleaseBuffer() not included due to direct pointer access.
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "StreamType getType( ) const", asMETHODPR(cgInputStream, getType, ( ) const, cgStreamType::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "String getName( ) const", asMETHODPR(cgInputStream, getName, ( ) const, cgString ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "String getSourceFile( ) const", asMETHODPR(cgInputStream, getSourceFile, ( ) const, cgString ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputStream", "bool sourceExists( ) const", asMETHODPR(cgInputStream, sourceExists, ( ) const, bool ), asCALL_THISCALL) );

        }

        //-----------------------------------------------------------------------------
        //  Name : constructInputStream ()
        /// <summary>
        /// This is a wrapper for the default cgInputStream constructor, since it
        /// is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructInputStream( cgInputStream *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgInputStream();
        }

        //-----------------------------------------------------------------------------
        //  Name : constructInputStream ()
        /// <summary>
        /// This is a wrapper for an alternative cgInputStream constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructInputStream( const cgInputStream & stream, cgInputStream *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgInputStream( stream );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructInputStream ()
        /// <summary>
        /// This is a wrapper for an alternative cgInputStream constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructInputStream( const cgString & fileName, cgInputStream *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgInputStream( fileName );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructInputStream ()
        /// <summary>
        /// This is a wrapper for an alternative cgInputStream constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructInputStream( const cgString & fileName, const cgString & streamName, cgInputStream *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgInputStream( fileName, streamName );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructInputStream ()
        /// <summary>
        /// This is a wrapper for an alternative cgInputStream constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructInputStream( const cgInputStream & container, size_t offset, size_t length, cgInputStream *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgInputStream( container, offset, length );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructInputStream ()
        /// <summary>
        /// This is a wrapper for an alternative cgInputStream constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructInputStream( const cgInputStream & container, size_t offset, size_t length, const cgString & streamName, cgInputStream *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgInputStream( container, offset, length, streamName );
        }

        //-----------------------------------------------------------------------------
        //  Name : destructInputStream ()
        /// <summary>
        /// A wrapper for the cgInputStream destructor. This will be triggered
        /// whenever a it goes out of scope inside the script.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void destructInputStream( cgInputStream * thisPointer )
        {
            thisPointer->~cgInputStream();
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::IO::InputStream