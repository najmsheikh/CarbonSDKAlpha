#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgBillboardBuffer.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace BillboardBuffer
{
    //-------------------------------------------------------------------------
    // Name : registerBillboardMethods ()
    // Desc : Register the base cgBillboard class methods. Can be called
    //        by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerBillboardMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setPosition( const Vector3 &in )", asMETHODPR(type, setPosition, (const cgVector3&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setPosition( float, float, float )", asMETHODPR(type, setPosition, (cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Vector3& getPosition( ) const", asMETHODPR(type, getPosition, () const, const cgVector3&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( SizeF &in )", asMETHODPR(type, setSize, (const cgSizeF&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( float, float )", asMETHODPR(type, setSize, (cgFloat, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const SizeF& getSize( ) const", asMETHODPR(type, getSize, () const, const cgSizeF&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setFrameGroup( int16 )", asMETHODPR(type, setFrameGroup, (cgInt16), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setFrame( int16, bool )", asMETHODPR(type, setFrame, (cgInt16, bool), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setVisible( bool )", asMETHODPR(type, setVisible, (bool), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool getVisible( ) const", asMETHODPR(type, getVisible, () const, bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void getColor( uint )", asMETHODPR(type, setColor, (cgUInt32), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "uint getColor( ) const", asMETHODPR(type, getColor, () const, cgUInt32), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool update( )", asMETHODPR(type, update, (), bool), asCALL_THISCALL) );
    
    } // End Method registerBillboardMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.BillboardBuffer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "BillboardBufferFlags" ) );

            // Reference types
            BINDSUCCESS( engine->registerObjectType( "BillboardBuffer", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "Billboard", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "Billboard2D", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "Billboard3D", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgBillboardBuffer::Flags (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "BillboardBufferFlags", "SupportSorting", cgBillboardBuffer::SupportSorting ) );
            BINDSUCCESS( engine->registerEnumValue( "BillboardBufferFlags", "ScreenSpace", cgBillboardBuffer::ScreenSpace ) );
            BINDSUCCESS( engine->registerEnumValue( "BillboardBufferFlags", "OrientationY", cgBillboardBuffer::OrientationY ) );
            BINDSUCCESS( engine->registerEnumValue( "BillboardBufferFlags", "OrientationAxis", cgBillboardBuffer::OrientationAxis ) );

            ///////////////////////////////////////////////////////////////////////
            // cgBillboard (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgBillboard>( engine );

            // Register methods
            registerBillboardMethods<cgBillboard>( engine, "Billboard" );

            ///////////////////////////////////////////////////////////////////////
            // cgBillboard2D (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgBillboard2D>( engine );

            // Register base class methods
            registerBillboardMethods<cgBillboard2D>( engine, "Billboard2D" );

            // Register the object factories
            BINDSUCCESS( engine->registerObjectBehavior( "Billboard2D", asBEHAVE_FACTORY, "Billboard2D@ f( )", asFUNCTIONPR(billboard2DFactory, ( ), cgBillboard2D*), asCALL_CDECL) );

            ///////////////////////////////////////////////////////////////////////
            // cgBillboard3D (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgBillboard3D>( engine );

            // Register base class methods
            registerBillboardMethods<cgBillboard3D>( engine, "Billboard3D" );

            // Register the object factories
            BINDSUCCESS( engine->registerObjectBehavior( "Billboard3D", asBEHAVE_FACTORY, "Billboard3D@ f( )", asFUNCTIONPR(billboard3DFactory, ( ), cgBillboard3D*), asCALL_CDECL) );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "void setScale( const Vector2 &in )", asMETHODPR(cgBillboard3D, setScale, (const cgVector2&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "void setScale( float, float )", asMETHODPR(cgBillboard3D, setScale, (cgFloat, cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "const Vector2& getScale( ) const", asMETHODPR(cgBillboard3D, getScale, () const, const cgVector2&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "void setRotation( float )", asMETHODPR(cgBillboard3D, setRotation, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "float getRotation( ) const", asMETHODPR(cgBillboard3D, getRotation, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "void setDirection( const Vector3 &in )", asMETHODPR(cgBillboard3D, setDirection, (const cgVector3&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "void setDirection( float, float, float )", asMETHODPR(cgBillboard3D, setDirection, (cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "const Vector3& getDirection( ) const", asMETHODPR(cgBillboard3D, getDirection, () const, const cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "void setHDRScale( float )", asMETHODPR(cgBillboard3D, setHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Billboard3D", "float getHDRScale( ) const", asMETHODPR(cgBillboard3D, getHDRScale, ( ) const, cgFloat), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // cgBillboardBuffer (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgBillboardBuffer>( engine );

            // Register the object factories
            BINDSUCCESS( engine->registerObjectBehavior( "BillboardBuffer", asBEHAVE_FACTORY, "BillboardBuffer@ f( )", asFUNCTIONPR(billboardBufferFactory, ( ), cgBillboardBuffer*), asCALL_CDECL) );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "bool prepareBuffer( uint, RenderDriver@+, InputStream )", asMETHODPR(cgBillboardBuffer, prepareBuffer, ( cgUInt32, cgRenderDriver*, cgInputStream ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "bool prepareBuffer( uint, RenderDriver@+, InputStream, InputStream )", asMETHODPR(cgBillboardBuffer, prepareBuffer, ( cgUInt32, cgRenderDriver*, cgInputStream, cgInputStream ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "bool prepareBufferFromAtlas( uint, RenderDriver@+, InputStream )", asMETHODPR(cgBillboardBuffer, prepareBufferFromAtlas, ( cgUInt32, cgRenderDriver*, cgInputStream ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "bool prepareBufferFromAtlas( uint, RenderDriver@+, InputStream, InputStream )", asMETHODPR(cgBillboardBuffer, prepareBufferFromAtlas, ( cgUInt32, cgRenderDriver*, cgInputStream, cgInputStream ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "int addBillboard( Billboard@ )", asMETHODPR(cgBillboardBuffer, addBillboard, ( cgBillboard* ), cgInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "bool buildUniformFrames( int16, int16 )", asMETHODPR(cgBillboardBuffer, buildUniformFrames, ( cgInt16, cgInt16 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "int16 addFrameGroup( )", asMETHODPR(cgBillboardBuffer, addFrameGroup, ( ), cgInt16), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "int16 addFrameGroup( const String &in )", asMETHODPR(cgBillboardBuffer, addFrameGroup, ( const cgString& ), cgInt16), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "int16 addFrame( int16, const Rect &in )", asMETHODPR(cgBillboardBuffer, addFrame, ( cgInt16, const cgRect& ), cgInt16), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "int16 addFrame( int16, const Rect &in, const String &in )", asMETHODPR(cgBillboardBuffer, addFrame, ( cgInt16, const cgRect&, const cgString& ), cgInt16), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "bool endPrepare( )", asMETHODPR(cgBillboardBuffer, endPrepare, ( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "void render( int32, int32, bool )", asMETHODPR(cgBillboardBuffer, render, ( cgInt32, cgInt32, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "void renderSorted( CameraNode@+, const Matrix &in, int32, int32, bool )", asMETHODPR(cgBillboardBuffer, renderSorted, ( cgCameraNode*, cgMatrix*, cgInt32, cgInt32, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "void billboardUpdated( int32 )", asMETHODPR(cgBillboardBuffer, billboardUpdated, ( cgInt32 ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "Billboard@+ getBillboard( int32 )", asMETHODPR(cgBillboardBuffer, getBillboard, ( cgInt32 ), cgBillboard*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "uint getBillboardCount( ) const", asMETHODPR(cgBillboardBuffer, getBillboardCount, ( ) const, cgUInt32), asCALL_THISCALL) );
            // ToDo: void                  * getBillboardVertices    ( cgInt32 billboardId );
            // ToDo: const FrameDesc       * getFrameData            ( cgInt16 groupIndex, cgInt16 frameIndex ) const;
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "int16 getFrameGroupIndex( const String &in ) const", asMETHODPR(cgBillboardBuffer, getFrameGroupIndex, ( const cgString& ) const, cgInt16), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "int16 getFrameIndex( int16, const String &in ) const", asMETHODPR(cgBillboardBuffer, getFrameIndex, ( cgInt16, const cgString& ) const, cgInt16), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "SurfaceShaderHandle getSurfaceShader( ) const", asMETHODPR(cgBillboardBuffer, getSurfaceShader, ( ) const, cgSurfaceShaderHandle), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "void setDirty( bool )", asMETHODPR(cgBillboardBuffer, setDirty, ( bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BillboardBuffer", "void clear( bool )", asMETHODPR(cgBillboardBuffer, clear, ( bool ), void), asCALL_THISCALL) );
        }

        //---------------------------------------------------------------------
        //  Name : billboardBufferFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgBillboardBuffer class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgBillboardBuffer * billboardBufferFactory( )
        {
            return new cgBillboardBuffer( );
        }

        //---------------------------------------------------------------------
        //  Name : billboard2DFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgBillboard2D class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgBillboard2D * billboard2DFactory( )
        {
            return new cgBillboard2D( );
        }

        //---------------------------------------------------------------------
        //  Name : billboard3DFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgBillboard3D class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgBillboard3D * billboard3DFactory( )
        {
            return new cgBillboard3D( );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::BillboardBuffer